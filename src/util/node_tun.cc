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

#include "util.h"
#include "util/net_msg.h"

namespace bats {
namespace src {

const char TUN_DEVICE[] = "/dev/net/tun";

bool Tun::Init() {
  SYSLOG(INFO) << "tun init";
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
  // setting address
  int masklen = 24;
  auto mask =
      bats::util::IntToIpString(htonl((uint32_t)NETMASK_VALUE(32 - masklen)));
  bats::util::exe_shell("ifconfig %s %s netmask %s", name_.c_str(),
                        ipaddr_.c_str(), mask.c_str());
  bats::util::exe_shell("echo 0 > /proc/sys/net/ipv4/conf/%s/rp_filter",
                        name_.c_str());
  bats::util::exe_shell("ifconfig %s up", name_.c_str());
  is_stop_ = false;
  return true;
}

int Tun::FDRecv() {
  auto msg = std::make_shared<bats::util::NetMsg>();
  int ret = read(fd_, (char*)msg->begin(), msg->size());
  if (ret < 0) {
    return ret;
  }
  msg->resize(ret);
  msg->decode();
  // handle IPv4 packet
  if (!msg->IsIPv4()) {
    SYSLOG(INFO) << "Tun drop none ipv4 msg";
    return 0;
  }
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

}  // namespace src
}  // namespace bats
