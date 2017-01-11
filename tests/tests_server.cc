#include <raft/server.hh>

int
main()
{
  raft::server<int> servers[3] = { 0, 1, 2};

  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      if (j != i)
        servers[i].add_node(j);

  servers[0].election_start([&](auto server, auto node, auto vreq)
  {
    std::cout << server << " sends " << vreq << " to " << node << std::endl;

    raft::rpc::vote_response vresp;

    servers[node.id()].recv_request_vote(vreq, vresp);
  });
}
