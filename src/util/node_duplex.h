/**
 * @file node_udp.h
 * @author peng lei (plhitsz@outlook.com)
 * @brief
 * @version 0.1
 * @date 2022-11-10
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef SRC_EXAMPLE_APP_SRC_NODE_DUPLEX_H_
#define SRC_EXAMPLE_APP_SRC_NODE_DUPLEX_H_

#include "channel.h"
#include "node.h"

/**
 * @brief A duplex node which may act as udp or tcp service.
 *
 */
class NodeDuplex : public Node<MsgChannel, NodeType::NODE_FULL_DUPLEX>,
                   public FullDuplex<typename MsgChannel::value_type> {
 public:
  typedef typename MsgChannel::value_type msg_type;
  NodeDuplex(const std::string& name)
      : Node<MsgChannel, NodeType::NODE_FULL_DUPLEX>(name) {}
  virtual ~NodeDuplex() {}
  // NodeDuplex  has its own way to implement `DoWork` function.
  void DoWork() override {
    ThreadAffinity();
    auto chn_index = IncThreads();
    auto& channel = up_channels_.at(chn_index);
    std::function<void(MsgChannel&)> handler = handler =
        std::bind(&NodeDuplex::HandleWritting, this, std::placeholders::_1);
    while (!is_stop_) {
      handler(channel);
    }
  }
  /**
   * @brief Read msg from upstream channels, and forward it to a file
   * descriptor.
   *
   * @param channel
   */
  void HandleWritting(MsgChannel& channel) override {
    msg_type msg;
    // receive
    channel.ReadMessage(msg);
    // process
    auto res = HandleMsg(msg);
    // write
    SinkWrite(res);
  }
  /**
   * @brief do some processing for the received msg.
   *
   * @param msg
   * @return msg_type
   */
  virtual auto HandleMsg(const msg_type& msg) -> msg_type = 0;

  // FullDuplex
  virtual bool Init() = 0;
  /**
   * @brief Define the way to read msg from the file
   * descriptor.
   *
   * @return int
   */
  virtual int SourceRecv() = 0;
  /**
   * @brief Define the way to write msg to the file
   * descriptor.
   *
   * @param msg
   * @return int
   */
  virtual int SinkWrite(const msg_type& msg) = 0;
};

#endif  // SRC_EXAMPLE_APP_SRC_NODE_DUPLEX_H_
