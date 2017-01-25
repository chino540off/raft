#ifndef RAFT_LOG_HH_
# define RAFT_LOG_HH_

namespace raft {

struct entry
{
  unsigned int term;
  unsigned int id;
};

} /** !raft  */

#endif /** !RAFT_LOG_HH_  */

