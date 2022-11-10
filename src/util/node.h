/**
 * @file base_node.h
 * @author peng lei (plhitsz@outlook.com)
 * @brief
 * @version 0.1
 * @date 2022-11-10
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef SRC_EXAMPLE_APP_SRC_NODE_H_
#define SRC_EXAMPLE_APP_SRC_NODE_H_
#include <glog/logging.h>

#include <functional>
#include <mutex>
#include <string>
#include <vector>

#include "macros.h"

enum class NodeType {
  NODE_SOURCE,  // node without up channels.
  NODE_SINK,    // node without down channels.
  NODE_RELAY,   // node process data from up channels.
  NODE_UDP,     // UDP node
  NODE_TUN,     // TUN node
};
/**
 * @brief Read message from related channels and process.
 * The node itself may be a thread or a process or multi-threads.
 *
 */
template <typename CHN, NodeType type = NodeType::NODE_RELAY>
class Node {
 public:
  // the message type of specified channels.
  typedef typename CHN::value_type msg_type;
  Node() = default;
  virtual ~Node() = default;
  const NodeType Type() const;
  const std::string& GetName() const;
  /**
   * @brief recv a stop signal.
   *
   * @param msg
   * @return true
   * @return false
   */
  bool StopSignal(const msg_type& msg);
  /**
   * @brief Add a new channel to node.
   *
   * @param channel
   * @param is_down True: the added channel is a down channel.
   */
  void AddChannel(CHN& channel, bool is_down);
  CHN& GetChannel(int i, bool is_down);
  int GetChannelNum(bool is_down);
  /**
   * @brief Increase the number of threads which will work on `DoWork` func.
   *
   * @return int
   */
  int IncThreads();
  /**
   * @brief Process the data from up-channel and relay data to down-channel.
   * This function is the entry of threads.
   */
  void DoWork();
  /**
   * @brief Write msg to fd. TUN and UDP node will handle writting on `fd`.
   *
   * @param channel
   */
  void HandleWritting(CHN& channel);
  /**
   * @brief Realy msg to other nodes.
   *
   * @param channel
   */
  void HandlerRelaying(CHN& channel);
  /**
   * @brief Dispatch msg to down nodes.
   *
   * @param msg
   */
  virtual void Dispatch(const msg_type& msg);
  // Do receiving from fd
  virtual int SourceRecv() = 0;
  // Do wrtting to fd
  virtual int SinkWrite(const msg_type& msg) = 0;
  // process the msg.
  virtual auto HandleMsg(const msg_type& msg) -> msg_type = 0;

  virtual int GetFd() = 0;

 private:
  void ThreadAffinity();

 protected:
  std::mutex mutex_;
  bool is_stop_ = false;
  int nid_ = 0;
  int tid_ = 0;
  int worker_cnt_ = 0;
  std::string name_;
  // sink only have up channels
  std::vector<std::reference_wrapper<CHN>> up_channels_;
  // source only have down channels.
  std::vector<std::reference_wrapper<CHN>> down_channels_;
  DISALLOW_COPY_AND_ASSIGN(Node)
};

// ******************************* basic implement ********************** //
template <typename CHN, NodeType type>
const std::string& Node<CHN, type>::GetName() const {
  return name_;
}

template <typename CHN, NodeType type>
bool Node<CHN, type>::StopSignal(const msg_type& msg) {
  if ((msg->type() == TYPE_SIGNAL) && (msg->signal() == SIGNAL_STOP)) {
    is_stop_ = true;
    return true;
  }
  return false;
}

template <typename CHN, NodeType type>
void Node<CHN, type>::AddChannel(CHN& channel, bool is_down) {
  auto& channels = is_down ? down_channels_ : up_channels_;
  channels.push_back(channel);
}

template <typename CHN, NodeType type>
CHN& Node<CHN, type>::GetChannel(int i, bool is_down) {
  auto& channels = is_down ? down_channels_ : up_channels_;
  return channels.at(i).get();
}

template <typename CHN, NodeType type>
int Node<CHN, type>::GetChannelNum(bool is_down) {
  auto& channels = is_down ? down_channels_ : up_channels_;
  return channels.size();
}

template <typename CHN, NodeType type>
void Node<CHN, type>::Dispatch(const msg_type& msg) {
  auto id = msg->id() % down_channels_.size();
  down_channels_.at(id).get().WriteMessage(msg);
}

template <typename CHN, NodeType type>
void Node<CHN, type>::HandleWritting(CHN& channel) {
  msg_type msg;
  // receive
  channel.ReadMessage(msg);
  // process
  auto res = HandleMsg(msg);
  // write
  SinkWrite(res);
}

template <typename CHN, NodeType type>
void Node<CHN, type>::HandlerRelaying(CHN& channel) {
  msg_type msg;
  // receive
  channel.ReadMessage(msg);
  // process
  auto res = HandleMsg(msg);
  // relay
  Dispatch(res);
}

template <typename CHN, NodeType type>
void Node<CHN, type>::DoWork() {
  ThreadAffinity();
  auto chn_index = IncThreads();
  auto& channel = up_channels_.at(chn_index);
  std::function<void(CHN&)> handler;
  switch (type) {
    case NodeType::NODE_UDP:
    case NodeType::NODE_TUN:
      handler = std::bind(&Node::HandleWritting, this, std::placeholders::_1);
      break;
    case NodeType::NODE_RELAY:
      handler = std::bind(&Node::HandlerRelaying, this, std::placeholders::_1);
      break;
    default:
      break;
  }
  while (!is_stop_) {
    handler(channel);
  }
}

template <typename CHN, NodeType type>
int Node<CHN, type>::IncThreads() {
  int selected = 0;
  std::unique_lock<std::mutex> lg(mutex_);
  selected = worker_cnt_ % up_channels_.size();
  worker_cnt_++;
  return selected;
}

template <typename CHN, NodeType type>
const NodeType Node<CHN, type>::Type() const {
  return type;
}

template <typename CHN, NodeType type>
void Node<CHN, type>::ThreadAffinity() {
  static int processor_id = 0;
  int g_max_processor_id = sysconf(_SC_NPROCESSORS_CONF);
  std::unique_lock<std::mutex> lg(mutex_);
  processor_id++;
  cpu_set_t cpuset;
  pthread_t thread = pthread_self();

  /* Set affinity mask to include CPUs 0 to g_max_processor_id - 1*/
  CPU_ZERO(&cpuset);
  CPU_SET(processor_id % g_max_processor_id, &cpuset);
  auto s = pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
  if (s != 0) {
    LOG(WARNING) << "failed to set affinity on "
                 << processor_id % g_max_processor_id;
  } else {
    LOG(INFO) << GetName() << " bind thread " << std::this_thread::get_id()
              << " to " << processor_id % g_max_processor_id;
  }
}

#endif  // SRC_EXAMPLE_APP_SRC_NODE_H_
