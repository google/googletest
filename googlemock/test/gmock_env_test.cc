// Copyright 2025, Google Inc.
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

// Tests for the StringFromGMockEnv() function that enables setting
// verbosity level from the GMOCK_VERBOSE environment variable.

#include "gmock/gmock.h"
#include "gtest/gtest.h"

// The StringFromGMockEnv() function is declared in gmock-port.h and implemented
// in gmock.cc. We test it indirectly through the GMOCK_FLAG(verbose) which is
// initialized with this function.

namespace {

// Function to set environment variable for testing
void SetEnv(const char* name, const char* value) {
#ifdef _WIN32
  _putenv_s(name, value);
#else
  setenv(name, value, 1);
#endif
}

// Function to unset environment variable for testing
void UnsetEnv(const char* name) {
#ifdef _WIN32
  _putenv_s(name, "");
#else
  unsetenv(name);
#endif
}

// Test fixture for environment variable tests
class GMockVerbosityEnvTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Save the original verbose flag value
    original_verbose_ = GMOCK_FLAG_GET(verbose);

    // Unset the GMOCK_VERBOSE environment variable to start with a clean state
    UnsetEnv("GMOCK_VERBOSE");
  }

  void TearDown() override {
    // Restore the original verbose flag value
    GMOCK_FLAG_SET(verbose, original_verbose_);

    // Clean up the environment variable
    UnsetEnv("GMOCK_VERBOSE");
  }

  std::string original_verbose_;
};

// Tests that when GMOCK_VERBOSE is not set, the default value is used
TEST_F(GMockVerbosityEnvTest, DefaultValueWhenEnvNotSet) {
  // Re-initialize Google Mock to pick up the environment variable
  // This will call InitGoogleMock which will use StringFromGMockEnv()
  int argc = 1;
  const char* argv[] = {"test_program"};
  testing::InitGoogleMock(&argc, const_cast<char**>(argv));

  // The default value should be "warning"
  EXPECT_EQ(GMOCK_FLAG_GET(verbose), "warning");
}

// Tests that when GMOCK_VERBOSE is set to "info", that value is used
TEST_F(GMockVerbosityEnvTest, InfoValueWhenEnvSet) {
  // Set the environment variable
  SetEnv("GMOCK_VERBOSE", "info");

  // Re-initialize Google Mock to pick up the environment variable
  int argc = 1;
  const char* argv[] = {"test_program"};
  testing::InitGoogleMock(&argc, const_cast<char**>(argv));

  // The value should be "info" as set in the environment
  EXPECT_EQ(GMOCK_FLAG_GET(verbose), "info");
}

// Tests that when GMOCK_VERBOSE is set to "error", that value is used
TEST_F(GMockVerbosityEnvTest, ErrorValueWhenEnvSet) {
  // Set the environment variable
  SetEnv("GMOCK_VERBOSE", "error");

  // Re-initialize Google Mock to pick up the environment variable
  int argc = 1;
  const char* argv[] = {"test_program"};
  testing::InitGoogleMock(&argc, const_cast<char**>(argv));

  // The value should be "error" as set in the environment
  EXPECT_EQ(GMOCK_FLAG_GET(verbose), "error");
}

// Tests that command line flags take precedence over environment variables
TEST_F(GMockVerbosityEnvTest, CommandLineFlagOverridesEnv) {
  // Set the environment variable
  SetEnv("GMOCK_VERBOSE", "info");

  // Set up command line arguments with the --gmock_verbose flag
  int argc = 2;
  const char* argv[] = {"test_program", "--gmock_verbose=error"};
  testing::InitGoogleMock(&argc, const_cast<char**>(argv));

  // The value should be "error" from the command line, not "info" from the
  // environment
  EXPECT_EQ(GMOCK_FLAG_GET(verbose), "error");
}

} // namespace

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}