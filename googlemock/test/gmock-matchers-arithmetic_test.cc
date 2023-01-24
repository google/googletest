// Copyright 2007, Google Inc.
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
// This file tests some commonly used argument matchers.

// Silence warning C4244: 'initializing': conversion from 'int' to 'short',
// possible loss of data and C4100, unreferenced local parameter
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4244)
#pragma warning(disable : 4100)
#endif

#include "test/gmock-matchers_test.h"

#include <limits>

namespace testing {
namespace gmock_matchers_test {
namespace {

typedef ::std::tuple<long, int> Tuple2;  // NOLINT

// Tests that Eq() matches a 2-tuple where the first field == the
// second field.
TEST(Eq2Test, MatchesEqualArguments) {
  Matcher<const Tuple2&> m = Eq();
  EXPECT_TRUE(m.Matches(Tuple2(5L, 5)));
  EXPECT_FALSE(m.Matches(Tuple2(5L, 6)));
}

// Tests that Eq() describes itself properly.
TEST(Eq2Test, CanDescribeSelf) {
  Matcher<const Tuple2&> m = Eq();
  EXPECT_EQ("are an equal pair", Describe(m));
}

// Tests that Ge() matches a 2-tuple where the first field >= the
// second field.
TEST(Ge2Test, MatchesGreaterThanOrEqualArguments) {
  Matcher<const Tuple2&> m = Ge();
  EXPECT_TRUE(m.Matches(Tuple2(5L, 4)));
  EXPECT_TRUE(m.Matches(Tuple2(5L, 5)));
  EXPECT_FALSE(m.Matches(Tuple2(5L, 6)));
}

// Tests that Ge() describes itself properly.
TEST(Ge2Test, CanDescribeSelf) {
  Matcher<const Tuple2&> m = Ge();
  EXPECT_EQ("are a pair where the first >= the second", Describe(m));
}

// Tests that Gt() matches a 2-tuple where the first field > the
// second field.
TEST(Gt2Test, MatchesGreaterThanArguments) {
  Matcher<const Tuple2&> m = Gt();
  EXPECT_TRUE(m.Matches(Tuple2(5L, 4)));
  EXPECT_FALSE(m.Matches(Tuple2(5L, 5)));
  EXPECT_FALSE(m.Matches(Tuple2(5L, 6)));
}

// Tests that Gt() describes itself properly.
TEST(Gt2Test, CanDescribeSelf) {
  Matcher<const Tuple2&> m = Gt();
  EXPECT_EQ("are a pair where the first > the second", Describe(m));
}

// Tests that Le() matches a 2-tuple where the first field <= the
// second field.
TEST(Le2Test, MatchesLessThanOrEqualArguments) {
  Matcher<const Tuple2&> m = Le();
  EXPECT_TRUE(m.Matches(Tuple2(5L, 6)));
  EXPECT_TRUE(m.Matches(Tuple2(5L, 5)));
  EXPECT_FALSE(m.Matches(Tuple2(5L, 4)));
}

// Tests that Le() describes itself properly.
TEST(Le2Test, CanDescribeSelf) {
  Matcher<const Tuple2&> m = Le();
  EXPECT_EQ("are a pair where the first <= the second", Describe(m));
}

// Tests that Lt() matches a 2-tuple where the first field < the
// second field.
TEST(Lt2Test, MatchesLessThanArguments) {
  Matcher<const Tuple2&> m = Lt();
  EXPECT_TRUE(m.Matches(Tuple2(5L, 6)));
  EXPECT_FALSE(m.Matches(Tuple2(5L, 5)));
  EXPECT_FALSE(m.Matches(Tuple2(5L, 4)));
}

// Tests that Lt() describes itself properly.
TEST(Lt2Test, CanDescribeSelf) {
  Matcher<const Tuple2&> m = Lt();
  EXPECT_EQ("are a pair where the first < the second", Describe(m));
}

// Tests that Ne() matches a 2-tuple where the first field != the
// second field.
TEST(Ne2Test, MatchesUnequalArguments) {
  Matcher<const Tuple2&> m = Ne();
  EXPECT_TRUE(m.Matches(Tuple2(5L, 6)));
  EXPECT_TRUE(m.Matches(Tuple2(5L, 4)));
  EXPECT_FALSE(m.Matches(Tuple2(5L, 5)));
}

// Tests that Ne() describes itself properly.
TEST(Ne2Test, CanDescribeSelf) {
  Matcher<const Tuple2&> m = Ne();
  EXPECT_EQ("are an unequal pair", Describe(m));
}

TEST(PairMatchBaseTest, WorksWithMoveOnly) {
  using Pointers = std::tuple<std::unique_ptr<int>, std::unique_ptr<int>>;
  Matcher<Pointers> matcher = Eq();
  Pointers pointers;
  // Tested values don't matter; the point is that matcher does not copy the
  // matched values.
  EXPECT_TRUE(matcher.Matches(pointers));
}

// Tests that IsNan() matches a NaN, with float.
TEST(IsNan, FloatMatchesNan) {
  float quiet_nan = std::numeric_limits<float>::quiet_NaN();
  float other_nan = std::nanf("1");
  float real_value = 1.0f;

  Matcher<float> m = IsNan();
  EXPECT_TRUE(m.Matches(quiet_nan));
  EXPECT_TRUE(m.Matches(other_nan));
  EXPECT_FALSE(m.Matches(real_value));

  Matcher<float&> m_ref = IsNan();
  EXPECT_TRUE(m_ref.Matches(quiet_nan));
  EXPECT_TRUE(m_ref.Matches(other_nan));
  EXPECT_FALSE(m_ref.Matches(real_value));

  Matcher<const float&> m_cref = IsNan();
  EXPECT_TRUE(m_cref.Matches(quiet_nan));
  EXPECT_TRUE(m_cref.Matches(other_nan));
  EXPECT_FALSE(m_cref.Matches(real_value));
}

// Tests that IsNan() matches a NaN, with double.
TEST(IsNan, DoubleMatchesNan) {
  double quiet_nan = std::numeric_limits<double>::quiet_NaN();
  double other_nan = std::nan("1");
  double real_value = 1.0;

  Matcher<double> m = IsNan();
  EXPECT_TRUE(m.Matches(quiet_nan));
  EXPECT_TRUE(m.Matches(other_nan));
  EXPECT_FALSE(m.Matches(real_value));

  Matcher<double&> m_ref = IsNan();
  EXPECT_TRUE(m_ref.Matches(quiet_nan));
  EXPECT_TRUE(m_ref.Matches(other_nan));
  EXPECT_FALSE(m_ref.Matches(real_value));

  Matcher<const double&> m_cref = IsNan();
  EXPECT_TRUE(m_cref.Matches(quiet_nan));
  EXPECT_TRUE(m_cref.Matches(other_nan));
  EXPECT_FALSE(m_cref.Matches(real_value));
}

// Tests that IsNan() matches a NaN, with long double.
TEST(IsNan, LongDoubleMatchesNan) {
  long double quiet_nan = std::numeric_limits<long double>::quiet_NaN();
  long double other_nan = std::nan("1");
  long double real_value = 1.0;

  Matcher<long double> m = IsNan();
  EXPECT_TRUE(m.Matches(quiet_nan));
  EXPECT_TRUE(m.Matches(other_nan));
  EXPECT_FALSE(m.Matches(real_value));

  Matcher<long double&> m_ref = IsNan();
  EXPECT_TRUE(m_ref.Matches(quiet_nan));
  EXPECT_TRUE(m_ref.Matches(other_nan));
  EXPECT_FALSE(m_ref.Matches(real_value));

  Matcher<const long double&> m_cref = IsNan();
  EXPECT_TRUE(m_cref.Matches(quiet_nan));
  EXPECT_TRUE(m_cref.Matches(other_nan));
  EXPECT_FALSE(m_cref.Matches(real_value));
}

// Tests that IsNan() works with Not.
TEST(IsNan, NotMatchesNan) {
  Matcher<float> mf = Not(IsNan());
  EXPECT_FALSE(mf.Matches(std::numeric_limits<float>::quiet_NaN()));
  EXPECT_FALSE(mf.Matches(std::nanf("1")));
  EXPECT_TRUE(mf.Matches(1.0));

  Matcher<double> md = Not(IsNan());
  EXPECT_FALSE(md.Matches(std::numeric_limits<double>::quiet_NaN()));
  EXPECT_FALSE(md.Matches(std::nan("1")));
  EXPECT_TRUE(md.Matches(1.0));

  Matcher<long double> mld = Not(IsNan());
  EXPECT_FALSE(mld.Matches(std::numeric_limits<long double>::quiet_NaN()));
  EXPECT_FALSE(mld.Matches(std::nanl("1")));
  EXPECT_TRUE(mld.Matches(1.0));
}

// Tests that IsNan() can describe itself.
TEST(IsNan, CanDescribeSelf) {
  Matcher<float> mf = IsNan();
  EXPECT_EQ("is NaN", Describe(mf));

  Matcher<double> md = IsNan();
  EXPECT_EQ("is NaN", Describe(md));

  Matcher<long double> mld = IsNan();
  EXPECT_EQ("is NaN", Describe(mld));
}

// Tests that IsNan() can describe itself with Not.
TEST(IsNan, CanDescribeSelfWithNot) {
  Matcher<float> mf = Not(IsNan());
  EXPECT_EQ("isn't NaN", Describe(mf));

  Matcher<double> md = Not(IsNan());
  EXPECT_EQ("isn't NaN", Describe(md));

  Matcher<long double> mld = Not(IsNan());
  EXPECT_EQ("isn't NaN", Describe(mld));
}

// Tests that FloatEq() matches a 2-tuple where
// FloatEq(first field) matches the second field.
TEST(FloatEq2Test, MatchesEqualArguments) {
  typedef ::std::tuple<float, float> Tpl;
  Matcher<const Tpl&> m = FloatEq();
  EXPECT_TRUE(m.Matches(Tpl(1.0f, 1.0f)));
  EXPECT_TRUE(m.Matches(Tpl(0.3f, 0.1f + 0.1f + 0.1f)));
  EXPECT_FALSE(m.Matches(Tpl(1.1f, 1.0f)));
}

// Tests that FloatEq() describes itself properly.
TEST(FloatEq2Test, CanDescribeSelf) {
  Matcher<const ::std::tuple<float, float>&> m = FloatEq();
  EXPECT_EQ("are an almost-equal pair", Describe(m));
}

// Tests that NanSensitiveFloatEq() matches a 2-tuple where
// NanSensitiveFloatEq(first field) matches the second field.
TEST(NanSensitiveFloatEqTest, MatchesEqualArgumentsWithNaN) {
  typedef ::std::tuple<float, float> Tpl;
  Matcher<const Tpl&> m = NanSensitiveFloatEq();
  EXPECT_TRUE(m.Matches(Tpl(1.0f, 1.0f)));
  EXPECT_TRUE(m.Matches(Tpl(std::numeric_limits<float>::quiet_NaN(),
                            std::numeric_limits<float>::quiet_NaN())));
  EXPECT_FALSE(m.Matches(Tpl(1.1f, 1.0f)));
  EXPECT_FALSE(m.Matches(Tpl(1.0f, std::numeric_limits<float>::quiet_NaN())));
  EXPECT_FALSE(m.Matches(Tpl(std::numeric_limits<float>::quiet_NaN(), 1.0f)));
}

// Tests that NanSensitiveFloatEq() describes itself properly.
TEST(NanSensitiveFloatEqTest, CanDescribeSelfWithNaNs) {
  Matcher<const ::std::tuple<float, float>&> m = NanSensitiveFloatEq();
  EXPECT_EQ("are an almost-equal pair", Describe(m));
}

// Tests that DoubleEq() matches a 2-tuple where
// DoubleEq(first field) matches the second field.
TEST(DoubleEq2Test, MatchesEqualArguments) {
  typedef ::std::tuple<double, double> Tpl;
  Matcher<const Tpl&> m = DoubleEq();
  EXPECT_TRUE(m.Matches(Tpl(1.0, 1.0)));
  EXPECT_TRUE(m.Matches(Tpl(0.3, 0.1 + 0.1 + 0.1)));
  EXPECT_FALSE(m.Matches(Tpl(1.1, 1.0)));
}

// Tests that DoubleEq() describes itself properly.
TEST(DoubleEq2Test, CanDescribeSelf) {
  Matcher<const ::std::tuple<double, double>&> m = DoubleEq();
  EXPECT_EQ("are an almost-equal pair", Describe(m));
}

// Tests that NanSensitiveDoubleEq() matches a 2-tuple where
// NanSensitiveDoubleEq(first field) matches the second field.
TEST(NanSensitiveDoubleEqTest, MatchesEqualArgumentsWithNaN) {
  typedef ::std::tuple<double, double> Tpl;
  Matcher<const Tpl&> m = NanSensitiveDoubleEq();
  EXPECT_TRUE(m.Matches(Tpl(1.0f, 1.0f)));
  EXPECT_TRUE(m.Matches(Tpl(std::numeric_limits<double>::quiet_NaN(),
                            std::numeric_limits<double>::quiet_NaN())));
  EXPECT_FALSE(m.Matches(Tpl(1.1f, 1.0f)));
  EXPECT_FALSE(m.Matches(Tpl(1.0f, std::numeric_limits<double>::quiet_NaN())));
  EXPECT_FALSE(m.Matches(Tpl(std::numeric_limits<double>::quiet_NaN(), 1.0f)));
}

// Tests that DoubleEq() describes itself properly.
TEST(NanSensitiveDoubleEqTest, CanDescribeSelfWithNaNs) {
  Matcher<const ::std::tuple<double, double>&> m = NanSensitiveDoubleEq();
  EXPECT_EQ("are an almost-equal pair", Describe(m));
}

// Tests that FloatEq() matches a 2-tuple where
// FloatNear(first field, max_abs_error) matches the second field.
TEST(FloatNear2Test, MatchesEqualArguments) {
  typedef ::std::tuple<float, float> Tpl;
  Matcher<const Tpl&> m = FloatNear(0.5f);
  EXPECT_TRUE(m.Matches(Tpl(1.0f, 1.0f)));
  EXPECT_TRUE(m.Matches(Tpl(1.3f, 1.0f)));
  EXPECT_FALSE(m.Matches(Tpl(1.8f, 1.0f)));
}

// Tests that FloatNear() describes itself properly.
TEST(FloatNear2Test, CanDescribeSelf) {
  Matcher<const ::std::tuple<float, float>&> m = FloatNear(0.5f);
  EXPECT_EQ("are an almost-equal pair", Describe(m));
}

// Tests that NanSensitiveFloatNear() matches a 2-tuple where
// NanSensitiveFloatNear(first field) matches the second field.
TEST(NanSensitiveFloatNearTest, MatchesNearbyArgumentsWithNaN) {
  typedef ::std::tuple<float, float> Tpl;
  Matcher<const Tpl&> m = NanSensitiveFloatNear(0.5f);
  EXPECT_TRUE(m.Matches(Tpl(1.0f, 1.0f)));
  EXPECT_TRUE(m.Matches(Tpl(1.1f, 1.0f)));
  EXPECT_TRUE(m.Matches(Tpl(std::numeric_limits<float>::quiet_NaN(),
                            std::numeric_limits<float>::quiet_NaN())));
  EXPECT_FALSE(m.Matches(Tpl(1.6f, 1.0f)));
  EXPECT_FALSE(m.Matches(Tpl(1.0f, std::numeric_limits<float>::quiet_NaN())));
  EXPECT_FALSE(m.Matches(Tpl(std::numeric_limits<float>::quiet_NaN(), 1.0f)));
}

// Tests that NanSensitiveFloatNear() describes itself properly.
TEST(NanSensitiveFloatNearTest, CanDescribeSelfWithNaNs) {
  Matcher<const ::std::tuple<float, float>&> m = NanSensitiveFloatNear(0.5f);
  EXPECT_EQ("are an almost-equal pair", Describe(m));
}

// Tests that FloatEq() matches a 2-tuple where
// DoubleNear(first field, max_abs_error) matches the second field.
TEST(DoubleNear2Test, MatchesEqualArguments) {
  typedef ::std::tuple<double, double> Tpl;
  Matcher<const Tpl&> m = DoubleNear(0.5);
  EXPECT_TRUE(m.Matches(Tpl(1.0, 1.0)));
  EXPECT_TRUE(m.Matches(Tpl(1.3, 1.0)));
  EXPECT_FALSE(m.Matches(Tpl(1.8, 1.0)));
}

// Tests that DoubleNear() describes itself properly.
TEST(DoubleNear2Test, CanDescribeSelf) {
  Matcher<const ::std::tuple<double, double>&> m = DoubleNear(0.5);
  EXPECT_EQ("are an almost-equal pair", Describe(m));
}

// Tests that NanSensitiveDoubleNear() matches a 2-tuple where
// NanSensitiveDoubleNear(first field) matches the second field.
TEST(NanSensitiveDoubleNearTest, MatchesNearbyArgumentsWithNaN) {
  typedef ::std::tuple<double, double> Tpl;
  Matcher<const Tpl&> m = NanSensitiveDoubleNear(0.5f);
  EXPECT_TRUE(m.Matches(Tpl(1.0f, 1.0f)));
  EXPECT_TRUE(m.Matches(Tpl(1.1f, 1.0f)));
  EXPECT_TRUE(m.Matches(Tpl(std::numeric_limits<double>::quiet_NaN(),
                            std::numeric_limits<double>::quiet_NaN())));
  EXPECT_FALSE(m.Matches(Tpl(1.6f, 1.0f)));
  EXPECT_FALSE(m.Matches(Tpl(1.0f, std::numeric_limits<double>::quiet_NaN())));
  EXPECT_FALSE(m.Matches(Tpl(std::numeric_limits<double>::quiet_NaN(), 1.0f)));
}

// Tests that NanSensitiveDoubleNear() describes itself properly.
TEST(NanSensitiveDoubleNearTest, CanDescribeSelfWithNaNs) {
  Matcher<const ::std::tuple<double, double>&> m = NanSensitiveDoubleNear(0.5f);
  EXPECT_EQ("are an almost-equal pair", Describe(m));
}

// Tests that Not(m) matches any value that doesn't match m.
TEST(NotTest, NegatesMatcher) {
  Matcher<int> m;
  m = Not(Eq(2));
  EXPECT_TRUE(m.Matches(3));
  EXPECT_FALSE(m.Matches(2));
}

// Tests that Not(m) describes itself properly.
TEST(NotTest, CanDescribeSelf) {
  Matcher<int> m = Not(Eq(5));
  EXPECT_EQ("isn't equal to 5", Describe(m));
}

// Tests that monomorphic matchers are safely cast by the Not matcher.
TEST(NotTest, NotMatcherSafelyCastsMonomorphicMatchers) {
  // greater_than_5 is a monomorphic matcher.
  Matcher<int> greater_than_5 = Gt(5);

  Matcher<const int&> m = Not(greater_than_5);
  Matcher<int&> m2 = Not(greater_than_5);
  Matcher<int&> m3 = Not(m);
}

// Helper to allow easy testing of AllOf matchers with num parameters.
void AllOfMatches(int num, const Matcher<int>& m) {
  SCOPED_TRACE(Describe(m));
  EXPECT_TRUE(m.Matches(0));
  for (int i = 1; i <= num; ++i) {
    EXPECT_FALSE(m.Matches(i));
  }
  EXPECT_TRUE(m.Matches(num + 1));
}

INSTANTIATE_GTEST_MATCHER_TEST_P(AllOfTest);

// Tests that AllOf(m1, ..., mn) matches any value that matches all of
// the given matchers.
TEST(AllOfTest, MatchesWhenAllMatch) {
  Matcher<int> m;
  m = AllOf(Le(2), Ge(1));
  EXPECT_TRUE(m.Matches(1));
  EXPECT_TRUE(m.Matches(2));
  EXPECT_FALSE(m.Matches(0));
  EXPECT_FALSE(m.Matches(3));

  m = AllOf(Gt(0), Ne(1), Ne(2));
  EXPECT_TRUE(m.Matches(3));
  EXPECT_FALSE(m.Matches(2));
  EXPECT_FALSE(m.Matches(1));
  EXPECT_FALSE(m.Matches(0));

  m = AllOf(Gt(0), Ne(1), Ne(2), Ne(3));
  EXPECT_TRUE(m.Matches(4));
  EXPECT_FALSE(m.Matches(3));
  EXPECT_FALSE(m.Matches(2));
  EXPECT_FALSE(m.Matches(1));
  EXPECT_FALSE(m.Matches(0));

  m = AllOf(Ge(0), Lt(10), Ne(3), Ne(5), Ne(7));
  EXPECT_TRUE(m.Matches(0));
  EXPECT_TRUE(m.Matches(1));
  EXPECT_FALSE(m.Matches(3));

  // The following tests for varying number of sub-matchers. Due to the way
  // the sub-matchers are handled it is enough to test every sub-matcher once
  // with sub-matchers using the same matcher type. Varying matcher types are
  // checked for above.
  AllOfMatches(2, AllOf(Ne(1), Ne(2)));
  AllOfMatches(3, AllOf(Ne(1), Ne(2), Ne(3)));
  AllOfMatches(4, AllOf(Ne(1), Ne(2), Ne(3), Ne(4)));
  AllOfMatches(5, AllOf(Ne(1), Ne(2), Ne(3), Ne(4), Ne(5)));
  AllOfMatches(6, AllOf(Ne(1), Ne(2), Ne(3), Ne(4), Ne(5), Ne(6)));
  AllOfMatches(7, AllOf(Ne(1), Ne(2), Ne(3), Ne(4), Ne(5), Ne(6), Ne(7)));
  AllOfMatches(8,
               AllOf(Ne(1), Ne(2), Ne(3), Ne(4), Ne(5), Ne(6), Ne(7), Ne(8)));
  AllOfMatches(
      9, AllOf(Ne(1), Ne(2), Ne(3), Ne(4), Ne(5), Ne(6), Ne(7), Ne(8), Ne(9)));
  AllOfMatches(10, AllOf(Ne(1), Ne(2), Ne(3), Ne(4), Ne(5), Ne(6), Ne(7), Ne(8),
                         Ne(9), Ne(10)));
  AllOfMatches(
      50, AllOf(Ne(1), Ne(2), Ne(3), Ne(4), Ne(5), Ne(6), Ne(7), Ne(8), Ne(9),
                Ne(10), Ne(11), Ne(12), Ne(13), Ne(14), Ne(15), Ne(16), Ne(17),
                Ne(18), Ne(19), Ne(20), Ne(21), Ne(22), Ne(23), Ne(24), Ne(25),
                Ne(26), Ne(27), Ne(28), Ne(29), Ne(30), Ne(31), Ne(32), Ne(33),
                Ne(34), Ne(35), Ne(36), Ne(37), Ne(38), Ne(39), Ne(40), Ne(41),
                Ne(42), Ne(43), Ne(44), Ne(45), Ne(46), Ne(47), Ne(48), Ne(49),
                Ne(50)));
}

// Tests that AllOf(m1, ..., mn) describes itself properly.
TEST(AllOfTest, CanDescribeSelf) {
  Matcher<int> m;
  m = AllOf(Le(2), Ge(1));
  EXPECT_EQ("(is <= 2) and (is >= 1)", Describe(m));

  m = AllOf(Gt(0), Ne(1), Ne(2));
  std::string expected_descr1 =
      "(is > 0) and (isn't equal to 1) and (isn't equal to 2)";
  EXPECT_EQ(expected_descr1, Describe(m));

  m = AllOf(Gt(0), Ne(1), Ne(2), Ne(3));
  std::string expected_descr2 =
      "(is > 0) and (isn't equal to 1) and (isn't equal to 2) and (isn't equal "
      "to 3)";
  EXPECT_EQ(expected_descr2, Describe(m));

  m = AllOf(Ge(0), Lt(10), Ne(3), Ne(5), Ne(7));
  std::string expected_descr3 =
      "(is >= 0) and (is < 10) and (isn't equal to 3) and (isn't equal to 5) "
      "and (isn't equal to 7)";
  EXPECT_EQ(expected_descr3, Describe(m));
}

// Tests that AllOf(m1, ..., mn) describes its negation properly.
TEST(AllOfTest, CanDescribeNegation) {
  Matcher<int> m;
  m = AllOf(Le(2), Ge(1));
  std::string expected_descr4 = "(isn't <= 2) or (isn't >= 1)";
  EXPECT_EQ(expected_descr4, DescribeNegation(m));

  m = AllOf(Gt(0), Ne(1), Ne(2));
  std::string expected_descr5 =
      "(isn't > 0) or (is equal to 1) or (is equal to 2)";
  EXPECT_EQ(expected_descr5, DescribeNegation(m));

  m = AllOf(Gt(0), Ne(1), Ne(2), Ne(3));
  std::string expected_descr6 =
      "(isn't > 0) or (is equal to 1) or (is equal to 2) or (is equal to 3)";
  EXPECT_EQ(expected_descr6, DescribeNegation(m));

  m = AllOf(Ge(0), Lt(10), Ne(3), Ne(5), Ne(7));
  std::string expected_desr7 =
      "(isn't >= 0) or (isn't < 10) or (is equal to 3) or (is equal to 5) or "
      "(is equal to 7)";
  EXPECT_EQ(expected_desr7, DescribeNegation(m));

  m = AllOf(Ne(1), Ne(2), Ne(3), Ne(4), Ne(5), Ne(6), Ne(7), Ne(8), Ne(9),
            Ne(10), Ne(11));
  AllOf(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
  EXPECT_THAT(Describe(m), EndsWith("and (isn't equal to 11)"));
  AllOfMatches(11, m);
}

// Tests that monomorphic matchers are safely cast by the AllOf matcher.
TEST(AllOfTest, AllOfMatcherSafelyCastsMonomorphicMatchers) {
  // greater_than_5 and less_than_10 are monomorphic matchers.
  Matcher<int> greater_than_5 = Gt(5);
  Matcher<int> less_than_10 = Lt(10);

  Matcher<const int&> m = AllOf(greater_than_5, less_than_10);
  Matcher<int&> m2 = AllOf(greater_than_5, less_than_10);
  Matcher<int&> m3 = AllOf(greater_than_5, m2);

  // Tests that BothOf works when composing itself.
  Matcher<const int&> m4 = AllOf(greater_than_5, less_than_10, less_than_10);
  Matcher<int&> m5 = AllOf(greater_than_5, less_than_10, less_than_10);
}

TEST_P(AllOfTestP, ExplainsResult) {
  Matcher<int> m;

  // Successful match.  Both matchers need to explain.  The second
  // matcher doesn't give an explanation, so only the first matcher's
  // explanation is printed.
  m = AllOf(GreaterThan(10), Lt(30));
  EXPECT_EQ("which is 15 more than 10", Explain(m, 25));

  // Successful match.  Both matchers need to explain.
  m = AllOf(GreaterThan(10), GreaterThan(20));
  EXPECT_EQ("which is 20 more than 10, and which is 10 more than 20",
            Explain(m, 30));

  // Successful match.  All matchers need to explain.  The second
  // matcher doesn't given an explanation.
  m = AllOf(GreaterThan(10), Lt(30), GreaterThan(20));
  EXPECT_EQ("which is 15 more than 10, and which is 5 more than 20",
            Explain(m, 25));

  // Successful match.  All matchers need to explain.
  m = AllOf(GreaterThan(10), GreaterThan(20), GreaterThan(30));
  EXPECT_EQ(
      "which is 30 more than 10, and which is 20 more than 20, "
      "and which is 10 more than 30",
      Explain(m, 40));

  // Failed match.  The first matcher, which failed, needs to
  // explain.
  m = AllOf(GreaterThan(10), GreaterThan(20));
  EXPECT_EQ("which is 5 less than 10", Explain(m, 5));

  // Failed match.  The second matcher, which failed, needs to
  // explain.  Since it doesn't given an explanation, nothing is
  // printed.
  m = AllOf(GreaterThan(10), Lt(30));
  EXPECT_EQ("", Explain(m, 40));

  // Failed match.  The second matcher, which failed, needs to
  // explain.
  m = AllOf(GreaterThan(10), GreaterThan(20));
  EXPECT_EQ("which is 5 less than 20", Explain(m, 15));
}

// Helper to allow easy testing of AnyOf matchers with num parameters.
static void AnyOfMatches(int num, const Matcher<int>& m) {
  SCOPED_TRACE(Describe(m));
  EXPECT_FALSE(m.Matches(0));
  for (int i = 1; i <= num; ++i) {
    EXPECT_TRUE(m.Matches(i));
  }
  EXPECT_FALSE(m.Matches(num + 1));
}

static void AnyOfStringMatches(int num, const Matcher<std::string>& m) {
  SCOPED_TRACE(Describe(m));
  EXPECT_FALSE(m.Matches(std::to_string(0)));

  for (int i = 1; i <= num; ++i) {
    EXPECT_TRUE(m.Matches(std::to_string(i)));
  }
  EXPECT_FALSE(m.Matches(std::to_string(num + 1)));
}

INSTANTIATE_GTEST_MATCHER_TEST_P(AnyOfTest);

// Tests that AnyOf(m1, ..., mn) matches any value that matches at
// least one of the given matchers.
TEST(AnyOfTest, MatchesWhenAnyMatches) {
  Matcher<int> m;
  m = AnyOf(Le(1), Ge(3));
  EXPECT_TRUE(m.Matches(1));
  EXPECT_TRUE(m.Matches(4));
  EXPECT_FALSE(m.Matches(2));

  m = AnyOf(Lt(0), Eq(1), Eq(2));
  EXPECT_TRUE(m.Matches(-1));
  EXPECT_TRUE(m.Matches(1));
  EXPECT_TRUE(m.Matches(2));
  EXPECT_FALSE(m.Matches(0));

  m = AnyOf(Lt(0), Eq(1), Eq(2), Eq(3));
  EXPECT_TRUE(m.Matches(-1));
  EXPECT_TRUE(m.Matches(1));
  EXPECT_TRUE(m.Matches(2));
  EXPECT_TRUE(m.Matches(3));
  EXPECT_FALSE(m.Matches(0));

  m = AnyOf(Le(0), Gt(10), 3, 5, 7);
  EXPECT_TRUE(m.Matches(0));
  EXPECT_TRUE(m.Matches(11));
  EXPECT_TRUE(m.Matches(3));
  EXPECT_FALSE(m.Matches(2));

  // The following tests for varying number of sub-matchers. Due to the way
  // the sub-matchers are handled it is enough to test every sub-matcher once
  // with sub-matchers using the same matcher type. Varying matcher types are
  // checked for above.
  AnyOfMatches(2, AnyOf(1, 2));
  AnyOfMatches(3, AnyOf(1, 2, 3));
  AnyOfMatches(4, AnyOf(1, 2, 3, 4));
  AnyOfMatches(5, AnyOf(1, 2, 3, 4, 5));
  AnyOfMatches(6, AnyOf(1, 2, 3, 4, 5, 6));
  AnyOfMatches(7, AnyOf(1, 2, 3, 4, 5, 6, 7));
  AnyOfMatches(8, AnyOf(1, 2, 3, 4, 5, 6, 7, 8));
  AnyOfMatches(9, AnyOf(1, 2, 3, 4, 5, 6, 7, 8, 9));
  AnyOfMatches(10, AnyOf(1, 2, 3, 4, 5, 6, 7, 8, 9, 10));
}

// Tests the variadic version of the AnyOfMatcher.
TEST(AnyOfTest, VariadicMatchesWhenAnyMatches) {
  // Also make sure AnyOf is defined in the right namespace and does not depend
  // on ADL.
  Matcher<int> m = ::testing::AnyOf(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);

  EXPECT_THAT(Describe(m), EndsWith("or (is equal to 11)"));
  AnyOfMatches(11, m);
  AnyOfMatches(50, AnyOf(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
                         17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
                         31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44,
                         45, 46, 47, 48, 49, 50));
  AnyOfStringMatches(
      50, AnyOf("1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12",
                "13", "14", "15", "16", "17", "18", "19", "20", "21", "22",
                "23", "24", "25", "26", "27", "28", "29", "30", "31", "32",
                "33", "34", "35", "36", "37", "38", "39", "40", "41", "42",
                "43", "44", "45", "46", "47", "48", "49", "50"));
}

TEST(ConditionalTest, MatchesFirstIfCondition) {
  Matcher<std::string> eq_red = Eq("red");
  Matcher<std::string> ne_red = Ne("red");
  Matcher<std::string> m = Conditional(true, eq_red, ne_red);
  EXPECT_TRUE(m.Matches("red"));
  EXPECT_FALSE(m.Matches("green"));

  StringMatchResultListener listener;
  StringMatchResultListener expected;
  EXPECT_FALSE(m.MatchAndExplain("green", &listener));
  EXPECT_FALSE(eq_red.MatchAndExplain("green", &expected));
  EXPECT_THAT(listener.str(), Eq(expected.str()));
}

TEST(ConditionalTest, MatchesSecondIfCondition) {
  Matcher<std::string> eq_red = Eq("red");
  Matcher<std::string> ne_red = Ne("red");
  Matcher<std::string> m = Conditional(false, eq_red, ne_red);
  EXPECT_FALSE(m.Matches("red"));
  EXPECT_TRUE(m.Matches("green"));

  StringMatchResultListener listener;
  StringMatchResultListener expected;
  EXPECT_FALSE(m.MatchAndExplain("red", &listener));
  EXPECT_FALSE(ne_red.MatchAndExplain("red", &expected));
  EXPECT_THAT(listener.str(), Eq(expected.str()));
}

// Tests that AnyOf(m1, ..., mn) describes itself properly.
TEST(AnyOfTest, CanDescribeSelf) {
  Matcher<int> m;
  m = AnyOf(Le(1), Ge(3));

  EXPECT_EQ("(is <= 1) or (is >= 3)", Describe(m));

  m = AnyOf(Lt(0), Eq(1), Eq(2));
  EXPECT_EQ("(is < 0) or (is equal to 1) or (is equal to 2)", Describe(m));

  m = AnyOf(Lt(0), Eq(1), Eq(2), Eq(3));
  EXPECT_EQ("(is < 0) or (is equal to 1) or (is equal to 2) or (is equal to 3)",
            Describe(m));

  m = AnyOf(Le(0), Gt(10), 3, 5, 7);
  EXPECT_EQ(
      "(is <= 0) or (is > 10) or (is equal to 3) or (is equal to 5) or (is "
      "equal to 7)",
      Describe(m));
}

// Tests that AnyOf(m1, ..., mn) describes its negation properly.
TEST(AnyOfTest, CanDescribeNegation) {
  Matcher<int> m;
  m = AnyOf(Le(1), Ge(3));
  EXPECT_EQ("(isn't <= 1) and (isn't >= 3)", DescribeNegation(m));

  m = AnyOf(Lt(0), Eq(1), Eq(2));
  EXPECT_EQ("(isn't < 0) and (isn't equal to 1) and (isn't equal to 2)",
            DescribeNegation(m));

  m = AnyOf(Lt(0), Eq(1), Eq(2), Eq(3));
  EXPECT_EQ(
      "(isn't < 0) and (isn't equal to 1) and (isn't equal to 2) and (isn't "
      "equal to 3)",
      DescribeNegation(m));

  m = AnyOf(Le(0), Gt(10), 3, 5, 7);
  EXPECT_EQ(
      "(isn't <= 0) and (isn't > 10) and (isn't equal to 3) and (isn't equal "
      "to 5) and (isn't equal to 7)",
      DescribeNegation(m));
}

// Tests that monomorphic matchers are safely cast by the AnyOf matcher.
TEST(AnyOfTest, AnyOfMatcherSafelyCastsMonomorphicMatchers) {
  // greater_than_5 and less_than_10 are monomorphic matchers.
  Matcher<int> greater_than_5 = Gt(5);
  Matcher<int> less_than_10 = Lt(10);

  Matcher<const int&> m = AnyOf(greater_than_5, less_than_10);
  Matcher<int&> m2 = AnyOf(greater_than_5, less_than_10);
  Matcher<int&> m3 = AnyOf(greater_than_5, m2);

  // Tests that EitherOf works when composing itself.
  Matcher<const int&> m4 = AnyOf(greater_than_5, less_than_10, less_than_10);
  Matcher<int&> m5 = AnyOf(greater_than_5, less_than_10, less_than_10);
}

TEST_P(AnyOfTestP, ExplainsResult) {
  Matcher<int> m;

  // Failed match.  Both matchers need to explain.  The second
  // matcher doesn't give an explanation, so only the first matcher's
  // explanation is printed.
  m = AnyOf(GreaterThan(10), Lt(0));
  EXPECT_EQ("which is 5 less than 10", Explain(m, 5));

  // Failed match.  Both matchers need to explain.
  m = AnyOf(GreaterThan(10), GreaterThan(20));
  EXPECT_EQ("which is 5 less than 10, and which is 15 less than 20",
            Explain(m, 5));

  // Failed match.  All matchers need to explain.  The second
  // matcher doesn't given an explanation.
  m = AnyOf(GreaterThan(10), Gt(20), GreaterThan(30));
  EXPECT_EQ("which is 5 less than 10, and which is 25 less than 30",
            Explain(m, 5));

  // Failed match.  All matchers need to explain.
  m = AnyOf(GreaterThan(10), GreaterThan(20), GreaterThan(30));
  EXPECT_EQ(
      "which is 5 less than 10, and which is 15 less than 20, "
      "and which is 25 less than 30",
      Explain(m, 5));

  // Successful match.  The first matcher, which succeeded, needs to
  // explain.
  m = AnyOf(GreaterThan(10), GreaterThan(20));
  EXPECT_EQ("which is 5 more than 10", Explain(m, 15));

  // Successful match.  The second matcher, which succeeded, needs to
  // explain.  Since it doesn't given an explanation, nothing is
  // printed.
  m = AnyOf(GreaterThan(10), Lt(30));
  EXPECT_EQ("", Explain(m, 0));

  // Successful match.  The second matcher, which succeeded, needs to
  // explain.
  m = AnyOf(GreaterThan(30), GreaterThan(20));
  EXPECT_EQ("which is 5 more than 20", Explain(m, 25));
}

// The following predicate function and predicate functor are for
// testing the Truly(predicate) matcher.

// Returns non-zero if the input is positive.  Note that the return
// type of this function is not bool.  It's OK as Truly() accepts any
// unary function or functor whose return type can be implicitly
// converted to bool.
int IsPositive(double x) { return x > 0 ? 1 : 0; }

// This functor returns true if the input is greater than the given
// number.
class IsGreaterThan {
 public:
  explicit IsGreaterThan(int threshold) : threshold_(threshold) {}

  bool operator()(int n) const { return n > threshold_; }

 private:
  int threshold_;
};

// For testing Truly().
const int foo = 0;

// This predicate returns true if and only if the argument references foo and
// has a zero value.
bool ReferencesFooAndIsZero(const int& n) { return (&n == &foo) && (n == 0); }

// Tests that Truly(predicate) matches what satisfies the given
// predicate.
TEST(TrulyTest, MatchesWhatSatisfiesThePredicate) {
  Matcher<double> m = Truly(IsPositive);
  EXPECT_TRUE(m.Matches(2.0));
  EXPECT_FALSE(m.Matches(-1.5));
}

// Tests that Truly(predicate_functor) works too.
TEST(TrulyTest, CanBeUsedWithFunctor) {
  Matcher<int> m = Truly(IsGreaterThan(5));
  EXPECT_TRUE(m.Matches(6));
  EXPECT_FALSE(m.Matches(4));
}

// A class that can be implicitly converted to bool.
class ConvertibleToBool {
 public:
  explicit ConvertibleToBool(int number) : number_(number) {}
  operator bool() const { return number_ != 0; }

 private:
  int number_;
};

ConvertibleToBool IsNotZero(int number) { return ConvertibleToBool(number); }

// Tests that the predicate used in Truly() may return a class that's
// implicitly convertible to bool, even when the class has no
// operator!().
TEST(TrulyTest, PredicateCanReturnAClassConvertibleToBool) {
  Matcher<int> m = Truly(IsNotZero);
  EXPECT_TRUE(m.Matches(1));
  EXPECT_FALSE(m.Matches(0));
}

// Tests that Truly(predicate) can describe itself properly.
TEST(TrulyTest, CanDescribeSelf) {
  Matcher<double> m = Truly(IsPositive);
  EXPECT_EQ("satisfies the given predicate", Describe(m));
}

// Tests that Truly(predicate) works when the matcher takes its
// argument by reference.
TEST(TrulyTest, WorksForByRefArguments) {
  Matcher<const int&> m = Truly(ReferencesFooAndIsZero);
  EXPECT_TRUE(m.Matches(foo));
  int n = 0;
  EXPECT_FALSE(m.Matches(n));
}

// Tests that Truly(predicate) provides a helpful reason when it fails.
TEST(TrulyTest, ExplainsFailures) {
  StringMatchResultListener listener;
  EXPECT_FALSE(ExplainMatchResult(Truly(IsPositive), -1, &listener));
  EXPECT_EQ(listener.str(), "didn't satisfy the given predicate");
}

// Tests that Matches(m) is a predicate satisfied by whatever that
// matches matcher m.
TEST(MatchesTest, IsSatisfiedByWhatMatchesTheMatcher) {
  EXPECT_TRUE(Matches(Ge(0))(1));
  EXPECT_FALSE(Matches(Eq('a'))('b'));
}

// Tests that Matches(m) works when the matcher takes its argument by
// reference.
TEST(MatchesTest, WorksOnByRefArguments) {
  int m = 0, n = 0;
  EXPECT_TRUE(Matches(AllOf(Ref(n), Eq(0)))(n));
  EXPECT_FALSE(Matches(Ref(m))(n));
}

// Tests that a Matcher on non-reference type can be used in
// Matches().
TEST(MatchesTest, WorksWithMatcherOnNonRefType) {
  Matcher<int> eq5 = Eq(5);
  EXPECT_TRUE(Matches(eq5)(5));
  EXPECT_FALSE(Matches(eq5)(2));
}

// Tests Value(value, matcher).  Since Value() is a simple wrapper for
// Matches(), which has been tested already, we don't spend a lot of
// effort on testing Value().
TEST(ValueTest, WorksWithPolymorphicMatcher) {
  EXPECT_TRUE(Value("hi", StartsWith("h")));
  EXPECT_FALSE(Value(5, Gt(10)));
}

TEST(ValueTest, WorksWithMonomorphicMatcher) {
  const Matcher<int> is_zero = Eq(0);
  EXPECT_TRUE(Value(0, is_zero));
  EXPECT_FALSE(Value('a', is_zero));

  int n = 0;
  const Matcher<const int&> ref_n = Ref(n);
  EXPECT_TRUE(Value(n, ref_n));
  EXPECT_FALSE(Value(1, ref_n));
}

TEST(AllArgsTest, WorksForTuple) {
  EXPECT_THAT(std::make_tuple(1, 2L), AllArgs(Lt()));
  EXPECT_THAT(std::make_tuple(2L, 1), Not(AllArgs(Lt())));
}

TEST(AllArgsTest, WorksForNonTuple) {
  EXPECT_THAT(42, AllArgs(Gt(0)));
  EXPECT_THAT('a', Not(AllArgs(Eq('b'))));
}

class AllArgsHelper {
 public:
  AllArgsHelper() {}

  MOCK_METHOD2(Helper, int(char x, int y));

 private:
  AllArgsHelper(const AllArgsHelper&) = delete;
  AllArgsHelper& operator=(const AllArgsHelper&) = delete;
};

TEST(AllArgsTest, WorksInWithClause) {
  AllArgsHelper helper;
  ON_CALL(helper, Helper(_, _)).With(AllArgs(Lt())).WillByDefault(Return(1));
  EXPECT_CALL(helper, Helper(_, _));
  EXPECT_CALL(helper, Helper(_, _)).With(AllArgs(Gt())).WillOnce(Return(2));

  EXPECT_EQ(1, helper.Helper('\1', 2));
  EXPECT_EQ(2, helper.Helper('a', 1));
}

class OptionalMatchersHelper {
 public:
  OptionalMatchersHelper() {}

  MOCK_METHOD0(NoArgs, int());

  MOCK_METHOD1(OneArg, int(int y));

  MOCK_METHOD2(TwoArgs, int(char x, int y));

  MOCK_METHOD1(Overloaded, int(char x));
  MOCK_METHOD2(Overloaded, int(char x, int y));

 private:
  OptionalMatchersHelper(const OptionalMatchersHelper&) = delete;
  OptionalMatchersHelper& operator=(const OptionalMatchersHelper&) = delete;
};

TEST(AllArgsTest, WorksWithoutMatchers) {
  OptionalMatchersHelper helper;

  ON_CALL(helper, NoArgs).WillByDefault(Return(10));
  ON_CALL(helper, OneArg).WillByDefault(Return(20));
  ON_CALL(helper, TwoArgs).WillByDefault(Return(30));

  EXPECT_EQ(10, helper.NoArgs());
  EXPECT_EQ(20, helper.OneArg(1));
  EXPECT_EQ(30, helper.TwoArgs('\1', 2));

  EXPECT_CALL(helper, NoArgs).Times(1);
  EXPECT_CALL(helper, OneArg).WillOnce(Return(100));
  EXPECT_CALL(helper, OneArg(17)).WillOnce(Return(200));
  EXPECT_CALL(helper, TwoArgs).Times(0);

  EXPECT_EQ(10, helper.NoArgs());
  EXPECT_EQ(100, helper.OneArg(1));
  EXPECT_EQ(200, helper.OneArg(17));
}

// Tests floating-point matchers.
template <typename RawType>
class FloatingPointTest : public testing::Test {
 protected:
  typedef testing::internal::FloatingPoint<RawType> Floating;
  typedef typename Floating::Bits Bits;

  FloatingPointTest()
      : max_ulps_(Floating::kMaxUlps),
        zero_bits_(Floating(0).bits()),
        one_bits_(Floating(1).bits()),
        infinity_bits_(Floating(Floating::Infinity()).bits()),
        close_to_positive_zero_(
            Floating::ReinterpretBits(zero_bits_ + max_ulps_ / 2)),
        close_to_negative_zero_(
            -Floating::ReinterpretBits(zero_bits_ + max_ulps_ - max_ulps_ / 2)),
        further_from_negative_zero_(-Floating::ReinterpretBits(
            zero_bits_ + max_ulps_ + 1 - max_ulps_ / 2)),
        close_to_one_(Floating::ReinterpretBits(one_bits_ + max_ulps_)),
        further_from_one_(Floating::ReinterpretBits(one_bits_ + max_ulps_ + 1)),
        infinity_(Floating::Infinity()),
        close_to_infinity_(
            Floating::ReinterpretBits(infinity_bits_ - max_ulps_)),
        further_from_infinity_(
            Floating::ReinterpretBits(infinity_bits_ - max_ulps_ - 1)),
        max_(std::numeric_limits<RawType>::max()),
        nan1_(Floating::ReinterpretBits(Floating::kExponentBitMask | 1)),
        nan2_(Floating::ReinterpretBits(Floating::kExponentBitMask | 200)) {}

  void TestSize() { EXPECT_EQ(sizeof(RawType), sizeof(Bits)); }

  // A battery of tests for FloatingEqMatcher::Matches.
  // matcher_maker is a pointer to a function which creates a FloatingEqMatcher.
  void TestMatches(
      testing::internal::FloatingEqMatcher<RawType> (*matcher_maker)(RawType)) {
    Matcher<RawType> m1 = matcher_maker(0.0);
    EXPECT_TRUE(m1.Matches(-0.0));
    EXPECT_TRUE(m1.Matches(close_to_positive_zero_));
    EXPECT_TRUE(m1.Matches(close_to_negative_zero_));
    EXPECT_FALSE(m1.Matches(1.0));

    Matcher<RawType> m2 = matcher_maker(close_to_positive_zero_);
    EXPECT_FALSE(m2.Matches(further_from_negative_zero_));

    Matcher<RawType> m3 = matcher_maker(1.0);
    EXPECT_TRUE(m3.Matches(close_to_one_));
    EXPECT_FALSE(m3.Matches(further_from_one_));

    // Test commutativity: matcher_maker(0.0).Matches(1.0) was tested above.
    EXPECT_FALSE(m3.Matches(0.0));

    Matcher<RawType> m4 = matcher_maker(-infinity_);
    EXPECT_TRUE(m4.Matches(-close_to_infinity_));

    Matcher<RawType> m5 = matcher_maker(infinity_);
    EXPECT_TRUE(m5.Matches(close_to_infinity_));

    // This is interesting as the representations of infinity_ and nan1_
    // are only 1 DLP apart.
    EXPECT_FALSE(m5.Matches(nan1_));

    // matcher_maker can produce a Matcher<const RawType&>, which is needed in
    // some cases.
    Matcher<const RawType&> m6 = matcher_maker(0.0);
    EXPECT_TRUE(m6.Matches(-0.0));
    EXPECT_TRUE(m6.Matches(close_to_positive_zero_));
    EXPECT_FALSE(m6.Matches(1.0));

    // matcher_maker can produce a Matcher<RawType&>, which is needed in some
    // cases.
    Matcher<RawType&> m7 = matcher_maker(0.0);
    RawType x = 0.0;
    EXPECT_TRUE(m7.Matches(x));
    x = 0.01f;
    EXPECT_FALSE(m7.Matches(x));
  }

  // Pre-calculated numbers to be used by the tests.

  const Bits max_ulps_;

  const Bits zero_bits_;      // The bits that represent 0.0.
  const Bits one_bits_;       // The bits that represent 1.0.
  const Bits infinity_bits_;  // The bits that represent +infinity.

  // Some numbers close to 0.0.
  const RawType close_to_positive_zero_;
  const RawType close_to_negative_zero_;
  const RawType further_from_negative_zero_;

  // Some numbers close to 1.0.
  const RawType close_to_one_;
  const RawType further_from_one_;

  // Some numbers close to +infinity.
  const RawType infinity_;
  const RawType close_to_infinity_;
  const RawType further_from_infinity_;

  // Maximum representable value that's not infinity.
  const RawType max_;

  // Some NaNs.
  const RawType nan1_;
  const RawType nan2_;
};

// Tests floating-point matchers with fixed epsilons.
template <typename RawType>
class FloatingPointNearTest : public FloatingPointTest<RawType> {
 protected:
  typedef FloatingPointTest<RawType> ParentType;

  // A battery of tests for FloatingEqMatcher::Matches with a fixed epsilon.
  // matcher_maker is a pointer to a function which creates a FloatingEqMatcher.
  void TestNearMatches(testing::internal::FloatingEqMatcher<RawType> (
      *matcher_maker)(RawType, RawType)) {
    Matcher<RawType> m1 = matcher_maker(0.0, 0.0);
    EXPECT_TRUE(m1.Matches(0.0));
    EXPECT_TRUE(m1.Matches(-0.0));
    EXPECT_FALSE(m1.Matches(ParentType::close_to_positive_zero_));
    EXPECT_FALSE(m1.Matches(ParentType::close_to_negative_zero_));
    EXPECT_FALSE(m1.Matches(1.0));

    Matcher<RawType> m2 = matcher_maker(0.0, 1.0);
    EXPECT_TRUE(m2.Matches(0.0));
    EXPECT_TRUE(m2.Matches(-0.0));
    EXPECT_TRUE(m2.Matches(1.0));
    EXPECT_TRUE(m2.Matches(-1.0));
    EXPECT_FALSE(m2.Matches(ParentType::close_to_one_));
    EXPECT_FALSE(m2.Matches(-ParentType::close_to_one_));

    // Check that inf matches inf, regardless of the of the specified max
    // absolute error.
    Matcher<RawType> m3 = matcher_maker(ParentType::infinity_, 0.0);
    EXPECT_TRUE(m3.Matches(ParentType::infinity_));
    EXPECT_FALSE(m3.Matches(ParentType::close_to_infinity_));
    EXPECT_FALSE(m3.Matches(-ParentType::infinity_));

    Matcher<RawType> m4 = matcher_maker(-ParentType::infinity_, 0.0);
    EXPECT_TRUE(m4.Matches(-ParentType::infinity_));
    EXPECT_FALSE(m4.Matches(-ParentType::close_to_infinity_));
    EXPECT_FALSE(m4.Matches(ParentType::infinity_));

    // Test various overflow scenarios.
    Matcher<RawType> m5 = matcher_maker(ParentType::max_, ParentType::max_);
    EXPECT_TRUE(m5.Matches(ParentType::max_));
    EXPECT_FALSE(m5.Matches(-ParentType::max_));

    Matcher<RawType> m6 = matcher_maker(-ParentType::max_, ParentType::max_);
    EXPECT_FALSE(m6.Matches(ParentType::max_));
    EXPECT_TRUE(m6.Matches(-ParentType::max_));

    Matcher<RawType> m7 = matcher_maker(ParentType::max_, 0);
    EXPECT_TRUE(m7.Matches(ParentType::max_));
    EXPECT_FALSE(m7.Matches(-ParentType::max_));

    Matcher<RawType> m8 = matcher_maker(-ParentType::max_, 0);
    EXPECT_FALSE(m8.Matches(ParentType::max_));
    EXPECT_TRUE(m8.Matches(-ParentType::max_));

    // The difference between max() and -max() normally overflows to infinity,
    // but it should still match if the max_abs_error is also infinity.
    Matcher<RawType> m9 =
        matcher_maker(ParentType::max_, ParentType::infinity_);
    EXPECT_TRUE(m8.Matches(-ParentType::max_));

    // matcher_maker can produce a Matcher<const RawType&>, which is needed in
    // some cases.
    Matcher<const RawType&> m10 = matcher_maker(0.0, 1.0);
    EXPECT_TRUE(m10.Matches(-0.0));
    EXPECT_TRUE(m10.Matches(ParentType::close_to_positive_zero_));
    EXPECT_FALSE(m10.Matches(ParentType::close_to_one_));

    // matcher_maker can produce a Matcher<RawType&>, which is needed in some
    // cases.
    Matcher<RawType&> m11 = matcher_maker(0.0, 1.0);
    RawType x = 0.0;
    EXPECT_TRUE(m11.Matches(x));
    x = 1.0f;
    EXPECT_TRUE(m11.Matches(x));
    x = -1.0f;
    EXPECT_TRUE(m11.Matches(x));
    x = 1.1f;
    EXPECT_FALSE(m11.Matches(x));
    x = -1.1f;
    EXPECT_FALSE(m11.Matches(x));
  }
};

// Instantiate FloatingPointTest for testing floats.
typedef FloatingPointTest<float> FloatTest;

TEST_F(FloatTest, FloatEqApproximatelyMatchesFloats) { TestMatches(&FloatEq); }

TEST_F(FloatTest, NanSensitiveFloatEqApproximatelyMatchesFloats) {
  TestMatches(&NanSensitiveFloatEq);
}

TEST_F(FloatTest, FloatEqCannotMatchNaN) {
  // FloatEq never matches NaN.
  Matcher<float> m = FloatEq(nan1_);
  EXPECT_FALSE(m.Matches(nan1_));
  EXPECT_FALSE(m.Matches(nan2_));
  EXPECT_FALSE(m.Matches(1.0));
}

TEST_F(FloatTest, NanSensitiveFloatEqCanMatchNaN) {
  // NanSensitiveFloatEq will match NaN.
  Matcher<float> m = NanSensitiveFloatEq(nan1_);
  EXPECT_TRUE(m.Matches(nan1_));
  EXPECT_TRUE(m.Matches(nan2_));
  EXPECT_FALSE(m.Matches(1.0));
}

TEST_F(FloatTest, FloatEqCanDescribeSelf) {
  Matcher<float> m1 = FloatEq(2.0f);
  EXPECT_EQ("is approximately 2", Describe(m1));
  EXPECT_EQ("isn't approximately 2", DescribeNegation(m1));

  Matcher<float> m2 = FloatEq(0.5f);
  EXPECT_EQ("is approximately 0.5", Describe(m2));
  EXPECT_EQ("isn't approximately 0.5", DescribeNegation(m2));

  Matcher<float> m3 = FloatEq(nan1_);
  EXPECT_EQ("never matches", Describe(m3));
  EXPECT_EQ("is anything", DescribeNegation(m3));
}

TEST_F(FloatTest, NanSensitiveFloatEqCanDescribeSelf) {
  Matcher<float> m1 = NanSensitiveFloatEq(2.0f);
  EXPECT_EQ("is approximately 2", Describe(m1));
  EXPECT_EQ("isn't approximately 2", DescribeNegation(m1));

  Matcher<float> m2 = NanSensitiveFloatEq(0.5f);
  EXPECT_EQ("is approximately 0.5", Describe(m2));
  EXPECT_EQ("isn't approximately 0.5", DescribeNegation(m2));

  Matcher<float> m3 = NanSensitiveFloatEq(nan1_);
  EXPECT_EQ("is NaN", Describe(m3));
  EXPECT_EQ("isn't NaN", DescribeNegation(m3));
}

// Instantiate FloatingPointTest for testing floats with a user-specified
// max absolute error.
typedef FloatingPointNearTest<float> FloatNearTest;

TEST_F(FloatNearTest, FloatNearMatches) { TestNearMatches(&FloatNear); }

TEST_F(FloatNearTest, NanSensitiveFloatNearApproximatelyMatchesFloats) {
  TestNearMatches(&NanSensitiveFloatNear);
}

TEST_F(FloatNearTest, FloatNearCanDescribeSelf) {
  Matcher<float> m1 = FloatNear(2.0f, 0.5f);
  EXPECT_EQ("is approximately 2 (absolute error <= 0.5)", Describe(m1));
  EXPECT_EQ("isn't approximately 2 (absolute error > 0.5)",
            DescribeNegation(m1));

  Matcher<float> m2 = FloatNear(0.5f, 0.5f);
  EXPECT_EQ("is approximately 0.5 (absolute error <= 0.5)", Describe(m2));
  EXPECT_EQ("isn't approximately 0.5 (absolute error > 0.5)",
            DescribeNegation(m2));

  Matcher<float> m3 = FloatNear(nan1_, 0.0);
  EXPECT_EQ("never matches", Describe(m3));
  EXPECT_EQ("is anything", DescribeNegation(m3));
}

TEST_F(FloatNearTest, NanSensitiveFloatNearCanDescribeSelf) {
  Matcher<float> m1 = NanSensitiveFloatNear(2.0f, 0.5f);
  EXPECT_EQ("is approximately 2 (absolute error <= 0.5)", Describe(m1));
  EXPECT_EQ("isn't approximately 2 (absolute error > 0.5)",
            DescribeNegation(m1));

  Matcher<float> m2 = NanSensitiveFloatNear(0.5f, 0.5f);
  EXPECT_EQ("is approximately 0.5 (absolute error <= 0.5)", Describe(m2));
  EXPECT_EQ("isn't approximately 0.5 (absolute error > 0.5)",
            DescribeNegation(m2));

  Matcher<float> m3 = NanSensitiveFloatNear(nan1_, 0.1f);
  EXPECT_EQ("is NaN", Describe(m3));
  EXPECT_EQ("isn't NaN", DescribeNegation(m3));
}

TEST_F(FloatNearTest, FloatNearCannotMatchNaN) {
  // FloatNear never matches NaN.
  Matcher<float> m = FloatNear(ParentType::nan1_, 0.1f);
  EXPECT_FALSE(m.Matches(nan1_));
  EXPECT_FALSE(m.Matches(nan2_));
  EXPECT_FALSE(m.Matches(1.0));
}

TEST_F(FloatNearTest, NanSensitiveFloatNearCanMatchNaN) {
  // NanSensitiveFloatNear will match NaN.
  Matcher<float> m = NanSensitiveFloatNear(nan1_, 0.1f);
  EXPECT_TRUE(m.Matches(nan1_));
  EXPECT_TRUE(m.Matches(nan2_));
  EXPECT_FALSE(m.Matches(1.0));
}

// Instantiate FloatingPointTest for testing doubles.
typedef FloatingPointTest<double> DoubleTest;

TEST_F(DoubleTest, DoubleEqApproximatelyMatchesDoubles) {
  TestMatches(&DoubleEq);
}

TEST_F(DoubleTest, NanSensitiveDoubleEqApproximatelyMatchesDoubles) {
  TestMatches(&NanSensitiveDoubleEq);
}

TEST_F(DoubleTest, DoubleEqCannotMatchNaN) {
  // DoubleEq never matches NaN.
  Matcher<double> m = DoubleEq(nan1_);
  EXPECT_FALSE(m.Matches(nan1_));
  EXPECT_FALSE(m.Matches(nan2_));
  EXPECT_FALSE(m.Matches(1.0));
}

TEST_F(DoubleTest, NanSensitiveDoubleEqCanMatchNaN) {
  // NanSensitiveDoubleEq will match NaN.
  Matcher<double> m = NanSensitiveDoubleEq(nan1_);
  EXPECT_TRUE(m.Matches(nan1_));
  EXPECT_TRUE(m.Matches(nan2_));
  EXPECT_FALSE(m.Matches(1.0));
}

TEST_F(DoubleTest, DoubleEqCanDescribeSelf) {
  Matcher<double> m1 = DoubleEq(2.0);
  EXPECT_EQ("is approximately 2", Describe(m1));
  EXPECT_EQ("isn't approximately 2", DescribeNegation(m1));

  Matcher<double> m2 = DoubleEq(0.5);
  EXPECT_EQ("is approximately 0.5", Describe(m2));
  EXPECT_EQ("isn't approximately 0.5", DescribeNegation(m2));

  Matcher<double> m3 = DoubleEq(nan1_);
  EXPECT_EQ("never matches", Describe(m3));
  EXPECT_EQ("is anything", DescribeNegation(m3));
}

TEST_F(DoubleTest, NanSensitiveDoubleEqCanDescribeSelf) {
  Matcher<double> m1 = NanSensitiveDoubleEq(2.0);
  EXPECT_EQ("is approximately 2", Describe(m1));
  EXPECT_EQ("isn't approximately 2", DescribeNegation(m1));

  Matcher<double> m2 = NanSensitiveDoubleEq(0.5);
  EXPECT_EQ("is approximately 0.5", Describe(m2));
  EXPECT_EQ("isn't approximately 0.5", DescribeNegation(m2));

  Matcher<double> m3 = NanSensitiveDoubleEq(nan1_);
  EXPECT_EQ("is NaN", Describe(m3));
  EXPECT_EQ("isn't NaN", DescribeNegation(m3));
}

// Instantiate FloatingPointTest for testing floats with a user-specified
// max absolute error.
typedef FloatingPointNearTest<double> DoubleNearTest;

TEST_F(DoubleNearTest, DoubleNearMatches) { TestNearMatches(&DoubleNear); }

TEST_F(DoubleNearTest, NanSensitiveDoubleNearApproximatelyMatchesDoubles) {
  TestNearMatches(&NanSensitiveDoubleNear);
}

TEST_F(DoubleNearTest, DoubleNearCanDescribeSelf) {
  Matcher<double> m1 = DoubleNear(2.0, 0.5);
  EXPECT_EQ("is approximately 2 (absolute error <= 0.5)", Describe(m1));
  EXPECT_EQ("isn't approximately 2 (absolute error > 0.5)",
            DescribeNegation(m1));

  Matcher<double> m2 = DoubleNear(0.5, 0.5);
  EXPECT_EQ("is approximately 0.5 (absolute error <= 0.5)", Describe(m2));
  EXPECT_EQ("isn't approximately 0.5 (absolute error > 0.5)",
            DescribeNegation(m2));

  Matcher<double> m3 = DoubleNear(nan1_, 0.0);
  EXPECT_EQ("never matches", Describe(m3));
  EXPECT_EQ("is anything", DescribeNegation(m3));
}

TEST_F(DoubleNearTest, ExplainsResultWhenMatchFails) {
  EXPECT_EQ("", Explain(DoubleNear(2.0, 0.1), 2.05));
  EXPECT_EQ("which is 0.2 from 2", Explain(DoubleNear(2.0, 0.1), 2.2));
  EXPECT_EQ("which is -0.3 from 2", Explain(DoubleNear(2.0, 0.1), 1.7));

  const std::string explanation =
      Explain(DoubleNear(2.1, 1e-10), 2.1 + 1.2e-10);
  // Different C++ implementations may print floating-point numbers
  // slightly differently.
  EXPECT_TRUE(explanation == "which is 1.2e-10 from 2.1" ||  // GCC
              explanation == "which is 1.2e-010 from 2.1")   // MSVC
      << " where explanation is \"" << explanation << "\".";
}

TEST_F(DoubleNearTest, NanSensitiveDoubleNearCanDescribeSelf) {
  Matcher<double> m1 = NanSensitiveDoubleNear(2.0, 0.5);
  EXPECT_EQ("is approximately 2 (absolute error <= 0.5)", Describe(m1));
  EXPECT_EQ("isn't approximately 2 (absolute error > 0.5)",
            DescribeNegation(m1));

  Matcher<double> m2 = NanSensitiveDoubleNear(0.5, 0.5);
  EXPECT_EQ("is approximately 0.5 (absolute error <= 0.5)", Describe(m2));
  EXPECT_EQ("isn't approximately 0.5 (absolute error > 0.5)",
            DescribeNegation(m2));

  Matcher<double> m3 = NanSensitiveDoubleNear(nan1_, 0.1);
  EXPECT_EQ("is NaN", Describe(m3));
  EXPECT_EQ("isn't NaN", DescribeNegation(m3));
}

TEST_F(DoubleNearTest, DoubleNearCannotMatchNaN) {
  // DoubleNear never matches NaN.
  Matcher<double> m = DoubleNear(ParentType::nan1_, 0.1);
  EXPECT_FALSE(m.Matches(nan1_));
  EXPECT_FALSE(m.Matches(nan2_));
  EXPECT_FALSE(m.Matches(1.0));
}

TEST_F(DoubleNearTest, NanSensitiveDoubleNearCanMatchNaN) {
  // NanSensitiveDoubleNear will match NaN.
  Matcher<double> m = NanSensitiveDoubleNear(nan1_, 0.1);
  EXPECT_TRUE(m.Matches(nan1_));
  EXPECT_TRUE(m.Matches(nan2_));
  EXPECT_FALSE(m.Matches(1.0));
}

TEST(NotTest, WorksOnMoveOnlyType) {
  std::unique_ptr<int> p(new int(3));
  EXPECT_THAT(p, Pointee(Eq(3)));
  EXPECT_THAT(p, Not(Pointee(Eq(2))));
}

TEST(AllOfTest, HugeMatcher) {
  // Verify that using AllOf with many arguments doesn't cause
  // the compiler to exceed template instantiation depth limit.
  EXPECT_THAT(0, testing::AllOf(_, _, _, _, _, _, _, _, _,
                                testing::AllOf(_, _, _, _, _, _, _, _, _, _)));
}

TEST(AnyOfTest, HugeMatcher) {
  // Verify that using AnyOf with many arguments doesn't cause
  // the compiler to exceed template instantiation depth limit.
  EXPECT_THAT(0, testing::AnyOf(_, _, _, _, _, _, _, _, _,
                                testing::AnyOf(_, _, _, _, _, _, _, _, _, _)));
}

namespace adl_test {

// Verifies that the implementation of ::testing::AllOf and ::testing::AnyOf
// don't issue unqualified recursive calls.  If they do, the argument dependent
// name lookup will cause AllOf/AnyOf in the 'adl_test' namespace to be found
// as a candidate and the compilation will break due to an ambiguous overload.

// The matcher must be in the same namespace as AllOf/AnyOf to make argument
// dependent lookup find those.
MATCHER(M, "") {
  (void)arg;
  return true;
}

template <typename T1, typename T2>
bool AllOf(const T1& /*t1*/, const T2& /*t2*/) {
  return true;
}

TEST(AllOfTest, DoesNotCallAllOfUnqualified) {
  EXPECT_THAT(42,
              testing::AllOf(M(), M(), M(), M(), M(), M(), M(), M(), M(), M()));
}

template <typename T1, typename T2>
bool AnyOf(const T1&, const T2&) {
  return true;
}

TEST(AnyOfTest, DoesNotCallAnyOfUnqualified) {
  EXPECT_THAT(42,
              testing::AnyOf(M(), M(), M(), M(), M(), M(), M(), M(), M(), M()));
}

}  // namespace adl_test

TEST(AllOfTest, WorksOnMoveOnlyType) {
  std::unique_ptr<int> p(new int(3));
  EXPECT_THAT(p, AllOf(Pointee(Eq(3)), Pointee(Gt(0)), Pointee(Lt(5))));
  EXPECT_THAT(p, Not(AllOf(Pointee(Eq(3)), Pointee(Gt(0)), Pointee(Lt(3)))));
}

TEST(AnyOfTest, WorksOnMoveOnlyType) {
  std::unique_ptr<int> p(new int(3));
  EXPECT_THAT(p, AnyOf(Pointee(Eq(5)), Pointee(Lt(0)), Pointee(Lt(5))));
  EXPECT_THAT(p, Not(AnyOf(Pointee(Eq(5)), Pointee(Lt(0)), Pointee(Gt(5)))));
}

}  // namespace
}  // namespace gmock_matchers_test
}  // namespace testing

#ifdef _MSC_VER
#pragma warning(pop)
#endif
