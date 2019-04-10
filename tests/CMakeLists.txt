add_executable(raft-tests
  ./tests_logger.cc
  ./tests_json.cc
  ./tests_log.cc
  ./tests_node.cc
  ./tests_rpc.cc
  ./tests_server.cc
)

add_dependencies(raft-tests googletest)

target_include_directories(raft-tests
  PRIVATE
    ${GTEST_INCLUDE_DIRS}
    ${SPDLOG_INCLUDE_DIRS}
    ${JSON_INCLUDE_DIRS}
    ${RAFT_INCLUDE_DIRS}
)

target_link_libraries(raft-tests
  ${GTEST_LIBS}
  ${CMAKE_THREAD_LIBS_INIT}
)

add_test(NAME raft-unit-tests COMMAND ./raft-tests)
