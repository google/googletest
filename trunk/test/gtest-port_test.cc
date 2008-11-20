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
// Author: vladl@google.com (Vlad Losev)
//
// This file tests the internal cross-platform support utilities.

#include <gtest/internal/gtest-port.h>
#include <gtest/gtest.h>
#include <gtest/gtest-spi.h>

TEST(GtestCheckSyntaxTest, BehavesLikeASingleStatement) {
  if (false)
    GTEST_CHECK_(false) << "This should never be executed; "
                           "It's a compilation test only.";

  if (true)
    GTEST_CHECK_(true);
  else
    ;  // NOLINT

  if (false)
    ;  // NOLINT
  else
    GTEST_CHECK_(true) << "";
}

TEST(GtestCheckSyntaxTest, WorksWithSwitch) {
  switch (0) {
    case 1:
      break;
    default:
      GTEST_CHECK_(true);
  }

  switch(0)
    case 0:
      GTEST_CHECK_(true) << "Check failed in switch case";
}

#ifdef GTEST_HAS_DEATH_TEST

TEST(GtestCheckDeathTest, DiesWithCorrectOutputOnFailure) {
  const bool a_false_condition = false;
  EXPECT_DEATH(GTEST_CHECK_(a_false_condition) << "Extra info",
#ifdef _MSC_VER
               "gtest-port_test\\.cc\\([0-9]+\\):"
#else
               "gtest-port_test\\.cc:[0-9]+"
#endif  // _MSC_VER
                   ".*a_false_condition.*Extra info.*");
}

TEST(GtestCheckDeathTest, LivesSilentlyOnSuccess) {
  EXPECT_EXIT({
      GTEST_CHECK_(true) << "Extra info";
      ::std::cerr << "Success\n";
      exit(0); },
      ::testing::ExitedWithCode(0), "Success");
}

#endif  // GTEST_HAS_DEATH_TEST

#ifdef GTEST_USES_POSIX_RE

using ::testing::internal::RE;

template <typename Str>
class RETest : public ::testing::Test {};

// Defines StringTypes as the list of all string types that class RE
// supports.
typedef testing::Types<
#if GTEST_HAS_STD_STRING
    ::std::string,
#endif  // GTEST_HAS_STD_STRING
#if GTEST_HAS_GLOBAL_STRING
    ::string,
#endif  // GTEST_HAS_GLOBAL_STRING
    const char*> StringTypes;

TYPED_TEST_CASE(RETest, StringTypes);

// Tests RE's implicit constructors.
TYPED_TEST(RETest, ImplicitConstructorWorks) {
  const RE empty = TypeParam("");
  EXPECT_STREQ("", empty.pattern());

  const RE simple = TypeParam("hello");
  EXPECT_STREQ("hello", simple.pattern());

  const RE normal = TypeParam(".*(\\w+)");
  EXPECT_STREQ(".*(\\w+)", normal.pattern());
}

// Tests that RE's constructors reject invalid regular expressions.
TYPED_TEST(RETest, RejectsInvalidRegex) {
  EXPECT_NONFATAL_FAILURE({
    const RE invalid = TypeParam("?");
  }, "\"?\" is not a valid POSIX Extended regular expression.");
}

// Tests RE::FullMatch().
TYPED_TEST(RETest, FullMatchWorks) {
  const RE empty = TypeParam("");
  EXPECT_TRUE(RE::FullMatch(TypeParam(""), empty));
  EXPECT_FALSE(RE::FullMatch(TypeParam("a"), empty));

  const RE re = TypeParam("a.*z");
  EXPECT_TRUE(RE::FullMatch(TypeParam("az"), re));
  EXPECT_TRUE(RE::FullMatch(TypeParam("axyz"), re));
  EXPECT_FALSE(RE::FullMatch(TypeParam("baz"), re));
  EXPECT_FALSE(RE::FullMatch(TypeParam("azy"), re));
}

// Tests RE::PartialMatch().
TYPED_TEST(RETest, PartialMatchWorks) {
  const RE empty = TypeParam("");
  EXPECT_TRUE(RE::PartialMatch(TypeParam(""), empty));
  EXPECT_TRUE(RE::PartialMatch(TypeParam("a"), empty));

  const RE re = TypeParam("a.*z");
  EXPECT_TRUE(RE::PartialMatch(TypeParam("az"), re));
  EXPECT_TRUE(RE::PartialMatch(TypeParam("axyz"), re));
  EXPECT_TRUE(RE::PartialMatch(TypeParam("baz"), re));
  EXPECT_TRUE(RE::PartialMatch(TypeParam("azy"), re));
  EXPECT_FALSE(RE::PartialMatch(TypeParam("zza"), re));
}

#endif  // GTEST_USES_POSIX_RE
