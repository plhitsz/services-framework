/**
 * @file base_channel.h
 * @author peng lei (plhitsz@outlook.com)
 * @brief
 * @version 0.1
 * @date 2022-11-10
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef SRC_EXAMPLE_APP_SRC_CHANNEL_H_
#define SRC_EXAMPLE_APP_SRC_CHANNEL_H_
#include "macros.h"
#include "msg.h"
#include "queue.h"
/**
 * @brief A channel between nodes. It may be a queue or shared memory.
 *
 */
template <typename T>
class BaseChannel {
 public:
  using value_type = T;
  BaseChannel() = default;
  virtual ~BaseChannel() = default;
  virtual void ReadMessage(T& msg) = 0;
  virtual void WriteMessage(const T& msg) = 0;
  virtual std::string Id() = 0;
};

/**
 * @brief Queue based channel.
 *
 * @tparam T
 */
template <typename T>
class QueueBasedChannel : public BaseChannel<T> {
 public:
  explicit QueueBasedChannel(const std::string& name) : queue_(name, 100) {}
  virtual ~QueueBasedChannel() = default;
  inline void ReadMessage(T& msg) override { queue_.WaitDequeue(msg); }
  inline void WriteMessage(const T& msg) override { queue_.WaitEnqueue(msg); }
  virtual std::string Id() { return queue_.Id(); }
  virtual Queue<T>& GetQueue() { return queue_; }

 private:
  Queue<T> queue_;
  DISALLOW_COPY_AND_ASSIGN(QueueBasedChannel)
};

using MsgChannel = QueueBasedChannel<BaseMsg_ptr>;

#endif  // SRC_EXAMPLE_APP_SRC_CHANNEL_H_
