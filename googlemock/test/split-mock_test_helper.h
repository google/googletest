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

// A test helper file declaring a mock class whose methods are defined in
// a .cc file.

#ifndef GOOGLEMOCK_TEST_GMOCK_LINK_TEST_H_
#define GOOGLEMOCK_TEST_GMOCK_LINK_TEST_H_

#include "gmock/gmock-function-mocker.h"

struct MockSplitDeclarationAndDefinitionBase {
  virtual int func_inherited(int) = 0;
};

struct MockSplitDeclarationAndDefinition
    : MockSplitDeclarationAndDefinitionBase {
  MockSplitDeclarationAndDefinition();
  ~MockSplitDeclarationAndDefinition();

  DECLARE_MOCK_METHOD(int, func, (int));
  DECLARE_MOCK_METHOD(int, func_const, (int), (const));

  DECLARE_MOCK_METHOD(int, func_overloaded, ());
  DECLARE_MOCK_METHOD(int, func_overloaded, (int));
  DECLARE_MOCK_METHOD(int, func_overloaded, (int), (const));
  DECLARE_MOCK_METHOD(int, func_overloaded, (int, int), (ref(&)));
  DECLARE_MOCK_METHOD(int, func_overloaded, (int, int), (const, ref(&&)));

  DECLARE_MOCK_METHOD2(func_legacy, int(int, int));
  DECLARE_MOCK_CONST_METHOD1(func_legacy_const, int(int));

  DECLARE_MOCK_METHOD(int, func_inherited, (int), (override final));

  using NESTED_TYPEDEF = int;
  DECLARE_MOCK_METHOD(NESTED_TYPEDEF, func_nested_typedef, (NESTED_TYPEDEF));
};

#endif  // GOOGLEMOCK_TEST_SPLIT_MOCK_TEST_HELPER_H_
