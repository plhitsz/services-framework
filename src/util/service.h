/**
 * @file service.h
 * @author peng lei (plhitsz@outlook.com)
 * @brief
 * @version 0.1
 * @date 2022-10-26
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef SRC_UTIL_SERVICE_H_
#define SRC_UTIL_SERVICE_H_

#include <stdint.h>

#include <memory>
#include <string>
#include <vector>

#include "msg.h"
#include "queue.h"
#include "util/util.h"

namespace base {
namespace util {

using MsgQueue = Queue<BaseMsg_ptr>;
using MsgQueue_ptr = std::shared_ptr<MsgQueue>;

enum class QueueType {
  QUEUE_IN,
  QUEUE_OUT,
};

/**
 * @brief Service type and its data flow bet `inqueue` and `outqueue`.
 *
 */
enum ServiceType {
  SERVICE_TUN,     // TUN socket service, recv from TUN by poller to
                   // `inqueue`, write the data of `outqueue` to TUN.
  SERVICE_UDP,     // UDP device service, recv from udp socket by poller to
                   // `inqueue`, send the data of `outqueue` to network.
  SERVICE_SOURCE,  // recv data from a given source, then put it into `outqueue`
  SERVICE_SINK,    // recv data from `inqueue`, then drop it or write it to a
                   // given sink.
  SERVICE_RELAY,   // recv data from `inqueue`, then put it into `outqueue`
};

/**
 * @brief an abstract service.
 *
 */
class Service {
 public:
  explicit Service(const std::string& name, ServiceType type)
      : name_(name), type_(type) {}
  virtual ~Service() {
    if (fd_ > 0) {
      close(fd_);
    }
  }
  // TODO(peng.lei): impl
  inline void AddQueue(MsgQueue_ptr q, QueueType type) {}
  inline int QueueSize(int idx, QueueType type) {}
  inline MsgQueue_ptr GetQueue(int idx, QueueType type) {}
  inline int GetFd() const { return fd_; }
  inline std::string GetName() const { return name_; }
  inline ServiceType GetType() const { return type_; }
  void ClearQueues() {
    for (int i = 0; i < in_.size(); i++) {
      in_[i] = nullptr;
    }
    for (int i = 0; i < out_.size(); i++) {
      out_[i] = nullptr;
    }
  }
  virtual int RecvData(char* buf, int len) = 0;
  virtual int SendData(char* buf, int len) = 0;
  /**
   * @brief looping on `id`-th queue (in or out)
   *
   * @param id
   */
  virtual void Loop(int id = 0) = 0;
  virtual bool Create() = 0;
  /**
   * @brief Define the dispatch method of relay `msg` to `out_` queues.
   *
   * @param msg
   */
  virtual void Dispatch(BaseMsg_ptr& msg) = 0;
  uint64_t GetRecvCnt() const { return recv_cnt_; }
  uint64_t GetSendCnt() const { return send_cnt_; }
  /**
   * @brief Increase or Decrease the number of working threads on this
   * service.
   *
   */
  void IncreaseWorkers() {
    std::unique_lock<std::mutex> lg(mutex_);
    thread_cnt_++;
  }
  void DecreaseWorkers() {
    std::unique_lock<std::mutex> lg(mutex_);
    thread_cnt_--;
  }
  bool StopState() const { return stop_; }
  bool& StopState() { return stop_; }
  bool isStopSignal(BaseMsg_ptr& msg) {
    if ((msg->type() == TYPE_SIGNAL) && (msg->signal() == SIGNAL_STOP)) {
      stop_ = true;
      return true;
    }
    return false;
  }

 protected:
  std::mutex mutex_;
  int fd_ = 0;
  // the number of working threads on this service.
  int thread_cnt_ = 0;
  uint64_t recv_cnt_ = 0;
  uint64_t send_cnt_ = 0;
  uint64_t max_cnt_ = 0;
  std::string name_;
  ServiceType type_;
  // enqueue the msg to `in_` when receiving.
  std::vector<MsgQueue_ptr> in_;
  // enqueue the msg to `out_` before sending.
  std::vector<MsgQueue_ptr> out_;
  // running state of current service.
  bool stop_ = false;

 private:
  DISALLOW_COPY_AND_ASSIGN(Service)
};

typedef std::shared_ptr<Service> Service_ptr;

}  // namespace util
}  // namespace base
#endif  // SRC_UTIL_SERVICE_H_
