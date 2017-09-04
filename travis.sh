#!/usr/bin/env sh
set -evx

# if possible, ask for the precise number of processors,
# otherwise take 2 processors as reasonable default; see
# https://docs.travis-ci.com/user/speeding-up-the-build/#Makefile-optimization
if [ -x /usr/bin/getconf ]; then
    MAKEFLAGS=j$(/usr/bin/getconf _NPROCESSORS_ONLN)
else
    MAKEFLAGS="j2"
fi
export MAKEFLAGS

env | sort

mkdir build || true
cd build
cmake -Dgtest_build_samples=ON \
      -Dgtest_build_tests=ON \
      -Dgmock_build_tests=ON \
      -DCMAKE_CXX_FLAGS=$CXX_FLAGS \
      -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
      ..
make
CTEST_OUTPUT_ON_FAILURE=1 make test
