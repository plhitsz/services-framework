/**
 * @file node_encoder.h
 * @author peng lei (plhitsz@outlook.com)
 * @brief
 * @version 0.1
 * @date 2022-11-10
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef SRC_EXAMPLE_APP_SRC_NODE_ENCODER_H_
#define SRC_EXAMPLE_APP_SRC_NODE_ENCODER_H_

#include "all_schema.h"
#include "encode.h"
#include "protocol.h"

using BatsEncoder_ptr = std::shared_ptr<bats::BatsEncoder>;

class Encoder : public Node<MsgChannel, NodeType::NODE_RELAY> {
 public:
  typedef typename MsgChannel::value_type msg_type;
  Encoder() {
    encode_ = std::make_shared<bats::BatsEncoder>();
    // used to reserve header room for coded msg.
    encode_->encodingInfo().mtu = 1500;
    encode_->encodingInfo().proto_header = PROTOCOL_OVERHEAD;

    // or use PrecodeNone to get rid of precoing
    auto pre = std::make_shared<bats::DefaultPrecodeSchema>();
    auto schema = std::make_shared<bats::EncodingSchema>();
    encode_->setEncodeSchema(schema);
    encode_->setPrecodeSchema(pre);
  }
  virtual ~Encoder() {}
  virtual int GetFd() { return -1; }
  virtual int SourceRecv() {}
  virtual int SinkWrite(const msg_type& msg) {}
  virtual auto HandleMsg(const msg_type& msg) -> msg_type {
    if (!msg) {
      return nullptr;
    }
    assert(msg->size() < 65540);
    auto& out = GetChannel(0, false).GetQueue();
    if (unlikely(StopSignal(msg))) {
      LOG(INFO) << GetName() << " received stop signal";
      out.WaitEnqueue(msg);
      return;
    }
    // FIXME: how to use `HandleMsg` and `encode`
    // callmap: HandleMsg--> DispatchMsg
    if (msg->NeedCoded()) {
      // encode_->encode(msg, out);
    } else {
      out.WaitEnqueue(msg);
    }
    // FIXME: optimize
    // data flow: TUN  ---> encoder ----->
    //                 ---> transparent transmit --->
  }

 private:
  BatsEncoder_ptr encode_ = nullptr;
};

#endif  // SRC_EXAMPLE_APP_SRC_NODE_ENCODER_H_
