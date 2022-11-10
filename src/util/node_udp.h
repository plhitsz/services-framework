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

#ifndef SRC_EXAMPLE_APP_SRC_NODE_UDP_H_
#define SRC_EXAMPLE_APP_SRC_NODE_UDP_H_
#include "channel.h"
#include "node.h"

namespace bats {
namespace src {
/**
 * @brief A udp service which is used to send protocol data to an endpoint.
 *
 */
class Udp : public Node<MsgChannel, NodeType::NODE_UDP> {
 public:
  typedef typename MsgChannel::value_type msg_type;
  explicit Udp(uint16_t port) : port_(port) {
    if (!Init()) {
      throw std::runtime_error("UDP node init failed.");
    }
  }
  virtual ~Udp() { close(fd_); }
  virtual int GetFd() { return fd_; }
  virtual int SourceRecv();
  virtual int SinkWrite(const msg_type& msg);
  virtual auto HandleMsg(const msg_type& msg) -> msg_type;

 private:
  bool Init();

 private:
  uint16_t port_ = 0;
  int fd_ = -1;
  DISALLOW_COPY_AND_ASSIGN(Udp)
};

typedef std::shared_ptr<Udp> Udp_ptr;

}  // namespace src
}  // namespace bats
#endif  // SRC_EXAMPLE_APP_SRC_NODE_UDP_H_
