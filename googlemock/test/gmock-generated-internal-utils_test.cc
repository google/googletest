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
//
// Author: wan@google.com (Zhanyong Wan)

// Google Mock - a framework for writing C++ mock classes.
//
// This file tests the internal utilities.

#include "gmock/internal/gmock-generated-internal-utils.h"
#include "gmock/internal/gmock-internal-utils.h"
#include "gtest/gtest.h"

namespace {

using ::testing::tuple;
using ::testing::Matcher;
using ::testing::internal::CompileAssertTypesEqual;
using ::testing::internal::MatcherTuple;
using ::testing::internal::Function;
using ::testing::internal::IgnoredValue;

// Tests the MatcherTuple template struct.

TEST(MatcherTupleTest, ForSize0) {
  CompileAssertTypesEqual<tuple<>, MatcherTuple<tuple<> >::type>();
}

TEST(MatcherTupleTest, ForSize1) {
  CompileAssertTypesEqual<tuple<Matcher<int> >,
                          MatcherTuple<tuple<int> >::type>();
}

TEST(MatcherTupleTest, ForSize2) {
  CompileAssertTypesEqual<tuple<Matcher<int>, Matcher<char> >,
                          MatcherTuple<tuple<int, char> >::type>();
}

TEST(MatcherTupleTest, ForSize5) {
  CompileAssertTypesEqual<tuple<Matcher<int>, Matcher<char>, Matcher<bool>,
                                Matcher<double>, Matcher<char*> >,
                          MatcherTuple<tuple<int, char, bool, double, char*>
                                      >::type>();
}

// Tests the Function template struct.

TEST(FunctionTest, Nullary) {
  typedef Function<int()> F;  // NOLINT
  CompileAssertTypesEqual<int, F::Result>();
  CompileAssertTypesEqual<tuple<>, F::ArgumentTuple>();
  CompileAssertTypesEqual<tuple<>, F::ArgumentMatcherTuple>();
  CompileAssertTypesEqual<void(), F::MakeResultVoid>();
  CompileAssertTypesEqual<IgnoredValue(), F::MakeResultIgnoredValue>();
}

TEST(FunctionTest, Unary) {
  typedef Function<int(bool)> F;  // NOLINT
  CompileAssertTypesEqual<int, F::Result>();
  CompileAssertTypesEqual<bool, F::Argument1>();
  CompileAssertTypesEqual<tuple<bool>, F::ArgumentTuple>();
  CompileAssertTypesEqual<tuple<Matcher<bool> >, F::ArgumentMatcherTuple>();
  CompileAssertTypesEqual<void(bool), F::MakeResultVoid>();  // NOLINT
  CompileAssertTypesEqual<IgnoredValue(bool),  // NOLINT
      F::MakeResultIgnoredValue>();
}

TEST(FunctionTest, Binary) {
  typedef Function<int(bool, const long&)> F;  // NOLINT
  CompileAssertTypesEqual<int, F::Result>();
  CompileAssertTypesEqual<bool, F::Argument1>();
  CompileAssertTypesEqual<const long&, F::Argument2>();  // NOLINT
  CompileAssertTypesEqual<tuple<bool, const long&>, F::ArgumentTuple>();  // NOLINT
  CompileAssertTypesEqual<tuple<Matcher<bool>, Matcher<const long&> >,  // NOLINT
                          F::ArgumentMatcherTuple>();
  CompileAssertTypesEqual<void(bool, const long&), F::MakeResultVoid>();  // NOLINT
  CompileAssertTypesEqual<IgnoredValue(bool, const long&),  // NOLINT
      F::MakeResultIgnoredValue>();
}

TEST(FunctionTest, LongArgumentList) {
  typedef Function<char(bool, int, char*, int&, const long&)> F;  // NOLINT
  CompileAssertTypesEqual<char, F::Result>();
  CompileAssertTypesEqual<bool, F::Argument1>();
  CompileAssertTypesEqual<int, F::Argument2>();
  CompileAssertTypesEqual<char*, F::Argument3>();
  CompileAssertTypesEqual<int&, F::Argument4>();
  CompileAssertTypesEqual<const long&, F::Argument5>();  // NOLINT
  CompileAssertTypesEqual<tuple<bool, int, char*, int&, const long&>,  // NOLINT
                          F::ArgumentTuple>();
  CompileAssertTypesEqual<tuple<Matcher<bool>, Matcher<int>, Matcher<char*>,
                                Matcher<int&>, Matcher<const long&> >,  // NOLINT
                          F::ArgumentMatcherTuple>();
  CompileAssertTypesEqual<void(bool, int, char*, int&, const long&),  // NOLINT
                          F::MakeResultVoid>();
  CompileAssertTypesEqual<
      IgnoredValue(bool, int, char*, int&, const long&),  // NOLINT
      F::MakeResultIgnoredValue>();
}

// Tests for argument counting macros

TEST(ArgumentMacros, Count) {
#if __GMOCK_NARGS() == 1
    // XXX: gcc with -std=c++11 (rather than gnu++11) doesn't support 
    //  the ##__VA_ARGS__ extension, and fails this test.
    // Disabling the test is easy. MOCK_METHOD that distinguishes between 0 and 1 parameters is harder.
#else
	EXPECT_EQ(0,  __GMOCK_NARGS());
#endif
	EXPECT_EQ(1,  __GMOCK_NARGS(a));
	EXPECT_EQ(2,  __GMOCK_NARGS(a, b));
	EXPECT_EQ(3,  __GMOCK_NARGS(a, b, c));
	EXPECT_EQ(4,  __GMOCK_NARGS(a, b, c, d));
	EXPECT_EQ(5,  __GMOCK_NARGS(a, b, c, d, e));
	EXPECT_EQ(6,  __GMOCK_NARGS(a, b, c, d, e, f));
	EXPECT_EQ(7,  __GMOCK_NARGS(a, b, c, d, e, f, g));
	EXPECT_EQ(8,  __GMOCK_NARGS(a, b, c, d, e, f, g, h));
	EXPECT_EQ(9,  __GMOCK_NARGS(a, b, c, d, e, f, g, h, i));
	EXPECT_EQ(10, __GMOCK_NARGS(a, b, c, d, e, f, g, h, i, j));
}

TEST(ArgumentMacros, FirstN)
{
    int a0[]  = { __GMOCK_FIRST(0,  1, 2, 3, 4, 5, 6, 7, 8, 9, 10) };
    int a1[]  = { __GMOCK_FIRST(1,  1, 2, 3, 4, 5, 6, 7, 8, 9, 10) };
    int a2[]  = { __GMOCK_FIRST(2,  1, 2, 3, 4, 5, 6, 7, 8, 9, 10) };
    int a3[]  = { __GMOCK_FIRST(3,  1, 2, 3, 4, 5, 6, 7, 8, 9, 10) };
    int a4[]  = { __GMOCK_FIRST(4,  1, 2, 3, 4, 5, 6, 7, 8, 9, 10) };
    int a5[]  = { __GMOCK_FIRST(5,  1, 2, 3, 4, 5, 6, 7, 8, 9, 10) };
    int a6[]  = { __GMOCK_FIRST(6,  1, 2, 3, 4, 5, 6, 7, 8, 9, 10) };
    int a7[]  = { __GMOCK_FIRST(7,  1, 2, 3, 4, 5, 6, 7, 8, 9, 10) };
    int a8[]  = { __GMOCK_FIRST(8,  1, 2, 3, 4, 5, 6, 7, 8, 9, 10) };
    int a9[]  = { __GMOCK_FIRST(9,  1, 2, 3, 4, 5, 6, 7, 8, 9, 10) };
    int a10[] = { __GMOCK_FIRST(10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10) };

    EXPECT_EQ(0 * sizeof(int),  sizeof(a0));
    EXPECT_EQ(1 * sizeof(int),  sizeof(a1));
    EXPECT_EQ(2 * sizeof(int),  sizeof(a2));
    EXPECT_EQ(3 * sizeof(int),  sizeof(a3));
    EXPECT_EQ(4 * sizeof(int),  sizeof(a4));
    EXPECT_EQ(5 * sizeof(int),  sizeof(a5));
    EXPECT_EQ(6 * sizeof(int),  sizeof(a6));
    EXPECT_EQ(7 * sizeof(int),  sizeof(a7));
    EXPECT_EQ(8 * sizeof(int),  sizeof(a8));
    EXPECT_EQ(9 * sizeof(int),  sizeof(a9));
    EXPECT_EQ(10 * sizeof(int), sizeof(a10));

    // See that it works even if the size is a macro
#define X 3
    int ax[] = { __GMOCK_FIRST(X, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10) };
	EXPECT_EQ(X * sizeof(int), sizeof(ax));
#undef X
}

}  // Unnamed namespace
