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


#include "gmock/gmock.h"

bool Overloaded(int n) { return n > 0; }
bool Overloaded(double x) { return x > 0; }

class Foo {
 public:
  virtual ~Foo() {}
  virtual int IntFunc(bool* p) = 0;
  virtual void VoidFunc(int n) = 0;
  virtual int& GetIntRef() = 0;
  virtual Foo* GetPointer() = 0;
  virtual int Do(void (*fp)()) = 0;
};

class MockFoo : public Foo {
 public:
  MOCK_METHOD1(IntFunc, int(bool* p));
  MOCK_METHOD1(VoidFunc, void(int n));
  MOCK_METHOD0(GetIntRef, int&());
  MOCK_METHOD0(GetPointer, Foo*());
  MOCK_METHOD1(Do, int(void (*fp)()));
};

class Bar {
 public:
  virtual ~Bar() {}
  int Overloaded() { return 1; }
  virtual void Overloaded(int n) {}
};

#if defined(TEST_MOP)

using ::testing::_;
using ::testing::Return;

// Tests that Google Mock Doctor can diagnose the Mock Object Pointer
// disease.
void Test() {
  MockFoo foo;
  ON_CALL(&foo, IntFunc(_)).WillByDefault(Return(0));
  EXPECT_CALL(&foo, VoidFunc(_));
}

#elif defined(TEST_NRS1)

using ::testing::_;
using ::testing::SetArgPointee;

// Tests that Google Mock Doctor can diagnose the Need to Return
// Something disease.
void Test() {
  MockFoo foo;
  EXPECT_CALL(foo, IntFunc(_))
      .WillOnce(SetArgPointee<0>(true));
}

#elif defined(TEST_NRS2)

using ::testing::_;
using ::testing::Return;

// Tests that Google Mock Doctor can diagnose the Need to Return
// Something disease.
void Test() {
  MockFoo foo;
  EXPECT_CALL(foo, IntFunc(_))
      .WillOnce(Return());
}

#elif defined(TEST_NRS3)

using ::testing::_;
using ::testing::InvokeArgument;

// Tests that Google Mock Doctor can diagnose the Need to Return
// Something disease.
void Test() {
  MockFoo foo;
  EXPECT_CALL(foo, Do(_))
      .WillOnce(InvokeArgument<0>());
}

#elif defined(TEST_IBRA)

// Tests that Google Mock Doctor can diagnose the Incomplete
// By-Reference Argument Type disease.

class Incomplete;

class MockBar {
 public:
  MOCK_METHOD1(ByRefFunc, void(const Incomplete&));
};

void Test() {
  MockBar bar;
}

#elif defined(TEST_OFM)

// Tests that Google Mock Doctor can diagnose the Overloaded Function
// Matcher disease.
void Test() {
  using ::testing::Matcher;
  using ::testing::Truly;

  Matcher<int> m = Truly(Overloaded);
}

#elif defined(TEST_NO_NUS_FOR_NON_GMOCK_SYMBOL)

// Tests that Google Mock Doctor doesn't report the Need to Use Symbol
// disease when the undeclared symbol is not from Google Mock.
void Test() {
  MockFoo foo;
  EXPECT_CALL(foo, IntFunc(NonGMockMatcher()));
}

#elif defined(TEST_NUS_VARIABLE)

// Tests that Google Mock Doctor can diagnose the Need to Use Symbol
// disease when the undeclared symbol is a variable.
void Test() {
  MockFoo foo;
  EXPECT_CALL(foo, IntFunc(_));
}

#elif defined(TEST_NUS_FUNCTION)

// Tests that Google Mock Doctor can diagnose the Need to Use Symbol
// disease when the undeclared symbol is a function.
void Test() {
  MockFoo foo;
  EXPECT_CALL(foo, IntFunc(NULL))
      .Times(AtLeast(1));
}

#elif defined(TEST_NUS_FUNCTION_TEMPLATE)

// Tests that Google Mock Doctor can diagnose the Need to Use Symbol
// disease when the undeclared symbol is a function template with no
// explicit template argument.
void Test() {
  MockFoo foo;
  EXPECT_CALL(foo, IntFunc(NULL))
      .WillOnce(Return(1));
}

#elif defined(TEST_NUS_FUNCTION_TEMPLATE_WITH_TYPE_ARG)

// Tests that Google Mock Doctor can diagnose the Need to Use Symbol
// disease when the undeclared symbol is a function template with an
// explicit template type argument.
void Test() {
  MockFoo foo;
  EXPECT_CALL(foo, IntFunc(A<bool*>()));
}

#elif defined(TEST_NUS_FUNCTION_TEMPLATE_WITH_NONTYPE_ARG)

// Tests that Google Mock Doctor can diagnose the Need to Use Symbol
// disease when the undeclared symbol is a function template with an
// explicit template non-type argument.
using ::testing::_;

void Test() {
  MockFoo foo;
  int n;
  EXPECT_CALL(foo, VoidFunc(_)).WillOnce(SaveArg<0>(&n));
}

#elif defined(TEST_NUS_CLASS)

// Tests that Google Mock Doctor can diagnose the Need to Use Symbol
// disease when the undeclared symbol is a class.
void Test() {
  MockFoo foo;
  Sequence s;
  Mock::VerifyAndClear(&foo);
}

#elif defined(TEST_NRR)

using ::testing::Return;

// Tests that Google Mock Doctor can diagnose the Need to Return
// Reference disease (using Return() when ReturnRef() should be used).
void Test() {
  int n = 0;
  MockFoo foo;
  EXPECT_CALL(foo, GetIntRef())
      .WillOnce(Return(n));
}

#elif defined(TEST_MULTI_OCCURRENCES_OF_SAME_DISEASE)

// Tests that Google Mock Doctor can diagnose multiple occurrences of
// the same disease in the same code.

class Incomplete;
class Incomplete2;

class MockBar {
 public:
  MOCK_METHOD1(ByRefFunc, void(const Incomplete&));
  MOCK_METHOD1(ByRefFunc, void(const Incomplete2&));
};

MockBar bar;

#elif defined(TEST_NRNULL)

using ::testing::Return;

// Tests that gMock Doctor can diagnose the Need to use ReturnNull
// disease (using Return(NULL) when ReturnNull() should be used).
void Test() {
  MockFoo foo;
  EXPECT_CALL(foo, GetPointer())
      .WillOnce(Return(NULL));
}

#elif defined(TEST_WPP)

using ::testing::_;
using ::testing::Return;

// Tests that gMock doctor can diagnose the Wrong Parenthesis Position
// disease.
void Test() {
  MockFoo foo;

  ON_CALL(foo, IntFunc(_).WillByDefault(Return(0)));
  EXPECT_CALL(foo, VoidFunc(_).Times(1));
  EXPECT_CALL(foo, VoidFunc(_).WillOnce(Return()));
}

#elif defined(TEST_TTB)

template <typename T>
class Stack {
 public:
  typedef unsigned int SomeType;
};

// Tests that gMock doctor can diagnose the Type in Template Base
// disease.
template <typename T>
class MockStack : public Stack<T> {
 public:
  // typedef typename Stack<T>::SomeType SomeType; would fix the errors.

  // Uses a type from Stack<T> as the mock function's return type.
  MOCK_METHOD0_T(IsEmpty, SomeType());

  // Uses a type from Stack<T> as the sole parameter of the mock function.
  MOCK_CONST_METHOD1_T(IsOK1, bool(SomeType));

  // Uses a type from Stack<T> as one of the parameters of the mock function.
  MOCK_CONST_METHOD3_T(IsOK2, bool(int, int, SomeType));
};

#endif
