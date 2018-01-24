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
	EXPECT_EQ(0,  __GMOCK_NARGS());
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
    // Initiallize first N values, the rest are 0. We get null-terminated strings
    char s0[11]  = { __GMOCK_FIRST(0,  'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j') };
    char s1[11]  = { __GMOCK_FIRST(1,  'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j') };
    char s2[11]  = { __GMOCK_FIRST(2,  'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j') };
    char s3[11]  = { __GMOCK_FIRST(3,  'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j') };
    char s4[11]  = { __GMOCK_FIRST(4,  'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j') };
    char s5[11]  = { __GMOCK_FIRST(5,  'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j') };
    char s6[11]  = { __GMOCK_FIRST(6,  'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j') };
    char s7[11]  = { __GMOCK_FIRST(7,  'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j') };
    char s8[11]  = { __GMOCK_FIRST(8,  'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j') };
    char s9[11]  = { __GMOCK_FIRST(9,  'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j') };
    char s10[11] = { __GMOCK_FIRST(10, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j') };

    EXPECT_EQ(0u,  strlen(s0));
    EXPECT_EQ(1u,  strlen(s1));
    EXPECT_EQ(2u,  strlen(s2));
    EXPECT_EQ(3u,  strlen(s3));
    EXPECT_EQ(4u,  strlen(s4));
    EXPECT_EQ(5u,  strlen(s5));
    EXPECT_EQ(6u,  strlen(s6));
    EXPECT_EQ(7u,  strlen(s7));
    EXPECT_EQ(8u,  strlen(s8));
    EXPECT_EQ(9u,  strlen(s9));
    EXPECT_EQ(10u, strlen(s10));

    // See that it works even if the size is a macro
#define X 3
    char sx[11]  = { __GMOCK_FIRST(X,  'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j') };
    EXPECT_EQ(3u, strlen(sx));
#undef X
}

}  // Unnamed namespace
