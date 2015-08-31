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

mkdir build

mkdir build/googletest
( cd build/googletest && cmake ../../googletest )
if [ "$TRAVIS_OS_NAME" != "osx" ]
then
( cd build/googletest && make )
fi

mkdir build/googlemock
( cd build/googlemock && cmake ../../googlemock )
if [ "$TRAVIS_OS_NAME" != "osx" ]
then
  ( cd build/googlemock && make )
fi
