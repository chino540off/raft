#include <iostream>

#include <raft/store.hh>

int
main(void)
{
  raft::store::memory<std::string, int> store;

  store.c("k", 21);
  std::cout << store.r("k") << std::endl;

  store.u("k", 42);
  std::cout << store.r("k") << std::endl;

  store.d("k");

  std::cout << store.r("k") << std::endl;

  return 0;
}
