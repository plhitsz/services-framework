#include "timer.h"

#include <gflags/gflags.h>
#include <gtest/gtest.h>
void callback(Timer_ptr t) { LOG(INFO) << "callback"; }
TEST(timer_test, timeout_test) {
  auto t_mgr = std::make_shared<TimerManager>();
  auto timer = std::make_shared<Timer>(15, callback);
  t_mgr->StartAsThread();
  t_mgr->AddOneshotTimer(timer);
}