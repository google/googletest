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

#include <gtest/gtest.h>

#include <iostream>

// We must define this macro in order to #include
// gtest-internal-inl.h.  This is how Google Test prevents a user from
// accidentally depending on its internal implementation.
#define GTEST_IMPLEMENTATION_ 1
#include "src/gtest-internal-inl.h"
#undef GTEST_IMPLEMENTATION_

#if GTEST_IS_THREADSAFE

namespace testing {
namespace {

using internal::String;
using internal::TestPropertyKeyIs;
using internal::Vector;

// In order to run tests in this file, for platforms where Google Test is
// thread safe, implement ThreadWithParam with the following interface:
//
// template <typename T> class ThreadWithParam {
//  public:
//   // Creates the thread. The thread should execute thread_func(param) when
//   // started by a call to Start().
//   ThreadWithParam(void (*thread_func)(T), T param);
//   // Starts the thread.
//   void Start();
//   // Waits for the thread to finish.
//   void Join();
// };

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

void ExpectKeyAndValueWereRecordedForId(const Vector<TestProperty>& properties,
                                        int id,
                                        const char* suffix) {
  TestPropertyKeyIs matches_key(IdToKey(id, suffix).c_str());
  const TestProperty* property = properties.FindIf(matches_key);
  ASSERT_TRUE(property != NULL)
      << "expecting " << suffix << " value for id " << id;
  EXPECT_STREQ(IdToString(id).c_str(), property->value());
}

// Calls a large number of Google Test assertions, where exactly one of them
// will fail.
void ManyAsserts(int id) {
  GTEST_LOG_(INFO) << "Thread #" << id << " running...";

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

void CheckTestFailureCount(int expected_failures) {
  const TestInfo* const info = UnitTest::GetInstance()->current_test_info();
  const TestResult* const result = info->result();
  GTEST_CHECK_(expected_failures == result->total_part_count())
      << "Logged " << result->total_part_count() << " failures "
      << " vs. " << expected_failures << " expected";
}

// Tests using SCOPED_TRACE() and Google Test assertions in many threads
// concurrently.
TEST(StressTest, CanUseScopedTraceAndAssertionsInManyThreads) {
  ThreadWithParam<int>* threads[kThreadCount] = {};
  for (int i = 0; i != kThreadCount; i++) {
    // Creates a thread to run the ManyAsserts() function.
    threads[i] = new ThreadWithParam<int>(&ManyAsserts, i);

    // Starts the thread.
    threads[i]->Start();
  }

  // At this point, we have many threads running.

  for (int i = 0; i != kThreadCount; i++) {
    // We block until the thread is done.
    threads[i]->Join();
    delete threads[i];
    threads[i] = NULL;
  }

  // Ensures that kThreadCount*kThreadCount failures have been reported.
  const TestInfo* const info = UnitTest::GetInstance()->current_test_info();
  const TestResult* const result = info->result();

  Vector<TestProperty> properties;
  // We have no access to the TestResult's list of properties but we can
  // copy them one by one.
  for (int i = 0; i < result->test_property_count(); ++i)
    properties.PushBack(result->GetTestProperty(i));

  EXPECT_EQ(kThreadCount * 2 + 1, result->test_property_count())
      << "String and int values recorded on each thread, "
      << "as well as one shared_key";
  for (int i = 0; i < kThreadCount; ++i) {
    ExpectKeyAndValueWereRecordedForId(properties, i, "string");
    ExpectKeyAndValueWereRecordedForId(properties, i, "int");
  }
  CheckTestFailureCount(kThreadCount*kThreadCount);
}

void FailingThread(bool is_fatal) {
  if (is_fatal)
    FAIL() << "Fatal failure in some other thread. "
           << "(This failure is expected.)";
  else
    ADD_FAILURE() << "Non-fatal failure in some other thread. "
                  << "(This failure is expected.)";
}

void GenerateFatalFailureInAnotherThread(bool is_fatal) {
  ThreadWithParam<bool> thread(&FailingThread, is_fatal);
  thread.Start();
  thread.Join();
}

TEST(NoFatalFailureTest, ExpectNoFatalFailureIgnoresFailuresInOtherThreads) {
  EXPECT_NO_FATAL_FAILURE(GenerateFatalFailureInAnotherThread(true));
  // We should only have one failure (the one from
  // GenerateFatalFailureInAnotherThread()), since the EXPECT_NO_FATAL_FAILURE
  // should succeed.
  CheckTestFailureCount(1);
}

void AssertNoFatalFailureIgnoresFailuresInOtherThreads() {
  ASSERT_NO_FATAL_FAILURE(GenerateFatalFailureInAnotherThread(true));
}
TEST(NoFatalFailureTest, AssertNoFatalFailureIgnoresFailuresInOtherThreads) {
  // Using a subroutine, to make sure, that the test continues.
  AssertNoFatalFailureIgnoresFailuresInOtherThreads();
  // We should only have one failure (the one from
  // GenerateFatalFailureInAnotherThread()), since the EXPECT_NO_FATAL_FAILURE
  // should succeed.
  CheckTestFailureCount(1);
}

TEST(FatalFailureTest, ExpectFatalFailureIgnoresFailuresInOtherThreads) {
  // This statement should fail, since the current thread doesn't generate a
  // fatal failure, only another one does.
  EXPECT_FATAL_FAILURE(GenerateFatalFailureInAnotherThread(true), "expected");
  CheckTestFailureCount(2);
}

TEST(FatalFailureOnAllThreadsTest, ExpectFatalFailureOnAllThreads) {
  // This statement should succeed, because failures in all threads are
  // considered.
  EXPECT_FATAL_FAILURE_ON_ALL_THREADS(
      GenerateFatalFailureInAnotherThread(true), "expected");
  CheckTestFailureCount(0);
  // We need to add a failure, because main() checks that there are failures.
  // But when only this test is run, we shouldn't have any failures.
  ADD_FAILURE() << "This is an expected non-fatal failure.";
}

TEST(NonFatalFailureTest, ExpectNonFatalFailureIgnoresFailuresInOtherThreads) {
  // This statement should fail, since the current thread doesn't generate a
  // fatal failure, only another one does.
  EXPECT_NONFATAL_FAILURE(GenerateFatalFailureInAnotherThread(false),
                          "expected");
  CheckTestFailureCount(2);
}

TEST(NonFatalFailureOnAllThreadsTest, ExpectNonFatalFailureOnAllThreads) {
  // This statement should succeed, because failures in all threads are
  // considered.
  EXPECT_NONFATAL_FAILURE_ON_ALL_THREADS(
      GenerateFatalFailureInAnotherThread(false), "expected");
  CheckTestFailureCount(0);
  // We need to add a failure, because main() checks that there are failures,
  // But when only this test is run, we shouldn't have any failures.
  ADD_FAILURE() << "This is an expected non-fatal failure.";
}

}  // namespace
}  // namespace testing

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);

  const int result = RUN_ALL_TESTS();  // Expected to fail.
  GTEST_CHECK_(result == 1) << "RUN_ALL_TESTS() did not fail as expected";

  printf("\nPASS\n");
  return 0;
}

#else
TEST(StressTest,
     DISABLED_ThreadSafetyTestsAreSkippedWhenGoogleTestIsNotThreadSafe) {
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
#endif  // GTEST_IS_THREADSAFE
