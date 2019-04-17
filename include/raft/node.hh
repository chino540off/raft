#ifndef RAFT_NODE_HH_
#define RAFT_NODE_HH_

#include <memory>

namespace raft
{

template <typename T, typename id_t = unsigned long int, typename index_t = unsigned long int>
class node
{
public:
  using user_data_t = std::shared_ptr<T>;

public:
  node(id_t const & id, user_data_t const & user_data = nullptr)
    : id_(id), next_index_(1), match_index_(0), user_data_(user_data), flags_(NODE_VOTING)
  {
  }

public:
  index_t
  next_index() const
  {
    return next_index_;
  }
  void
  next_index(index_t const & idx)
  {
    next_index_ = (idx < 1) ? 1 : idx;
  }

public:
  index_t
  match_index() const
  {
    return match_index_;
  }
  void
  match_index(index_t const & idx)
  {
    match_index_ = idx;
  }

public:
  id_t
  id() const
  {
    return id_;
  }

private:
  enum flags
  {
    NODE_VOTE_FOR_ME = (1 << 0),
    NODE_VOTING = (1 << 1),
    NODE_SUFFICIENT_LOG = (1 << 2),
    NODE_INACTIVE = (1 << 3),
    NODE_VOTING_COMMITED = (1 << 4),
    NODE_ADDITION_COMMITED = (1 << 5),
  };

  template <typename F>
  bool
  _check_flag(F f) const
  {
    return (flags_ & f) != 0;
  }

  template <typename F, typename V>
  void
  _set_flag(F f, V v)
  {
    v ? flags_ |= f : flags_ &= ~f;
  }

public:
  bool
  has_vote_for_me() const
  {
    return _check_flag(NODE_VOTE_FOR_ME);
  }
  template <typename V>
  void
  has_vote_for_me(V v)
  {
    _set_flag(NODE_VOTE_FOR_ME, v);
  }

  bool
  is_voting() const
  {
    return _check_flag(NODE_VOTING);
  }
  template <typename V>
  void
  is_voting(V v)
  {
    _set_flag(NODE_VOTING, v);
  }

  bool
  has_sufficient_logs() const
  {
    return _check_flag(NODE_SUFFICIENT_LOG);
  }
  template <typename V>
  void
  has_sufficient_logs(V v)
  {
    _set_flag(NODE_SUFFICIENT_LOG, v);
  }

  bool
  is_active() const
  {
    return !_check_flag(NODE_INACTIVE);
  }
  template <typename V>
  void
  is_active(V v)
  {
    _set_flag(NODE_INACTIVE, !v);
  }

  bool
  is_voting_commited() const
  {
    return _check_flag(NODE_VOTING_COMMITED);
  }
  template <typename V>
  void
  is_voting_commited(V v)
  {
    _set_flag(NODE_VOTING_COMMITED, v);
  }

  bool
  is_addition_commited() const
  {
    return _check_flag(NODE_ADDITION_COMMITED);
  }
  template <typename V>
  void
  is_addition_commited(V v)
  {
    _set_flag(NODE_ADDITION_COMMITED, v);
  }

public:
  template <typename ostream>
  ostream &
  print(ostream & os) const
  {
    os << "{"
       << "\"id\": " << id_ << "}";
    return os;
  }

private:
  id_t id_;
  index_t next_index_;
  index_t match_index_;
  user_data_t user_data_;

  unsigned int flags_;
};

template <typename ostream,
          typename T,
          typename id_t = unsigned long int,
          typename index_t = unsigned long int>
ostream &
operator<<(ostream & os, node<T, id_t, index_t> const & node)
{
  return node.print(os);
}

} /** !raft  */

#endif /** !RAFT_NODE_HH_  */
