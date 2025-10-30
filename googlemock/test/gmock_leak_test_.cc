// Copyright 2009, Google Inc.
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

// Google Mock - a framework for writing C++ mock classes.
//
// This program is for verifying that a leaked mock object can be
// caught by Google Mock's leak detector.

#include "gmock/gmock.h"

namespace {

using ::testing::Return;

class FooInterface {
 public:
  virtual ~FooInterface() = default;
  virtual void DoThis() = 0;
};

class MockFoo : public FooInterface {
 public:
  MockFoo() = default;

  MOCK_METHOD0(DoThis, void());

 private:
  MockFoo(const MockFoo&) = delete;
  MockFoo& operator=(const MockFoo&) = delete;
};

TEST(LeakTest, LeakedMockWithExpectCallCausesFailureWhenLeakCheckingIsEnabled) {
  MockFoo* foo = new MockFoo;

  EXPECT_CALL(*foo, DoThis());
  foo->DoThis();

  // In order to test the leak detector, we deliberately leak foo.

  // Makes sure Google Mock's leak detector can change the exit code
  // to 1 even when the code is already exiting with 0.
  exit(0);
}

TEST(LeakTest, LeakedMockWithOnCallCausesFailureWhenLeakCheckingIsEnabled) {
  MockFoo* foo = new MockFoo;

  ON_CALL(*foo, DoThis()).WillByDefault(Return());

  // In order to test the leak detector, we deliberately leak foo.

  // Makes sure Google Mock's leak detector can change the exit code
  // to 1 even when the code is already exiting with 0.
  exit(0);
}

TEST(LeakTest, CatchesMultipleLeakedMockObjects) {
  MockFoo* foo1 = new MockFoo;
  MockFoo* foo2 = new MockFoo;

  ON_CALL(*foo1, DoThis()).WillByDefault(Return());
  EXPECT_CALL(*foo2, DoThis());
  foo2->DoThis();

  // In order to test the leak detector, we deliberately leak foo1 and
  // foo2.

  // Makes sure Google Mock's leak detector can change the exit code
  // to 1 even when the code is already exiting with 0.
  exit(0);
}

TEST(LeakTest, InstantNoLeak) {
  MockFoo* foo = new MockFoo;

  EXPECT_CALL(*foo, DoThis());
  foo->DoThis();

  delete foo;
  // Since foo is properly deleted instant leak check should not see a leaked
  // mock object and therefore not fail the test.
  testing::Mock::CheckLeakInstant();
}

TEST(LeakTest, InstantLeak) {
  MockFoo* foo = new MockFoo;

  EXPECT_CALL(*foo, DoThis());
  foo->DoThis();

  // At this point foo is still allocated. Calling the instant leak check should
  // detect it and fail the test.
  testing::Mock::CheckLeakInstant();

  // Free foo in order to not fail the end of program leak check.
  delete foo;
}

TEST(LeakTest, InstantNoLeakAllowed) {
  MockFoo* foo = new MockFoo;
  testing::Mock::AllowLeak(foo);

  EXPECT_CALL(*foo, DoThis());
  foo->DoThis();

  // At this point foo is still allocated However since we made foo a leakable
  // mock object with AllowLeak() the instant leak check should ignore it and
  // pass the test.
  testing::Mock::CheckLeakInstant();

  // Free foo in order to not fail the end of program leak check.
  delete foo;
}

}  // namespace
