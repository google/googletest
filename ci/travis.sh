#!/usr/bin/env sh
set -evx

. ci/get-nprocessors.sh

# Tell make to use the processors. No preceding '-' required.
MAKEFLAGS="j${NPROCESSORS}"
export MAKEFLAGS

env | sort

# Set default values to OFF for these variables if not specified.
: "${NO_EXCEPTION:=OFF}"
: "${NO_RTTI:=OFF}"
: "${COMPILER_IS_GNUCXX:=OFF}"

mkdir build || true
cd build
cmake -Dgtest_build_samples=ON \
      -Dgtest_build_tests=ON \
      -Dgmock_build_tests=ON \
      -Dcxx_no_exception="$NO_EXCEPTION" \
      -Dcxx_no_rtti="$NO_RTTI" \
      -DCMAKE_COMPILER_IS_GNUCXX="$COMPILER_IS_GNUCXX" \
      -DCMAKE_CXX_FLAGS="$CXX_FLAGS" \
      -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
      ..
make
CTEST_OUTPUT_ON_FAILURE=1 make test
