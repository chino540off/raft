INCLUDE(ExternalProject)

ExternalProject_Add(json
  GIT_REPOSITORY https://github.com/nlohmann/json
  GIT_TAG v3.6.1
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
            -DJSON_BuildTests=OFF
)

ExternalProject_Get_Property(json install_dir)
SET(JSON_INCLUDE_DIRS ${install_dir}/include PARENT_SCOPE)
