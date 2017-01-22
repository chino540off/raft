#include <utils/logger.hh>

int main(void)
{
  utils::logger::Logger log;
  log(utils::logger::level::INFO) << "value: " << 2 << std::endl;

  return 0;
}
