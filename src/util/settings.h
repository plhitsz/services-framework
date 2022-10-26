/**
 * @file settings.h
 * @author peng lei (plhitsz@outlook.com)
 * @brief
 * @version 0.1
 * @date 2022-10-26
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef SRC_UTIL_SETTINGS_H_
#define SRC_UTIL_SETTINGS_H_

#include <stdarg.h>

#include <fstream>
#include <map>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace base {
namespace util {

typedef std::map<std::string, std::string> sectionKv;
const char SETTING_FILE_PATH[] = "/etc/cfg/my-settings.ini";

std::vector<std::string> split(const std::string& str,
                               const std::string& pattern);

class Settings {
 public:
  static Settings& getInstance() {
    static Settings* ins = nullptr;
    if (!ins) {
      static std::once_flag flag;
      std::call_once(flag, [&]() { ins = new (std::nothrow) Settings(); });
    }
    return *ins;
  }

  template <typename T>
  T getValue(const std::string& fmt, ...);

  template <typename T>
  T getValue(const std::string& name, T v);

  template <typename T>
  void setValue(const std::string& name, T v);

  void dump();

 private:
  Settings() {}
  ~Settings() {}
  Settings& operator=(Settings const&) = delete;
  void write_ini(std::basic_ostream<char>& stream,
                 const std::map<std::string, std::string>& local);
  void read_ini(std::basic_istream<char>& stream,
                std::map<std::string, std::string>& local);
  template <typename T>
  bool isSupported(T v);
};

// we only support std::string, int and double
template <typename T>
bool Settings::isSupported(T v) {
  return !!(decay_equiv<T, std::string>::value || decay_equiv<T, int>::value ||
            decay_equiv<T, double>::value);
}

inline std::string value(const std::string& value, const std::string& v) {
  if (value.empty()) return v;
  return value;
}

inline int value(const std::string& value, int v) {
  if (value.empty()) return v;
  std::string::size_type sz;
  return std::stoi(value, &sz);
}

inline double value(const std::string& value, double v) {
  if (value.empty()) return v;
  std::string::size_type sz;
  return std::stod(value, &sz);
}

template <typename T>
T Settings::getValue(const std::string& fmt, ...) {
  T v;
  if (!isSupported(v)) {
    throw std::runtime_error("Type not supported");
  }
  std::basic_ifstream<char> stream(SETTING_FILE_PATH,
                                   std::ios_base::out | std::ios_base::app);
  if (!stream) {
    throw std::runtime_error("File open failed");
  }
  sectionKv res;
  stream.imbue(std::locale());
  read_ini(stream, res);

  char buf[1024];
  va_list args;
  va_start(args, fmt);
  int ret = vsnprintf(buf, sizeof buf, fmt.c_str(), args);
  va_end(args);
  std::string name(buf);
  return value(res[name], v);
}

template <typename T>
T Settings::getValue(const std::string& name, T v) {
  if (!isSupported(v)) {
    throw std::runtime_error("Type not supported");
  }
  std::basic_ifstream<char> stream(SETTING_FILE_PATH,
                                   std::ios_base::out | std::ios_base::app);
  if (!stream) {
    throw std::runtime_error("File open failed");
  }
  sectionKv res;
  stream.imbue(std::locale());
  read_ini(stream, res);

  return value(res[name], v);
}

template <typename T>
void Settings::setValue(const std::string& name, T v) {
  if (!isSupported(v)) {
    throw std::runtime_error("Type not supported");
  }
  sectionKv res;
  // read
  {
    std::basic_ifstream<char> stream(SETTING_FILE_PATH,
                                     std::ios_base::out | std::ios_base::app);
    if (!stream) {
      throw std::runtime_error("File open failed");
    }
    stream.imbue(std::locale());
    read_ini(stream, res);
  }
  std::stringstream ss;
  std::string tmpstr;
  ss << v;
  ss >> tmpstr;
  auto it = res.find(name);
  if (it != res.end()) {
    it = res.erase(it);
  }
  res.insert(std::make_pair(name, tmpstr));
  // write
  std::basic_ofstream<char> otream(SETTING_FILE_PATH);
  if (!otream) {
    throw std::runtime_error("File open failed");
  }
  otream.imbue(std::locale());
  write_ini(otream, res);
}

}  // namespace util
}  // namespace base

#endif  // SRC_UTIL_SETTINGS_H_
