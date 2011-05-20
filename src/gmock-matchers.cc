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

// Google Mock - a framework for writing C++ mock classes.
//
// This file implements Matcher<const string&>, Matcher<string>, and
// utilities for defining matchers.

#include "gmock/gmock-matchers.h"
#include "gmock/gmock-generated-matchers.h"

#include <string.h>
#include <sstream>
#include <string>

namespace testing {

// Constructs a matcher that matches a const string& whose value is
// equal to s.
Matcher<const internal::string&>::Matcher(const internal::string& s) {
  *this = Eq(s);
}

// Constructs a matcher that matches a const string& whose value is
// equal to s.
Matcher<const internal::string&>::Matcher(const char* s) {
  *this = Eq(internal::string(s));
}

// Constructs a matcher that matches a string whose value is equal to s.
Matcher<internal::string>::Matcher(const internal::string& s) { *this = Eq(s); }

// Constructs a matcher that matches a string whose value is equal to s.
Matcher<internal::string>::Matcher(const char* s) {
  *this = Eq(internal::string(s));
}

namespace internal {

// Joins a vector of strings as if they are fields of a tuple; returns
// the joined string.
GTEST_API_ string JoinAsTuple(const Strings& fields) {
  switch (fields.size()) {
    case 0:
      return "";
    case 1:
      return fields[0];
    default:
      string result = "(" + fields[0];
      for (size_t i = 1; i < fields.size(); i++) {
        result += ", ";
        result += fields[i];
      }
      result += ")";
      return result;
  }
}

// Returns the description for a matcher defined using the MATCHER*()
// macro where the user-supplied description string is "", if
// 'negation' is false; otherwise returns the description of the
// negation of the matcher.  'param_values' contains a list of strings
// that are the print-out of the matcher's parameters.
GTEST_API_ string FormatMatcherDescription(bool negation,
                                           const char* matcher_name,
                                           const Strings& param_values) {
  string result = ConvertIdentifierNameToWords(matcher_name);
  if (param_values.size() >= 1)
    result += " " + JoinAsTuple(param_values);
  return negation ? "not (" + result + ")" : result;
}

}  // namespace internal
}  // namespace testing
