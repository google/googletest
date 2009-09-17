// Copyright 2009 Google Inc. All rights reserved.
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
// Author: vladl@google.com (Vlad Losev)
//
// The Google C++ Testing Framework (Google Test)
//
// This file verifies Google Test event listeners receive events at the
// right times.

#include <gtest/gtest.h>

// Indicates that this translation unit is part of Google Test's
// implementation.  It must come before gtest-internal-inl.h is
// included, or there will be a compiler error.  This trick is to
// prevent a user from accidentally including gtest-internal-inl.h in
// his code.
#define GTEST_IMPLEMENTATION_ 1
#include "src/gtest-internal-inl.h"  // For Vector.
#undef GTEST_IMPLEMENTATION_

using ::testing::AddGlobalTestEnvironment;
using ::testing::Environment;
using ::testing::InitGoogleTest;
using ::testing::Test;
using ::testing::TestInfo;
using ::testing::TestPartResult;
using ::testing::UnitTest;
using ::testing::internal::String;
using ::testing::internal::TestCase;
using ::testing::internal::UnitTestEventListenerInterface;
using ::testing::internal::Vector;

// Used by tests to register their events.
Vector<String>* g_events = NULL;

namespace testing {
namespace internal {

// TODO(vladl@google.com): Remove this and use UnitTest::listeners()
// directly after it is published.
class UnitTestAccessor {
 public:
  static EventListeners& GetEventListeners() {
    return UnitTest::GetInstance()->listeners();
  }
  static bool UnitTestFailed() { return UnitTest::GetInstance()->Failed(); }
};

class EventRecordingListener : public UnitTestEventListenerInterface {
 public:
  EventRecordingListener(const char* name) : name_(name) {}

 protected:
  virtual void OnTestProgramStart(const UnitTest& /*unit_test*/) {
    g_events->PushBack(GetFullMethodName("OnTestProgramStart"));
  }

  virtual void OnTestIterationStart(const UnitTest& /*unit_test*/,
                                    int iteration) {
    Message message;
    message << GetFullMethodName("OnTestIterationStart")
            << "(" << iteration << ")";
    g_events->PushBack(message.GetString());
  }

  virtual void OnEnvironmentsSetUpStart(const UnitTest& /*unit_test*/) {
    g_events->PushBack(GetFullMethodName("OnEnvironmentsSetUpStart"));
  }

  virtual void OnEnvironmentsSetUpEnd(const UnitTest& /*unit_test*/) {
    g_events->PushBack(GetFullMethodName("OnEnvironmentsSetUpEnd"));
  }

  virtual void OnTestCaseStart(const TestCase& /*test_case*/) {
    g_events->PushBack(GetFullMethodName("OnTestCaseStart"));
  }

  virtual void OnTestStart(const TestInfo& /*test_info*/) {
    g_events->PushBack(GetFullMethodName("OnTestStart"));
  }

  virtual void OnTestPartResult(const TestPartResult& /*test_part_result*/) {
    g_events->PushBack(GetFullMethodName("OnTestPartResult"));
  }

  virtual void OnTestEnd(const TestInfo& /*test_info*/) {
    g_events->PushBack(GetFullMethodName("OnTestEnd"));
  }

  virtual void OnTestCaseEnd(const TestCase& /*test_case*/) {
    g_events->PushBack(GetFullMethodName("OnTestCaseEnd"));
  }

  virtual void OnEnvironmentsTearDownStart(const UnitTest& /*unit_test*/) {
    g_events->PushBack(GetFullMethodName("OnEnvironmentsTearDownStart"));
  }

  virtual void OnEnvironmentsTearDownEnd(const UnitTest& /*unit_test*/) {
    g_events->PushBack(GetFullMethodName("OnEnvironmentsTearDownEnd"));
  }

  virtual void OnTestIterationEnd(const UnitTest& /*unit_test*/,
                                  int iteration) {
    Message message;
    message << GetFullMethodName("OnTestIterationEnd")
            << "("  << iteration << ")";
    g_events->PushBack(message.GetString());
  }

  virtual void OnTestProgramEnd(const UnitTest& /*unit_test*/) {
    g_events->PushBack(GetFullMethodName("OnTestProgramEnd"));
  }

 private:
  String GetFullMethodName(const char* name) {
    Message message;
    message << name_ << "." << name;
    return message.GetString();
  }

  String name_;
};

class EnvironmentInvocationCatcher : public Environment {
 protected:
  virtual void SetUp() {
    g_events->PushBack(String("Environment::SetUp"));
  }

  virtual void TearDown() {
    g_events->PushBack(String("Environment::TearDown"));
  }
};

class ListenerTest : public Test {
 protected:
  static void SetUpTestCase() {
    g_events->PushBack(String("ListenerTest::SetUpTestCase"));
  }

  static void TearDownTestCase() {
    g_events->PushBack(String("ListenerTest::TearDownTestCase"));
  }

  virtual void SetUp() {
    g_events->PushBack(String("ListenerTest::SetUp"));
  }

  virtual void TearDown() {
    g_events->PushBack(String("ListenerTest::TearDown"));
  }
};

TEST_F(ListenerTest, DoesFoo) {
  // Test execution order within a test case is not guaranteed so we are not
  // recording the test name.
  g_events->PushBack(String("ListenerTest::* Test Body"));
  SUCCEED();  // Triggers OnTestPartResult.
}

TEST_F(ListenerTest, DoesBar) {
  g_events->PushBack(String("ListenerTest::* Test Body"));
  SUCCEED();  // Triggers OnTestPartResult.
}

}  // namespace internal

}  // namespace testing

using ::testing::internal::EnvironmentInvocationCatcher;
using ::testing::internal::EventRecordingListener;
using ::testing::internal::UnitTestAccessor;

void VerifyResults(const Vector<String>& data,
                   const char* const* expected_data,
                   int expected_data_size) {
  const int actual_size = data.size();
  // If the following assertion fails, a new entry will be appended to
  // data.  Hence we save data.size() first.
  EXPECT_EQ(expected_data_size, actual_size);

  // Compares the common prefix.
  const int shorter_size = expected_data_size <= actual_size ?
      expected_data_size : actual_size;
  int i = 0;
  for (; i < shorter_size; ++i) {
    ASSERT_STREQ(expected_data[i], data.GetElement(i).c_str())
        << "at position " << i;
  }

  // Prints extra elements in the actual data.
  for (; i < actual_size; ++i) {
    printf("  Actual event #%d: %s\n", i, data.GetElement(i).c_str());
  }
}

int main(int argc, char **argv) {
  Vector<String> events;
  g_events = &events;
  InitGoogleTest(&argc, argv);

  UnitTestAccessor::GetEventListeners().Append(
      new EventRecordingListener("1st"));
  UnitTestAccessor::GetEventListeners().Append(
      new EventRecordingListener("2nd"));

  AddGlobalTestEnvironment(new EnvironmentInvocationCatcher);

  GTEST_CHECK_(events.size() == 0)
      << "AddGlobalTestEnvironment should not generate any events itself.";

  ::testing::GTEST_FLAG(repeat) = 2;
  int ret_val = RUN_ALL_TESTS();

  const char* const expected_events[] = {
    "1st.OnTestProgramStart",
    "2nd.OnTestProgramStart",
    "1st.OnTestIterationStart(0)",
    "2nd.OnTestIterationStart(0)",
    "1st.OnEnvironmentsSetUpStart",
    "2nd.OnEnvironmentsSetUpStart",
    "Environment::SetUp",
    "2nd.OnEnvironmentsSetUpEnd",
    "1st.OnEnvironmentsSetUpEnd",
    "1st.OnTestCaseStart",
    "2nd.OnTestCaseStart",
    "ListenerTest::SetUpTestCase",
    "1st.OnTestStart",
    "2nd.OnTestStart",
    "ListenerTest::SetUp",
    "ListenerTest::* Test Body",
    "1st.OnTestPartResult",
    "2nd.OnTestPartResult",
    "ListenerTest::TearDown",
    "2nd.OnTestEnd",
    "1st.OnTestEnd",
    "1st.OnTestStart",
    "2nd.OnTestStart",
    "ListenerTest::SetUp",
    "ListenerTest::* Test Body",
    "1st.OnTestPartResult",
    "2nd.OnTestPartResult",
    "ListenerTest::TearDown",
    "2nd.OnTestEnd",
    "1st.OnTestEnd",
    "ListenerTest::TearDownTestCase",
    "2nd.OnTestCaseEnd",
    "1st.OnTestCaseEnd",
    "1st.OnEnvironmentsTearDownStart",
    "2nd.OnEnvironmentsTearDownStart",
    "Environment::TearDown",
    "2nd.OnEnvironmentsTearDownEnd",
    "1st.OnEnvironmentsTearDownEnd",
    "2nd.OnTestIterationEnd(0)",
    "1st.OnTestIterationEnd(0)",
    "1st.OnTestIterationStart(1)",
    "2nd.OnTestIterationStart(1)",
    "1st.OnEnvironmentsSetUpStart",
    "2nd.OnEnvironmentsSetUpStart",
    "Environment::SetUp",
    "2nd.OnEnvironmentsSetUpEnd",
    "1st.OnEnvironmentsSetUpEnd",
    "1st.OnTestCaseStart",
    "2nd.OnTestCaseStart",
    "ListenerTest::SetUpTestCase",
    "1st.OnTestStart",
    "2nd.OnTestStart",
    "ListenerTest::SetUp",
    "ListenerTest::* Test Body",
    "1st.OnTestPartResult",
    "2nd.OnTestPartResult",
    "ListenerTest::TearDown",
    "2nd.OnTestEnd",
    "1st.OnTestEnd",
    "1st.OnTestStart",
    "2nd.OnTestStart",
    "ListenerTest::SetUp",
    "ListenerTest::* Test Body",
    "1st.OnTestPartResult",
    "2nd.OnTestPartResult",
    "ListenerTest::TearDown",
    "2nd.OnTestEnd",
    "1st.OnTestEnd",
    "ListenerTest::TearDownTestCase",
    "2nd.OnTestCaseEnd",
    "1st.OnTestCaseEnd",
    "1st.OnEnvironmentsTearDownStart",
    "2nd.OnEnvironmentsTearDownStart",
    "Environment::TearDown",
    "2nd.OnEnvironmentsTearDownEnd",
    "1st.OnEnvironmentsTearDownEnd",
    "2nd.OnTestIterationEnd(1)",
    "1st.OnTestIterationEnd(1)",
    "2nd.OnTestProgramEnd",
    "1st.OnTestProgramEnd"
  };
  VerifyResults(events,
                expected_events,
                sizeof(expected_events)/sizeof(expected_events[0]));

  // We need to check manually for ad hoc test failures that happen after
  // RUN_ALL_TESTS finishes.
  if (UnitTestAccessor::UnitTestFailed())
    ret_val = 1;

  return ret_val;
}
