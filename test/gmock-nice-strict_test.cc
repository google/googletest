// Copyright 2008, Google Inc.
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

#include <gmock/gmock-generated-nice-strict.h>

#include <string>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <gtest/gtest-spi.h>

namespace testing {
namespace gmock_nice_strict_test {

using testing::internal::string;
using testing::GMOCK_FLAG(verbose);
using testing::HasSubstr;
using testing::NiceMock;
using testing::StrictMock;

// Defines some mock classes needed by the tests.

class Foo {
 public:
  virtual ~Foo() {}

  virtual void DoThis() = 0;
  virtual int DoThat(bool flag) = 0;
};

class MockFoo : public Foo {
 public:
  void Delete() { delete this; }

  MOCK_METHOD0(DoThis, void());
  MOCK_METHOD1(DoThat, int(bool flag));
};

class MockBar {
 public:
  explicit MockBar(const string& s) : str_(s) {}

  MockBar(char a1, char a2, string a3, string a4, int a5, int a6,
          const string& a7, const string& a8, bool a9, bool a10) {
    str_ = string() + a1 + a2 + a3 + a4 + static_cast<char>(a5) +
        static_cast<char>(a6) + a7 + a8 + (a9 ? 'T' : 'F') + (a10 ? 'T' : 'F');
  }

  virtual ~MockBar() {}

  const string& str() const { return str_; }

  MOCK_METHOD0(This, int());
  MOCK_METHOD2(That, string(int, bool));

 private:
  string str_;
};

// TODO(wan@google.com): find a way to re-enable these tests.
#if 0

// Tests that a nice mock generates no warning for uninteresting calls.
TEST(NiceMockTest, NoWarningForUninterestingCall) {
  NiceMock<MockFoo> nice_foo;

  CaptureTestStdout();
  nice_foo.DoThis();
  nice_foo.DoThat(true);
  EXPECT_EQ("", GetCapturedTestStdout());
}

// Tests that a nice mock generates no warning for uninteresting calls
// that delete the mock object.
TEST(NiceMockTest, NoWarningForUninterestingCallAfterDeath) {
  NiceMock<MockFoo>* const nice_foo = new NiceMock<MockFoo>;

  ON_CALL(*nice_foo, DoThis())
      .WillByDefault(Invoke(nice_foo, &MockFoo::Delete));

  CaptureTestStdout();
  nice_foo->DoThis();
  EXPECT_EQ("", GetCapturedTestStdout());
}

// Tests that a nice mock generates informational logs for
// uninteresting calls.
TEST(NiceMockTest, InfoForUninterestingCall) {
  NiceMock<MockFoo> nice_foo;

  GMOCK_FLAG(verbose) = "info";
  CaptureTestStdout();
  nice_foo.DoThis();
  EXPECT_THAT(GetCapturedTestStdout(),
              HasSubstr("Uninteresting mock function call"));

  CaptureTestStdout();
  nice_foo.DoThat(true);
  EXPECT_THAT(GetCapturedTestStdout(),
              HasSubstr("Uninteresting mock function call"));
}

#endif  // 0

// Tests that a nice mock allows expected calls.
TEST(NiceMockTest, AllowsExpectedCall) {
  NiceMock<MockFoo> nice_foo;

  EXPECT_CALL(nice_foo, DoThis());
  nice_foo.DoThis();
}

// Tests that an unexpected call on a nice mock fails.
TEST(NiceMockTest, UnexpectedCallFails) {
  NiceMock<MockFoo> nice_foo;

  EXPECT_CALL(nice_foo, DoThis()).Times(0);
  EXPECT_NONFATAL_FAILURE(nice_foo.DoThis(), "called more times than expected");
}

// Tests that NiceMock works with a mock class that has a non-default
// constructor.
TEST(NiceMockTest, NonDefaultConstructor) {
  NiceMock<MockBar> nice_bar("hi");
  EXPECT_EQ("hi", nice_bar.str());

  nice_bar.This();
  nice_bar.That(5, true);
}

// Tests that NiceMock works with a mock class that has a 10-ary
// non-default constructor.
TEST(NiceMockTest, NonDefaultConstructor10) {
  NiceMock<MockBar> nice_bar('a', 'b', "c", "d", 'e', 'f',
                             "g", "h", true, false);
  EXPECT_EQ("abcdefghTF", nice_bar.str());

  nice_bar.This();
  nice_bar.That(5, true);
}

// Tests that a strict mock allows expected calls.
TEST(StrictMockTest, AllowsExpectedCall) {
  StrictMock<MockFoo> strict_foo;

  EXPECT_CALL(strict_foo, DoThis());
  strict_foo.DoThis();
}

// Tests that an unexpected call on a strict mock fails.
TEST(StrictMockTest, UnexpectedCallFails) {
  StrictMock<MockFoo> strict_foo;

  EXPECT_CALL(strict_foo, DoThis()).Times(0);
  EXPECT_NONFATAL_FAILURE(strict_foo.DoThis(),
                          "called more times than expected");
}

// Tests that an uninteresting call on a strict mock fails.
TEST(StrictMockTest, UninterestingCallFails) {
  StrictMock<MockFoo> strict_foo;

  EXPECT_NONFATAL_FAILURE(strict_foo.DoThis(),
                          "Uninteresting mock function call");
}

// Tests that an uninteresting call on a strict mock fails, even if
// the call deletes the mock object.
TEST(StrictMockTest, UninterestingCallFailsAfterDeath) {
  StrictMock<MockFoo>* const strict_foo = new StrictMock<MockFoo>;

  ON_CALL(*strict_foo, DoThis())
      .WillByDefault(Invoke(strict_foo, &MockFoo::Delete));

  EXPECT_NONFATAL_FAILURE(strict_foo->DoThis(),
                          "Uninteresting mock function call");
}

// Tests that StrictMock works with a mock class that has a
// non-default constructor.
TEST(StrictMockTest, NonDefaultConstructor) {
  StrictMock<MockBar> strict_bar("hi");
  EXPECT_EQ("hi", strict_bar.str());

  EXPECT_NONFATAL_FAILURE(strict_bar.That(5, true),
                          "Uninteresting mock function call");
}

// Tests that StrictMock works with a mock class that has a 10-ary
// non-default constructor.
TEST(StrictMockTest, NonDefaultConstructor10) {
  StrictMock<MockBar> strict_bar('a', 'b', "c", "d", 'e', 'f',
                                 "g", "h", true, false);
  EXPECT_EQ("abcdefghTF", strict_bar.str());

  EXPECT_NONFATAL_FAILURE(strict_bar.That(5, true),
                          "Uninteresting mock function call");
}

}  // namespace gmock_nice_strict_test
}  // namespace testing
