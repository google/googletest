#!/usr/bin/env sh
set -evx
env | sort
mkdir build
for d in googletest googlemock
do
  ( mkdir build/$d &&
    cd build/$d &&
    cmake ../../$d &&
    make)
done
