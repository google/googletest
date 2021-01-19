// Copyright 2015, Google Inc.
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
// This file provides an injection point for custom printers in a local
// installation of gTest.
// It will be included from gtest-printers.h and the overrides in this file
// will be visible to everyone.
//
// Injection point for custom user configurations. See README for details
//
// ** Custom implementation starts here **

#ifndef GTEST_INCLUDE_GTEST_INTERNAL_CUSTOM_GTEST_PRINTERS_H_
#define GTEST_INCLUDE_GTEST_INTERNAL_CUSTOM_GTEST_PRINTERS_H_
// absl:google3-begin(Custom Implementation)

#include "gtest/gtest-printers.h"

#if GTEST_GOOGLE3_MODE_
#include "absl/status/statusor.h"
#include "absl/strings/cord.h"
#include "util/task/codes.pb.h"
#include "util/task/codes.proto.h"
#include "util/task/status.h"

namespace testing {
namespace internal {

// Specializations for types related to util::Status.  We define these
// printers here to avoid widening the //util/task API.

template <>
class UniversalPrinter< ::util::ErrorSpace> {
 public:
  static void Print(const ::util::ErrorSpace& space, ::std::ostream* os) {
    *os << space.SpaceName();
  }
};

template <>
class UniversalPrinter< ::util::error::Code> {
 public:
  static void Print(::util::error::Code code, ::std::ostream* os) {
    if (::util::error::Code_IsValid(code)) {
      *os << ::util::error::Code_Name(code);
    } else {
      *os << static_cast<int>(code);
    }
  }
};

template <typename T>
class UniversalPrinter<absl::StatusOr<T> > {
 public:
  static void Print(const absl::StatusOr<T>& status_or, ::std::ostream* os) {
    *os << status_or.status();
    if (status_or.ok()) {
      *os << ": ";
      UniversalPrint(status_or.value(), os);
    }
  }
};

class FormatForComparisonAsStringImpl {
 public:
  static ::std::string Format(const char* value) {
    return ::testing::PrintToString(value);
  }
};

// If a C string is compared with a absl::string_view or Cord, we know it's
// meant to point to a NUL-terminated string, and thus can print it as a string.

template <>
class FormatForComparison<char*, absl::string_view>
    : public FormatForComparisonAsStringImpl {};
template <>
class FormatForComparison<const char*, absl::string_view>
    : public FormatForComparisonAsStringImpl {};

template <>
class FormatForComparison<char*, absl::Cord>
    : public FormatForComparisonAsStringImpl {};
template <>
class FormatForComparison<const char*, absl::Cord>
    : public FormatForComparisonAsStringImpl {};

}  // namespace internal
}  // namespace testing

#endif  // GTEST_GOOGLE3_MODE_
// absl:google3-end

#endif  // GTEST_INCLUDE_GTEST_INTERNAL_CUSTOM_GTEST_PRINTERS_H_
