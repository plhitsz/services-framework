#include "timetool.h"

#include <time.h>

#include <chrono>
#include <ctime>
#include <iomanip>
#include <limits>
#include <sstream>
#include <thread>

namespace base {
namespace util {

using std::chrono::high_resolution_clock;
using std::chrono::steady_clock;
using std::chrono::system_clock;

const Time Time::MAX = Time(std::numeric_limits<uint64_t>::max());
const Time Time::MIN = Time(0);

Time::Time(uint64_t nanoseconds) { nanoseconds_ = nanoseconds; }

Time::Time(int nanoseconds) {
  nanoseconds_ = static_cast<uint64_t>(nanoseconds);
}

Time::Time(double seconds) {
  nanoseconds_ = static_cast<uint64_t>(seconds * 1000000000UL);
}

Time::Time(uint32_t seconds, uint32_t nanoseconds) {
  nanoseconds_ = static_cast<uint64_t>(seconds) * 1000000000UL + nanoseconds;
}

Time::Time(const Time& other) { nanoseconds_ = other.nanoseconds_; }

Time& Time::operator=(const Time& other) {
  this->nanoseconds_ = other.nanoseconds_;
  return *this;
}

Time Time::Now() {
  auto now = high_resolution_clock::now();
  auto nano_time_point =
      std::chrono::time_point_cast<std::chrono::nanoseconds>(now);
  auto epoch = nano_time_point.time_since_epoch();
  uint64_t now_nano =
      std::chrono::duration_cast<std::chrono::nanoseconds>(epoch).count();
  return Time(now_nano);
}

Time Time::MonoTime() {
  auto now = steady_clock::now();
  auto nano_time_point =
      std::chrono::time_point_cast<std::chrono::nanoseconds>(now);
  auto epoch = nano_time_point.time_since_epoch();
  uint64_t now_nano =
      std::chrono::duration_cast<std::chrono::nanoseconds>(epoch).count();
  return Time(now_nano);
}

std::string Time::ToString() const {
  auto nano = std::chrono::nanoseconds(nanoseconds_);
  system_clock::time_point tp(nano);
  auto time = system_clock::to_time_t(tp);
  struct tm stm;
  auto ret = localtime_r(&time, &stm);
  if (ret == nullptr) {
    return std::to_string(static_cast<double>(nanoseconds_) / 1000000000.0);
  }

  std::stringstream ss;
  ss << std::put_time(ret, "%F %T");
  ss << "." << std::setw(9) << std::setfill('0') << nanoseconds_ % 1000000000UL;
  return ss.str();
}

void Time::SleepUntil(const Time& time) {
  auto nano = std::chrono::nanoseconds(time.ToNanosecond());
  system_clock::time_point tp(nano);
  std::this_thread::sleep_until(tp);
}

}  // namespace util
}  // namespace base
