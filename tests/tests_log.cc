#include <cassert>

#include <raft/log.hh>

int main(void)
{
  raft::log<unsigned int> log;

  for (unsigned int i = 1; i <= 10; ++i)
  {
    log.append(i + 100);
    assert(log.current() == i);
  }
  std::cout << log << std::endl;

  for (unsigned int i = 1; i <= 10; ++i)
  {
    log.poll([i](auto entry)
    {
      assert(entry.elt == i + 100);
    });
  }

  log.poll([](auto) { assert(false); });

  std::cout << log << std::endl;

  for (unsigned int i = 1; i <= 10; ++i)
    log.append(i + 100);

  for (unsigned int i = 1; i <= 10; ++i)
    assert(log.at(i + 10).elt == i + 100);

  assert(log.count() == 10);

  std::cout << log << std::endl;

  unsigned int count = 0;
  log.remove(10, [&count](auto) { count++; });
  assert(count == 10);

  return 0;
}
