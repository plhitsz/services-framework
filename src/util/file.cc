/**
 * @file file.cc
 * @author peng lei (plhitsz@outlook.com)
 * @brief
 * @version 0.1
 * @date 2022-10-26
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "file.h"

#include <cassert>
#include <memory>

int readFile(const std::string& filename, int maxSize, std::string& content) {
  auto file_ptr = std::make_shared<FileHander>(
      filename, std::ios_base::in | std::ios_base::binary);
  return file_ptr->readToString(maxSize, content);
}

void writeFile(const std::string& filename, const std::string& content) {
  auto file_ptr = std::make_shared<FileHander>(
      filename, std::ios_base::out | std::ios_base::binary);
  file_ptr->writeFromString(content);
}

FileHander::FileHander(const std::string& filename,
                       std::ios_base::openmode mode) {
  stream_.open(filename, mode);
  if (!stream_.is_open()) {
    throw std::runtime_error("File open failed " + filename);
  }
  stream_.imbue(std::locale());
  stream_.seekg(0, std::ios_base::end);
  filesize_ = stream_.tellg();
  stream_.seekg(0, std::ios_base::beg);
}

void FileHander::writeFromString(const std::string& out) {
  if (!stream_) {
    return;
  }
  stream_.write((char*)out.data(), out.size());
  filesize_ += out.size();
}

void FileHander::writeFromOctetVec(const octetVec& out) {
  if (!stream_) {
    return;
  }
  stream_.write((char*)out.data(), out.size());
  filesize_ += out.size();
}

int FileHander::readToString(int maxSize, std::string& out) {
  int read_len = maxSize;
  out.resize(read_len);
  stream_.read((char*)out.data(), read_len);
  return stream_.gcount();
}

int FileHander::readToOctetVec(int maxSize, octetVec& out) {
  int read_len = maxSize;
  out.resize(read_len);
  stream_.read((char*)out.data(), read_len);
  int ret = stream_.gcount();
  out.resize(ret);
  out.shrink_to_fit();
  return ret;
}

int FileHander::read(int maxSize, octet* out) {
  stream_.read((char*)out, maxSize);
  return stream_.gcount();
}