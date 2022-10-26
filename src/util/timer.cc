/**
 * @file timer.cc
 * @author Peng Lei
 * @brief
 * @version 0.1
 * @date 2022-10-26
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "timer.h"

#include <chrono>

void TimerManager::Stop() {
  stop_ = true;
  timeout_->BreakAllWait();
  // worker exit
  for (auto& th : thread_pool_) {
    if (th.joinable()) {
      th.join();
    }
  }
}
/**
 * @brief Start the timer service as a single thread.
 *
 */
void TimerManager::StartAsThread() {
  InitThreadPool();
  thread_pool_.emplace_back([this]() {
    while (!stop_) {
      std::this_thread::sleep_for(std::chrono::milliseconds(resolution_));
      std::unique_lock<std::mutex> lg(mutex_, std::defer_lock);
      lg.lock();
      tick_ += resolution_;
      lg.unlock();

      TimersTick();
    }
  });
}

void TimerManager::Start() {
  InitThreadPool();
  while (!stop_) {
    std::this_thread::sleep_for(std::chrono::milliseconds(resolution_));
    std::unique_lock<std::mutex> lg(mutex_, std::defer_lock);
    lg.lock();
    tick_ += resolution_;
    lg.unlock();

    TimersTick();
  }
}
/**
 * @brief The thread pool to handle the callback of registered timeout event.
 *
 */
void TimerManager::InitThreadPool() {
  int worker_num = 1;
  thread_pool_.reserve(worker_num);
  timeout_ = std::make_shared<TimerQueue>("timerq", 10);
  for (int i = 0; i < worker_num; i++) {
    thread_pool_.emplace_back([this]() {
      while (!stop_) {
        Timer_ptr timer = nullptr;
        timeout_->WaitDequeue(timer);
        if (timer) {
          timer->handler(timer);
        }
      }
    });
  }
}
void TimerManager::TimersTick() {
  std::unique_lock<std::mutex> lg(mutex_);
  for (auto it = timers_.begin(); it != timers_.end();) {
    auto& t = *it;
    std::unique_lock<std::mutex> lg(t->mutex);
    if (!t->cancelled && t->expires + t->start == tick_) {
      if (t->oneshot) {
        t->cancelled = true;
        t->refcnt--;
      }

      t->start = tick_;
      // this may block if too many timers.
      timeout_->WaitEnqueue(t);
    }

    if (t->cancelled && t->refcnt <= 0) {
      it = timers_.erase(it);
    } else {
      ++it;
    }
  }
}
/**
 * @brief when timer is timeout, call `hanlder`.
 *
 */
void TimerManager::AddTimer(Timer_ptr timer) {
  if (timer->expires % resolution_ != 0) {
    throw std::runtime_error("Invalid expire time settting.");
  }

  if (timer->expires <= 0) {
    return;
  }

  std::unique_lock<std::mutex> lg(mutex_);
  timer->start = tick_;
  timers_.push_back(timer);
}

/**
 * @brief The timer which will be triggered by once.
 *
 */
void TimerManager::AddOneshotTimer(Timer_ptr timer) {
  timer->oneshot = true;
  AddTimer(timer);
}
/**
 * @brief Cancel the timer.
 *
 * @param t
 */
void TimerManager::CancelTimer(Timer_ptr t) {
  std::unique_lock<std::mutex> lg(t->mutex);
  t->refcnt--;
  t->cancelled = true;
}
