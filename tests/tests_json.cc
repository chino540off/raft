#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

// for convenience
using json = nlohmann::json;

TEST(TestJson, Basic)
{
  // parse explicitly
  auto j = json::parse("{ \"happy\": true, \"pi\": 3.141 }");

  j.at("happy");
  std::cout << j.dump(4) << std::endl;
}
