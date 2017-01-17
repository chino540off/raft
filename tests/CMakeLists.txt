CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(raft-tests)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -Wextra -pedantic -pthread -std=c++1z -Wl,--no-as-needed")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -ggdb -fprofile-arcs -ftest-coverage")
SET(CMAKE_BUILD_TYPE Debug)

INCLUDE_DIRECTORIES(
    ../include
    ${PROJECT_SOURCE_DIR}
)

ADD_EXECUTABLE(raft-tests-fsm tests_fsm.cc)
ADD_TEST(raft-tests-fsm ${CMAKE_BINARY_DIR}/tests/raft-tests-fsm)

ADD_EXECUTABLE(raft-tests-store tests_store.cc)
TARGET_LINK_LIBRARIES(raft-tests-store stdc++fs)
ADD_TEST(raft-tests-store ${CMAKE_BINARY_DIR}/tests/raft-tests-store)

ADD_EXECUTABLE(raft-tests-server tests_server.cc)
ADD_TEST(raft-tests-server ${CMAKE_BINARY_DIR}/tests/raft-tests-server)

