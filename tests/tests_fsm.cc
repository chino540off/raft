#include <cassert>
#include <iostream>
#include <type_traits>

#include <utils/fsm.hh>

enum class event
{
  open,
  close,
};

template <typename ostream>
inline ostream &
operator<<(ostream & os, event const & e)
{
  switch (e)
  {
    case event::open:
      return os << "open", os;
    case event::close:
      return os << "close", os;
    default:
      assert(true);
  }
  return os;
}

enum class state
{
  opened,
  closed,
};

template <typename ostream>
inline ostream &
operator<<(ostream & os, state const & s)
{
  switch (s)
  {
    case state::opened:
      return os << "opened", os;
    case state::closed:
      return os << "closed", os;
    default:
      assert(true);
  }
  return os;
}

int
main()
{
  utils::fsm::stack<state, event> fsm(state::opened);

  fsm.on(state::opened, event::open) = [&fsm](utils::fsm::callback) { return false; };
  fsm.on(state::opened, utils::fsm::event<event>::_quit) = [&fsm](utils::fsm::callback) {
    return true;
  };
  fsm.on(state::opened, event::close) = [&fsm](utils::fsm::callback) {
    return fsm.set(state::closed);
  };

  fsm.on(state::closed, event::close) = [&fsm](utils::fsm::callback) { return false; };
  fsm.on(state::closed, utils::fsm::event<event>::_quit) = [&fsm](utils::fsm::callback) {
    return true;
  };
  fsm.on(state::closed, event::open) = [&fsm](utils::fsm::callback) {
    return fsm.set(state::opened);
  };

  assert(fsm.get_state() == state::opened);
  std::cout << fsm << std::endl << "------------------------------" << std::endl;
  fsm(event::close);
  assert(fsm.get_state() == state::closed);
  fsm(event::close);
  assert(fsm.get_state() == state::closed);
  fsm(event::open);
  assert(fsm.get_state() == state::opened);
  fsm(event::open);
  assert(fsm.get_state() == state::opened);

  std::cout << fsm << std::endl << "------------------------------" << std::endl;
}
