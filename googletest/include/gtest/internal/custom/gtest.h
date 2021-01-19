// Copyright 2015, Google Inc.
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
// Injection point for custom user configurations. See README for details
//
// ** Custom implementation starts here **

#ifndef GTEST_INCLUDE_GTEST_INTERNAL_CUSTOM_GTEST_H_
#define GTEST_INCLUDE_GTEST_INTERNAL_CUSTOM_GTEST_H_

#include "gtest/gtest.h"
#include "third_party/googletest/googletest/src/gtest-internal-inl.h"

#if GTEST_GOOGLE3_MODE_

#define GTEST_REMOVE_LEGACY_TEST_CASEAPI_ 1

#include <string>
#include <vector>

namespace testing {
namespace internal {

// In google3 we use ::InitGoogle instead.
#define GTEST_CUSTOM_INIT_GOOGLE_TEST_FUNCTION_ \
  ::testing::internal::InitGoogleTestForGoogle3
void InitGoogleTestForGoogle3(int* argc, char** argv);
void InitGoogleTestForGoogle3(int* argc, wchar_t** argv);

#define GTEST_OS_STACK_TRACE_GETTER_ \
  ::testing::internal::Google3OsStackTraceGetter

// Google3 implementation of the stack trace getter.
class Google3OsStackTraceGetter : public OsStackTraceGetterInterface {
 public:
  Google3OsStackTraceGetter() : caller_frame_(nullptr) {}

  std::string CurrentStackTrace(int max_depth, int skip_count) override
      GTEST_LOCK_EXCLUDED_(mutex_);

  void UponLeavingGTest() override GTEST_LOCK_EXCLUDED_(mutex_);

 private:
  Mutex mutex_;  // protects all internal state

  // We save the stack frame below the frame that calls user code.
  // We do this because the address of the frame immediately below
  // the user code changes between the call to UponLeavingGTest()
  // and any calls to CurrentStackTrace() from within the user code.
  void* caller_frame_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(Google3OsStackTraceGetter);
};

#define GTEST_CUSTOM_TEST_EVENT_LISTENER_ \
  ::testing::internal::GoogleProcessStateListener

// Report process state changes to Google3 base, after the unit test
// has been initialized, and before it is torn down, as well as at
// the start and end of each test case and test.
class GoogleProcessStateListener : public EmptyTestEventListener {
 public:
  GoogleProcessStateListener() {}
  ~GoogleProcessStateListener() override;

  void OnEnvironmentsSetUpEnd(const UnitTest& unit_test) override;
  void OnEnvironmentsTearDownStart(const UnitTest& unit_test) override;
  void OnTestSuiteStart(const TestSuite& test_case) override;
  void OnTestSuiteEnd(const TestSuite& test_case) override;
  void OnTestStart(const TestInfo& test_info) override;
  void OnTestEnd(const TestInfo& test_info) override;

 private:
  GTEST_DISALLOW_COPY_AND_ASSIGN_(GoogleProcessStateListener);
};  // class GoogleProcessStateListener


// For KilledBySignal.
bool KilledBySignalOverride(int signum, int exit_status, bool* result);

#define GTEST_KILLED_BY_SIGNAL_OVERRIDE_ \
  ::testing::internal::KilledBySignalOverride

// Override --debugger_command (if any) with an empty one:
// we don't want the child to have a debugger automagically attaching
// to it when it (expectedly) dies.
// Also, enable --logtostderr.
::std::vector<std::string> GetGoogle3DeathTestCommandLineArgs();

#define GTEST_EXTRA_DEATH_TEST_COMMAND_LINE_ARGS_() \
  ::testing::internal::GetGoogle3DeathTestCommandLineArgs()


// For b/11021341, disable (slow) AddressToLineNumberDecorator.
void Google3DeathTestChildSetup();

#define GTEST_EXTRA_DEATH_TEST_CHILD_SETUP_() \
  ::testing::internal::Google3DeathTestChildSetup()

// returns temp directory for test in Google3 manner
std::string GetGoogle3TestTempDir();

// In google3 we use ::testing:GetTestTmpdir() instead.
#define GTEST_CUSTOM_TEMPDIR_FUNCTION_ \
  ::testing::internal::GetGoogle3TestTempDir

}  // namespace internal

}  // namespace testing
#endif  // GTEST_GOOGLE3_MODE_

#endif  // GTEST_INCLUDE_GTEST_INTERNAL_CUSTOM_GTEST_H_
