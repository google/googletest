// Copyright 2005, Google Inc.
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

// Unit test for Google Test test filters.
//
// A user can specify which test(s) in a Google Test program to run via
// either the GTEST_FILTER environment variable or the --gtest_filter
// flag.  This is used for testing such functionality.
//
// The program will be invoked from a Python unit test.  Don't run it
// directly.

#include "gtest/gtest.h"

namespace {

// Test HasFixtureTest.

class HasFixtureTest : public testing::Test {};

TEST_F(HasFixtureTest, Test0) {}

TEST_F(HasFixtureTest, Test1) { FAIL() << "Expected failure."; }

TEST_F(HasFixtureTest, Test2) { FAIL() << "Expected failure."; }

TEST_F(HasFixtureTest, Test3) { FAIL() << "Expected failure."; }

TEST_F(HasFixtureTest, Test4) { FAIL() << "Expected failure."; }

// Test HasSimpleTest.

TEST(HasSimpleTest, Test0) {}

TEST(HasSimpleTest, Test1) { FAIL() << "Expected failure."; }

TEST(HasSimpleTest, Test2) { FAIL() << "Expected failure."; }

TEST(HasSimpleTest, Test3) { FAIL() << "Expected failure."; }

TEST(HasSimpleTest, Test4) { FAIL() << "Expected failure."; }

// Test HasDisabledTest.

TEST(HasDisabledTest, Test0) {}

TEST(HasDisabledTest, DISABLED_Test1) { FAIL() << "Expected failure."; }

TEST(HasDisabledTest, Test2) { FAIL() << "Expected failure."; }

TEST(HasDisabledTest, Test3) { FAIL() << "Expected failure."; }

TEST(HasDisabledTest, Test4) { FAIL() << "Expected failure."; }

// Test HasDeathTest

TEST(HasDeathTest, Test0) { EXPECT_DEATH_IF_SUPPORTED(exit(1), ".*"); }

TEST(HasDeathTest, Test1) {
  EXPECT_DEATH_IF_SUPPORTED(FAIL() << "Expected failure.", ".*");
}

TEST(HasDeathTest, Test2) {
  EXPECT_DEATH_IF_SUPPORTED(FAIL() << "Expected failure.", ".*");
}

TEST(HasDeathTest, Test3) {
  EXPECT_DEATH_IF_SUPPORTED(FAIL() << "Expected failure.", ".*");
}

TEST(HasDeathTest, Test4) {
  EXPECT_DEATH_IF_SUPPORTED(FAIL() << "Expected failure.", ".*");
}

// Test DISABLED_HasDisabledSuite

TEST(DISABLED_HasDisabledSuite, Test0) {}

TEST(DISABLED_HasDisabledSuite, Test1) { FAIL() << "Expected failure."; }

TEST(DISABLED_HasDisabledSuite, Test2) { FAIL() << "Expected failure."; }

TEST(DISABLED_HasDisabledSuite, Test3) { FAIL() << "Expected failure."; }

TEST(DISABLED_HasDisabledSuite, Test4) { FAIL() << "Expected failure."; }

// Test HasParametersTest

class HasParametersTest : public testing::TestWithParam<int> {};

TEST_P(HasParametersTest, Test1) { FAIL() << "Expected failure."; }

TEST_P(HasParametersTest, Test2) { FAIL() << "Expected failure."; }

INSTANTIATE_TEST_SUITE_P(HasParametersSuite, HasParametersTest,
                         testing::Values(1, 2));

class MyTestListener : public ::testing::EmptyTestEventListener {
  void OnTestSuiteStart(const ::testing::TestSuite& test_suite) override {
    printf("We are in OnTestSuiteStart of %s.\n", test_suite.name());
  }

  void OnTestStart(const ::testing::TestInfo& test_info) override {
    printf("We are in OnTestStart of %s.%s.\n", test_info.test_suite_name(),
           test_info.name());
  }

  void OnTestPartResult(
      const ::testing::TestPartResult& test_part_result) override {
    printf("We are in OnTestPartResult %s:%d.\n", test_part_result.file_name(),
           test_part_result.line_number());
  }

  void OnTestEnd(const ::testing::TestInfo& test_info) override {
    printf("We are in OnTestEnd of %s.%s.\n", test_info.test_suite_name(),
           test_info.name());
  }

  void OnTestSuiteEnd(const ::testing::TestSuite& test_suite) override {
    printf("We are in OnTestSuiteEnd of %s.\n", test_suite.name());
  }
};

TEST(HasSkipTest, Test0) { SUCCEED() << "Expected success."; }

TEST(HasSkipTest, Test1) { GTEST_SKIP() << "Expected skip."; }

TEST(HasSkipTest, Test2) { FAIL() << "Expected failure."; }

TEST(HasSkipTest, Test3) { FAIL() << "Expected failure."; }

TEST(HasSkipTest, Test4) { FAIL() << "Expected failure."; }

}  // namespace

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::testing::UnitTest::GetInstance()->listeners().Append(new MyTestListener());
  return RUN_ALL_TESTS();
}
