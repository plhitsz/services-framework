/**
 * @file node_collector.h
 * @author peng lei (plhitsz@outlook.com)
 * @brief
 * @version 0.1
 * @date 2022-11-14
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef SRC_EXAMPLE_APP_SRC_NODE_COLLECTOR_H_
#define SRC_EXAMPLE_APP_SRC_NODE_COLLECTOR_H_

#include <unordered_map>

#include "bats_buffer.h"
#include "channel.h"
#include "node.h"
#include "util/flow_recorder.h"
#include "util/net_msg.h"
#include "util/route_table.h"
#include "util/settings.h"

namespace bats {
namespace src {
/**
 * @brief Binary data collector. This Collector will do the following work:
 * (1) Accumulate enough bytes or counting the timeout for the buffer before
 * encoding.
 * (2) Make the buffer, which will be sent to encoder, erialized with
 * proper SEQ.
 * (3) Find the proper decoder for the buffer.
 * (4) Tracking the flow state in order to filter the small packet.
 */
class Collector : public Node<MsgChannelPtr, NodeType::NODE_RELAY> {
 public:
  Collector() : Node<MsgChannelPtr, NodeType::NODE_RELAY>("collector") {
    if (!Init()) {
      throw std::runtime_error("Collector Service init failed.");
    }
  }
  virtual ~Collector() { timeout_mgr_->Stop(); }
  // Node
  void HandleMsg(const msg_type& msg) override;
  void HandleWritting(MsgChannelPtr& channel) override {}
  void Dispatch(const msg_type& msg) override;

 private:
  bool Init();
  /**
   * @brief relay the timeout buffer to a downstream service.
   *
   * @param t A timer object.
   */
  void HandleTimeoutBuffer(Timer_ptr t);
  /**
   * @brief Get the Bats Endpoint object according to the address of decoder.
   *
   * @param nexthop The address of decoder.
   * @return BatsBuffer_ptr&
   */
  BatsBuffer_ptr& GetBatsBuffer(const std::string& nexthop);
  /**
   * @brief Force to relay current buffer to the next queue. This is for raw
   * packets.
   *
   * @param msg
   * @param nexthop
   */
  void ForceRelayData(const msg_type& msg, const std::string& nexthop);
  /**
   * @brief Buffering the packet until met the coding condition.
   *
   * @param msg
   * @param nexthop
   */
  void BufferingData(const msg_type& msg, const std::string& nexthop);

 private:
  bool simulate_mode_ = false;
  int max_block_size_ = 0;
  int coding_threshold_ = 0;
  // the address of local TUN device. a encoder or decoder is globally
  // identified by this address.
  std::string src_addr_;
  // multi threads work on this variable
  std::unordered_map<std::string, BatsBuffer_ptr> bats_buffer_map_;
  // The manager of timers.
  TimerManager_ptr timeout_mgr_ = nullptr;
  // routing table of application level.
  RouteTable_ptr route_t_ = nullptr;
  // track the state of each flow.
  FlowRecoder_prt flow_r_ = nullptr;
  MsgChannelPtr udp_channel_ = nullptr;
  std::vector<MsgChannelPtr> encode_channles_;
  bool dispath_chn_init_ = false;
};
}  // namespace src
}  // namespace bats

#endif  // SRC_EXAMPLE_APP_SRC_NODE_COLLECTOR_H_
