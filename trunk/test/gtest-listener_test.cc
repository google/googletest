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
 protected:
  virtual void OnUnitTestStart(const UnitTest& unit_test) {
    g_events->PushBack(String("TestEventListener::OnUnitTestStart"));
  }

  virtual void OnGlobalSetUpStart(const UnitTest& unit_test) {
    g_events->PushBack(String("TestEventListener::OnGlobalSetUpStart"));
  }

  virtual void OnGlobalSetUpEnd(const UnitTest& unit_test) {
    g_events->PushBack(String("TestEventListener::OnGlobalSetUpEnd"));
  }

  virtual void OnTestCaseStart(const TestCase& test_case) {
    g_events->PushBack(String("TestEventListener::OnTestCaseStart"));
  }

  virtual void OnTestStart(const TestInfo& test_info) {
    g_events->PushBack(String("TestEventListener::OnTestStart"));
  }

  virtual void OnNewTestPartResult(const TestPartResult& test_part_result) {
    g_events->PushBack(String("TestEventListener::OnNewTestPartResult"));
  }

  virtual void OnTestEnd(const TestInfo& test_info) {
    g_events->PushBack(String("TestEventListener::OnTestEnd"));
  }

  virtual void OnTestCaseEnd(const TestCase& test_case) {
    g_events->PushBack(String("TestEventListener::OnTestCaseEnd"));
  }

  virtual void OnGlobalTearDownStart(const UnitTest& unit_test) {
    g_events->PushBack(String("TestEventListener::OnGlobalTearDownStart"));
  }

  virtual void OnGlobalTearDownEnd(const UnitTest& unit_test) {
    g_events->PushBack(String("TestEventListener::OnGlobalTearDownEnd"));
  }

  virtual void OnUnitTestEnd(const UnitTest& unit_test) {
    g_events->PushBack(String("TestEventListener::OnUnitTestEnd"));
  }
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

int main(int argc, char **argv) {
  Vector<String> events;
  g_events = &events;
  InitGoogleTest(&argc, argv);

  UnitTestEventListenerInterface* listener = new EventRecordingListener;
  UnitTestAccessor::GetEventListeners().Append(listener);

  AddGlobalTestEnvironment(new EnvironmentInvocationCatcher);

  GTEST_CHECK_(events.size() == 0)
      << "AddGlobalTestEnvironment should not generate any events itself.";

  int ret_val = RUN_ALL_TESTS();

  const char* const expected_events[] = {
    "TestEventListener::OnUnitTestStart",
    "TestEventListener::OnGlobalSetUpStart",
    "Environment::SetUp",
    "TestEventListener::OnGlobalSetUpEnd",
    "TestEventListener::OnTestCaseStart",
    "ListenerTest::SetUpTestCase",
    "TestEventListener::OnTestStart",
    "ListenerTest::SetUp",
    "ListenerTest::* Test Body",
    "TestEventListener::OnNewTestPartResult",
    "ListenerTest::TearDown",
    "TestEventListener::OnTestEnd",
    "TestEventListener::OnTestStart",
    "ListenerTest::SetUp",
    "ListenerTest::* Test Body",
    "TestEventListener::OnNewTestPartResult",
    "ListenerTest::TearDown",
    "TestEventListener::OnTestEnd",
    "ListenerTest::TearDownTestCase",
    "TestEventListener::OnTestCaseEnd",
    "TestEventListener::OnGlobalTearDownStart",
    "Environment::TearDown",
    "TestEventListener::OnGlobalTearDownEnd",
    "TestEventListener::OnUnitTestEnd"
  };
  const int kExpectedEventsSize =
      sizeof(expected_events)/sizeof(expected_events[0]);

  // Cannot use ASSERT_EQ() here because it requires the scoping function to
  // return void.
  GTEST_CHECK_(events.size() == kExpectedEventsSize);

  for (int i = 0; i < events.size(); ++i)
    GTEST_CHECK_(String(events.GetElement(i)) == expected_events[i])
        << "At position " << i;

  // We need to check manually for ad hoc test failures that happen after
  // RUN_ALL_TESTS finishes.
  if (UnitTestAccessor::UnitTestFailed())
    ret_val = 1;

  return ret_val;
}
