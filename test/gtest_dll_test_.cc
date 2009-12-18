// Copyright 2009 Google Inc.  All Rights Reserved.
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
// Tests for Google Test itself.  This verifies that Google Test can be
// linked into an executable successfully when built as a DLL on Windows.
// The test is not meant to check the success of test assertions employed in
// it. It only checks that constructs in them can be successfully linked.
//
// If you add new features to Google Test's documented interface, you need to
// add tests exercising them to this file.
//
// If you start having 'unresolved external symbol' linker errors in this file
// after the changes you have made, re-generate src/gtest.def by running
// scripts/generate_gtest_def.py.

#include <gtest/gtest.h>
#include <gtest/gtest-spi.h>

#include <windows.h>
#include <vector>

using ::std::vector;
using ::std::tr1::tuple;


using ::testing::AddGlobalTestEnvironment;
using ::testing::AssertionFailure;
using ::testing::AssertionResult;
using ::testing::AssertionSuccess;
using ::testing::DoubleLE;
using ::testing::EmptyTestEventListener;
using ::testing::Environment;
using ::testing::ExitedWithCode;
using ::testing::FloatLE;
using ::testing::GTEST_FLAG(also_run_disabled_tests);
using ::testing::GTEST_FLAG(break_on_failure);
using ::testing::GTEST_FLAG(catch_exceptions);
using ::testing::GTEST_FLAG(color);
using ::testing::GTEST_FLAG(filter);
using ::testing::GTEST_FLAG(output);
using ::testing::GTEST_FLAG(print_time);
using ::testing::GTEST_FLAG(random_seed);
using ::testing::GTEST_FLAG(repeat);
using ::testing::GTEST_FLAG(shuffle);
using ::testing::GTEST_FLAG(stack_trace_depth);
using ::testing::GTEST_FLAG(throw_on_failure);
using ::testing::InitGoogleTest;
using ::testing::Message;
using ::testing::Test;
using ::testing::TestCase;
using ::testing::TestEventListener;
using ::testing::TestEventListeners;
using ::testing::TestInfo;
using ::testing::TestPartResult;
using ::testing::TestProperty;
using ::testing::TestResult;
using ::testing::UnitTest;
using ::testing::internal::AlwaysTrue;
using ::testing::internal::AlwaysFalse;

#if GTEST_HAS_PARAM_TEST
using ::testing::Bool;
using ::testing::Combine;
using ::testing::TestWithParam;
using ::testing::Values;
using ::testing::ValuesIn;
#endif  // GTEST_HAS_PARAM_TEST

#if GTEST_HAS_TYPED_TEST
using ::testing::Types;
#endif   // GTEST_HAS_TYPED_TEST

// Tests linking of TEST constructs.
TEST(TestMacroTest, LinksSuccessfully) {
}

// Tests linking of TEST_F constructs.
class FixtureTest : public Test {
};

TEST_F(FixtureTest, LinksSuccessfully) {
}

// Tests linking of value parameterized tests.
#if GTEST_HAS_PARAM_TEST
class IntParamTest : public TestWithParam<int> {};

TEST_P(IntParamTest, LinksSuccessfully) {}

const int c_array[] = {1, 2};
INSTANTIATE_TEST_CASE_P(ValuesInCArrayTest, IntParamTest, ValuesIn(c_array));

INSTANTIATE_TEST_CASE_P(ValuesInIteratorPairTest, IntParamTest,
                        ValuesIn(c_array, c_array + 2));

vector<int> stl_vector(c_array, c_array + 2);
INSTANTIATE_TEST_CASE_P(ValuesInStlVectorTest, IntParamTest,
                        ValuesIn(stl_vector));

class BoolParamTest : public TestWithParam<bool> {};

INSTANTIATE_TEST_CASE_P(BoolTest, BoolParamTest, Bool());

INSTANTIATE_TEST_CASE_P(ValuesTest, IntParamTest, Values(1, 2));

#if GTEST_HAS_COMBINE
class CombineTest : public TestWithParam<tuple<int, bool> > {};

INSTANTIATE_TEST_CASE_P(CombineTest, CombineTest, Combine(Values(1), Bool()));
#endif  // GTEST_HAS_COMBINE
#endif  // GTEST_HAS_PARAM_TEST

// Tests linking of typed tests.
#if GTEST_HAS_TYPED_TEST
template <typename T> class TypedTest : public Test {};

TYPED_TEST_CASE(TypedTest, Types<int>);

TYPED_TEST(TypedTest, LinksSuccessfully) {}
#endif   // GTEST_HAS_TYPED_TEST

// Tests linking of type-parameterized tests.
#if GTEST_HAS_TYPED_TEST_P
template <typename T> class TypeParameterizedTest : public Test {};

TYPED_TEST_CASE_P(TypeParameterizedTest);

TYPED_TEST_P(TypeParameterizedTest, LinksSuccessfully) {}

REGISTER_TYPED_TEST_CASE_P(TypeParameterizedTest, LinksSuccessfully);

INSTANTIATE_TYPED_TEST_CASE_P(Char, TypeParameterizedTest, Types<char>);
#endif  // GTEST_HAS_TYPED_TEST_P

// Tests linking of explicit success or failure.
TEST(ExplicitSuccessFailureTest, ExplicitSuccessAndFailure) {
  if (AlwaysTrue())
    SUCCEED() << "This is a success statement";
  if (AlwaysFalse()) {
    ADD_FAILURE() << "This is a non-fatal failure assertion";
    FAIL() << "This is a fatal failure assertion";
  }
}

// Tests linking of Boolean assertions.
AssertionResult IsEven(int n) {
  if (n % 2 == 0)
    return AssertionSuccess() << n << " is even";
  else
    return AssertionFailure() << n << " is odd";
}

TEST(BooleanAssertionTest, LinksSuccessfully) {
  EXPECT_TRUE(true) << "true is true";
  EXPECT_FALSE(false) << "false is not true";
  ASSERT_TRUE(true);
  ASSERT_FALSE(false);
  EXPECT_TRUE(IsEven(2));
  EXPECT_FALSE(IsEven(3));
}

// Tests linking of predicate assertions.
bool IsOdd(int n) { return n % 2 != 0; }

bool Ge(int val1, int val2) { return val1 >= val2; }

TEST(PredicateAssertionTest, LinksSuccessfully) {
  EXPECT_PRED1(IsOdd, 1);
  EXPECT_PRED2(Ge, 2, 1);
}

AssertionResult AddToFive(const char* val1_expr,
                          const char* val2_expr,
                          int val1,
                          int val2) {
  if (val1 + val2 == 5)
    return AssertionSuccess();

  return AssertionFailure() << val1_expr << " and " << val2_expr
                            << " (" << val1 << " and " << val2 << ") "
                            << "do not add up to five, as their sum is "
                            << val1 + val2;
}

TEST(PredicateFormatterAssertionTest, LinksSuccessfully) {
  EXPECT_PRED_FORMAT2(AddToFive, 1 + 2, 2);
}


// Tests linking of comparison assertions.
TEST(ComparisonAssertionTest, LinksSuccessfully) {
  EXPECT_EQ(1, 1);
  EXPECT_NE(1, 2);
  EXPECT_LT(1, 2);
  EXPECT_LE(1, 1);
  EXPECT_GT(2, 1);
  EXPECT_GE(2, 1);

  EXPECT_EQ('\n', '\n');
  EXPECT_NE('\n', '\r');
  EXPECT_LT('\n', 'a');
  EXPECT_LE('\n', 'b');
  EXPECT_GT('a', '\t');
  EXPECT_GE('b', '\t');
}

TEST(StringComparisonAssertionTest, LinksSuccessfully) {
  EXPECT_STREQ("test", "test");
  EXPECT_STRNE("test", "prod");

  char test_str[5] = "test";
  char prod_str[5] = "prod";

  EXPECT_STREQ(test_str, test_str);
  EXPECT_STRNE(test_str, prod_str);

  EXPECT_STRCASEEQ("test", "TEST");
  EXPECT_STRCASENE("test", "prod");

  wchar_t test_wstr[5] = L"test";
  wchar_t prod_wstr[5] = L"prod";

  EXPECT_STREQ(L"test", L"test");
  EXPECT_STRNE(L"test", L"prod");

  EXPECT_STREQ(test_wstr, test_wstr);
  EXPECT_STRNE(test_wstr, prod_wstr);

#if GTEST_HAS_STD_STRING
  EXPECT_EQ("test", ::std::string("test"));
  EXPECT_NE("test", ::std::string("prod"));

  EXPECT_EQ(::std::string("test"), "test");
  EXPECT_NE(::std::string("prod"), "test");

  EXPECT_EQ(test_str, ::std::string("test"));
  EXPECT_NE(test_str, ::std::string("prod"));

  EXPECT_EQ(::std::string("test"), test_str);
  EXPECT_NE(::std::string("prod"), test_str);

  EXPECT_EQ(::std::string("test"), ::std::string("test"));
  EXPECT_NE(::std::string("test"), ::std::string("prod"));
#endif  // GTEST_HAS_STD_STRING

#if GTEST_HAS_STD_WSTRING
  EXPECT_EQ(L"test", ::std::wstring(L"test"));
  EXPECT_NE(L"test", ::std::wstring(L"prod"));

  EXPECT_EQ(::std::wstring(L"test"), L"test");
  EXPECT_NE(::std::wstring(L"prod"), L"test");

  EXPECT_EQ(test_wstr, ::std::wstring(L"test"));
  EXPECT_NE(test_wstr, ::std::wstring(L"prod"));

  EXPECT_EQ(::std::wstring(L"test"), test_wstr);
  EXPECT_NE(::std::wstring(L"prod"), test_wstr);

  EXPECT_EQ(::std::wstring(L"test"), ::std::wstring(L"test"));
  EXPECT_NE(::std::wstring(L"test"), ::std::wstring(L"prod"));
#endif  // GTEST_HAS_STD_WSTRING
}

// Tests linking of floating point assertions.
TEST(FloatingPointComparisonAssertionTest, LinksSuccessfully) {
  EXPECT_FLOAT_EQ(0.0f, 0.0f);
  EXPECT_DOUBLE_EQ(0.0, 0.0);
  EXPECT_NEAR(0.0, 0.1, 0.2);
  EXPECT_PRED_FORMAT2(::testing::FloatLE, 0.0f, 0.01f);
  EXPECT_PRED_FORMAT2(::testing::DoubleLE, 0.0, 0.001);
}

// Tests linking of HRESULT assertions.
TEST(HresultAssertionTest, LinksSuccessfully) {
  EXPECT_HRESULT_SUCCEEDED(S_OK);
  EXPECT_HRESULT_FAILED(E_FAIL);
}

#if GTEST_HAS_EXCEPTIONS
// Tests linking of exception assertions.
TEST(ExceptionAssertionTest, LinksSuccessfully) {
  EXPECT_THROW(throw 1, int);
  EXPECT_ANY_THROW(throw 1);
  EXPECT_NO_THROW(int x = 1);
}
#endif  // GTEST_HAS_EXCEPTIONS

// Tests linking of death test assertions.
TEST(DeathTestAssertionDeathTest, LinksSuccessfully) {
  EXPECT_DEATH_IF_SUPPORTED(exit(1), "");

#if GTEST_HAS_DEATH_TEST
  EXPECT_EXIT(exit(1), ExitedWithCode(1), "");
#endif  // GTEST_HAS_DEATH_TEST
}

// Tests linking of SCOPED_TRACE.
void Sub() { EXPECT_EQ(1, 1); }

TEST(ScopedTraceTest, LinksSuccessfully) {
  SCOPED_TRACE("X");
  Sub();
}

// Tests linking of failure absence assertions.
TEST(NoFailureAssertionTest, LinksSuccessfully) {
  EXPECT_NO_FATAL_FAILURE(IsEven(2));
}

// Tests linking of HasFatalFailure.
TEST(HasFatalFailureTest, LinksSuccessfully) {
  EXPECT_FALSE(HasFatalFailure());
  EXPECT_FALSE(HasNonfatalFailure());
  EXPECT_FALSE(HasFailure());
}

// Tests linking of RecordProperty.
TEST(RecordPropertyTest, LinksSuccessfully) {
  RecordProperty("DummyPropery", "DummyValue");
}

// Tests linking of environments.
class MyEnvironment : public Environment {};

Environment* const environment = AddGlobalTestEnvironment(new MyEnvironment);

// Tests linking of flags.
TEST(FlagTest, LinksSuccessfully) {
  Message message;

  message << GTEST_FLAG(filter);
  message << GTEST_FLAG(also_run_disabled_tests);
  message << GTEST_FLAG(repeat);
  message << GTEST_FLAG(shuffle);
  message << GTEST_FLAG(random_seed);
  message << GTEST_FLAG(color);
  message << GTEST_FLAG(print_time);
  message << GTEST_FLAG(output);
  message << GTEST_FLAG(break_on_failure);
  message << GTEST_FLAG(throw_on_failure);
  message << GTEST_FLAG(catch_exceptions);
  message << GTEST_FLAG(stack_trace_depth);
}

// Tests linking of failure catching assertions.
void FunctionWithFailure() { FAIL(); }

TEST(FailureCatchingAssertionTest, LinksCorrectly) {
  EXPECT_FATAL_FAILURE(FunctionWithFailure(), "");
  EXPECT_NONFATAL_FAILURE(ADD_FAILURE(), "");
  EXPECT_FATAL_FAILURE_ON_ALL_THREADS(FunctionWithFailure(), "");
  EXPECT_NONFATAL_FAILURE_ON_ALL_THREADS(ADD_FAILURE(), "");
}

// Tests linking of the reflection API.
TEST(ReflectionApiTest, LinksCorrectly) {
  // UnitTest API.
  UnitTest* unit_test = UnitTest::GetInstance();

  unit_test->original_working_dir();
  EXPECT_TRUE(unit_test->current_test_case() != NULL);
  EXPECT_TRUE(unit_test->current_test_info() != NULL);
  EXPECT_NE(0, unit_test->random_seed());
  EXPECT_GE(unit_test->successful_test_case_count(), 0);
  EXPECT_EQ(0, unit_test->failed_test_case_count());
  EXPECT_GE(unit_test->total_test_case_count(), 0);
  EXPECT_GT(unit_test->test_case_to_run_count(), 0);
  EXPECT_GE(unit_test->successful_test_count(), 0);
  EXPECT_EQ(0, unit_test->failed_test_count());
  EXPECT_EQ(0, unit_test->disabled_test_count());
  EXPECT_GT(unit_test->total_test_count(), 0);
  EXPECT_GT(unit_test->test_to_run_count(), 0);
  EXPECT_GE(unit_test->elapsed_time(), 0);
  EXPECT_TRUE(unit_test->Passed());
  EXPECT_FALSE(unit_test->Failed());
  EXPECT_TRUE(unit_test->GetTestCase(0) != NULL);

  // TestCase API.
  const TestCase*const  test_case = unit_test->current_test_case();

  EXPECT_STRNE("", test_case->name());
  const char* const test_case_comment = test_case->comment();
  EXPECT_TRUE(test_case->should_run());
  EXPECT_GE(test_case->successful_test_count(), 0);
  EXPECT_EQ(0, test_case->failed_test_count());
  EXPECT_EQ(0, test_case->disabled_test_count());
  EXPECT_GT(test_case->test_to_run_count(), 0);
  EXPECT_GT(test_case->total_test_count(), 0);
  EXPECT_TRUE(test_case->Passed());
  EXPECT_FALSE(test_case->Failed());
  EXPECT_GE(test_case->elapsed_time(), 0);
  EXPECT_TRUE(test_case->GetTestInfo(0) != NULL);

  // TestInfo API.
  const TestInfo* const test_info = unit_test->current_test_info();

  EXPECT_STRNE("", test_info->test_case_name());
  EXPECT_STRNE("", test_info->name());
  EXPECT_STREQ(test_case_comment, test_info->test_case_comment());
  const char* const comment = test_info->comment();
  EXPECT_TRUE(comment == NULL || strlen(comment) >= 0);
  EXPECT_TRUE(test_info->should_run());
  EXPECT_TRUE(test_info->result() != NULL);

  // TestResult API.
  const TestResult* const test_result = test_info->result();

  SUCCEED() << "This generates a successful test part instance for API testing";
  RecordProperty("Test Name", "Test Value");
  EXPECT_EQ(1, test_result->total_part_count());
  EXPECT_EQ(1, test_result->test_property_count());
  EXPECT_TRUE(test_result->Passed());
  EXPECT_FALSE(test_result->Failed());
  EXPECT_FALSE(test_result->HasFatalFailure());
  EXPECT_FALSE(test_result->HasNonfatalFailure());
  EXPECT_GE(test_result->elapsed_time(), 0);
  const TestPartResult& test_part_result = test_result->GetTestPartResult(0);
  const TestProperty& test_property = test_result->GetTestProperty(0);

  // TestPartResult API.
  EXPECT_EQ(TestPartResult::kSuccess, test_part_result.type());
  EXPECT_STRNE("", test_part_result.file_name());
  EXPECT_GT(test_part_result.line_number(), 0);
  EXPECT_STRNE("", test_part_result.summary());
  EXPECT_STRNE("", test_part_result.message());
  EXPECT_TRUE(test_part_result.passed());
  EXPECT_FALSE(test_part_result.failed());
  EXPECT_FALSE(test_part_result.nonfatally_failed());
  EXPECT_FALSE(test_part_result.fatally_failed());

  // TestProperty API.
  EXPECT_STREQ("Test Name", test_property.key());
  EXPECT_STREQ("Test Value", test_property.value());
}

// Tests linking of the event listener API.
class MyListener : public TestEventListener {
  virtual void OnTestProgramStart(const UnitTest& /*unit_test*/) {}
  virtual void OnTestIterationStart(const UnitTest& /*unit_test*/,
                                    int /*iteration*/) {}
  virtual void OnEnvironmentsSetUpStart(const UnitTest& /*unit_test*/) {}
  virtual void OnEnvironmentsSetUpEnd(const UnitTest& /*unit_test*/) {}
  virtual void OnTestCaseStart(const TestCase& /*test_case*/) {}
  virtual void OnTestStart(const TestInfo& /*test_info*/) {}
  virtual void OnTestPartResult(const TestPartResult& /*test_part_result*/) {}
  virtual void OnTestEnd(const TestInfo& /*test_info*/) {}
  virtual void OnTestCaseEnd(const TestCase& /*test_case*/) {}
  virtual void OnEnvironmentsTearDownStart(const UnitTest& /*unit_test*/) {}
  virtual void OnEnvironmentsTearDownEnd(const UnitTest& /*unit_test*/) {}
  virtual void OnTestIterationEnd(const UnitTest& /*unit_test*/,
                                  int /*iteration*/) {}
  virtual void OnTestProgramEnd(const UnitTest& /*unit_test*/) {}
};

class MyOtherListener : public EmptyTestEventListener {};

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);

  TestEventListeners& listeners =  UnitTest::GetInstance()->listeners();
  TestEventListener* listener = new MyListener;

  listeners.Append(listener);
  listeners.Release(listener);
  listeners.Append(new MyOtherListener);
  listener = listeners.default_result_printer();
  listener = listeners.default_xml_generator();

  RUN_ALL_TESTS();
  return  0;
}
