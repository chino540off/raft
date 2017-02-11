#ifndef RAFT_LOG_HH_
# define RAFT_LOG_HH_

# include <iostream>
# include <iomanip>
# include <deque>
# include <functional>

namespace raft {

template <typename E, typename T, typename I>
struct entry
{
  T term;
  I id;
  E elt;
};

template <typename ostream,
          typename E, typename T, typename I>
inline ostream & operator<<(ostream & os, entry<E, T, I> const & entry)
{
  os << "entry(term: " << entry.term
     << ", id: " << entry.id
     << ", elt: " << entry.elt
     << ")";
  return os;
}

template <typename E,
          typename T = unsigned long int,
          typename I = unsigned long int>
class log
{
  private:
    typedef entry<E, T, I> entry_t;
    typedef std::deque<entry_t> logs_t;
    typedef typename logs_t::size_type index_t;

  public:
    typedef E element_t;
    typedef T term_t;
    typedef I id_t;

  public:
    log(): _base(0) { }

  public:
    auto current() const
    {
      return _deque.size() + _base;
    }

    auto remove(index_t i, std::function<void(entry_t const &)> callback)
    {
      i = i - _base;

      for (index_t end = _deque.size(); i < end; ++i)
      {
        entry_t entry = _deque.front();
        callback(entry);

        _deque.pop_front();
      }
    }

    auto append(element_t const & e, term_t const & term = 0, id_t const & id = 0)
    {
      _deque.push_back({term, id, e});
    }

    auto const & at(index_t i) const
    {
      i = i - _base - 1;
      return _deque.at(i);
    }

    auto count() const
    {
      return _deque.size();
    }

    auto poll(std::function<void(entry_t const &)> callback)
    {
      if (_deque.size() == 0)
        return;

      entry_t entry = _deque.front();
      callback(entry);

      _deque.pop_front();
      _base++;
    }

  public:
    template <typename ostream>
    auto & print(ostream & os) const
    {
      os << "log(count: " << _deque.size()
         << ", base: " << _base
         << "): "<< std::endl;

      for (index_t i = 0; i < _deque.size(); ++i)
      {
        os << "[" << std::setfill('0') << std::setw(16) << i + _base + 1 << "]: "
           << _deque[i];

        if (i < _deque.size() - 1)
          os << std::endl;
      }
      return os;
    }

  private:
    logs_t _deque;
    index_t _base;
};

template <typename ostream,
          typename E, typename T, typename I>
inline auto & operator<<(ostream & os, log<E, T, I> const & log)
{
  return log.print(os);
}

} /** !raft  */

#endif /** !RAFT_LOG_HH_  */

