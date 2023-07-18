// (c)2023 Sony Interactive Entertainment Inc
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
//     * Neither the name of Sony Interactive Entertainment Inc. nor the
// names of its contributors may be used to endorse or promote products
// derived from this software without specific prior written permission.
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

// Interface to the RGT code parser, used by the rest of RGT.
// Identifies TEST functions, with source text ranges.

#ifndef GTEST_INCLUDE_GTEST_INTERNAL_GTEST_RGT_PARSER_H_
#define GTEST_INCLUDE_GTEST_INTERNAL_GTEST_RGT_PARSER_H_

#include <cassert>
#include <cstring>
#include <string>

#include "gtest/internal/gtest-port.h"

#if GTEST_HAS_RGT

namespace testing {
namespace internal {

// Utility class that refers to a (maybe not 0-terminated) const char
// array and its length. It does not own the storage.
class StringRef {
  const char *data_;
  size_t length_;
public:
  // Default to no string.
  StringRef() : data_(nullptr), length_(0) {}

  // Construct from a null-terminated C string.
  StringRef(const char *s) : data_(s) {
    assert(s);
    length_ = ::strlen(s);
  }

  // Construct with user-provided length; this is not
  // necessarily null-terminated.
  StringRef(const char *s, size_t l) : data_(s), length_(l) {
    assert(s);
  }

  const char *Data() const { return data_; }
  size_t Length() const { return length_; }
  bool Empty() const { return length_ == 0; }
  bool Equals(StringRef rhs) const {
    return length_ == rhs.Length() &&
        (length_ == 0 || ::strncmp(data_, rhs.Data(), length_) == 0);
  }
  bool Equals(const char *rhs) const {
    if (data_ == nullptr || rhs == nullptr)
      return data_ == rhs;
    return Equals(StringRef(rhs, ::strlen(rhs)));
  }
};

// The range of lines occupied by a function.
struct LineRange {
  int start;
  int end;
};

// Information about a particular Test function.
struct RgtTestInfo {
  StringRef test_kind;
  LineRange lines;
};

// Implementation details of the parser.
class RgtParserImpl;

// Read/lex/parse a source file, looking for information helpful to
// reporting Rotten Green Tests. This is a very simplistic lex/parse
// and in particular doesn't do any preprocessor operations, other than
// handling #if directives in a very naive way.
// The intent is to have one instance per source file.
class GTEST_API_ RgtParser {
  RgtParserImpl *impl_;
  bool parsed_;
public:
  RgtParser();
  ~RgtParser();

  // Returns true if file was accessible, false otherwise.
  bool Parsed() { return parsed_; }

  // Read and parse the file, if possible.
  void Parse(const char *file_path);

  // Get info about the test method and the given line in this file.
  const RgtTestInfo *FindTest(int line) const;

  // If there's an EXPECT_[NON]FATAL_FAILURE at this line, return the
  // line where that ends. Otherwise return line.
  int GetAssertEndLine(int line) const;

  // Methods for internal testing only. Not for use by main RGT code.

  // Parse the provided string.
  void ParseString(StringRef str);
  // Return the token stream.
  void PrintTokens(std::string &out) const;
};

} // end namespace internal
} // end namespace testing

#endif // GTEST_HAS_RGT

#endif  // GTEST_INCLUDE_GTEST_INTERNAL_GTEST_RGT_PARSER_H_
