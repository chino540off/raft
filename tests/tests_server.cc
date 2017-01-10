#include <raft/server.hh>

int
main()
{
  raft::server<int> server[3] = { 0, 1, 2};

  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      if (j != i)
        server[i].add_node(j);

  server[0].election_start([&](auto s, auto n, auto rv)
  {
    std::cout << s.id() << " sends request_vote to " << n.id() << std::endl;
    server[n.id()].recv_request_vote(rv);
  });
}
