#ifndef RAFT_FSM_HH_
# define RAFT_FSM_HH_

# include <utils/fsm.hh>
# include <iostream>

namespace raft {

enum class event
{
  election,
  majority,
  high_term,
  new_term,
  new_leader,
};

enum class state
{
  follower,
  candidate,
  leader,
};

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

