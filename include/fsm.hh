#ifndef FSM_HH_
# define FSM_HH_

# include <algorithm>
# include <deque>
# include <functional>
# include <iostream>
# include <map>
# include <sstream>
# include <string>

namespace fsm
{
  template<typename T>
  inline std::string to_string(T const & t)
  {
      std::stringstream ss;
      return ss << t ? ss.str() : std::string();
  }

  template<>
  inline std::string to_string(std::string const & t)
  {
      return t;
  }

  typedef std::function<bool(void)> callback;
  typedef std::function<bool(fsm::callback)> transit;

  struct state
  {
    int name;
    fsm::callback cb;

    state(int const & name = 'null'):
      name(name)
    {}

    state operator()(fsm::callback cb) const
    {
      state self = *this;
      self.cb = cb;
      return self;
    }

    operator int () const
    {
        return name;
    }

    bool operator<(state const & other) const
    {
        return name < other.name;
    }
    bool operator==(state const & other) const
    {
        return name == other.name;
    }

    template<typename ostream>
    inline friend ostream & operator<<(ostream & out, state const & t)
    {
      if (t.name >= 256)
      {
        out << char((t.name >> 24) & 0xff);
        out << char((t.name >> 16) & 0xff);
        out << char((t.name >>  8) & 0xff);
        out << char((t.name >>  0) & 0xff);
      }
      else
      {
        out << t.name;
      }
      return out;
    }
  };

  typedef state trigger;

  struct transition
  {
    fsm::state previous, current;
    fsm::trigger trigger;
  };

  template<typename ostream>
  inline ostream & operator<<(ostream & out, fsm::transition const & t)
  {
      out << t.previous << " -> " << t.trigger << " -> " << t.current;
      return out;
  }

  class stack
  {
    public:
      stack(fsm::state const & start = 'null'):
        deque(1)
      {
        deque[0] = start;
        call(deque.back(), 'init');
      }

      stack(int start):
        stack(fsm::state(start))
      {}

      ~stack()
      {
        // ensure state destructors are called (w/ 'quit')
        while (size())
        {
          pop();
        }
      }

      // pause current state (w/ 'push') and create a new active child (w/ 'init')
      void push(fsm::state const & state)
      {
        if (deque.size() && deque.back() == state)
        {
          return;
        }
        // queue
        call(deque.back(), 'push');
        deque.push_back(state);
        call(deque.back(), 'init');
      }

      // terminate current state and return to parent (if any)
      void pop()
      {
        if (deque.size())
        {
          call(deque.back(), 'quit');
          deque.pop_back();
        }
        if (deque.size())
        {
          call(deque.back(), 'back');
        }
      }

      // set current active state
      bool set(fsm::state const & state)
      {
        if (deque.size())
        {
          replace(deque.back(), state);
        }
        else
        {
          push(state);
        }

        return true;
      }

      // number of children (stack)
      size_t size() const
      {
        return deque.size();
      }

      // info
      // [] classic behaviour: "hello"[5] = undefined, "hello"[-1] = undefined
      // [] extended behaviour: "hello"[5] = h, "hello"[-1] = o, "hello"[-2] = l
      fsm::state get_state(signed pos = -1) const
      {
        signed size = (signed)(deque.size());
        return size ? *(deque.begin() + (pos >= 0 ? pos % size : size - 1 + ((pos+1) % size))) : fsm::state();
      }

      fsm::transition get_log(signed pos = -1) const
      {
        signed size = (signed)(log.size());
        return size ? *(log.begin() + (pos >= 0 ? pos % size : size - 1 + ((pos+1) % size))) : fsm::transition();
      }

      std::string get_trigger() const
      {
        std::stringstream ss;
        return ss << current_trigger, ss.str();
      }

      bool is_state(fsm::state const & state) const
      {
        return deque.empty() ? false : (deque.back() == state);
      }

      /* (idle)___(trigger)__/''(hold)''''(release)''\__
      bool is_idle()      const { return transition.previous == transition.current; }
      bool is_triggered() const { return transition.previous == transition.current; }
      bool is_hold()      const { return transition.previous == transition.current; }
      bool is_released()  const { return transition.previous == transition.current; } */

      // setup
      fsm::transit & on(fsm::state const & from, fsm::state const & to)
      {
        return transits[bistate(from, to)];
      }

      // generic call
      bool call(const fsm::state &from, const fsm::state &to) const
      {
        std::map<bistate, fsm::transit>::const_iterator found = transits.find(bistate(from, to));

        if (found != transits.end())
        {
          log.push_back({from, current_trigger, to});

          if (log.size() > 50)
          {
            log.pop_front();
          }
          return found->second(to.cb);
        }
        return false;
      }

      // user commands
      bool command(fsm::state const & trigger)
      {
        size_t size = this->size();
        if (!size)
        {
          return false;
        }

        current_trigger = fsm::state();
        std::deque<states::reverse_iterator> aborted;

        for (auto it = deque.rbegin(); it != deque.rend(); ++it)
        {
          fsm::state &self = *it;

          if (!call(self,trigger))
          {
            aborted.push_back(it);
            continue;
          }

          for (auto it = aborted.begin(), end = aborted.end(); it != end; ++it)
          {
            call(**it, 'quit');
            deque.erase(--(it->base()));
          }

          current_trigger = trigger;
          return true;
        }
        return false;
      }

      bool command(fsm::state const & trigger, fsm::callback cb)
      {
        return command(trigger(cb));
      }

      // debug
      template<typename ostream>
      ostream & debug(ostream & out) const
      {
        int total = log.size();
        std::string sep = "\t";

        out << "status {" << std::endl;

        for (states::const_reverse_iterator it = deque.rbegin(), end = deque.rend(); it != end; ++it)
        {
          out << sep << *it;
          sep = " -> ";
        }
        out << std::endl;

        out << "} log (" << total << " entries) {" << std::endl;

        for (int i = 0 ; i < total; ++i)
        {
          out << "\t" << log[i] << std::endl;
        }
        out << "}" << std::endl;
        return out;
      }

      // aliases
      bool operator()(fsm::state const & trigger, fsm::callback cb)
      {
        return command(trigger, cb);
      }

    protected:
      void replace(fsm::state & current, fsm::state const & next)
      {
        call(current, 'quit');
        current = next;
        call(current, 'init');
      }

      typedef std::pair<int, int> bistate;
      std::map<bistate, fsm::transit> transits;

      mutable std::deque<fsm::transition> log;
      std::deque<fsm::state> deque;
      fsm::state current_trigger;

      typedef std::deque<fsm::state> states;
  };

  template<typename ostream>
  inline ostream & operator<<(ostream & out, stack const & t)
  {
    return t.debug(out), out;
  }

}
#endif /** !FSM_HH_  */

