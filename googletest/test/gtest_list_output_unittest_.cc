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
//
// Author: david.schuldenfrei@gmail.com (David Schuldenfrei)

// Unit test for Google Test's --gtest_list_tests and --gtest_output flag.
//
// A user can ask Google Test to list all tests that will run,
// and have the output saved in a Json/Xml file.
// The tests will not be run after listing.
//
// This program will be invoked from a Python unit test.
// Don't run it directly.

#include "gtest/gtest.h"

TEST(FooTest, Test1) {}

TEST(FooTest, Test2) {}

class FooTestFixture : public ::testing::Test {};
TEST_F(FooTestFixture, Test3) {}
TEST_F(FooTestFixture, Test4) {}

class ValueParamTest : public ::testing::TestWithParam<int> {};
TEST_P(ValueParamTest, Test5) {}
TEST_P(ValueParamTest, Test6) {}
INSTANTIATE_TEST_SUITE_P(ValueParam, ValueParamTest, ::testing::Values(33, 42));

#if GTEST_HAS_TYPED_TEST
template <typename T>
class TypedTest : public ::testing::Test {};
typedef testing::Types<int, bool> TypedTestTypes;
TYPED_TEST_SUITE(TypedTest, TypedTestTypes);
TYPED_TEST(TypedTest, Test7) {}
TYPED_TEST(TypedTest, Test8) {}
#endif

#if GTEST_HAS_TYPED_TEST_P
template <typename T>
class TypeParameterizedTestSuite : public ::testing::Test {};
TYPED_TEST_SUITE_P(TypeParameterizedTestSuite);
TYPED_TEST_P(TypeParameterizedTestSuite, Test9) {}
TYPED_TEST_P(TypeParameterizedTestSuite, Test10) {}
REGISTER_TYPED_TEST_SUITE_P(TypeParameterizedTestSuite, Test9, Test10);
typedef testing::Types<int, bool> TypeParameterizedTestSuiteTypes;  // NOLINT
INSTANTIATE_TYPED_TEST_SUITE_P(Single, TypeParameterizedTestSuite,
                               TypeParameterizedTestSuiteTypes);
#endif

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
