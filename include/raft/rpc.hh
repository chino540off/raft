#ifndef RAFT_RPC_HH_
#define RAFT_RPC_HH_

#include <vector>

#include <raft/log.hh>

namespace raft
{
namespace rpc
{

enum class vote_t
{
  node_not_found = -2,
  err = -1,
  not_granted = 0,
  granted = 1,
};

template <typename ostream>
ostream &
operator<<(ostream & os, vote_t const & v)
{
  switch (v)
  {
    case vote_t::granted:
      return os << "granted", os;
    case vote_t::not_granted:
      return os << "not_granted", os;
    case vote_t::err:
      return os << "error", os;
    case vote_t::node_not_found:
      return os << "node_not_found", os;
    default:
      assert(false);
  }
}

/** Vote request message.
 * Sent to nodes when a server wants to become leader.
 * This message could force a leader/candidate to become a follower. */
template <typename term_t, typename index_t, typename node_id_t>
struct vote_request_t
{
  /** currentTerm, to force other leader/candidate to step down */
  term_t term;

  /** candidate requesting vote */
  node_id_t candidate_id;

  /** index of candidate's last log entry */
  index_t last_log_idx;

  /** term of candidate's last log entry */
  term_t last_log_term;
};

template <typename ostream, typename term_t, typename index_t, typename node_id_t>
ostream &
operator<<(ostream & os, vote_request_t<term_t, index_t, node_id_t> const & msg)
{
  return os << "{"
            << "\"candidate_id\": " << msg.candidate_id << ", "
            << "\"term\": " << msg.term << ", "
            << "\"last_idx\": " << msg.last_log_idx << ", "
            << "\"last_term\": " << msg.last_log_term << "}",
         os;
}

/** Vote response message.
 * Indicates if node has accepted the server's vote request. */
template <typename term_t>
struct vote_response_t
{
  /** currentTerm, for candidate to update itself */
  term_t term;

  /** true means candidate received vote */
  vote_t vote;
};

template <typename ostream, typename term_t>
ostream &
operator<<(ostream & os, vote_response_t<term_t> const & msg)
{
  return os << "{"
            << "\"term\": " << msg.term << ", "
            << "\"vote\": \"" << msg.vote << "\"}",
         os;
}

/** Appendentries message.
 * This message is used to tell nodes if it's safe to apply entries to the FSM.
 * Can be sent without any entries as a keep alive message.
 * This message could force a leader/candidate to become a follower. */
template <typename T, typename term_t, typename index_t, typename index_id_t>
struct appendentries_request_t
{
  /** currentTerm, to force other leader/candidate to step down */
  term_t term;

  /** the index of the log just before the newest entry for the node who
   * receives this message */
  index_t prev_log_idx;

  /** the term of the log just before the newest entry for the node who
   * receives this message */
  term_t prev_log_term;

  /** the index of the entry that has been appended to the majority of the
   * cluster. Entries up to this index will be applied to the FSM */
  index_t leader_commit;

  /** array of entries within this message */
  std::vector<entry<T, term_t, index_id_t>> entries;
};

template <typename ostream, typename T, typename term_t, typename index_t, typename index_id_t>
ostream &
operator<<(ostream & os, appendentries_request_t<T, term_t, index_t, index_id_t> const & msg)
{
  os << "{"
     << "\"term\": " << msg.term << ", "
     << "\"prev_log_idx\": " << msg.prev_log_idx << ", "
     << "\"prev_log_term\": " << msg.prev_log_term << ", "
     << "\"leader_commit\": " << msg.leader_commit << ", "
     << "\"entries\": [";

  auto first = true;
  for (auto & entry : msg.entries)
  {
    if (!first)
      os << ", ";
    first = false;

    os << entry;
  }

  return os << "]}", os;
}

/** Appendentries response message.
 * Can be sent without any entries as a keep alive message.
 * This message could force a leader/candidate to become a follower. */
template <typename term_t, typename index_t>
struct appendentries_response_t
{
  /** currentTerm, to force other leader/candidate to step down */
  term_t term;

  /** true if follower contained entry matching prevLogidx and prevLogTerm */
  bool success;

  /* Non-Raft fields follow: */
  /* Having the following fields allows us to do less book keeping in
   * regards to full fledged RPC */

  /** If success, this is the highest log IDX we've received and appended to
   * our log; otherwise, this is the our currentIndex */
  index_t current_idx;

  /** The first idx that we received within the appendentries message */
  index_t first_idx;
};

template <typename ostream, typename term_t, typename index_t>
ostream &
operator<<(ostream & os, appendentries_response_t<term_t, index_t> const & msg)
{
  return os << "{"
            << "\"term\": " << msg.term << ", "
            << "\"success\": " << msg.success << ", "
            << "\"current_idx\": " << msg.current_idx << ", "
            << "\"first_idx\": " << msg.first_idx << "}",
         os;
}

} /** !rpc  */
} /** !raft  */

#endif /** !RAFT_RPC_HH_  */
