#include <raft/server.hh>

namespace raft {


template <typename T,
          typename node_id_t,
          typename term_t_,
          typename index_id_t_>
bool
server<T, node_id_t, term_t_, index_id_t_>::should_grant_vote(std::shared_ptr<node_t> node,
                                                              vote_request_t const & req)
{
  if (!node->is_voting())
    return false;

  if (req.term < current_term())
    return false;

  if (vote_for() != nullptr)
    return false;

  index_id_t idx = current_index();
  if (idx == 0)
    return true;

  std::shared_ptr<entry_t> entry = log_.get(req. idx);
  term_t entry_term;

  if (entry)
    entry_term = entry->term;
  else // FIXME if snapshot_last_term
    return false;

  if (entry_term < req.last_log_term)
    return true;

  if (req.last_log_term == entry_term
      && idx <= req.last_log_idx)
    return true;

  return false;
}

template <typename T,
          typename node_id_t,
          typename term_t_,
          typename index_id_t_>
status_t
server<T, node_id_t, term_t_, index_id_t_>::recv_vote_request(std::shared_ptr<node_t> node,
                                                              vote_request_t const & req,
                                                              vote_response_t & resp)
{
  status_t ret =status_t::ok;

  if (node == nullptr)
    node = node_get(req.candidate_id);

  /* Reject request if we have a leader */
  if (leader_ != nullptr
      && leader_ != node
      && timeout_elasped_ < election_timeout_)
  {
    resp.vote = rpc::vote_t::not_granted;
    goto end;
  }

  if (current_term() < req.term)
  {
    ret = current_term(req.term);
    if (any(ret))
    {
      resp.vote = rpc::vote_t::not_granted;
      goto end;
    }

    // FIXME -> become_follower
    fsm_(event_t::high_term);
    leader_ = nullptr;
  }

  if (should_grant_vote(req))
  {
    /* It shouldn't be possible for a leader or candidate to grant a vote
     * Both states would have voted for themselves */
    assert(!(is_leader() || is_candidate()));

    ret = vote_for(node);
    if (any(ret))
      resp.vote = rpc::vote_t::not_granted;
    else
      resp.vote = rpc::vote_t::granted;

    leader_ = nullptr;
    timeout_elasped_ = 0;
  }
  else
  {
    if (node == nullptr)
    {
      resp.vote = rpc::vote_t::node_not_found;
    }
    else
      resp.vote = rpc::vote_t::not_granted;
  }

end:
  resp.term = current_term();
  return ret;
}

} /** !raft  */
