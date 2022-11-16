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
#ifndef SRC_EXAMPLE_APP_SRC_NODE_MANAGER_H_
#define SRC_EXAMPLE_APP_SRC_NODE_MANAGER_H_
#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "channel.h"
#include "node.h"
#include "node_duplex.h"
/**
 * @brief a global instance which manage all the nodes.
 * All nodes and the connections between them build a topology network.
 */
class NodeManager {
 public:
  static NodeManager* Instance(bool create_if_needed = true) {
    static NodeManager* instance = nullptr;
    if (!instance && create_if_needed) {
      static std::once_flag flag;
      std::call_once(flag,
                     [&] { instance = new (std::nothrow) NodeManager(); });
    }
    return instance;
  }

  static void CleanUp() {
    auto instance = Instance(false);
    if (instance != nullptr) {
      instance->Shutdown();
      delete instance;
    }
  }
  // release all nodes
  inline void Shutdown();
  /**
   * @brief Build a connection between node `up` and `down` with a channel obj.
   *
   * @tparam CHN
   * @tparam uptype Node type of the first param.
   * @tparam downtype Node type of the second param.
   * @param up
   * @param down
   * @param reuse_chn Whether reuse the exsit channels.
   * @return true
   * @return false
   */
  template <typename CHN, NodeType uptype, NodeType downtype>
  bool Connect(Node<CHN, uptype>& up, Node<CHN, downtype>& down,
               bool reuse_chn = true);
  /**
   * @brief Make the node run as a threads or multithreads.
   *
   * @tparam CHN
   * @tparam type
   * @param node
   * @param num The number of the threads which be requested to work on `node`.
   * @return true
   * @return false
   */
  template <typename CHN, NodeType type>
  bool RunAsThreads(Node<CHN, type>& node, int num = 1);
  /**
   * @brief Verify the topology. (make sure that it has no loop and connectness)
   *
   */
  bool Verify();
  /**
   * @brief Dump the status of each node.
   *
   */
  void View();

 private:
  NodeManager() = default;
  virtual ~NodeManager() = default;
  std::unordered_map<std::string, std::pair<NodeType, void*>> node_list_;
  std::vector<MsgChannelPtr> channel_list_;
  std::vector<std::thread> worker_list_;
  std::atomic_bool stop_ = {false};
  DISALLOW_COPY_AND_ASSIGN(NodeManager)
};

template <typename CHN, NodeType uptype, NodeType downtype>
bool NodeManager::Connect(Node<CHN, uptype>& up, Node<CHN, downtype>& down,
                          bool reuse_chn) {
  std::string flow;
  auto qname = up.GetName() + ":" + down.GetName();
  // verify the connection.
  if (up.Type() == NodeType::NODE_SINK) {
    throw std::runtime_error(
        "A sink service can't be a upstream service! ref: source --> sink");
  }
  if (down.Type() == NodeType::NODE_SOURCE) {
    throw std::runtime_error(
        "A source service can't be a downstream service! ref: source --> "
        "sink");
  }
  // ---up---[node]---down---
  if (uptype == NodeType::NODE_SOURCE || uptype == NodeType::NODE_RELAY) {
    flow = up.GetName() + "[out] ---(";
  } else {
    flow = up.GetName() + "[in] ---(";
  }

  // checking the exsit channels
  if (reuse_chn) {
    if (up.GetChannelNum(ChnType::CHN_OUT) != 0) {
      auto& selected_chn = up.GetChannel(0, ChnType::CHN_OUT);
      down.AddChannel(selected_chn, ChnType::CHN_IN);
      flow += selected_chn->Id();
    } else if (down.GetChannelNum(ChnType::CHN_IN) != 0) {
      auto& selected_chn = down.GetChannel(0, ChnType::CHN_IN);
      up.AddChannel(selected_chn, ChnType::CHN_OUT);
      flow += selected_chn->Id();
    }
  }

  // reuse_chn = false or no channels at all.
  if (!reuse_chn || (up.GetChannelNum(ChnType::CHN_OUT) +
                         down.GetChannelNum(ChnType::CHN_IN) ==
                     0)) {
    typedef typename MsgChannelPtr::element_type MsgChannelType;
    auto selected_chn = std::make_shared<MsgChannelType>(qname);
    up.AddChannel(selected_chn, ChnType::CHN_OUT);
    down.AddChannel(selected_chn, ChnType::CHN_IN);
    flow += selected_chn->Id();
    channel_list_.push_back(selected_chn);
  }

  flow += " ";
  flow += qname;
  if (downtype == NodeType::NODE_FULL_DUPLEX) {
    flow += ")-->[out]";
  } else {
    flow += ")-->[in]";
  }
  flow += down.GetName();
  LOG(INFO) << flow;
  return true;
}

template <typename CHN, NodeType type>
bool NodeManager::RunAsThreads(Node<CHN, type>& node, int num) {
  if (type == NodeType::NODE_FULL_DUPLEX && num > 1) {
    throw std::runtime_error("Duplex node can only bind one thread.");
  }
  auto itr = node_list_.find(node.GetName());
  if (itr == node_list_.end()) {
    node_list_.insert({node.GetName(), {type, (void*)&node}});
  } else {
    throw std::runtime_error("Duplicate node!");
  }

  auto c = node.GetChannelNum(ChnType::CHN_IN) +
           node.GetChannelNum(ChnType::CHN_OUT);
  if (c <= 0) {
    throw std::runtime_error("node has no available channels.");
  }

  for (int i = 0; i < num; i++) {
    worker_list_.emplace_back(std::thread(&Node<CHN, type>::DoWork, &node));
  }
  return true;
}

bool NodeManager::Verify() { return true; }
void NodeManager::View() {
  LOG(INFO) << "==================== Node view (" << node_list_.size()
            << ") ===================";
  for (auto& item : node_list_) {
    void* node = item.second.second;
    MsgRelayNode* rnode = nullptr;
    NodeDuplex* dnode = nullptr;
    switch (item.second.first) {
      case NodeType::NODE_RELAY:
        rnode = static_cast<MsgRelayNode*>(node);
        break;
      case NodeType::NODE_FULL_DUPLEX:
        dnode = static_cast<NodeDuplex*>(node);
        break;
      default:
        break;
    }
    if (rnode) {
      LOG(INFO) << *rnode;
    }
    if (dnode) {
      LOG(INFO) << *dnode;
    }
  }
  LOG(INFO) << "==================== Channel view (" << channel_list_.size()
            << ") ===================";
  for (auto& chn : channel_list_) {
    LOG(INFO) << *chn;
  }
}

inline void NodeManager::Shutdown() {
  if (stop_.exchange(true)) {
    return;
  }
  // reset the flag for services.
  for (auto& item : node_list_) {
    void* node = item.second.second;
    MsgRelayNode* rnode = nullptr;
    NodeDuplex* dnode = nullptr;
    switch (item.second.first) {
      case NodeType::NODE_RELAY:
        rnode = static_cast<MsgRelayNode*>(node);
        break;
      case NodeType::NODE_FULL_DUPLEX:
        dnode = static_cast<NodeDuplex*>(node);
        break;
      default:
        break;
    }
    if (rnode) {
      rnode->Stop();
    }
    if (dnode) {
      dnode->Stop();
    }
  }
  // worker exit
  for (auto& th : worker_list_) {
    if (th.joinable()) {
      th.join();
    }
  }
  std::vector<MsgChannelPtr>().swap(channel_list_);
  std::vector<std::thread>().swap(worker_list_);
  SYSLOG(INFO) << "release all resource!";
}

#endif  // SRC_EXAMPLE_APP_SRC_NODE_MANAGER_H_
