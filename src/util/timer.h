/**
 * @file timer.h
 * @author Peng Lei
 * @brief
 * @version 0.1
 * @date 2022-09-22
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef SRC_UTIL_TIMER_H_
#define SRC_UTIL_TIMER_H_
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "queue.h"

class Timer;

using Timer_ptr = std::shared_ptr<Timer>;
using TimerQueue = Queue<Timer_ptr>;
using TimerQueue_ptr = std::shared_ptr<TimerQueue>;

class Timer {
 public:
  Timer(uint64_t expire, std::function<void(Timer_ptr t)> hldr)
      : expires(expire), handler(hldr) {
    this->refcnt = 1;
    this->cancelled = false;
    this->oneshot = false;
  }
  virtual ~Timer() {}

 public:
  std::mutex mutex;
  int refcnt;
  bool cancelled;
  bool oneshot;
  uint64_t expires;
  uint64_t start;
  std::function<void(Timer_ptr t)> handler;
};

class TimerManager {
 public:
  TimerManager() {}
  ~TimerManager() { Stop(); }

  void Start();
  void StartAsThread();
  void Stop();

  void AddTimer(Timer_ptr timer);
  void AddOneshotTimer(Timer_ptr timer);
  void CancelTimer(Timer_ptr t);
  int Size() { return timers_.size(); }

 private:
  void TimersTick();
  uint64_t GetTimerTick();
  void InitThreadPool();

 private:
  std::mutex mutex_;
  std::list<Timer_ptr> timers_;
  uint64_t tick_ = 0;
  uint64_t resolution_ = 5;
  TimerQueue_ptr timeout_;
  std::vector<std::thread> thread_pool_;
  bool stop_ = false;
  DISALLOW_COPY_AND_ASSIGN(TimerManager)
};

using TimerManager_ptr = std::shared_ptr<TimerManager>;

#endif  // SRC_UTIL_TIMER_H_
