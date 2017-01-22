CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(raft)

INCLUDE(CTest)

# Test app
ADD_SUBDIRECTORY(tests)

INSTALL(
    DIRECTORY include/
    DESTINATION include
    COMPONENT dev
)

IF(UNIX OR CYGWIN)
    SET(SHARE_DIR share/${CMAKE_PROJECT_NAME})
ELSEIF (WIN32)
    SET(SHARE_DIR bin)
ENDIF()