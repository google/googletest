// Copyright 2008 Google Inc. All Rights Reserved.
// Author: mheule@google.com (Markus Heule)

#include <gtest/gtest-test-part.h>

#include <gtest/gtest.h>

using testing::Test;
using testing::TestPartResult;
using testing::TestPartResultArray;

using testing::TPRT_FATAL_FAILURE;
using testing::TPRT_NONFATAL_FAILURE;
using testing::TPRT_SUCCESS;

namespace {

// Tests the TestPartResult class.

// The test fixture for testing TestPartResult.
class TestPartResultTest : public Test {
 protected:
  TestPartResultTest()
      : r1_(TPRT_SUCCESS, "foo/bar.cc", 10, "Success!"),
        r2_(TPRT_NONFATAL_FAILURE, "foo/bar.cc", -1, "Failure!"),
        r3_(TPRT_FATAL_FAILURE, NULL, -1, "Failure!") {}

  TestPartResult r1_, r2_, r3_;
};

// Tests TestPartResult::type().
TEST_F(TestPartResultTest, type) {
  EXPECT_EQ(TPRT_SUCCESS, r1_.type());
  EXPECT_EQ(TPRT_NONFATAL_FAILURE, r2_.type());
  EXPECT_EQ(TPRT_FATAL_FAILURE, r3_.type());
}

// Tests TestPartResult::file_name().
TEST_F(TestPartResultTest, file_name) {
  EXPECT_STREQ("foo/bar.cc", r1_.file_name());
  EXPECT_STREQ(NULL, r3_.file_name());
}

// Tests TestPartResult::line_number().
TEST_F(TestPartResultTest, line_number) {
  EXPECT_EQ(10, r1_.line_number());
  EXPECT_EQ(-1, r2_.line_number());
}

// Tests TestPartResult::message().
TEST_F(TestPartResultTest, message) {
  EXPECT_STREQ("Success!", r1_.message());
}

// Tests TestPartResult::passed().
TEST_F(TestPartResultTest, Passed) {
  EXPECT_TRUE(r1_.passed());
  EXPECT_FALSE(r2_.passed());
  EXPECT_FALSE(r3_.passed());
}

// Tests TestPartResult::failed().
TEST_F(TestPartResultTest, Failed) {
  EXPECT_FALSE(r1_.failed());
  EXPECT_TRUE(r2_.failed());
  EXPECT_TRUE(r3_.failed());
}

// Tests TestPartResult::fatally_failed().
TEST_F(TestPartResultTest, FatallyFailed) {
  EXPECT_FALSE(r1_.fatally_failed());
  EXPECT_FALSE(r2_.fatally_failed());
  EXPECT_TRUE(r3_.fatally_failed());
}

// Tests TestPartResult::nonfatally_failed().
TEST_F(TestPartResultTest, NonfatallyFailed) {
  EXPECT_FALSE(r1_.nonfatally_failed());
  EXPECT_TRUE(r2_.nonfatally_failed());
  EXPECT_FALSE(r3_.nonfatally_failed());
}

// Tests the TestPartResultArray class.

class TestPartResultArrayTest : public Test {
 protected:
  TestPartResultArrayTest()
      : r1_(TPRT_NONFATAL_FAILURE, "foo/bar.cc", -1, "Failure 1"),
        r2_(TPRT_FATAL_FAILURE, "foo/bar.cc", -1, "Failure 2") {}

  const TestPartResult r1_, r2_;
};

// Tests that TestPartResultArray initially has size 0.
TEST_F(TestPartResultArrayTest, InitialSizeIsZero) {
  TestPartResultArray results;
  EXPECT_EQ(0, results.size());
}

// Tests that TestPartResultArray contains the given TestPartResult
// after one Append() operation.
TEST_F(TestPartResultArrayTest, ContainsGivenResultAfterAppend) {
  TestPartResultArray results;
  results.Append(r1_);
  EXPECT_EQ(1, results.size());
  EXPECT_STREQ("Failure 1", results.GetTestPartResult(0).message());
}

// Tests that TestPartResultArray contains the given TestPartResults
// after two Append() operations.
TEST_F(TestPartResultArrayTest, ContainsGivenResultsAfterTwoAppends) {
  TestPartResultArray results;
  results.Append(r1_);
  results.Append(r2_);
  EXPECT_EQ(2, results.size());
  EXPECT_STREQ("Failure 1", results.GetTestPartResult(0).message());
  EXPECT_STREQ("Failure 2", results.GetTestPartResult(1).message());
}

#ifdef GTEST_HAS_DEATH_TEST

typedef TestPartResultArrayTest TestPartResultArrayDeathTest;

// Tests that the program dies when GetTestPartResult() is called with
// an invalid index.
TEST_F(TestPartResultArrayDeathTest, DiesWhenIndexIsOutOfBound) {
  TestPartResultArray results;
  results.Append(r1_);

  EXPECT_DEATH(results.GetTestPartResult(-1), "");
  EXPECT_DEATH(results.GetTestPartResult(1), "");
}

#endif  // GTEST_HAS_DEATH_TEST

// TODO(mheule@google.com): Add a test for the class HasNewFatalFailureHelper.

}  // namespace
