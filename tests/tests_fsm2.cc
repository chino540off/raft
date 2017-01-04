#include <iostream>
#include <raft.hh>

#include <cassert>

int main()
{
  raft::fsm raft;

  assert(raft.state() == raft::state::follower);
  assert(!raft(raft::event::majority, []() { return true; }));

  assert(raft(raft::event::election, []() { return true; }));
  assert(raft.state() == raft::state::candidate);

  assert(raft(raft::event::majority, []() { return true; }));
  assert(raft.state() == raft::state::leader);
}
