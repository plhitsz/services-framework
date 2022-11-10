/**
 * @file node_udp.cc
 * @author peng lei (plhitsz@outlook.com)
 * @brief
 * @version 0.1
 * @date 2022-11-10
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "node_udp.h"

#include <arpa/inet.h>
#include <glog/logging.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>

namespace bats {
namespace src {

bool Udp::Init() {
  if ((fd_ = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    return false;
  }
  struct sockaddr_in servaddr;
  memset(&servaddr, 0, sizeof(servaddr));
  // Filling server information
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_port = htons(port_);
  int64_t temp = 1L;
  if (setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &temp, sizeof(int)) < 0) {
    LOG(ERROR) << "setsockopt error " << strerror(errno);
    return false;
  }

  if (setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, &temp, sizeof(int)) < 0) {
    LOG(ERROR) << "setsockopt error " << strerror(errno);
    return false;
  }

  // Bind the socket with the server address
  if (bind(fd_, (const struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
    LOG(ERROR) << "bind error " << strerror(errno);
    return false;
  }
  LOG(INFO) << "bind socket " << fd_ << " on port " << port_;
  return true;
}

int Udp::SourceRecv() {
  typedef typename msg_type::element_type msg_org_type;
  msg_type msg = std::make_shared<msg_org_type>();
  int ret = recvfrom(fd_, (char*)msg->begin(), msg->size(), 0, NULL, NULL);
  if (ret >= 0) {
    Dispatch(msg);
  }
  return ret;
}

int Udp::SinkWrite(const msg_type& msg) {
  return sendto(fd_, (char*)msg->begin(), msg->size(), 0,
                (struct sockaddr*)&msg->encodeInfo().dst_addr,
                sizeof(sockaddr_in));
}

auto Udp::HandleMsg(const msg_type& msg) -> msg_type { return msg; }

}  // namespace src
}  // namespace bats
