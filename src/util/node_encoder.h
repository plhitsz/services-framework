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
#include "channel.h"
#include "encode.h"
#include "node.h"
#include "protocol.h"
namespace bats {
namespace src {

using BatsEncoder_ptr = std::shared_ptr<bats::BatsEncoder>;

class Encoder : public Node<MsgChannelPtr, NodeType::NODE_RELAY>,
                public std::enable_shared_from_this<Encoder> {
 public:
  Encoder() : Node<MsgChannelPtr, NodeType::NODE_RELAY>("encoder") {
    encode_ = std::make_shared<bats::BatsEncoder>();
    // used to reserve header room for coded msg.
    encode_->encodingInfo().mtu = 1500;
    encode_->encodingInfo().proto_header = PROTOCOL_OVERHEAD;

    // or use PrecodeNone to get rid of precoing
    auto pre = std::make_shared<bats::DefaultPrecodeSchema>();
    auto schema = std::make_shared<bats::EncodingSchema>();
    encode_->setEncodeSchema(schema);
    encode_->setPrecodeSchema(pre);
    encode_callback_f_ =
        std::bind(&Encoder::Dispatch, this, std::placeholders::_1);
    is_stop_ = false;
  }
  virtual ~Encoder() {}
  // handle msg to be coded only.
  virtual void HandleMsg(const msg_type& msg) {
    if (unlikely(!msg)) {
      return;
    }
    assert(msg->size() < 65540);
    if (unlikely(StopSignal(msg))) {
      LOG(INFO) << GetName() << " received stop signal";
      Dispatch(msg);
      return;
    }

    encode_->encode(msg, encode_callback_f_);
  }

 private:
  BatsEncoder_ptr encode_ = nullptr;
  std::function<void(msg_type&)> encode_callback_f_;
};
}  // namespace src
}  // namespace bats
#endif  // SRC_EXAMPLE_APP_SRC_NODE_ENCODER_H_
