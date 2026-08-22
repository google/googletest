// Copyright 2025, Google Inc.
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

// Tests GOOGLEMOCK_REQUIRE_STRICT_OR_NICE / GMOCK_FORCE_EXPLICIT_MOCK_KIND
// (https://github.com/google/googletest/issues/4882).

#define GOOGLEMOCK_REQUIRE_STRICT_OR_NICE 1

#include "gmock/gmock.h"
#include "gtest/gtest-spi.h"
#include "gtest/gtest.h"

namespace {

using ::testing::NiceMock;
using ::testing::StrictMock;

class MockFoo {
 public:
  MOCK_METHOD(void, DoThis, ());
};

class MockFooDefaultNice {
 public:
  DEFAULT_TO_NICE();
  MOCK_METHOD(void, DoThis, ());
};

class MockFooDefaultStrict {
 public:
  DEFAULT_TO_STRICT();
  MOCK_METHOD(void, DoThis, ());
};

TEST(RequireStrictOrNiceTest, BareMockFailsOnCall) {
  MockFoo mock;
  EXPECT_FATAL_FAILURE(mock.DoThis(), "GOOGLEMOCK_REQUIRE_STRICT_OR_NICE");
}

TEST(RequireStrictOrNiceTest, BareMockFailsOnExpectCall) {
  MockFoo mock;
  EXPECT_FATAL_FAILURE(
      {
        EXPECT_CALL(mock, DoThis());
      },
      "GOOGLEMOCK_REQUIRE_STRICT_OR_NICE");
}

TEST(RequireStrictOrNiceTest, NiceMockAllowed) {
  NiceMock<MockFoo> mock;
  EXPECT_CALL(mock, DoThis());
  mock.DoThis();
  EXPECT_TRUE(testing::Mock::IsNice(&mock));
}

TEST(RequireStrictOrNiceTest, StrictMockAllowed) {
  StrictMock<MockFoo> mock;
  EXPECT_CALL(mock, DoThis());
  mock.DoThis();
  EXPECT_TRUE(testing::Mock::IsStrict(&mock));
}

TEST(RequireStrictOrNiceTest, DefaultToNiceAllowed) {
  MockFooDefaultNice mock;
  mock.DoThis();  // uninteresting, but nice
  EXPECT_TRUE(testing::Mock::IsNice(&mock));
}

TEST(RequireStrictOrNiceTest, DefaultToStrictAllowed) {
  MockFooDefaultStrict mock;
  EXPECT_CALL(mock, DoThis());
  mock.DoThis();
  EXPECT_TRUE(testing::Mock::IsStrict(&mock));
}

}  // namespace
