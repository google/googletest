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
// This file implements some things that extend gtest but require gmock

// GOOGLETEST_CM0002 DO NOT DELETE

#ifndef GMOCK_INCLUDE_GMOCK_GMOCK_GTEST_FUSION_H_
#define GMOCK_INCLUDE_GMOCK_GMOCK_GTEST_FUSION_H_

#include "gmock/gmock-matchers.h"
#include "gtest/gtest.h"
#include <sstream>

#define GTEST_TEST_THROW_MATCH_(statement, expected_exception, matcher, fail) \
  GTEST_AMBIGUOUS_ELSE_BLOCKER_                                               \
  if (::testing::internal::AdditionalMessage gtest_msg = "") {                \
    bool gtest_caught_expected = false;                                       \
    try {                                                                     \
      GTEST_SUPPRESS_UNREACHABLE_CODE_WARNING_BELOW_(statement);              \
    } catch (expected_exception const& e) {                                   \
      const auto matcherImpl = SafeMatcherCast<expected_exception>(matcher);  \
      ::testing::StringMatchResultListener listener;                          \
      std::stringstream b;                                                    \
      matcherImpl.DescribeTo(&b);                                             \
      if (!matcherImpl.MatchAndExplain(e, &listener)) {                       \
        gtest_msg.set(                                                        \
            "Expected: " #statement " throws an exception of "                \
            "type " #expected_exception " which " +                           \
            b.str() + ".\n  Actual: it throws " #expected_exception);         \
        if (!listener.str().empty())                                          \
          gtest_msg.append(" " + listener.str());                             \
        gtest_msg.append(".");                                                \
        goto GTEST_CONCAT_TOKEN_(gtest_label_testthrow_, __LINE__);           \
      }                                                                       \
      gtest_caught_expected = true;                                           \
    } catch (...) {                                                           \
      gtest_msg.set("Expected: " #statement                                   \
                    " throws an exception of type " #expected_exception       \
                    ".\n  Actual: it throws a different type.");              \
      goto GTEST_CONCAT_TOKEN_(gtest_label_testthrow_, __LINE__);             \
    }                                                                         \
    if (!gtest_caught_expected) {                                             \
      gtest_msg.set("Expected: " #statement                                   \
                    " throws an exception of type " #expected_exception       \
                    ".\n  Actual: it throws nothing.");                       \
      goto GTEST_CONCAT_TOKEN_(gtest_label_testthrow_, __LINE__);             \
    }                                                                         \
  } else                                                                      \
    GTEST_CONCAT_TOKEN_(gtest_label_testthrow_, __LINE__)                     \
        : fail(gtest_msg.get().c_str())

#define EXPECT_THROW_MATCH(statement, expected_exception, matcher) \
  GTEST_TEST_THROW_MATCH_(statement, expected_exception, matcher,  \
                          GTEST_NONFATAL_FAILURE_)
#define ASSERT_THROW_MATCH(statement, expected_exception, matcher) \
  GTEST_TEST_THROW_MATCH_(statement, expected_exception, matcher,  \
                          GTEST_FATAL_FAILURE_)


#endif  // GMOCK_INCLUDE_GMOCK_GMOCK_GTEST_FUSION_H_
