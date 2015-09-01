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
set -evx

env | sort

mkdir build || true

( ( mkdir build/googletest || true ) &&
  cd build/googletest &&
  cmake -Dgtest_build_tests=ON -Dgtest_build_samples=ON ../../googletest &&
  make && make test)
( ( mkdir build/googlemock || true ) &&
  cd build/googlemock &&
  cmake -Dgmock_build_tests=ON -Dgtest_build_samples=ON ../../googlemock &&
  make && make test)
