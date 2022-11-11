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
#include "poll_data.h"
#include "poller.h"
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
  inline void Shutdown() {}
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
   * @brief For udp or tun node, they are using `FDRecv` to get input data.
   * Other `relay` nodes no neeed to call this function.
   *
   * @param node
   * @return true
   * @return false
   */
  inline bool RegisterToPoller(NodeDuplex& node);

 private:
  NodeManager() = default;
  virtual ~NodeManager() = default;
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

  bool up_use_out = false;
  if (uptype == NodeType::NODE_SOURCE || uptype == NodeType::NODE_RELAY) {
    up_use_out = true;
    flow = up.GetName() + "[out] ---(";
  } else {
    up_use_out = false;
    flow = up.GetName() + "[in] ---(";
  }
  bool down_use_out = false;
  if (downtype == NodeType::NODE_FULL_DUPLEX) {
    down_use_out = true;
  } else {
    down_use_out = false;
  }

  bool connect_done = false;
  // checking the exsit channels
  if (reuse_chn) {
    if (up_use_out && up.GetChannelNum(up_use_out) != 0) {
      auto& selected_chn = up.GetChannel(0, true);
      down.AddChannel(selected_chn, down_use_out);
      flow += selected_chn->Id();
      connect_done = true;
    } else if (!up_use_out && up.GetChannelNum(up_use_out) != 0) {
      auto& selected_chn = up.GetChannel(0, false);
      down.AddChannel(selected_chn, down_use_out);
      flow += selected_chn->Id();
      connect_done = true;
    } else if (down_use_out && down.GetChannelNum(down_use_out) != 0) {
      // select from downstream
      auto& selected_chn = down.GetChannel(0, down_use_out);
      up.AddChannel(selected_chn, up_use_out);
      flow += selected_chn->Id();
      connect_done = true;
    } else if (!down_use_out && down.GetChannelNum(down_use_out) != 0) {
      // select from downstream
      auto& selected_chn = down.GetChannel(0, down_use_out);
      up.AddChannel(selected_chn, up_use_out);
      flow += selected_chn->Id();
      connect_done = true;
    }
  }

  // reuse_chn = false or no channels at all.
  if (!connect_done) {
    typedef typename MsgChannelPtr::element_type MsgChannelType;
    auto selected_chn = std::make_shared<MsgChannelType>(qname);
    down.AddChannel(selected_chn, down_use_out);
    up.AddChannel(selected_chn, up_use_out);
    flow += selected_chn->Id();
  }
  flow += " ";
  flow += qname;
  if (down_use_out) {
    flow += ")-->[out]";
  } else {
    flow += ")-->[in]";
  }
  flow += down.GetName();
  LOG(INFO) << flow;
  return true;
  return false;
}

template <typename CHN, NodeType type>
bool NodeManager::RunAsThreads(Node<CHN, type>& node, int num) {
  auto c = node.GetChannelNum(false) + node.GetChannelNum(true);
  if (c <= 0) {
    throw std::runtime_error("node has no available channels.");
  }

  for (int i = 0; i < num; i++) {
    worker_list_.emplace_back(std::thread(&Node<CHN, type>::DoWork, &node));
  }
  return true;
}

inline bool NodeManager::RegisterToPoller(NodeDuplex& node) {
  bats::io::PollRequest req;
  req.fd = node.GetFd();
  assert(req.fd > 0);
  fcntl(req.fd, F_SETFL, O_NONBLOCK);
  req.events = EPOLLIN | EPOLLET;  // level trigger
  req.timeout_ms = 0;
  // FIXME: refer ? or pointer.
  // req.ser = node;
  req.callback = [](const bats::io::PollResponse& rsp, NodeDuplex& node) {
    auto& response = rsp;
    if (response.events & EPOLLIN) {
      while (true) {
        auto ret = node.FDRecv();
        if (ret < 0 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {
          break;
        }
      }
    }
  };
  return bats::io::Poller::Instance()->Register(req);
}

#endif  // SRC_EXAMPLE_APP_SRC_NODE_MANAGER_H_
