// Copyright 2005, Google Inc.
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
//
// Tests for Google Test itself.  This verifies that the basic constructs of
// Google Test work.

#include <gtest/gtest.h>
#include <gtest/gtest-spi.h>

// Indicates that this translation unit is part of Google Test's
// implementation.  It must come before gtest-internal-inl.h is
// included, or there will be a compiler error.  This trick is to
// prevent a user from accidentally including gtest-internal-inl.h in
// his code.
#define GTEST_IMPLEMENTATION
#include "src/gtest-internal-inl.h"
#undef GTEST_IMPLEMENTATION

#include <stdlib.h>

#if GTEST_HAS_PTHREAD
#include <pthread.h>
#endif  // GTEST_HAS_PTHREAD

#ifdef GTEST_OS_LINUX
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <vector>
#endif  // GTEST_OS_LINUX

namespace testing {
namespace internal {
const char* FormatTimeInMillisAsSeconds(TimeInMillis ms);
bool ParseInt32Flag(const char* str, const char* flag, Int32* value);
}  // namespace internal
}  // namespace testing

using testing::internal::FormatTimeInMillisAsSeconds;
using testing::internal::ParseInt32Flag;

namespace testing {

GTEST_DECLARE_string_(output);
GTEST_DECLARE_string_(color);

namespace internal {
bool ShouldUseColor(bool stdout_is_tty);
}  // namespace internal
}  // namespace testing

using testing::AssertionFailure;
using testing::AssertionResult;
using testing::AssertionSuccess;
using testing::DoubleLE;
using testing::FloatLE;
using testing::GTEST_FLAG(also_run_disabled_tests);
using testing::GTEST_FLAG(break_on_failure);
using testing::GTEST_FLAG(catch_exceptions);
using testing::GTEST_FLAG(death_test_use_fork);
using testing::GTEST_FLAG(color);
using testing::GTEST_FLAG(filter);
using testing::GTEST_FLAG(list_tests);
using testing::GTEST_FLAG(output);
using testing::GTEST_FLAG(print_time);
using testing::GTEST_FLAG(repeat);
using testing::GTEST_FLAG(show_internal_stack_frames);
using testing::GTEST_FLAG(stack_trace_depth);
using testing::IsNotSubstring;
using testing::IsSubstring;
using testing::Message;
using testing::ScopedFakeTestPartResultReporter;
using testing::StaticAssertTypeEq;
using testing::Test;
using testing::TestPartResult;
using testing::TestPartResultArray;
using testing::TPRT_FATAL_FAILURE;
using testing::TPRT_NONFATAL_FAILURE;
using testing::TPRT_SUCCESS;
using testing::UnitTest;
using testing::internal::kTestTypeIdInGoogleTest;
using testing::internal::AppendUserMessage;
using testing::internal::CodePointToUtf8;
using testing::internal::EqFailure;
using testing::internal::FloatingPoint;
using testing::internal::GetCurrentOsStackTraceExceptTop;
using testing::internal::GetFailedPartCount;
using testing::internal::GetTestTypeId;
using testing::internal::GetTypeId;
using testing::internal::GTestFlagSaver;
using testing::internal::Int32;
using testing::internal::List;
using testing::internal::ShouldUseColor;
using testing::internal::StreamableToString;
using testing::internal::String;
using testing::internal::TestProperty;
using testing::internal::TestResult;
using testing::internal::ThreadLocal;
using testing::internal::UnitTestImpl;
using testing::internal::WideStringToUtf8;

// This line tests that we can define tests in an unnamed namespace.
namespace {

// Tests GetTypeId.

TEST(GetTypeIdTest, ReturnsSameValueForSameType) {
  EXPECT_EQ(GetTypeId<int>(), GetTypeId<int>());
  EXPECT_EQ(GetTypeId<Test>(), GetTypeId<Test>());
}

class SubClassOfTest : public Test {};
class AnotherSubClassOfTest : public Test {};

TEST(GetTypeIdTest, ReturnsDifferentValuesForDifferentTypes) {
  EXPECT_NE(GetTypeId<int>(), GetTypeId<const int>());
  EXPECT_NE(GetTypeId<int>(), GetTypeId<char>());
  EXPECT_NE(GetTypeId<int>(), GetTestTypeId());
  EXPECT_NE(GetTypeId<SubClassOfTest>(), GetTestTypeId());
  EXPECT_NE(GetTypeId<AnotherSubClassOfTest>(), GetTestTypeId());
  EXPECT_NE(GetTypeId<AnotherSubClassOfTest>(), GetTypeId<SubClassOfTest>());
}

// Verifies that GetTestTypeId() returns the same value, no matter it
// is called from inside Google Test or outside of it.
TEST(GetTestTypeIdTest, ReturnsTheSameValueInsideOrOutsideOfGoogleTest) {
  EXPECT_EQ(kTestTypeIdInGoogleTest, GetTestTypeId());
}

// Tests FormatTimeInMillisAsSeconds().

TEST(FormatTimeInMillisAsSecondsTest, FormatsZero) {
  EXPECT_STREQ("0", FormatTimeInMillisAsSeconds(0));
}

TEST(FormatTimeInMillisAsSecondsTest, FormatsPositiveNumber) {
  EXPECT_STREQ("0.003", FormatTimeInMillisAsSeconds(3));
  EXPECT_STREQ("0.01", FormatTimeInMillisAsSeconds(10));
  EXPECT_STREQ("0.2", FormatTimeInMillisAsSeconds(200));
  EXPECT_STREQ("1.2", FormatTimeInMillisAsSeconds(1200));
  EXPECT_STREQ("3", FormatTimeInMillisAsSeconds(3000));
}

TEST(FormatTimeInMillisAsSecondsTest, FormatsNegativeNumber) {
  EXPECT_STREQ("-0.003", FormatTimeInMillisAsSeconds(-3));
  EXPECT_STREQ("-0.01", FormatTimeInMillisAsSeconds(-10));
  EXPECT_STREQ("-0.2", FormatTimeInMillisAsSeconds(-200));
  EXPECT_STREQ("-1.2", FormatTimeInMillisAsSeconds(-1200));
  EXPECT_STREQ("-3", FormatTimeInMillisAsSeconds(-3000));
}

#ifndef GTEST_OS_SYMBIAN
// NULL testing does not work with Symbian compilers.

// Tests that GTEST_IS_NULL_LITERAL_(x) is true when x is a null
// pointer literal.
TEST(NullLiteralTest, IsTrueForNullLiterals) {
  EXPECT_TRUE(GTEST_IS_NULL_LITERAL_(NULL));
  EXPECT_TRUE(GTEST_IS_NULL_LITERAL_(0));
  EXPECT_TRUE(GTEST_IS_NULL_LITERAL_(1 - 1));
  EXPECT_TRUE(GTEST_IS_NULL_LITERAL_(0U));
  EXPECT_TRUE(GTEST_IS_NULL_LITERAL_(0L));
  EXPECT_TRUE(GTEST_IS_NULL_LITERAL_(false));
  EXPECT_TRUE(GTEST_IS_NULL_LITERAL_(true && false));
}

// Tests that GTEST_IS_NULL_LITERAL_(x) is false when x is not a null
// pointer literal.
TEST(NullLiteralTest, IsFalseForNonNullLiterals) {
  EXPECT_FALSE(GTEST_IS_NULL_LITERAL_(1));
  EXPECT_FALSE(GTEST_IS_NULL_LITERAL_(0.0));
  EXPECT_FALSE(GTEST_IS_NULL_LITERAL_('a'));
  EXPECT_FALSE(GTEST_IS_NULL_LITERAL_(static_cast<void*>(NULL)));
}

#endif  // GTEST_OS_SYMBIAN
//
// Tests CodePointToUtf8().

// Tests that the NUL character L'\0' is encoded correctly.
TEST(CodePointToUtf8Test, CanEncodeNul) {
  char buffer[32];
  EXPECT_STREQ("", CodePointToUtf8(L'\0', buffer));
}

// Tests that ASCII characters are encoded correctly.
TEST(CodePointToUtf8Test, CanEncodeAscii) {
  char buffer[32];
  EXPECT_STREQ("a", CodePointToUtf8(L'a', buffer));
  EXPECT_STREQ("Z", CodePointToUtf8(L'Z', buffer));
  EXPECT_STREQ("&", CodePointToUtf8(L'&', buffer));
  EXPECT_STREQ("\x7F", CodePointToUtf8(L'\x7F', buffer));
}

// Tests that Unicode code-points that have 8 to 11 bits are encoded
// as 110xxxxx 10xxxxxx.
TEST(CodePointToUtf8Test, CanEncode8To11Bits) {
  char buffer[32];
  // 000 1101 0011 => 110-00011 10-010011
  EXPECT_STREQ("\xC3\x93", CodePointToUtf8(L'\xD3', buffer));

  // 101 0111 0110 => 110-10101 10-110110
  EXPECT_STREQ("\xD5\xB6", CodePointToUtf8(L'\x576', buffer));
}

// Tests that Unicode code-points that have 12 to 16 bits are encoded
// as 1110xxxx 10xxxxxx 10xxxxxx.
TEST(CodePointToUtf8Test, CanEncode12To16Bits) {
  char buffer[32];
  // 0000 1000 1101 0011 => 1110-0000 10-100011 10-010011
  EXPECT_STREQ("\xE0\xA3\x93", CodePointToUtf8(L'\x8D3', buffer));

  // 1100 0111 0100 1101 => 1110-1100 10-011101 10-001101
  EXPECT_STREQ("\xEC\x9D\x8D", CodePointToUtf8(L'\xC74D', buffer));
}

#ifndef GTEST_WIDE_STRING_USES_UTF16_
// Tests in this group require a wchar_t to hold > 16 bits, and thus
// are skipped on Windows, Cygwin, and Symbian, where a wchar_t is
// 16-bit wide. This code may not compile on those systems.

// Tests that Unicode code-points that have 17 to 21 bits are encoded
// as 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx.
TEST(CodePointToUtf8Test, CanEncode17To21Bits) {
  char buffer[32];
  // 0 0001 0000 1000 1101 0011 => 11110-000 10-010000 10-100011 10-010011
  EXPECT_STREQ("\xF0\x90\xA3\x93", CodePointToUtf8(L'\x108D3', buffer));

  // 0 0001 0000 0100 0000 0000 => 11110-000 10-010000 10-010000 10-000000
  EXPECT_STREQ("\xF0\x90\x90\x80", CodePointToUtf8(L'\x10400', buffer));

  // 1 0000 1000 0110 0011 0100 => 11110-100 10-001000 10-011000 10-110100
  EXPECT_STREQ("\xF4\x88\x98\xB4", CodePointToUtf8(L'\x108634', buffer));
}

// Tests that encoding an invalid code-point generates the expected result.
TEST(CodePointToUtf8Test, CanEncodeInvalidCodePoint) {
  char buffer[32];
  EXPECT_STREQ("(Invalid Unicode 0x1234ABCD)",
               CodePointToUtf8(L'\x1234ABCD', buffer));
}

#endif  // GTEST_WIDE_STRING_USES_UTF16_

// Tests WideStringToUtf8().

// Tests that the NUL character L'\0' is encoded correctly.
TEST(WideStringToUtf8Test, CanEncodeNul) {
  EXPECT_STREQ("", WideStringToUtf8(L"", 0).c_str());
  EXPECT_STREQ("", WideStringToUtf8(L"", -1).c_str());
}

// Tests that ASCII strings are encoded correctly.
TEST(WideStringToUtf8Test, CanEncodeAscii) {
  EXPECT_STREQ("a", WideStringToUtf8(L"a", 1).c_str());
  EXPECT_STREQ("ab", WideStringToUtf8(L"ab", 2).c_str());
  EXPECT_STREQ("a", WideStringToUtf8(L"a", -1).c_str());
  EXPECT_STREQ("ab", WideStringToUtf8(L"ab", -1).c_str());
}

// Tests that Unicode code-points that have 8 to 11 bits are encoded
// as 110xxxxx 10xxxxxx.
TEST(WideStringToUtf8Test, CanEncode8To11Bits) {
  // 000 1101 0011 => 110-00011 10-010011
  EXPECT_STREQ("\xC3\x93", WideStringToUtf8(L"\xD3", 1).c_str());
  EXPECT_STREQ("\xC3\x93", WideStringToUtf8(L"\xD3", -1).c_str());

  // 101 0111 0110 => 110-10101 10-110110
  EXPECT_STREQ("\xD5\xB6", WideStringToUtf8(L"\x576", 1).c_str());
  EXPECT_STREQ("\xD5\xB6", WideStringToUtf8(L"\x576", -1).c_str());
}

// Tests that Unicode code-points that have 12 to 16 bits are encoded
// as 1110xxxx 10xxxxxx 10xxxxxx.
TEST(WideStringToUtf8Test, CanEncode12To16Bits) {
  // 0000 1000 1101 0011 => 1110-0000 10-100011 10-010011
  EXPECT_STREQ("\xE0\xA3\x93", WideStringToUtf8(L"\x8D3", 1).c_str());
  EXPECT_STREQ("\xE0\xA3\x93", WideStringToUtf8(L"\x8D3", -1).c_str());

  // 1100 0111 0100 1101 => 1110-1100 10-011101 10-001101
  EXPECT_STREQ("\xEC\x9D\x8D", WideStringToUtf8(L"\xC74D", 1).c_str());
  EXPECT_STREQ("\xEC\x9D\x8D", WideStringToUtf8(L"\xC74D", -1).c_str());
}

// Tests that the conversion stops when the function encounters \0 character.
TEST(WideStringToUtf8Test, StopsOnNulCharacter) {
  EXPECT_STREQ("ABC", WideStringToUtf8(L"ABC\0XYZ", 100).c_str());
}

// Tests that the conversion stops when the function reaches the limit
// specified by the 'length' parameter.
TEST(WideStringToUtf8Test, StopsWhenLengthLimitReached) {
  EXPECT_STREQ("ABC", WideStringToUtf8(L"ABCDEF", 3).c_str());
}


#ifndef GTEST_WIDE_STRING_USES_UTF16_
// Tests that Unicode code-points that have 17 to 21 bits are encoded
// as 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx. This code may not compile
// on the systems using UTF-16 encoding.
TEST(WideStringToUtf8Test, CanEncode17To21Bits) {
  // 0 0001 0000 1000 1101 0011 => 11110-000 10-010000 10-100011 10-010011
  EXPECT_STREQ("\xF0\x90\xA3\x93", WideStringToUtf8(L"\x108D3", 1).c_str());
  EXPECT_STREQ("\xF0\x90\xA3\x93", WideStringToUtf8(L"\x108D3", -1).c_str());

  // 1 0000 1000 0110 0011 0100 => 11110-100 10-001000 10-011000 10-110100
  EXPECT_STREQ("\xF4\x88\x98\xB4", WideStringToUtf8(L"\x108634", 1).c_str());
  EXPECT_STREQ("\xF4\x88\x98\xB4", WideStringToUtf8(L"\x108634", -1).c_str());
}

// Tests that encoding an invalid code-point generates the expected result.
TEST(WideStringToUtf8Test, CanEncodeInvalidCodePoint) {
  EXPECT_STREQ("(Invalid Unicode 0xABCDFF)",
               WideStringToUtf8(L"\xABCDFF", -1).c_str());
}
#else
// Tests that surrogate pairs are encoded correctly on the systems using
// UTF-16 encoding in the wide strings.
TEST(WideStringToUtf8Test, CanEncodeValidUtf16SUrrogatePairs) {
  EXPECT_STREQ("\xF0\x90\x90\x80",
               WideStringToUtf8(L"\xD801\xDC00", -1).c_str());
}

// Tests that encoding an invalid UTF-16 surrogate pair
// generates the expected result.
TEST(WideStringToUtf8Test, CanEncodeInvalidUtf16SurrogatePair) {
  // Leading surrogate is at the end of the string.
  EXPECT_STREQ("\xED\xA0\x80", WideStringToUtf8(L"\xD800", -1).c_str());
  // Leading surrogate is not followed by the trailing surrogate.
  EXPECT_STREQ("\xED\xA0\x80$", WideStringToUtf8(L"\xD800$", -1).c_str());
  // Trailing surrogate appearas without a leading surrogate.
  EXPECT_STREQ("\xED\xB0\x80PQR", WideStringToUtf8(L"\xDC00PQR", -1).c_str());
}
#endif  // GTEST_WIDE_STRING_USES_UTF16_

// Tests that codepoint concatenation works correctly.
#ifndef GTEST_WIDE_STRING_USES_UTF16_
TEST(WideStringToUtf8Test, ConcatenatesCodepointsCorrectly) {
  EXPECT_STREQ(
      "\xF4\x88\x98\xB4"
          "\xEC\x9D\x8D"
          "\n"
          "\xD5\xB6"
          "\xE0\xA3\x93"
          "\xF4\x88\x98\xB4",
      WideStringToUtf8(L"\x108634\xC74D\n\x576\x8D3\x108634", -1).c_str());
}
#else
TEST(WideStringToUtf8Test, ConcatenatesCodepointsCorrectly) {
  EXPECT_STREQ(
      "\xEC\x9D\x8D" "\n" "\xD5\xB6" "\xE0\xA3\x93",
      WideStringToUtf8(L"\xC74D\n\x576\x8D3", -1).c_str());
}
#endif  // GTEST_WIDE_STRING_USES_UTF16_

// Tests the List template class.

// Tests List::PushFront().
TEST(ListTest, PushFront) {
  List<int> a;
  ASSERT_EQ(0u, a.size());

  // Calls PushFront() on an empty list.
  a.PushFront(1);
  ASSERT_EQ(1u, a.size());
  EXPECT_EQ(1, a.Head()->element());
  ASSERT_EQ(a.Head(), a.Last());

  // Calls PushFront() on a singleton list.
  a.PushFront(2);
  ASSERT_EQ(2u, a.size());
  EXPECT_EQ(2, a.Head()->element());
  EXPECT_EQ(1, a.Last()->element());

  // Calls PushFront() on a list with more than one elements.
  a.PushFront(3);
  ASSERT_EQ(3u, a.size());
  EXPECT_EQ(3, a.Head()->element());
  EXPECT_EQ(2, a.Head()->next()->element());
  EXPECT_EQ(1, a.Last()->element());
}

// Tests List::PopFront().
TEST(ListTest, PopFront) {
  List<int> a;

  // Popping on an empty list should fail.
  EXPECT_FALSE(a.PopFront(NULL));

  // Popping again on an empty list should fail, and the result element
  // shouldn't be overwritten.
  int element = 1;
  EXPECT_FALSE(a.PopFront(&element));
  EXPECT_EQ(1, element);

  a.PushFront(2);
  a.PushFront(3);

  // PopFront() should pop the element in the front of the list.
  EXPECT_TRUE(a.PopFront(&element));
  EXPECT_EQ(3, element);

  // After popping the last element, the list should be empty.
  EXPECT_TRUE(a.PopFront(NULL));
  EXPECT_EQ(0u, a.size());
}

// Tests inserting at the beginning using List::InsertAfter().
TEST(ListTest, InsertAfterAtBeginning) {
  List<int> a;
  ASSERT_EQ(0u, a.size());

  // Inserts into an empty list.
  a.InsertAfter(NULL, 1);
  ASSERT_EQ(1u, a.size());
  EXPECT_EQ(1, a.Head()->element());
  ASSERT_EQ(a.Head(), a.Last());

  // Inserts at the beginning of a singleton list.
  a.InsertAfter(NULL, 2);
  ASSERT_EQ(2u, a.size());
  EXPECT_EQ(2, a.Head()->element());
  EXPECT_EQ(1, a.Last()->element());

  // Inserts at the beginning of a list with more than one elements.
  a.InsertAfter(NULL, 3);
  ASSERT_EQ(3u, a.size());
  EXPECT_EQ(3, a.Head()->element());
  EXPECT_EQ(2, a.Head()->next()->element());
  EXPECT_EQ(1, a.Last()->element());
}

// Tests inserting at a location other than the beginning using
// List::InsertAfter().
TEST(ListTest, InsertAfterNotAtBeginning) {
  // Prepares a singleton list.
  List<int> a;
  a.PushBack(1);

  // Inserts at the end of a singleton list.
  a.InsertAfter(a.Last(), 2);
  ASSERT_EQ(2u, a.size());
  EXPECT_EQ(1, a.Head()->element());
  EXPECT_EQ(2, a.Last()->element());

  // Inserts at the end of a list with more than one elements.
  a.InsertAfter(a.Last(), 3);
  ASSERT_EQ(3u, a.size());
  EXPECT_EQ(1, a.Head()->element());
  EXPECT_EQ(2, a.Head()->next()->element());
  EXPECT_EQ(3, a.Last()->element());

  // Inserts in the middle of a list.
  a.InsertAfter(a.Head(), 4);
  ASSERT_EQ(4u, a.size());
  EXPECT_EQ(1, a.Head()->element());
  EXPECT_EQ(4, a.Head()->next()->element());
  EXPECT_EQ(2, a.Head()->next()->next()->element());
  EXPECT_EQ(3, a.Last()->element());
}


// Tests the String class.

// Tests String's constructors.
TEST(StringTest, Constructors) {
  // Default ctor.
  String s1;
  // We aren't using EXPECT_EQ(NULL, s1.c_str()) because comparing
  // pointers with NULL isn't supported on all platforms.
  EXPECT_TRUE(NULL == s1.c_str());

  // Implicitly constructs from a C-string.
  String s2 = "Hi";
  EXPECT_STREQ("Hi", s2.c_str());

  // Constructs from a C-string and a length.
  String s3("hello", 3);
  EXPECT_STREQ("hel", s3.c_str());

  // Copy ctor.
  String s4 = s3;
  EXPECT_STREQ("hel", s4.c_str());
}

#if GTEST_HAS_STD_STRING

TEST(StringTest, ConvertsFromStdString) {
  // An empty std::string.
  const std::string src1("");
  const String dest1 = src1;
  EXPECT_STREQ("", dest1.c_str());

  // A normal std::string.
  const std::string src2("Hi");
  const String dest2 = src2;
  EXPECT_STREQ("Hi", dest2.c_str());

  // An std::string with an embedded NUL character.
  const char src3[] = "Hello\0world.";
  const String dest3 = std::string(src3, sizeof(src3));
  EXPECT_STREQ("Hello", dest3.c_str());
}

TEST(StringTest, ConvertsToStdString) {
  // An empty String.
  const String src1("");
  const std::string dest1 = src1;
  EXPECT_EQ("", dest1);

  // A normal String.
  const String src2("Hi");
  const std::string dest2 = src2;
  EXPECT_EQ("Hi", dest2);
}

#endif  // GTEST_HAS_STD_STRING

#if GTEST_HAS_GLOBAL_STRING

TEST(StringTest, ConvertsFromGlobalString) {
  // An empty ::string.
  const ::string src1("");
  const String dest1 = src1;
  EXPECT_STREQ("", dest1.c_str());

  // A normal ::string.
  const ::string src2("Hi");
  const String dest2 = src2;
  EXPECT_STREQ("Hi", dest2.c_str());

  // An ::string with an embedded NUL character.
  const char src3[] = "Hello\0world.";
  const String dest3 = ::string(src3, sizeof(src3));
  EXPECT_STREQ("Hello", dest3.c_str());
}

TEST(StringTest, ConvertsToGlobalString) {
  // An empty String.
  const String src1("");
  const ::string dest1 = src1;
  EXPECT_EQ("", dest1);

  // A normal String.
  const String src2("Hi");
  const ::string dest2 = src2;
  EXPECT_EQ("Hi", dest2);
}

#endif  // GTEST_HAS_GLOBAL_STRING

// Tests String::ShowCString().
TEST(StringTest, ShowCString) {
  EXPECT_STREQ("(null)", String::ShowCString(NULL));
  EXPECT_STREQ("", String::ShowCString(""));
  EXPECT_STREQ("foo", String::ShowCString("foo"));
}

// Tests String::ShowCStringQuoted().
TEST(StringTest, ShowCStringQuoted) {
  EXPECT_STREQ("(null)",
               String::ShowCStringQuoted(NULL).c_str());
  EXPECT_STREQ("\"\"",
               String::ShowCStringQuoted("").c_str());
  EXPECT_STREQ("\"foo\"",
               String::ShowCStringQuoted("foo").c_str());
}

// Tests String::operator==().
TEST(StringTest, Equals) {
  const String null(NULL);
  EXPECT_TRUE(null == NULL);  // NOLINT
  EXPECT_FALSE(null == "");  // NOLINT
  EXPECT_FALSE(null == "bar");  // NOLINT

  const String empty("");
  EXPECT_FALSE(empty == NULL);  // NOLINT
  EXPECT_TRUE(empty == "");  // NOLINT
  EXPECT_FALSE(empty == "bar");  // NOLINT

  const String foo("foo");
  EXPECT_FALSE(foo == NULL);  // NOLINT
  EXPECT_FALSE(foo == "");  // NOLINT
  EXPECT_FALSE(foo == "bar");  // NOLINT
  EXPECT_TRUE(foo == "foo");  // NOLINT
}

// Tests String::operator!=().
TEST(StringTest, NotEquals) {
  const String null(NULL);
  EXPECT_FALSE(null != NULL);  // NOLINT
  EXPECT_TRUE(null != "");  // NOLINT
  EXPECT_TRUE(null != "bar");  // NOLINT

  const String empty("");
  EXPECT_TRUE(empty != NULL);  // NOLINT
  EXPECT_FALSE(empty != "");  // NOLINT
  EXPECT_TRUE(empty != "bar");  // NOLINT

  const String foo("foo");
  EXPECT_TRUE(foo != NULL);  // NOLINT
  EXPECT_TRUE(foo != "");  // NOLINT
  EXPECT_TRUE(foo != "bar");  // NOLINT
  EXPECT_FALSE(foo != "foo");  // NOLINT
}

// Tests String::EndsWith().
TEST(StringTest, EndsWith) {
  EXPECT_TRUE(String("foobar").EndsWith("bar"));
  EXPECT_TRUE(String("foobar").EndsWith(""));
  EXPECT_TRUE(String("").EndsWith(""));

  EXPECT_FALSE(String("foobar").EndsWith("foo"));
  EXPECT_FALSE(String("").EndsWith("foo"));
}

// Tests String::EndsWithCaseInsensitive().
TEST(StringTest, EndsWithCaseInsensitive) {
  EXPECT_TRUE(String("foobar").EndsWithCaseInsensitive("BAR"));
  EXPECT_TRUE(String("foobaR").EndsWithCaseInsensitive("bar"));
  EXPECT_TRUE(String("foobar").EndsWithCaseInsensitive(""));
  EXPECT_TRUE(String("").EndsWithCaseInsensitive(""));

  EXPECT_FALSE(String("Foobar").EndsWithCaseInsensitive("foo"));
  EXPECT_FALSE(String("foobar").EndsWithCaseInsensitive("Foo"));
  EXPECT_FALSE(String("").EndsWithCaseInsensitive("foo"));
}

// Tests String::CaseInsensitiveWideCStringEquals
TEST(StringTest, CaseInsensitiveWideCStringEquals) {
  EXPECT_TRUE(String::CaseInsensitiveWideCStringEquals(NULL, NULL));
  EXPECT_FALSE(String::CaseInsensitiveWideCStringEquals(NULL, L""));
  EXPECT_FALSE(String::CaseInsensitiveWideCStringEquals(L"", NULL));
  EXPECT_FALSE(String::CaseInsensitiveWideCStringEquals(NULL, L"foobar"));
  EXPECT_FALSE(String::CaseInsensitiveWideCStringEquals(L"foobar", NULL));
  EXPECT_TRUE(String::CaseInsensitiveWideCStringEquals(L"foobar", L"foobar"));
  EXPECT_TRUE(String::CaseInsensitiveWideCStringEquals(L"foobar", L"FOOBAR"));
  EXPECT_TRUE(String::CaseInsensitiveWideCStringEquals(L"FOOBAR", L"foobar"));
}

// Tests that NULL can be assigned to a String.
TEST(StringTest, CanBeAssignedNULL) {
  const String src(NULL);
  String dest;

  dest = src;
  EXPECT_STREQ(NULL, dest.c_str());
}

// Tests that the empty string "" can be assigned to a String.
TEST(StringTest, CanBeAssignedEmpty) {
  const String src("");
  String dest;

  dest = src;
  EXPECT_STREQ("", dest.c_str());
}

// Tests that a non-empty string can be assigned to a String.
TEST(StringTest, CanBeAssignedNonEmpty) {
  const String src("hello");
  String dest;

  dest = src;
  EXPECT_STREQ("hello", dest.c_str());
}

// Tests that a String can be assigned to itself.
TEST(StringTest, CanBeAssignedSelf) {
  String dest("hello");

  dest = dest;
  EXPECT_STREQ("hello", dest.c_str());
}

#ifdef GTEST_OS_WINDOWS

// Tests String::ShowWideCString().
TEST(StringTest, ShowWideCString) {
  EXPECT_STREQ("(null)",
               String::ShowWideCString(NULL).c_str());
  EXPECT_STREQ("", String::ShowWideCString(L"").c_str());
  EXPECT_STREQ("foo", String::ShowWideCString(L"foo").c_str());
}

// Tests String::ShowWideCStringQuoted().
TEST(StringTest, ShowWideCStringQuoted) {
  EXPECT_STREQ("(null)",
               String::ShowWideCStringQuoted(NULL).c_str());
  EXPECT_STREQ("L\"\"",
               String::ShowWideCStringQuoted(L"").c_str());
  EXPECT_STREQ("L\"foo\"",
               String::ShowWideCStringQuoted(L"foo").c_str());
}

#ifdef _WIN32_WCE
TEST(StringTest, AnsiAndUtf16Null) {
  EXPECT_EQ(NULL, String::AnsiToUtf16(NULL));
  EXPECT_EQ(NULL, String::Utf16ToAnsi(NULL));
}

TEST(StringTest, AnsiAndUtf16ConvertBasic) {
  const char* ansi = String::Utf16ToAnsi(L"str");
  EXPECT_STREQ("str", ansi);
  delete [] ansi;
  const WCHAR* utf16 = String::AnsiToUtf16("str");
  EXPECT_TRUE(wcsncmp(L"str", utf16, 3) == 0);
  delete [] utf16;
}

TEST(StringTest, AnsiAndUtf16ConvertPathChars) {
  const char* ansi = String::Utf16ToAnsi(L".:\\ \"*?");
  EXPECT_STREQ(".:\\ \"*?", ansi);
  delete [] ansi;
  const WCHAR* utf16 = String::AnsiToUtf16(".:\\ \"*?");
  EXPECT_TRUE(wcsncmp(L".:\\ \"*?", utf16, 3) == 0);
  delete [] utf16;
}
#endif  // _WIN32_WCE

#endif  // GTEST_OS_WINDOWS

// Tests TestProperty construction.
TEST(TestPropertyTest, StringValue) {
  TestProperty property("key", "1");
  EXPECT_STREQ("key", property.key());
  EXPECT_STREQ("1", property.value());
}

// Tests TestProperty replacing a value.
TEST(TestPropertyTest, ReplaceStringValue) {
  TestProperty property("key", "1");
  EXPECT_STREQ("1", property.value());
  property.SetValue("2");
  EXPECT_STREQ("2", property.value());
}

class ScopedFakeTestPartResultReporterTest : public Test {
 protected:
  enum FailureMode {
    FATAL_FAILURE,
    NONFATAL_FAILURE
  };
  static void AddFailure(FailureMode failure) {
    if (failure == FATAL_FAILURE) {
      FAIL() << "Expected fatal failure.";
    } else {
      ADD_FAILURE() << "Expected non-fatal failure.";
    }
  }
};

// Tests that ScopedFakeTestPartResultReporter intercepts test
// failures.
TEST_F(ScopedFakeTestPartResultReporterTest, InterceptsTestFailures) {
  TestPartResultArray results;
  {
    ScopedFakeTestPartResultReporter reporter(
        ScopedFakeTestPartResultReporter::INTERCEPT_ONLY_CURRENT_THREAD,
        &results);
    AddFailure(NONFATAL_FAILURE);
    AddFailure(FATAL_FAILURE);
  }

  EXPECT_EQ(2, results.size());
  EXPECT_TRUE(results.GetTestPartResult(0).nonfatally_failed());
  EXPECT_TRUE(results.GetTestPartResult(1).fatally_failed());
}

TEST_F(ScopedFakeTestPartResultReporterTest, DeprecatedConstructor) {
  TestPartResultArray results;
  {
    // Tests, that the deprecated constructor still works.
    ScopedFakeTestPartResultReporter reporter(&results);
    AddFailure(NONFATAL_FAILURE);
  }
  EXPECT_EQ(1, results.size());
}

#if GTEST_IS_THREADSAFE && GTEST_HAS_PTHREAD

class ScopedFakeTestPartResultReporterWithThreadsTest
  : public ScopedFakeTestPartResultReporterTest {
 protected:
  static void AddFailureInOtherThread(FailureMode failure) {
    pthread_t tid;
    pthread_create(&tid,
                   NULL,
                   ScopedFakeTestPartResultReporterWithThreadsTest::
                       FailureThread,
                   &failure);
    pthread_join(tid, NULL);
  }
 private:
  static void* FailureThread(void* attr) {
    FailureMode* failure = static_cast<FailureMode*>(attr);
    AddFailure(*failure);
    return NULL;
  }
};

TEST_F(ScopedFakeTestPartResultReporterWithThreadsTest,
       InterceptsTestFailuresInAllThreads) {
  TestPartResultArray results;
  {
    ScopedFakeTestPartResultReporter reporter(
        ScopedFakeTestPartResultReporter::INTERCEPT_ALL_THREADS, &results);
    AddFailure(NONFATAL_FAILURE);
    AddFailure(FATAL_FAILURE);
    AddFailureInOtherThread(NONFATAL_FAILURE);
    AddFailureInOtherThread(FATAL_FAILURE);
  }

  EXPECT_EQ(4, results.size());
  EXPECT_TRUE(results.GetTestPartResult(0).nonfatally_failed());
  EXPECT_TRUE(results.GetTestPartResult(1).fatally_failed());
  EXPECT_TRUE(results.GetTestPartResult(2).nonfatally_failed());
  EXPECT_TRUE(results.GetTestPartResult(3).fatally_failed());
}

#endif  // GTEST_IS_THREADSAFE && GTEST_HAS_PTHREAD

// Tests EXPECT_{,NON}FATAL_FAILURE{,ON_ALL_THREADS}.

typedef ScopedFakeTestPartResultReporterTest ExpectFailureTest;

TEST_F(ExpectFailureTest, ExpectFatalFaliure) {
  EXPECT_FATAL_FAILURE(AddFailure(FATAL_FAILURE), "Expected fatal failure.");
}

TEST_F(ExpectFailureTest, ExpectNonFatalFailure) {
  EXPECT_NONFATAL_FAILURE(AddFailure(NONFATAL_FAILURE),
                          "Expected non-fatal failure.");
}

TEST_F(ExpectFailureTest, ExpectFatalFailureOnAllThreads) {
  EXPECT_FATAL_FAILURE_ON_ALL_THREADS(AddFailure(FATAL_FAILURE),
                                      "Expected fatal failure.");
}

TEST_F(ExpectFailureTest, ExpectNonFatalFailureOnAllThreads) {
  EXPECT_NONFATAL_FAILURE_ON_ALL_THREADS(AddFailure(NONFATAL_FAILURE),
                                         "Expected non-fatal failure.");
}

// Tests that the EXPECT_{,NON}FATAL_FAILURE{,_ON_ALL_THREADS} accepts
// a statement that contains a macro which expands to code containing
// an unprotected comma.

static int global_var = 0;
#define GTEST_USE_UNPROTECTED_COMMA_ global_var++, global_var++

TEST_F(ExpectFailureTest, AcceptsMacroThatExpandsToUnprotectedComma) {
  EXPECT_FATAL_FAILURE({
    GTEST_USE_UNPROTECTED_COMMA_;
    AddFailure(FATAL_FAILURE);
  }, "");

  EXPECT_FATAL_FAILURE_ON_ALL_THREADS({
    GTEST_USE_UNPROTECTED_COMMA_;
    AddFailure(FATAL_FAILURE);
  }, "");

  EXPECT_NONFATAL_FAILURE({
    GTEST_USE_UNPROTECTED_COMMA_;
    AddFailure(NONFATAL_FAILURE);
  }, "");

  EXPECT_NONFATAL_FAILURE_ON_ALL_THREADS({
    GTEST_USE_UNPROTECTED_COMMA_;
    AddFailure(NONFATAL_FAILURE);
  }, "");
}

#if GTEST_IS_THREADSAFE && GTEST_HAS_PTHREAD

typedef ScopedFakeTestPartResultReporterWithThreadsTest
    ExpectFailureWithThreadsTest;

TEST_F(ExpectFailureWithThreadsTest, ExpectFatalFailureOnAllThreads) {
  EXPECT_FATAL_FAILURE_ON_ALL_THREADS(AddFailureInOtherThread(FATAL_FAILURE),
                                      "Expected fatal failure.");
}

TEST_F(ExpectFailureWithThreadsTest, ExpectNonFatalFailureOnAllThreads) {
  EXPECT_NONFATAL_FAILURE_ON_ALL_THREADS(
      AddFailureInOtherThread(NONFATAL_FAILURE), "Expected non-fatal failure.");
}

#endif  // GTEST_IS_THREADSAFE && GTEST_HAS_PTHREAD

// Tests the TestResult class

// The test fixture for testing TestResult.
class TestResultTest : public Test {
 protected:
  typedef List<TestPartResult> TPRList;

  // We make use of 2 TestPartResult objects,
  TestPartResult * pr1, * pr2;

  // ... and 3 TestResult objects.
  TestResult * r0, * r1, * r2;

  virtual void SetUp() {
    // pr1 is for success.
    pr1 = new TestPartResult(TPRT_SUCCESS, "foo/bar.cc", 10, "Success!");

    // pr2 is for fatal failure.
    pr2 = new TestPartResult(TPRT_FATAL_FAILURE, "foo/bar.cc",
                             -1,  // This line number means "unknown"
                             "Failure!");

    // Creates the TestResult objects.
    r0 = new TestResult();
    r1 = new TestResult();
    r2 = new TestResult();

    // In order to test TestResult, we need to modify its internal
    // state, in particular the TestPartResult list it holds.
    // test_part_results() returns a const reference to this list.
    // We cast it to a non-const object s.t. it can be modified (yes,
    // this is a hack).
    TPRList * list1, * list2;
    list1 = const_cast<List<TestPartResult> *>(
        & r1->test_part_results());
    list2 = const_cast<List<TestPartResult> *>(
        & r2->test_part_results());

    // r0 is an empty TestResult.

    // r1 contains a single SUCCESS TestPartResult.
    list1->PushBack(*pr1);

    // r2 contains a SUCCESS, and a FAILURE.
    list2->PushBack(*pr1);
    list2->PushBack(*pr2);
  }

  virtual void TearDown() {
    delete pr1;
    delete pr2;

    delete r0;
    delete r1;
    delete r2;
  }
};

// Tests TestResult::test_part_results()
TEST_F(TestResultTest, test_part_results) {
  ASSERT_EQ(0u, r0->test_part_results().size());
  ASSERT_EQ(1u, r1->test_part_results().size());
  ASSERT_EQ(2u, r2->test_part_results().size());
}

// Tests TestResult::successful_part_count()
TEST_F(TestResultTest, successful_part_count) {
  ASSERT_EQ(0u, r0->successful_part_count());
  ASSERT_EQ(1u, r1->successful_part_count());
  ASSERT_EQ(1u, r2->successful_part_count());
}

// Tests TestResult::failed_part_count()
TEST_F(TestResultTest, failed_part_count) {
  ASSERT_EQ(0u, r0->failed_part_count());
  ASSERT_EQ(0u, r1->failed_part_count());
  ASSERT_EQ(1u, r2->failed_part_count());
}

// Tests testing::internal::GetFailedPartCount().
TEST_F(TestResultTest, GetFailedPartCount) {
  ASSERT_EQ(0u, GetFailedPartCount(r0));
  ASSERT_EQ(0u, GetFailedPartCount(r1));
  ASSERT_EQ(1u, GetFailedPartCount(r2));
}

// Tests TestResult::total_part_count()
TEST_F(TestResultTest, total_part_count) {
  ASSERT_EQ(0u, r0->total_part_count());
  ASSERT_EQ(1u, r1->total_part_count());
  ASSERT_EQ(2u, r2->total_part_count());
}

// Tests TestResult::Passed()
TEST_F(TestResultTest, Passed) {
  ASSERT_TRUE(r0->Passed());
  ASSERT_TRUE(r1->Passed());
  ASSERT_FALSE(r2->Passed());
}

// Tests TestResult::Failed()
TEST_F(TestResultTest, Failed) {
  ASSERT_FALSE(r0->Failed());
  ASSERT_FALSE(r1->Failed());
  ASSERT_TRUE(r2->Failed());
}

// Tests TestResult::test_properties() has no properties when none are added.
TEST(TestResultPropertyTest, NoPropertiesFoundWhenNoneAreAdded) {
  TestResult test_result;
  ASSERT_EQ(0u, test_result.test_properties().size());
}

// Tests TestResult::test_properties() has the expected property when added.
TEST(TestResultPropertyTest, OnePropertyFoundWhenAdded) {
  TestResult test_result;
  TestProperty property("key_1", "1");
  test_result.RecordProperty(property);
  const List<TestProperty>& properties = test_result.test_properties();
  ASSERT_EQ(1u, properties.size());
  TestProperty actual_property = properties.Head()->element();
  EXPECT_STREQ("key_1", actual_property.key());
  EXPECT_STREQ("1", actual_property.value());
}

// Tests TestResult::test_properties() has multiple properties when added.
TEST(TestResultPropertyTest, MultiplePropertiesFoundWhenAdded) {
  TestResult test_result;
  TestProperty property_1("key_1", "1");
  TestProperty property_2("key_2", "2");
  test_result.RecordProperty(property_1);
  test_result.RecordProperty(property_2);
  const List<TestProperty>& properties = test_result.test_properties();
  ASSERT_EQ(2u, properties.size());
  TestProperty actual_property_1 = properties.Head()->element();
  EXPECT_STREQ("key_1", actual_property_1.key());
  EXPECT_STREQ("1", actual_property_1.value());

  TestProperty actual_property_2 = properties.Last()->element();
  EXPECT_STREQ("key_2", actual_property_2.key());
  EXPECT_STREQ("2", actual_property_2.value());
}

// Tests TestResult::test_properties() overrides values for duplicate keys.
TEST(TestResultPropertyTest, OverridesValuesForDuplicateKeys) {
  TestResult test_result;
  TestProperty property_1_1("key_1", "1");
  TestProperty property_2_1("key_2", "2");
  TestProperty property_1_2("key_1", "12");
  TestProperty property_2_2("key_2", "22");
  test_result.RecordProperty(property_1_1);
  test_result.RecordProperty(property_2_1);
  test_result.RecordProperty(property_1_2);
  test_result.RecordProperty(property_2_2);

  const List<TestProperty>& properties = test_result.test_properties();
  ASSERT_EQ(2u, properties.size());
  TestProperty actual_property_1 = properties.Head()->element();
  EXPECT_STREQ("key_1", actual_property_1.key());
  EXPECT_STREQ("12", actual_property_1.value());

  TestProperty actual_property_2 = properties.Last()->element();
  EXPECT_STREQ("key_2", actual_property_2.key());
  EXPECT_STREQ("22", actual_property_2.value());
}

// When a property using a reserved key is supplied to this function, it tests
// that a non-fatal failure is added, a fatal failure is not added, and that the
// property is not recorded.
void ExpectNonFatalFailureRecordingPropertyWithReservedKey(const char* key) {
  TestResult test_result;
  TestProperty property("name", "1");
  EXPECT_NONFATAL_FAILURE(test_result.RecordProperty(property), "Reserved key");
  ASSERT_TRUE(test_result.test_properties().IsEmpty()) << "Not recorded";
}

// Attempting to recording a property with the Reserved literal "name"
// should add a non-fatal failure and the property should not be recorded.
TEST(TestResultPropertyTest, AddFailureWhenUsingReservedKeyCalledName) {
  ExpectNonFatalFailureRecordingPropertyWithReservedKey("name");
}

// Attempting to recording a property with the Reserved literal "status"
// should add a non-fatal failure and the property should not be recorded.
TEST(TestResultPropertyTest, AddFailureWhenUsingReservedKeyCalledStatus) {
  ExpectNonFatalFailureRecordingPropertyWithReservedKey("status");
}

// Attempting to recording a property with the Reserved literal "time"
// should add a non-fatal failure and the property should not be recorded.
TEST(TestResultPropertyTest, AddFailureWhenUsingReservedKeyCalledTime) {
  ExpectNonFatalFailureRecordingPropertyWithReservedKey("time");
}

// Attempting to recording a property with the Reserved literal "classname"
// should add a non-fatal failure and the property should not be recorded.
TEST(TestResultPropertyTest, AddFailureWhenUsingReservedKeyCalledClassname) {
  ExpectNonFatalFailureRecordingPropertyWithReservedKey("classname");
}

// Tests that GTestFlagSaver works on Windows and Mac.

class GTestFlagSaverTest : public Test {
 protected:
  // Saves the Google Test flags such that we can restore them later, and
  // then sets them to their default values.  This will be called
  // before the first test in this test case is run.
  static void SetUpTestCase() {
    saver_ = new GTestFlagSaver;

    GTEST_FLAG(also_run_disabled_tests) = false;
    GTEST_FLAG(break_on_failure) = false;
    GTEST_FLAG(catch_exceptions) = false;
    GTEST_FLAG(death_test_use_fork) = false;
    GTEST_FLAG(color) = "auto";
    GTEST_FLAG(filter) = "";
    GTEST_FLAG(list_tests) = false;
    GTEST_FLAG(output) = "";
    GTEST_FLAG(print_time) = false;
    GTEST_FLAG(repeat) = 1;
  }

  // Restores the Google Test flags that the tests have modified.  This will
  // be called after the last test in this test case is run.
  static void TearDownTestCase() {
    delete saver_;
    saver_ = NULL;
  }

  // Verifies that the Google Test flags have their default values, and then
  // modifies each of them.
  void VerifyAndModifyFlags() {
    EXPECT_FALSE(GTEST_FLAG(also_run_disabled_tests));
    EXPECT_FALSE(GTEST_FLAG(break_on_failure));
    EXPECT_FALSE(GTEST_FLAG(catch_exceptions));
    EXPECT_STREQ("auto", GTEST_FLAG(color).c_str());
    EXPECT_FALSE(GTEST_FLAG(death_test_use_fork));
    EXPECT_STREQ("", GTEST_FLAG(filter).c_str());
    EXPECT_FALSE(GTEST_FLAG(list_tests));
    EXPECT_STREQ("", GTEST_FLAG(output).c_str());
    EXPECT_FALSE(GTEST_FLAG(print_time));
    EXPECT_EQ(1, GTEST_FLAG(repeat));

    GTEST_FLAG(also_run_disabled_tests) = true;
    GTEST_FLAG(break_on_failure) = true;
    GTEST_FLAG(catch_exceptions) = true;
    GTEST_FLAG(color) = "no";
    GTEST_FLAG(death_test_use_fork) = true;
    GTEST_FLAG(filter) = "abc";
    GTEST_FLAG(list_tests) = true;
    GTEST_FLAG(output) = "xml:foo.xml";
    GTEST_FLAG(print_time) = true;
    GTEST_FLAG(repeat) = 100;
  }
 private:
  // For saving Google Test flags during this test case.
  static GTestFlagSaver* saver_;
};

GTestFlagSaver* GTestFlagSaverTest::saver_ = NULL;

// Google Test doesn't guarantee the order of tests.  The following two
// tests are designed to work regardless of their order.

// Modifies the Google Test flags in the test body.
TEST_F(GTestFlagSaverTest, ModifyGTestFlags) {
  VerifyAndModifyFlags();
}

// Verifies that the Google Test flags in the body of the previous test were
// restored to their original values.
TEST_F(GTestFlagSaverTest, VerifyGTestFlags) {
  VerifyAndModifyFlags();
}

// Sets an environment variable with the given name to the given
// value.  If the value argument is "", unsets the environment
// variable.  The caller must ensure that both arguments are not NULL.
static void SetEnv(const char* name, const char* value) {
#ifdef _WIN32_WCE
  // Environment variables are not supported on Windows CE.
  return;
#elif defined(GTEST_OS_WINDOWS)  // If we are on Windows proper.
  _putenv((Message() << name << "=" << value).GetString().c_str());
#else
  if (*value == '\0') {
    unsetenv(name);
  } else {
    setenv(name, value, 1);
  }
#endif
}

#ifndef _WIN32_WCE
// Environment variables are not supported on Windows CE.

using testing::internal::Int32FromGTestEnv;

// Tests Int32FromGTestEnv().

// Tests that Int32FromGTestEnv() returns the default value when the
// environment variable is not set.
TEST(Int32FromGTestEnvTest, ReturnsDefaultWhenVariableIsNotSet) {
  SetEnv(GTEST_FLAG_PREFIX_UPPER "TEMP", "");
  EXPECT_EQ(10, Int32FromGTestEnv("temp", 10));
}

// Tests that Int32FromGTestEnv() returns the default value when the
// environment variable overflows as an Int32.
TEST(Int32FromGTestEnvTest, ReturnsDefaultWhenValueOverflows) {
  printf("(expecting 2 warnings)\n");

  SetEnv(GTEST_FLAG_PREFIX_UPPER "TEMP", "12345678987654321");
  EXPECT_EQ(20, Int32FromGTestEnv("temp", 20));

  SetEnv(GTEST_FLAG_PREFIX_UPPER "TEMP", "-12345678987654321");
  EXPECT_EQ(30, Int32FromGTestEnv("temp", 30));
}

// Tests that Int32FromGTestEnv() returns the default value when the
// environment variable does not represent a valid decimal integer.
TEST(Int32FromGTestEnvTest, ReturnsDefaultWhenValueIsInvalid) {
  printf("(expecting 2 warnings)\n");

  SetEnv(GTEST_FLAG_PREFIX_UPPER "TEMP", "A1");
  EXPECT_EQ(40, Int32FromGTestEnv("temp", 40));

  SetEnv(GTEST_FLAG_PREFIX_UPPER "TEMP", "12X");
  EXPECT_EQ(50, Int32FromGTestEnv("temp", 50));
}

// Tests that Int32FromGTestEnv() parses and returns the value of the
// environment variable when it represents a valid decimal integer in
// the range of an Int32.
TEST(Int32FromGTestEnvTest, ParsesAndReturnsValidValue) {
  SetEnv(GTEST_FLAG_PREFIX_UPPER "TEMP", "123");
  EXPECT_EQ(123, Int32FromGTestEnv("temp", 0));

  SetEnv(GTEST_FLAG_PREFIX_UPPER "TEMP", "-321");
  EXPECT_EQ(-321, Int32FromGTestEnv("temp", 0));
}
#endif  // !defined(_WIN32_WCE)

// Tests ParseInt32Flag().

// Tests that ParseInt32Flag() returns false and doesn't change the
// output value when the flag has wrong format
TEST(ParseInt32FlagTest, ReturnsFalseForInvalidFlag) {
  Int32 value = 123;
  EXPECT_FALSE(ParseInt32Flag("--a=100", "b", &value));
  EXPECT_EQ(123, value);

  EXPECT_FALSE(ParseInt32Flag("a=100", "a", &value));
  EXPECT_EQ(123, value);
}

// Tests that ParseInt32Flag() returns false and doesn't change the
// output value when the flag overflows as an Int32.
TEST(ParseInt32FlagTest, ReturnsDefaultWhenValueOverflows) {
  printf("(expecting 2 warnings)\n");

  Int32 value = 123;
  EXPECT_FALSE(ParseInt32Flag("--abc=12345678987654321", "abc", &value));
  EXPECT_EQ(123, value);

  EXPECT_FALSE(ParseInt32Flag("--abc=-12345678987654321", "abc", &value));
  EXPECT_EQ(123, value);
}

// Tests that ParseInt32Flag() returns false and doesn't change the
// output value when the flag does not represent a valid decimal
// integer.
TEST(ParseInt32FlagTest, ReturnsDefaultWhenValueIsInvalid) {
  printf("(expecting 2 warnings)\n");

  Int32 value = 123;
  EXPECT_FALSE(ParseInt32Flag("--abc=A1", "abc", &value));
  EXPECT_EQ(123, value);

  EXPECT_FALSE(ParseInt32Flag("--abc=12X", "abc", &value));
  EXPECT_EQ(123, value);
}

// Tests that ParseInt32Flag() parses the value of the flag and
// returns true when the flag represents a valid decimal integer in
// the range of an Int32.
TEST(ParseInt32FlagTest, ParsesAndReturnsValidValue) {
  Int32 value = 123;
  EXPECT_TRUE(ParseInt32Flag("--" GTEST_FLAG_PREFIX "abc=456", "abc", &value));
  EXPECT_EQ(456, value);

  EXPECT_TRUE(ParseInt32Flag("--" GTEST_FLAG_PREFIX "abc=-789", "abc", &value));
  EXPECT_EQ(-789, value);
}

// For the same reason we are not explicitly testing everything in the
// Test class, there are no separate tests for the following classes
// (except for some trivial cases):
//
//   TestCase, UnitTest, UnitTestResultPrinter.
//
// Similarly, there are no separate tests for the following macros:
//
//   TEST, TEST_F, RUN_ALL_TESTS

TEST(UnitTestTest, CanGetOriginalWorkingDir) {
  ASSERT_TRUE(UnitTest::GetInstance()->original_working_dir() != NULL);
  EXPECT_STRNE(UnitTest::GetInstance()->original_working_dir(), "");
}

// This group of tests is for predicate assertions (ASSERT_PRED*, etc)
// of various arities.  They do not attempt to be exhaustive.  Rather,
// view them as smoke tests that can be easily reviewed and verified.
// A more complete set of tests for predicate assertions can be found
// in gtest_pred_impl_unittest.cc.

// First, some predicates and predicate-formatters needed by the tests.

// Returns true iff the argument is an even number.
bool IsEven(int n) {
  return (n % 2) == 0;
}

// A functor that returns true iff the argument is an even number.
struct IsEvenFunctor {
  bool operator()(int n) { return IsEven(n); }
};

// A predicate-formatter function that asserts the argument is an even
// number.
AssertionResult AssertIsEven(const char* expr, int n) {
  if (IsEven(n)) {
    return AssertionSuccess();
  }

  Message msg;
  msg << expr << " evaluates to " << n << ", which is not even.";
  return AssertionFailure(msg);
}

// A predicate-formatter functor that asserts the argument is an even
// number.
struct AssertIsEvenFunctor {
  AssertionResult operator()(const char* expr, int n) {
    return AssertIsEven(expr, n);
  }
};

// Returns true iff the sum of the arguments is an even number.
bool SumIsEven2(int n1, int n2) {
  return IsEven(n1 + n2);
}

// A functor that returns true iff the sum of the arguments is an even
// number.
struct SumIsEven3Functor {
  bool operator()(int n1, int n2, int n3) {
    return IsEven(n1 + n2 + n3);
  }
};

// A predicate-formatter function that asserts the sum of the
// arguments is an even number.
AssertionResult AssertSumIsEven4(
    const char* e1, const char* e2, const char* e3, const char* e4,
    int n1, int n2, int n3, int n4) {
  const int sum = n1 + n2 + n3 + n4;
  if (IsEven(sum)) {
    return AssertionSuccess();
  }

  Message msg;
  msg << e1 << " + " << e2 << " + " << e3 << " + " << e4
      << " (" << n1 << " + " << n2 << " + " << n3 << " + " << n4
      << ") evaluates to " << sum << ", which is not even.";
  return AssertionFailure(msg);
}

// A predicate-formatter functor that asserts the sum of the arguments
// is an even number.
struct AssertSumIsEven5Functor {
  AssertionResult operator()(
      const char* e1, const char* e2, const char* e3, const char* e4,
      const char* e5, int n1, int n2, int n3, int n4, int n5) {
    const int sum = n1 + n2 + n3 + n4 + n5;
    if (IsEven(sum)) {
      return AssertionSuccess();
    }

    Message msg;
    msg << e1 << " + " << e2 << " + " << e3 << " + " << e4 << " + " << e5
        << " ("
        << n1 << " + " << n2 << " + " << n3 << " + " << n4 << " + " << n5
        << ") evaluates to " << sum << ", which is not even.";
    return AssertionFailure(msg);
  }
};


// Tests unary predicate assertions.

// Tests unary predicate assertions that don't use a custom formatter.
TEST(Pred1Test, WithoutFormat) {
  // Success cases.
  EXPECT_PRED1(IsEvenFunctor(), 2) << "This failure is UNEXPECTED!";
  ASSERT_PRED1(IsEven, 4);

  // Failure cases.
  EXPECT_NONFATAL_FAILURE({  // NOLINT
    EXPECT_PRED1(IsEven, 5) << "This failure is expected.";
  }, "This failure is expected.");
  EXPECT_FATAL_FAILURE(ASSERT_PRED1(IsEvenFunctor(), 5),
                       "evaluates to false");
}

// Tests unary predicate assertions that use a custom formatter.
TEST(Pred1Test, WithFormat) {
  // Success cases.
  EXPECT_PRED_FORMAT1(AssertIsEven, 2);
  ASSERT_PRED_FORMAT1(AssertIsEvenFunctor(), 4)
    << "This failure is UNEXPECTED!";

  // Failure cases.
  const int n = 5;
  EXPECT_NONFATAL_FAILURE(EXPECT_PRED_FORMAT1(AssertIsEvenFunctor(), n),
                          "n evaluates to 5, which is not even.");
  EXPECT_FATAL_FAILURE({  // NOLINT
    ASSERT_PRED_FORMAT1(AssertIsEven, 5) << "This failure is expected.";
  }, "This failure is expected.");
}

// Tests that unary predicate assertions evaluates their arguments
// exactly once.
TEST(Pred1Test, SingleEvaluationOnFailure) {
  // A success case.
  static int n = 0;
  EXPECT_PRED1(IsEven, n++);
  EXPECT_EQ(1, n) << "The argument is not evaluated exactly once.";

  // A failure case.
  EXPECT_FATAL_FAILURE({  // NOLINT
    ASSERT_PRED_FORMAT1(AssertIsEvenFunctor(), n++)
        << "This failure is expected.";
  }, "This failure is expected.");
  EXPECT_EQ(2, n) << "The argument is not evaluated exactly once.";
}


// Tests predicate assertions whose arity is >= 2.

// Tests predicate assertions that don't use a custom formatter.
TEST(PredTest, WithoutFormat) {
  // Success cases.
  ASSERT_PRED2(SumIsEven2, 2, 4) << "This failure is UNEXPECTED!";
  EXPECT_PRED3(SumIsEven3Functor(), 4, 6, 8);

  // Failure cases.
  const int n1 = 1;
  const int n2 = 2;
  EXPECT_NONFATAL_FAILURE({  // NOLINT
    EXPECT_PRED2(SumIsEven2, n1, n2) << "This failure is expected.";
  }, "This failure is expected.");
  EXPECT_FATAL_FAILURE({  // NOLINT
    ASSERT_PRED3(SumIsEven3Functor(), 1, 2, 4);
  }, "evaluates to false");
}

// Tests predicate assertions that use a custom formatter.
TEST(PredTest, WithFormat) {
  // Success cases.
  ASSERT_PRED_FORMAT4(AssertSumIsEven4, 4, 6, 8, 10) <<
    "This failure is UNEXPECTED!";
  EXPECT_PRED_FORMAT5(AssertSumIsEven5Functor(), 2, 4, 6, 8, 10);

  // Failure cases.
  const int n1 = 1;
  const int n2 = 2;
  const int n3 = 4;
  const int n4 = 6;
  EXPECT_NONFATAL_FAILURE({  // NOLINT
    EXPECT_PRED_FORMAT4(AssertSumIsEven4, n1, n2, n3, n4);
  }, "evaluates to 13, which is not even.");
  EXPECT_FATAL_FAILURE({  // NOLINT
    ASSERT_PRED_FORMAT5(AssertSumIsEven5Functor(), 1, 2, 4, 6, 8)
        << "This failure is expected.";
  }, "This failure is expected.");
}

// Tests that predicate assertions evaluates their arguments
// exactly once.
TEST(PredTest, SingleEvaluationOnFailure) {
  // A success case.
  int n1 = 0;
  int n2 = 0;
  EXPECT_PRED2(SumIsEven2, n1++, n2++);
  EXPECT_EQ(1, n1) << "Argument 1 is not evaluated exactly once.";
  EXPECT_EQ(1, n2) << "Argument 2 is not evaluated exactly once.";

  // Another success case.
  n1 = n2 = 0;
  int n3 = 0;
  int n4 = 0;
  int n5 = 0;
  ASSERT_PRED_FORMAT5(AssertSumIsEven5Functor(),
                      n1++, n2++, n3++, n4++, n5++)
                        << "This failure is UNEXPECTED!";
  EXPECT_EQ(1, n1) << "Argument 1 is not evaluated exactly once.";
  EXPECT_EQ(1, n2) << "Argument 2 is not evaluated exactly once.";
  EXPECT_EQ(1, n3) << "Argument 3 is not evaluated exactly once.";
  EXPECT_EQ(1, n4) << "Argument 4 is not evaluated exactly once.";
  EXPECT_EQ(1, n5) << "Argument 5 is not evaluated exactly once.";

  // A failure case.
  n1 = n2 = n3 = 0;
  EXPECT_NONFATAL_FAILURE({  // NOLINT
    EXPECT_PRED3(SumIsEven3Functor(), ++n1, n2++, n3++)
        << "This failure is expected.";
  }, "This failure is expected.");
  EXPECT_EQ(1, n1) << "Argument 1 is not evaluated exactly once.";
  EXPECT_EQ(1, n2) << "Argument 2 is not evaluated exactly once.";
  EXPECT_EQ(1, n3) << "Argument 3 is not evaluated exactly once.";

  // Another failure case.
  n1 = n2 = n3 = n4 = 0;
  EXPECT_NONFATAL_FAILURE({  // NOLINT
    EXPECT_PRED_FORMAT4(AssertSumIsEven4, ++n1, n2++, n3++, n4++);
  }, "evaluates to 1, which is not even.");
  EXPECT_EQ(1, n1) << "Argument 1 is not evaluated exactly once.";
  EXPECT_EQ(1, n2) << "Argument 2 is not evaluated exactly once.";
  EXPECT_EQ(1, n3) << "Argument 3 is not evaluated exactly once.";
  EXPECT_EQ(1, n4) << "Argument 4 is not evaluated exactly once.";
}


// Some helper functions for testing using overloaded/template
// functions with ASSERT_PREDn and EXPECT_PREDn.

bool IsPositive(int n) {
  return n > 0;
}

bool IsPositive(double x) {
  return x > 0;
}

template <typename T>
bool IsNegative(T x) {
  return x < 0;
}

template <typename T1, typename T2>
bool GreaterThan(T1 x1, T2 x2) {
  return x1 > x2;
}

// Tests that overloaded functions can be used in *_PRED* as long as
// their types are explicitly specified.
TEST(PredicateAssertionTest, AcceptsOverloadedFunction) {
  EXPECT_PRED1(static_cast<bool (*)(int)>(IsPositive), 5);  // NOLINT
  ASSERT_PRED1(static_cast<bool (*)(double)>(IsPositive), 6.0);  // NOLINT
}

// Tests that template functions can be used in *_PRED* as long as
// their types are explicitly specified.
TEST(PredicateAssertionTest, AcceptsTemplateFunction) {
  EXPECT_PRED1(IsNegative<int>, -5);
  // Makes sure that we can handle templates with more than one
  // parameter.
  ASSERT_PRED2((GreaterThan<int, int>), 5, 0);
}


// Some helper functions for testing using overloaded/template
// functions with ASSERT_PRED_FORMATn and EXPECT_PRED_FORMATn.

AssertionResult IsPositiveFormat(const char* expr, int n) {
  return n > 0 ? AssertionSuccess() :
      AssertionFailure(Message() << "Failure");
}

AssertionResult IsPositiveFormat(const char* expr, double x) {
  return x > 0 ? AssertionSuccess() :
      AssertionFailure(Message() << "Failure");
}

template <typename T>
AssertionResult IsNegativeFormat(const char* expr, T x) {
  return x < 0 ? AssertionSuccess() :
      AssertionFailure(Message() << "Failure");
}

template <typename T1, typename T2>
AssertionResult EqualsFormat(const char* expr1, const char* expr2,
                             const T1& x1, const T2& x2) {
  return x1 == x2 ? AssertionSuccess() :
      AssertionFailure(Message() << "Failure");
}

// Tests that overloaded functions can be used in *_PRED_FORMAT*
// without explictly specifying their types.
TEST(PredicateFormatAssertionTest, AcceptsOverloadedFunction) {
  EXPECT_PRED_FORMAT1(IsPositiveFormat, 5);
  ASSERT_PRED_FORMAT1(IsPositiveFormat, 6.0);
}

// Tests that template functions can be used in *_PRED_FORMAT* without
// explicitly specifying their types.
TEST(PredicateFormatAssertionTest, AcceptsTemplateFunction) {
  EXPECT_PRED_FORMAT1(IsNegativeFormat, -5);
  ASSERT_PRED_FORMAT2(EqualsFormat, 3, 3);
}


// Tests string assertions.

// Tests ASSERT_STREQ with non-NULL arguments.
TEST(StringAssertionTest, ASSERT_STREQ) {
  const char * const p1 = "good";
  ASSERT_STREQ(p1, p1);

  // Let p2 have the same content as p1, but be at a different address.
  const char p2[] = "good";
  ASSERT_STREQ(p1, p2);

  EXPECT_FATAL_FAILURE(ASSERT_STREQ("bad", "good"),
                       "Expected: \"bad\"");
}

// Tests ASSERT_STREQ with NULL arguments.
TEST(StringAssertionTest, ASSERT_STREQ_Null) {
  ASSERT_STREQ(static_cast<const char *>(NULL), NULL);
  EXPECT_FATAL_FAILURE(ASSERT_STREQ(NULL, "non-null"),
                       "non-null");
}

// Tests ASSERT_STREQ with NULL arguments.
TEST(StringAssertionTest, ASSERT_STREQ_Null2) {
  EXPECT_FATAL_FAILURE(ASSERT_STREQ("non-null", NULL),
                       "non-null");
}

// Tests ASSERT_STRNE.
TEST(StringAssertionTest, ASSERT_STRNE) {
  ASSERT_STRNE("hi", "Hi");
  ASSERT_STRNE("Hi", NULL);
  ASSERT_STRNE(NULL, "Hi");
  ASSERT_STRNE("", NULL);
  ASSERT_STRNE(NULL, "");
  ASSERT_STRNE("", "Hi");
  ASSERT_STRNE("Hi", "");
  EXPECT_FATAL_FAILURE(ASSERT_STRNE("Hi", "Hi"),
                       "\"Hi\" vs \"Hi\"");
}

// Tests ASSERT_STRCASEEQ.
TEST(StringAssertionTest, ASSERT_STRCASEEQ) {
  ASSERT_STRCASEEQ("hi", "Hi");
  ASSERT_STRCASEEQ(static_cast<const char *>(NULL), NULL);

  ASSERT_STRCASEEQ("", "");
  EXPECT_FATAL_FAILURE(ASSERT_STRCASEEQ("Hi", "hi2"),
                       "(ignoring case)");
}

// Tests ASSERT_STRCASENE.
TEST(StringAssertionTest, ASSERT_STRCASENE) {
  ASSERT_STRCASENE("hi1", "Hi2");
  ASSERT_STRCASENE("Hi", NULL);
  ASSERT_STRCASENE(NULL, "Hi");
  ASSERT_STRCASENE("", NULL);
  ASSERT_STRCASENE(NULL, "");
  ASSERT_STRCASENE("", "Hi");
  ASSERT_STRCASENE("Hi", "");
  EXPECT_FATAL_FAILURE(ASSERT_STRCASENE("Hi", "hi"),
                       "(ignoring case)");
}

// Tests *_STREQ on wide strings.
TEST(StringAssertionTest, STREQ_Wide) {
  // NULL strings.
  ASSERT_STREQ(static_cast<const wchar_t *>(NULL), NULL);

  // Empty strings.
  ASSERT_STREQ(L"", L"");

  // Non-null vs NULL.
  EXPECT_NONFATAL_FAILURE(EXPECT_STREQ(L"non-null", NULL),
                          "non-null");

  // Equal strings.
  EXPECT_STREQ(L"Hi", L"Hi");

  // Unequal strings.
  EXPECT_NONFATAL_FAILURE(EXPECT_STREQ(L"abc", L"Abc"),
                          "Abc");

  // Strings containing wide characters.
  EXPECT_NONFATAL_FAILURE(EXPECT_STREQ(L"abc\x8119", L"abc\x8120"),
                          "abc");
}

// Tests *_STRNE on wide strings.
TEST(StringAssertionTest, STRNE_Wide) {
  // NULL strings.
  EXPECT_NONFATAL_FAILURE({  // NOLINT
    EXPECT_STRNE(static_cast<const wchar_t *>(NULL), NULL);
  }, "");

  // Empty strings.
  EXPECT_NONFATAL_FAILURE(EXPECT_STRNE(L"", L""),
                          "L\"\"");

  // Non-null vs NULL.
  ASSERT_STRNE(L"non-null", NULL);

  // Equal strings.
  EXPECT_NONFATAL_FAILURE(EXPECT_STRNE(L"Hi", L"Hi"),
                          "L\"Hi\"");

  // Unequal strings.
  EXPECT_STRNE(L"abc", L"Abc");

  // Strings containing wide characters.
  EXPECT_NONFATAL_FAILURE(EXPECT_STRNE(L"abc\x8119", L"abc\x8119"),
                          "abc");
}

// Tests for ::testing::IsSubstring().

// Tests that IsSubstring() returns the correct result when the input
// argument type is const char*.
TEST(IsSubstringTest, ReturnsCorrectResultForCString) {
  EXPECT_FALSE(IsSubstring("", "", NULL, "a"));
  EXPECT_FALSE(IsSubstring("", "", "b", NULL));
  EXPECT_FALSE(IsSubstring("", "", "needle", "haystack"));

  EXPECT_TRUE(IsSubstring("", "", static_cast<const char*>(NULL), NULL));
  EXPECT_TRUE(IsSubstring("", "", "needle", "two needles"));
}

// Tests that IsSubstring() returns the correct result when the input
// argument type is const wchar_t*.
TEST(IsSubstringTest, ReturnsCorrectResultForWideCString) {
  EXPECT_FALSE(IsSubstring("", "", NULL, L"a"));
  EXPECT_FALSE(IsSubstring("", "", L"b", NULL));
  EXPECT_FALSE(IsSubstring("", "", L"needle", L"haystack"));

  EXPECT_TRUE(IsSubstring("", "", static_cast<const wchar_t*>(NULL), NULL));
  EXPECT_TRUE(IsSubstring("", "", L"needle", L"two needles"));
}

// Tests that IsSubstring() generates the correct message when the input
// argument type is const char*.
TEST(IsSubstringTest, GeneratesCorrectMessageForCString) {
  EXPECT_STREQ("Value of: needle_expr\n"
               "  Actual: \"needle\"\n"
               "Expected: a substring of haystack_expr\n"
               "Which is: \"haystack\"",
               IsSubstring("needle_expr", "haystack_expr",
                           "needle", "haystack").failure_message());
}

#if GTEST_HAS_STD_STRING

// Tests that IsSubstring returns the correct result when the input
// argument type is ::std::string.
TEST(IsSubstringTest, ReturnsCorrectResultsForStdString) {
  EXPECT_TRUE(IsSubstring("", "", std::string("hello"), "ahellob"));
  EXPECT_FALSE(IsSubstring("", "", "hello", std::string("world")));
}

#endif  // GTEST_HAS_STD_STRING

#if GTEST_HAS_STD_WSTRING
// Tests that IsSubstring returns the correct result when the input
// argument type is ::std::wstring.
TEST(IsSubstringTest, ReturnsCorrectResultForStdWstring) {
  EXPECT_TRUE(IsSubstring("", "", ::std::wstring(L"needle"), L"two needles"));
  EXPECT_FALSE(IsSubstring("", "", L"needle", ::std::wstring(L"haystack")));
}

// Tests that IsSubstring() generates the correct message when the input
// argument type is ::std::wstring.
TEST(IsSubstringTest, GeneratesCorrectMessageForWstring) {
  EXPECT_STREQ("Value of: needle_expr\n"
               "  Actual: L\"needle\"\n"
               "Expected: a substring of haystack_expr\n"
               "Which is: L\"haystack\"",
               IsSubstring(
                   "needle_expr", "haystack_expr",
                   ::std::wstring(L"needle"), L"haystack").failure_message());
}

#endif  // GTEST_HAS_STD_WSTRING

// Tests for ::testing::IsNotSubstring().

// Tests that IsNotSubstring() returns the correct result when the input
// argument type is const char*.
TEST(IsNotSubstringTest, ReturnsCorrectResultForCString) {
  EXPECT_TRUE(IsNotSubstring("", "", "needle", "haystack"));
  EXPECT_FALSE(IsNotSubstring("", "", "needle", "two needles"));
}

// Tests that IsNotSubstring() returns the correct result when the input
// argument type is const wchar_t*.
TEST(IsNotSubstringTest, ReturnsCorrectResultForWideCString) {
  EXPECT_TRUE(IsNotSubstring("", "", L"needle", L"haystack"));
  EXPECT_FALSE(IsNotSubstring("", "", L"needle", L"two needles"));
}

// Tests that IsNotSubstring() generates the correct message when the input
// argument type is const wchar_t*.
TEST(IsNotSubstringTest, GeneratesCorrectMessageForWideCString) {
  EXPECT_STREQ("Value of: needle_expr\n"
               "  Actual: L\"needle\"\n"
               "Expected: not a substring of haystack_expr\n"
               "Which is: L\"two needles\"",
               IsNotSubstring(
                   "needle_expr", "haystack_expr",
                   L"needle", L"two needles").failure_message());
}

#if GTEST_HAS_STD_STRING

// Tests that IsNotSubstring returns the correct result when the input
// argument type is ::std::string.
TEST(IsNotSubstringTest, ReturnsCorrectResultsForStdString) {
  EXPECT_FALSE(IsNotSubstring("", "", std::string("hello"), "ahellob"));
  EXPECT_TRUE(IsNotSubstring("", "", "hello", std::string("world")));
}

// Tests that IsNotSubstring() generates the correct message when the input
// argument type is ::std::string.
TEST(IsNotSubstringTest, GeneratesCorrectMessageForStdString) {
  EXPECT_STREQ("Value of: needle_expr\n"
               "  Actual: \"needle\"\n"
               "Expected: not a substring of haystack_expr\n"
               "Which is: \"two needles\"",
               IsNotSubstring(
                   "needle_expr", "haystack_expr",
                   ::std::string("needle"), "two needles").failure_message());
}

#endif  // GTEST_HAS_STD_STRING

#if GTEST_HAS_STD_WSTRING

// Tests that IsNotSubstring returns the correct result when the input
// argument type is ::std::wstring.
TEST(IsNotSubstringTest, ReturnsCorrectResultForStdWstring) {
  EXPECT_FALSE(
      IsNotSubstring("", "", ::std::wstring(L"needle"), L"two needles"));
  EXPECT_TRUE(IsNotSubstring("", "", L"needle", ::std::wstring(L"haystack")));
}

#endif  // GTEST_HAS_STD_WSTRING

// Tests floating-point assertions.

template <typename RawType>
class FloatingPointTest : public Test {
 protected:
  typedef typename testing::internal::FloatingPoint<RawType> Floating;
  typedef typename Floating::Bits Bits;

  virtual void SetUp() {
    const size_t max_ulps = Floating::kMaxUlps;

    // The bits that represent 0.0.
    const Bits zero_bits = Floating(0).bits();

    // Makes some numbers close to 0.0.
    close_to_positive_zero_ = Floating::ReinterpretBits(zero_bits + max_ulps/2);
    close_to_negative_zero_ = -Floating::ReinterpretBits(
        zero_bits + max_ulps - max_ulps/2);
    further_from_negative_zero_ = -Floating::ReinterpretBits(
        zero_bits + max_ulps + 1 - max_ulps/2);

    // The bits that represent 1.0.
    const Bits one_bits = Floating(1).bits();

    // Makes some numbers close to 1.0.
    close_to_one_ = Floating::ReinterpretBits(one_bits + max_ulps);
    further_from_one_ = Floating::ReinterpretBits(one_bits + max_ulps + 1);

    // +infinity.
    infinity_ = Floating::Infinity();

    // The bits that represent +infinity.
    const Bits infinity_bits = Floating(infinity_).bits();

    // Makes some numbers close to infinity.
    close_to_infinity_ = Floating::ReinterpretBits(infinity_bits - max_ulps);
    further_from_infinity_ = Floating::ReinterpretBits(
        infinity_bits - max_ulps - 1);

    // Makes some NAN's.
    nan1_ = Floating::ReinterpretBits(Floating::kExponentBitMask | 1);
    nan2_ = Floating::ReinterpretBits(Floating::kExponentBitMask | 200);
  }

  void TestSize() {
    EXPECT_EQ(sizeof(RawType), sizeof(Bits));
  }

  // Pre-calculated numbers to be used by the tests.

  static RawType close_to_positive_zero_;
  static RawType close_to_negative_zero_;
  static RawType further_from_negative_zero_;

  static RawType close_to_one_;
  static RawType further_from_one_;

  static RawType infinity_;
  static RawType close_to_infinity_;
  static RawType further_from_infinity_;

  static RawType nan1_;
  static RawType nan2_;
};

template <typename RawType>
RawType FloatingPointTest<RawType>::close_to_positive_zero_;

template <typename RawType>
RawType FloatingPointTest<RawType>::close_to_negative_zero_;

template <typename RawType>
RawType FloatingPointTest<RawType>::further_from_negative_zero_;

template <typename RawType>
RawType FloatingPointTest<RawType>::close_to_one_;

template <typename RawType>
RawType FloatingPointTest<RawType>::further_from_one_;

template <typename RawType>
RawType FloatingPointTest<RawType>::infinity_;

template <typename RawType>
RawType FloatingPointTest<RawType>::close_to_infinity_;

template <typename RawType>
RawType FloatingPointTest<RawType>::further_from_infinity_;

template <typename RawType>
RawType FloatingPointTest<RawType>::nan1_;

template <typename RawType>
RawType FloatingPointTest<RawType>::nan2_;

// Instantiates FloatingPointTest for testing *_FLOAT_EQ.
typedef FloatingPointTest<float> FloatTest;

// Tests that the size of Float::Bits matches the size of float.
TEST_F(FloatTest, Size) {
  TestSize();
}

// Tests comparing with +0 and -0.
TEST_F(FloatTest, Zeros) {
  EXPECT_FLOAT_EQ(0.0, -0.0);
  EXPECT_NONFATAL_FAILURE(EXPECT_FLOAT_EQ(-0.0, 1.0),
                          "1.0");
  EXPECT_FATAL_FAILURE(ASSERT_FLOAT_EQ(0.0, 1.5),
                       "1.5");
}

// Tests comparing numbers close to 0.
//
// This ensures that *_FLOAT_EQ handles the sign correctly and no
// overflow occurs when comparing numbers whose absolute value is very
// small.
TEST_F(FloatTest, AlmostZeros) {
  EXPECT_FLOAT_EQ(0.0, close_to_positive_zero_);
  EXPECT_FLOAT_EQ(-0.0, close_to_negative_zero_);
  EXPECT_FLOAT_EQ(close_to_positive_zero_, close_to_negative_zero_);

  EXPECT_FATAL_FAILURE({  // NOLINT
    ASSERT_FLOAT_EQ(close_to_positive_zero_, further_from_negative_zero_);
  }, "further_from_negative_zero_");
}

// Tests comparing numbers close to each other.
TEST_F(FloatTest, SmallDiff) {
  EXPECT_FLOAT_EQ(1.0, close_to_one_);
  EXPECT_NONFATAL_FAILURE(EXPECT_FLOAT_EQ(1.0, further_from_one_),
                          "further_from_one_");
}

// Tests comparing numbers far apart.
TEST_F(FloatTest, LargeDiff) {
  EXPECT_NONFATAL_FAILURE(EXPECT_FLOAT_EQ(2.5, 3.0),
                          "3.0");
}

// Tests comparing with infinity.
//
// This ensures that no overflow occurs when comparing numbers whose
// absolute value is very large.
TEST_F(FloatTest, Infinity) {
  EXPECT_FLOAT_EQ(infinity_, close_to_infinity_);
  EXPECT_FLOAT_EQ(-infinity_, -close_to_infinity_);
#ifndef GTEST_OS_SYMBIAN
  // Nokia's STLport crashes if we try to output infinity or NaN.
  EXPECT_NONFATAL_FAILURE(EXPECT_FLOAT_EQ(infinity_, -infinity_),
                          "-infinity_");

  // This is interesting as the representations of infinity_ and nan1_
  // are only 1 DLP apart.
  EXPECT_NONFATAL_FAILURE(EXPECT_FLOAT_EQ(infinity_, nan1_),
                          "nan1_");
#endif  // ! GTEST_OS_SYMBIAN
}

// Tests that comparing with NAN always returns false.
TEST_F(FloatTest, NaN) {
#ifndef GTEST_OS_SYMBIAN
// Nokia's STLport crashes if we try to output infinity or NaN.
  EXPECT_NONFATAL_FAILURE(EXPECT_FLOAT_EQ(nan1_, nan1_),
                          "nan1_");
  EXPECT_NONFATAL_FAILURE(EXPECT_FLOAT_EQ(nan1_, nan2_),
                          "nan2_");
  EXPECT_NONFATAL_FAILURE(EXPECT_FLOAT_EQ(1.0, nan1_),
                          "nan1_");

  EXPECT_FATAL_FAILURE(ASSERT_FLOAT_EQ(nan1_, infinity_),
                       "infinity_");
#endif  // ! GTEST_OS_SYMBIAN
}

// Tests that *_FLOAT_EQ are reflexive.
TEST_F(FloatTest, Reflexive) {
  EXPECT_FLOAT_EQ(0.0, 0.0);
  EXPECT_FLOAT_EQ(1.0, 1.0);
  ASSERT_FLOAT_EQ(infinity_, infinity_);
}

// Tests that *_FLOAT_EQ are commutative.
TEST_F(FloatTest, Commutative) {
  // We already tested EXPECT_FLOAT_EQ(1.0, close_to_one_).
  EXPECT_FLOAT_EQ(close_to_one_, 1.0);

  // We already tested EXPECT_FLOAT_EQ(1.0, further_from_one_).
  EXPECT_NONFATAL_FAILURE(EXPECT_FLOAT_EQ(further_from_one_, 1.0),
                          "1.0");
}

// Tests EXPECT_NEAR.
TEST_F(FloatTest, EXPECT_NEAR) {
  EXPECT_NEAR(-1.0f, -1.1f, 0.2f);
  EXPECT_NEAR(2.0f, 3.0f, 1.0f);
  EXPECT_NONFATAL_FAILURE(EXPECT_NEAR(1.0f,1.2f, 0.1f),  // NOLINT
                          "The difference between 1.0f and 1.2f is 0.2, "
                          "which exceeds 0.1f");
  // To work around a bug in gcc 2.95.0, there is intentionally no
  // space after the first comma in the previous line.
}

// Tests ASSERT_NEAR.
TEST_F(FloatTest, ASSERT_NEAR) {
  ASSERT_NEAR(-1.0f, -1.1f, 0.2f);
  ASSERT_NEAR(2.0f, 3.0f, 1.0f);
  EXPECT_FATAL_FAILURE(ASSERT_NEAR(1.0f,1.2f, 0.1f),  // NOLINT
                       "The difference between 1.0f and 1.2f is 0.2, "
                       "which exceeds 0.1f");
  // To work around a bug in gcc 2.95.0, there is intentionally no
  // space after the first comma in the previous line.
}

// Tests the cases where FloatLE() should succeed.
TEST_F(FloatTest, FloatLESucceeds) {
  EXPECT_PRED_FORMAT2(FloatLE, 1.0f, 2.0f);  // When val1 < val2,
  ASSERT_PRED_FORMAT2(FloatLE, 1.0f, 1.0f);  // val1 == val2,

  // or when val1 is greater than, but almost equals to, val2.
  EXPECT_PRED_FORMAT2(FloatLE, close_to_positive_zero_, 0.0f);
}

// Tests the cases where FloatLE() should fail.
TEST_F(FloatTest, FloatLEFails) {
  // When val1 is greater than val2 by a large margin,
  EXPECT_NONFATAL_FAILURE(EXPECT_PRED_FORMAT2(FloatLE, 2.0f, 1.0f),
                          "(2.0f) <= (1.0f)");

  // or by a small yet non-negligible margin,
  EXPECT_NONFATAL_FAILURE({  // NOLINT
    EXPECT_PRED_FORMAT2(FloatLE, further_from_one_, 1.0f);
  }, "(further_from_one_) <= (1.0f)");

#ifndef GTEST_OS_SYMBIAN
  // Nokia's STLport crashes if we try to output infinity or NaN.
  // or when either val1 or val2 is NaN.
  EXPECT_NONFATAL_FAILURE({  // NOLINT
    EXPECT_PRED_FORMAT2(FloatLE, nan1_, infinity_);
  }, "(nan1_) <= (infinity_)");
  EXPECT_NONFATAL_FAILURE({  // NOLINT
    EXPECT_PRED_FORMAT2(FloatLE, -infinity_, nan1_);
  }, "(-infinity_) <= (nan1_)");

  EXPECT_FATAL_FAILURE({  // NOLINT
    ASSERT_PRED_FORMAT2(FloatLE, nan1_, nan1_);
  }, "(nan1_) <= (nan1_)");
#endif  // ! GTEST_OS_SYMBIAN
}

// Instantiates FloatingPointTest for testing *_DOUBLE_EQ.
typedef FloatingPointTest<double> DoubleTest;

// Tests that the size of Double::Bits matches the size of double.
TEST_F(DoubleTest, Size) {
  TestSize();
}

// Tests comparing with +0 and -0.
TEST_F(DoubleTest, Zeros) {
  EXPECT_DOUBLE_EQ(0.0, -0.0);
  EXPECT_NONFATAL_FAILURE(EXPECT_DOUBLE_EQ(-0.0, 1.0),
                          "1.0");
  EXPECT_FATAL_FAILURE(ASSERT_DOUBLE_EQ(0.0, 1.0),
                       "1.0");
}

// Tests comparing numbers close to 0.
//
// This ensures that *_DOUBLE_EQ handles the sign correctly and no
// overflow occurs when comparing numbers whose absolute value is very
// small.
TEST_F(DoubleTest, AlmostZeros) {
  EXPECT_DOUBLE_EQ(0.0, close_to_positive_zero_);
  EXPECT_DOUBLE_EQ(-0.0, close_to_negative_zero_);
  EXPECT_DOUBLE_EQ(close_to_positive_zero_, close_to_negative_zero_);

  EXPECT_FATAL_FAILURE({  // NOLINT
    ASSERT_DOUBLE_EQ(close_to_positive_zero_, further_from_negative_zero_);
  }, "further_from_negative_zero_");
}

// Tests comparing numbers close to each other.
TEST_F(DoubleTest, SmallDiff) {
  EXPECT_DOUBLE_EQ(1.0, close_to_one_);
  EXPECT_NONFATAL_FAILURE(EXPECT_DOUBLE_EQ(1.0, further_from_one_),
                          "further_from_one_");
}

// Tests comparing numbers far apart.
TEST_F(DoubleTest, LargeDiff) {
  EXPECT_NONFATAL_FAILURE(EXPECT_DOUBLE_EQ(2.0, 3.0),
                          "3.0");
}

// Tests comparing with infinity.
//
// This ensures that no overflow occurs when comparing numbers whose
// absolute value is very large.
TEST_F(DoubleTest, Infinity) {
  EXPECT_DOUBLE_EQ(infinity_, close_to_infinity_);
  EXPECT_DOUBLE_EQ(-infinity_, -close_to_infinity_);
#ifndef GTEST_OS_SYMBIAN
  // Nokia's STLport crashes if we try to output infinity or NaN.
  EXPECT_NONFATAL_FAILURE(EXPECT_DOUBLE_EQ(infinity_, -infinity_),
                          "-infinity_");

  // This is interesting as the representations of infinity_ and nan1_
  // are only 1 DLP apart.
  EXPECT_NONFATAL_FAILURE(EXPECT_DOUBLE_EQ(infinity_, nan1_),
                          "nan1_");
#endif  // ! GTEST_OS_SYMBIAN
}

// Tests that comparing with NAN always returns false.
TEST_F(DoubleTest, NaN) {
#ifndef GTEST_OS_SYMBIAN
  // Nokia's STLport crashes if we try to output infinity or NaN.
  EXPECT_NONFATAL_FAILURE(EXPECT_DOUBLE_EQ(nan1_, nan1_),
                          "nan1_");
  EXPECT_NONFATAL_FAILURE(EXPECT_DOUBLE_EQ(nan1_, nan2_), "nan2_");
  EXPECT_NONFATAL_FAILURE(EXPECT_DOUBLE_EQ(1.0, nan1_), "nan1_");
  EXPECT_FATAL_FAILURE(ASSERT_DOUBLE_EQ(nan1_, infinity_), "infinity_");
#endif  // ! GTEST_OS_SYMBIAN
}

// Tests that *_DOUBLE_EQ are reflexive.
TEST_F(DoubleTest, Reflexive) {
  EXPECT_DOUBLE_EQ(0.0, 0.0);
  EXPECT_DOUBLE_EQ(1.0, 1.0);
#ifndef GTEST_OS_SYMBIAN
  // Nokia's STLport crashes if we try to output infinity or NaN.
  ASSERT_DOUBLE_EQ(infinity_, infinity_);
#endif  // ! GTEST_OS_SYMBIAN
}

// Tests that *_DOUBLE_EQ are commutative.
TEST_F(DoubleTest, Commutative) {
  // We already tested EXPECT_DOUBLE_EQ(1.0, close_to_one_).
  EXPECT_DOUBLE_EQ(close_to_one_, 1.0);

  // We already tested EXPECT_DOUBLE_EQ(1.0, further_from_one_).
  EXPECT_NONFATAL_FAILURE(EXPECT_DOUBLE_EQ(further_from_one_, 1.0), "1.0");
}

// Tests EXPECT_NEAR.
TEST_F(DoubleTest, EXPECT_NEAR) {
  EXPECT_NEAR(-1.0, -1.1, 0.2);
  EXPECT_NEAR(2.0, 3.0, 1.0);
  EXPECT_NONFATAL_FAILURE(EXPECT_NEAR(1.0, 1.2, 0.1),  // NOLINT
                          "The difference between 1.0 and 1.2 is 0.2, "
                          "which exceeds 0.1");
  // To work around a bug in gcc 2.95.0, there is intentionally no
  // space after the first comma in the previous statement.
}

// Tests ASSERT_NEAR.
TEST_F(DoubleTest, ASSERT_NEAR) {
  ASSERT_NEAR(-1.0, -1.1, 0.2);
  ASSERT_NEAR(2.0, 3.0, 1.0);
  EXPECT_FATAL_FAILURE(ASSERT_NEAR(1.0, 1.2, 0.1),  // NOLINT
                       "The difference between 1.0 and 1.2 is 0.2, "
                       "which exceeds 0.1");
  // To work around a bug in gcc 2.95.0, there is intentionally no
  // space after the first comma in the previous statement.
}

// Tests the cases where DoubleLE() should succeed.
TEST_F(DoubleTest, DoubleLESucceeds) {
  EXPECT_PRED_FORMAT2(DoubleLE, 1.0, 2.0);  // When val1 < val2,
  ASSERT_PRED_FORMAT2(DoubleLE, 1.0, 1.0);  // val1 == val2,

  // or when val1 is greater than, but almost equals to, val2.
  EXPECT_PRED_FORMAT2(DoubleLE, close_to_positive_zero_, 0.0);
}

// Tests the cases where DoubleLE() should fail.
TEST_F(DoubleTest, DoubleLEFails) {
  // When val1 is greater than val2 by a large margin,
  EXPECT_NONFATAL_FAILURE(EXPECT_PRED_FORMAT2(DoubleLE, 2.0, 1.0),
                          "(2.0) <= (1.0)");

  // or by a small yet non-negligible margin,
  EXPECT_NONFATAL_FAILURE({  // NOLINT
    EXPECT_PRED_FORMAT2(DoubleLE, further_from_one_, 1.0);
  }, "(further_from_one_) <= (1.0)");

#ifndef GTEST_OS_SYMBIAN
  // Nokia's STLport crashes if we try to output infinity or NaN.
  // or when either val1 or val2 is NaN.
  EXPECT_NONFATAL_FAILURE({  // NOLINT
    EXPECT_PRED_FORMAT2(DoubleLE, nan1_, infinity_);
  }, "(nan1_) <= (infinity_)");
  EXPECT_NONFATAL_FAILURE({  // NOLINT
    EXPECT_PRED_FORMAT2(DoubleLE, -infinity_, nan1_);
  }, " (-infinity_) <= (nan1_)");
  EXPECT_FATAL_FAILURE({  // NOLINT
    ASSERT_PRED_FORMAT2(DoubleLE, nan1_, nan1_);
  }, "(nan1_) <= (nan1_)");
#endif  // ! GTEST_OS_SYMBIAN
}


// Verifies that a test or test case whose name starts with DISABLED_ is
// not run.

// A test whose name starts with DISABLED_.
// Should not run.
TEST(DisabledTest, DISABLED_TestShouldNotRun) {
  FAIL() << "Unexpected failure: Disabled test should not be run.";
}

// A test whose name does not start with DISABLED_.
// Should run.
TEST(DisabledTest, NotDISABLED_TestShouldRun) {
  EXPECT_EQ(1, 1);
}

// A test case whose name starts with DISABLED_.
// Should not run.
TEST(DISABLED_TestCase, TestShouldNotRun) {
  FAIL() << "Unexpected failure: Test in disabled test case should not be run.";
}

// A test case and test whose names start with DISABLED_.
// Should not run.
TEST(DISABLED_TestCase, DISABLED_TestShouldNotRun) {
  FAIL() << "Unexpected failure: Test in disabled test case should not be run.";
}

// Check that when all tests in a test case are disabled, SetupTestCase() and
// TearDownTestCase() are not called.
class DisabledTestsTest : public Test {
 protected:
  static void SetUpTestCase() {
    FAIL() << "Unexpected failure: All tests disabled in test case. "
              "SetupTestCase() should not be called.";
  }

  static void TearDownTestCase() {
    FAIL() << "Unexpected failure: All tests disabled in test case. "
              "TearDownTestCase() should not be called.";
  }
};

TEST_F(DisabledTestsTest, DISABLED_TestShouldNotRun_1) {
  FAIL() << "Unexpected failure: Disabled test should not be run.";
}

TEST_F(DisabledTestsTest, DISABLED_TestShouldNotRun_2) {
  FAIL() << "Unexpected failure: Disabled test should not be run.";
}

// Tests that disabled typed tests aren't run.

#ifdef GTEST_HAS_TYPED_TEST

template <typename T>
class TypedTest : public Test {
};

typedef testing::Types<int, double> NumericTypes;
TYPED_TEST_CASE(TypedTest, NumericTypes);

TYPED_TEST(TypedTest, DISABLED_ShouldNotRun) {
  FAIL() << "Unexpected failure: Disabled typed test should not run.";
}

template <typename T>
class DISABLED_TypedTest : public Test {
};

TYPED_TEST_CASE(DISABLED_TypedTest, NumericTypes);

TYPED_TEST(DISABLED_TypedTest, ShouldNotRun) {
  FAIL() << "Unexpected failure: Disabled typed test should not run.";
}

#endif  // GTEST_HAS_TYPED_TEST

// Tests that disabled type-parameterized tests aren't run.

#ifdef GTEST_HAS_TYPED_TEST_P

template <typename T>
class TypedTestP : public Test {
};

TYPED_TEST_CASE_P(TypedTestP);

TYPED_TEST_P(TypedTestP, DISABLED_ShouldNotRun) {
  FAIL() << "Unexpected failure: "
         << "Disabled type-parameterized test should not run.";
}

REGISTER_TYPED_TEST_CASE_P(TypedTestP, DISABLED_ShouldNotRun);

INSTANTIATE_TYPED_TEST_CASE_P(My, TypedTestP, NumericTypes);

template <typename T>
class DISABLED_TypedTestP : public Test {
};

TYPED_TEST_CASE_P(DISABLED_TypedTestP);

TYPED_TEST_P(DISABLED_TypedTestP, ShouldNotRun) {
  FAIL() << "Unexpected failure: "
         << "Disabled type-parameterized test should not run.";
}

REGISTER_TYPED_TEST_CASE_P(DISABLED_TypedTestP, ShouldNotRun);

INSTANTIATE_TYPED_TEST_CASE_P(My, DISABLED_TypedTestP, NumericTypes);

#endif  // GTEST_HAS_TYPED_TEST_P

// Tests that assertion macros evaluate their arguments exactly once.

class SingleEvaluationTest : public Test {
 protected:
  SingleEvaluationTest() {
    p1_ = s1_;
    p2_ = s2_;
    a_ = 0;
    b_ = 0;
  }

  // This helper function is needed by the FailedASSERT_STREQ test
  // below.
  static void CompareAndIncrementCharPtrs() {
    ASSERT_STREQ(p1_++, p2_++);
  }

  // This helper function is needed by the FailedASSERT_NE test below.
  static void CompareAndIncrementInts() {
    ASSERT_NE(a_++, b_++);
  }

  static const char* const s1_;
  static const char* const s2_;
  static const char* p1_;
  static const char* p2_;

  static int a_;
  static int b_;
};

const char* const SingleEvaluationTest::s1_ = "01234";
const char* const SingleEvaluationTest::s2_ = "abcde";
const char* SingleEvaluationTest::p1_;
const char* SingleEvaluationTest::p2_;
int SingleEvaluationTest::a_;
int SingleEvaluationTest::b_;

// Tests that when ASSERT_STREQ fails, it evaluates its arguments
// exactly once.
TEST_F(SingleEvaluationTest, FailedASSERT_STREQ) {
  EXPECT_FATAL_FAILURE(CompareAndIncrementCharPtrs(),
                       "p2_++");
  EXPECT_EQ(s1_ + 1, p1_);
  EXPECT_EQ(s2_ + 1, p2_);
}

// Tests that string assertion arguments are evaluated exactly once.
TEST_F(SingleEvaluationTest, ASSERT_STR) {
  // successful EXPECT_STRNE
  EXPECT_STRNE(p1_++, p2_++);
  EXPECT_EQ(s1_ + 1, p1_);
  EXPECT_EQ(s2_ + 1, p2_);

  // failed EXPECT_STRCASEEQ
  EXPECT_NONFATAL_FAILURE(EXPECT_STRCASEEQ(p1_++, p2_++),
                          "ignoring case");
  EXPECT_EQ(s1_ + 2, p1_);
  EXPECT_EQ(s2_ + 2, p2_);
}

// Tests that when ASSERT_NE fails, it evaluates its arguments exactly
// once.
TEST_F(SingleEvaluationTest, FailedASSERT_NE) {
  EXPECT_FATAL_FAILURE(CompareAndIncrementInts(), "(a_++) != (b_++)");
  EXPECT_EQ(1, a_);
  EXPECT_EQ(1, b_);
}

// Tests that assertion arguments are evaluated exactly once.
TEST_F(SingleEvaluationTest, OtherCases) {
  // successful EXPECT_TRUE
  EXPECT_TRUE(0 == a_++);  // NOLINT
  EXPECT_EQ(1, a_);

  // failed EXPECT_TRUE
  EXPECT_NONFATAL_FAILURE(EXPECT_TRUE(-1 == a_++), "-1 == a_++");
  EXPECT_EQ(2, a_);

  // successful EXPECT_GT
  EXPECT_GT(a_++, b_++);
  EXPECT_EQ(3, a_);
  EXPECT_EQ(1, b_);

  // failed EXPECT_LT
  EXPECT_NONFATAL_FAILURE(EXPECT_LT(a_++, b_++), "(a_++) < (b_++)");
  EXPECT_EQ(4, a_);
  EXPECT_EQ(2, b_);

  // successful ASSERT_TRUE
  ASSERT_TRUE(0 < a_++);  // NOLINT
  EXPECT_EQ(5, a_);

  // successful ASSERT_GT
  ASSERT_GT(a_++, b_++);
  EXPECT_EQ(6, a_);
  EXPECT_EQ(3, b_);
}

#if GTEST_HAS_EXCEPTIONS

void ThrowAnInteger() {
  throw 1;
}

// Tests that assertion arguments are evaluated exactly once.
TEST_F(SingleEvaluationTest, ExceptionTests) {
  // successful EXPECT_THROW
  EXPECT_THROW({  // NOLINT
    a_++;
    ThrowAnInteger();
  }, int);
  EXPECT_EQ(1, a_);

  // failed EXPECT_THROW, throws different
  EXPECT_NONFATAL_FAILURE(EXPECT_THROW({  // NOLINT
    a_++;
    ThrowAnInteger();
  }, bool), "throws a different type");
  EXPECT_EQ(2, a_);

  // failed EXPECT_THROW, throws nothing
  EXPECT_NONFATAL_FAILURE(EXPECT_THROW(a_++, bool), "throws nothing");
  EXPECT_EQ(3, a_);

  // successful EXPECT_NO_THROW
  EXPECT_NO_THROW(a_++);
  EXPECT_EQ(4, a_);

  // failed EXPECT_NO_THROW
  EXPECT_NONFATAL_FAILURE(EXPECT_NO_THROW({  // NOLINT
    a_++;
    ThrowAnInteger();
  }), "it throws");
  EXPECT_EQ(5, a_);

  // successful EXPECT_ANY_THROW
  EXPECT_ANY_THROW({  // NOLINT
    a_++;
    ThrowAnInteger();
  });
  EXPECT_EQ(6, a_);

  // failed EXPECT_ANY_THROW
  EXPECT_NONFATAL_FAILURE(EXPECT_ANY_THROW(a_++), "it doesn't");
  EXPECT_EQ(7, a_);
}

#endif  // GTEST_HAS_EXCEPTIONS

// Tests {ASSERT|EXPECT}_NO_FATAL_FAILURE.
class NoFatalFailureTest : public Test {
 protected:
  void Succeeds() {}
  void FailsNonFatal() {
    ADD_FAILURE() << "some non-fatal failure";
  }
  void Fails() {
    FAIL() << "some fatal failure";
  }

  void DoAssertNoFatalFailureOnFails() {
    ASSERT_NO_FATAL_FAILURE(Fails());
    ADD_FAILURE() << "shold not reach here.";
  }

  void DoExpectNoFatalFailureOnFails() {
    EXPECT_NO_FATAL_FAILURE(Fails());
    ADD_FAILURE() << "other failure";
  }
};

TEST_F(NoFatalFailureTest, NoFailure) {
  EXPECT_NO_FATAL_FAILURE(Succeeds());
  ASSERT_NO_FATAL_FAILURE(Succeeds());
}

TEST_F(NoFatalFailureTest, NonFatalIsNoFailure) {
  EXPECT_NONFATAL_FAILURE(
      EXPECT_NO_FATAL_FAILURE(FailsNonFatal()),
      "some non-fatal failure");
  EXPECT_NONFATAL_FAILURE(
      ASSERT_NO_FATAL_FAILURE(FailsNonFatal()),
      "some non-fatal failure");
}

TEST_F(NoFatalFailureTest, AssertNoFatalFailureOnFatalFailure) {
  TestPartResultArray gtest_failures;
  {
    ScopedFakeTestPartResultReporter gtest_reporter(&gtest_failures);
    DoAssertNoFatalFailureOnFails();
  }
  ASSERT_EQ(2, gtest_failures.size());
  EXPECT_EQ(testing::TPRT_FATAL_FAILURE,
            gtest_failures.GetTestPartResult(0).type());
  EXPECT_EQ(testing::TPRT_FATAL_FAILURE,
            gtest_failures.GetTestPartResult(1).type());
  EXPECT_PRED_FORMAT2(testing::IsSubstring, "some fatal failure",
                      gtest_failures.GetTestPartResult(0).message());
  EXPECT_PRED_FORMAT2(testing::IsSubstring, "it does",
                      gtest_failures.GetTestPartResult(1).message());
}

TEST_F(NoFatalFailureTest, ExpectNoFatalFailureOnFatalFailure) {
  TestPartResultArray gtest_failures;
  {
    ScopedFakeTestPartResultReporter gtest_reporter(&gtest_failures);
    DoExpectNoFatalFailureOnFails();
  }
  ASSERT_EQ(3, gtest_failures.size());
  EXPECT_EQ(testing::TPRT_FATAL_FAILURE,
            gtest_failures.GetTestPartResult(0).type());
  EXPECT_EQ(testing::TPRT_NONFATAL_FAILURE,
            gtest_failures.GetTestPartResult(1).type());
  EXPECT_EQ(testing::TPRT_NONFATAL_FAILURE,
            gtest_failures.GetTestPartResult(2).type());
  EXPECT_PRED_FORMAT2(testing::IsSubstring, "some fatal failure",
                      gtest_failures.GetTestPartResult(0).message());
  EXPECT_PRED_FORMAT2(testing::IsSubstring, "it does",
                      gtest_failures.GetTestPartResult(1).message());
  EXPECT_PRED_FORMAT2(testing::IsSubstring, "other failure",
                      gtest_failures.GetTestPartResult(2).message());
}

TEST_F(NoFatalFailureTest, MessageIsStreamable) {
  TestPartResultArray gtest_failures;
  {
    ScopedFakeTestPartResultReporter gtest_reporter(&gtest_failures);
    EXPECT_NO_FATAL_FAILURE(FAIL() << "foo") << "my message";
  }
  ASSERT_EQ(2, gtest_failures.size());
  EXPECT_EQ(testing::TPRT_NONFATAL_FAILURE,
            gtest_failures.GetTestPartResult(0).type());
  EXPECT_EQ(testing::TPRT_NONFATAL_FAILURE,
            gtest_failures.GetTestPartResult(1).type());
  EXPECT_PRED_FORMAT2(testing::IsSubstring, "foo",
                      gtest_failures.GetTestPartResult(0).message());
  EXPECT_PRED_FORMAT2(testing::IsSubstring, "my message",
                      gtest_failures.GetTestPartResult(1).message());
}

// Tests non-string assertions.

// Tests EqFailure(), used for implementing *EQ* assertions.
TEST(AssertionTest, EqFailure) {
  const String foo_val("5"), bar_val("6");
  const String msg1(
      EqFailure("foo", "bar", foo_val, bar_val, false)
      .failure_message());
  EXPECT_STREQ(
      "Value of: bar\n"
      "  Actual: 6\n"
      "Expected: foo\n"
      "Which is: 5",
      msg1.c_str());

  const String msg2(
      EqFailure("foo", "6", foo_val, bar_val, false)
      .failure_message());
  EXPECT_STREQ(
      "Value of: 6\n"
      "Expected: foo\n"
      "Which is: 5",
      msg2.c_str());

  const String msg3(
      EqFailure("5", "bar", foo_val, bar_val, false)
      .failure_message());
  EXPECT_STREQ(
      "Value of: bar\n"
      "  Actual: 6\n"
      "Expected: 5",
      msg3.c_str());

  const String msg4(
      EqFailure("5", "6", foo_val, bar_val, false).failure_message());
  EXPECT_STREQ(
      "Value of: 6\n"
      "Expected: 5",
      msg4.c_str());

  const String msg5(
      EqFailure("foo", "bar",
                String("\"x\""), String("\"y\""),
                true).failure_message());
  EXPECT_STREQ(
      "Value of: bar\n"
      "  Actual: \"y\"\n"
      "Expected: foo (ignoring case)\n"
      "Which is: \"x\"",
      msg5.c_str());
}

// Tests AppendUserMessage(), used for implementing the *EQ* macros.
TEST(AssertionTest, AppendUserMessage) {
  const String foo("foo");

  Message msg;
  EXPECT_STREQ("foo",
               AppendUserMessage(foo, msg).c_str());

  msg << "bar";
  EXPECT_STREQ("foo\nbar",
               AppendUserMessage(foo, msg).c_str());
}

// Tests ASSERT_TRUE.
TEST(AssertionTest, ASSERT_TRUE) {
  ASSERT_TRUE(2 > 1);  // NOLINT
  EXPECT_FATAL_FAILURE(ASSERT_TRUE(2 < 1),
                       "2 < 1");
}

// Tests ASSERT_FALSE.
TEST(AssertionTest, ASSERT_FALSE) {
  ASSERT_FALSE(2 < 1);  // NOLINT
  EXPECT_FATAL_FAILURE(ASSERT_FALSE(2 > 1),
                       "Value of: 2 > 1\n"
                       "  Actual: true\n"
                       "Expected: false");
}

// Tests using ASSERT_EQ on double values.  The purpose is to make
// sure that the specialization we did for integer and anonymous enums
// isn't used for double arguments.
TEST(ExpectTest, ASSERT_EQ_Double) {
  // A success.
  ASSERT_EQ(5.6, 5.6);

  // A failure.
  EXPECT_FATAL_FAILURE(ASSERT_EQ(5.1, 5.2),
                       "5.1");
}

// Tests ASSERT_EQ.
TEST(AssertionTest, ASSERT_EQ) {
  ASSERT_EQ(5, 2 + 3);
  EXPECT_FATAL_FAILURE(ASSERT_EQ(5, 2*3),
                       "Value of: 2*3\n"
                       "  Actual: 6\n"
                       "Expected: 5");
}

// Tests ASSERT_EQ(NULL, pointer).
#ifndef GTEST_OS_SYMBIAN
// The NULL-detection template magic fails to compile with
// the Nokia compiler and crashes the ARM compiler, hence
// not testing on Symbian.
TEST(AssertionTest, ASSERT_EQ_NULL) {
  // A success.
  const char* p = NULL;
  ASSERT_EQ(NULL, p);

  // A failure.
  static int n = 0;
  EXPECT_FATAL_FAILURE(ASSERT_EQ(NULL, &n),
                       "Value of: &n\n");
}
#endif  // GTEST_OS_SYMBIAN

// Tests ASSERT_EQ(0, non_pointer).  Since the literal 0 can be
// treated as a null pointer by the compiler, we need to make sure
// that ASSERT_EQ(0, non_pointer) isn't interpreted by Google Test as
// ASSERT_EQ(static_cast<void*>(NULL), non_pointer).
TEST(ExpectTest, ASSERT_EQ_0) {
  int n = 0;

  // A success.
  ASSERT_EQ(0, n);

  // A failure.
  EXPECT_FATAL_FAILURE(ASSERT_EQ(0, 5.6),
                       "Expected: 0");
}

// Tests ASSERT_NE.
TEST(AssertionTest, ASSERT_NE) {
  ASSERT_NE(6, 7);
  EXPECT_FATAL_FAILURE(ASSERT_NE('a', 'a'),
                       "Expected: ('a') != ('a'), "
                       "actual: 'a' (97, 0x61) vs 'a' (97, 0x61)");
}

// Tests ASSERT_LE.
TEST(AssertionTest, ASSERT_LE) {
  ASSERT_LE(2, 3);
  ASSERT_LE(2, 2);
  EXPECT_FATAL_FAILURE(ASSERT_LE(2, 0),
                       "Expected: (2) <= (0), actual: 2 vs 0");
}

// Tests ASSERT_LT.
TEST(AssertionTest, ASSERT_LT) {
  ASSERT_LT(2, 3);
  EXPECT_FATAL_FAILURE(ASSERT_LT(2, 2),
                       "Expected: (2) < (2), actual: 2 vs 2");
}

// Tests ASSERT_GE.
TEST(AssertionTest, ASSERT_GE) {
  ASSERT_GE(2, 1);
  ASSERT_GE(2, 2);
  EXPECT_FATAL_FAILURE(ASSERT_GE(2, 3),
                       "Expected: (2) >= (3), actual: 2 vs 3");
}

// Tests ASSERT_GT.
TEST(AssertionTest, ASSERT_GT) {
  ASSERT_GT(2, 1);
  EXPECT_FATAL_FAILURE(ASSERT_GT(2, 2),
                       "Expected: (2) > (2), actual: 2 vs 2");
}

#if GTEST_HAS_EXCEPTIONS

// Tests ASSERT_THROW.
TEST(AssertionTest, ASSERT_THROW) {
  ASSERT_THROW(ThrowAnInteger(), int);
  EXPECT_FATAL_FAILURE(ASSERT_THROW(ThrowAnInteger(), bool),
                       "Expected: ThrowAnInteger() throws an exception of type"\
                       " bool.\n  Actual: it throws a different type.");
  EXPECT_FATAL_FAILURE(ASSERT_THROW(1, bool),
                       "Expected: 1 throws an exception of type bool.\n"\
                       "  Actual: it throws nothing.");
}

// Tests ASSERT_NO_THROW.
TEST(AssertionTest, ASSERT_NO_THROW) {
  ASSERT_NO_THROW(1);
  EXPECT_FATAL_FAILURE(ASSERT_NO_THROW(ThrowAnInteger()),
                       "Expected: ThrowAnInteger() doesn't throw an exception."\
                       "\n  Actual: it throws.");
}

// Tests ASSERT_ANY_THROW.
TEST(AssertionTest, ASSERT_ANY_THROW) {
  ASSERT_ANY_THROW(ThrowAnInteger());
  EXPECT_FATAL_FAILURE(ASSERT_ANY_THROW(1),
                       "Expected: 1 throws an exception.\n  Actual: it "\
                       "doesn't.");
}

#endif  // GTEST_HAS_EXCEPTIONS

// Makes sure we deal with the precedence of <<.  This test should
// compile.
TEST(AssertionTest, AssertPrecedence) {
  ASSERT_EQ(1 < 2, true);
  ASSERT_EQ(true && false, false);
}

// A subroutine used by the following test.
void TestEq1(int x) {
  ASSERT_EQ(1, x);
}

// Tests calling a test subroutine that's not part of a fixture.
TEST(AssertionTest, NonFixtureSubroutine) {
  EXPECT_FATAL_FAILURE(TestEq1(2),
                       "Value of: x");
}

// An uncopyable class.
class Uncopyable {
 public:
  explicit Uncopyable(int value) : value_(value) {}

  int value() const { return value_; }
  bool operator==(const Uncopyable& rhs) const {
    return value() == rhs.value();
  }
 private:
  // This constructor deliberately has no implementation, as we don't
  // want this class to be copyable.
  Uncopyable(const Uncopyable&);  // NOLINT

  int value_;
};

::std::ostream& operator<<(::std::ostream& os, const Uncopyable& value) {
  return os << value.value();
}


bool IsPositiveUncopyable(const Uncopyable& x) {
  return x.value() > 0;
}

// A subroutine used by the following test.
void TestAssertNonPositive() {
  Uncopyable y(-1);
  ASSERT_PRED1(IsPositiveUncopyable, y);
}
// A subroutine used by the following test.
void TestAssertEqualsUncopyable() {
  Uncopyable x(5);
  Uncopyable y(-1);
  ASSERT_EQ(x, y);
}

// Tests that uncopyable objects can be used in assertions.
TEST(AssertionTest, AssertWorksWithUncopyableObject) {
  Uncopyable x(5);
  ASSERT_PRED1(IsPositiveUncopyable, x);
  ASSERT_EQ(x, x);
  EXPECT_FATAL_FAILURE(TestAssertNonPositive(),
    "IsPositiveUncopyable(y) evaluates to false, where\ny evaluates to -1");
  EXPECT_FATAL_FAILURE(TestAssertEqualsUncopyable(),
    "Value of: y\n  Actual: -1\nExpected: x\nWhich is: 5");
}

// Tests that uncopyable objects can be used in expects.
TEST(AssertionTest, ExpectWorksWithUncopyableObject) {
  Uncopyable x(5);
  EXPECT_PRED1(IsPositiveUncopyable, x);
  Uncopyable y(-1);
  EXPECT_NONFATAL_FAILURE(EXPECT_PRED1(IsPositiveUncopyable, y),
    "IsPositiveUncopyable(y) evaluates to false, where\ny evaluates to -1");
  EXPECT_EQ(x, x);
  EXPECT_NONFATAL_FAILURE(EXPECT_EQ(x, y),
    "Value of: y\n  Actual: -1\nExpected: x\nWhich is: 5");
}


// The version of gcc used in XCode 2.2 has a bug and doesn't allow
// anonymous enums in assertions.  Therefore the following test is
// done only on Linux and Windows.
#if defined(GTEST_OS_LINUX) || defined(GTEST_OS_WINDOWS)

// Tests using assertions with anonymous enums.
enum {
  CASE_A = -1,
#ifdef GTEST_OS_LINUX
  // We want to test the case where the size of the anonymous enum is
  // larger than sizeof(int), to make sure our implementation of the
  // assertions doesn't truncate the enums.  However, MSVC
  // (incorrectly) doesn't allow an enum value to exceed the range of
  // an int, so this has to be conditionally compiled.
  //
  // On Linux, CASE_B and CASE_A have the same value when truncated to
  // int size.  We want to test whether this will confuse the
  // assertions.
  CASE_B = testing::internal::kMaxBiggestInt,
#else
  CASE_B = INT_MAX,
#endif  // GTEST_OS_LINUX
};

TEST(AssertionTest, AnonymousEnum) {
#ifdef GTEST_OS_LINUX
  EXPECT_EQ(static_cast<int>(CASE_A), static_cast<int>(CASE_B));
#endif  // GTEST_OS_LINUX

  EXPECT_EQ(CASE_A, CASE_A);
  EXPECT_NE(CASE_A, CASE_B);
  EXPECT_LT(CASE_A, CASE_B);
  EXPECT_LE(CASE_A, CASE_B);
  EXPECT_GT(CASE_B, CASE_A);
  EXPECT_GE(CASE_A, CASE_A);
  EXPECT_NONFATAL_FAILURE(EXPECT_GE(CASE_A, CASE_B),
                          "(CASE_A) >= (CASE_B)");

  ASSERT_EQ(CASE_A, CASE_A);
  ASSERT_NE(CASE_A, CASE_B);
  ASSERT_LT(CASE_A, CASE_B);
  ASSERT_LE(CASE_A, CASE_B);
  ASSERT_GT(CASE_B, CASE_A);
  ASSERT_GE(CASE_A, CASE_A);
  EXPECT_FATAL_FAILURE(ASSERT_EQ(CASE_A, CASE_B),
                       "Value of: CASE_B");
}

#endif  // defined(GTEST_OS_LINUX) || defined(GTEST_OS_WINDOWS)

#if defined(GTEST_OS_WINDOWS)

static HRESULT UnexpectedHRESULTFailure() {
  return E_UNEXPECTED;
}

static HRESULT OkHRESULTSuccess() {
  return S_OK;
}

static HRESULT FalseHRESULTSuccess() {
  return S_FALSE;
}

// HRESULT assertion tests test both zero and non-zero
// success codes as well as failure message for each.
//
// Windows CE doesn't support message texts.
TEST(HRESULTAssertionTest, EXPECT_HRESULT_SUCCEEDED) {
  EXPECT_HRESULT_SUCCEEDED(S_OK);
  EXPECT_HRESULT_SUCCEEDED(S_FALSE);

  EXPECT_NONFATAL_FAILURE(EXPECT_HRESULT_SUCCEEDED(UnexpectedHRESULTFailure()),
    "Expected: (UnexpectedHRESULTFailure()) succeeds.\n"
    "  Actual: 0x8000FFFF");
}

TEST(HRESULTAssertionTest, ASSERT_HRESULT_SUCCEEDED) {
  ASSERT_HRESULT_SUCCEEDED(S_OK);
  ASSERT_HRESULT_SUCCEEDED(S_FALSE);

  EXPECT_FATAL_FAILURE(ASSERT_HRESULT_SUCCEEDED(UnexpectedHRESULTFailure()),
    "Expected: (UnexpectedHRESULTFailure()) succeeds.\n"
    "  Actual: 0x8000FFFF");
}

TEST(HRESULTAssertionTest, EXPECT_HRESULT_FAILED) {
  EXPECT_HRESULT_FAILED(E_UNEXPECTED);

  EXPECT_NONFATAL_FAILURE(EXPECT_HRESULT_FAILED(OkHRESULTSuccess()),
    "Expected: (OkHRESULTSuccess()) fails.\n"
    "  Actual: 0x00000000");
  EXPECT_NONFATAL_FAILURE(EXPECT_HRESULT_FAILED(FalseHRESULTSuccess()),
    "Expected: (FalseHRESULTSuccess()) fails.\n"
    "  Actual: 0x00000001");
}

TEST(HRESULTAssertionTest, ASSERT_HRESULT_FAILED) {
  ASSERT_HRESULT_FAILED(E_UNEXPECTED);

  EXPECT_FATAL_FAILURE(ASSERT_HRESULT_FAILED(OkHRESULTSuccess()),
    "Expected: (OkHRESULTSuccess()) fails.\n"
    "  Actual: 0x00000000");
  EXPECT_FATAL_FAILURE(ASSERT_HRESULT_FAILED(FalseHRESULTSuccess()),
    "Expected: (FalseHRESULTSuccess()) fails.\n"
    "  Actual: 0x00000001");
}

// Tests that streaming to the HRESULT macros works.
TEST(HRESULTAssertionTest, Streaming) {
  EXPECT_HRESULT_SUCCEEDED(S_OK) << "unexpected failure";
  ASSERT_HRESULT_SUCCEEDED(S_OK) << "unexpected failure";
  EXPECT_HRESULT_FAILED(E_UNEXPECTED) << "unexpected failure";
  ASSERT_HRESULT_FAILED(E_UNEXPECTED) << "unexpected failure";

  EXPECT_NONFATAL_FAILURE(
      EXPECT_HRESULT_SUCCEEDED(E_UNEXPECTED) << "expected failure",
      "expected failure");

  EXPECT_FATAL_FAILURE(
      ASSERT_HRESULT_SUCCEEDED(E_UNEXPECTED) << "expected failure",
      "expected failure");

  EXPECT_NONFATAL_FAILURE(
      EXPECT_HRESULT_FAILED(S_OK) << "expected failure",
      "expected failure");

  EXPECT_FATAL_FAILURE(
      ASSERT_HRESULT_FAILED(S_OK) << "expected failure",
      "expected failure");
}

#endif  // defined(GTEST_OS_WINDOWS)

// Tests that the assertion macros behave like single statements.
TEST(AssertionSyntaxTest, BasicAssertionsBehavesLikeSingleStatement) {
  if (false)
    ASSERT_TRUE(false) << "This should never be executed; "
                          "It's a compilation test only.";

  if (true)
    EXPECT_FALSE(false);
  else
    ;

  if (false)
    ASSERT_LT(1, 3);

  if (false)
    ;
  else
    EXPECT_GT(3, 2) << "";
}

#if GTEST_HAS_EXCEPTIONS
TEST(AssertionSyntaxTest, ExceptionAssertionsBehavesLikeSingleStatement) {
  if (false)
    EXPECT_THROW(1, bool);

  if (true)
    EXPECT_THROW(ThrowAnInteger(), int);
  else
    ;

  if (false)
    EXPECT_NO_THROW(ThrowAnInteger());

  if (true)
    EXPECT_NO_THROW(1);
  else
    ;

  if (false)
    EXPECT_ANY_THROW(1);

  if (true)
    EXPECT_ANY_THROW(ThrowAnInteger());
  else
    ;
}
#endif  // GTEST_HAS_EXCEPTIONS

TEST(AssertionSyntaxTest, NoFatalFailureAssertionsBehavesLikeSingleStatement) {
  if (false)
    EXPECT_NO_FATAL_FAILURE(FAIL()) << "This should never be executed. "
                                    << "It's a compilation test only.";
  else
    ;

  if (false)
    ASSERT_NO_FATAL_FAILURE(FAIL()) << "";
  else
    ;

  if (true)
    EXPECT_NO_FATAL_FAILURE(SUCCEED());
  else
    ;

  if (false)
    ;
  else
    ASSERT_NO_FATAL_FAILURE(SUCCEED());
}

// Tests that the assertion macros work well with switch statements.
TEST(AssertionSyntaxTest, WorksWithSwitch) {
  switch (0) {
    case 1:
      break;
    default:
      ASSERT_TRUE(true);
  }

  switch (0)
    case 0:
      EXPECT_FALSE(false) << "EXPECT_FALSE failed in switch case";

  // Binary assertions are implemented using a different code path
  // than the Boolean assertions.  Hence we test them separately.
  switch (0) {
    case 1:
    default:
      ASSERT_EQ(1, 1) << "ASSERT_EQ failed in default switch handler";
  }

  switch (0)
    case 0:
      EXPECT_NE(1, 2);
}

#if GTEST_HAS_EXCEPTIONS

void ThrowAString() {
    throw "String";
}

// Test that the exception assertion macros compile and work with const
// type qualifier.
TEST(AssertionSyntaxTest, WorksWithConst) {
    ASSERT_THROW(ThrowAString(), const char*);

    EXPECT_THROW(ThrowAString(), const char*);
}

#endif  // GTEST_HAS_EXCEPTIONS

}  // namespace

// Returns the number of successful parts in the current test.
static size_t GetSuccessfulPartCount() {
  return UnitTest::GetInstance()->impl()->current_test_result()->
    successful_part_count();
}

namespace testing {

// Tests that Google Test tracks SUCCEED*.
TEST(SuccessfulAssertionTest, SUCCEED) {
  SUCCEED();
  SUCCEED() << "OK";
  EXPECT_EQ(2u, GetSuccessfulPartCount());
}

// Tests that Google Test doesn't track successful EXPECT_*.
TEST(SuccessfulAssertionTest, EXPECT) {
  EXPECT_TRUE(true);
  EXPECT_EQ(0u, GetSuccessfulPartCount());
}

// Tests that Google Test doesn't track successful EXPECT_STR*.
TEST(SuccessfulAssertionTest, EXPECT_STR) {
  EXPECT_STREQ("", "");
  EXPECT_EQ(0u, GetSuccessfulPartCount());
}

// Tests that Google Test doesn't track successful ASSERT_*.
TEST(SuccessfulAssertionTest, ASSERT) {
  ASSERT_TRUE(true);
  EXPECT_EQ(0u, GetSuccessfulPartCount());
}

// Tests that Google Test doesn't track successful ASSERT_STR*.
TEST(SuccessfulAssertionTest, ASSERT_STR) {
  ASSERT_STREQ("", "");
  EXPECT_EQ(0u, GetSuccessfulPartCount());
}

}  // namespace testing

namespace {

// Tests EXPECT_TRUE.
TEST(ExpectTest, EXPECT_TRUE) {
  EXPECT_TRUE(2 > 1);  // NOLINT
  EXPECT_NONFATAL_FAILURE(EXPECT_TRUE(2 < 1),
                          "Value of: 2 < 1\n"
                          "  Actual: false\n"
                          "Expected: true");
  EXPECT_NONFATAL_FAILURE(EXPECT_TRUE(2 > 3),
                          "2 > 3");
}

// Tests EXPECT_FALSE.
TEST(ExpectTest, EXPECT_FALSE) {
  EXPECT_FALSE(2 < 1);  // NOLINT
  EXPECT_NONFATAL_FAILURE(EXPECT_FALSE(2 > 1),
                          "Value of: 2 > 1\n"
                          "  Actual: true\n"
                          "Expected: false");
  EXPECT_NONFATAL_FAILURE(EXPECT_FALSE(2 < 3),
                          "2 < 3");
}

// Tests EXPECT_EQ.
TEST(ExpectTest, EXPECT_EQ) {
  EXPECT_EQ(5, 2 + 3);
  EXPECT_NONFATAL_FAILURE(EXPECT_EQ(5, 2*3),
                          "Value of: 2*3\n"
                          "  Actual: 6\n"
                          "Expected: 5");
  EXPECT_NONFATAL_FAILURE(EXPECT_EQ(5, 2 - 3),
                          "2 - 3");
}

// Tests using EXPECT_EQ on double values.  The purpose is to make
// sure that the specialization we did for integer and anonymous enums
// isn't used for double arguments.
TEST(ExpectTest, EXPECT_EQ_Double) {
  // A success.
  EXPECT_EQ(5.6, 5.6);

  // A failure.
  EXPECT_NONFATAL_FAILURE(EXPECT_EQ(5.1, 5.2),
                          "5.1");
}

#ifndef GTEST_OS_SYMBIAN
// Tests EXPECT_EQ(NULL, pointer).
TEST(ExpectTest, EXPECT_EQ_NULL) {
  // A success.
  const char* p = NULL;
  EXPECT_EQ(NULL, p);

  // A failure.
  int n = 0;
  EXPECT_NONFATAL_FAILURE(EXPECT_EQ(NULL, &n),
                          "Value of: &n\n");
}
#endif  // GTEST_OS_SYMBIAN

// Tests EXPECT_EQ(0, non_pointer).  Since the literal 0 can be
// treated as a null pointer by the compiler, we need to make sure
// that EXPECT_EQ(0, non_pointer) isn't interpreted by Google Test as
// EXPECT_EQ(static_cast<void*>(NULL), non_pointer).
TEST(ExpectTest, EXPECT_EQ_0) {
  int n = 0;

  // A success.
  EXPECT_EQ(0, n);

  // A failure.
  EXPECT_NONFATAL_FAILURE(EXPECT_EQ(0, 5.6),
                          "Expected: 0");
}

// Tests EXPECT_NE.
TEST(ExpectTest, EXPECT_NE) {
  EXPECT_NE(6, 7);

  EXPECT_NONFATAL_FAILURE(EXPECT_NE('a', 'a'),
                          "Expected: ('a') != ('a'), "
                          "actual: 'a' (97, 0x61) vs 'a' (97, 0x61)");
  EXPECT_NONFATAL_FAILURE(EXPECT_NE(2, 2),
                          "2");
  char* const p0 = NULL;
  EXPECT_NONFATAL_FAILURE(EXPECT_NE(p0, p0),
                          "p0");
  // Only way to get the Nokia compiler to compile the cast
  // is to have a separate void* variable first. Putting
  // the two casts on the same line doesn't work, neither does
  // a direct C-style to char*.
  void* pv1 = (void*)0x1234;  // NOLINT
  char* const p1 = reinterpret_cast<char*>(pv1);
  EXPECT_NONFATAL_FAILURE(EXPECT_NE(p1, p1),
                          "p1");
}

// Tests EXPECT_LE.
TEST(ExpectTest, EXPECT_LE) {
  EXPECT_LE(2, 3);
  EXPECT_LE(2, 2);
  EXPECT_NONFATAL_FAILURE(EXPECT_LE(2, 0),
                          "Expected: (2) <= (0), actual: 2 vs 0");
  EXPECT_NONFATAL_FAILURE(EXPECT_LE(1.1, 0.9),
                          "(1.1) <= (0.9)");
}

// Tests EXPECT_LT.
TEST(ExpectTest, EXPECT_LT) {
  EXPECT_LT(2, 3);
  EXPECT_NONFATAL_FAILURE(EXPECT_LT(2, 2),
                          "Expected: (2) < (2), actual: 2 vs 2");
  EXPECT_NONFATAL_FAILURE(EXPECT_LT(2, 1),
                          "(2) < (1)");
}

// Tests EXPECT_GE.
TEST(ExpectTest, EXPECT_GE) {
  EXPECT_GE(2, 1);
  EXPECT_GE(2, 2);
  EXPECT_NONFATAL_FAILURE(EXPECT_GE(2, 3),
                          "Expected: (2) >= (3), actual: 2 vs 3");
  EXPECT_NONFATAL_FAILURE(EXPECT_GE(0.9, 1.1),
                          "(0.9) >= (1.1)");
}

// Tests EXPECT_GT.
TEST(ExpectTest, EXPECT_GT) {
  EXPECT_GT(2, 1);
  EXPECT_NONFATAL_FAILURE(EXPECT_GT(2, 2),
                          "Expected: (2) > (2), actual: 2 vs 2");
  EXPECT_NONFATAL_FAILURE(EXPECT_GT(2, 3),
                          "(2) > (3)");
}

#if GTEST_HAS_EXCEPTIONS

// Tests EXPECT_THROW.
TEST(ExpectTest, EXPECT_THROW) {
  EXPECT_THROW(ThrowAnInteger(), int);
  EXPECT_NONFATAL_FAILURE(EXPECT_THROW(ThrowAnInteger(), bool),
                          "Expected: ThrowAnInteger() throws an exception of "\
                          "type bool.\n  Actual: it throws a different type.");
  EXPECT_NONFATAL_FAILURE(EXPECT_THROW(1, bool),
                          "Expected: 1 throws an exception of type bool.\n"\
                          "  Actual: it throws nothing.");
}

// Tests EXPECT_NO_THROW.
TEST(ExpectTest, EXPECT_NO_THROW) {
  EXPECT_NO_THROW(1);
  EXPECT_NONFATAL_FAILURE(EXPECT_NO_THROW(ThrowAnInteger()),
                          "Expected: ThrowAnInteger() doesn't throw an "\
                          "exception.\n  Actual: it throws.");
}

// Tests EXPECT_ANY_THROW.
TEST(ExpectTest, EXPECT_ANY_THROW) {
  EXPECT_ANY_THROW(ThrowAnInteger());
  EXPECT_NONFATAL_FAILURE(EXPECT_ANY_THROW(1),
                          "Expected: 1 throws an exception.\n  Actual: it "\
                          "doesn't.");
}

#endif  // GTEST_HAS_EXCEPTIONS

// Make sure we deal with the precedence of <<.
TEST(ExpectTest, ExpectPrecedence) {
  EXPECT_EQ(1 < 2, true);
  EXPECT_NONFATAL_FAILURE(EXPECT_EQ(true, true && false),
                          "Value of: true && false");
}


// Tests the StreamableToString() function.

// Tests using StreamableToString() on a scalar.
TEST(StreamableToStringTest, Scalar) {
  EXPECT_STREQ("5", StreamableToString(5).c_str());
}

// Tests using StreamableToString() on a non-char pointer.
TEST(StreamableToStringTest, Pointer) {
  int n = 0;
  int* p = &n;
  EXPECT_STRNE("(null)", StreamableToString(p).c_str());
}

// Tests using StreamableToString() on a NULL non-char pointer.
TEST(StreamableToStringTest, NullPointer) {
  int* p = NULL;
  EXPECT_STREQ("(null)", StreamableToString(p).c_str());
}

// Tests using StreamableToString() on a C string.
TEST(StreamableToStringTest, CString) {
  EXPECT_STREQ("Foo", StreamableToString("Foo").c_str());
}

// Tests using StreamableToString() on a NULL C string.
TEST(StreamableToStringTest, NullCString) {
  char* p = NULL;
  EXPECT_STREQ("(null)", StreamableToString(p).c_str());
}

// Tests using streamable values as assertion messages.

#if GTEST_HAS_STD_STRING
// Tests using std::string as an assertion message.
TEST(StreamableTest, string) {
  static const std::string str(
      "This failure message is a std::string, and is expected.");
  EXPECT_FATAL_FAILURE(FAIL() << str,
                       str.c_str());
}

// Tests that we can output strings containing embedded NULs.
// Limited to Linux because we can only do this with std::string's.
TEST(StreamableTest, stringWithEmbeddedNUL) {
  static const char char_array_with_nul[] =
      "Here's a NUL\0 and some more string";
  static const std::string string_with_nul(char_array_with_nul,
                                           sizeof(char_array_with_nul)
                                           - 1);  // drops the trailing NUL
  EXPECT_FATAL_FAILURE(FAIL() << string_with_nul,
                       "Here's a NUL\\0 and some more string");
}

#endif  // GTEST_HAS_STD_STRING

// Tests that we can output a NUL char.
TEST(StreamableTest, NULChar) {
  EXPECT_FATAL_FAILURE({  // NOLINT
    FAIL() << "A NUL" << '\0' << " and some more string";
  }, "A NUL\\0 and some more string");
}

// Tests using int as an assertion message.
TEST(StreamableTest, int) {
  EXPECT_FATAL_FAILURE(FAIL() << 900913,
                       "900913");
}

// Tests using NULL char pointer as an assertion message.
//
// In MSVC, streaming a NULL char * causes access violation.  Google Test
// implemented a workaround (substituting "(null)" for NULL).  This
// tests whether the workaround works.
TEST(StreamableTest, NullCharPtr) {
  EXPECT_FATAL_FAILURE(FAIL() << static_cast<const char*>(NULL),
                       "(null)");
}

// Tests that basic IO manipulators (endl, ends, and flush) can be
// streamed to testing::Message.
TEST(StreamableTest, BasicIoManip) {
  EXPECT_FATAL_FAILURE({  // NOLINT
    FAIL() << "Line 1." << std::endl
           << "A NUL char " << std::ends << std::flush << " in line 2.";
  }, "Line 1.\nA NUL char \\0 in line 2.");
}

// Tests the macros that haven't been covered so far.

void AddFailureHelper(bool* aborted) {
  *aborted = true;
  ADD_FAILURE() << "Failure";
  *aborted = false;
}

// Tests ADD_FAILURE.
TEST(MacroTest, ADD_FAILURE) {
  bool aborted = true;
  EXPECT_NONFATAL_FAILURE(AddFailureHelper(&aborted),
                          "Failure");
  EXPECT_FALSE(aborted);
}

// Tests FAIL.
TEST(MacroTest, FAIL) {
  EXPECT_FATAL_FAILURE(FAIL(),
                       "Failed");
  EXPECT_FATAL_FAILURE(FAIL() << "Intentional failure.",
                       "Intentional failure.");
}

// Tests SUCCEED
TEST(MacroTest, SUCCEED) {
  SUCCEED();
  SUCCEED() << "Explicit success.";
}


// Tests for EXPECT_EQ() and ASSERT_EQ().
//
// These tests fail *intentionally*, s.t. the failure messages can be
// generated and tested.
//
// We have different tests for different argument types.

// Tests using bool values in {EXPECT|ASSERT}_EQ.
TEST(EqAssertionTest, Bool) {
  EXPECT_EQ(true,  true);
  EXPECT_FATAL_FAILURE(ASSERT_EQ(false, true),
                       "Value of: true");
}

// Tests using int values in {EXPECT|ASSERT}_EQ.
TEST(EqAssertionTest, Int) {
  ASSERT_EQ(32, 32);
  EXPECT_NONFATAL_FAILURE(EXPECT_EQ(32, 33),
                          "33");
}

// Tests using time_t values in {EXPECT|ASSERT}_EQ.
TEST(EqAssertionTest, Time_T) {
  EXPECT_EQ(static_cast<time_t>(0),
            static_cast<time_t>(0));
  EXPECT_FATAL_FAILURE(ASSERT_EQ(static_cast<time_t>(0),
                                 static_cast<time_t>(1234)),
                       "1234");
}

// Tests using char values in {EXPECT|ASSERT}_EQ.
TEST(EqAssertionTest, Char) {
  ASSERT_EQ('z', 'z');
  const char ch = 'b';
  EXPECT_NONFATAL_FAILURE(EXPECT_EQ('\0', ch),
                          "ch");
  EXPECT_NONFATAL_FAILURE(EXPECT_EQ('a', ch),
                          "ch");
}

// Tests using wchar_t values in {EXPECT|ASSERT}_EQ.
TEST(EqAssertionTest, WideChar) {
  EXPECT_EQ(L'b', L'b');

  EXPECT_NONFATAL_FAILURE(EXPECT_EQ(L'\0', L'x'),
                          "Value of: L'x'\n"
                          "  Actual: L'x' (120, 0x78)\n"
                          "Expected: L'\0'\n"
                          "Which is: L'\0' (0, 0x0)");

  static wchar_t wchar;
  wchar = L'b';
  EXPECT_NONFATAL_FAILURE(EXPECT_EQ(L'a', wchar),
                          "wchar");
  wchar = L'\x8119';
  EXPECT_FATAL_FAILURE(ASSERT_EQ(L'\x8120', wchar),
                       "Value of: wchar");
}

#if GTEST_HAS_STD_STRING
// Tests using ::std::string values in {EXPECT|ASSERT}_EQ.
TEST(EqAssertionTest, StdString) {
  // Compares a const char* to an std::string that has identical
  // content.
  ASSERT_EQ("Test", ::std::string("Test"));

  // Compares two identical std::strings.
  static const ::std::string str1("A * in the middle");
  static const ::std::string str2(str1);
  EXPECT_EQ(str1, str2);

  // Compares a const char* to an std::string that has different
  // content
  EXPECT_NONFATAL_FAILURE(EXPECT_EQ("Test", ::std::string("test")),
                          "::std::string(\"test\")");

  // Compares an std::string to a char* that has different content.
  char* const p1 = const_cast<char*>("foo");
  EXPECT_NONFATAL_FAILURE(EXPECT_EQ(::std::string("bar"), p1),
                          "p1");

  // Compares two std::strings that have different contents, one of
  // which having a NUL character in the middle.  This should fail.
  static ::std::string str3(str1);
  str3.at(2) = '\0';
  EXPECT_FATAL_FAILURE(ASSERT_EQ(str1, str3),
                       "Value of: str3\n"
                       "  Actual: \"A \\0 in the middle\"");
}

#endif  // GTEST_HAS_STD_STRING

#if GTEST_HAS_STD_WSTRING

// Tests using ::std::wstring values in {EXPECT|ASSERT}_EQ.
TEST(EqAssertionTest, StdWideString) {
  // Compares an std::wstring to a const wchar_t* that has identical
  // content.
  EXPECT_EQ(::std::wstring(L"Test\x8119"), L"Test\x8119");

  // Compares two identical std::wstrings.
  const ::std::wstring wstr1(L"A * in the middle");
  const ::std::wstring wstr2(wstr1);
  ASSERT_EQ(wstr1, wstr2);

  // Compares an std::wstring to a const wchar_t* that has different
  // content.
  EXPECT_NONFATAL_FAILURE({  // NOLINT
    EXPECT_EQ(::std::wstring(L"Test\x8119"), L"Test\x8120");
  }, "L\"Test\\x8120\"");

  // Compares two std::wstrings that have different contents, one of
  // which having a NUL character in the middle.
  ::std::wstring wstr3(wstr1);
  wstr3.at(2) = L'\0';
  EXPECT_NONFATAL_FAILURE(EXPECT_EQ(wstr1, wstr3),
                          "wstr3");

  // Compares a wchar_t* to an std::wstring that has different
  // content.
  EXPECT_FATAL_FAILURE({  // NOLINT
    ASSERT_EQ(const_cast<wchar_t*>(L"foo"), ::std::wstring(L"bar"));
  }, "");
}

#endif  // GTEST_HAS_STD_WSTRING

#if GTEST_HAS_GLOBAL_STRING
// Tests using ::string values in {EXPECT|ASSERT}_EQ.
TEST(EqAssertionTest, GlobalString) {
  // Compares a const char* to a ::string that has identical content.
  EXPECT_EQ("Test", ::string("Test"));

  // Compares two identical ::strings.
  const ::string str1("A * in the middle");
  const ::string str2(str1);
  ASSERT_EQ(str1, str2);

  // Compares a ::string to a const char* that has different content.
  EXPECT_NONFATAL_FAILURE(EXPECT_EQ(::string("Test"), "test"),
                          "test");

  // Compares two ::strings that have different contents, one of which
  // having a NUL character in the middle.
  ::string str3(str1);
  str3.at(2) = '\0';
  EXPECT_NONFATAL_FAILURE(EXPECT_EQ(str1, str3),
                          "str3");

  // Compares a ::string to a char* that has different content.
  EXPECT_FATAL_FAILURE({  // NOLINT
    ASSERT_EQ(::string("bar"), const_cast<char*>("foo"));
  }, "");
}

#endif  // GTEST_HAS_GLOBAL_STRING

#if GTEST_HAS_GLOBAL_WSTRING

// Tests using ::wstring values in {EXPECT|ASSERT}_EQ.
TEST(EqAssertionTest, GlobalWideString) {
  // Compares a const wchar_t* to a ::wstring that has identical content.
  ASSERT_EQ(L"Test\x8119", ::wstring(L"Test\x8119"));

  // Compares two identical ::wstrings.
  static const ::wstring wstr1(L"A * in the middle");
  static const ::wstring wstr2(wstr1);
  EXPECT_EQ(wstr1, wstr2);

  // Compares a const wchar_t* to a ::wstring that has different
  // content.
  EXPECT_NONFATAL_FAILURE({  // NOLINT
    EXPECT_EQ(L"Test\x8120", ::wstring(L"Test\x8119"));
  }, "Test\\x8119");

  // Compares a wchar_t* to a ::wstring that has different content.
  wchar_t* const p1 = const_cast<wchar_t*>(L"foo");
  EXPECT_NONFATAL_FAILURE(EXPECT_EQ(p1, ::wstring(L"bar")),
                          "bar");

  // Compares two ::wstrings that have different contents, one of which
  // having a NUL character in the middle.
  static ::wstring wstr3;
  wstr3 = wstr1;
  wstr3.at(2) = L'\0';
  EXPECT_FATAL_FAILURE(ASSERT_EQ(wstr1, wstr3),
                       "wstr3");
}

#endif  // GTEST_HAS_GLOBAL_WSTRING

// Tests using char pointers in {EXPECT|ASSERT}_EQ.
TEST(EqAssertionTest, CharPointer) {
  char* const p0 = NULL;
  // Only way to get the Nokia compiler to compile the cast
  // is to have a separate void* variable first. Putting
  // the two casts on the same line doesn't work, neither does
  // a direct C-style to char*.
  void* pv1 = (void*)0x1234;  // NOLINT
  void* pv2 = (void*)0xABC0;  // NOLINT
  char* const p1 = reinterpret_cast<char*>(pv1);
  char* const p2 = reinterpret_cast<char*>(pv2);
  ASSERT_EQ(p1, p1);

  EXPECT_NONFATAL_FAILURE(EXPECT_EQ(p0, p2),
                          "Value of: p2");
  EXPECT_NONFATAL_FAILURE(EXPECT_EQ(p1, p2),
                          "p2");
  EXPECT_FATAL_FAILURE(ASSERT_EQ(reinterpret_cast<char*>(0x1234),
                                 reinterpret_cast<char*>(0xABC0)),
                       "ABC0");
}

// Tests using wchar_t pointers in {EXPECT|ASSERT}_EQ.
TEST(EqAssertionTest, WideCharPointer) {
  wchar_t* const p0 = NULL;
  // Only way to get the Nokia compiler to compile the cast
  // is to have a separate void* variable first. Putting
  // the two casts on the same line doesn't work, neither does
  // a direct C-style to char*.
  void* pv1 = (void*)0x1234;  // NOLINT
  void* pv2 = (void*)0xABC0;  // NOLINT
  wchar_t* const p1 = reinterpret_cast<wchar_t*>(pv1);
  wchar_t* const p2 = reinterpret_cast<wchar_t*>(pv2);
  EXPECT_EQ(p0, p0);

  EXPECT_NONFATAL_FAILURE(EXPECT_EQ(p0, p2),
                          "Value of: p2");
  EXPECT_NONFATAL_FAILURE(EXPECT_EQ(p1, p2),
                          "p2");
  void* pv3 = (void*)0x1234;  // NOLINT
  void* pv4 = (void*)0xABC0;  // NOLINT
  const wchar_t* p3 = reinterpret_cast<const wchar_t*>(pv3);
  const wchar_t* p4 = reinterpret_cast<const wchar_t*>(pv4);
  EXPECT_NONFATAL_FAILURE(EXPECT_EQ(p3, p4),
                          "p4");
}

// Tests using other types of pointers in {EXPECT|ASSERT}_EQ.
TEST(EqAssertionTest, OtherPointer) {
  ASSERT_EQ(static_cast<const int*>(NULL),
            static_cast<const int*>(NULL));
  EXPECT_FATAL_FAILURE(ASSERT_EQ(static_cast<const int*>(NULL),
                                 reinterpret_cast<const int*>(0x1234)),
                       "0x1234");
}

// Tests the FRIEND_TEST macro.

// This class has a private member we want to test.  We will test it
// both in a TEST and in a TEST_F.
class Foo {
 public:
  Foo() {}

 private:
  int Bar() const { return 1; }

  // Declares the friend tests that can access the private member
  // Bar().
  FRIEND_TEST(FRIEND_TEST_Test, TEST);
  FRIEND_TEST(FRIEND_TEST_Test2, TEST_F);
};

// Tests that the FRIEND_TEST declaration allows a TEST to access a
// class's private members.  This should compile.
TEST(FRIEND_TEST_Test, TEST) {
  ASSERT_EQ(1, Foo().Bar());
}

// The fixture needed to test using FRIEND_TEST with TEST_F.
class FRIEND_TEST_Test2 : public Test {
 protected:
  Foo foo;
};

// Tests that the FRIEND_TEST declaration allows a TEST_F to access a
// class's private members.  This should compile.
TEST_F(FRIEND_TEST_Test2, TEST_F) {
  ASSERT_EQ(1, foo.Bar());
}

// Tests the life cycle of Test objects.

// The test fixture for testing the life cycle of Test objects.
//
// This class counts the number of live test objects that uses this
// fixture.
class TestLifeCycleTest : public Test {
 protected:
  // Constructor.  Increments the number of test objects that uses
  // this fixture.
  TestLifeCycleTest() { count_++; }

  // Destructor.  Decrements the number of test objects that uses this
  // fixture.
  ~TestLifeCycleTest() { count_--; }

  // Returns the number of live test objects that uses this fixture.
  int count() const { return count_; }

 private:
  static int count_;
};

int TestLifeCycleTest::count_ = 0;

// Tests the life cycle of test objects.
TEST_F(TestLifeCycleTest, Test1) {
  // There should be only one test object in this test case that's
  // currently alive.
  ASSERT_EQ(1, count());
}

// Tests the life cycle of test objects.
TEST_F(TestLifeCycleTest, Test2) {
  // After Test1 is done and Test2 is started, there should still be
  // only one live test object, as the object for Test1 should've been
  // deleted.
  ASSERT_EQ(1, count());
}

}  // namespace

// Tests streaming a user type whose definition and operator << are
// both in the global namespace.
class Base {
 public:
  explicit Base(int x) : x_(x) {}
  int x() const { return x_; }
 private:
  int x_;
};
std::ostream& operator<<(std::ostream& os,
                         const Base& val) {
  return os << val.x();
}
std::ostream& operator<<(std::ostream& os,
                         const Base* pointer) {
  return os << "(" << pointer->x() << ")";
}

TEST(MessageTest, CanStreamUserTypeInGlobalNameSpace) {
  Message msg;
  Base a(1);

  msg << a << &a;  // Uses ::operator<<.
  EXPECT_STREQ("1(1)", msg.GetString().c_str());
}

// Tests streaming a user type whose definition and operator<< are
// both in an unnamed namespace.
namespace {
class MyTypeInUnnamedNameSpace : public Base {
 public:
  explicit MyTypeInUnnamedNameSpace(int x): Base(x) {}
};
std::ostream& operator<<(std::ostream& os,
                         const MyTypeInUnnamedNameSpace& val) {
  return os << val.x();
}
std::ostream& operator<<(std::ostream& os,
                         const MyTypeInUnnamedNameSpace* pointer) {
  return os << "(" << pointer->x() << ")";
}
}  // namespace

TEST(MessageTest, CanStreamUserTypeInUnnamedNameSpace) {
  Message msg;
  MyTypeInUnnamedNameSpace a(1);

  msg << a << &a;  // Uses <unnamed_namespace>::operator<<.
  EXPECT_STREQ("1(1)", msg.GetString().c_str());
}

// Tests streaming a user type whose definition and operator<< are
// both in a user namespace.
namespace namespace1 {
class MyTypeInNameSpace1 : public Base {
 public:
  explicit MyTypeInNameSpace1(int x): Base(x) {}
};
std::ostream& operator<<(std::ostream& os,
                         const MyTypeInNameSpace1& val) {
  return os << val.x();
}
std::ostream& operator<<(std::ostream& os,
                         const MyTypeInNameSpace1* pointer) {
  return os << "(" << pointer->x() << ")";
}
}  // namespace namespace1

TEST(MessageTest, CanStreamUserTypeInUserNameSpace) {
  Message msg;
  namespace1::MyTypeInNameSpace1 a(1);

  msg << a << &a;  // Uses namespace1::operator<<.
  EXPECT_STREQ("1(1)", msg.GetString().c_str());
}

// Tests streaming a user type whose definition is in a user namespace
// but whose operator<< is in the global namespace.
namespace namespace2 {
class MyTypeInNameSpace2 : public ::Base {
 public:
  explicit MyTypeInNameSpace2(int x): Base(x) {}
};
}  // namespace namespace2
std::ostream& operator<<(std::ostream& os,
                         const namespace2::MyTypeInNameSpace2& val) {
  return os << val.x();
}
std::ostream& operator<<(std::ostream& os,
                         const namespace2::MyTypeInNameSpace2* pointer) {
  return os << "(" << pointer->x() << ")";
}

TEST(MessageTest, CanStreamUserTypeInUserNameSpaceWithStreamOperatorInGlobal) {
  Message msg;
  namespace2::MyTypeInNameSpace2 a(1);

  msg << a << &a;  // Uses ::operator<<.
  EXPECT_STREQ("1(1)", msg.GetString().c_str());
}

// Tests streaming NULL pointers to testing::Message.
TEST(MessageTest, NullPointers) {
  Message msg;
  char* const p1 = NULL;
  unsigned char* const p2 = NULL;
  int* p3 = NULL;
  double* p4 = NULL;
  bool* p5 = NULL;
  Message* p6 = NULL;

  msg << p1 << p2 << p3 << p4 << p5 << p6;
  ASSERT_STREQ("(null)(null)(null)(null)(null)(null)",
               msg.GetString().c_str());
}

// Tests streaming wide strings to testing::Message.
TEST(MessageTest, WideStrings) {
  // Streams a NULL of type const wchar_t*.
  const wchar_t* const_wstr = NULL;
  EXPECT_STREQ("(null)",
               (Message() << const_wstr).GetString().c_str());

  // Streams a NULL of type wchar_t*.
  wchar_t* wstr = NULL;
  EXPECT_STREQ("(null)",
               (Message() << wstr).GetString().c_str());

  // Streams a non-NULL of type const wchar_t*.
  const_wstr = L"abc\x8119";
  EXPECT_STREQ("abc\xe8\x84\x99",
               (Message() << const_wstr).GetString().c_str());

  // Streams a non-NULL of type wchar_t*.
  wstr = const_cast<wchar_t*>(const_wstr);
  EXPECT_STREQ("abc\xe8\x84\x99",
               (Message() << wstr).GetString().c_str());
}


// This line tests that we can define tests in the testing namespace.
namespace testing {

// Tests the TestInfo class.

class TestInfoTest : public Test {
 protected:
  static TestInfo * GetTestInfo(const char* test_name) {
    return UnitTest::GetInstance()->impl()->
      GetTestCase("TestInfoTest", "", NULL, NULL)->
        GetTestInfo(test_name);
  }

  static const TestResult* GetTestResult(
      const TestInfo* test_info) {
    return test_info->result();
  }
};

// Tests TestInfo::test_case_name() and TestInfo::name().
TEST_F(TestInfoTest, Names) {
  TestInfo * const test_info = GetTestInfo("Names");

  ASSERT_STREQ("TestInfoTest", test_info->test_case_name());
  ASSERT_STREQ("Names", test_info->name());
}

// Tests TestInfo::result().
TEST_F(TestInfoTest, result) {
  TestInfo * const test_info = GetTestInfo("result");

  // Initially, there is no TestPartResult for this test.
  ASSERT_EQ(0u, GetTestResult(test_info)->total_part_count());

  // After the previous assertion, there is still none.
  ASSERT_EQ(0u, GetTestResult(test_info)->total_part_count());
}

// Tests setting up and tearing down a test case.

class SetUpTestCaseTest : public Test {
 protected:
  // This will be called once before the first test in this test case
  // is run.
  static void SetUpTestCase() {
    printf("Setting up the test case . . .\n");

    // Initializes some shared resource.  In this simple example, we
    // just create a C string.  More complex stuff can be done if
    // desired.
    shared_resource_ = "123";

    // Increments the number of test cases that have been set up.
    counter_++;

    // SetUpTestCase() should be called only once.
    EXPECT_EQ(1, counter_);
  }

  // This will be called once after the last test in this test case is
  // run.
  static void TearDownTestCase() {
    printf("Tearing down the test case . . .\n");

    // Decrements the number of test cases that have been set up.
    counter_--;

    // TearDownTestCase() should be called only once.
    EXPECT_EQ(0, counter_);

    // Cleans up the shared resource.
    shared_resource_ = NULL;
  }

  // This will be called before each test in this test case.
  virtual void SetUp() {
    // SetUpTestCase() should be called only once, so counter_ should
    // always be 1.
    EXPECT_EQ(1, counter_);
  }

  // Number of test cases that have been set up.
  static int counter_;

  // Some resource to be shared by all tests in this test case.
  static const char* shared_resource_;
};

int SetUpTestCaseTest::counter_ = 0;
const char* SetUpTestCaseTest::shared_resource_ = NULL;

// A test that uses the shared resource.
TEST_F(SetUpTestCaseTest, Test1) {
  EXPECT_STRNE(NULL, shared_resource_);
}

// Another test that uses the shared resource.
TEST_F(SetUpTestCaseTest, Test2) {
  EXPECT_STREQ("123", shared_resource_);
}

// The InitGoogleTestTest test case tests testing::InitGoogleTest().

// The Flags struct stores a copy of all Google Test flags.
struct Flags {
  // Constructs a Flags struct where each flag has its default value.
  Flags() : also_run_disabled_tests(false),
            break_on_failure(false),
            catch_exceptions(false),
            death_test_use_fork(false),
            filter(""),
            list_tests(false),
            output(""),
            print_time(false),
            repeat(1) {}

  // Factory methods.

  // Creates a Flags struct where the gtest_also_run_disabled_tests flag has
  // the given value.
  static Flags AlsoRunDisabledTests(bool also_run_disabled_tests) {
    Flags flags;
    flags.also_run_disabled_tests = also_run_disabled_tests;
    return flags;
  }

  // Creates a Flags struct where the gtest_break_on_failure flag has
  // the given value.
  static Flags BreakOnFailure(bool break_on_failure) {
    Flags flags;
    flags.break_on_failure = break_on_failure;
    return flags;
  }

  // Creates a Flags struct where the gtest_catch_exceptions flag has
  // the given value.
  static Flags CatchExceptions(bool catch_exceptions) {
    Flags flags;
    flags.catch_exceptions = catch_exceptions;
    return flags;
  }

  // Creates a Flags struct where the gtest_death_test_use_fork flag has
  // the given value.
  static Flags DeathTestUseFork(bool death_test_use_fork) {
    Flags flags;
    flags.death_test_use_fork = death_test_use_fork;
    return flags;
  }

  // Creates a Flags struct where the gtest_filter flag has the given
  // value.
  static Flags Filter(const char* filter) {
    Flags flags;
    flags.filter = filter;
    return flags;
  }

  // Creates a Flags struct where the gtest_list_tests flag has the
  // given value.
  static Flags ListTests(bool list_tests) {
    Flags flags;
    flags.list_tests = list_tests;
    return flags;
  }

  // Creates a Flags struct where the gtest_output flag has the given
  // value.
  static Flags Output(const char* output) {
    Flags flags;
    flags.output = output;
    return flags;
  }

  // Creates a Flags struct where the gtest_print_time flag has the given
  // value.
  static Flags PrintTime(bool print_time) {
    Flags flags;
    flags.print_time = print_time;
    return flags;
  }

  // Creates a Flags struct where the gtest_repeat flag has the given
  // value.
  static Flags Repeat(Int32 repeat) {
    Flags flags;
    flags.repeat = repeat;
    return flags;
  }

  // These fields store the flag values.
  bool also_run_disabled_tests;
  bool break_on_failure;
  bool catch_exceptions;
  bool death_test_use_fork;
  const char* filter;
  bool list_tests;
  const char* output;
  bool print_time;
  Int32 repeat;
};

// Fixture for testing InitGoogleTest().
class InitGoogleTestTest : public Test {
 protected:
  // Clears the flags before each test.
  virtual void SetUp() {
    GTEST_FLAG(also_run_disabled_tests) = false;
    GTEST_FLAG(break_on_failure) = false;
    GTEST_FLAG(catch_exceptions) = false;
    GTEST_FLAG(death_test_use_fork) = false;
    GTEST_FLAG(filter) = "";
    GTEST_FLAG(list_tests) = false;
    GTEST_FLAG(output) = "";
    GTEST_FLAG(print_time) = false;
    GTEST_FLAG(repeat) = 1;
  }

  // Asserts that two narrow or wide string arrays are equal.
  template <typename CharType>
  static void AssertStringArrayEq(size_t size1, CharType** array1,
                                  size_t size2, CharType** array2) {
    ASSERT_EQ(size1, size2) << " Array sizes different.";

    for (size_t i = 0; i != size1; i++) {
      ASSERT_STREQ(array1[i], array2[i]) << " where i == " << i;
    }
  }

  // Verifies that the flag values match the expected values.
  static void CheckFlags(const Flags& expected) {
    EXPECT_EQ(expected.also_run_disabled_tests,
              GTEST_FLAG(also_run_disabled_tests));
    EXPECT_EQ(expected.break_on_failure, GTEST_FLAG(break_on_failure));
    EXPECT_EQ(expected.catch_exceptions, GTEST_FLAG(catch_exceptions));
    EXPECT_EQ(expected.death_test_use_fork, GTEST_FLAG(death_test_use_fork));
    EXPECT_STREQ(expected.filter, GTEST_FLAG(filter).c_str());
    EXPECT_EQ(expected.list_tests, GTEST_FLAG(list_tests));
    EXPECT_STREQ(expected.output, GTEST_FLAG(output).c_str());
    EXPECT_EQ(expected.print_time, GTEST_FLAG(print_time));
    EXPECT_EQ(expected.repeat, GTEST_FLAG(repeat));
  }

  // Parses a command line (specified by argc1 and argv1), then
  // verifies that the flag values are expected and that the
  // recognized flags are removed from the command line.
  template <typename CharType>
  static void TestParsingFlags(int argc1, const CharType** argv1,
                               int argc2, const CharType** argv2,
                               const Flags& expected) {
    // Parses the command line.
    internal::ParseGoogleTestFlagsOnly(&argc1, const_cast<CharType**>(argv1));

    // Verifies the flag values.
    CheckFlags(expected);

    // Verifies that the recognized flags are removed from the command
    // line.
    AssertStringArrayEq(argc1 + 1, argv1, argc2 + 1, argv2);
  }

  // This macro wraps TestParsingFlags s.t. the user doesn't need
  // to specify the array sizes.
#define TEST_PARSING_FLAGS(argv1, argv2, expected) \
  TestParsingFlags(sizeof(argv1)/sizeof(*argv1) - 1, argv1, \
                   sizeof(argv2)/sizeof(*argv2) - 1, argv2, expected)
};

// Tests parsing an empty command line.
TEST_F(InitGoogleTestTest, Empty) {
  const char* argv[] = {
    NULL
  };

  const char* argv2[] = {
    NULL
  };

  TEST_PARSING_FLAGS(argv, argv2, Flags());
}

// Tests parsing a command line that has no flag.
TEST_F(InitGoogleTestTest, NoFlag) {
  const char* argv[] = {
    "foo.exe",
    NULL
  };

  const char* argv2[] = {
    "foo.exe",
    NULL
  };

  TEST_PARSING_FLAGS(argv, argv2, Flags());
}

// Tests parsing a bad --gtest_filter flag.
TEST_F(InitGoogleTestTest, FilterBad) {
  const char* argv[] = {
    "foo.exe",
    "--gtest_filter",
    NULL
  };

  const char* argv2[] = {
    "foo.exe",
    "--gtest_filter",
    NULL
  };

  TEST_PARSING_FLAGS(argv, argv2, Flags::Filter(""));
}

// Tests parsing an empty --gtest_filter flag.
TEST_F(InitGoogleTestTest, FilterEmpty) {
  const char* argv[] = {
    "foo.exe",
    "--gtest_filter=",
    NULL
  };

  const char* argv2[] = {
    "foo.exe",
    NULL
  };

  TEST_PARSING_FLAGS(argv, argv2, Flags::Filter(""));
}

// Tests parsing a non-empty --gtest_filter flag.
TEST_F(InitGoogleTestTest, FilterNonEmpty) {
  const char* argv[] = {
    "foo.exe",
    "--gtest_filter=abc",
    NULL
  };

  const char* argv2[] = {
    "foo.exe",
    NULL
  };

  TEST_PARSING_FLAGS(argv, argv2, Flags::Filter("abc"));
}

// Tests parsing --gtest_break_on_failure.
TEST_F(InitGoogleTestTest, BreakOnFailureNoDef) {
  const char* argv[] = {
    "foo.exe",
    "--gtest_break_on_failure",
    NULL
};

  const char* argv2[] = {
    "foo.exe",
    NULL
  };

  TEST_PARSING_FLAGS(argv, argv2, Flags::BreakOnFailure(true));
}

// Tests parsing --gtest_break_on_failure=0.
TEST_F(InitGoogleTestTest, BreakOnFailureFalse_0) {
  const char* argv[] = {
    "foo.exe",
    "--gtest_break_on_failure=0",
    NULL
  };

  const char* argv2[] = {
    "foo.exe",
    NULL
  };

  TEST_PARSING_FLAGS(argv, argv2, Flags::BreakOnFailure(false));
}

// Tests parsing --gtest_break_on_failure=f.
TEST_F(InitGoogleTestTest, BreakOnFailureFalse_f) {
  const char* argv[] = {
    "foo.exe",
    "--gtest_break_on_failure=f",
    NULL
  };

  const char* argv2[] = {
    "foo.exe",
    NULL
  };

  TEST_PARSING_FLAGS(argv, argv2, Flags::BreakOnFailure(false));
}

// Tests parsing --gtest_break_on_failure=F.
TEST_F(InitGoogleTestTest, BreakOnFailureFalse_F) {
  const char* argv[] = {
    "foo.exe",
    "--gtest_break_on_failure=F",
    NULL
  };

  const char* argv2[] = {
    "foo.exe",
    NULL
  };

  TEST_PARSING_FLAGS(argv, argv2, Flags::BreakOnFailure(false));
}

// Tests parsing a --gtest_break_on_failure flag that has a "true"
// definition.
TEST_F(InitGoogleTestTest, BreakOnFailureTrue) {
  const char* argv[] = {
    "foo.exe",
    "--gtest_break_on_failure=1",
    NULL
  };

  const char* argv2[] = {
    "foo.exe",
    NULL
  };

  TEST_PARSING_FLAGS(argv, argv2, Flags::BreakOnFailure(true));
}

// Tests parsing --gtest_catch_exceptions.
TEST_F(InitGoogleTestTest, CatchExceptions) {
  const char* argv[] = {
    "foo.exe",
    "--gtest_catch_exceptions",
    NULL
  };

  const char* argv2[] = {
    "foo.exe",
    NULL
  };

  TEST_PARSING_FLAGS(argv, argv2, Flags::CatchExceptions(true));
}

// Tests parsing --gtest_death_test_use_fork.
TEST_F(InitGoogleTestTest, DeathTestUseFork) {
  const char* argv[] = {
    "foo.exe",
    "--gtest_death_test_use_fork",
    NULL
  };

  const char* argv2[] = {
    "foo.exe",
    NULL
  };

  TEST_PARSING_FLAGS(argv, argv2, Flags::DeathTestUseFork(true));
}

// Tests having the same flag twice with different values.  The
// expected behavior is that the one coming last takes precedence.
TEST_F(InitGoogleTestTest, DuplicatedFlags) {
  const char* argv[] = {
    "foo.exe",
    "--gtest_filter=a",
    "--gtest_filter=b",
    NULL
  };

  const char* argv2[] = {
    "foo.exe",
    NULL
  };

  TEST_PARSING_FLAGS(argv, argv2, Flags::Filter("b"));
}

// Tests having an unrecognized flag on the command line.
TEST_F(InitGoogleTestTest, UnrecognizedFlag) {
  const char* argv[] = {
    "foo.exe",
    "--gtest_break_on_failure",
    "bar",  // Unrecognized by Google Test.
    "--gtest_filter=b",
    NULL
  };

  const char* argv2[] = {
    "foo.exe",
    "bar",
    NULL
  };

  Flags flags;
  flags.break_on_failure = true;
  flags.filter = "b";
  TEST_PARSING_FLAGS(argv, argv2, flags);
}

// Tests having a --gtest_list_tests flag
TEST_F(InitGoogleTestTest, ListTestsFlag) {
    const char* argv[] = {
      "foo.exe",
      "--gtest_list_tests",
      NULL
    };

    const char* argv2[] = {
      "foo.exe",
      NULL
    };

    TEST_PARSING_FLAGS(argv, argv2, Flags::ListTests(true));
}

// Tests having a --gtest_list_tests flag with a "true" value
TEST_F(InitGoogleTestTest, ListTestsTrue) {
    const char* argv[] = {
      "foo.exe",
      "--gtest_list_tests=1",
      NULL
    };

    const char* argv2[] = {
      "foo.exe",
      NULL
    };

    TEST_PARSING_FLAGS(argv, argv2, Flags::ListTests(true));
}

// Tests having a --gtest_list_tests flag with a "false" value
TEST_F(InitGoogleTestTest, ListTestsFalse) {
    const char* argv[] = {
      "foo.exe",
      "--gtest_list_tests=0",
      NULL
    };

    const char* argv2[] = {
      "foo.exe",
      NULL
    };

    TEST_PARSING_FLAGS(argv, argv2, Flags::ListTests(false));
}

// Tests parsing --gtest_list_tests=f.
TEST_F(InitGoogleTestTest, ListTestsFalse_f) {
  const char* argv[] = {
    "foo.exe",
    "--gtest_list_tests=f",
    NULL
  };

  const char* argv2[] = {
    "foo.exe",
    NULL
  };

  TEST_PARSING_FLAGS(argv, argv2, Flags::ListTests(false));
}

// Tests parsing --gtest_break_on_failure=F.
TEST_F(InitGoogleTestTest, ListTestsFalse_F) {
  const char* argv[] = {
    "foo.exe",
    "--gtest_list_tests=F",
    NULL
  };

  const char* argv2[] = {
    "foo.exe",
    NULL
  };

  TEST_PARSING_FLAGS(argv, argv2, Flags::ListTests(false));
}

// Tests parsing --gtest_output (invalid).
TEST_F(InitGoogleTestTest, OutputEmpty) {
  const char* argv[] = {
    "foo.exe",
    "--gtest_output",
    NULL
  };

  const char* argv2[] = {
    "foo.exe",
    "--gtest_output",
    NULL
  };

  TEST_PARSING_FLAGS(argv, argv2, Flags());
}

// Tests parsing --gtest_output=xml
TEST_F(InitGoogleTestTest, OutputXml) {
  const char* argv[] = {
    "foo.exe",
    "--gtest_output=xml",
    NULL
  };

  const char* argv2[] = {
    "foo.exe",
    NULL
  };

  TEST_PARSING_FLAGS(argv, argv2, Flags::Output("xml"));
}

// Tests parsing --gtest_output=xml:file
TEST_F(InitGoogleTestTest, OutputXmlFile) {
  const char* argv[] = {
    "foo.exe",
    "--gtest_output=xml:file",
    NULL
  };

  const char* argv2[] = {
    "foo.exe",
    NULL
  };

  TEST_PARSING_FLAGS(argv, argv2, Flags::Output("xml:file"));
}

// Tests parsing --gtest_output=xml:directory/path/
TEST_F(InitGoogleTestTest, OutputXmlDirectory) {
  const char* argv[] = {
    "foo.exe",
    "--gtest_output=xml:directory/path/",
    NULL
  };

  const char* argv2[] = {
    "foo.exe",
    NULL
  };

  TEST_PARSING_FLAGS(argv, argv2, Flags::Output("xml:directory/path/"));
}

// Tests having a --gtest_print_time flag
TEST_F(InitGoogleTestTest, PrintTimeFlag) {
    const char* argv[] = {
      "foo.exe",
      "--gtest_print_time",
      NULL
    };

    const char* argv2[] = {
      "foo.exe",
      NULL
    };

    TEST_PARSING_FLAGS(argv, argv2, Flags::PrintTime(true));
}

// Tests having a --gtest_print_time flag with a "true" value
TEST_F(InitGoogleTestTest, PrintTimeTrue) {
    const char* argv[] = {
      "foo.exe",
      "--gtest_print_time=1",
      NULL
    };

    const char* argv2[] = {
      "foo.exe",
      NULL
    };

    TEST_PARSING_FLAGS(argv, argv2, Flags::PrintTime(true));
}

// Tests having a --gtest_print_time flag with a "false" value
TEST_F(InitGoogleTestTest, PrintTimeFalse) {
    const char* argv[] = {
      "foo.exe",
      "--gtest_print_time=0",
      NULL
    };

    const char* argv2[] = {
      "foo.exe",
      NULL
    };

    TEST_PARSING_FLAGS(argv, argv2, Flags::PrintTime(false));
}

// Tests parsing --gtest_print_time=f.
TEST_F(InitGoogleTestTest, PrintTimeFalse_f) {
  const char* argv[] = {
    "foo.exe",
    "--gtest_print_time=f",
    NULL
  };

  const char* argv2[] = {
    "foo.exe",
    NULL
  };

  TEST_PARSING_FLAGS(argv, argv2, Flags::PrintTime(false));
}

// Tests parsing --gtest_print_time=F.
TEST_F(InitGoogleTestTest, PrintTimeFalse_F) {
  const char* argv[] = {
    "foo.exe",
    "--gtest_print_time=F",
    NULL
  };

  const char* argv2[] = {
    "foo.exe",
    NULL
  };

  TEST_PARSING_FLAGS(argv, argv2, Flags::PrintTime(false));
}

// Tests parsing --gtest_repeat=number
TEST_F(InitGoogleTestTest, Repeat) {
  const char* argv[] = {
    "foo.exe",
    "--gtest_repeat=1000",
    NULL
  };

  const char* argv2[] = {
    "foo.exe",
    NULL
  };

  TEST_PARSING_FLAGS(argv, argv2, Flags::Repeat(1000));
}

// Tests having a --gtest_also_run_disabled_tests flag
TEST_F(InitGoogleTestTest, AlsoRunDisabledTestsFlag) {
    const char* argv[] = {
      "foo.exe",
      "--gtest_also_run_disabled_tests",
      NULL
    };

    const char* argv2[] = {
      "foo.exe",
      NULL
    };

    TEST_PARSING_FLAGS(argv, argv2, Flags::AlsoRunDisabledTests(true));
}

// Tests having a --gtest_also_run_disabled_tests flag with a "true" value
TEST_F(InitGoogleTestTest, AlsoRunDisabledTestsTrue) {
    const char* argv[] = {
      "foo.exe",
      "--gtest_also_run_disabled_tests=1",
      NULL
    };

    const char* argv2[] = {
      "foo.exe",
      NULL
    };

    TEST_PARSING_FLAGS(argv, argv2, Flags::AlsoRunDisabledTests(true));
}

// Tests having a --gtest_also_run_disabled_tests flag with a "false" value
TEST_F(InitGoogleTestTest, AlsoRunDisabledTestsFalse) {
    const char* argv[] = {
      "foo.exe",
      "--gtest_also_run_disabled_tests=0",
      NULL
    };

    const char* argv2[] = {
      "foo.exe",
      NULL
    };

    TEST_PARSING_FLAGS(argv, argv2, Flags::AlsoRunDisabledTests(false));
}

#ifdef GTEST_OS_WINDOWS
// Tests parsing wide strings.
TEST_F(InitGoogleTestTest, WideStrings) {
  const wchar_t* argv[] = {
    L"foo.exe",
    L"--gtest_filter=Foo*",
    L"--gtest_list_tests=1",
    L"--gtest_break_on_failure",
    L"--non_gtest_flag",
    NULL
  };

  const wchar_t* argv2[] = {
    L"foo.exe",
    L"--non_gtest_flag",
    NULL
  };

  Flags expected_flags;
  expected_flags.break_on_failure = true;
  expected_flags.filter = "Foo*";
  expected_flags.list_tests = true;

  TEST_PARSING_FLAGS(argv, argv2, expected_flags);
}
#endif  // GTEST_OS_WINDOWS

// Tests current_test_info() in UnitTest.
class CurrentTestInfoTest : public Test {
 protected:
  // Tests that current_test_info() returns NULL before the first test in
  // the test case is run.
  static void SetUpTestCase() {
    // There should be no tests running at this point.
    const TestInfo* test_info =
      UnitTest::GetInstance()->current_test_info();
    EXPECT_EQ(NULL, test_info)
        << "There should be no tests running at this point.";
  }

  // Tests that current_test_info() returns NULL after the last test in
  // the test case has run.
  static void TearDownTestCase() {
    const TestInfo* test_info =
      UnitTest::GetInstance()->current_test_info();
    EXPECT_EQ(NULL, test_info)
        << "There should be no tests running at this point.";
  }
};

// Tests that current_test_info() returns TestInfo for currently running
// test by checking the expected test name against the actual one.
TEST_F(CurrentTestInfoTest, WorksForFirstTestInATestCase) {
  const TestInfo* test_info =
    UnitTest::GetInstance()->current_test_info();
  ASSERT_TRUE(NULL != test_info)
      << "There is a test running so we should have a valid TestInfo.";
  EXPECT_STREQ("CurrentTestInfoTest", test_info->test_case_name())
      << "Expected the name of the currently running test case.";
  EXPECT_STREQ("WorksForFirstTestInATestCase", test_info->name())
      << "Expected the name of the currently running test.";
}

// Tests that current_test_info() returns TestInfo for currently running
// test by checking the expected test name against the actual one.  We
// use this test to see that the TestInfo object actually changed from
// the previous invocation.
TEST_F(CurrentTestInfoTest, WorksForSecondTestInATestCase) {
  const TestInfo* test_info =
    UnitTest::GetInstance()->current_test_info();
  ASSERT_TRUE(NULL != test_info)
      << "There is a test running so we should have a valid TestInfo.";
  EXPECT_STREQ("CurrentTestInfoTest", test_info->test_case_name())
      << "Expected the name of the currently running test case.";
  EXPECT_STREQ("WorksForSecondTestInATestCase", test_info->name())
      << "Expected the name of the currently running test.";
}

}  // namespace testing

// These two lines test that we can define tests in a namespace that
// has the name "testing" and is nested in another namespace.
namespace my_namespace {
namespace testing {

// Makes sure that TEST knows to use ::testing::Test instead of
// ::my_namespace::testing::Test.
class Test {};

// Makes sure that an assertion knows to use ::testing::Message instead of
// ::my_namespace::testing::Message.
class Message {};

// Makes sure that an assertion knows to use
// ::testing::AssertionResult instead of
// ::my_namespace::testing::AssertionResult.
class AssertionResult {};

// Tests that an assertion that should succeed works as expected.
TEST(NestedTestingNamespaceTest, Success) {
  EXPECT_EQ(1, 1) << "This shouldn't fail.";
}

// Tests that an assertion that should fail works as expected.
TEST(NestedTestingNamespaceTest, Failure) {
  EXPECT_FATAL_FAILURE(FAIL() << "This failure is expected.",
                       "This failure is expected.");
}

}  // namespace testing
}  // namespace my_namespace

// Tests that one can call superclass SetUp and TearDown methods--
// that is, that they are not private.
// No tests are based on this fixture; the test "passes" if it compiles
// successfully.
class ProtectedFixtureMethodsTest : public Test {
 protected:
  virtual void SetUp() {
    Test::SetUp();
  }
  virtual void TearDown() {
    Test::TearDown();
  }
};

// StreamingAssertionsTest tests the streaming versions of a representative
// sample of assertions.
TEST(StreamingAssertionsTest, Unconditional) {
  SUCCEED() << "expected success";
  EXPECT_NONFATAL_FAILURE(ADD_FAILURE() << "expected failure",
                          "expected failure");
  EXPECT_FATAL_FAILURE(FAIL() << "expected failure",
                       "expected failure");
}

TEST(StreamingAssertionsTest, Truth) {
  EXPECT_TRUE(true) << "unexpected failure";
  ASSERT_TRUE(true) << "unexpected failure";
  EXPECT_NONFATAL_FAILURE(EXPECT_TRUE(false) << "expected failure",
                          "expected failure");
  EXPECT_FATAL_FAILURE(ASSERT_TRUE(false) << "expected failure",
                       "expected failure");
}

TEST(StreamingAssertionsTest, Truth2) {
  EXPECT_FALSE(false) << "unexpected failure";
  ASSERT_FALSE(false) << "unexpected failure";
  EXPECT_NONFATAL_FAILURE(EXPECT_FALSE(true) << "expected failure",
                          "expected failure");
  EXPECT_FATAL_FAILURE(ASSERT_FALSE(true) << "expected failure",
                       "expected failure");
}

TEST(StreamingAssertionsTest, IntegerEquals) {
  EXPECT_EQ(1, 1) << "unexpected failure";
  ASSERT_EQ(1, 1) << "unexpected failure";
  EXPECT_NONFATAL_FAILURE(EXPECT_EQ(1, 2) << "expected failure",
                          "expected failure");
  EXPECT_FATAL_FAILURE(ASSERT_EQ(1, 2) << "expected failure",
                       "expected failure");
}

TEST(StreamingAssertionsTest, IntegerLessThan) {
  EXPECT_LT(1, 2) << "unexpected failure";
  ASSERT_LT(1, 2) << "unexpected failure";
  EXPECT_NONFATAL_FAILURE(EXPECT_LT(2, 1) << "expected failure",
                          "expected failure");
  EXPECT_FATAL_FAILURE(ASSERT_LT(2, 1) << "expected failure",
                       "expected failure");
}

TEST(StreamingAssertionsTest, StringsEqual) {
  EXPECT_STREQ("foo", "foo") << "unexpected failure";
  ASSERT_STREQ("foo", "foo") << "unexpected failure";
  EXPECT_NONFATAL_FAILURE(EXPECT_STREQ("foo", "bar") << "expected failure",
                          "expected failure");
  EXPECT_FATAL_FAILURE(ASSERT_STREQ("foo", "bar") << "expected failure",
                       "expected failure");
}

TEST(StreamingAssertionsTest, StringsNotEqual) {
  EXPECT_STRNE("foo", "bar") << "unexpected failure";
  ASSERT_STRNE("foo", "bar") << "unexpected failure";
  EXPECT_NONFATAL_FAILURE(EXPECT_STRNE("foo", "foo") << "expected failure",
                          "expected failure");
  EXPECT_FATAL_FAILURE(ASSERT_STRNE("foo", "foo") << "expected failure",
                       "expected failure");
}

TEST(StreamingAssertionsTest, StringsEqualIgnoringCase) {
  EXPECT_STRCASEEQ("foo", "FOO") << "unexpected failure";
  ASSERT_STRCASEEQ("foo", "FOO") << "unexpected failure";
  EXPECT_NONFATAL_FAILURE(EXPECT_STRCASEEQ("foo", "bar") << "expected failure",
                          "expected failure");
  EXPECT_FATAL_FAILURE(ASSERT_STRCASEEQ("foo", "bar") << "expected failure",
                       "expected failure");
}

TEST(StreamingAssertionsTest, StringNotEqualIgnoringCase) {
  EXPECT_STRCASENE("foo", "bar") << "unexpected failure";
  ASSERT_STRCASENE("foo", "bar") << "unexpected failure";
  EXPECT_NONFATAL_FAILURE(EXPECT_STRCASENE("foo", "FOO") << "expected failure",
                          "expected failure");
  EXPECT_FATAL_FAILURE(ASSERT_STRCASENE("bar", "BAR") << "expected failure",
                       "expected failure");
}

TEST(StreamingAssertionsTest, FloatingPointEquals) {
  EXPECT_FLOAT_EQ(1.0, 1.0) << "unexpected failure";
  ASSERT_FLOAT_EQ(1.0, 1.0) << "unexpected failure";
  EXPECT_NONFATAL_FAILURE(EXPECT_FLOAT_EQ(0.0, 1.0) << "expected failure",
                          "expected failure");
  EXPECT_FATAL_FAILURE(ASSERT_FLOAT_EQ(0.0, 1.0) << "expected failure",
                       "expected failure");
}

#if GTEST_HAS_EXCEPTIONS

TEST(StreamingAssertionsTest, Throw) {
  EXPECT_THROW(ThrowAnInteger(), int) << "unexpected failure";
  ASSERT_THROW(ThrowAnInteger(), int) << "unexpected failure";
  EXPECT_NONFATAL_FAILURE(EXPECT_THROW(ThrowAnInteger(), bool) <<
                          "expected failure", "expected failure");
  EXPECT_FATAL_FAILURE(ASSERT_THROW(ThrowAnInteger(), bool) <<
                       "expected failure", "expected failure");
}

TEST(StreamingAssertionsTest, NoThrow) {
  EXPECT_NO_THROW(1) << "unexpected failure";
  ASSERT_NO_THROW(1) << "unexpected failure";
  EXPECT_NONFATAL_FAILURE(EXPECT_NO_THROW(ThrowAnInteger()) <<
                          "expected failure", "expected failure");
  EXPECT_FATAL_FAILURE(ASSERT_NO_THROW(ThrowAnInteger()) <<
                       "expected failure", "expected failure");
}

TEST(StreamingAssertionsTest, AnyThrow) {
  EXPECT_ANY_THROW(ThrowAnInteger()) << "unexpected failure";
  ASSERT_ANY_THROW(ThrowAnInteger()) << "unexpected failure";
  EXPECT_NONFATAL_FAILURE(EXPECT_ANY_THROW(1) <<
                          "expected failure", "expected failure");
  EXPECT_FATAL_FAILURE(ASSERT_ANY_THROW(1) <<
                       "expected failure", "expected failure");
}

#endif  // GTEST_HAS_EXCEPTIONS

// Tests that Google Test correctly decides whether to use colors in the output.

TEST(ColoredOutputTest, UsesColorsWhenGTestColorFlagIsYes) {
  GTEST_FLAG(color) = "yes";

  SetEnv("TERM", "xterm");  // TERM supports colors.
  EXPECT_TRUE(ShouldUseColor(true));  // Stdout is a TTY.
  EXPECT_TRUE(ShouldUseColor(false));  // Stdout is not a TTY.

  SetEnv("TERM", "dumb");  // TERM doesn't support colors.
  EXPECT_TRUE(ShouldUseColor(true));  // Stdout is a TTY.
  EXPECT_TRUE(ShouldUseColor(false));  // Stdout is not a TTY.
}

TEST(ColoredOutputTest, UsesColorsWhenGTestColorFlagIsAliasOfYes) {
  SetEnv("TERM", "dumb");  // TERM doesn't support colors.

  GTEST_FLAG(color) = "True";
  EXPECT_TRUE(ShouldUseColor(false));  // Stdout is not a TTY.

  GTEST_FLAG(color) = "t";
  EXPECT_TRUE(ShouldUseColor(false));  // Stdout is not a TTY.

  GTEST_FLAG(color) = "1";
  EXPECT_TRUE(ShouldUseColor(false));  // Stdout is not a TTY.
}

TEST(ColoredOutputTest, UsesNoColorWhenGTestColorFlagIsNo) {
  GTEST_FLAG(color) = "no";

  SetEnv("TERM", "xterm");  // TERM supports colors.
  EXPECT_FALSE(ShouldUseColor(true));  // Stdout is a TTY.
  EXPECT_FALSE(ShouldUseColor(false));  // Stdout is not a TTY.

  SetEnv("TERM", "dumb");  // TERM doesn't support colors.
  EXPECT_FALSE(ShouldUseColor(true));  // Stdout is a TTY.
  EXPECT_FALSE(ShouldUseColor(false));  // Stdout is not a TTY.
}

TEST(ColoredOutputTest, UsesNoColorWhenGTestColorFlagIsInvalid) {
  SetEnv("TERM", "xterm");  // TERM supports colors.

  GTEST_FLAG(color) = "F";
  EXPECT_FALSE(ShouldUseColor(true));  // Stdout is a TTY.

  GTEST_FLAG(color) = "0";
  EXPECT_FALSE(ShouldUseColor(true));  // Stdout is a TTY.

  GTEST_FLAG(color) = "unknown";
  EXPECT_FALSE(ShouldUseColor(true));  // Stdout is a TTY.
}

TEST(ColoredOutputTest, UsesColorsWhenStdoutIsTty) {
  GTEST_FLAG(color) = "auto";

  SetEnv("TERM", "xterm");  // TERM supports colors.
  EXPECT_FALSE(ShouldUseColor(false));  // Stdout is not a TTY.
  EXPECT_TRUE(ShouldUseColor(true));    // Stdout is a TTY.
}

TEST(ColoredOutputTest, UsesColorsWhenTermSupportsColors) {
  GTEST_FLAG(color) = "auto";

#ifdef GTEST_OS_WINDOWS
  // On Windows, we ignore the TERM variable as it's usually not set.

  SetEnv("TERM", "dumb");
  EXPECT_TRUE(ShouldUseColor(true));  // Stdout is a TTY.

  SetEnv("TERM", "");
  EXPECT_TRUE(ShouldUseColor(true));  // Stdout is a TTY.

  SetEnv("TERM", "xterm");
  EXPECT_TRUE(ShouldUseColor(true));  // Stdout is a TTY.
#else
  // On non-Windows platforms, we rely on TERM to determine if the
  // terminal supports colors.

  SetEnv("TERM", "dumb");  // TERM doesn't support colors.
  EXPECT_FALSE(ShouldUseColor(true));  // Stdout is a TTY.

  SetEnv("TERM", "emacs");  // TERM doesn't support colors.
  EXPECT_FALSE(ShouldUseColor(true));  // Stdout is a TTY.

  SetEnv("TERM", "vt100");  // TERM doesn't support colors.
  EXPECT_FALSE(ShouldUseColor(true));  // Stdout is a TTY.

  SetEnv("TERM", "xterm-mono");  // TERM doesn't support colors.
  EXPECT_FALSE(ShouldUseColor(true));  // Stdout is a TTY.

  SetEnv("TERM", "xterm");  // TERM supports colors.
  EXPECT_TRUE(ShouldUseColor(true));  // Stdout is a TTY.

  SetEnv("TERM", "xterm-color");  // TERM supports colors.
  EXPECT_TRUE(ShouldUseColor(true));  // Stdout is a TTY.
#endif  // GTEST_OS_WINDOWS
}

// Verifies that StaticAssertTypeEq works in a namespace scope.

static bool dummy1 = StaticAssertTypeEq<bool, bool>();
static bool dummy2 = StaticAssertTypeEq<const int, const int>();

// Verifies that StaticAssertTypeEq works in a class.

template <typename T>
class StaticAssertTypeEqTestHelper {
 public:
  StaticAssertTypeEqTestHelper() { StaticAssertTypeEq<bool, T>(); }
};

TEST(StaticAssertTypeEqTest, WorksInClass) {
  StaticAssertTypeEqTestHelper<bool>();
}

// Verifies that StaticAssertTypeEq works inside a function.

typedef int IntAlias;

TEST(StaticAssertTypeEqTest, CompilesForEqualTypes) {
  StaticAssertTypeEq<int, IntAlias>();
  StaticAssertTypeEq<int*, IntAlias*>();
}

TEST(ThreadLocalTest, DefaultConstructor) {
  ThreadLocal<int> t1;
  EXPECT_EQ(0, t1.get());

  ThreadLocal<void*> t2;
  EXPECT_TRUE(t2.get() == NULL);
}

TEST(ThreadLocalTest, Init) {
  ThreadLocal<int> t1(123);
  EXPECT_EQ(123, t1.get());

  int i = 0;
  ThreadLocal<int*> t2(&i);
  EXPECT_EQ(&i, t2.get());
}

TEST(GetCurrentOsStackTraceExceptTopTest, ReturnsTheStackTrace) {
  testing::UnitTest* const unit_test = testing::UnitTest::GetInstance();

  // We don't have a stack walker in Google Test yet.
  EXPECT_STREQ("", GetCurrentOsStackTraceExceptTop(unit_test, 0).c_str());
  EXPECT_STREQ("", GetCurrentOsStackTraceExceptTop(unit_test, 1).c_str());
}

#ifndef GTEST_OS_SYMBIAN
// We will want to integrate running the unittests to a different
// main application on Symbian.
int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);

#ifdef GTEST_HAS_DEATH_TEST
  if (!testing::internal::GTEST_FLAG(internal_run_death_test).empty()) {
    // Skip the usual output capturing if we're running as the child
    // process of an threadsafe-style death test.
    freopen("/dev/null", "w", stdout);
  }
#endif  // GTEST_HAS_DEATH_TEST

  // Runs all tests using Google Test.
  return RUN_ALL_TESTS();
}
#endif  // GTEST_OS_SYMBIAN
