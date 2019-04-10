#ifndef UTILS_FSM_HH_
#define UTILS_FSM_HH_

#include <algorithm>
#include <deque>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

namespace utils
{
namespace fsm
{

typedef std::function<bool(void)> callback;
typedef std::function<bool(fsm::callback)> transit;

template <typename T>
struct node
{
  static node<T> _null;
  static node<T> _init;
  static node<T> _push;
  static node<T> _back;
  static node<T> _quit;

  T value;
  fsm::callback cb;

  template <typename std::enable_if<std::is_enum<T>::value>::type * = nullptr>
  node(T v) : value(v)
  {
  }

  node
  operator()(fsm::callback cb) const
  {
    node self = *this;
    self.cb = cb;
    return self;
  }

  explicit operator T() const { return value; }

  bool
  operator<(node const & other) const
  {
    return value < other.value;
  }
  bool
  operator==(node const & other) const
  {
    return value == other.value;
  }
};

template <typename T>
node<T> node<T>::_null = node<T>((T) 'null');
template <typename T>
node<T> node<T>::_init = node<T>((T) 'init');
template <typename T>
node<T> node<T>::_push = node<T>((T) 'push');
template <typename T>
node<T> node<T>::_back = node<T>((T) 'back');
template <typename T>
node<T> node<T>::_quit = node<T>((T) 'quit');

template <typename T>
using event = node<T>;

template <typename T>
using state = node<T>;

template <typename S, typename E>
struct transition
{
  fsm::state<S> from, to;
  fsm::event<E> event;
};

template <typename S, typename E>
class stack
{
public:
  stack(fsm::state<S> const & start = fsm::state<S>::_null) : current_event(fsm::event<E>::_init)
  {
    deque.push_back(start);
    call(deque.back(), fsm::event<E>::_init);
  }

  stack(S start) : stack(fsm::state<S>(start)) {}

  ~stack()
  {
    // ensure state destructors are called (w/ 'quit')
    while (size())
    {
      pop();
    }
  }

  // pause current state (w/ 'push') and create a new active child (w/ 'init')
  void
  push(fsm::state<S> const & state)
  {
    if (deque.size() && deque.back() == state)
    {
      return;
    }
    // queue
    call(deque.back(), fsm::event<E>::_push);
    deque.push_back(state);
    call(deque.back(), fsm::event<E>::_init);
  }

  // terminate current state and return to parent (if any)
  void
  pop()
  {
    if (deque.size())
    {
      call(deque.back(), fsm::event<E>::_quit);
      deque.pop_back();
    }
    if (deque.size())
    {
      call(deque.back(), fsm::event<E>::_back);
    }
  }

  // set current active state
  bool
  set(fsm::state<S> const & state)
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
  size_t
  size() const
  {
    return deque.size();
  }

  // info
  // [] classic behaviour: "hello"[5] = undefined, "hello"[-1] = undefined
  // [] extended behaviour: "hello"[5] = h, "hello"[-1] = o, "hello"[-2] = l
  S
  get_state(signed pos = -1) const
  {
    signed size = (signed) (deque.size());
    return size ? (deque.begin() + (pos >= 0 ? pos % size : size - 1 + ((pos + 1) % size)))->value
                : fsm::state<S>::_null.value;
  }

  fsm::transition<S, E>
  get_log(signed pos = -1) const
  {
    signed size = (signed) (log.size());
    return size ? *(log.begin() + (pos >= 0 ? pos % size : size - 1 + ((pos + 1) % size)))
                : fsm::transition<S, E>();
  }

  std::string
  get_trigger() const
  {
    std::stringstream ss;
    return ss << current_event, ss.str();
  }

  bool
  is_state(fsm::state<S> const & state) const
  {
    return deque.empty() ? false : (deque.back() == state);
  }

  /* (idle)___(event)__/''(hold)''''(release)''\__
  bool is_idle()      const { return transition.previous == transition.current; }
  bool is_triggered() const { return transition.previous == transition.current; }
  bool is_hold()      const { return transition.previous == transition.current; }
  bool is_released()  const { return transition.previous == transition.current; } */

  // setup
  fsm::transit &
  on(fsm::state<S> const & from, fsm::event<E> const & event)
  {
    return transits[ bistate(from, event) ];
  }

  // generic call
  bool
  call(fsm::state<S> const & from, fsm::event<E> const & event) const
  {
    auto found = transits.find(std::make_pair(from, event));
    auto current = from;

    if (found != transits.end())
    {
      bool ret = found->second(event.cb);

      log.push_back({current, get_state(), event});
      if (log.size() > 50)
      {
        log.pop_front();
      }

      return ret;
    }
    return false;
  }

  // user commands
  bool
  command(fsm::event<E> const & event)
  {
    if (!size())
    {
      return false;
    }

    current_event = fsm::event<E>::_init;
    std::deque<typename std::deque<fsm::state<S>>::reverse_iterator> aborted;

    for (auto it = deque.rbegin(); it != deque.rend(); ++it)
    {
      fsm::state<S> & self = *it;

      if (!call(self, event))
      {
        aborted.push_back(it);
        continue;
      }

      for (auto it = aborted.begin(), end = aborted.end(); it != end; ++it)
      {
        call(**it, fsm::event<E>::_quit);
        deque.erase(--(it->base()));
      }

      current_event = event;
      return true;
    }
    return false;
  }

  bool
  command(fsm::event<E> const & event, fsm::callback cb)
  {
    return command(event(cb));
  }

  // aliases
  bool
  operator()(fsm::event<E> const & event, fsm::callback cb = nullptr)
  {
    return command(event, cb);
  }

  // debug
  template <typename ostream>
  ostream &
  debug(ostream & os) const
  {
    int total = log.size();

    os << "status" << std::endl << "{" << std::endl;
    std::string sep = "\t";

    for (auto it = deque.rbegin(), end = deque.rend(); it != end; ++it)
    {
      os << sep << *it;
      sep = " -> ";
    }
    os << std::endl << "}" << std::endl;

    os << "log (" << total << " entries)" << std::endl << "{" << std::endl;
    for (auto & i : log)
    {
      os << "\t" << i << std::endl;
    }
    os << "}";
    return os;
  }

protected:
  void
  replace(fsm::state<S> & current, fsm::state<S> const & next)
  {
    call(current, fsm::event<E>::_quit);
    current = next;
    call(current, fsm::event<E>::_init);
  }

  typedef std::pair<fsm::state<S>, fsm::event<E>> bistate;
  std::map<bistate, fsm::transit> transits;
  std::deque<fsm::state<S>> deque;
  fsm::event<E> current_event;

  mutable std::deque<fsm::transition<S, E>> log;
};

template <typename ostream, typename T>
inline ostream &
operator<<(ostream & os, node<T> const n)
{
  if (n == node<T>::_null)
    return os << "_null", os;
  else if (n == node<T>::_init)
    return os << "_init", os;
  else if (n == node<T>::_push)
    return os << "_push", os;
  else if (n == node<T>::_back)
    return os << "_back", os;
  else if (n == node<T>::_quit)
    return os << "_quit", os;
  else
    return os << n.value, os;
}

template <typename ostream, typename S, typename E>
inline ostream &
operator<<(ostream & os, transition<S, E> const t)
{
  return os << t.from << " --(" << t.event << ")--> " << t.to, os;
}

template <typename ostream, typename S, typename E>
inline ostream &
operator<<(ostream & os, stack<S, E> const & s)
{
  return s.debug(os), os;
}

} /** !fsm  */
} /** !utils  */

#endif /** !UTILS_FSM_HH_  */
