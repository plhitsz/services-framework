/**
 * @file util.h
 * @author Penglei
 * @brief
 * @version 0.1
 * @date 2022-10-26
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef SRC_UTIL_UTIL_H_
#define SRC_UTIL_UTIL_H_

#include <arpa/inet.h>
#include <glog/logging.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace base {
namespace util {

template <typename... Args>
bool exe_shell(const std::string& format, Args&&... args) {
  auto size =
      std::snprintf(nullptr, 0, format.c_str(), std::forward<Args>(args)...);
  std::string output(size + 1, '\0');
  std::snprintf(&output[0], format.c_str(), std::forward<Args>(args)...);
  pid_t status = std::system(output.c_str());
  LOG(INFO) << output;
  if (status != -1 && (WIFEXITED(status) && (0 == WEXITSTATUS(status)))) {
    return true;
  }
  return false;
}

inline std::string exe_shell_ret(const std::string& cmd) {
  FILE* fp = popen(cmd.c_str(), "r");
  if (!fp) {
    return "";
  }
  std::string rst;
  while (!feof(fp)) {
    char buf[128];
    size_t bytes = fread(buf, 1, 128, fp);
    buf[bytes - 1] = '\0';
    rst += buf;
  }
  pclose(fp);
  return rst;
}

// to verify T and U has the same type
template <typename T, typename U>
struct decay_equiv : std::is_same<typename std::decay<T>::type, U>::type {};

// Remove trailing and leading spaces
template <class Str>
Str trim(const Str& s, const std::locale& loc = std::locale()) {
  typename Str::const_iterator first = s.begin();
  typename Str::const_iterator end = s.end();
  while (first != end && std::isspace(*first, loc)) ++first;
  if (first == end) return Str();
  typename Str::const_iterator last = end;
  do {
    --last;
  } while (std::isspace(*last, loc));
  if (first != s.begin() || last + 1 != end)
    return Str(first, last + 1);
  else
    return s;
}

inline uint32_t IpStringToInt(const std::string& ip) {
  struct in_addr tmp_addr;
  inet_aton(ip.c_str(), &tmp_addr);
  return (uint32_t)tmp_addr.s_addr;
}

inline std::string IntToIpString(uint32_t value) {
  struct in_addr tmp_addr;
  tmp_addr.s_addr = value;
  return std::string(inet_ntoa(tmp_addr));
}

inline int Stoi(const std::string& value) {
  std::string::size_type sz;
  return std::stoi(value, &sz);
}

inline std::string Hexdump(const char* data, int len) {
  std::string res = "\n";
  std::string enter = "\n";
  for (int i = 0; i < len; i++) {
    char buf[16];
    int ret = snprintf(buf, sizeof(buf), "%02x ", (uint8_t)data[i]);
    res.append(buf, ret);
    if (i != 0 && ((i + 1) % 16 == 0)) {
      res.append(enter);
    }
  }
  res.append(enter);
  return res;
}

}  // namespace util
}  // namespace base

#endif  // SRC_UTIL_UTIL_H_
