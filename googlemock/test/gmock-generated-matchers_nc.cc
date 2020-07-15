// Copyright 2009, Google Inc.
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
// Google Mock matchers.

#include "gmock/gmock.h"

#if defined(TEST_WRONG_ARG_TYPE_IN_MATCHER_MACRO)

// Tests using an MATCHER definition to match a value of an
// incompatible type.
MATCHER(WrongArgType, "") { return 10/arg > 2; }

void Test() {
  testing::Matcher<const char*> m = WrongArgType();
}

#elif defined(TEST_MATCHER_MACRO_IN_CLASS)

// Tests using MATCHER in a class scope.
class Foo {
 public:
  // This won't compile as C++ doesn't allow defining a method of a
  // nested class out-of-line in the enclosing class.
  MATCHER(Bar, "") { return arg > 0; }
};

#elif defined(TEST_MATCHER_MACRO_IN_FUNCTION)

// Tests using MATCHER in a function body.
void Test() {
  // This won't compile as C++ doesn't allow member templates in local
  // classes.  We may want to revisit this when C++0x is widely
  // implemented.
  MATCHER(Bar, "") { return arg > 0; }
}

#else

// Sanity check - this should compile.

#endif
