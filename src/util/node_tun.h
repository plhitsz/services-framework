/**
 * @file tun_node.h
 * @author peng lei (plhitsz@outlook.com)
 * @brief
 * @version 0.1
 * @date 2022-11-10
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef SRC_EXAMPLE_APP_SRC_NODE_TUN_H_
#define SRC_EXAMPLE_APP_SRC_NODE_TUN_H_
#include "channel.h"
#include "node.h"

namespace bats {
namespace src {

/**
 * @brief A tun service.
 *
 */
class Tun : public Node<MsgChannel, NodeType::NODE_TUN> {
 public:
  typedef typename MsgChannel::value_type msg_type;
  Tun() {
    if (!Init()) {
      throw std::runtime_error("TUN node init failed.");
    }
  }
  virtual ~Tun() { close(fd_); }
  virtual int GetFd() { return fd_; }
  virtual int SourceRecv();
  virtual int SinkWrite(const msg_type& msg);
  virtual auto HandleMsg(const msg_type& msg) -> msg_type;

 private:
  bool Init();

 private:
  int fd_ = -1;
  DISALLOW_COPY_AND_ASSIGN(Tun)
};

typedef std::shared_ptr<Tun> Tun_ptr;

}  // namespace src
}  // namespace bats
#endif  // SRC_EXAMPLE_APP_SRC_NODE_TUN_H_
