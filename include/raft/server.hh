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
    typedef std::shared_ptr<raft::node<T>> node_t;
    typedef std::map<unsigned int, node_t> nodes_t;

    template <typename rpc_t>
    using rpc_callback = std::function<void(server<T> const &,
                                            raft::node<T> const &,
                                            rpc_t const &)>;

    typedef rpc_callback<raft::rpc::vote_request>   vote_req_callback;

  public:
    server(unsigned int id):
      _me(std::make_shared<raft::node<T>>(id)),
      _current_term(0)
    {
      _nodes.insert(std::make_pair(id, _me));
    }

  public:
    auto is_follower() const
    {
      return _fsm.state() == raft::state::follower;
    }
    auto is_candidate() const
    {
      return _fsm.state() == raft::state::candidate;
    }
    auto is_leader() const
    {
      return _fsm.state() == raft::state::leader;
    }

  public:
    auto id() const
    {
      return _me->id();
    }

  public:
    auto add_node(unsigned int id, bool voting = true)
    {
      auto node = std::make_shared<raft::node<T>>(id);

      node->is_voting(voting);

      _nodes.insert(std::make_pair(id, node));

      return node;
    }

    auto get_node(unsigned int id)
    {
      auto it = _nodes.find(id);

      return it->second;
    }

    auto remove_node(unsigned int id)
    {
      auto it = _nodes.find(id);

      if (it != _nodes.end())
        _nodes.erase(it);
    }

  private:
    bool should_grant_vote(raft::rpc::vote_request const & vreq)
    {
      if (!_me->is_voting())
        return false;

      if (vreq.term < _current_term)
        return false;

      // FIXME: check if already voted
      // FIXME: check logs indexes

      return true;
    }

  public:
    auto election_start(vote_req_callback cb)
    {
      std::cout << "start election on " << _me->id() << std::endl;

      return _fsm(raft::event::election, [&]()
      {
        for (auto & i : _nodes)
          i.second->has_vote_for_me(false);

        _leader = nullptr;

        raft::rpc::vote_request vreq = { _current_term, _me->id(), 0, /* FIXME */ 0, /* FIXME */ };

        for (auto & i : _nodes)
          if (i.second != _me && i.second->is_voting())
            cb(*this, *i.second, vreq);

        return true;
      });
    }

    auto recv_request_vote(raft::rpc::vote_request const &  vreq,
                           raft::rpc::vote_response &       vresp)
    {
      std::cout << *this << " receives " << vreq << std::endl;

      auto node = get_node(vreq.candidate_id);

      if (_current_term < vreq.term)
      {
        _current_term = vreq.term;
        _fsm(raft::event::high_term, []()
        {
          std::cout << "Higher term: become follower" << std::endl;
          return true;
        });
      }

      if (should_grant_vote(vreq))
      {
        vresp.vote_granted = 1;
        // FIXME: vote for vreq.candidate_id
      }
      else
      {
        if (node == nullptr)
        {
          vresp.vote_granted = -1;
        }
        else
        {
          vresp.vote_granted = 0;
        }
      }

      vresp.term = _current_term;

      std::cout << *this << " replies " << vresp << std::endl;
    }

  private:
    raft::fsm _fsm;
    nodes_t _nodes;
    node_t _leader;
    node_t _vote_for;

    node_t _me;
    unsigned int _current_term;

};

template <typename ostream, typename T>
ostream &
operator<<(ostream & os, server<T> const & server)
{
  os << "Server(" << server.id() << ")";
  return os;
}

} /** !raft  */

#endif /** !RAFT_SERVER_HH_  */

