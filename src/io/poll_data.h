#ifndef BATS_IO_POLL_DATA_H_
#define BATS_IO_POLL_DATA_H_

#include <sys/epoll.h>

#include <cstdint>
#include <functional>
namespace bats {
namespace io {

struct PollResponse {
  explicit PollResponse(uint32_t e = 0) : events(e) {}
  uint32_t events;
};

struct PollRequest {
  int fd = -1;
  uint32_t events = 0;
  int timeout_ms = -1;
  std::function<void(const PollResponse&)> callback = nullptr;
};

struct PollCtrlParam {
  int operation;
  int fd;
  epoll_event event;
};

}  // namespace io
}  // namespace bats

#endif  // BATS_IO_POLL_DATA_H_
