/**
 * @file node_collector.cc
 * @author peng lei (plhitsz@outlook.com)
 * @brief
 * @version 0.1
 * @date 2022-11-14
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "node_collector.h"

#include "util/net_msg.h"
namespace bats {
namespace src {

bool Collector::Init() {
  bats::util::Settings& settings = bats::util::Settings::getInstance();
  max_block_size_ = settings.getValue<int>("coding.max_block_size", 65540);
  coding_threshold_ = settings.getValue<int>("coding.encode_threshold", 30000);
  src_addr_ = settings.getValue<std::string>("tun.address", "10.0.0.1");

  // init route table and flow recorder
  route_t_ = std::make_shared<RouteTable>(32);
  route_t_->LoadConfig();
  flow_r_ = std::make_shared<FlowRecoder>(PROTO_TCP);
  timeout_mgr_ = std::make_shared<TimerManager>();
  timeout_mgr_->StartAsThread();
  is_stop_ = false;
  return true;
}

void Collector::HandleMsg(const msg_type& msg) {
  auto net = std::dynamic_pointer_cast<bats::util::NetMsg>(msg);
  assert(net != nullptr);
  // Find a proper decoder for this msg according to its real destination.
  std::string nexthop = "127.0.0.1";
  if (!simulate_mode_) {
    nexthop = route_t_->RouteMatch(net->DstAddr());
  }

  if (unlikely(nexthop.empty())) {
    LOG(WARNING) << "Drop the msg without default route.";
    return;
  }

  // updating the state of flow to decide which msg need to be coded.
  if (flow_r_) {
    flow_r_->FlowUpdate(net);
  }

  if (msg->NeedCoded()) {
    BufferingData(msg, nexthop);
  } else {
    ForceRelayData(msg, nexthop);
  }
}

/**
 * @brief relay the timeout buffer to a downstream service.
 *
 * @param t A timer object.
 */
void Collector::HandleTimeoutBuffer(Timer_ptr timeout) {
  auto tbuf = std::dynamic_pointer_cast<TimoutBuffer>(timeout);
  if (tbuf == nullptr) {
    return;
  }
  auto& bats_buffer = tbuf->buffer;

  std::unique_lock<std::mutex> lg(mutex_, std::defer_lock);
  lg.lock();
  auto buf = bats_buffer->GetBuf();
  if (buf == nullptr || buf->FilledBytes() <= 0) {
    lg.unlock();
    return;
  }
  // setting the valid length of buffer.
  buf->resize(buf->FilledBytes());
  bats_buffer->ResetBuf();
  lg.unlock();
#ifndef NDEBUG
  LOG(INFO) << "forward " << buf->size() << " bytes to encoder, file id "
            << buf->id() << " Timer size: " << timeout_mgr_->Size()
            << std::endl;
#endif
  Dispatch(buf);
}

/**
 * @brief Get the Bats Endpoint object according to the address of decoder.
 *
 * @param nexthop The address of decoder.
 * @return BatsBuffer_ptr&
 */
BatsBuffer_ptr& Collector::GetBatsBuffer(const std::string& nexthop) {
  auto itr = bats_buffer_map_.find(nexthop);
  if (unlikely(itr == bats_buffer_map_.end())) {
    bats_buffer_map_[nexthop] =
        std::make_shared<BatsBuffer>(src_addr_, nexthop, 8888, max_block_size_);
    // register the buffer to the timer manager.
    auto tbuf = std::make_shared<TimoutBuffer>(
        15, std::bind(&Collector::HandleTimeoutBuffer, this,
                      std::placeholders::_1));
    tbuf->buffer = bats_buffer_map_.at(nexthop);
    timeout_mgr_->AddTimer(tbuf);
  }
  return bats_buffer_map_.at(nexthop);
}

void Collector::ForceRelayData(const msg_type& msg,
                               const std::string& nexthop) {
  std::vector<bats::util::BatsMsg_ptr> to_encoder;
  std::unique_lock<std::mutex> lg(mutex_, std::defer_lock);
  lg.lock();
  auto& bats_buffer = GetBatsBuffer(nexthop);
  auto buffer = bats_buffer->GetBuf();
  if (buffer == nullptr) {
    bats_buffer->ResetBuf();
    buffer = bats_buffer->GetBuf();
  }
  if (buffer->FilledBytes() > 0) {
#ifndef NDEBUG
    LOG(INFO) << "forward " << buffer->size() << " bytes to encoder, file id "
              << buffer->id() << std::endl;
#endif
    buffer->resize(buffer->FilledBytes());
    to_encoder.push_back(buffer);
    bats_buffer->ResetBuf();
    buffer = bats_buffer->GetBuf();
  }
  // reserve header room for raw packet.
  buffer->reserveHeader(PROTOCOL_OVERHEAD);
  buffer->fill((const octet*)msg->begin(), msg->size());
  buffer->resize(buffer->FilledBytes());
  buffer->NeedCoded() = false;
  to_encoder.push_back(buffer);
  bats_buffer->ResetBuf();
  lg.unlock();

  // to encoder
  for (auto& b : to_encoder) {
    Dispatch(b);
  }
  std::vector<bats::util::BatsMsg_ptr>().swap(to_encoder);
}

void Collector::BufferingData(const msg_type& msg, const std::string& nexthop) {
  std::vector<bats::util::BatsMsg_ptr> to_encoder;
  std::unique_lock<std::mutex> lg(mutex_, std::defer_lock);
  lg.lock();
  auto& bats_buffer = GetBatsBuffer(nexthop);
  auto buffer = bats_buffer->GetBuf();
  if (buffer == nullptr) {
    bats_buffer->ResetBuf();
    buffer = bats_buffer->GetBuf();
  }
  // buffer is full
  if (buffer->FilledBytes() + msg->size() > buffer->size()) {
    // enqueue the previous buffer
    buffer->resize(buffer->FilledBytes());
#ifndef NDEBUG
    LOG(INFO) << "forward " << buffer->size() << " bytes to encoder, file id "
              << buffer->id() << std::endl;
#endif
    to_encoder.push_back(buffer);
    // create a new one
    bats_buffer->ResetBuf();
  }

  buffer->fill((const octet*)msg->begin(), msg->size());
  // got enough bytes
  if (buffer->FilledBytes() > coding_threshold_) {
    buffer->resize(buffer->FilledBytes());
#ifndef NDEBUG
    LOG(INFO) << "forward " << buffer->size() << " bytes to encoder, file id "
              << buffer->id() << std::endl;
#endif
    bats_buffer->ResetBuf();
    to_encoder.push_back(buffer);
  }
  lg.unlock();

  // to encoder
  for (auto& b : to_encoder) {
    Dispatch(b);
  }
  std::vector<bats::util::BatsMsg_ptr>().swap(to_encoder);
}

// dispatch msg according to its type(coded or none-coded?)
inline void Collector::Dispatch(const msg_type& msg) {
  if (unlikely(down_channels_.empty())) {
    return;
  }

  if (unlikely(!dispath_chn_init_)) {
    for (auto& chn : down_channels_) {
      if (chn->Name().find(":UDP") == std::string::npos) {
        encode_channles_.push_back(chn);
      } else {
        udp_channel_ = chn;
        dispath_chn_init_ = true;
      }
    }
  }

  if (msg->NeedCoded()) {
    auto id = msg->id() % encode_channles_.size();
    encode_channles_.at(id)->WriteMessage(msg);
  } else {
    udp_channel_->WriteMessage(msg);
  }
}

}  // namespace src
}  // namespace bats
