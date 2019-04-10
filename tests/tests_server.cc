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

// TEST(TestServer, AppendEntryIsRetrievable)
//{
//  raft::server<int> s;
//
//}
