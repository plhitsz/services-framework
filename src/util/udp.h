/**
 * @file udp.h
 * @author Penglei
 * @brief
 * @version 0.1
 * @date 2022-10-26
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef SRC_UTIL_UDP_H_
#define SRC_UTIL_UDP_H_

#include <arpa/inet.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "util/service.h"
#include "util/util.h"

namespace base {
namespace util {
/**
 * @brief A udp service which is used to send protocol data to an endpoint.
 *
 */
class Udp : public Service {
 public:
  explicit Udp(uint16_t port) : Service("UDP", SERVICE_UDP), port_(port) {
    if (!Create()) {
      throw std::runtime_error("UDP Service init failed.");
    }
  }
  virtual ~Udp() {}

  int RecvData(char* buf, int len) override;
  int SendData(char* buf, int len) override;
  void Dispatch(BaseMsg_ptr& msg) override;
  bool Create() override;
  void Loop(int id = 0) override;
  int SendData(char* buf, int len, sockaddr_in* dst);

 private:
  uint16_t port_;
  DISALLOW_COPY_AND_ASSIGN(Udp)
};

typedef std::shared_ptr<Udp> Udp_ptr;
}  // namespace util
}  // namespace base
#endif  // SRC_UTIL_UDP_H_
