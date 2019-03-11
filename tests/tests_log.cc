#include <gtest/gtest.h>

#include <raft/log.hh>

#define entry(id) {raft::entry_type_t::regular, 0, id, 0}

TEST(TestLog, NewIsEmpty)
{
  raft::log<int> l;

  EXPECT_EQ(l.count(), 0);
}

TEST(TestLog, AppendIsNotEmpty)
{
  raft::log<int, int, int> l;

  l.append(entry(42), [](auto e, auto idx)
  {
    EXPECT_EQ(e.id, 42);
    EXPECT_EQ(idx, 1);

    return raft::log_status_t::ok;
  });

  EXPECT_EQ(l.count(), 1);
}

TEST(TestLog, Print)
{
  raft::log<int> l;

  l.append(entry(42));

  std::cout << l << std::endl;
}

TEST(TestLog, GetAtIdx)
{
  raft::log<int> l;

  l.append(entry(42));
  l.append(entry(21));
  l.append(entry(51));

  EXPECT_EQ(l.count(), 3);
  EXPECT_EQ(l.at(1)->id, 42);
  EXPECT_EQ(l.at(2)->id, 21);
  EXPECT_EQ(l.at(3)->id, 51);
}

TEST(TestLog, GetAtIdxOutOfBound)
{
  raft::log<int> l;

  EXPECT_EQ(l.at(0), nullptr);
  EXPECT_EQ(l.at(1), nullptr);

  l.append(entry(42));
  EXPECT_EQ(l.at(2), nullptr);
}

TEST(TestLog, Delete)
{
  raft::log<int> l;

  l.append(entry(42));
  l.append(entry(21));
  l.append(entry(51));

  EXPECT_EQ(l.count(), 3);
  EXPECT_EQ(l.current(), 3);

  l.remove(3, [](auto e, auto idx){
    EXPECT_EQ(e.id, 51);
    EXPECT_EQ(idx, 3);
    return raft::log_status_t::ok;
  });
  EXPECT_EQ(l.count(), 2);
  EXPECT_EQ(l.at(3), nullptr);

  l.remove(2, [](auto e, auto idx){
    EXPECT_EQ(e.id, 21);
    EXPECT_EQ(idx, 2);
    return raft::log_status_t::ok;
  });
  EXPECT_EQ(l.count(), 1);
  EXPECT_EQ(l.at(2), nullptr);

  l.remove(1, [](auto e, auto idx){
    EXPECT_EQ(e.id, 42);
    EXPECT_EQ(idx, 1);
    return raft::log_status_t::ok;
  });
  EXPECT_EQ(l.count(), 0);
  EXPECT_EQ(l.at(1), nullptr);
}

TEST(TestLog, DeleteOnwards)
{
  raft::log<int> l;

  l.append(entry(42));
  l.append(entry(21));
  l.append(entry(51));

  EXPECT_EQ(l.count(), 3);
  l.remove(2);
  EXPECT_EQ(l.count(), 1);
  EXPECT_EQ(l.at(1)->id, 42);
  EXPECT_EQ(l.at(2), nullptr);
  EXPECT_EQ(l.at(3), nullptr);
}

TEST(TestLog, DeleteHandlesLogPopFailure)
{
  raft::log<int> l;

  auto failing_log_cb = [] (auto, auto) { return raft::log_status_t::fail; };

  l.append(entry(42));
  l.append(entry(21));
  l.append(entry(51));

  EXPECT_EQ(l.count(), 3);
  EXPECT_EQ(l.current(), 3);
  EXPECT_EQ(l.remove(2, failing_log_cb), raft::log_status_t::fail);

  EXPECT_EQ(l.count(), 3);
  EXPECT_EQ(l.tail()->id, 51);
}

TEST(TestLog, DeleteFailsForIndexZero)
{
  raft::log<int> l;

  l.append(entry(42));
  l.append(entry(21));
  l.append(entry(51));

  EXPECT_EQ(l.remove(0), raft::log_status_t::fail);
}

TEST(TestLog, Tail)
{
  raft::log<int> l;

  l.append(entry(42));
  l.append(entry(21));
  l.append(entry(51));

  EXPECT_EQ(l.count(), 3);
  EXPECT_EQ(l.tail()->id, 51);
}

TEST(TestLog, LoadFromSnapshot)
{
  raft::log<int> l;

  EXPECT_EQ(l.current(), 0);

  l.load(10, 5);
  EXPECT_EQ(l.current(), 10);
  EXPECT_EQ(l.count(), 0);
}

TEST(TestLog, LoadFromSnapshotclearsLogs)
{
  raft::log<int> l;

  l.append(entry(42));
  l.append(entry(21));

  EXPECT_EQ(l.count(), 2);
  EXPECT_EQ(l.current(), 2);

  l.load(10, 5);
  EXPECT_EQ(l.current(), 10);
  EXPECT_EQ(l.count(), 0);
}

TEST(TestLog, Poll)
{
  raft::log<int> l;

  l.append(entry(42));
  l.append(entry(21));
  l.append(entry(51));

  EXPECT_EQ(l.count(), 3);
  EXPECT_EQ(l.current(), 3);

  auto ret = l.poll([](auto e, auto idx)
  {
    EXPECT_EQ(e.id, 42);
    EXPECT_EQ(idx, 1);
    return raft::log_status_t::ok;
  });
  EXPECT_FALSE(raft::any(ret));
  EXPECT_EQ(l.count(), 2);
  EXPECT_EQ(l.current(), 3);
  EXPECT_EQ(l.at(1), nullptr);
  EXPECT_NE(l.at(2), nullptr);
  EXPECT_NE(l.at(3), nullptr);

  ret = l.poll([](auto e, auto idx)
  {
    EXPECT_EQ(e.id, 21);
    EXPECT_EQ(idx, 2);
    return raft::log_status_t::ok;
  });
  EXPECT_FALSE(raft::any(ret));
  EXPECT_EQ(l.count(), 1);
  EXPECT_EQ(l.current(), 3);
  EXPECT_EQ(l.at(1), nullptr);
  EXPECT_EQ(l.at(2), nullptr);
  EXPECT_NE(l.at(3), nullptr);

  ret = l.poll([](auto e, auto idx)
  {
    EXPECT_EQ(e.id, 51);
    EXPECT_EQ(idx, 3);
    return raft::log_status_t::ok;
  });
  EXPECT_FALSE(raft::any(ret));
  EXPECT_EQ(l.count(), 0);
  EXPECT_EQ(l.current(), 3);
  EXPECT_EQ(l.at(1), nullptr);
  EXPECT_EQ(l.at(2), nullptr);
  EXPECT_EQ(l.at(3), nullptr);
}

TEST(TestLog, DeleteAfterPolling)
{
  raft::log<int> l;

  // append
  l.append(entry(42));
  EXPECT_EQ(l.count(), 1);

  // poll
  auto ret = l.poll([](auto e, auto idx)
  {
    EXPECT_EQ(e.id, 42);
    EXPECT_EQ(idx, 1);
    return raft::log_status_t::ok;
  });
  EXPECT_FALSE(raft::any(ret));
  EXPECT_EQ(l.count(), 0);

  // append
  l.append(entry(21));
  EXPECT_EQ(l.count(), 1);

  // delete
  EXPECT_EQ(l.remove(1), raft::log_status_t::ok);
  EXPECT_EQ(l.count(), 0);
}
