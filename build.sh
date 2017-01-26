#! /bin/bash

function build()
{
  if [ -d build ]; then
          rm -rf build
  fi

  mkdir build
  cd build
  cmake .. -DCMAKE_BUILD_TYPE=Debug
  make
}

function test()
{
  cd build
  ctest
}

$1
