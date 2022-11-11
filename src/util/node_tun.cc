/**
 * @file node_tun.cc
 * @author peng lei (plhitsz@outlook.com)
 * @brief
 * @version 0.1
 * @date 2022-11-10
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "node_tun.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <glog/logging.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <netinet/ip.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace bats {
namespace src {

const char TUN_DEVICE[] = "/dev/net/tun";

bool Tun::Init() {
  if ((fd_ = open(TUN_DEVICE, O_RDWR)) < 0) {
    LOG(INFO) << "Opening " << TUN_DEVICE << " failed";
    return false;
  }

  struct ifreq ifr;
  memset(&ifr, 0, sizeof(ifr));
  ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
  strncpy(ifr.ifr_name, name_.c_str(), IFNAMSIZ);

  if (ioctl(fd_, TUNSETIFF, (void*)&ifr) < 0) {
    LOG(INFO) << "ioctl " << name_ << " failed";
    close(fd_);
    return false;
  }
  is_stop_ = false;
  return true;
}

int Tun::FDRecv() {
  typedef typename msg_type::element_type msg_org_type;
  msg_type msg = std::make_shared<msg_org_type>();
  int ret = read(fd_, (char*)msg->begin(), msg->size());
  // FIXME: resize ?
  if (ret >= 0) {
    Dispatch(msg);
  }
  return ret;
}

int Tun::FDWrite(const msg_type& msg) {
  if (!msg) {
    return -1;
  }
  char* data = (char*)msg->begin();
  char* frame_start = data;
  while (true) {
    struct ip* iph = (struct ip*)frame_start;
    if (frame_start[0] == 0x00) {
      // is pending data
      break;
    }
    if (unlikely(0x4 != iph->ip_v && 0x6 != iph->ip_v)) {
      LOG(WARNING) << "file " << msg->id() << " is corrupt ! ip packet("
                   << msg->size() << ") not recognized ";
      break;
    }
    size_t frame_size = ntohs(iph->ip_len);
    // check fragments are set correctly
    if (frame_start + frame_size - data > msg->size() ||
        *(char*)frame_start == 0) {
      LOG(INFO) << ("frames have larger size than the file. "
                    "frame size ")
                << frame_size;
      break;
    }
    int ret = write(fd_, frame_start, frame_size);
    if (ret < 0) {
      LOG(INFO) << ("write errors");
    }
    frame_start += frame_size;
    int remain_bytes = msg->size() - (frame_start - data);
    if (remain_bytes <= 20) {
      break;
    }
  }
  return msg->size();
}

auto Tun::HandleMsg(const msg_type& msg) -> msg_type { return msg; }

}  // namespace src
}  // namespace bats
