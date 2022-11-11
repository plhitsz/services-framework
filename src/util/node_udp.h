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
#include "node_duplex.h"

namespace bats {
namespace src {
/**
 * @brief A udp service which is used to send protocol data to an endpoint.
 *
 */
class Udp : public NodeDuplex {
 public:
  explicit Udp(uint16_t port) : NodeDuplex("UDP"), port_(port) {
    if (!Init()) {
      throw std::runtime_error("UDP node init failed.");
    }
  }
  virtual ~Udp() {}
  // Node
  virtual auto HandleMsg(const msg_type& msg) -> msg_type;

  // FullDuplex
  int FDRecv() override;
  int FDWrite(const msg_type& msg) override;
  bool Init() override;

 private:
  uint16_t port_ = 0;
  DISALLOW_COPY_AND_ASSIGN(Udp)
};

typedef std::shared_ptr<Udp> Udp_ptr;

}  // namespace src
}  // namespace bats
#endif  // SRC_EXAMPLE_APP_SRC_NODE_UDP_H_
