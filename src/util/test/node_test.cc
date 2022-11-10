#include <gflags/gflags.h>
#include <gtest/gtest.h>

#include "src/example/app/src/node_manager.h"
#include "src/example/app/src/node_tun.h"
#include "src/example/app/src/node_udp.h"

TEST(node_test, node_connection) {
  bats::src::Udp u(8888);
  bats::src::Tun t;
  NodeManager::Instance()->Connect(u, t);
  EXPECT_TRUE(u.isOk());
  EXPECT_TRUE(t.isOk());

  NodeManager::Instance()->Shutdown();
}

TEST(node_test, udp_test) {
  bats::src::Udp u(8888);
  NodeManager::Instance()->RunAsThreads(u, 1);
  NodeManager::Instance()->RegisterToPoller(u);

  EXPECT_TRUE(u.isOk());
}