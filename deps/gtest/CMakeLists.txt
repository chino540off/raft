include(ExternalProject)

ExternalProject_Add(googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.8.0
  CMAKE_ARGS -Dgtest_force_shared_crt=ON
             -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
)

ExternalProject_Get_Property(googletest install_dir)
set(GTEST_INCLUDE_DIRS ${install_dir}/include PARENT_SCOPE)
set(GTEST_LIBS_DIR     ${install_dir}/lib/ PARENT_SCOPE)
set(GTEST_LIBS         ${install_dir}/lib/libgtest.a ${install_dir}/lib/libgtest_main.a PARENT_SCOPE)
