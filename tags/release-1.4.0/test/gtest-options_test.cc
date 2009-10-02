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

#if GTEST_OS_WINDOWS_MOBILE
#include <windows.h>
#elif GTEST_OS_WINDOWS
#include <direct.h>
#endif  // GTEST_OS_WINDOWS_MOBILE

// Indicates that this translation unit is part of Google Test's
// implementation.  It must come before gtest-internal-inl.h is
// included, or there will be a compiler error.  This trick is to
// prevent a user from accidentally including gtest-internal-inl.h in
// his code.
#define GTEST_IMPLEMENTATION_ 1
#include "src/gtest-internal-inl.h"
#undef GTEST_IMPLEMENTATION_

namespace testing {
namespace internal {
namespace {

// Turns the given relative path into an absolute path.
FilePath GetAbsolutePathOf(const FilePath& relative_path) {
  return FilePath::ConcatPaths(FilePath::GetCurrentDir(), relative_path);
}

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
  EXPECT_STREQ(GetAbsolutePathOf(FilePath("test_detail.xml")).c_str(),
               UnitTestOptions::GetAbsolutePathToOutputFile().c_str());
}

TEST(XmlOutputTest, GetOutputFileSingleFile) {
  GTEST_FLAG(output) = "xml:filename.abc";
  EXPECT_STREQ(GetAbsolutePathOf(FilePath("filename.abc")).c_str(),
               UnitTestOptions::GetAbsolutePathToOutputFile().c_str());
}

TEST(XmlOutputTest, GetOutputFileFromDirectoryPath) {
#if GTEST_OS_WINDOWS
  GTEST_FLAG(output) = "xml:path\\";
  const String& output_file = UnitTestOptions::GetAbsolutePathToOutputFile();
  EXPECT_TRUE(
      _strcmpi(output_file.c_str(),
               GetAbsolutePathOf(
                   FilePath("path\\gtest-options_test.xml")).c_str()) == 0 ||
      _strcmpi(output_file.c_str(),
               GetAbsolutePathOf(
                   FilePath("path\\gtest-options-ex_test.xml")).c_str()) == 0 ||
      _strcmpi(output_file.c_str(),
               GetAbsolutePathOf(
                   FilePath("path\\gtest_all_test.xml")).c_str()) == 0)
                       << " output_file = " << output_file;
#else
  GTEST_FLAG(output) = "xml:path/";
  const String& output_file = UnitTestOptions::GetAbsolutePathToOutputFile();
  // TODO(wan@google.com): libtool causes the test binary file to be
  //   named lt-gtest-options_test.  Therefore the output file may be
  //   named .../lt-gtest-options_test.xml.  We should remove this
  //   hard-coded logic when Chandler Carruth's libtool replacement is
  //   ready.
  EXPECT_TRUE(output_file ==
              GetAbsolutePathOf(
                  FilePath("path/gtest-options_test.xml")).c_str() ||
              output_file ==
              GetAbsolutePathOf(
                  FilePath("path/lt-gtest-options_test.xml")).c_str() ||
              output_file ==
              GetAbsolutePathOf(
                  FilePath("path/gtest_all_test.xml")).c_str() ||
              output_file ==
              GetAbsolutePathOf(
                  FilePath("path/lt-gtest_all_test.xml")).c_str())
                      << " output_file = " << output_file;
#endif
}

TEST(OutputFileHelpersTest, GetCurrentExecutableName) {
  const FilePath executable = GetCurrentExecutableName();
  const char* const exe_str = executable.c_str();
#if GTEST_OS_WINDOWS
  ASSERT_TRUE(_strcmpi("gtest-options_test", exe_str) == 0 ||
              _strcmpi("gtest-options-ex_test", exe_str) == 0 ||
              _strcmpi("gtest_all_test", exe_str) == 0)
              << "GetCurrentExecutableName() returns " << exe_str;
#else
  // TODO(wan@google.com): remove the hard-coded "lt-" prefix when
  //   Chandler Carruth's libtool replacement is ready.
  EXPECT_TRUE(String(exe_str) == "gtest-options_test" ||
              String(exe_str) == "lt-gtest-options_test" ||
              String(exe_str) == "gtest_all_test" ||
              String(exe_str) == "lt-gtest_all_test")
                  << "GetCurrentExecutableName() returns " << exe_str;
#endif  // GTEST_OS_WINDOWS
}

class XmlOutputChangeDirTest : public Test {
 protected:
  virtual void SetUp() {
    original_working_dir_ = FilePath::GetCurrentDir();
    posix::ChDir("..");
    // This will make the test fail if run from the root directory.
    EXPECT_STRNE(original_working_dir_.c_str(),
                 FilePath::GetCurrentDir().c_str());
  }

  virtual void TearDown() {
    posix::ChDir(original_working_dir_.c_str());
  }

  FilePath original_working_dir_;
};

TEST_F(XmlOutputChangeDirTest, PreserveOriginalWorkingDirWithDefault) {
  GTEST_FLAG(output) = "";
  EXPECT_STREQ(FilePath::ConcatPaths(original_working_dir_,
                                     FilePath("test_detail.xml")).c_str(),
               UnitTestOptions::GetAbsolutePathToOutputFile().c_str());
}

TEST_F(XmlOutputChangeDirTest, PreserveOriginalWorkingDirWithDefaultXML) {
  GTEST_FLAG(output) = "xml";
  EXPECT_STREQ(FilePath::ConcatPaths(original_working_dir_,
                                     FilePath("test_detail.xml")).c_str(),
               UnitTestOptions::GetAbsolutePathToOutputFile().c_str());
}

TEST_F(XmlOutputChangeDirTest, PreserveOriginalWorkingDirWithRelativeFile) {
  GTEST_FLAG(output) = "xml:filename.abc";
  EXPECT_STREQ(FilePath::ConcatPaths(original_working_dir_,
                                     FilePath("filename.abc")).c_str(),
               UnitTestOptions::GetAbsolutePathToOutputFile().c_str());
}

TEST_F(XmlOutputChangeDirTest, PreserveOriginalWorkingDirWithRelativePath) {
#if GTEST_OS_WINDOWS
  GTEST_FLAG(output) = "xml:path\\";
  const String& output_file = UnitTestOptions::GetAbsolutePathToOutputFile();
  EXPECT_TRUE(
      _strcmpi(output_file.c_str(),
               FilePath::ConcatPaths(
                   original_working_dir_,
                   FilePath("path\\gtest-options_test.xml")).c_str()) == 0 ||
      _strcmpi(output_file.c_str(),
               FilePath::ConcatPaths(
                   original_working_dir_,
                   FilePath("path\\gtest-options-ex_test.xml")).c_str()) == 0 ||
      _strcmpi(output_file.c_str(),
               FilePath::ConcatPaths(
                   original_working_dir_,
                   FilePath("path\\gtest_all_test.xml")).c_str()) == 0)
                       << " output_file = " << output_file;
#else
  GTEST_FLAG(output) = "xml:path/";
  const String& output_file = UnitTestOptions::GetAbsolutePathToOutputFile();
  // TODO(wan@google.com): libtool causes the test binary file to be
  //   named lt-gtest-options_test.  Therefore the output file may be
  //   named .../lt-gtest-options_test.xml.  We should remove this
  //   hard-coded logic when Chandler Carruth's libtool replacement is
  //   ready.
  EXPECT_TRUE(output_file == FilePath::ConcatPaths(original_working_dir_,
                      FilePath("path/gtest-options_test.xml")).c_str() ||
              output_file == FilePath::ConcatPaths(original_working_dir_,
                      FilePath("path/lt-gtest-options_test.xml")).c_str() ||
              output_file == FilePath::ConcatPaths(original_working_dir_,
                      FilePath("path/gtest_all_test.xml")).c_str() ||
              output_file == FilePath::ConcatPaths(original_working_dir_,
                      FilePath("path/lt-gtest_all_test.xml")).c_str())
                  << " output_file = " << output_file;
#endif
}

TEST_F(XmlOutputChangeDirTest, PreserveOriginalWorkingDirWithAbsoluteFile) {
#if GTEST_OS_WINDOWS
  GTEST_FLAG(output) = "xml:c:\\tmp\\filename.abc";
  EXPECT_STREQ(FilePath("c:\\tmp\\filename.abc").c_str(),
               UnitTestOptions::GetAbsolutePathToOutputFile().c_str());
#else
  GTEST_FLAG(output) ="xml:/tmp/filename.abc";
  EXPECT_STREQ(FilePath("/tmp/filename.abc").c_str(),
               UnitTestOptions::GetAbsolutePathToOutputFile().c_str());
#endif
}

TEST_F(XmlOutputChangeDirTest, PreserveOriginalWorkingDirWithAbsolutePath) {
#if GTEST_OS_WINDOWS
  GTEST_FLAG(output) = "xml:c:\\tmp\\";
  const String& output_file = UnitTestOptions::GetAbsolutePathToOutputFile();
  EXPECT_TRUE(
      _strcmpi(output_file.c_str(),
               FilePath("c:\\tmp\\gtest-options_test.xml").c_str()) == 0 ||
      _strcmpi(output_file.c_str(),
               FilePath("c:\\tmp\\gtest-options-ex_test.xml").c_str()) == 0 ||
      _strcmpi(output_file.c_str(),
               FilePath("c:\\tmp\\gtest_all_test.xml").c_str()) == 0)
                   << " output_file = " << output_file;
#else
  GTEST_FLAG(output) = "xml:/tmp/";
  const String& output_file = UnitTestOptions::GetAbsolutePathToOutputFile();
  // TODO(wan@google.com): libtool causes the test binary file to be
  //   named lt-gtest-options_test.  Therefore the output file may be
  //   named .../lt-gtest-options_test.xml.  We should remove this
  //   hard-coded logic when Chandler Carruth's libtool replacement is
  //   ready.
  EXPECT_TRUE(output_file == "/tmp/gtest-options_test.xml" ||
              output_file == "/tmp/lt-gtest-options_test.xml" ||
              output_file == "/tmp/gtest_all_test.xml" ||
              output_file == "/tmp/lt-gtest_all_test.xml")
                  << " output_file = " << output_file;
#endif
}

}  // namespace
}  // namespace internal
}  // namespace testing
