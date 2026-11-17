// Copyright 2026, Google Inc.
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

// The Google C++ Testing and Mocking Framework (Google Test)
//
// This file defines the OsStackTraceGetterInterface, which lets users replace
// the component that collects OS stack traces for test failures.

#ifndef GOOGLETEST_INCLUDE_GTEST_GTEST_OS_STACK_TRACE_H_
#define GOOGLETEST_INCLUDE_GTEST_GTEST_OS_STACK_TRACE_H_

#include <string>

#include "gtest/internal/gtest-port.h"

namespace testing {

// The role interface for getting the OS stack trace as a string.
class GTEST_API_ OsStackTraceGetterInterface {
 public:
  OsStackTraceGetterInterface() = default;
  virtual ~OsStackTraceGetterInterface() = default;

  // Returns the current OS stack trace as an std::string.  Parameters:
  //
  //   max_depth  - the maximum number of stack frames to be included
  //                in the trace.
  //   skip_count - the number of top frames to be skipped; doesn't count
  //                against max_depth.
  virtual std::string CurrentStackTrace(int max_depth, int skip_count) = 0;

  // UponLeavingGTest() should be called immediately before Google Test calls
  // user code. It saves some information about the current stack that
  // CurrentStackTrace() will use to find and hide Google Test stack frames.
  virtual void UponLeavingGTest() = 0;

  // This string is inserted in place of stack frames that are part of
  // Google Test's implementation.
  static const char* const kElidedFramesMarker;

 private:
  OsStackTraceGetterInterface(const OsStackTraceGetterInterface&) = delete;
  OsStackTraceGetterInterface& operator=(const OsStackTraceGetterInterface&) =
      delete;
};

}  // namespace testing

#endif  // GOOGLETEST_INCLUDE_GTEST_GTEST_OS_STACK_TRACE_H_
