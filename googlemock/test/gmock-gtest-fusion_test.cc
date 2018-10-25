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
// This file tests the built-in actions.

// Silence C4800 (C4800: 'int *const ': forcing value
// to bool 'true' or 'false') for MSVC 14,15
#ifdef _MSC_VER
#if _MSC_VER <= 1900
#pragma warning(push)
#pragma warning(disable : 4800)
#endif
#endif

#include "gmock/gmock-gtest-fusion.h"
#include <stdexcept>
#include "gtest/gtest.h"
#include "gtest/gtest-spi.h"

namespace {

class CustomException
{
public:
  int Member;
  CustomException(int member) : Member(member) {}
};
void ThrowAnInteger() { throw 42; }
void ThrowAChar() { throw 'h'; }
void ThrowNothing() {}
void ThrowException(int member) {
  throw CustomException(member);
}

using namespace ::testing;

// Tests ASSERT_THROW.
TEST(AssertionTest, ASSERT_THROW_MATCH) {
  ASSERT_THROW_MATCH(ThrowAnInteger(), int, Ge(0));

  EXPECT_FATAL_FAILURE(
      ASSERT_THROW_MATCH(ThrowAChar(), int, Ge(0)),
      "Expected: ThrowAChar() throws an exception of type int.\n"
      "  Actual: it throws a different type.");
  
  EXPECT_FATAL_FAILURE(
      ASSERT_THROW_MATCH(ThrowNothing(), int, Ge(0)),
      "Expected: ThrowNothing() throws an exception of type int.\n"
      "  Actual: it throws nothing.");

  EXPECT_FATAL_FAILURE(
      ASSERT_THROW_MATCH(ThrowAnInteger(), int, Lt(20)),
      "Expected: ThrowAnInteger() throws an exception"
      " of type int which is < 20.\n"
      "  Actual: it throws int.");

  EXPECT_FATAL_FAILURE(
      ASSERT_THROW_MATCH(ThrowException(1336), CustomException,
          Field("Member", &CustomException::Member, Eq(1337))),
      "Expected: ThrowException(1336) throws an exception of type "
      "CustomException which is an object whose "
      "field `Member` is equal to 1337.\n"
      "  Actual: it throws CustomException whose "
      "field `Member` is 1336 (of type int).");
}

}  // Unnamed namespace

#ifdef _MSC_VER
#if _MSC_VER == 1900
#pragma warning(pop)
#endif
#endif
