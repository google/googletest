#!/usr/bin/env sh
set -evx
env | sort
t = $GTEST_TARGET
mkdir build
mkdir build/$t
cd build/$t
cmake ../../$t
make
