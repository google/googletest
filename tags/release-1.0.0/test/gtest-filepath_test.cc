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
// Google Test filepath utilities
//
// This file tests classes and functions used internally by
// Google Test.  They are subject to change without notice.
//
// This file is #included from gtest_unittest.cc, to avoid changing
// build or make-files for some existing Google Test clients. Do not
// #include this file anywhere else!

#include <gtest/internal/gtest-filepath.h>
#include <gtest/gtest.h>

// Indicates that this translation unit is part of Google Test's
// implementation.  It must come before gtest-internal-inl.h is
// included, or there will be a compiler error.  This trick is to
// prevent a user from accidentally including gtest-internal-inl.h in
// his code.
#define GTEST_IMPLEMENTATION
#include "src/gtest-internal-inl.h"
#undef GTEST_IMPLEMENTATION

#ifdef GTEST_OS_WINDOWS
#include <direct.h>
#define PATH_SEP "\\"
#else
#define PATH_SEP "/"
#endif  // GTEST_OS_WINDOWS

namespace testing {
namespace internal {
namespace {

// FilePath's functions used by UnitTestOptions::GetOutputFile.

// RemoveDirectoryName "" -> ""
TEST(RemoveDirectoryNameTest, WhenEmptyName) {
  EXPECT_STREQ("", FilePath("").RemoveDirectoryName().c_str());
}

// RemoveDirectoryName "afile" -> "afile"
TEST(RemoveDirectoryNameTest, ButNoDirectory) {
  EXPECT_STREQ("afile",
      FilePath("afile").RemoveDirectoryName().c_str());
}

// RemoveDirectoryName "/afile" -> "afile"
TEST(RemoveDirectoryNameTest, RootFileShouldGiveFileName) {
  EXPECT_STREQ("afile",
      FilePath(PATH_SEP "afile").RemoveDirectoryName().c_str());
}

// RemoveDirectoryName "adir/" -> ""
TEST(RemoveDirectoryNameTest, WhereThereIsNoFileName) {
  EXPECT_STREQ("",
      FilePath("adir" PATH_SEP).RemoveDirectoryName().c_str());
}

// RemoveDirectoryName "adir/afile" -> "afile"
TEST(RemoveDirectoryNameTest, ShouldGiveFileName) {
  EXPECT_STREQ("afile",
      FilePath("adir" PATH_SEP "afile").RemoveDirectoryName().c_str());
}

// RemoveDirectoryName "adir/subdir/afile" -> "afile"
TEST(RemoveDirectoryNameTest, ShouldAlsoGiveFileName) {
  EXPECT_STREQ("afile",
      FilePath("adir" PATH_SEP "subdir" PATH_SEP "afile")
      .RemoveDirectoryName().c_str());
}


// RemoveFileName "" -> "./"
TEST(RemoveFileNameTest, EmptyName) {
  EXPECT_STREQ("." PATH_SEP,
      FilePath("").RemoveFileName().c_str());
}

// RemoveFileName "adir/" -> "adir/"
TEST(RemoveFileNameTest, ButNoFile) {
  EXPECT_STREQ("adir" PATH_SEP,
      FilePath("adir" PATH_SEP).RemoveFileName().c_str());
}

// RemoveFileName "adir/afile" -> "adir/"
TEST(RemoveFileNameTest, GivesDirName) {
  EXPECT_STREQ("adir" PATH_SEP,
      FilePath("adir" PATH_SEP "afile")
      .RemoveFileName().c_str());
}

// RemoveFileName "adir/subdir/afile" -> "adir/subdir/"
TEST(RemoveFileNameTest, GivesDirAndSubDirName) {
  EXPECT_STREQ("adir" PATH_SEP "subdir" PATH_SEP,
      FilePath("adir" PATH_SEP "subdir" PATH_SEP "afile")
      .RemoveFileName().c_str());
}

// RemoveFileName "/afile" -> "/"
TEST(RemoveFileNameTest, GivesRootDir) {
  EXPECT_STREQ(PATH_SEP,
      FilePath(PATH_SEP "afile").RemoveFileName().c_str());
}


TEST(MakeFileNameTest, GenerateWhenNumberIsZero) {
  FilePath actual = FilePath::MakeFileName(FilePath("foo"), FilePath("bar"),
      0, "xml");
  EXPECT_STREQ("foo" PATH_SEP "bar.xml", actual.c_str());
}

TEST(MakeFileNameTest, GenerateFileNameNumberGtZero) {
  FilePath actual = FilePath::MakeFileName(FilePath("foo"), FilePath("bar"),
      12, "xml");
  EXPECT_STREQ("foo" PATH_SEP "bar_12.xml", actual.c_str());
}

TEST(MakeFileNameTest, GenerateFileNameWithSlashNumberIsZero) {
  FilePath actual = FilePath::MakeFileName(FilePath("foo" PATH_SEP),
      FilePath("bar"), 0, "xml");
  EXPECT_STREQ("foo" PATH_SEP "bar.xml", actual.c_str());
}

TEST(MakeFileNameTest, GenerateFileNameWithSlashNumberGtZero) {
  FilePath actual = FilePath::MakeFileName(FilePath("foo" PATH_SEP),
      FilePath("bar"), 12, "xml");
  EXPECT_STREQ("foo" PATH_SEP "bar_12.xml", actual.c_str());
}


// RemoveTrailingPathSeparator "" -> ""
TEST(RemoveTrailingPathSeparatorTest, EmptyString) {
  EXPECT_STREQ("",
      FilePath("").RemoveTrailingPathSeparator().c_str());
}

// RemoveTrailingPathSeparator "foo" -> "foo"
TEST(RemoveTrailingPathSeparatorTest, FileNoSlashString) {
  EXPECT_STREQ("foo",
      FilePath("foo").RemoveTrailingPathSeparator().c_str());
}

// RemoveTrailingPathSeparator "foo/" -> "foo"
TEST(RemoveTrailingPathSeparatorTest, ShouldRemoveTrailingSeparator) {
  EXPECT_STREQ("foo",
      FilePath("foo" PATH_SEP).RemoveTrailingPathSeparator().c_str());
}

// RemoveTrailingPathSeparator "foo/bar/" -> "foo/bar/"
TEST(RemoveTrailingPathSeparatorTest, ShouldRemoveLastSeparator) {
  EXPECT_STREQ("foo" PATH_SEP "bar",
      FilePath("foo" PATH_SEP "bar" PATH_SEP).RemoveTrailingPathSeparator()
      .c_str());
}

// RemoveTrailingPathSeparator "foo/bar" -> "foo/bar"
TEST(RemoveTrailingPathSeparatorTest, ShouldReturnUnmodified) {
  EXPECT_STREQ("foo" PATH_SEP "bar",
      FilePath("foo" PATH_SEP "bar").RemoveTrailingPathSeparator().c_str());
}


class DirectoryCreationTest : public Test {
 protected:
  virtual void SetUp() {
    testdata_path_.Set(FilePath(String::Format("%s%s%s",
        TempDir().c_str(), GetCurrentExecutableName().c_str(),
        "_directory_creation" PATH_SEP "test" PATH_SEP)));
    testdata_file_.Set(testdata_path_.RemoveTrailingPathSeparator());

    unique_file0_.Set(FilePath::MakeFileName(testdata_path_, FilePath("unique"),
        0, "txt"));
    unique_file1_.Set(FilePath::MakeFileName(testdata_path_, FilePath("unique"),
        1, "txt"));

    remove(testdata_file_.c_str());
    remove(unique_file0_.c_str());
    remove(unique_file1_.c_str());
#ifdef GTEST_OS_WINDOWS
    _rmdir(testdata_path_.c_str());
#else
    rmdir(testdata_path_.c_str());
#endif  // GTEST_OS_WINDOWS
  }

  virtual void TearDown() {
    remove(testdata_file_.c_str());
    remove(unique_file0_.c_str());
    remove(unique_file1_.c_str());
#ifdef GTEST_OS_WINDOWS
    _rmdir(testdata_path_.c_str());
#else
    rmdir(testdata_path_.c_str());
#endif  // GTEST_OS_WINDOWS
  }

  String TempDir() const {
#ifdef _WIN32_WCE
    return String("\\temp\\");

#elif defined(GTEST_OS_WINDOWS)
    // MSVC 8 deprecates getenv(), so we want to suppress warning 4996
    // (deprecated function) there.
#pragma warning(push)          // Saves the current warning state.
#pragma warning(disable:4996)  // Temporarily disables warning 4996.
    const char* temp_dir = getenv("TEMP");
#pragma warning(pop)           // Restores the warning state.

    if (temp_dir == NULL || temp_dir[0] == '\0')
      return String("\\temp\\");
    else if (String(temp_dir).EndsWith("\\"))
      return String(temp_dir);
    else
      return String::Format("%s\\", temp_dir);
#else
    return String("/tmp/");
#endif
  }

  void CreateTextFile(const char* filename) {
#ifdef GTEST_OS_WINDOWS
    // MSVC 8 deprecates fopen(), so we want to suppress warning 4996
    // (deprecated function) there.#pragma warning(push)
#pragma warning(push)          // Saves the current warning state.
#pragma warning(disable:4996)  // Temporarily disables warning 4996.
    FILE* f = fopen(filename, "w");
#pragma warning(pop)           // Restores the warning state.
#else  // We are on Linux or Mac OS.
    FILE* f = fopen(filename, "w");
#endif  // GTEST_OS_WINDOWS
    fprintf(f, "text\n");
    fclose(f);
  }

  // Strings representing a directory and a file, with identical paths
  // except for the trailing separator character that distinquishes
  // a directory named 'test' from a file named 'test'. Example names:
  FilePath testdata_path_;  // "/tmp/directory_creation/test/"
  FilePath testdata_file_;  // "/tmp/directory_creation/test"
  FilePath unique_file0_;  // "/tmp/directory_creation/test/unique.txt"
  FilePath unique_file1_;  // "/tmp/directory_creation/test/unique_1.txt"
};

TEST_F(DirectoryCreationTest, CreateDirectoriesRecursively) {
  EXPECT_FALSE(testdata_path_.DirectoryExists()) << testdata_path_.c_str();
  EXPECT_TRUE(testdata_path_.CreateDirectoriesRecursively());
  EXPECT_TRUE(testdata_path_.DirectoryExists());
}

TEST_F(DirectoryCreationTest, CreateDirectoriesForAlreadyExistingPath) {
  EXPECT_FALSE(testdata_path_.DirectoryExists()) << testdata_path_.c_str();
  EXPECT_TRUE(testdata_path_.CreateDirectoriesRecursively());
  // Call 'create' again... should still succeed.
  EXPECT_TRUE(testdata_path_.CreateDirectoriesRecursively());
}

TEST_F(DirectoryCreationTest, CreateDirectoriesAndUniqueFilename) {
  FilePath file_path(FilePath::GenerateUniqueFileName(testdata_path_,
      FilePath("unique"), "txt"));
  EXPECT_STREQ(unique_file0_.c_str(), file_path.c_str());
  EXPECT_FALSE(file_path.FileOrDirectoryExists());  // file not there

  testdata_path_.CreateDirectoriesRecursively();
  EXPECT_FALSE(file_path.FileOrDirectoryExists());  // file still not there
  CreateTextFile(file_path.c_str());
  EXPECT_TRUE(file_path.FileOrDirectoryExists());

  FilePath file_path2(FilePath::GenerateUniqueFileName(testdata_path_,
      FilePath("unique"), "txt"));
  EXPECT_STREQ(unique_file1_.c_str(), file_path2.c_str());
  EXPECT_FALSE(file_path2.FileOrDirectoryExists());  // file not there
  CreateTextFile(file_path2.c_str());
  EXPECT_TRUE(file_path2.FileOrDirectoryExists());
}

TEST_F(DirectoryCreationTest, CreateDirectoriesFail) {
  // force a failure by putting a file where we will try to create a directory.
  CreateTextFile(testdata_file_.c_str());
  EXPECT_TRUE(testdata_file_.FileOrDirectoryExists());
  EXPECT_FALSE(testdata_file_.DirectoryExists());
  EXPECT_FALSE(testdata_file_.CreateDirectoriesRecursively());
}

TEST(NoDirectoryCreationTest, CreateNoDirectoriesForDefaultXmlFile) {
  const FilePath test_detail_xml("test_detail.xml");
  EXPECT_FALSE(test_detail_xml.CreateDirectoriesRecursively());
}

TEST(FilePathTest, DefaultConstructor) {
  FilePath fp;
  EXPECT_STREQ("", fp.c_str());
}

TEST(FilePathTest, CharAndCopyConstructors) {
  const FilePath fp("spicy");
  EXPECT_STREQ("spicy", fp.c_str());

  const FilePath fp_copy(fp);
  EXPECT_STREQ("spicy", fp_copy.c_str());
}

TEST(FilePathTest, StringConstructor) {
  const FilePath fp(String("cider"));
  EXPECT_STREQ("cider", fp.c_str());
}

TEST(FilePathTest, Set) {
  const FilePath apple("apple");
  FilePath mac("mac");
  mac.Set(apple);  // Implement Set() since overloading operator= is forbidden.
  EXPECT_STREQ("apple", mac.c_str());
  EXPECT_STREQ("apple", apple.c_str());
}

TEST(FilePathTest, ToString) {
  const FilePath file("drink");
  String str(file.ToString());
  EXPECT_STREQ("drink", str.c_str());
}

TEST(FilePathTest, RemoveExtension) {
  EXPECT_STREQ("app", FilePath("app.exe").RemoveExtension("exe").c_str());
  EXPECT_STREQ("APP", FilePath("APP.EXE").RemoveExtension("exe").c_str());
}

TEST(FilePathTest, RemoveExtensionWhenThereIsNoExtension) {
  EXPECT_STREQ("app", FilePath("app").RemoveExtension("exe").c_str());
}

TEST(FilePathTest, IsDirectory) {
  EXPECT_FALSE(FilePath("cola").IsDirectory());
  EXPECT_TRUE(FilePath("koala" PATH_SEP).IsDirectory());
}

}  // namespace
}  // namespace internal
}  // namespace testing

#undef PATH_SEP
