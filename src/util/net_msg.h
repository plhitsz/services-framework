/**
 * @file net_msg.h
 * @author peng lei (plhitsz@outlook.com)
 * @brief
 * @version 0.1
 * @date 2022-10-26
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef SRC_UTIL_NET_MSG_H_
#define SRC_UTIL_NET_MSG_H_

#include <glog/logging.h>

#include <memory>
#include <string>

#include "msg.h"
#include "util/util.h"

namespace base {
namespace util {
#define MIN_IPHEADER_LEN 20
class NetworkMsg;
typedef std::shared_ptr<NetworkMsg> NetMsg_ptr;

class NetworkMsg : public BaseMsg {
 public:
  NetworkMsg() {}
  explicit NetworkMsg(int len) : BaseMsg(len) {}
  virtual ~NetworkMsg() {}
  const std::string colon = ":";
  bool IsIPv4() override;
  void decode() override;

  /* network byte order */
  MEM_FUNCTION(DstAddr, uint32_t, dst_ip_)
  MEM_FUNCTION(SrcAddr, uint32_t, src_ip_)
  MEM_FUNCTION(SrcAddrString, std::string, src_ip_str_)
  MEM_FUNCTION(DstAddrString, std::string, dst_ip_str_)
  MEM_FUNCTION(SrcPort, uint16_t, src_port_)
  MEM_FUNCTION(DstPort, uint16_t, dst_port_)
  MEM_FUNCTION(Protocol, uint8_t, protocol_)
  inline std::string GetFLowIndex() { return flowindex_; }
  void reset();

 protected:
  uint32_t src_ip_;
  uint32_t dst_ip_;
  std::string src_ip_str_;
  std::string dst_ip_str_;
  std::string flowindex_;  // flow index (srcip:dstip:src_port:dst_port)
  uint16_t src_port_;
  uint16_t dst_port_;
  uint8_t protocol_; /* TCP or UDP or ICMP */
  DISALLOW_COPY_AND_ASSIGN(NetworkMsg)
};

}  // namespace util
}  // namespace base

#endif  // SRC_UTIL_NET_MSG_H_
