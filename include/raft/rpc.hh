#ifndef RAFT_RPC_HH_
# define RAFT_RPC_HH_

namespace raft {
namespace rpc {

struct vote_request_t
{
  unsigned int term;
  unsigned int candidate_id;
  unsigned int last_log_idx;
  unsigned int last_log_term;
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

struct vote_response_t
{
  unsigned int term;
  int vote_granted;
};

template <typename ostream>
ostream &
operator<<(ostream & os, vote_response_t const & vresp)
{
  os << "VoteResponse"
    << " (term: " << vresp.term
    << ", vote: " << ((vresp.vote_granted == 1) ? "granted" :
                     (vresp.vote_granted == 0) ? "not granted" : "unknown")
    << ")";
  return os;
}

} /** !rpc  */
} /** !raft  */

#endif /** !RAFT_RPC_HH_  */

