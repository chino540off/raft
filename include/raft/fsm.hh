#ifndef RAFT_FSM_HH_
#define RAFT_FSM_HH_

#include <cassert>
#include <iostream>

#include <utils/fsm.hh>

namespace raft
{

enum class event_t
{
  election,
  majority,
  high_term,
  new_term,
  new_leader,
};

template <typename ostream>
ostream &
operator<<(ostream & os, event_t const & e)
{
  switch (e)
  {
    case event_t::election:
      return os << "election", os;
    case event_t::majority:
      return os << "majority", os;
    case event_t::high_term:
      return os << "high term", os;
    case event_t::new_term:
      return os << "new term", os;
    case event_t::new_leader:
      return os << "new leader", os;
  }
}

enum class state_t
{
  follower,
  candidate,
  leader,
};

template <typename ostream>
ostream &
operator<<(ostream & os, state_t s)
{
  switch (s)
  {
    case state_t::follower:
      return os << "follower", os;
    case state_t::candidate:
      return os << "candidate", os;
    case state_t::leader:
      return os << "leader", os;
  }
}

class fsm
{
public:
  fsm()
  {
    _fsm.set(raft::state_t::follower);

    // follower -(election)-> candidate
    _fsm.on(raft::state_t::follower, raft::event_t::election) = [&](utils::fsm::callback cb) {
      return transit(cb, raft::state_t::candidate);
    };
    // candidate -(election)-> candidate
    _fsm.on(raft::state_t::candidate, raft::event_t::election) = [&](utils::fsm::callback cb) {
      return transit(cb, raft::state_t::candidate);
    };
    // candidate -(majority)-> leader
    _fsm.on(raft::state_t::candidate, raft::event_t::majority) = [&](utils::fsm::callback cb) {
      return transit(cb, raft::state_t::leader);
    };
    // candidate -(new_leader)-> follower
    _fsm.on(raft::state_t::candidate, raft::event_t::new_leader) = [&](utils::fsm::callback cb) {
      return transit(cb, raft::state_t::follower);
    };
    // candidate -(new_term)-> follower
    _fsm.on(raft::state_t::candidate, raft::event_t::new_term) = [&](utils::fsm::callback cb) {
      return transit(cb, raft::state_t::follower);
    };
    // leader -(high_term)-> follower
    _fsm.on(raft::state_t::leader, raft::event_t::high_term) = [&](utils::fsm::callback cb) {
      return transit(cb, raft::state_t::follower);
    };
  }

public:
  bool
  operator()(raft::event_t const & event, utils::fsm::callback cb)
  {
    return _fsm.command(event, cb);
  }

  bool
  operator()(raft::event_t const & event)
  {
    return _fsm.command(event, []() { return true; });
  }

  raft::state_t
  state() const
  {
    return _fsm.get_state();
  }

private:
  bool
  transit(utils::fsm::callback cb, raft::state_t state)
  {
    return cb() ? _fsm.set(state) : false;
  }

private:
  utils::fsm::stack<raft::state_t, raft::event_t> _fsm;
};
}

#endif /** !RAFT_FSM_HH_  */
