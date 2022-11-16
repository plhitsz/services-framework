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

#include <memory>

#include "channel.h"
#include "io/poll_data.h"
#include "io/poller.h"
#include "node.h"

class NodeDuplex;
using NodeDuplexPtr = std::shared_ptr<NodeDuplex>;
/**
 * @brief A duplex node which may act as udp or tcp service.
 *
 */
class NodeDuplex
    : public Node<MsgChannelPtr, NodeType::NODE_FULL_DUPLEX>,
      public FullDuplex<typename MsgChannelPtr::element_type::value_type>,
      public std::enable_shared_from_this<NodeDuplex> {
 public:
  typedef typename MsgChannelPtr::element_type::value_type msg_type;
  explicit NodeDuplex(const std::string& name)
      : Node<MsgChannelPtr, NodeType::NODE_FULL_DUPLEX>(name) {}
  virtual ~NodeDuplex() {}
  // Duplex nodes use `up_channel` to recv data from poller and use
  // `down_channel` to store the data which is requested to be written to `fd`.
  void AddChannel(MsgChannelPtr& channel,
                  ChnType ct = ChnType::CHN_OUT) override final {
    auto& channels = ((ct == ChnType::CHN_IN) ? down_channels_ : up_channels_);
    channels.push_back(channel);
  }
  MsgChannelPtr& GetChannel(int i,
                            ChnType ct = ChnType::CHN_OUT) override final {
    auto& channels = ((ct == ChnType::CHN_IN) ? down_channels_ : up_channels_);
    return channels.at(i);
  }
  int GetChannelNum(ChnType ct = ChnType::CHN_OUT) const override final {
    auto& channels = ((ct == ChnType::CHN_IN) ? down_channels_ : up_channels_);
    return channels.size();
  }
  /**
   * @brief Read msg from upstream channels, and forward it to a file
   * descriptor.
   *
   * @param channel
   */
  void HandleWritting(MsgChannelPtr& channel) override final {
    msg_type msg;
    // receive
    channel->ReadMessage(msg);
    if (msg == nullptr) {
      return;
    }
    HandleMsg(msg);
  }
  /**
   * @brief do some processing for the received msg.
   *
   * @param msg
   * @return msg_type
   */
  void HandleMsg(const msg_type& msg) override final {
    // write
    FDWrite(msg);
  }
  /**
   * @brief For udp or tun node, they are using `FDRecv` to get input data.
   * Other `relay` nodes no neeed to call this function.
   *
   * @param node
   * @return true
   * @return false
   */
  inline bool RegisterToPoller() {
    auto self = shared_from_this();
    bats::io::PollRequest req;
    req.fd = this->GetFd();
    assert(req.fd > 0);
    fcntl(req.fd, F_SETFL, O_NONBLOCK);
    req.events = EPOLLIN | EPOLLET;  // level trigger
    req.timeout_ms = 0;
    req.callback = [self](const bats::io::PollResponse& rsp) {
      auto& response = rsp;
      if (response.events & EPOLLIN) {
        while (true) {
          auto ret = self->FDRecv();
          if (ret < 0 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {
            break;
          }
        }
      }
    };
    return bats::io::Poller::Instance()->Register(req);
  }

 private:
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
