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
// Author: vladl@google.com (Vlad Losev)

// Google Mock - a framework for writing C++ mock classes.
//
// This file tests the internal cross-platform support utilities.

#include <gmock/internal/gmock-port.h>
#include <gtest/gtest.h>

TEST(GmockCheckSyntaxTest, BehavesLikeASingleStatement) {
  if (false)
    GMOCK_CHECK_(false) << "This should never be executed; "
                           "It's a compilation test only.";

  if (true)
    GMOCK_CHECK_(true);
  else
    ;

  if (false)
    ;
  else
    GMOCK_CHECK_(true) << "";
}

TEST(GmockCheckSyntaxTest, WorksWithSwitch) {
  switch (0) {
    case 1:
      break;
    default:
      GMOCK_CHECK_(true);
  }

  switch(0)
    case 0:
      GMOCK_CHECK_(true) << "Check failed in switch case";
}

#if GTEST_HAS_DEATH_TEST

TEST(GmockCheckDeathTest, DiesWithCorrectOutputOnFailure) {
  const bool a_false_condition = false;
  // MSVC and gcc use different formats to print source file locations.
  // Google Mock's failure messages use the same format as used by the
  // compiler, in order for the IDE to recognize them.  Therefore we look
  // for different patterns here depending on the compiler.
  const char regex[] =
#ifdef _MSC_VER
     "gmock-port_test\\.cc\\(\\d+\\):"
#else
     "gmock-port_test\\.cc:[0-9]+"
#endif  // _MSC_VER
     ".*a_false_condition.*Extra info";

  EXPECT_DEATH(GMOCK_CHECK_(a_false_condition) << "Extra info", regex);
}

TEST(GmockCheckDeathTest, LivesSilentlyOnSuccess) {
  EXPECT_EXIT({
      GMOCK_CHECK_(true) << "Extra info";
      ::std::cerr << "Success\n";
      exit(0); },
      ::testing::ExitedWithCode(0), "Success");
}

#endif  // GTEST_HAS_DEATH_TEST
