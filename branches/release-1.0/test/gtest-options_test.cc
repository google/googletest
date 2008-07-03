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
// Authors: keith.ray@gmail.com (Keith Ray)
//
// Google Test UnitTestOptions tests
//
// This file tests classes and functions used internally by
// Google Test.  They are subject to change without notice.
//
// This file is #included from gtest.cc, to avoid changing build or
// make-files on Windows and other platforms. Do not #include this file
// anywhere else!

#include <gtest/gtest.h>

// Indicates that this translation unit is part of Google Test's
// implementation.  It must come before gtest-internal-inl.h is
// included, or there will be a compiler error.  This trick is to
// prevent a user from accidentally including gtest-internal-inl.h in
// his code.
#define GTEST_IMPLEMENTATION
#include "src/gtest-internal-inl.h"
#undef GTEST_IMPLEMENTATION

namespace testing {

namespace internal {
namespace {

// Testing UnitTestOptions::GetOutputFormat/GetOutputFile.

TEST(XmlOutputTest, GetOutputFormatDefault) {
  GTEST_FLAG(output) = "";
  EXPECT_STREQ("", UnitTestOptions::GetOutputFormat().c_str());
}

TEST(XmlOutputTest, GetOutputFormat) {
  GTEST_FLAG(output) = "xml:filename";
  EXPECT_STREQ("xml", UnitTestOptions::GetOutputFormat().c_str());
}

TEST(XmlOutputTest, GetOutputFileDefault) {
  GTEST_FLAG(output) = "";
  EXPECT_STREQ("test_detail.xml",
               UnitTestOptions::GetOutputFile().c_str());
}

TEST(XmlOutputTest, GetOutputFileSingleFile) {
  GTEST_FLAG(output) = "xml:filename.abc";
  EXPECT_STREQ("filename.abc",
               UnitTestOptions::GetOutputFile().c_str());
}

TEST(XmlOutputTest, GetOutputFileFromDirectoryPath) {
#ifdef GTEST_OS_WINDOWS
  GTEST_FLAG(output) = "xml:pathname\\";
  const String& output_file = UnitTestOptions::GetOutputFile();
  EXPECT_TRUE(_strcmpi(output_file.c_str(),
                       "pathname\\gtest-options_test.xml") == 0 ||
              _strcmpi(output_file.c_str(),
                       "pathname\\gtest-options-ex_test.xml") == 0)
                           << " output_file = " << output_file;
#else
  GTEST_FLAG(output) = "xml:pathname/";
  const String& output_file = UnitTestOptions::GetOutputFile();
  // TODO(wan@google.com): libtool causes the test binary file to be
  //   named lt-gtest-options_test.  Therefore the output file may be
  //   named .../lt-gtest-options_test.xml.  We should remove this
  //   hard-coded logic when Chandler Carruth's libtool replacement is
  //   ready.
  EXPECT_TRUE(output_file == "pathname/gtest-options_test.xml" ||
              output_file == "pathname/lt-gtest-options_test.xml")
                  << " output_file = " << output_file;
#endif
}

TEST(OutputFileHelpersTest, GetCurrentExecutableName) {
  const FilePath executable = GetCurrentExecutableName();
  const char* const exe_str = executable.c_str();
#if defined(_WIN32_WCE) || defined(GTEST_OS_WINDOWS)
  ASSERT_TRUE(_strcmpi("gtest-options_test", exe_str) == 0 ||
              _strcmpi("gtest-options-ex_test", exe_str) == 0)
              << "GetCurrentExecutableName() returns " << exe_str;
#else
  // TODO(wan@google.com): remove the hard-coded "lt-" prefix when
  //   Chandler Carruth's libtool replacement is ready.
  EXPECT_TRUE(String(exe_str) == "gtest-options_test" ||
              String(exe_str) == "lt-gtest-options_test")
                  << "GetCurrentExecutableName() returns " << exe_str;
#endif
}

}  // namespace
}  // namespace internal
}  // namespace testing
