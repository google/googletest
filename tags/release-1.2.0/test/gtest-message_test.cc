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
// Tests for the Message class.

#include <gtest/gtest-message.h>

#include <gtest/gtest.h>

namespace {

using ::testing::Message;
using ::testing::internal::StrStream;

// A helper function that turns a Message into a C string.
const char* ToCString(const Message& msg) {
  static testing::internal::String result;
  result = msg.GetString();
  return result.c_str();
}

// Tests the testing::Message class

// Tests the default constructor.
TEST(MessageTest, DefaultConstructor) {
  const Message msg;
  EXPECT_STREQ("", ToCString(msg));
}

// Tests the copy constructor.
TEST(MessageTest, CopyConstructor) {
  const Message msg1("Hello");
  const Message msg2(msg1);
  EXPECT_STREQ("Hello", ToCString(msg2));
}

// Tests constructing a Message from a C-string.
TEST(MessageTest, ConstructsFromCString) {
  Message msg("Hello");
  EXPECT_STREQ("Hello", ToCString(msg));
}

// Tests streaming a non-char pointer.
TEST(MessageTest, StreamsPointer) {
  int n = 0;
  int* p = &n;
  EXPECT_STRNE("(null)", ToCString(Message() << p));
}

// Tests streaming a NULL non-char pointer.
TEST(MessageTest, StreamsNullPointer) {
  int* p = NULL;
  EXPECT_STREQ("(null)", ToCString(Message() << p));
}

// Tests streaming a C string.
TEST(MessageTest, StreamsCString) {
  EXPECT_STREQ("Foo", ToCString(Message() << "Foo"));
}

// Tests streaming a NULL C string.
TEST(MessageTest, StreamsNullCString) {
  char* p = NULL;
  EXPECT_STREQ("(null)", ToCString(Message() << p));
}

#if GTEST_HAS_STD_STRING

// Tests streaming std::string.
//
// As std::string has problem in MSVC when exception is disabled, we only
// test this where std::string can be used.
TEST(MessageTest, StreamsString) {
  const ::std::string str("Hello");
  EXPECT_STREQ("Hello", ToCString(Message() << str));
}

// Tests that we can output strings containing embedded NULs.
TEST(MessageTest, StreamsStringWithEmbeddedNUL) {
  const char char_array_with_nul[] =
      "Here's a NUL\0 and some more string";
  const ::std::string string_with_nul(char_array_with_nul,
                                      sizeof(char_array_with_nul) - 1);
  EXPECT_STREQ("Here's a NUL\\0 and some more string",
               ToCString(Message() << string_with_nul));
}

#endif  // GTEST_HAS_STD_STRING

// Tests streaming a NUL char.
TEST(MessageTest, StreamsNULChar) {
  EXPECT_STREQ("\\0", ToCString(Message() << '\0'));
}

// Tests streaming int.
TEST(MessageTest, StreamsInt) {
  EXPECT_STREQ("123", ToCString(Message() << 123));
}

// Tests that basic IO manipulators (endl, ends, and flush) can be
// streamed to Message.
TEST(MessageTest, StreamsBasicIoManip) {
  EXPECT_STREQ("Line 1.\nA NUL char \\0 in line 2.",
               ToCString(Message() << "Line 1." << std::endl
                         << "A NUL char " << std::ends << std::flush
                         << " in line 2."));
}

// Tests Message::GetString()
TEST(MessageTest, GetString) {
  Message msg;
  msg << 1 << " lamb";
  EXPECT_STREQ("1 lamb", msg.GetString().c_str());
}

// Tests streaming a Message object to an ostream.
TEST(MessageTest, StreamsToOStream) {
  Message msg("Hello");
  StrStream ss;
  ss << msg;
  EXPECT_STREQ("Hello", testing::internal::StrStreamToString(&ss).c_str());
}

// Tests that a Message object doesn't take up too much stack space.
TEST(MessageTest, DoesNotTakeUpMuchStackSpace) {
  EXPECT_LE(sizeof(Message), 16U);
}

}  // namespace
