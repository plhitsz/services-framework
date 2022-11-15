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

#include "protocol.h"
#include "util.h"
#include "util/bats_msg.h"
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
  is_stop_ = false;
  return true;
}

int Udp::FDRecv() {
  auto msg = std::make_shared<bats::util::BatsMsg>();
  int ret = recvfrom(fd_, (char*)msg->begin(), msg->size(), 0, NULL, NULL);
  if (ret > 0) {
    msg->resize(ret);
    msg->decode();
    Dispatch(msg);
  }
  return ret;
}

int Udp::FDWrite(const msg_type& msg) {
  // =========== init protocol header =============
  struct BatsHeader* proto_hdr = (struct BatsHeader*)msg->begin();
  if (msg->NeedCoded()) {
    proto_hdr->flow_id = htonll(msg->encodeInfo().flow_id);
    proto_hdr->file_id = htonl(msg->id());
    proto_hdr->batch_id = htonl(msg->seq());
    proto_hdr->pac_num = htons(msg->encodeInfo().pac_num);
    proto_hdr->pac_type = 1;
  } else {
    // setting `file_id` and `pac_type`
    proto_hdr->file_id = htonl(msg->id());
    proto_hdr->pac_type = 0;  // raw packet.
    proto_hdr->flow_id = htonll(msg->encodeInfo().flow_id);
  }
  return sendto(fd_, (char*)msg->begin(), msg->size(), 0,
                (struct sockaddr*)&msg->encodeInfo().dst_addr,
                sizeof(sockaddr_in));
}

}  // namespace src
}  // namespace bats
