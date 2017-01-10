#ifndef RAFT_HH_
# define RAFT_HH_

namespace raft {

class node
{
  public:
    template <typename T>
    node(T id):
      _next_idx(1),
      _match_idx(0),
      _id(id),
      _flags(NODE_VOTING)
    {
    }

  public:
    auto next_idx() const
    {
      return _next_idx;
    }
    template <typename T>
    auto next_idx(T const & next_idx)
    {
      _next_idx = (next_idx < 1) ? 1 : next_idx;
    }

    auto match_idx() const
    {
      return _match_idx;
    }
    template <typename T>
    auto match_idx(T const & match_idx)
    {
      _match_idx = match_idx;
    }

    auto id() const
    {
      return _id;
    }

  private:
    enum flags
    {
      NODE_VOTE_FOR_ME    = (1 << 0),
      NODE_VOTING         = (1 << 1),
      NODE_SUFFICIENT_LOG = (1 << 2),
    };

    template <typename F>
    auto _check_flag(F f) const
    {
      return (_flags & f) != 0;
    }

    template <typename F, typename V>
    auto _set_flag(F f, V v)
    {
      v ? _flags |= f : _flags &= ~f;
      return v;
    }

  public:
    auto has_vote_for_me() const
    {
      return _check_flag(NODE_VOTE_FOR_ME);
    }
    template <typename V>
    auto has_vote_for_me(V v)
    {
      return _set_flag(NODE_VOTE_FOR_ME, v);
    }

    auto is_voting() const
    {
      return _check_flag(NODE_VOTING);
    }
    template <typename V>
    auto is_voting(V v)
    {
      return _set_flag(NODE_VOTING ,v);
    }

    auto has_sufficient_logs() const
    {
      return _check_flag(NODE_SUFFICIENT_LOG);
    }
    template <typename V>
    auto has_sufficient_logs(V v)
    {
      return _set_flag(NODE_SUFFICIENT_LOG ,v);
    }

  private:
    unsigned int _next_idx;
    unsigned int _match_idx;
    unsigned int _id;
    unsigned int _flags;
};

}; /** !raft  */

#endif /** !RAFT_HH_  */

