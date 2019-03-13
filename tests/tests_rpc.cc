#include <gtest/gtest.h>

#include <raft/rpc.hh>

TEST(TestRPC, VoteRequestPrint)
{
  raft::rpc::vote_request_t<int, int, int> msg{1, 2, 3, 4};

  std::cout << msg << std::endl;
}

TEST(TestRPC, VoteResponsePrint)
{
  raft::rpc::vote_response_t<int> msg{1, raft::rpc::vote_t::granted};

  std::cout << msg << std::endl;
}

TEST(TestRPC, AppendEntriesRequestPrint)
{
  raft::rpc::appendentries_request_t<std::string, int, int, int> msg {
      3, 42, 2, 3,
      {
        {raft::entry_type_t::regular, 2, 42, "hello"},
        {raft::entry_type_t::user, 2, 43, "world"},
      }
  };

  std::cout << msg << std::endl;
}

TEST(TestRPC, AppendEntriesResponsePrint)
{
  raft::rpc::appendentries_response_t<int, int> msg{3, true, 2, 3};

  std::cout << msg << std::endl;
}
