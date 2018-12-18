// Copyright 2018, Cristian Klein
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

//
// Utilities for testing Google Test itself and code that uses Google Test
// (e.g. frameworks built on top of Google Test).

// GOOGLETEST_CM0004 DO NOT DELETE

#ifndef GTEST_INCLUDE_GTEST_GTEST_SNAPSHOT_H_
#define GTEST_INCLUDE_GTEST_GTEST_SNAPSHOT_H_

#include "gtest/gtest.h"

#include <iostream>
#include <fstream>

GTEST_DISABLE_MSC_WARNINGS_PUSH_(4251 \
/* class A needs to have dll-interface to be used by clients of class B */)

namespace testing {

namespace internal {

// Helper to return the name of the file where the snapshot is stored for the current
// test.
inline
std::string GetSnapshotFile() {
  const TestInfo *info = ::testing::UnitTest::GetInstance()->current_test_info();
  const std::string value_param = info->value_param();
  std::string snapshot_file = info->file();

  snapshot_file.push_back('_');

  for (char c : value_param) {
    if (isalnum(c))
      snapshot_file.push_back(c);
  }

  snapshot_file += ".snap";

  return snapshot_file;
}

// Helper to update (if `update_snapshot` flags it true) and load snapshot.
// Could be a template, but currently only std::string is supported.
template <typename T>
std::string SnapshotHelper(const T& val) {
  const std::string snapshot_file = GetSnapshotFile();
  
  if (GTEST_FLAG(update_snapshot)) {
    GTEST_LOG_(INFO) << "Updating snapshot " << snapshot_file;
    std::ofstream snapshot(snapshot_file);

    if (!snapshot.is_open())
      return "Could not open snapshot for update: " + snapshot_file;

    snapshot << val;

    snapshot.close();
  }

  std::ifstream snapshot(snapshot_file);
  if (!snapshot.is_open())
    return "Could not open snapshot: " + snapshot_file;

  std::stringstream expected;
  expected << snapshot.rdbuf();
  snapshot.close();

  return expected.str();
}

}  // namespace internal

}  // namespace testing

GTEST_DISABLE_MSC_WARNINGS_POP_()  //  4251


#endif  // GTEST_INCLUDE_GTEST_GTEST_SNAPSHOT_H_
