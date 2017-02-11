#! /bin/bash -ex

function _build()
{
  if [ -d build ]; then
          rm -rf build
  fi

  mkdir build
  cd build
  cmake .. -DCMAKE_BUILD_TYPE=Debug
  make
}

function _test()
{
  cd build
  ctest
  cd -
}

function _cov()
{
  output_info=lcov.info
  output=lcov

  lcov --base-directory . --directory . --zerocounters -q

  _test

  lcov --base-directory . --directory . -c -o $output_info
  genhtml -o $output -t "test coverage" --num-spaces 4 $output_info
}

_$1
