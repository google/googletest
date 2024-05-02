#include <sys/stat.h>

#include <cstdlib>
#include <cstring>
#include <string>

#include "gtest/gtest.h"
#include "gtest/internal/gtest-port.h"

#if GTEST_HAS_FILE_SYSTEM

namespace {

class SetEnv {
 public:
  // Sets the environment value with name `name` to `value`, unless `value` is
  // nullptr, in which case it unsets it. Restores the original value on
  // destruction.
  SetEnv(const char* name, const char* value) : name_(name) {
    const char* old_value = getenv(name);
    if (old_value != nullptr) {
      saved_value_ = old_value;
      have_saved_value_ = true;
    }
    if (value == nullptr) {
      GTEST_CHECK_POSIX_SUCCESS_(unsetenv(name));
    } else {
      GTEST_CHECK_POSIX_SUCCESS_(setenv(name, value, 1 /*overwrite*/));
    }
  }

  ~SetEnv() {
    if (have_saved_value_) {
      GTEST_CHECK_POSIX_SUCCESS_(
          setenv(name_.c_str(), saved_value_.c_str(), 1 /*overwrite*/));
    } else {
      GTEST_CHECK_POSIX_SUCCESS_(unsetenv(name_.c_str()));
    }
  }

 private:
  std::string name_;
  bool have_saved_value_ = false;
  std::string saved_value_;
};

class MakeTempDir {
 public:
  // Creates a directory with a unique name including `testname`.
  // The destructor removes it.
  explicit MakeTempDir(const std::string& testname) {
    // mkdtemp requires that the last 6 characters of the input pattern
    // are Xs, and the string is modified by replacing those characters.
    std::string pattern = "/tmp/" + testname + "_XXXXXX";
    GTEST_CHECK_(mkdtemp(pattern.data()) != nullptr);
    dirname_ = pattern;
  }

  ~MakeTempDir() { GTEST_CHECK_POSIX_SUCCESS_(rmdir(dirname_.c_str())); }

  const char* DirName() const { return dirname_.c_str(); }

 private:
  std::string dirname_;
};

bool StartsWith(const std::string& str, const std::string& prefix) {
  return str.substr(0, prefix.size()) == prefix;
}

TEST(TempDirTest, InEnvironment) {
  // Since the test infrastructure might be verifying directory existence or
  // even creating subdirectories, we need to be careful that the directories we
  // specify are actually valid.
  MakeTempDir temp_dir("TempDirTest_InEnvironment");
  SetEnv set_env("TEST_TMPDIR", temp_dir.DirName());
  EXPECT_TRUE(StartsWith(testing::TempDir(), temp_dir.DirName()));
}

TEST(TempDirTest, NotInEnvironment) {
  SetEnv set_env("TEST_TMPDIR", nullptr);
  EXPECT_NE(testing::TempDir(), "");
}

TEST(SrcDirTest, InEnvironment) {
  // Since the test infrastructure might be verifying directory existence or
  // even creating subdirectories, we need to be careful that the directories we
  // specify are actually valid.
  MakeTempDir temp_dir("SrcDirTest_InEnvironment");
  SetEnv set_env("TEST_SRCDIR", temp_dir.DirName());
  EXPECT_TRUE(StartsWith(testing::SrcDir(), temp_dir.DirName()));
}

TEST(SrcDirTest, NotInEnvironment) {
  SetEnv set_env("TEST_SRCDIR", nullptr);
  EXPECT_NE(testing::SrcDir(), "");
}

#endif  // GTEST_HAS_FILE_SYSTEM

}  // namespace
