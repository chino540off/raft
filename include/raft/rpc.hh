#ifndef RAFT_RPC_HH_
# define RAFT_RPC_HH_

namespace raft {
namespace rpc {

struct vote_request_t
{
  unsigned long int term;
  unsigned long int candidate_id;
  unsigned long int last_log_idx;
  unsigned long int last_log_term;
};

template <typename ostream>
ostream &
operator<<(ostream & os, vote_request_t const & vreq)
{
  os << "VoteRequest"
    << " (candidate_id: " << vreq.candidate_id
    << ", term: " << vreq.term
    << ", last_idx: " << vreq.last_log_idx
    << ", last_term: " << vreq.last_log_term
    << ")";
  return os;
}

enum class vote_t
{
  error = -1,
  not_granted = 0,
  granted = 1,
};

template <typename ostream>
ostream &
operator<<(ostream & os, vote_t const & vote)
{
  switch (vote)
  {
    case vote_t::not_granted: os << "not granted"; break;
    case vote_t::granted:     os << "granted"; break;
    default:                  os << "error"; break;
  }
  return os;
}

struct vote_response_t
{
  unsigned long int term;
  vote_t vote;
};

template <typename ostream>
ostream &
operator<<(ostream & os, vote_response_t const & vresp)
{
  os << "VoteResponse"
    << " (term: " << vresp.term
    << ", vote: " << vresp.vote
    << ")";
  return os;
}

} /** !rpc  */
} /** !raft  */

#endif /** !RAFT_RPC_HH_  */

