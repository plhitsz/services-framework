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
// The following guarantees declaration of the byte swap functions
#ifdef _MSC_VER
#include <stdlib.h>  // NOLINT(build/include)
#elif defined(__FreeBSD__)
#include <sys/endian.h>
#elif defined(__GLIBC__)
#include <byteswap.h>  // IWYU pragma: export
#endif
#include <endian.h>

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

// Use compiler byte-swapping intrinsics if they are available.  32-bit
// and 64-bit versions are available in Clang and GCC as of GCC 4.3.0.
// The 16-bit version is available in Clang and GCC only as of GCC 4.8.0.
// For simplicity, we enable them all only for GCC 4.8.0 or later.
#if defined(__clang__) || \
    (defined(__GNUC__) && \
     ((__GNUC__ == 4 && __GNUC_MINOR__ >= 8) || __GNUC__ >= 5))
inline uint64_t gbswap_64(uint64_t host_int) {
  return __builtin_bswap64(host_int);
}
#elif defined(_MSC_VER)
inline uint64_t gbswap_64(uint64_t host_int) {
  return _byteswap_uint64(host_int);
}
#else
inline uint64_t gbswap_64(uint64_t host_int) {
#if defined(__GNUC__) && defined(__x86_64__) && !defined(__APPLE__)
  // Adapted from /usr/include/byteswap.h.  Not available on Mac.
  if (__builtin_constant_p(host_int)) {
    return __bswap_constant_64(host_int);
  } else {
    uint64_t result;
    __asm__("bswap %0" : "=r"(result) : "0"(host_int));
    return result;
  }
#elif defined(__GLIBC__)
  return bswap_64(host_int);
#else
  return (((host_int & uint64_t{0xFF}) << 56) |
          ((host_int & uint64_t{0xFF00}) << 40) |
          ((host_int & uint64_t{0xFF0000}) << 24) |
          ((host_int & uint64_t{0xFF000000}) << 8) |
          ((host_int & uint64_t{0xFF00000000}) >> 8) |
          ((host_int & uint64_t{0xFF0000000000}) >> 24) |
          ((host_int & uint64_t{0xFF000000000000}) >> 40) |
          ((host_int & uint64_t{0xFF00000000000000}) >> 56));
#endif  // bswap_64
}
#endif  // intrinsics available

#if __BYTE_ORDER == __LITTLE_ENDIAN
inline uint64_t htonll(uint64_t x) { return gbswap_64(x); }
#elif __BYTE_ORDER == __BIG_ENDIAN
inline uint64_t htonll(uint64_t x) { return x; }
#else
#error \
    "Unsupported byte order: Either __LITTLE_ENDIAN or " \
           "__BIG_ENDIAN must be defined"
#endif  // byte order

inline uint64_t ntohll(uint64_t x) { return htonll(x); }

#endif  // SRC_INCLUDE_MACROS_H_
