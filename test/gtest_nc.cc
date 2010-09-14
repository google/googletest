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

// This file is the input to a negative-compilation test for Google
// Test.  Code here is NOT supposed to compile.  Its purpose is to
// verify that certain incorrect usages of the Google Test API are
// indeed rejected by the compiler.
//
// We still need to write the negative-compilation test itself, which
// will be tightly coupled with the build environment.
//
// TODO(wan@google.com): finish the negative-compilation test.

#ifdef TEST_CANNOT_IGNORE_RUN_ALL_TESTS_RESULT
// Tests that the result of RUN_ALL_TESTS() cannot be ignored.

#include "gtest/gtest.h"

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  RUN_ALL_TESTS();  // This line shouldn't compile.
}

#elif defined(TEST_USER_CANNOT_INCLUDE_GTEST_INTERNAL_INL_H)
// Tests that a user cannot include gtest-internal-inl.h in his code.

#include "src/gtest-internal-inl.h"

#elif defined(TEST_CATCHES_DECLARING_SETUP_IN_TEST_FIXTURE_WITH_TYPO)
// Tests that the compiler catches the typo when a user declares a
// Setup() method in a test fixture.

#include "gtest/gtest.h"

class MyTest : public testing::Test {
 protected:
  void Setup() {}
};

#elif defined(TEST_CATCHES_CALLING_SETUP_IN_TEST_WITH_TYPO)
// Tests that the compiler catches the typo when a user calls Setup()
// from a test fixture.

#include "gtest/gtest.h"

class MyTest : public testing::Test {
 protected:
  virtual void SetUp() {
    testing::Test::Setup();  // Tries to call SetUp() in the parent class.
  }
};

#elif defined(TEST_CATCHES_DECLARING_SETUP_IN_ENVIRONMENT_WITH_TYPO)
// Tests that the compiler catches the typo when a user declares a
// Setup() method in a subclass of Environment.

#include "gtest/gtest.h"

class MyEnvironment : public testing::Environment {
 public:
  void Setup() {}
};

#elif defined(TEST_CATCHES_CALLING_SETUP_IN_ENVIRONMENT_WITH_TYPO)
// Tests that the compiler catches the typo when a user calls Setup()
// in an Environment.

#include "gtest/gtest.h"

class MyEnvironment : public testing::Environment {
 protected:
  virtual void SetUp() {
    // Tries to call SetUp() in the parent class.
    testing::Environment::Setup();
  }
};

#elif defined(TEST_CATCHES_WRONG_CASE_IN_TYPED_TEST_P)
// Tests that the compiler catches using the wrong test case name in
// TYPED_TEST_P.

#include "gtest/gtest.h"

template <typename T>
class FooTest : public testing::Test {
};

template <typename T>
class BarTest : public testing::Test {
};

TYPED_TEST_CASE_P(FooTest);
TYPED_TEST_P(BarTest, A) {}  // Wrong test case name.
REGISTER_TYPED_TEST_CASE_P(FooTest, A);
INSTANTIATE_TYPED_TEST_CASE_P(My, FooTest, testing::Types<int>);

#elif defined(TEST_CATCHES_WRONG_CASE_IN_REGISTER_TYPED_TEST_CASE_P)
// Tests that the compiler catches using the wrong test case name in
// REGISTER_TYPED_TEST_CASE_P.

#include "gtest/gtest.h"

template <typename T>
class FooTest : public testing::Test {
};

template <typename T>
class BarTest : public testing::Test {
};

TYPED_TEST_CASE_P(FooTest);
TYPED_TEST_P(FooTest, A) {}
REGISTER_TYPED_TEST_CASE_P(BarTest, A);  // Wrong test case name.
INSTANTIATE_TYPED_TEST_CASE_P(My, FooTest, testing::Types<int>);

#elif defined(TEST_CATCHES_WRONG_CASE_IN_INSTANTIATE_TYPED_TEST_CASE_P)
// Tests that the compiler catches using the wrong test case name in
// INSTANTIATE_TYPED_TEST_CASE_P.

#include "gtest/gtest.h"

template <typename T>
class FooTest : public testing::Test {
};

template <typename T>
class BarTest : public testing::Test {
};

TYPED_TEST_CASE_P(FooTest);
TYPED_TEST_P(FooTest, A) {}
REGISTER_TYPED_TEST_CASE_P(FooTest, A);

// Wrong test case name.
INSTANTIATE_TYPED_TEST_CASE_P(My, BarTest, testing::Types<int>);

#elif defined(TEST_CATCHES_INSTANTIATE_TYPED_TESET_CASE_P_WITH_SAME_NAME_PREFIX)
// Tests that the compiler catches instantiating TYPED_TEST_CASE_P
// twice with the same name prefix.

#include "gtest/gtest.h"

template <typename T>
class FooTest : public testing::Test {
};

TYPED_TEST_CASE_P(FooTest);
TYPED_TEST_P(FooTest, A) {}
REGISTER_TYPED_TEST_CASE_P(FooTest, A);

INSTANTIATE_TYPED_TEST_CASE_P(My, FooTest, testing::Types<int>);

// Wrong name prefix: "My" has been used.
INSTANTIATE_TYPED_TEST_CASE_P(My, FooTest, testing::Types<double>);

#elif defined(TEST_STATIC_ASSERT_TYPE_EQ_IS_NOT_A_TYPE)

#include "gtest/gtest.h"

// Tests that StaticAssertTypeEq<T1, T2> cannot be used as a type.
testing::StaticAssertTypeEq<int, int> dummy;

#elif defined(TEST_STATIC_ASSERT_TYPE_EQ_WORKS_IN_NAMESPACE)

#include "gtest/gtest.h"

// Tests that StaticAssertTypeEq<T1, T2> works in a namespace scope.
static bool dummy = testing::StaticAssertTypeEq<int, const int>();

#elif defined(TEST_STATIC_ASSERT_TYPE_EQ_WORKS_IN_CLASS)

#include "gtest/gtest.h"

template <typename T>
class Helper {
 public:
  // Tests that StaticAssertTypeEq<T1, T2> works in a class.
  Helper() { testing::StaticAssertTypeEq<int, T>(); }

  void DoSomething() {}
};

void Test() {
  Helper<bool> h;
  h.DoSomething();  // To avoid the "unused variable" warning.
}

#elif defined(TEST_STATIC_ASSERT_TYPE_EQ_WORKS_IN_FUNCTION)

#include "gtest/gtest.h"

void Test() {
  // Tests that StaticAssertTypeEq<T1, T2> works inside a function.
  testing::StaticAssertTypeEq<const int, int>();
}

#else
// A sanity test.  This should compile.

#include "gtest/gtest.h"

int main() {
  return RUN_ALL_TESTS();
}

#endif
