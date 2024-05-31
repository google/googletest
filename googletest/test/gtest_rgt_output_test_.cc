// Copyright (C) 2024 Sony Interactive Entertainment Inc.
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

// The purpose of this file is to generate Google Test output for various
// "rotten green test" conditions. In a "rotten" test, the test passes, but
// at least one test assertion was not executed. The output will then be
// verified by gtest_rgt_output_test.py to ensure that Google Test generates
// the desired messages. Therefore, many tests in this file are MEANT TO BE
// ROTTEN.
//
// However, NONE ARE MEANT TO FAIL. If the test has an overall failing result,
// the helper methods won't have their rotten assertions reported, and that is
// something we do want to verify. Instead, googletest-ouput-test shows that
// (a) an overall Fail result means that rotten helper methods aren't reported;
// (b) failing assertions are not reported as rotten.
//
// This test shows (c) rotten assertions (and rotten helpers) in a passing test
// are reported properly.
//
// Note: "assertions" here can mean either EXPECT_* or ASSERT_*.
//
// Modeled on googletest-output-test.

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

// EXPECT_[NON]FATAL_FAILURE executes an assertion that is supposed to fail.
// The bookkeeping for assertions will show the failing one as not executed,
// but because it's containined in a Test that passes, we shouldn't report it.

TEST(NotRotten, ExpectNonfatalFailureIsntRotten) {
  EXPECT_NONFATAL_FAILURE({ EXPECT_EQ(1, 0) << "Non-fatal"; }, "Non-fatal");
}

TEST(NotRotten, ExpectFatalFailureIsntRotten) {
  EXPECT_FATAL_FAILURE( { ASSERT_EQ(1, 0) << "Fatal"; }, "Fatal");
}

// ASSERT/EXPECT_NO_FATAL_FAILURE that is executed (pass or fail) isn't rotten.

void MustPass() { }

TEST(NotRotten, ExpectNoFatalFailurePasses) {
  EXPECT_NO_FATAL_FAILURE(MustPass());
  ASSERT_NO_FATAL_FAILURE(MustPass());
}

// As an unfortunate consequence of how EXPECT_[NON]FATAL_FAILURE is handled,
// if one is executed, it disables rotten detection entirely within the
// containing Test. It would be nice to fix that someday.
TEST(NotRotten, ExpectNonfatalFailureSadlyDisablesRotten) {
  EXPECT_NONFATAL_FAILURE( { EXPECT_EQ(1, 0) << "Non-fatal"; }, "Non-fatal");
  if (testing::internal::AlwaysFalse()) {
    EXPECT_EQ(1, 0);
  }
}

TEST(NotRotten, ExpectFatalFailureSadlyDisablesRotten) {
  EXPECT_FATAL_FAILURE( { ASSERT_EQ(1, 0) << "Fatal"; }, "Fatal");
  if (testing::internal::AlwaysFalse()) {
    EXPECT_EQ(1, 0);
  }
}

// Tests that will be reported as rotten.

TEST(IsRotten, SimpleExpect) {
  if (testing::internal::AlwaysFalse()) {
    EXPECT_EQ(0, 0);
  }
}

TEST(IsRotten, SimpleAssert) {
  if (testing::internal::AlwaysFalse()) {
    ASSERT_EQ(0, 0);
  }
}

TEST(IsRotten, OneIsEnough) {
  EXPECT_EQ(0, 0);
  if (testing::internal::AlwaysFalse()) {
    ASSERT_EQ(1, 1);
  }
}

// If EXPECT_[NON]FATAL_FAILURE exists, but isn't executed, that is detected.
TEST(IsRotten, MissedExpectFailure) {
  if (testing::internal::AlwaysFalse()) {
    EXPECT_NONFATAL_FAILURE( { EXPECT_EQ(1, 0) << "Non-fatal"; }, "Non-fatal");
    EXPECT_FATAL_FAILURE( { ASSERT_EQ(1, 0) << "Fatal"; }, "Fatal");
  }
}

TEST(IsRotten, ExpectNoFatalFailure) {
  if (testing::internal::AlwaysFalse()) {
    EXPECT_NO_FATAL_FAILURE(MustPass());
    ASSERT_NO_FATAL_FAILURE(MustPass());
  }
}

void RottenHelperNeverCalled() {
  EXPECT_EQ(0, 0);
}

// Test that RGT detection works correctly with fixtures.
// Just a few sample repeats, not everything.

class NotRottenFixture : public testing::Test {};
class IsRottenFixture : public testing::Test {};

TEST_F(NotRottenFixture, AllPass) {
  EXPECT_EQ(0, 0);
  ASSERT_EQ(1, 1);
}

TEST_F(NotRottenFixture, SkippedIsntRotten) {
  if (testing::internal::AlwaysTrue()) {
    GTEST_SKIP();
  }
  ASSERT_EQ(0, 1);
}

TEST_F(IsRottenFixture, SingleExpect) {
  if (testing::internal::AlwaysFalse()) {
    EXPECT_EQ(0, 0);
  }
}

// RGT and Parameterized Tests.

// TEST_P has one broad issue with respect to RGT.
//
// Rotten test detection relies on having compile-time access to the TestInfo*
// for each test. TEST_P doesn't have that. As a consequence, in RGT these
// tests behave like helper methods. Lacking any better information, RGT
// reporting assumes that all helper assertions should be executed.
// Unfortunately this means the usual things that disable rotten detection
// (disabled, skipped, EXPECT_[NON]FATAL_FAILURE) are ineffective.
// These tests will therefore be prone to false positives.
//
// However, that behavior is consistent, so we include it in the output test.

class NotRottenParamTest : public testing::TestWithParam<int> {};

TEST_P(NotRottenParamTest, Passes) { EXPECT_GE(2, GetParam()); }

// If the assertions are conditional on the parameter, they aren't all executed
// on each iteration. Because we treat these like helpers, we report them at
// the end of the test, and if every path was eventually taken, we're fine.
// If we support parameterized tests properly in the future, we should continue
// not reporting until all iterations have executed, to avoid false positives.

TEST_P(NotRottenParamTest, ConditionalOnParam) {
  if (GetParam() == 1) {
    EXPECT_EQ(1, 1);
  } else {
    EXPECT_EQ(2, 2);
  }
}

INSTANTIATE_TEST_SUITE_P(, NotRottenParamTest, testing::Values(1, 2));

class IsRottenParamTest : public testing::TestWithParam<int> {};

TEST_P(IsRottenParamTest, SkippedIsSadlyRotten) {
  if (testing::internal::AlwaysTrue()) {
    GTEST_SKIP();
  }
  EXPECT_EQ(0, GetParam());
}

TEST_P(IsRottenParamTest, DISABLED_DisabledIsSadlyRotten) {
  EXPECT_EQ(0, GetParam());
}

TEST_P(IsRottenParamTest, ActuallyRotten) {
  switch (GetParam()) {
  case 0:
    EXPECT_EQ(0, 0);
    break;
  case 1:
    EXPECT_EQ(1, 1);
    break;
  default:
    EXPECT_EQ(2, 2);
    break;
  }
}

INSTANTIATE_TEST_SUITE_P(, IsRottenParamTest, testing::Values(1, 2));

// RGT and Typed Tests.

// TYPED_TEST and TYPED_TEST_P have two broad issues with respect to RGT.
//
// 1) Rotten test detection relies on having compile-time access to the
//    TestInfo* for each test. TYPED_TEST[_P] don't have that. As a
//    consequence, in RGT they behave like helper methods. Lacking any
//    better information, RGT reporting assumes that all helper assertions
//    should be executed. Unfortunately this means the usual things that
//    disable rotten detection (disabled, skipped, EXPECT_[NON]FATAL_FAILURE)
//    are ineffective.
//    These tests will therefore be prone to false positives.
//
// 2) Assertion bookkeeping relies on certain static data being allocated to
//    particular object-file sections. GCC prior to GCC 14 (and sometimes,
//    even in GCC 14) does not obey section attributes for static data in a
//    template (or 'inline') function.
//    See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=94342.
//    TYPED_TEST[_P] are implemented as templates, so the assertions info is
//    effectively lost with older GCC.
//    These tests will therefore be prone to false negatives.
//
// The first issue is a problem regardless of the test environment. If that
// were the only problem it would be fine to verify the behavior of these
// tests. (We do this with TEST_P, which has the same issue.)
//
// The second issue is specific to GCC prior to GCC 14. Being an environmental
// issue (e.g., results with GCC and Clang will differ), we need to handle it
// in a way that allows this test to pass for everyone.
//
// Therefore, we disable RGT output testing for TYPED_TEST and TYPED_TEST_P,
// unless GTEST_DEBUG_RGT is enabled. This allows users to optionally exercise
// these tests and make sure no Bad Stuff happens, but by default users aren't
// affected.

#if GTEST_DEBUG_RGT

using TypedTestTypes = testing::Types<char, int>;

template <typename T>
class NotRottenTypedTest : public testing::Test {};
TYPED_TEST_SUITE(NotRottenTypedTest, TypedTestTypes);

TYPED_TEST(NotRottenTypedTest, Passes) { EXPECT_EQ(0, TypeParam()); }

// If the assertions are conditional on the parameter, they aren't all
// executed in each instantiation. Because we treat these like helpers,
// and all the instantiations are deduplicated for reporting purposes,
// we report at the end of the test, and it's clean. If we support
// typed tests properly, we should continue not reporting until all
// instantiations have executed, to avoid false positives.

TYPED_TEST(NotRottenTypedTest, ConditionalOnType) {
  if (std::is_same<TypeParam, char>::value) {
    EXPECT_EQ(0, 0);
  } else {
    EXPECT_EQ(1, 1);
  }
}

template <typename T>
class IsRottenTypedTest : public testing::Test {};
TYPED_TEST_SUITE(IsRottenTypedTest, TypedTestTypes);

TYPED_TEST(IsRottenTypedTest, SkippedIsSadlyRotten) {
  assert(gtest_test_info_ == nullptr);
  if (testing::internal::AlwaysTrue()) {
    GTEST_SKIP();
  }
  EXPECT_EQ(0, 1);
}

TYPED_TEST(IsRottenTypedTest, DISABLED_DisabledIsSadlyRotten) {
  EXPECT_EQ(0, 1);
}

TYPED_TEST(IsRottenTypedTest, ActuallyRotten) {
  if (testing::internal::AlwaysFalse()) {
    EXPECT_EQ(0, 0);
  }
}

// RGT and Typed Parameterized Tests.

// See the note above about general issues with TYPED_TEST_P.

template <typename T>
class NotRottenTypeParamTest : public testing::Test {};
TYPED_TEST_SUITE_P(NotRottenTypeParamTest);

TYPED_TEST_P(NotRottenTypeParamTest, Passes) { EXPECT_EQ(0, TypeParam()); }

// If the assertions are conditional on the parameter, they aren't all
// executed in each instantiation. Because we treat these like helpers,
// and all the instantiations are deduplicated for reporting purposes,
// we report at the end of the test, and it's clean. If we support
// typed tests properly, we should continue not reporting until all
// instantiations have executed, to avoid false positives.

TYPED_TEST_P(NotRottenTypeParamTest, ConditionalOnType) {
  if (std::is_same<TypeParam, char>::value) {
    EXPECT_EQ(0, 0);
  } else {
    EXPECT_EQ(1, 1);
  }
}

REGISTER_TYPED_TEST_SUITE_P(NotRottenTypeParamTest,
                            Passes, ConditionalOnType);
INSTANTIATE_TYPED_TEST_SUITE_P(My, NotRottenTypeParamTest,
                               TypedTestTypes);

template <typename T>
class IsRottenTypeParamTest : public testing::Test {};
TYPED_TEST_SUITE_P(IsRottenTypeParamTest);

TYPED_TEST_P(IsRottenTypeParamTest, SkippedIsSadlyRotten) {
  if (testing::internal::AlwaysTrue()) {
    GTEST_SKIP();
  }
  EXPECT_EQ(0, 1);
}

TYPED_TEST_P(IsRottenTypeParamTest, DISABLED_DisabledIsSadlyRotten) {
  EXPECT_EQ(0, 1);
}

TYPED_TEST_P(IsRottenTypeParamTest, ActuallyRotten) {
  if (testing::internal::AlwaysFalse()) {
    EXPECT_EQ(0, 0);
  }
}

REGISTER_TYPED_TEST_SUITE_P(IsRottenTypeParamTest,
                            SkippedIsSadlyRotten,
                            DISABLED_DisabledIsSadlyRotten,
                            ActuallyRotten);
INSTANTIATE_TYPED_TEST_SUITE_P(My, IsRottenTypeParamTest,
                               TypedTestTypes);

#endif // GTEST_DEBUG_RGT

#endif // GTEST_HAS_RGT
