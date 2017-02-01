#ifndef RAFT_LOG_HH_
# define RAFT_LOG_HH_

namespace raft {

struct entry
{
  unsigned int term;
  unsigned int id;
};

template <typename T>
class log
{
  public:
    int current() const
    {
      return _deque.size();
    }

    void remove(int i)
    {
    }

    void append(T const & e)
    {
      _deque.push_back(e);
    }

    T const & at(int i) const
    {
      return _deque.at(i);
    }

    auto count() const
    {
      return _deque.size();
    }

  private:
    std::deque<T> _deque;
};

} /** !raft  */

#endif /** !RAFT_LOG_HH_  */

