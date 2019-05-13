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
//
// This file tests EXPECT_EQ works when used with type that has overloaded &
// operator.

#include <gtest/gtest.h>
#include <string>

class string_ref;

/**
 * This is a synthetic pointer to a fixed size string.
 */
class string_ptr {
 public:
  string_ptr(const char *data, size_t size) : data_(data), size_(size) {}

  string_ptr &operator++() noexcept {
    data_ += size_;
    return *this;
  }

  string_ref operator*() const noexcept;

 private:
  const char *data_;
  size_t size_;
};

/**
 * This is a synthetic reference of a fixed size string.
 */
class string_ref {
 public:
  string_ref(const char *data, size_t size) : data_(data), size_(size) {}

  string_ptr operator&() const noexcept { return {data_, size_}; }

  bool operator==(const char *s) const noexcept {
    if (size_ > 0 && data_[size_ - 1] != 0) {
      return std::string(data_, size_) == std::string(s);
    } else {
      return std::string(data_) == std::string(s);
    }
  }

 private:
  const char *data_;
  size_t size_;
};

string_ref string_ptr::operator*() const noexcept { return {data_, size_}; }

TEST(string_ref, compare) {
  const char *s = "alex\0davidjohn\0";
  string_ptr ptr(s, 5);
  EXPECT_EQ(*ptr, "alex");
  EXPECT_TRUE(*ptr == "alex");
  ++ptr;
  EXPECT_EQ(*ptr, "david");
  EXPECT_TRUE(*ptr == "david");
  ++ptr;
  EXPECT_EQ(*ptr, "john");
}
