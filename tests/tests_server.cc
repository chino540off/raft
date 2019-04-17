#include <gtest/gtest.h>

#include <raft/server.hh>

TEST(TestServer, ServerVotedForRecordsWhoWeVotedFor)
{
  raft::server<int> s;

  s.node_add(2);
  s.vote_for(s.node_get(2));

  EXPECT_EQ(s.voted_for()->id(), 2);
}

TEST(TestServer, GetMyNode)
{
  raft::server<int> s;

  auto me = s.node_add(1, true);

  EXPECT_EQ(s.my_node(), me);
}

TEST(TestServer, IndexStartsAt1)
{
  raft::server<int> s;

  EXPECT_EQ(s.current_index(), 0);

  s.append({raft::entry_type_t::regular, 0, 1, 0});

  EXPECT_EQ(s.current_index(), 1);
}

TEST(TestServer, CurrentTermDefaultsTo0)
{
  raft::server<int> s;

  EXPECT_EQ(s.current_term(), 0);
}

TEST(TestServer, CurrentTermSetsTerm)
{
  raft::server<int> s;

  s.current_term(5);
  EXPECT_EQ(s.current_term(), 5);
}

TEST(TestServer, VotingResultsInVoting)
{
  raft::server<int> s;

  s.node_add(1);
  s.node_add(2);

  s.vote_for(s.node_get(1));
  EXPECT_EQ(s.voted_for()->id(), 1);

  s.vote_for(s.node_get(2));
  EXPECT_EQ(s.voted_for()->id(), 2);
}

TEST(TestServer, AddNodeMakesNonVotingNodeVoting)
{
  raft::server<int> s;

  auto n = s.node_non_voting_add(1);
  EXPECT_FALSE(n->is_voting());

  s.node_add(1);
  EXPECT_TRUE(n->is_voting());

  EXPECT_EQ(s.node_count(), 1);
}

TEST(TestServer, AddNodeWithAlreadyExistingIdIsNotAllowed)
{
  raft::server<int> s;

  s.node_add(1);
  s.node_add(2);

  EXPECT_EQ(s.node_add(1), nullptr);
  EXPECT_EQ(s.node_add(2), nullptr);
}

TEST(TestServer, AddNonVotingNodeWithAlreadyExistingIdIsNotAllowed)
{
  raft::server<int> s;

  s.node_non_voting_add(1);
  s.node_non_voting_add(2);

  EXPECT_EQ(s.node_non_voting_add(1), nullptr);
  EXPECT_EQ(s.node_non_voting_add(2), nullptr);
}

TEST(TestServer, AddNonVotingNodeWithAlreadyExistingVotingIdIsNotAllowed)
{
  raft::server<int> s;

  s.node_add(1);
  s.node_add(2);

  EXPECT_EQ(s.node_non_voting_add(1), nullptr);
  EXPECT_EQ(s.node_non_voting_add(2), nullptr);
}

TEST(TestServer, RemoveNode)
{
  raft::server<int> s;

  s.node_add(1);
  s.node_add(2);
  EXPECT_NE(s.node_get(1), nullptr);
  EXPECT_NE(s.node_get(2), nullptr);

  s.node_remove(1);
  EXPECT_EQ(s.node_get(1), nullptr);
  EXPECT_NE(s.node_get(2), nullptr);

  s.node_remove(2);
  EXPECT_EQ(s.node_get(1), nullptr);
  EXPECT_EQ(s.node_get(2), nullptr);
}

TEST(TestServer, ElectionStartIncrementsTerm)
{
  raft::server<int> s;

  s.current_term(1);
  s.election_start();
  EXPECT_EQ(s.current_term(), 2);
}

TEST(TestServer, ServerStartsAsFollower)
{
  raft::server<int> s;

  EXPECT_TRUE(s.state() == raft::state_t::follower);
}

TEST(TestServer, AppendEntryIsRetrievable)
{
  raft::server<int> s;

  s.current_term(5);
  s.append({raft::entry_type_t::regular, 142, 121, 151});
  s.append({raft::entry_type_t::regular, 242, 221, 251});

  EXPECT_NE(s.get(1), nullptr);
  EXPECT_EQ(s.get(1)->term, 142);
  EXPECT_EQ(s.get(1)->id, 121);
  EXPECT_EQ(s.get(1)->elt, 151);

  EXPECT_NE(s.get(2), nullptr);
  EXPECT_EQ(s.get(2)->term, 242);
  EXPECT_EQ(s.get(2)->id, 221);
  EXPECT_EQ(s.get(2)->elt, 251);
}

TEST(TestServer, EntryIsRetrieveableUsingIdx)
{
  raft::server<std::string> s;

  decltype(s)::entry_t e1{raft::entry_type_t::regular, 1, 1, "str1"};
  decltype(s)::entry_t e2{raft::entry_type_t::regular, 1, 2, "str2"};

  s.append(e1);
  s.append(e2);

  auto e = s.get(2);
  EXPECT_NE(e, nullptr);
  EXPECT_EQ(e->elt, "str2");
}

TEST(TestServer, WontApplyEntryIfWeDontHaveEntryToApply)
{
  raft::server<int> s;

  s.commit_index(0);
  s.last_applied_index(0);

  s.apply_entry();

  EXPECT_EQ(s.last_applied_index(), 0);
  EXPECT_EQ(s.commit_index(), 0);
}

TEST(TestServer, SetsNumNodes)
{
  raft::server<int> s;

  s.node_add(1, true);
  s.node_add(2);

  EXPECT_EQ(s.node_count(), 2);
}

TEST(TestServer, CantGetNodeWeDontHave)
{
  raft::server<int> s;

  s.node_add(1, true);
  s.node_add(2);

  EXPECT_EQ(s.node_get(0), nullptr);
  EXPECT_NE(s.node_get(1), nullptr);
  EXPECT_NE(s.node_get(2), nullptr);
  EXPECT_EQ(s.node_get(3), nullptr);
}

TEST(TestServer, VotesAreMajorityIsTrue)
{
  raft::server<int> s;

  EXPECT_FALSE(s.is_majority(3, 1));
  EXPECT_TRUE(s.is_majority(3, 2));
  EXPECT_FALSE(s.is_majority(5, 2));
  EXPECT_TRUE(s.is_majority(5, 3));
  EXPECT_FALSE(s.is_majority(1, 2));
}

TEST(TestServer, RecvVoteResponseDontIncreaseVotesForMeWhenNotGranted)
{
  raft::server<int> s;

  s.node_add(1, true);
  s.node_add(2);
  s.current_term(1);

  EXPECT_EQ(s.num_voting_nodes_for_me(), 0);

  auto ret = s.recv_vote_response(s.node_get(2), {1, raft::rpc::vote_t::not_granted});

  EXPECT_TRUE(ret == raft::status_t::ok);
  EXPECT_EQ(s.num_voting_nodes_for_me(), 0);
}

TEST(TestServer, RecvVoteResponseDontIncreaseVotesForMeTermIsNotEqual)
{
  raft::server<int> s;

  s.node_add(1, true);
  s.node_add(2);
  s.current_term(3);

  EXPECT_EQ(s.num_voting_nodes_for_me(), 0);

  auto ret = s.recv_vote_response(s.node_get(2), {2, raft::rpc::vote_t::granted});

  EXPECT_TRUE(ret == raft::status_t::ok);
  EXPECT_EQ(s.num_voting_nodes_for_me(), 0);
}

TEST(TestServer, RecvVoteResponseDontIncreaseVotesForMe)
{
  raft::server<int> s;

  s.node_add(1, true);
  s.node_add(2);
  s.current_term(1);

  s.become_candidate();
  EXPECT_EQ(s.current_term(), 2);
  EXPECT_EQ(s.num_voting_nodes_for_me(), 1);

  auto ret = s.recv_vote_response(s.node_get(2), {2, raft::rpc::vote_t::granted});

  EXPECT_TRUE(ret == raft::status_t::ok);
  EXPECT_EQ(s.num_voting_nodes_for_me(), 2);
}

TEST(TestServer, RecvVoteRequestResponseMustBeCandiddateToReceive)
{
  raft::server<int> s;

  s.node_add(1, true);
  s.node_add(2);
  s.current_term(1);

  EXPECT_EQ(s.num_voting_nodes_for_me(), 0);

  s.become_leader();

  auto ret = s.recv_vote_response(s.node_get(2), {1, raft::rpc::vote_t::granted});

  EXPECT_TRUE(ret == raft::status_t::ok);
  EXPECT_EQ(s.num_voting_nodes_for_me(), 0);
}

TEST(TestServer, RecvVoteRequestResponseReplyFalseIfTermLessThanCurrentTerm)
{
  raft::server<int> s;

  s.node_add(1, true);
  s.node_add(2);
  s.current_term(2);

  decltype(s)::vote_response_t resp;

  auto ret = s.recv_vote_request(s.node_get(2), {1, 0, 0, 0}, resp);
  EXPECT_TRUE(ret == raft::status_t::ok);
  EXPECT_TRUE(resp.vote == raft::rpc::vote_t::not_granted);
}

TEST(TestServer, LeaderRecvVoteRequestDoesNotStepDown)
{
  raft::server<int> s;

  auto n1 = s.node_add(1, true);
  auto n2 = s.node_add(2);
  s.current_term(1);
  s.vote_for(s.node_get(1));

  s.become_leader();

  decltype(s)::vote_response_t resp;
  auto ret = s.recv_vote_request(s.node_get(2), {1, 0, 0, 0}, resp);

  EXPECT_TRUE(ret == raft::status_t::ok);
  EXPECT_EQ(s.leader(), n1);
}

TEST(TestServer, RecvVoteRequestReplyTrueIfTermGreaterThanOrEqualToCurrentTerm)
{
  raft::server<int> s;

  auto n1 = s.node_add(1, true);
  auto n2 = s.node_add(2);
  s.current_term(1);

  decltype(s)::vote_response_t resp;
  auto ret = s.recv_vote_request(s.node_get(2), {2, 2, 1, 0}, resp);

  EXPECT_TRUE(ret == raft::status_t::ok);
  EXPECT_TRUE(resp.vote == raft::rpc::vote_t::granted);
}

TEST(TestServer, RecvRequestVoteRestTimeout)
{
  raft::server<int> s;

  auto n1 = s.node_add(1, true);
  auto n2 = s.node_add(2);
  s.current_term(1);

  s.election_timeout(1000ms);
  s.periodic(900ms);

  decltype(s)::vote_response_t resp;
  auto ret = s.recv_vote_request(s.node_get(2), {2, 2, 1, 0}, resp);

  EXPECT_TRUE(ret == raft::status_t::ok);
  EXPECT_TRUE(resp.vote == raft::rpc::vote_t::granted);
  EXPECT_EQ(s.elapsed_timeout(), 0ms);
}

TEST(TestServer, RecvRequestVoteCandidateStepDownIfTermIsHigherThanCurrentTerm)
{
  raft::server<int> s;

  auto n1 = s.node_add(1, true);
  auto n2 = s.node_add(2);
  s.become_candidate();
  s.current_term(1);

  EXPECT_EQ(s.voted_for(), n1);

  decltype(s)::vote_response_t resp;
  auto ret = s.recv_vote_request(s.node_get(2), {2, 2, 1, 0}, resp);

  EXPECT_TRUE(ret == raft::status_t::ok);
  EXPECT_TRUE(s.is_follower());
  EXPECT_EQ(s.current_term(), 2);
  EXPECT_EQ(s.voted_for(), n2);
}

TEST(TestServer, RecvRequestVoteDependsOnCandidateId)
{
  raft::server<int> s;

  auto n1 = s.node_add(1, true);
  auto n2 = s.node_add(2);
  s.become_candidate();
  s.current_term(1);

  EXPECT_EQ(s.voted_for(), n1);

  decltype(s)::vote_response_t resp;
  s.recv_vote_request(nullptr, {2, 3, 1, 0}, resp);
  EXPECT_TRUE(s.is_follower());
  EXPECT_EQ(s.current_term(), 2);
  EXPECT_EQ(s.voted_for()->id(), 3);
}
