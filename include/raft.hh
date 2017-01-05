#ifndef RAFT_HH_
# define RAFT_HH_

# include <fsm.hh>
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

      _fsm.on(state::follower,   event::election)    = [&](::fsm::callback cb) { return transit(cb, state::candidate);  };
      _fsm.on(state::candidate,  event::election)    = [&](::fsm::callback cb) { return transit(cb, state::candidate);  };
      _fsm.on(state::candidate,  event::majority)    = [&](::fsm::callback cb) { return transit(cb, state::leader);     };
      _fsm.on(state::candidate,  event::new_leader)  = [&](::fsm::callback cb) { return transit(cb, state::follower);   };
      _fsm.on(state::candidate,  event::new_term)    = [&](::fsm::callback cb) { return transit(cb, state::follower);   };
      _fsm.on(state::leader,     event::high_term)   = [&](::fsm::callback cb) { return transit(cb, state::follower);   };
    }

  public:
    bool operator()(::fsm::state const & trigger, ::fsm::callback cb)
    {
      return _fsm.command(trigger(cb));
    }

    int state() const
    {
      return _fsm.get_state();
    }

  private:
    bool transit(::fsm::callback cb, ::fsm::state state)
    {
      return cb() ? _fsm.set(state) : false;
    }

  private:
    ::fsm::stack _fsm;
};

};
#endif /** !RAFT_HH_  */

