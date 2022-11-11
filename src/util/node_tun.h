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
#include "node_duplex.h"

namespace bats {
namespace src {

/**
 * @brief A tun service.
 *
 */
class Tun : public NodeDuplex {
 public:
  Tun() : NodeDuplex("TUN") {
    if (!Init()) {
      throw std::runtime_error("TUN node init failed.");
    }
  }
  virtual ~Tun() {}
  // Node
  auto HandleMsg(const msg_type& msg) -> msg_type override;

  // FullDuplex
  int FDRecv() override;
  int FDWrite(const msg_type& msg) override;
  bool Init() override;

 private:
  DISALLOW_COPY_AND_ASSIGN(Tun)
};

typedef std::shared_ptr<Tun> Tun_ptr;

}  // namespace src
}  // namespace bats
#endif  // SRC_EXAMPLE_APP_SRC_NODE_TUN_H_
