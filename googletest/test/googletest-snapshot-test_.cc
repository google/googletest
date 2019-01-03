// Copyright 2018, Google Inc.
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


// Tests Google Test's EXPECT_EQ_SNAPSHOT.
//
// This program will be invoked by googletest-snapshot-test.py.

#include "gtest/gtest.h"

#include <regex>
#include <vector>

std::vector<std::string> split(const std::string& text, const std::regex& re) {
  const std::vector<std::string> parts(
    std::sregex_token_iterator(text.begin(), text.end(), re, -1),
    std::sregex_token_iterator());
  return parts;
}

// The Unit Under Test
std::string greeter(const std::string &whom) {
  return "Hello " + whom;
}

// Parameters to test
std::vector<std::string> GetParamsToTest() {
  GTEST_DISABLE_MSC_DEPRECATED_PUSH_(/* getenv: deprecated */)
  const char *c_params = getenv("GREETERTEST_PARAMS");
  GTEST_DISABLE_MSC_DEPRECATED_POP_()

  std::string params = c_params ? c_params : "";
  return split(params, std::regex("\\s+"));
}

class GreeterTest : public ::testing::TestWithParam<std::string> {
    // nothing
};

TEST_P(GreeterTest, GreeterTest) {
  std::string p = GetParam();
  EXPECT_EQ_SNAPSHOT(greeter(p));
}

INSTANTIATE_TEST_CASE_P(GreeterTestWithParams,
    GreeterTest,
    ::testing::ValuesIn(GetParamsToTest()));

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
