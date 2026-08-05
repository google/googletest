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

// Regression test for https://github.com/google/googletest/issues/4933.
//
// Verifies that GTEST_INTERNAL_HAS_COMPARE_LIB is set to 1 if and only if
// the standard feature test macro __cpp_lib_three_way_comparison is defined.
// The macro must NOT use a fallback based on the mere presence of the
// <compare> header and C++ language version, because some toolchains
// (e.g. Android NDK 24/25 with Clang 14) ship an incomplete <compare>
// header that lacks the comparison operators required by gtest-printers.

#include "gtest/internal/gtest-port.h"

#ifdef __cpp_lib_three_way_comparison
static_assert(
    GTEST_INTERNAL_HAS_COMPARE_LIB == 1,
    "GTEST_INTERNAL_HAS_COMPARE_LIB should be 1 when "
    "__cpp_lib_three_way_comparison is defined");
#else
static_assert(
    GTEST_INTERNAL_HAS_COMPARE_LIB == 0,
    "GTEST_INTERNAL_HAS_COMPARE_LIB should be 0 when "
    "__cpp_lib_three_way_comparison is not defined. "
    "See https://github.com/google/googletest/issues/4933");
#endif

int main() { return 0; }
