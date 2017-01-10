#ifndef RAFT_RPC_HH_
# define RAFT_RPC_HH_

namespace raft {
namespace rpc {

struct request_vote
{
  unsigned int term;
  unsigned int candidate_id;
  unsigned int last_log_idx;
  unsigned int last_log_term;
};

struct response_vote
{
  unsigned term;
  bool vote_granted;
};

}; /** !rpc  */
}; /** !raft  */

#endif /** !RAFT_RPC_HH_  */

