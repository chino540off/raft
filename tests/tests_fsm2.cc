#include <iostream>
#include <raft.hh>

#include <cassert>

int main()
{
  raft::fsm raft;

  assert(raft.state() == raft::fsm::state::follower);
  assert(!raft(raft::fsm::event::majority, []() { return true; }));

  assert(raft(raft::fsm::event::election, []() { return true; }));
  assert(raft.state() == raft::fsm::state::candidate);

  assert(raft(raft::fsm::event::majority, []() { return true; }));
  assert(raft.state() == raft::fsm::state::leader);
}
