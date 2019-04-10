#include <gtest/gtest.h>

#include <raft/node.hh>

TEST(TestNode, IsVotingByDefault)
{
  raft::node<int> n(1);

  EXPECT_TRUE(n.is_voting());
}

TEST(TestNode, SetNextIdx)
{
  raft::node<int> n(1);

  n.next_idx(3);
  EXPECT_EQ(n.next_idx(), 3);
}

#define check_flag(__n, __name)                                                                    \
  do                                                                                               \
  {                                                                                                \
    __n.__name(false);                                                                             \
    EXPECT_FALSE(__n.__name());                                                                    \
                                                                                                   \
    __n.__name(true);                                                                              \
    EXPECT_TRUE(__n.__name());                                                                     \
  } while (0)

TEST(TestNode, Flags)
{
  raft::node<int> n(1);

  // has vote for me
  check_flag(n, has_vote_for_me);

  // is voting
  check_flag(n, is_voting);

  // has sufficient log
  check_flag(n, has_sufficient_logs);

  // is active
  check_flag(n, is_active);

  // is voting commited
  check_flag(n, is_voting_commited);

  // is addition commited
  check_flag(n, is_addition_commited);
}

TEST(TestNode, Print)
{
  raft::node<int> n(1);

  std::cout << n << std::endl;
}
