#include <cassert>
#include <iostream>

#include <raft/store/fs.hh>
#include <raft/store/memory.hh>

template <typename T, typename Key, typename Value>
void
utest(T store, Key const & k, Value const & v1, Value const & v2)
{
  store.c(k, v1);
  assert(store.r(k) == v1);

  store.u(k, v2);
  assert(store.r(k) == v2);

  store.d(k);

  try
  {
    store.r(k);
    assert(false);
  }
  catch (std::out_of_range & e)
  {
    assert(true);
  }
}

struct MyKey
{
  int k;
};

template <typename ostream>
inline ostream &
operator<<(ostream & os, MyKey const & k)
{
  os << k.k;
  return os;
}

struct MyValue
{
  int v;

  bool
  operator==(MyValue const & v)
  {
    return this->v == v.v;
  }
};

int
main(void)
{
  utest(raft::store::memory<std::string, int>(), "k", 42, 21);
  utest(raft::store::filesystem<std::string, int>(), "k", 42, 21);
  utest(raft::store::filesystem<int, int>(), 101, 42, 21);
  utest(raft::store::filesystem<MyKey, MyValue>(), MyKey({101}), MyValue({42}), MyValue({21}));

  return 0;
}
