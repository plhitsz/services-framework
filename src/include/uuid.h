/**
 * @file uuid.h
 * @author  plhitsz@outlook.com
 * @brief
 * @version 0.1
 * @date 2022-10-25
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef SRC_INCLUDE_UUID_H_
#define SRC_INCLUDE_UUID_H_

#include <climits>
#include <iostream>
#include <random>
#include <sstream>
#include <string>

inline unsigned char RandomChar() {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(0, 255);
  return static_cast<unsigned char>(dis(gen));
}
/**
 * @brief Simple way to generate a global unique ID
 *
 * @param len
 * @return std::string
 */
inline std::string GenerateUuid(const unsigned int len = 16) {
  std::stringstream ss;
  for (auto i = 0; i < len; i++) {
    auto rc = RandomChar();
    std::stringstream hexstream;
    hexstream << std::hex << int(rc);
    auto hex = hexstream.str();
    ss << (hex.length() < 2 ? '0' + hex : hex);
  }
  return ss.str();
}

#endif  // SRC_INCLUDE_UUID_H_
