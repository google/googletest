// Copyright 2007, Google Inc.
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
// Author: wan@google.com (Zhanyong Wan)

// Tests that SCOPED_TRACE() and various Google Test assertions can be
// used in a large number of threads concurrently.

#include <iostream>
#include <gtest/gtest.h>

// We must define this macro in order to #include
// gtest-internal-inl.h.  This is how Google Test prevents a user from
// accidentally depending on its internal implementation.
#define GTEST_IMPLEMENTATION_ 1
#include "src/gtest-internal-inl.h"
#undef GTEST_IMPLEMENTATION_

namespace testing {
namespace {

using internal::List;
using internal::ListNode;
using internal::String;
using internal::TestProperty;
using internal::TestPropertyKeyIs;

// How many threads to create?
const int kThreadCount = 50;

String IdToKey(int id, const char* suffix) {
  Message key;
  key << "key_" << id << "_" << suffix;
  return key.GetString();
}

String IdToString(int id) {
  Message id_message;
  id_message << id;
  return id_message.GetString();
}

void ExpectKeyAndValueWereRecordedForId(const List<TestProperty>& properties,
                                        int id,
                                        const char* suffix) {
  TestPropertyKeyIs matches_key(IdToKey(id, suffix).c_str());
  const ListNode<TestProperty>* node = properties.FindIf(matches_key);
  EXPECT_TRUE(node != NULL) << "expecting " << suffix << " node for id " << id;
  EXPECT_STREQ(IdToString(id).c_str(), node->element().value());
}

// Calls a large number of Google Test assertions, where exactly one of them
// will fail.
void ManyAsserts(int id) {
  ::std::cout << "Thread #" << id << " running...\n";

  SCOPED_TRACE(Message() << "Thread #" << id);

  for (int i = 0; i < kThreadCount; i++) {
    SCOPED_TRACE(Message() << "Iteration #" << i);

    // A bunch of assertions that should succeed.
    EXPECT_TRUE(true);
    ASSERT_FALSE(false) << "This shouldn't fail.";
    EXPECT_STREQ("a", "a");
    ASSERT_LE(5, 6);
    EXPECT_EQ(i, i) << "This shouldn't fail.";

    // RecordProperty() should interact safely with other threads as well.
    // The shared_key forces property updates.
    Test::RecordProperty(IdToKey(id, "string").c_str(), IdToString(id).c_str());
    Test::RecordProperty(IdToKey(id, "int").c_str(), id);
    Test::RecordProperty("shared_key", IdToString(id).c_str());

    // This assertion should fail kThreadCount times per thread.  It
    // is for testing whether Google Test can handle failed assertions in a
    // multi-threaded context.
    EXPECT_LT(i, 0) << "This should always fail.";
  }
}

// Tests using SCOPED_TRACE() and Google Test assertions in many threads
// concurrently.
TEST(StressTest, CanUseScopedTraceAndAssertionsInManyThreads) {
  // TODO(wan): when Google Test is made thread-safe, run
  // ManyAsserts() in many threads here.
}

TEST(NoFatalFailureTest, ExpectNoFatalFailureIgnoresFailuresInOtherThreads) {
  // TODO(mheule@google.com): Test this works correctly when Google
  // Test is made thread-safe.
}

TEST(NoFatalFailureTest, AssertNoFatalFailureIgnoresFailuresInOtherThreads) {
  // TODO(mheule@google.com): Test this works correctly when Google
  // Test is made thread-safe.
}

TEST(FatalFailureTest, ExpectFatalFailureIgnoresFailuresInOtherThreads) {
  // TODO(mheule@google.com): Test this works correctly when Google
  // Test is made thread-safe.
}

TEST(FatalFailureOnAllThreadsTest, ExpectFatalFailureOnAllThreads) {
  // TODO(wan@google.com): Test this works correctly when Google Test
  // is made thread-safe.
}

TEST(NonFatalFailureTest, ExpectNonFatalFailureIgnoresFailuresInOtherThreads) {
  // TODO(mheule@google.com): Test this works correctly when Google
  // Test is made thread-safe.
}

TEST(NonFatalFailureOnAllThreadsTest, ExpectNonFatalFailureOnAllThreads) {
  // TODO(wan@google.com): Test this works correctly when Google Test
  // is made thread-safe.
}

}  // namespace
}  // namespace testing

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
