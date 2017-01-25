#include <cassert>

#include <raft/server.hh>

int
main()
{
  raft::server<int> servers[3] = { 0, 1, 2 };

  raft::rpc::vote_request_t vreqs[3];
  raft::rpc::vote_response_t vresps[3];

  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      if (j != i)
        servers[i].add_node(j);

  servers[0].election_start([&vreqs](auto , auto , auto vreq) { vreqs[0] = vreq; });
  assert(servers[0].is_candidate());

  servers[1].recv_vote_request(vreqs[0], vresps[1]);
  servers[0].recv_vote_response(vresps[1], 1);
  assert(servers[0].is_candidate());

  servers[2].recv_vote_request(vreqs[0], vresps[2]);
  servers[0].recv_vote_response(vresps[2], 2);
  assert(servers[0].is_leader());
}
