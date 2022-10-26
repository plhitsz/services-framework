/**
 * @file file.h
 * @author Peng Lei
 * @brief File reading handler
 * @copyright Copyright (c) 2022
 */
#ifndef SRC_UTIL_FILE_H_
#define SRC_UTIL_FILE_H_
#include <fstream>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>

class FileHander  // nocopyable
{
 public:
  FileHander() = default;
  ~FileHander() = default;
  FileHander(const std::string& filename, std::ios_base::openmode mode);
  int readToString(int maxSize, std::string& out);
  int readToOctetVec(int maxSize, octetVec& out);
  int read(int maxSize, octet* out);
  void writeFromString(const std::string& out);
  void writeFromOctetVec(const octetVec& out);

  // delete
  FileHander(const FileHander&) = delete;
  void operator=(const FileHander&) = delete;

 private:
  std::fstream stream_;
  std::string filename_;
  int filesize_;
};

using FileHander_ptr = std::shared_ptr<FileHander>;

#endif  // SRC_UTIL_FILE_H_
