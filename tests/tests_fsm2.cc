#include <iostream>
#include <raft/fsm.hh>

#include <cassert>

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
