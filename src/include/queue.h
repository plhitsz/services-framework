/**
 * @file queue.h
 * @author peng lei (plhitsz@outlook.com)
 * @brief
 * @version 0.1
 * @date 2022-10-25
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef SRC_INCLUDE_QUEUE_H_
#define SRC_INCLUDE_QUEUE_H_

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <utility>

#include "uuid.h"

using namespace std::chrono_literals;

class WaitStrategy {
 public:
  virtual void NotifyOne() {}
  virtual void BreakAllWait() {}
  virtual bool EmptyWait() = 0;
  virtual ~WaitStrategy() {}
};

/* mutex and condition */
class BlockWaitStrategy : public WaitStrategy {
 public:
  BlockWaitStrategy() {}
  void NotifyOne() override { cv_.notify_one(); }
  bool EmptyWait() override {
    std::unique_lock<std::mutex> lock(mutex_);
    // wait until the condition variable is woken up or after the specified
    // timeout duration
    cv_.wait_for(lock, 30ms);
    return true;
  }
  void BreakAllWait() override { cv_.notify_all(); }

 private:
  std::mutex mutex_;
  std::condition_variable cv_;
};
/**
 * @brief Template queue for smart pointer.
 *
 */
template <typename T,
          typename std::enable_if<is_smart_pointer<T>::value, int>::type = 0>
class Queue {
 public:
  Queue() { uuid_ = GenerateUuid(); }
  /**
   * @brief Construct a queue object.
   *
   * @param name The name of the queue.
   * @param size The capacity of the queue.
   */
  Queue(const std::string& name, int size) : name_(name) {
    uuid_ = GenerateUuid();
    auto res = Init(size, new BlockWaitStrategy());
    if (!res) {
      throw std::runtime_error("Queue init failed, no enough memory left.");
    }
  }
  /**
   * @brief Destroy a queue object.
   *
   */
  ~Queue() {
    if (wait_strategy_) {
      BreakAllWait();
    }
    pool_.clear();
    std::deque<T>().swap(pool_);
  }
  /**
   * @brief Keep trying enqueue an element to the queue.
   *
   * @param element The element to be enqueued to the queue.
   * @return true Return true if enqueue action is done.
   * @return false Return false if enqueue action was timeout.
   */
  bool WaitEnqueue(const T& element) {
    while (!break_all_wait_) {
      if (Enqueue(element)) {
        return true;
      }
      if (wait_strategy_->EmptyWait()) {
        continue;
      }
      // wait timeout
      break;
    }
    return false;
  }
  /**
   * @brief Keep trying dequeue an element from the queue.
   *
   * @param element The element to be dequeued from the queue.
   * @return true Return true if dequeue action is done.
   * @return false Return true if dequeue action is done.
   */
  bool WaitDequeue(T& element) {
    while (!break_all_wait_) {
      if (Dequeue(element)) {
        return true;
      }
      if (wait_strategy_->EmptyWait()) {
        continue;
      }
      // wait timeout
      break;
    }
    return false;
  }
  /**
   * @brief Notify all the threads to break the wait.
   *
   */
  void BreakAllWait() {
    break_all_wait_ = true;
    wait_strategy_->BreakAllWait();
  }
  /**
   * @brief The number of elements in the queue.
   *
   * @return int The number of elements in the queue.
   */
  int Size() { return pool_.size(); }
  /**
   * @brief  Whether the queue is empty.
   *
   * @return true Return true if the queue is empty.
   * @return false Return false if the queue is not empty.
   */
  bool Empty() { return Size() == 0; }
  /**
   * @brief Get the ID of the queue.
   *
   * @return const std::string&
   */
  const std::string& Id() const { return uuid_; }
  const std::string& GetName() const { return name_; }

 private:
  Queue& operator=(const Queue& other) = delete;
  Queue(const Queue& other) = delete;
  void SetWaitStrategy(WaitStrategy* WaitStrategy) {
    wait_strategy_.reset(WaitStrategy);
  }
  bool Init(int size) { return Init(size, new BlockWaitStrategy()); }
  bool Init(int size, WaitStrategy* strategy) {
    pool_size_ = size;
    wait_strategy_.reset(strategy);
    return true;
  }
  /**
   * @brief Enqueue a element withot wait.
   *
   * @param element
   * @return true Return true if enqueue done.
   * @return false Return false if enqueue failed.
   */
  bool Enqueue(const T& element) {
    std::unique_lock<std::mutex> lg(mutex_);
    if (pool_.size() >= pool_size_) {
      return false;
    }
    pool_.push_back(element);
    wait_strategy_->NotifyOne();
    return true;
  }

  bool Dequeue(T& element) {
    std::unique_lock<std::mutex> lg(mutex_);
    if (pool_.empty()) {
      return false;
    }
    element = pool_.front();
    pool_.pop_front();
    wait_strategy_->NotifyOne();
    return true;
  }

 private:
  std::mutex mutex_;
  std::deque<T> pool_;
  int pool_size_ = 0;
  std::string name_;
  std::string uuid_;
  std::unique_ptr<WaitStrategy> wait_strategy_ = nullptr;
  volatile bool break_all_wait_ = false;
};

#endif  // SRC_INCLUDE_QUEUE_H_
