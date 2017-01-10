#include <cassert>

#include <raft/node.hh>

int main(void)
{
  auto node = raft::node<void>(1);

  assert(node.is_voting());
  assert(node.is_voting(false) == false);
  assert(node.is_voting() == false);
}
