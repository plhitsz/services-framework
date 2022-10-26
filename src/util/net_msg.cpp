#include "net_msg.h"

#include <glog/logging.h>
#include <netinet/ip_icmp.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

#include <memory>
#include <string>

namespace base {
namespace util {
/*
 * parse msg body to get src_ip info
 * */
void NetworkMsg::decode() {
  if (unlikely(this->size() < MIN_IPHEADER_LEN)) {
    LOG(WARNING) << "invalid netmsg size " << this->size();
    return;
  }
  char* buf = (char*)this->begin();
  struct ip* ip_header = (struct ip*)buf;
  if (unlikely(0x4 != ip_header->ip_v)) {
    return;
  }

  if (ip_header->ip_p == IPPROTO_UDP) {
    dst_port_ = ntohs(((struct udphdr*)(&(buf[ip_header->ip_hl * 4])))->dest);
    src_port_ = ntohs(((struct udphdr*)(&(buf[ip_header->ip_hl * 4])))->source);
  } else if (ip_header->ip_p == IPPROTO_TCP) {
    dst_port_ = ntohs(((struct tcphdr*)(&(buf[ip_header->ip_hl * 4])))->dest);
    src_port_ = ntohs(((struct tcphdr*)(&(buf[ip_header->ip_hl * 4])))->source);
  } else {
    // ICMP
    dst_port_ = 0;
    src_port_ = 0;
  }
  protocol_ = (uint8_t)ip_header->ip_p;
  dst_ip_ = ip_header->ip_dst.s_addr;
  src_ip_ = ip_header->ip_src.s_addr;
  src_ip_str_ = std::string(inet_ntoa(ip_header->ip_src));
  dst_ip_str_ = std::string(inet_ntoa(ip_header->ip_dst));
  flowindex_ = src_ip_str_ + colon + dst_ip_str_ + colon +
               std::to_string(src_port_) + colon + std::to_string(dst_port_);
}

void NetworkMsg::reset() {
  src_ip_ = 0;
  dst_ip_ = 0;
  src_ip_str_ = "";
  dst_ip_str_ = "";
  src_port_ = 0;
  dst_port_ = 0;
  protocol_ = 0x00;
}

bool NetworkMsg::IsIPv4() {
  if (unlikely(this->size() < MIN_IPHEADER_LEN)) {
    return false;
  }
  return (0x4 == ((struct ip*)(this->begin()))->ip_v);
}

}  // namespace util
}  // namespace base
