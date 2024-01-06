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

if [[ -z ${GTEST_ROOT:-} ]]; then
  GTEST_ROOT="$(realpath $(dirname ${0})/..)"
fi

# Test the CMake build
for cmake_off_on in OFF ON; do
  BUILD_DIR=$(mktemp -d build_dir.XXXXXXXX)
  cd ${BUILD_DIR}
  time cmake ${GTEST_ROOT} \
    -DCMAKE_CXX_STANDARD=14 \
    -Dgtest_build_samples=ON \
    -Dgtest_build_tests=ON \
    -Dgmock_build_tests=ON \
    -Dcxx_no_exception=${cmake_off_on} \
    -Dcxx_no_rtti=${cmake_off_on}
  time make
  time ctest -j$(nproc) --output-on-failure
done

# Test the Bazel build

# If we are running on Kokoro, check for a versioned Bazel binary.
KOKORO_GFILE_BAZEL_BIN="bazel-5.1.1-darwin-x86_64"
if [[ ${KOKORO_GFILE_DIR:-} ]] && [[ -f ${KOKORO_GFILE_DIR}/${KOKORO_GFILE_BAZEL_BIN} ]]; then
  BAZEL_BIN="${KOKORO_GFILE_DIR}/${KOKORO_GFILE_BAZEL_BIN}"
  chmod +x ${BAZEL_BIN}
else
  BAZEL_BIN="bazel"
fi

cd ${GTEST_ROOT}
for absl in 0 1; do
  ${BAZEL_BIN} test ... \
    --copt="-Wall" \
    --copt="-Werror" \
    --copt="-Wundef" \
    --cxxopt="-std=c++14" \
    --define="absl=${absl}" \
    --features=external_include_paths \
    --keep_going \
    --show_timestamps \
    --test_output=errors
done
