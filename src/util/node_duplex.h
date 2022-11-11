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
class NodeDuplex
    : public Node<MsgChannelPtr, NodeType::NODE_FULL_DUPLEX>,
      public FullDuplex<typename MsgChannelPtr::element_type::value_type> {
 public:
  typedef typename MsgChannelPtr::element_type::value_type msg_type;
  explicit NodeDuplex(const std::string& name)
      : Node<MsgChannelPtr, NodeType::NODE_FULL_DUPLEX>(name) {}
  virtual ~NodeDuplex() {}
  // NodeDuplex  has its own way to implement `DoWork` function.
  void DoWork() override {
    ThreadAffinity();
    auto chn_index = IncThreads();
    std::cout << chn_index << " up_channels_.size " << up_channels_.size()
              << std::endl;
    // auto& channel = up_channels_.at(chn_index);
    MsgChannelPtr& channel = up_channels_.at(chn_index);
    std::cout << channel->Id() << std::endl;
    std::function<void(MsgChannelPtr&)> handler =
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
  void HandleWritting(MsgChannelPtr& channel) override {
    msg_type msg;
    std::cout << channel->Id() << std::endl;
    // receive
    channel->ReadMessage(msg);
    if (msg == nullptr) {
      return;
    }
    // process
    auto res = HandleMsg(msg);
    // write
    FDWrite(res);
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
  virtual int FDRecv() = 0;
  /**
   * @brief Define the way to write msg to the file
   * descriptor.
   *
   * @param msg
   * @return int
   */
  virtual int FDWrite(const msg_type& msg) = 0;
};

#endif  // SRC_EXAMPLE_APP_SRC_NODE_DUPLEX_H_
