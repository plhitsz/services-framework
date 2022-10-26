/**
 * @file logginghelper.h
 * @author peng lei (plhitsz@outlook.com)
 * @brief
 * @version 0.1
 * @date 2022-10-26
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef SRC_INCLUDE_LOGGINGHELPER_H_
#define SRC_INCLUDE_LOGGINGHELPER_H_

#include <glog/logging.h>
namespace base {
namespace util {

class LoggingHelper {
 public:
  explicit LoggingHelper(char** argv) {
    google::InitGoogleLogging(argv[0]);
    FLAGS_colorlogtostderr = true;
    FLAGS_minloglevel = 0;
    FLAGS_logbufsecs = 0;
    google::SetStderrLogging(google::GLOG_INFO);
    google::InstallFailureSignalHandler();
  }
  ~LoggingHelper() { google::ShutdownGoogleLogging(); }
};

}  // namespace util
}  // namespace base

#endif  // SRC_INCLUDE_LOGGINGHELPER_H_
