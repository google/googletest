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


// Google Mock - a framework for writing C++ mock classes.
//
// This file contains negative compilation tests for script-generated
// Google Mock actions.

#include "gmock/gmock.h"

using testing::Action;
using testing::Invoke;
using testing::WithArgs;

int Nullary() { return 0; }
int Binary(int a, int b) { return a + b; }

#if defined(TEST_NULLARY_WITH_ARGS)

// Tests that WithArgs(action) doesn't compile.
void Test() {
  Action<int(int)> a = WithArgs(Invoke(Nullary));
}

#elif defined(TEST_TOO_FEW_ARGS_FOR_WITH_ARGS)

// Tests that you cannot pass too few arguments to the inner action in
// WithArgs().
void Test() {
  Action<int(int, int, int)> a = WithArgs<1>(Invoke(Binary));
}

#elif defined(TEST_TOO_MANY_ARGS_FOR_WITH_ARGS)

// Tests that you cannot pass too many arguments to the inner action in
// WithArgs().
void Test() {
  Action<int(int, int, int)> a = WithArgs<1, 2, 0>(Invoke(Binary));
}

#elif defined(TEST_INCOMPATIBLE_ARG_TYPES_FOR_WITH_ARGS)

// Tests that you cannot pass arguments of incompatible types to the
// inner action in WithArgs().
void Test() {
  Action<int(int, const char*, int)> a = WithArgs<1, 2>(Invoke(Binary));
}

#elif defined(TEST_WRONG_ARG_TYPE_IN_ACTION_MACRO)

// Tests using an ACTION definition in a mock function whose argument
// types are incompatible.
ACTION(WrongArgType) { return 10/arg0; }

void Test() {
  Action<int(const char*)> a = WrongArgType();
}

#elif defined(TEST_WRONG_RETURN_TYPE_IN_ACTION_MACRO)

// Tests using an ACTION definition in a mock function whose return
// types is incompatible.
ACTION(WrongReturnType) { return 10; }

void Test() {
  Action<const char*()> a = WrongReturnType();
}

#elif defined(TEST_EXCESSIVE_ARG_IN_ACTION_MACRO)

// Tests using an ACTION definition in a mock function that doesn't
// provide enough arguments.
ACTION(UseExcessiveArg) { return arg0 + arg1; }

void Test() {
  Action<int(int)> a = UseExcessiveArg();
}

#elif defined(TEST_ACTION_MACRO_IN_CLASS)

// Tests using ACTION in a class scope.
class Foo {
 public:
  // This won't compile as C++ doesn't allow defining a method of a
  // nested class out-of-line in the enclosing class.
  ACTION(Bar) { return arg0; }
};

#elif defined(TEST_ACTION_MACRO_IN_FUNCTION)

// Tests using ACTION in a function body.
void Test() {
  // This won't compile as C++ doesn't allow member templates in local
  // classes.  We may want to revisit this when C++0x is widely
  // implemented.
  ACTION(Bar) { return arg0; }
}

#elif defined(TEST_SET_ARG_REFEREE_MUST_BE_USED_WITH_REFERENCE)

// Verifies that using SetArgReferee<k>(...) where the k-th argument
// of the mock function is not a reference generates a compiler error.
void Test() {
  Action<void(bool, int)> a = testing::SetArgReferee<1>(5);
}

#elif defined(TEST_DELETE_ARG_MUST_BE_USED_WITH_POINTER)

// Verifies that using DeleteArg<k>(...) where the k-th argument of the mock
// function is not a pointer generates a compiler error.
void Test() {
  Action<void(int)> a = testing::DeleteArg<0>();  // NOLINT
}

#elif defined(TEST_CANNOT_OVERLOAD_ACTION_TEMPLATE_ON_TEMPLATE_PARAM_NUMBER)

// Tests that ACTION_TEMPLATE cannot be overloaded on the number of
// template parameters alone.

ACTION_TEMPLATE(OverloadedAction,
                HAS_1_TEMPLATE_PARAMS(typename, T),
                AND_1_VALUE_PARAMS(p)) {}

ACTION_TEMPLATE(OverloadedAction,
                HAS_2_TEMPLATE_PARAMS(typename, T1, typename, T2),
                AND_1_VALUE_PARAMS(p)) {}

#elif defined(TEST_CANNOT_OVERLOAD_ACTION_AND_ACTION_TEMPLATE_W_SAME_VALUE_PS)

// Tests that ACTION_TEMPLATE and ACTION_P cannot be overloaded when
// they have the same number of value parameters.

ACTION_P(OverloadedAction, p) {}

ACTION_TEMPLATE(OverloadedAction,
                HAS_1_TEMPLATE_PARAMS(typename, T),
                AND_1_VALUE_PARAMS(p)) {}

#else

// Sanity check - this should compile.

#endif
