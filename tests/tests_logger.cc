#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

TEST(TestLogger, Basic)
{
  spdlog::info("Welcome to spdlog!");
  spdlog::error("Some error message with arg: {}", 1);
}
