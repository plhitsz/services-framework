/**
 * @file macros.h
 * @author Lei Peng
 * @brief highly used macros defination
 * @copyright Copyright (c) 2022
 */
#ifndef SRC_INCLUDE_MACROS_H_
#define SRC_INCLUDE_MACROS_H_

#include <sys/time.h>
#include <time.h>

#if __GNUC__ >= 3
#define likely(x) (__builtin_expect((x), 1))
#define unlikely(x) (__builtin_expect((x), 0))
#else
#define likely(x) (x)
#define unlikely(x) (x)
#endif

#define CACHELINE_SIZE (64)

#define UNUSED(param) (void)param

#define DISALLOW_COPY_AND_ASSIGN(Class) \
  Class(const Class&) = delete;         \
  Class& operator=(const Class&) = delete;

#define MEM_FUNCTION(NAME, RETTYPE, VALUE)      \
  const RETTYPE& NAME() const { return VALUE; } \
  RETTYPE& NAME() { return VALUE; }

inline uint64_t tv2ms(struct timeval tv) {
  uint64_t ms = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
  return ms ? ms : 1;
}

inline uint64_t gettimestamp_ms() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv2ms(tv);
}

#endif  // SRC_INCLUDE_MACROS_H_
