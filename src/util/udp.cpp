#include "udp.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <glog/logging.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <iostream>

namespace base {
namespace util {

bool Udp::Create() {
  if ((fd_ = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    return false;
  }
  struct sockaddr_in servaddr;
  memset(&servaddr, 0, sizeof(servaddr));
  // Filling server information
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_port = htons(port_);
  LOG(INFO) << "bind socket " << fd_ << " on port " << port_;
  long temp = 1L;
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
  return true;
}

int Udp::RecvData(char* buf, int len) {
  return recvfrom(fd_, buf, len, 0, NULL, NULL);
}

int Udp::SendData(char* buf, int len) {
  int totalbytes = 0;
  return totalbytes;
}

int Udp::SendData(char* buf, int len, sockaddr_in* dst) {
  return sendto(fd_, buf, len, 0, (struct sockaddr*)dst, sizeof(sockaddr_in));
}
/**
 * @brief event loop for sending data to network
 *
 */
void Udp::Loop(int id) {
  if (unlikely(OutQueueSize() - 1 < id)) {
    return;
  }
  BaseMsg_ptr msg = nullptr;
  OutQueue(id)->WaitDequeue(msg);
  if (msg) {
    // TODO(peng.lei):
  }
}

/**
 * @brief dispatch msg according to its ID.
 *
 * @param msg
 */
void Udp::Dispatch(BaseMsg_ptr& msg) {
  if (unlikely(in_.empty())) {
    return;
  }
  recv_cnt_++;
  auto index = msg->id() % in_.size();
  InQueue(index)->WaitEnqueue(msg);
}

}  // namespace util
}  // namespace base
