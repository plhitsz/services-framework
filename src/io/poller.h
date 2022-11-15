#ifndef BATS_POOLER_HEADER_H_
#define BATS_POOLER_HEADER_H_

#include <fcntl.h>
#include <unistd.h>

#include <atomic>
#include <condition_variable>
#include <list>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

#include "poll_data.h"
namespace bats {
namespace io {

class Poller {
 public:
  using RequestPtr = std::shared_ptr<PollRequest>;
  using RequestMap = std::unordered_map<int, RequestPtr>;
  using CtrlParamMap = std::unordered_map<int, PollCtrlParam>;

  static Poller* Instance() {
    static Poller* ins = nullptr;
    if (!ins) {
      static std::once_flag flag;
      std::call_once(flag, [&]() { ins = new (std::nothrow) Poller(); });
    }
    return ins;
  }

  virtual ~Poller();

  void Shutdown();

  bool Register(const PollRequest& req);
  bool Unregister(const PollRequest& req);

 private:
  Poller();
  Poller(Poller const&) = delete;
  Poller& operator=(Poller const&) = delete;

  bool Init();
  void Clear();
  void Poll(int timeout_ms);
  void ThreadFunc();
  void HandleChanges();
  int GetTimeoutMs();
  void Notify();

  int epoll_fd_ = -1;
  std::thread thread_;
  std::atomic<bool> is_shutdown_ = {true};

  int pipe_fd_[2] = {-1, -1};
  std::mutex pipe_mutex_;

  RequestMap requests_;
  CtrlParamMap ctrl_params_;

  std::mutex poll_mutex_;
  std::condition_variable condition_;

  const int kPollSize = 32;
  const int kPollTimeoutMs = 100;
};

}  // namespace io
}  // namespace bats

#endif  // BATS_IO_POLLER_H_
