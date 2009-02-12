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
//
// Author: wan@google.com (Zhanyong Wan)

// Google Mock - a framework for writing C++ mock classes.
//
// This file defines some utilities useful for implementing Google
// Mock.  They are subject to change without notice, so please DO NOT
// USE THEM IN USER CODE.

#include <gmock/internal/gmock-internal-utils.h>

#include <ctype.h>
#include <ostream>  // NOLINT
#include <string>
#include <gmock/gmock.h>
#include <gmock/internal/gmock-port.h>
#include <gtest/gtest.h>

namespace testing {
namespace internal {

// Converts an identifier name to a space-separated list of lower-case
// words.  Each maximum substring of the form [A-Za-z][a-z]*|\d+ is
// treated as one word.  For example, both "FooBar123" and
// "foo_bar_123" are converted to "foo bar 123".
string ConvertIdentifierNameToWords(const char* id_name) {
  string result;
  char prev_char = '\0';
  for (const char* p = id_name; *p != '\0'; prev_char = *(p++)) {
    // We don't care about the current locale as the input is
    // guaranteed to be a valid C++ identifier name.
    const bool starts_new_word = isupper(*p) ||
        (!isalpha(prev_char) && islower(*p)) ||
        (!isdigit(prev_char) && isdigit(*p));

    if (isalnum(*p)) {
      if (starts_new_word && result != "")
        result += ' ';
      result += tolower(*p);
    }
  }
  return result;
}

// This class reports Google Mock failures as Google Test failures.  A
// user can define another class in a similar fashion if he intends to
// use Google Mock with a testing framework other than Google Test.
class GoogleTestFailureReporter : public FailureReporterInterface {
 public:
  virtual void ReportFailure(FailureType type, const char* file, int line,
                             const string& message) {
    AssertHelper(type == FATAL ? TPRT_FATAL_FAILURE : TPRT_NONFATAL_FAILURE,
                 file, line, message.c_str()) = Message();
    if (type == FATAL) {
      abort();
    }
  }
};

// Returns the global failure reporter.  Will create a
// GoogleTestFailureReporter and return it the first time called.
FailureReporterInterface* GetFailureReporter() {
  // Points to the global failure reporter used by Google Mock.  gcc
  // guarantees that the following use of failure_reporter is
  // thread-safe.  We may need to add additional synchronization to
  // protect failure_reporter if we port Google Mock to other
  // compilers.
  static FailureReporterInterface* const failure_reporter =
      new GoogleTestFailureReporter();
  return failure_reporter;
}

// Protects global resources (stdout in particular) used by Log().
static Mutex g_log_mutex(Mutex::NO_CONSTRUCTOR_NEEDED_FOR_STATIC_MUTEX);

// Prints the given message to stdout iff 'severity' >= the level
// specified by the --gmock_verbose flag.  If stack_frames_to_skip >=
// 0, also prints the stack trace excluding the top
// stack_frames_to_skip frames.  In opt mode, any positive
// stack_frames_to_skip is treated as 0, since we don't know which
// function calls will be inlined by the compiler and need to be
// conservative.
void Log(LogSeverity severity, const string& message,
         int stack_frames_to_skip) {
  if (GMOCK_FLAG(verbose) == kErrorVerbosity) {
    // The user is not interested in logs.
    return;
  } else if (GMOCK_FLAG(verbose) != kInfoVerbosity) {
    // The user is interested in warnings but not informational logs.
    // Note that invalid values of GMOCK_FLAG(verbose) are treated as
    // "warning", which is the default value of the flag.
    if (severity == INFO) {
      return;
    }
  }

  // Ensures that logs from different threads don't interleave.
  MutexLock l(&g_log_mutex);
  using ::std::cout;
  if (severity == WARNING) {
    // Prints a GMOCK WARNING marker to make the warnings easily searchable.
    cout << "\nGMOCK WARNING:";
  }
  // Pre-pends a new-line to message if it doesn't start with one.
  if (message.empty() || message[0] != '\n') {
    cout << "\n";
  }
  cout << message;
  if (stack_frames_to_skip >= 0) {
#ifdef NDEBUG
    // In opt mode, we have to be conservative and skip no stack frame.
    const int actual_to_skip = 0;
#else
    // In dbg mode, we can do what the caller tell us to do (plus one
    // for skipping this function's stack frame).
    const int actual_to_skip = stack_frames_to_skip + 1;
#endif  // NDEBUG

    // Appends a new-line to message if it doesn't end with one.
    if (!message.empty() && *message.rbegin() != '\n') {
      cout << "\n";
    }
    cout << "Stack trace:\n"
         << ::testing::internal::GetCurrentOsStackTraceExceptTop(
             ::testing::UnitTest::GetInstance(), actual_to_skip);
  }
  cout << ::std::flush;
}

}  // namespace internal
}  // namespace testing
