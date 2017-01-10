#ifndef RAFT_FSM_HH_
# define RAFT_FSM_HH_

# include <utils/fsm.hh>
# include <iostream>

namespace raft {

class fsm
{
  public:
    enum event
    {
      election,
      majority,
      high_term,
      new_term,
      new_leader,
    };

    enum state
    {
      follower,
      candidate,
      leader,
    };

  public:
    fsm()
    {
      _fsm.set(state::follower);

      _fsm.on(state::follower,   event::election)    = [&](utils::fsm::callback cb) { return transit(cb, state::candidate);  };
      _fsm.on(state::candidate,  event::election)    = [&](utils::fsm::callback cb) { return transit(cb, state::candidate);  };
      _fsm.on(state::candidate,  event::majority)    = [&](utils::fsm::callback cb) { return transit(cb, state::leader);     };
      _fsm.on(state::candidate,  event::new_leader)  = [&](utils::fsm::callback cb) { return transit(cb, state::follower);   };
      _fsm.on(state::candidate,  event::new_term)    = [&](utils::fsm::callback cb) { return transit(cb, state::follower);   };
      _fsm.on(state::leader,     event::high_term)   = [&](utils::fsm::callback cb) { return transit(cb, state::follower);   };
    }

  public:
    bool operator()(utils::fsm::state const & trigger, utils::fsm::callback cb)
    {
      return _fsm.command(trigger(cb));
    }

    int state() const
    {
      return _fsm.get_state();
    }

  private:
    bool transit(utils::fsm::callback cb, utils::fsm::state state)
    {
      return cb() ? _fsm.set(state) : false;
    }

  private:
    utils::fsm::stack _fsm;
};

};
#endif /** !RAFT_FSM_HH_  */

