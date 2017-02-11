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
          typename T = unsigned int,
          typename I = unsigned int>
class log
{
  typedef entry<E, T, I> log_entry;

  public:
    log(): _base(0) { }

  public:
    unsigned int current() const
    {
      return _deque.size() + _base;
    }

    void remove(int i, std::function<void(log_entry const &)> callback)
    {
      i = i - _base;

      for (int end = _deque.size(); i < end; ++i)
      {
        log_entry entry = _deque.front();
        callback(entry);

        _deque.pop_front();
      }
    }

    void append(E const & e, T const & term = 0, I const & id = 0)
    {
      _deque.push_back({term, id, e});
    }

    log_entry const & at(unsigned int i) const
    {
      i = i - _base - 1;
      return _deque.at(i);
    }

    auto count() const
    {
      return _deque.size();
    }

    void poll(std::function<void(log_entry const &)> callback)
    {
      if (_deque.size() == 0)
        return;

      log_entry entry = _deque.front();
      callback(entry);

      _deque.pop_front();
      _base++;
    }

  public:
    template <typename ostream>
    ostream & print(ostream & os) const
    {
      os << "log(count: " << _deque.size()
         << ", base: " << _base
         << "): "<< std::endl;
      for (unsigned int i = 0; i < _deque.size(); ++i)
      {
        os << "[" << std::setfill('0') << std::setw(16) << i + _base + 1 << "]: "
           << _deque[i];

        if (i < _deque.size() - 1)
          os << std::endl;
      }
      return os;
    }

  private:
    std::deque<log_entry> _deque;
    unsigned int _base;
};

template <typename ostream,
          typename E, typename T, typename I>
inline ostream & operator<<(ostream & os, log<E, T, I> const & log)
{
  return log.print(os);
}

} /** !raft  */

#endif /** !RAFT_LOG_HH_  */

