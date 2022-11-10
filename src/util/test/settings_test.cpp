#include "settings.h"

#include <arpa/inet.h>
#include <gtest/gtest.h>
#include <netinet/in.h>
#include <sys/socket.h>

namespace base {
namespace util {

TEST(Settings, read_default) {
  Settings& settings = Settings::getInstance();
  auto res = settings.getValue<std::string>("main.string", "test");
  EXPECT_EQ(res, "test");

  auto ires = settings.getValue<int>("main.int", 1);
  EXPECT_EQ(ires, 1);

  auto dres = settings.getValue<double>("main.double", 1.1);
  EXPECT_EQ(dres, 1.1);
}

TEST(Settings, write_and_read) {
  Settings& settings = Settings::getInstance();
  settings.setValue<std::string>("main.string", "test");
  auto res = settings.getValue<std::string>("main.string", "none");
  EXPECT_EQ(res, "test");
  settings.setValue<int>("main.int", 1);
  auto ires = settings.getValue<int>("main.int", 2);
  EXPECT_EQ(ires, 1);

  settings.setValue<double>("main.double", 1.1);
  auto dres = settings.getValue<double>("main.double", 1.2);
  EXPECT_EQ(dres, 1.1);
}

TEST(Settings, invalid_read_operation) {
  Settings& settings = Settings::getInstance();
  try {
    // float is not supported
    auto res = settings.getValue<float>("main.float", 1.11);
  } catch (const std::exception& e) {
    EXPECT_TRUE(true);
    return;
  }
  EXPECT_TRUE(false);
}

TEST(Settings, invalid_write_operation) {
  Settings& settings = Settings::getInstance();
  try {
    // float is not supported
    settings.setValue<float>("main.float", 1.11);
  } catch (const std::exception& e) {
    EXPECT_TRUE(true);
    return;
  }
  EXPECT_TRUE(false);
}

/*
 *[section]
 *key.a=b
 * */
TEST(Settings, write_format) {
  Settings& settings = Settings::getInstance();
  settings.setValue<std::string>("main.a.b", "hi");

  auto a = settings.getValue<std::string>("main.a", "none");
  auto b = settings.getValue<std::string>("main.a.b", "none");
  EXPECT_EQ(a, "none");
  EXPECT_EQ(b, "hi");
}

TEST(Settings, read_format) {
  Settings& settings = Settings::getInstance();
  settings.setValue<std::string>("main.a[1]", "hi");
  auto a = settings.getValue<std::string>("main.a[%d]", 1);
  EXPECT_EQ(a, "hi");
}

TEST(settings, route_item_read) {
  Settings& settings = Settings::getInstance();
  auto count = settings.getValue<int>("config.route_count", 0);
  for (int i = 0; i < count; i++) {
    auto dest = settings.getValue<std::string>("route%d.destination", i);
    auto mask = settings.getValue<std::string>("route%d.mask", i);
    auto nexthop = settings.getValue<std::string>("route%d.nexthop", i);
    auto metric = settings.getValue<std::string>("route%d.metric", i);

    struct in_addr tmp_addr;
    inet_aton(mask.c_str(), &tmp_addr);
    std::cout << ((unsigned int)tmp_addr.s_addr) << std::endl;
  }
}

}  // namespace util
}  // namespace base

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
