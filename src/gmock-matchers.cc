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

#include <gmock/gmock-matchers.h>
#include <gmock/gmock-generated-matchers.h>

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

// Utilities for validating and formatting description strings in the
// MATCHER*() macros.

// Returns the 0-based index of the given parameter in the
// NULL-terminated parameter array; if the parameter is "*", returns
// kTupleInterpolation; if it's not found in the list, returns
// kInvalidInterpolation.
int GetParamIndex(const char* param_names[], const string& param_name) {
  if (param_name == "*")
    return kTupleInterpolation;

  for (int i = 0; param_names[i] != NULL; i++) {
    if (param_name == param_names[i])
      return i;
  }
  return kInvalidInterpolation;
}

// If *pstr starts with the given prefix, modifies *pstr to be right
// past the prefix and returns true; otherwise leaves *pstr unchanged
// and returns false.  None of pstr, *pstr, and prefix can be NULL.
bool SkipPrefix(const char* prefix, const char** pstr) {
  const size_t prefix_len = strlen(prefix);
  if (strncmp(*pstr, prefix, prefix_len) == 0) {
    *pstr += prefix_len;
    return true;
  }
  return false;
}

// Helper function used by ValidateMatcherDescription() to format
// error messages.
string FormatMatcherDescriptionSyntaxError(const char* description,
                                           const char* error_pos) {
  ::std::stringstream ss;
  ss << "Syntax error at index " << (error_pos - description)
     << " in matcher description \"" << description << "\": ";
  return ss.str();
}

// Parses a matcher description string and returns a vector of
// interpolations that appear in the string; generates non-fatal
// failures iff 'description' is an invalid matcher description.
// 'param_names' is a NULL-terminated array of parameter names in the
// order they appear in the MATCHER_P*() parameter list.
Interpolations ValidateMatcherDescription(
    const char* param_names[], const char* description) {
  Interpolations interps;
  for (const char* p = description; *p != '\0';) {
    if (SkipPrefix("%%", &p)) {
      interps.push_back(Interpolation(p - 2, p, kPercentInterpolation));
    } else if (SkipPrefix("%(", &p)) {
      const char* const q = strstr(p, ")s");
      if (q == NULL) {
        // TODO(wan@google.com): change the source file location in
        // the failure to point to where the MATCHER*() macro is used.
        ADD_FAILURE() << FormatMatcherDescriptionSyntaxError(description, p - 2)
                      << "an interpolation must end with \")s\", "
                      << "but \"" << (p - 2) << "\" does not.";
      } else {
        const string param_name(p, q);
        const int param_index = GetParamIndex(param_names, param_name);
        if (param_index == kInvalidInterpolation) {
          ADD_FAILURE() << FormatMatcherDescriptionSyntaxError(description, p)
                        << "\"" << param_name
                        << "\" is an invalid parameter name.";
        } else {
          interps.push_back(Interpolation(p - 2, q + 2, param_index));
          p = q + 2;
        }
      }
    } else {
      EXPECT_NE(*p, '%') << FormatMatcherDescriptionSyntaxError(description, p)
                         << "use \"%%\" instead of \"%\" to print \"%\".";
      ++p;
    }
  }
  return interps;
}

// Joins a vector of strings as if they are fields of a tuple; returns
// the joined string.
string JoinAsTuple(const Strings& fields) {
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

// Returns the actual matcher description, given the matcher name,
// user-supplied description template string, interpolations in the
// string, and the printed values of the matcher parameters.
string FormatMatcherDescription(
    const char* matcher_name, const char* description,
    const Interpolations& interp, const Strings& param_values) {
  string result;
  if (*description == '\0') {
    // When the user supplies an empty description, we calculate one
    // from the matcher name.
    result = ConvertIdentifierNameToWords(matcher_name);
    if (param_values.size() >= 1)
      result += " " + JoinAsTuple(param_values);
  } else {
    // The end position of the last interpolation.
    const char* last_interp_end = description;
    for (size_t i = 0; i < interp.size(); i++) {
      result.append(last_interp_end, interp[i].start_pos);
      const int param_index = interp[i].param_index;
      if (param_index == kTupleInterpolation) {
        result += JoinAsTuple(param_values);
      } else if (param_index == kPercentInterpolation) {
        result += '%';
      } else if (param_index != kInvalidInterpolation) {
        result += param_values[param_index];
      }
      last_interp_end = interp[i].end_pos;
    }
    result += last_interp_end;
  }

  return result;
}

}  // namespace internal
}  // namespace testing
