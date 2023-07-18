// (c)2023 Sony Interactive Entertainment Inc
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
//     * Neither the name of Sony Interactive Entertainment Inc. nor the
// names of its contributors may be used to endorse or promote products
// derived from this software without specific prior written permission.
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

// The purpose of this file is to generate Google Test output for
// various "rotten green test" conditions (the test passes, but at
// least one test assertion was not executed).  The output will then be
// verified by gtest_rgt_output_test.py to ensure that Google Test
// generates the desired messages.  Therefore, many tests in this file
// are MEANT TO BE ROTTEN. Also, many are MEANT TO FAIL.
//
// Note: "assertions" here can mean either EXPECT_* or ASSERT_*.
//
// Modeled on googletest-output-test.

// TODO: Merge this with googletest-output-test? Only the rotten tests
// would need to be migrated.

#include "gtest/gtest.h"
#include "src/gtest-internal-inl.h"

#if GTEST_HAS_RGT

// Tests that will not be reported as rotten for various reasons.

TEST(NotRotten, EmptyIsntRotten) { }

TEST(NotRotten, DISABLED_DisabledIsntRotten) {
  EXPECT_EQ(0, 1);
}

TEST(NotRotten, SkippedIsntRotten) {
  if (testing::internal::AlwaysTrue()) {
    GTEST_SKIP();
  }
  EXPECT_EQ(1, 2);
}

TEST(NotRotten, AllPass) {
  EXPECT_EQ(0, 0);
  EXPECT_EQ(1, 1);
}

TEST(NotRotten, ExpectFailingIsntRotten) {
  EXPECT_EQ(0, 0);
  EXPECT_EQ(1, 2);
}

TEST(NotRotten, AssertFailingIsntRotten) {
  ASSERT_EQ(0, 1);
  EXPECT_EQ(2, 2);
}

// A test that fails isn't reported as rotten, even if it actually
// contains rotten assertions. Only passing (green) tests can be rotten.
TEST(NotRotten, ExpectFailingWithUnexecutedIsntRotten) {
  EXPECT_EQ(1, 2);
  if (testing::internal::AlwaysFalse()) {
    EXPECT_EQ(3, 3);
  }
}

// EXPECT_[NON]FATAL_FAILURE contains an assertion that fails.
// The bookkeeping for assertions will show the failing one as not executed,
// and it's contained within a passing test; but being expected, we shouldn't
// report it as rotten. We try various combinations of single and multiple
// lines because of the way macros think about what line they are on.
// (Which varies across compilers.)

TEST(NotRotten, SingleLineExpectNonfatalFailureIsntRotten) {
  EXPECT_NONFATAL_FAILURE({ EXPECT_EQ(1, 0) << "Non-fatal"; }, "Non-fatal");
}

TEST(NotRotten, DoubleLineExpectNonfatalFailureIsntRotten) {
  EXPECT_NONFATAL_FAILURE(
      { EXPECT_EQ(1, 0) << "Non-fatal"; }, "Non-fatal");
}

TEST(NotRotten, MultiLineExpectNonfatalFailureIsntRotten) {
  EXPECT_NONFATAL_FAILURE(
      {
        EXPECT_EQ(1, 0) << "Expected non-fatal failure.";
      },
      "Expected non-fatal failure.");
}

TEST(NotRotten, SingleLineExpectFatalFailureIsntRotten) {
  EXPECT_FATAL_FAILURE( { ASSERT_EQ(1, 0) << "Fatal"; }, "Fatal");
}

TEST(NotRotten, DoubleLineExpectFatalFailureIsntRotten) {
  EXPECT_FATAL_FAILURE(
      { ASSERT_EQ(1, 0) << "Fatal"; }, "Fatal");
}

TEST(NotRotten, MultiLineExpectFatalFailureIsntRotten) {
  EXPECT_FATAL_FAILURE(
      {
        ASSERT_EQ(1, 0) << "Expected non-fatal failure.";
      },
      "Expected non-fatal failure.");
}

// An EXPECT_[NON]FATAL_FAILURE that itself fails (doesn't detect an
// appropriate failure in the contained assertion) isn't rotten.

TEST(NotRotten, ExpectFatalFailureFails) {
  EXPECT_FATAL_FAILURE({ }, "Fatal failure fails");
  EXPECT_FATAL_FAILURE({ EXPECT_EQ(1, 1) << "Passes"; }, "Passes");
}

TEST(NotRotten, ExpectNonfatalFailureFails) {
  EXPECT_NONFATAL_FAILURE({ }, "Nonfatal failure fails");
  EXPECT_NONFATAL_FAILURE({ ASSERT_EQ(1, 2) << "Fails"; }, "Fails");
}

// ASSERT/EXPECT_NO_FATAL_FAILURE that is executed (pass or fail)
// isn't rotten.

void MustPass() { }
void MustFail() {
  ASSERT_EQ(1, 0);
}

TEST(NotRotten, ExpectNoFatalFailurePasses) {
  EXPECT_NO_FATAL_FAILURE(MustPass());
  ASSERT_NO_FATAL_FAILURE(MustPass());
}

TEST(NotRotten, ExpectNoFatalFailureFails) {
  EXPECT_NO_FATAL_FAILURE(MustFail());
  ASSERT_NO_FATAL_FAILURE(MustFail());
}

// Tests that will be reported as rotten.

TEST(IsRotten, SingleLineExpect) {
  if (testing::internal::AlwaysFalse()) {
    EXPECT_EQ(0, 0);
  }
}

// Show that GTEST_TEST is also handled correctly.
GTEST_TEST(IsRotten, MultiLineExpect) {
  if (testing::internal::AlwaysFalse()) {
    EXPECT_EQ(1,
              1);
  }
}

TEST(IsRotten, SingleLineAssert) {
  if (testing::internal::AlwaysFalse()) {
    ASSERT_EQ(0, 0);
  }
}

TEST(IsRotten, MultiLineAssert) {
  if (testing::internal::AlwaysFalse()) {
    ASSERT_EQ(1,
              1);
  }
}

TEST(IsRotten, MultipleRottenAssertions) {
  if (testing::internal::AlwaysFalse()) {
    EXPECT_EQ(0, 0);
  }
  if (testing::internal::AlwaysFalse()) {
    ASSERT_EQ(1,
              1);
  }
}

TEST(IsRotten, ExpectNoFatalFailure) {
  if (testing::internal::AlwaysFalse()) {
    EXPECT_NO_FATAL_FAILURE(MustPass());
    ASSERT_NO_FATAL_FAILURE(MustFail());
  }
}

// Test that RGT detection works correctly with fixtures.
// Just a few repeats from above tests, not everything.

class NotRottenFixture : public testing::Test {};
class IsRottenFixture : public testing::Test {};

TEST_F(NotRottenFixture, ExpectFailureIsntRotten) {
  EXPECT_EQ(0, 1);
}

TEST_F(NotRottenFixture, AssertFailureIsntRotten) {
  ASSERT_EQ(0, 1);
}

// Show that GTEST_TEST_F is also handled correctly.
GTEST_TEST_F(IsRottenFixture, SingleLineExpect) {
  if (testing::internal::AlwaysFalse()) {
    EXPECT_EQ(0, 0);
  }
}

TEST_F(IsRottenFixture, MultiLineAssert) {
  if (testing::internal::AlwaysFalse()) {
    ASSERT_EQ(1,
              1);
  }
}

// Test that RGT detection works correctly with parameterized tests.

class NotRottenParamTest : public testing::TestWithParam<int> {};

TEST_P(NotRottenParamTest, Passes) { EXPECT_EQ(1, GetParam()); }

TEST_P(NotRottenParamTest, AssertFailingIsntRotten) {
  ASSERT_EQ(2, GetParam());
  EXPECT_EQ(1, GetParam());
}

TEST_P(NotRottenParamTest, FailingIsntRotten) {
  EXPECT_EQ(2, GetParam());
  if (testing::internal::AlwaysFalse()) {
    EXPECT_EQ(1, GetParam());
  }
}

INSTANTIATE_TEST_SUITE_P(, NotRottenParamTest, testing::Values(1));

class IsRottenParamTest : public testing::TestWithParam<int> {};

TEST_P(IsRottenParamTest, SingleExpect) {
  if (testing::internal::AlwaysFalse()) {
    EXPECT_EQ(0, GetParam());
  }
}

// If the assertions are conditional on the parameter, they aren't all
// executed on each iteration.
// TODO: Whether that's reported as rotten depends on when we do the reporting.

TEST_P(IsRottenParamTest, ConditionalOnParam) {
  if (GetParam() == 1) {
    EXPECT_EQ(1, 1);
  } else {
    EXPECT_EQ(2, 2);
  }
}

INSTANTIATE_TEST_SUITE_P(, IsRottenParamTest, testing::Values(1, 2));

// Test whether RGT reporting works correctly with typed tests.

using TypedTestTypes = testing::Types<char, int>;

template <typename T>
class NotRottenTypedTest : public testing::Test {};
TYPED_TEST_SUITE(NotRottenTypedTest, TypedTestTypes);

TYPED_TEST(NotRottenTypedTest, SuccessIsntRotten) {
  EXPECT_EQ(0, TypeParam());
}

TYPED_TEST(NotRottenTypedTest, FailureIsntRotten) {
  EXPECT_EQ(1, TypeParam());
}

template <typename T>
class IsRottenTypedTest : public testing::Test {};
TYPED_TEST_SUITE(IsRottenTypedTest, TypedTestTypes);

// If the assertions are conditional on the parameter, they aren't all
// executed in each instantiation. Unfortunately this means we'll see
// false-positive reports until enough instantiations run to cover all
// the conditions. This is a known issue.

TYPED_TEST(IsRottenTypedTest, ConditionalOnType) {
  if (std::is_same<TypeParam, char>::value) {
    EXPECT_EQ(0, 0);
  } else {
    EXPECT_EQ(1, 1);
  }
}

// Test whether RGT reporting works correctly with type-parameterized tests.

template <typename T>
class NotRottenTypeParamTest : public testing::Test {};
TYPED_TEST_SUITE_P(NotRottenTypeParamTest);

TYPED_TEST_P(NotRottenTypeParamTest, SuccessIsntRotten) {
  EXPECT_EQ(0, TypeParam());
}

TYPED_TEST_P(NotRottenTypeParamTest, FailureIsntRotten) {
  EXPECT_EQ(1, TypeParam());
}

REGISTER_TYPED_TEST_SUITE_P(NotRottenTypeParamTest,
                            SuccessIsntRotten, FailureIsntRotten);
INSTANTIATE_TYPED_TEST_SUITE_P(My, NotRottenTypeParamTest,
                               TypedTestTypes);

template <typename T>
class IsRottenTypeParamTest : public testing::Test {};
TYPED_TEST_SUITE_P(IsRottenTypeParamTest);

// If the assertions are conditional on the parameter, they aren't all
// executed in each instantiation. Unfortunately this means we'll see
// false-positive reports until enough instantiations run to cover all
// the conditions. This is a known issue.

TYPED_TEST_P(IsRottenTypeParamTest, ConditionalOnType) {
  if (std::is_same<TypeParam, char>::value) {
    EXPECT_EQ(0, 0);
  } else {
    EXPECT_EQ(1, 1);
  }
}

REGISTER_TYPED_TEST_SUITE_P(IsRottenTypeParamTest,
                            ConditionalOnType);
INSTANTIATE_TYPED_TEST_SUITE_P(My, IsRottenTypeParamTest,
                               TypedTestTypes);

#endif // GTEST_HAS_RGT
