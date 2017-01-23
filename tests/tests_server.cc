#include <cassert>
#include <queue>
#include <thread>

#include <raft/server.hh>

enum class msg_type_t
{
  VOTE_REQUEST,
  VOTE_RESPONSE,
};

struct msg_t
{
  msg_t(raft::rpc::vote_request_t const & msg, unsigned int _from, unsigned int _to):
    type(msg_type_t::VOTE_REQUEST),
    from(_from),
    to(_to),
    vote_request(msg) { }

  msg_t(raft::rpc::vote_response_t const & msg, unsigned int _from, unsigned int _to):
    type(msg_type_t::VOTE_RESPONSE),
    from(_from),
    to(_to),
    vote_response(msg) { }

  msg_type_t type;
  unsigned int from;
  unsigned int to;

  union
  {
    raft::rpc::vote_request_t   vote_request;
    raft::rpc::vote_response_t  vote_response;
  };
};

using namespace std::chrono_literals;

int
main()
{
  raft::server<int> servers[3] = { 0, 1, 2 };
  std::queue<msg_t> msg_queue[3];
  std::thread threads[3];

  for (unsigned int i = 0; i < 3; ++i)
  {
    for (unsigned int j = 0; j < 3; ++j)
      if (j != i)
        servers[i].add_node(j);

    threads[i] = std::thread([&msg_queue, &servers, i]()
    {
      std::cout << "Run thread " << i << std::endl;

      while (true)
      {
        while (!msg_queue[i].empty())
        {
          auto msg = msg_queue[i].front();
          msg_queue[i].pop();

          switch (msg.type)
          {
            case msg_type_t::VOTE_REQUEST:
              {
                raft::rpc::vote_response_t vresp;

                servers[i].recv_vote_request(msg.vote_request, vresp);
                msg_queue[msg.from].push(msg_t(vresp, msg.to, msg.from));
              }
              break;

            case msg_type_t::VOTE_RESPONSE:
              {
                servers[i].recv_vote_response(msg.vote_response, msg.from);
              }
              break;

            default:
              assert(false);
          }
        }

        std::this_thread::sleep_for(100ms);
      }
    });
  }

  servers[0].election_start([&msg_queue](auto server, auto node, auto vreq)
  {
    std::cout << server << " sends " << vreq << " to " << node << std::endl;

    msg_queue[node.id()].push(msg_t(vreq, server.id(), node.id()));
  });

  for (unsigned int i = 0; i < 3; ++i)
    threads[i].join();
}
