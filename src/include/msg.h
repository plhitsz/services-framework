/**
 * @file msg.h
 * @author peng lei (plhitsz@outlook.com)
 * @brief
 * @version 0.1
 * @date 2022-10-25
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef SRC_INCLUDE_MSG_H_
#define SRC_INCLUDE_MSG_H_

#include <algorithm>
#include <cassert>
#include <memory>
#include <string>
#include <utility>
#include <vector>

enum MSG_TYPE {
  TYPE_DATA,
  TYPE_IPV4_DATA,
  TYPE_IPV6_DATA,
  TYPE_SIGNAL, /* control sig msg */
};

enum MSG_SIGNAL {
  SIGNAL_NONE,
  SIGNAL_STOP,
};

class BaseMsg;
typedef std::shared_ptr<BaseMsg> BaseMsg_ptr;
constexpr static int default_buffer_len = 2048;
/**
 * @brief Message object to store binary data.
 *
 */
class BaseMsg {
 public:
  using iterator = octet*;
  using const_iterator = const octet*;
  /**
   * @brief Construct a Message object.
   *
   * @param len The length of the buffer in the message object
   */
  explicit BaseMsg(int len = default_buffer_len) : type_(TYPE_DATA) {
    resize(len);
    head_ = curr_ = 0;
  }
  /**
   * @brief Construct a Message object from an std::vector<uint8_t> value.
   * Message object will be initialized with the data from `vec`.
   * @param vec The input std::vector<uint8_t> value.
   */
  explicit BaseMsg(std::vector<uint8_t>&& vec) : type_(TYPE_DATA) {
    resize(vec.size());
    data_ = std::move(vec);
    head_ = curr_ = 0;
  }
  virtual ~BaseMsg() {}
  /**
   * @brief Whether the contant of Message object is a IPv4 packet.
   *
   * @return true Return true if the contant of Message object is a IPv4 packet.
   * @return false Return false if the contant of Message object is not a IPv4
   * packet.
   */
  virtual bool IsIPv4() { return true; }
  /**
   * @brief Decode the contant of Message to readable data structure.
   *
   */
  virtual void decode() {}
  /**
   * @brief The length of the stored data in message object.
   *
   * @return int
   */
  int size() const { return data_.size(); }
  /**
   * @brief Resize the length of the stored data in message object.
   *
   * @param sz The new length of the stored data in message object.
   */
  void resize(int sz) { data_.resize(sz); }
  /**
   * @brief Reserve space for the buffer in message object.
   *
   * @param sz The reserve space size.
   */
  void reserve(int sz) { data_.reserve(sz); }
  /**
   * @brief Get the underlying container of the message object.
   *
   * @return std::vector<uint8_t>&
   */
  std::vector<uint8_t>& Data() { return data_; }
  /**
   * @brief Get the underlying container of the message object.
   *
   * @return std::vector<uint8_t>&
   */
  const std::vector<uint8_t>& Data() const { return data_; }
  /**
   * @brief The sequence of the message object.
   *
   * @return uint32_t&
   */
  uint32_t& seq() { return seq_; }
  uint32_t seq() const { return seq_; }
  /**
   * @brief The ID of the message object.
   *
   * @return uint32_t&
   */
  uint32_t& id() { return id_; }
  uint32_t id() const { return id_; }
  /**
   * @brief Get or set the type of the msg object.
   *
   * @return MSG_TYPE&
   */
  MSG_TYPE& type() { return type_; }
  MSG_TYPE type() const { return type_; }
  /**
   * @brief Get or set the signal value of the msg object.
   *
   * @return MSG_SIGNAL&
   */
  MSG_SIGNAL& signal() { return signal_; }
  MSG_SIGNAL signal() const { return signal_; }
  /**
   * @brief iterator method for message object.
   *
   * @return iterator
   */
  iterator begin() { return &*data_.begin(); }
  iterator end() { return &*data_.end(); }
  const_iterator begin() const { return &*data_.begin(); }
  const_iterator end() const { return &*data_.end(); }
  /**
   * @brief Reserve a header room at the beginning of the buffer.
   *
   * @param size The length of header room.
   * @return true Return true if reserve aciton success.
   * @return false Return false if reserve aciton fail.
   */
  bool reserveHeader(int size) {
    if (size > data_.capacity()) {
      // deny Reserve action
      return false;
    }
    curr_ += size;
    tail_ += size;
    return true;
  }
  /**
   * @brief Push the header to the reserved headroom.
   *
   * @param size The length of header.
   * @return octet*
   */
  octet* pushHeader(int size) {
    if (size <= curr_ - head_) {
      head_ += size;
      return begin() + (head_ - size);
    }
    return nullptr;
  }

  /**
   * @brief fill octet bytes into message.
   *
   * @param buf The input octet bytes.
   * @param len Then length of input octet bytes.
   */
  void fill(const octet* buf, int len) {
    if (tail_ + len <= data_.capacity()) {
      std::copy(buf, buf + len, begin() + tail_);
      tail_ += len;
      return;
    }
    assert(false && "fill failed.");
  }
  /**
   * @brief Checking whether `lh` is equal to `rh`.
   *
   * @param lh The left side value at `==`.
   * @param rh The right side value at `==`.
   * @return true Return true if `lh` is equal to `rh`.
   * @return false Return false if `lh` is not equal to `rh`.
   */
  friend bool operator==(const BaseMsg& lh, const BaseMsg& rh) {
    if (lh.size() != rh.size()) {
      std::cout << "diff at size " << lh.size() << "," << rh.size()
                << std::endl;
      return false;
    }
    return (lh.data_ == rh.data_);
  }
  /**
   * @brief Steam the format output of message object to std::ostream.
   *
   * @param os
   * @param r
   * @return std::ostream&
   */
  friend std::ostream& operator<<(std::ostream& os, const BaseMsg& r) {
    os << "[" << r.seq() << "] ";   // seq
    os << "[" << r.size() << "] ";  // len
    std::for_each(r.begin(), r.end(),
                  [&os](const octet n) { os << static_cast<int>(n) << ' '; });
    return os;
  }
  /**
   * @brief Print the format output of the message object to std::cout.
   *
   */
  void print(int max) const {
    int bytes = 1;
    int lines = 0;
    for (int bytes = 1; bytes <= data_.size() && lines < max; bytes++) {
      if (bytes % 16 == 1) {
        std::cout << "[";
        std::cout.width(4);
        std::cout.fill(' ');
        std::cout << lines;
        std::cout << "] ";
      }
      {
        std::ios_base::fmtflags f(std::cout.flags());
        std::cout.width(3);
        std::cout.fill(' ');
        std::cout << std::hex << static_cast<int>(data_[bytes - 1]);
        std::cout.flags(f);
      }
      std::cout << ' ';
      if (bytes != 1 && (bytes % 16 == 0)) {
        lines++;
        std::cout << std::endl;
      }
    }
  }

 protected:
  MSG_TYPE type_ = TYPE_DATA;
  MSG_SIGNAL signal_ = SIGNAL_NONE;
  std::vector<uint8_t> data_;
  uint32_t id_ = 0;
  uint32_t seq_ = 0;
  int head_ = 0;  // the last byte of the last pushed header.
  int curr_ = 0;  // the first byte of the payload
  int tail_ = 0;  // the last byte of the payload
};

#endif  // SRC_INCLUDE_MSG_H_
