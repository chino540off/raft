INCLUDE(ExternalProject)

ExternalProject_Add(spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog
  GIT_TAG v1.3.1
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
            -DSPDLOG_BUILD_EXAMPLES=OFF
            -DSPDLOG_BUILD_BENCH=OFF
            -DSPDLOG_BUILD_TESTS=OFF
)

ExternalProject_Get_Property(spdlog install_dir)
SET(SPDLOG_INCLUDE_DIRS ${install_dir}/include PARENT_SCOPE)
