#ifndef RAFT_SERVER_HH_
# define RAFT_SERVER_HH_

# include <map>

# include <raft/fsm.hh>
# include <raft/node.hh>
# include <raft/rpc.hh>

namespace raft {

template <typename T>
class server
{
  public:
    typedef raft::node<T> _node_t;
    typedef std::shared_ptr<_node_t> node_t;
    typedef std::map<unsigned int, node_t> nodes_t;

    template <typename rpc>
    using rpc_callback = std::function<void(server<T> &, _node_t &, rpc const &)>;

    typedef rpc_callback<raft::rpc::request_vote>   req_vote_callback;
    typedef rpc_callback<raft::rpc::response_vote>  resp_vote_callback;

  public:
    server(unsigned int id):
      _me(std::make_shared<_node_t>(id)),
      _current_term(0)
    {
      _nodes.insert(std::make_pair(id, _me));
    }

  public:
    auto is_follower() const
    {
      return _fsm.state() == raft::fsm::state::follower;
    }
    auto is_candidate() const
    {
      return _fsm.state() == raft::fsm::state::candidate;
    }
    auto is_leader() const
    {
      return _fsm.state() == raft::fsm::state::leader;
    }

  public:
    auto id() const
    {
      return _me->id();
    }

  public:
    auto add_node(unsigned int id, bool voting = true)
    {
      auto node = std::make_shared<_node_t>(id);

      node->is_voting(voting);

      _nodes.insert(std::make_pair(id, node));

      return node;
    }

    auto remove_node(unsigned int id)
    {
      auto it = _nodes.find(id);

      if (it != _nodes.end())
        _nodes.erase(it);
    }

  public:
    auto election_start(req_vote_callback cb)
    {
      return _fsm(raft::fsm::event::election, [&]()
      {
        std::cout << "start election on " << _me->id() << std::endl;

        for (auto & i : _nodes)
          i.second->has_vote_for_me(false);

        _leader = nullptr;

        raft::rpc::request_vote rv =
        {
          .term = _current_term,
          .candidate_id = _me->id(),
          .last_log_idx = 0, // FIXME
          .last_log_term = 0, // FIXME
        };

        for (auto & i : _nodes)
          if (i.second != _me && i.second->is_voting())
            cb(*this, *i.second, rv);

        return true;
      });
    }

    auto recv_request_vote(raft::rpc::request_vote const & rv)
    {
      std::cout << _me->id() << " receive Request Vote" << std::endl;
    }

  private:
    raft::fsm _fsm;
    nodes_t _nodes;
    node_t _leader;

    node_t _me;
    unsigned int _current_term;

};

}; /** !raft  */

#endif /** !RAFT_SERVER_HH_  */

