#include <iostream>
#include <cassert>

#include <raft/store/memory.hh>
#include <raft/store/fs.hh>

template <typename T>
void
utest(T store)
{
  store.c("k", 21);
  assert(store.r("k") == 21);

  store.u("k", 42);
  assert(store.r("k") == 42);

  store.d("k");

  try
  {
    store.r("k");
    assert(false);
  }
  catch (std::out_of_range & e)
  {
    assert(true);
  }

}

int
main(void)
{
  raft::store::memory<std::string, int> store1;
  raft::store::filesystem<std::string, int> store2;

  utest(store1);
  utest(store2);

  return 0;
}
