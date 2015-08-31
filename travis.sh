#!/usr/bin/env sh
# This is called by `.travis.yml` via Travis CI.
# Travis supplies $TRAVIS_OS_NAME.
#  http://docs.travis-ci.com/user/multi-os/
# Our .travis.yml also defines:
#   - SHARED_LIB=ON/OFF
#   - STATIC_LIB=ON/OFF
#   - CMAKE_PKG=ON/OFF
#   - BUILD_TYPE=release/debug
#   - VERBOSE_MAKE=false/true
#   - VERBOSE (set or not)

# -e: fail on error
# -v: show commands
# -x: show expanded commands
set -vex

env | sort

cmake -DJSONCPP_WITH_CMAKE_PACKAGE=$CMAKE_PKG -DBUILD_SHARED_LIBS=$SHARED_LIB -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_VERBOSE_MAKEFILE=$VERBOSE_MAKE .
make

# Python is not available in Travis for osx.
#  https://github.com/travis-ci/travis-ci/issues/2320
if [ "$TRAVIS_OS_NAME" != "osx" ]
then
  make
  # valgrind --error-exitcode=42 --leak-check=full ./src/test_lib_json/jsoncpp_test
fi
