#!/bin/bash
#
# Copyright 2020, Google Inc.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#     * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following disclaimer
# in the documentation and/or other materials provided with the
# distribution.
#     * Neither the name of Google Inc. nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

set -euox pipefail

readonly LINUX_LATEST_CONTAINER="gcr.io/google.com/absl-177019/linux_hybrid-latest:20241218"
readonly LINUX_GCC_FLOOR_CONTAINER="gcr.io/google.com/absl-177019/linux_gcc-floor:20250205"

if [[ -z ${GTEST_ROOT:-} ]]; then
  GTEST_ROOT="$(realpath $(dirname ${0})/..)"
fi

if [[ -z ${STD:-} ]]; then
  STD="c++17 c++20"
fi

# Test CMake + GCC
for cmake_off_on in OFF ON; do
  time docker run \
    --volume="${GTEST_ROOT}:/src:ro" \
    --tmpfs="/build:exec" \
    --workdir="/build" \
    --rm \
    --env="CC=/usr/local/bin/gcc" \
    --env=CXXFLAGS="-Werror -Wdeprecated" \
    ${LINUX_LATEST_CONTAINER} \
    /bin/bash -c "
      cmake /src \
        -DCMAKE_CXX_STANDARD=17 \
        -Dgtest_build_samples=ON \
        -Dgtest_build_tests=ON \
        -Dgmock_build_tests=ON \
        -Dcxx_no_exception=${cmake_off_on} \
        -Dcxx_no_rtti=${cmake_off_on} && \
      make -j$(nproc) && \
      ctest -j$(nproc) --output-on-failure"
done

# Test CMake + Clang
for cmake_off_on in OFF ON; do
  time docker run \
    --volume="${GTEST_ROOT}:/src:ro" \
    --tmpfs="/build:exec" \
    --workdir="/build" \
    --rm \
    --env="CC=/opt/llvm/clang/bin/clang" \
    --env=CXXFLAGS="-Werror -Wdeprecated --gcc-toolchain=/usr/local" \
    ${LINUX_LATEST_CONTAINER} \
    /bin/bash -c "
      cmake /src \
        -DCMAKE_CXX_STANDARD=17 \
        -Dgtest_build_samples=ON \
        -Dgtest_build_tests=ON \
        -Dgmock_build_tests=ON \
        -Dcxx_no_exception=${cmake_off_on} \
        -Dcxx_no_rtti=${cmake_off_on} && \
      make -j$(nproc) && \
      ctest -j$(nproc) --output-on-failure"
done

# Do one test with an older version of GCC
time docker run \
  --volume="${GTEST_ROOT}:/src:ro" \
  --workdir="/src" \
  --rm \
  --env="CC=/usr/local/bin/gcc" \
  --env="BAZEL_CXXOPTS=-std=c++17" \
  ${LINUX_GCC_FLOOR_CONTAINER} \
    /usr/local/bin/bazel test ... \
      --copt="-Wall" \
      --copt="-Werror" \
      --copt="-Wuninitialized" \
      --copt="-Wundef" \
      --copt="-Wno-error=pragmas" \
      --enable_bzlmod=false \
      --features=external_include_paths \
      --keep_going \
      --show_timestamps \
      --test_output=errors

# Test GCC
for std in ${STD}; do
  for absl in 0 1; do
    time docker run \
      --volume="${GTEST_ROOT}:/src:ro" \
      --workdir="/src" \
      --rm \
      --env="CC=/usr/local/bin/gcc" \
      --env="BAZEL_CXXOPTS=-std=${std}" \
      ${LINUX_LATEST_CONTAINER} \
      /usr/local/bin/bazel test ... \
        --copt="-Wall" \
        --copt="-Werror" \
        --copt="-Wuninitialized" \
        --copt="-Wundef" \
        --define="absl=${absl}" \
        --enable_bzlmod=true \
        --features=external_include_paths \
        --keep_going \
        --show_timestamps \
        --test_output=errors
  done
done

# Test Clang
for std in ${STD}; do
  for absl in 0 1; do
    time docker run \
      --volume="${GTEST_ROOT}:/src:ro" \
      --workdir="/src" \
      --rm \
      --env="CC=/opt/llvm/clang/bin/clang" \
      --env="BAZEL_CXXOPTS=-std=${std}" \
      ${LINUX_LATEST_CONTAINER} \
      /usr/local/bin/bazel test ... \
        --copt="--gcc-toolchain=/usr/local" \
        --copt="-Wall" \
        --copt="-Werror" \
        --copt="-Wuninitialized" \
        --copt="-Wundef" \
        --define="absl=${absl}" \
        --enable_bzlmod=true \
        --features=external_include_paths \
        --keep_going \
        --linkopt="--gcc-toolchain=/usr/local" \
        --show_timestamps \
        --test_output=errors
  done
done
