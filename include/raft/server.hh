#ifndef RAFT_SERVER_HH_
#define RAFT_SERVER_HH_

#include <cassert>
#include <chrono>
#include <memory>
#include <random>
#include <unordered_map>

#include <raft/fsm.hh>
#include <raft/log.hh>
#include <raft/node.hh>
#include <raft/rpc.hh>
#include <raft/traits.hh>

using namespace std::chrono_literals;

namespace raft
{

enum class status_t
{
  ok = 0,
  fail = 1,
  enomem = 2,
};

template <>
struct enum_traits<status_t>
{
  static constexpr bool has_any = true;
};

status_t
convert(log_status_t e)
{
  switch (e)
  {
    case log_status_t::ok:
      return status_t::ok;

    case log_status_t::fail:
      return status_t::fail;

    default:
      assert(false);
  }
}

template <typename T,
          typename node_user_data_t = void,
          typename node_id_t = unsigned long int,
          typename term_t_ = unsigned long int,
          typename index_id_t_ = unsigned long int>
class server
{
public:
  using log_t = log<T, term_t_, index_id_t_>;
  using index_t = typename log_t::index_t;
  using index_id_t = typename log_t::id_t;
  using term_t = typename log_t::term_t;
  using entry_t = typename log_t::entry_t;

  using node_t = node<node_user_data_t, node_id_t, index_t>;
  using nodes_t = std::unordered_map<node_id_t, std::shared_ptr<node_t>>;

  using vote_request_t = rpc::vote_request_t<term_t, index_t, node_id_t>;
  using vote_response_t = rpc::vote_response_t<term_t>;
  using appendentries_request_t = rpc::appendentries_request_t<T, term_t, index_t, index_id_t>;
  using appendentries_response_t = rpc::appendentries_response_t<term_t, index_t>;

public:
  server()
    : current_term_(0)
    , commit_index_(0)
    , last_applied_index_(0)
    , elapsed_timeout_(0ms)
    , request_timeout_(200ms)
    , election_timeout_(1000ms)
    , gen_(rd_())
    , state_(state_t::follower)
    , this_node_(nullptr)
    , voted_for_(nullptr)
    , leader_(nullptr)
  {
    randomize_election_timeout();
  }

public:
  std::shared_ptr<node_t>
  node_add(node_id_t const & id, bool is_self = false)
  {
    /* set to voting if node already exists */
    auto node = node_get(id);
    if (node)
    {
      if (!node->is_voting())
      {
        node->is_voting(true);
        return node;
      }
      else
      {
        /* we shouldn't add a node twice */
        return nullptr;
      }
    }

    node = std::make_shared<node_t>(id);
    if (node == nullptr)
      return nullptr;

    nodes_.insert({id, node});

    if (is_self)
      this_node_ = node;

    return node;
  }

  std::shared_ptr<node_t>
  node_non_voting_add(node_id_t const & id, bool is_self = false)
  {
    if (node_get(id))
      return nullptr;

    auto node = node_add(id, is_self);
    if (node == nullptr)
      return nullptr;

    node->is_voting(false);

    return node;
  }

  void
  node_remove(node_id_t id)
  {
    auto it = nodes_.find(id);

    if (it != nodes_.end())
      nodes_.erase(it);
  }

  std::shared_ptr<node_t>
  node_get(node_id_t const & id) const
  {
    auto it = nodes_.find(id);

    if (it == nodes_.cend())
      return nullptr;
    else
      return it->second;
  }

  typename nodes_t::size_type
  node_count() const
  {
    return nodes_.size();
  }

  std::shared_ptr<node_t>
  my_node() const
  {
    return this_node_;
  }

public:
  std::shared_ptr<node_t>
  voted_for() const
  {
    return voted_for_;
  }
  status_t
  vote_for(std::shared_ptr<node_t> node)
  {
    // FIXME persist vote
    voted_for_ = node;

    return status_t::ok;
  }

  unsigned int
  num_voting_nodes() const
  {
    unsigned int num = 0;

    for (auto & it : nodes_)
    {
      auto node = it.second;
      if (node->is_active() && node->is_voting())
        ++num;
    }

    return num;
  }

  unsigned int
  num_voting_nodes_for_me() const
  {
    unsigned int num = 0;

    for (auto & it : nodes_)
    {
      auto node = it.second;
      if (node->is_active() && node->is_voting() && node->has_vote_for_me())
        ++num;
    }

    if (voted_for_ == this_node_)
      ++num;

    return num;
  }

  bool
  should_grant_vote(std::shared_ptr<node_t> node, vote_request_t const & req);

  bool
  is_majority(unsigned int const nnodes, unsigned int const nvotes)
  {
    if (nnodes < nvotes)
      return false;

    return ((nnodes / 2) + 1) <= nvotes;
  }

public:
  std::shared_ptr<node_t>
  leader() const
  {
    return leader_;
  }

public:
  term_t
  current_term() const
  {
    return current_term_;
  }
  status_t
  current_term(term_t const & t)
  {
    if (current_term_ < t)
    {
      // FIXME persist term
      current_term_ = t;
      voted_for_ = nullptr;
    }

    return status_t::ok;
  }

  index_t
  current_index() const
  {
    return log_.current();
  }

  index_t
  commit_index() const
  {
    return commit_index_;
  }
  void
  commit_index(index_t const & idx)
  {
    assert(commit_index_ <= idx);
    assert(idx <= current_index());
    commit_index_ = idx;
  }

  index_t
  last_applied_index() const
  {
    return last_applied_index_;
  }
  void
  last_applied_index(index_t const & idx)
  {
    last_applied_index_ = idx;
  }

  term_t
  last_log_term() const
  {
    index_t index = current_index();

    if (0 < index)
    {
      std::shared_ptr<entry_t> e = get(index);

      if (e)
        return e->term;
    }

    return 0;
  }

public:
  status_t
  apply_entry()
  {
    // if (!is_apply_allowed())
    //  return status_t::fail;

    if (last_applied_index_ == commit_index())
      return status_t::fail;

    index_t log_index = last_applied_index_ + 1;
    std::shared_ptr<entry_t> e = get(log_index);
    if (e == nullptr)
      return status_t::fail;

    ++last_applied_index_;
    // FIXME: apply log

    if (log_index == voting_cfg_change_log_index_)
      voting_cfg_change_log_index_ = -1;

    return status_t::ok;
  }

public:
  status_t
  append(entry_t const & e)
  {
    return convert(log_.append(e));
  }

  std::shared_ptr<entry_t>
  get(index_t const & index) const
  {
    return log_.at(index);
  }

public:
  void
  become_follower()
  {
    state_ = state_t::follower;
    randomize_election_timeout();
    elapsed_timeout_ = 0ms;
  }

  status_t
  become_candidate()
  {
    status_t ret = current_term(current_term() + 1);
    if (any(ret))
      return ret;

    for (auto & p : nodes_)
    {
      auto node = p.second;

      p.second->has_vote_for_me(false);
    }

    vote_for(this_node_);
    leader_ = nullptr;
    state_ = state_t::candidate;

    randomize_election_timeout();
    elapsed_timeout_ = 0ms;

    for (auto & p : nodes_)
    {
      auto node = p.second;

      if (node != this_node_ && node->is_active() && node->is_voting())
      {
        send_request_vote(node);
      }
    }

    return status_t::ok;
  }

  status_t
  become_leader()
  {
    state_ = state_t::leader;
    leader_ = this_node_;

    elapsed_timeout_ = 0ms;
    for (auto & it : nodes_)
    {
      auto node = it.second;

      if (node == this_node_ || node->is_active())
        continue;

      node->next_index(current_index() + 1);
      node->match_index(0);

      // FIXME: send appendentries
    }

    return status_t::ok;
  }

public:
  status_t
  election_start()
  {
    return become_candidate();
  }

public:
  template <typename F>
  status_t
  send_request_vote(std::shared_ptr<node_t> node, F && f)
  {
    assert(node != nullptr);
    assert(node != this_node_);

    vote_request_t msg{current_term_, node->id(), current_index(), last_log_term()};

    return f(node, msg);
  }

  status_t
  send_request_vote(std::shared_ptr<node_t> node)
  {
    return send_request_vote(node, [](auto, auto) { return status_t::ok; });
  }

public:
  status_t
  recv_vote_request(std::shared_ptr<node_t> node,
                    vote_request_t const & req,
                    vote_response_t & resp);

  status_t
  recv_vote_response(std::shared_ptr<node_t> node, vote_response_t const & resp);

public:
  state_t
  state() const
  {
    return state_;
  }

  void
  state(state_t state)
  {
    state_ = state;
  }

  bool
  is_leader() const
  {
    return state_ == state_t::leader;
  }

  bool
  is_candidate() const
  {
    return state_ == state_t::candidate;
  }

  bool
  is_follower() const
  {
    return state_ == state_t::follower;
  }

public:
  void
  election_timeout(std::chrono::milliseconds t)
  {
    election_timeout_ = t;
  }

  std::chrono::milliseconds
  election_timeout() const
  {
    return election_timeout_;
  }

  std::chrono::milliseconds
  elapsed_timeout() const
  {
    return elapsed_timeout_;
  }

  status_t
  periodic(std::chrono::milliseconds p)
  {
    elapsed_timeout_ = elapsed_timeout_ + p;

    if (num_voting_nodes() == 1 && this_node_->is_voting() && !is_leader())
    {
      become_leader();
    }

    if (is_leader())
    {
      if (request_timeout_ <= elapsed_timeout_)
      {
        // FIXME: send_appendentries_all
      }
    }
    else if (election_timeout_rand_ < elapsed_timeout_)
    {
    }

    return status_t::ok;
  }

private:
  void
  randomize_election_timeout()
  {
    std::uniform_int_distribution<> dis(0, election_timeout_.count() - 1);

    election_timeout_rand_ = election_timeout_ + std::chrono::milliseconds(dis(gen_));
  }

private:
  term_t current_term_;
  index_t commit_index_;
  index_t last_applied_index_;
  index_t voting_cfg_change_log_index_;

  std::chrono::milliseconds elapsed_timeout_;
  std::chrono::milliseconds request_timeout_;
  std::chrono::milliseconds election_timeout_;
  std::chrono::milliseconds election_timeout_rand_;

  std::random_device rd_; // Will be used to obtain a seed for the random number engine
  std::mt19937 gen_;      // Standard mersenne_twister_engine seeded with rd()

  log_t log_;

  // fsm fsm_;
  state_t state_;

  nodes_t nodes_;
  std::shared_ptr<node_t> this_node_;
  std::shared_ptr<node_t> voted_for_;
  std::shared_ptr<node_t> leader_;
};

template <typename ostream, typename T>
ostream &
operator<<(ostream & os, server<T> const & server)
{
  return server.print(os);
}

} /** !raft  */

#include <raft/server.hxx>

#endif /** !RAFT_SERVER_HH_  */
