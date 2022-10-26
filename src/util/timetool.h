/**
 * @file timetool.h
 * @author peng lei (plhitsz@outlook.com)
 * @brief
 * @version 0.1
 * @date 2022-10-26
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef SRC_UTIL_TIMETOOL_H_
#define SRC_UTIL_TIMETOOL_H_

#include <limits>
#include <string>

namespace base {
namespace util {

/**
 * @brief builtin time type Time.
 */
class Time {
 public:
  static const Time MAX;
  static const Time MIN;
  Time() = default;
  explicit Time(uint64_t nanoseconds);
  explicit Time(int nanoseconds);
  explicit Time(double seconds);
  Time(uint32_t seconds, uint32_t nanoseconds);
  Time(const Time& other);
  Time& operator=(const Time& other);

  /**
   * @brief get the current time.
   *
   * @return return the current time.
   */
  static Time Now();
  static Time MonoTime();

  /**
   * @brief Sleep Until time.
   *
   * @param time the Time object.
   */
  static void SleepUntil(const Time& time);

  /**
   * @brief convert time to second.
   *
   * @return return a double value unit is second.
   */
  inline double ToSecond() const {
    return static_cast<double>(nanoseconds_) / 1000000000UL;
  }
  /**
   * @brief convert time to microsecond (us).
   *
   * @return return a unit64_t value unit is us.
   */
  inline uint64_t ToMicrosecond() const {
    return static_cast<uint64_t>(nanoseconds_ / 1000.0);
  }

  inline uint64_t ToMillisecond() const {
    return static_cast<uint64_t>(nanoseconds_ / 1000000.0);
  }
  /**
   * @brief convert time to nanosecond.
   *
   * @return return a unit64_t value unit is nanosecond.
   */
  inline uint64_t ToNanosecond() const { return nanoseconds_; }
  /**
   * @brief convert time to a string.
   *
   * @return return a string.
   */
  std::string ToString() const;

 private:
  uint64_t nanoseconds_ = 0;
};

}  // namespace util
}  // namespace base

#endif  // SRC_UTIL_TIMETOOL_H_
