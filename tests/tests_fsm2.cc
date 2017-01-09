#include <iostream>
#include <cassert>

#include <raft/fsm.hh>

int main()
{
  raft::fsm fsm;

  assert(fsm.state() == raft::fsm::state::follower);
  assert(!fsm(raft::fsm::event::majority, []() { return true; }));

  assert(fsm(raft::fsm::event::election, []() { return true; }));
  assert(fsm.state() == raft::fsm::state::candidate);

  assert(fsm(raft::fsm::event::majority, []() { return true; }));
  assert(fsm.state() == raft::fsm::state::leader);
}
