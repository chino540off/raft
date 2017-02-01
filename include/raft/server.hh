#ifndef RAFT_SERVER_HH_
# define RAFT_SERVER_HH_

# include <cassert>
# include <map>

# include <utils/logger.hh>

using loglevel = utils::logger::level;

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

    typedef rpc_callback<raft::rpc::vote_request_t>   vote_req_callback;

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

    /**
     * @brief
     *
     * @return
     */
    auto is_majority() const
    {
      unsigned int votes = 0;
      unsigned int voting_nodes = 0;

      for (auto & it : _nodes)
      {
        auto node = it.second;

        if (node == _me)
          continue;

        if (node->is_voting())
        {
          ++voting_nodes;
          if (node->has_vote_for_me())
          {
            ++votes;
          }
        }
      }

      _log(loglevel::DEBUG, votes, "/", voting_nodes, "votes");

      return ((voting_nodes / 2) + 1) <= votes;
    }

    auto already_vote() const
    {
      return _vote_for != nullptr;
    }

    auto vote_for(node_t node)
    {
      _vote_for = node;
    }

  public:
    auto id() const
    {
      return _me->id();
    }

  public:
    /**
     * @brief
     *
     * @tparam ostream
     * @param os
     *
     * @return
     */
    template <typename ostream>
    ostream & print(ostream & os) const
    {
      os << "Server(id: " << id()
         << ", term: " << _current_term
         << ", state: " << _fsm.state()
         <<  ")";
      return os;
    }


  public:
    /**
     * @brief
     *
     * @param id
     * @param voting
     *
     * @return
     */
    auto add_node(unsigned int id, bool voting = true)
    {
      auto node = std::make_shared<raft::node<T>>(id);

      node->is_voting(voting);

      _nodes.insert(std::make_pair(id, node));

      return node;
    }

    /**
     * @brief
     *
     * @param id
     *
     * @return
     */
    auto get_node(unsigned int id)
    {
      auto it = _nodes.find(id);

      return it->second;
    }

    /**
     * @brief
     *
     * @param id
     *
     * @return
     */
    auto remove_node(unsigned int id)
    {
      auto it = _nodes.find(id);

      if (it != _nodes.end())
        _nodes.erase(it);
    }

  private:
    /**
     * @brief
     *
     * @param vreq
     *
     * @return
     */
    bool should_grant_vote(raft::rpc::vote_request_t const & vreq)
    {
      if (!_me->is_voting())
        return false;

      if (vreq.term < _current_term)
        return false;

      if (already_vote())
        return false;

      // FIXME: check logs indexes

      return true;
    }

  public:
    /**
     * @brief
     *
     * @param cb
     *
     * @return
     */
    auto election_start(vote_req_callback cb)
    {
      _log(loglevel::DEBUG, "start election on ", _me->id());

      auto ret = _fsm(raft::event::election, [&]()
      {
        for (auto & i : _nodes)
          i.second->has_vote_for_me(false);

        vote_for(_me);

        _leader = nullptr;

        raft::rpc::vote_request_t vreq = {
          _current_term,
          _me->id(),
          0 /* FIXME */,
          0 /* FIXME */,
        };

        for (auto & i : _nodes)
          if (i.second != _me && i.second->is_voting())
            cb(*this, *i.second, vreq);

        return true;
      });

      assert(_fsm.state() == raft::state::candidate);
      return ret;
    }

    /**
     * @brief
     *
     * @param vreq
     * @param vresp
     *
     * @return
     */
    auto recv_vote_request(raft::rpc::vote_request_t const & vreq,
                           raft::rpc::vote_response_t & vresp)
    {
      _log(loglevel::INFO, *this, " receives ", vreq);

      auto node = get_node(vreq.candidate_id);

      if (_current_term < vreq.term)
      {
        _current_term = vreq.term;
        _fsm(raft::event::high_term, [this]()
        {
          _log(loglevel::INFO, "Higher term: become follower");
          return true;
        });
      }

      if (should_grant_vote(vreq))
      {
        vresp.vote = rpc::vote_t::granted;
        _leader = nullptr;
        _vote_for = node;
      }
      else
      {
        vresp.vote = (node != nullptr) ? rpc::vote_t::not_granted : rpc::vote_t::error;
      }

      vresp.term = _current_term;

      _log(loglevel::INFO, *this, " replies ", vresp);
    }

    /**
     * @brief
     *
     * @param vresp
     * @param from
     *
     * @return
     */
    auto recv_vote_response(raft::rpc::vote_response_t const & vresp, unsigned int from)
    {
      _log(loglevel::INFO, *this, "receives", vresp);

      if (!is_candidate())
      {
        DEBUG(_log, *this, "is not candidate");
        return;
      }
      else if (_current_term < vresp.term)
      {
        _current_term = vresp.term;
        _fsm(raft::event::high_term, [this]()
        {
          _log(loglevel::INFO, "Higher term: become follower");
          return true;
        });
      }
      else if (_current_term != vresp.term)
      {
        return;
      }

      switch (vresp.vote)
      {
        case rpc::vote_t::granted:
        {
          auto node = get_node(from);
          if (node)
            node->has_vote_for_me(true);

          if (is_majority())
            _fsm(raft::event::majority, [this]()
            {
              _log(loglevel::INFO, "Majority: become leader");
              return true;
            });
          break;
        }

        case rpc::vote_t::not_granted:
        case rpc::vote_t::error:
          break;
      }
    }

  private:
    raft::fsm _fsm;
    nodes_t _nodes;
    node_t _leader;
    node_t _vote_for;

    node_t _me;
    unsigned int _current_term;

    mutable utils::logger::Logger _log;
};

template <typename ostream, typename T>
ostream &
operator<<(ostream & os, server<T> const & server)
{
  return server.print(os);
}

} /** !raft  */

#endif /** !RAFT_SERVER_HH_  */

