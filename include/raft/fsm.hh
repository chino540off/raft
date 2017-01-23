#ifndef RAFT_FSM_HH_
# define RAFT_FSM_HH_

# include <cassert>
# include <iostream>

# include <utils/fsm.hh>
# include <utils/logger.hh>

static utils::logger::Logger _log_fsm;
using loglevel = utils::logger::level;

namespace raft {

enum class event
{
  election,
  majority,
  high_term,
  new_term,
  new_leader,
};

template <typename ostream>
ostream &
operator<<(ostream & os, event const & e)
{
  switch (e)
  {
    case event::election: os << "election"; break;
    case event::majority: os << "majority"; break;
    case event::high_term: os << "high term"; break;
    case event::new_term: os << "new term"; break;
    case event::new_leader: os << "new leader"; break;
    default: assert(false);
  }
  return os;
}

enum class state
{
  follower,
  candidate,
  leader,
};

template <typename ostream>
ostream &
operator<<(ostream & os, state const & s)
{
  switch (s)
  {
    case state::follower: os << "follower"; break;
    case state::candidate: os << "candidate"; break;
    case state::leader: os << "leader"; break;
    default: assert(false);
  }
  return os;
}

class fsm
{
  public:

  public:
    fsm()
    {
      _fsm.set(raft::state::follower);

      _fsm.on(raft::state::follower,   raft::event::election)    = [&](utils::fsm::callback cb) { return transit(cb, raft::state::candidate);  };
      _fsm.on(raft::state::candidate,  raft::event::election)    = [&](utils::fsm::callback cb) { return transit(cb, raft::state::candidate);  };
      _fsm.on(raft::state::candidate,  raft::event::majority)    = [&](utils::fsm::callback cb) { return transit(cb, raft::state::leader);     };
      _fsm.on(raft::state::candidate,  raft::event::new_leader)  = [&](utils::fsm::callback cb) { return transit(cb, raft::state::follower);   };
      _fsm.on(raft::state::candidate,  raft::event::new_term)    = [&](utils::fsm::callback cb) { return transit(cb, raft::state::follower);   };
      _fsm.on(raft::state::leader,     raft::event::high_term)   = [&](utils::fsm::callback cb) { return transit(cb, raft::state::follower);   };
    }

  public:
    bool operator()(raft::event const & event, utils::fsm::callback cb)
    {
      return _fsm.command(event, cb);
    }

    raft::state state() const
    {
      return _fsm.get_state();
    }

  private:
    bool transit(utils::fsm::callback cb, raft::state state)
    {
      return cb() ? _fsm.set(state) : false;
    }

  private:
    utils::fsm::stack<raft::state, raft::event> _fsm;
};

}

#endif /** !RAFT_FSM_HH_  */

