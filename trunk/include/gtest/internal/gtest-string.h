// Copyright 2005, Google Inc.
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
// Authors: wan@google.com (Zhanyong Wan), eefacm@gmail.com (Sean Mcafee)
//
// The Google C++ Testing Framework (Google Test)
//
// This header file declares the String class and functions used internally by
// Google Test.  They are subject to change without notice. They should not used
// by code external to Google Test.
//
// This header file is #included by <gtest/internal/gtest-internal.h>.
// It should not be #included by other files.

#ifndef GTEST_INCLUDE_GTEST_INTERNAL_GTEST_STRING_H_
#define GTEST_INCLUDE_GTEST_INTERNAL_GTEST_STRING_H_

#include <string.h>
#include <gtest/internal/gtest-port.h>

#if GTEST_HAS_GLOBAL_STRING || GTEST_HAS_STD_STRING
#include <string>
#endif  // GTEST_HAS_GLOBAL_STRING || GTEST_HAS_STD_STRING

namespace testing {
namespace internal {

// Holds data in a String object.  We need this class in order to put
// String's data members on the heap instead of on the stack.
// Otherwise tests using many assertions (and thus Strings) in one
// function may need too much stack frame space to compile.
class StringData {
  StringData() : c_str_(NULL), length_(0) {}
  ~StringData() { delete[] c_str_; }

 private:
  friend class String;

  const char* c_str_;
  size_t length_;  // Length of the string (excluding the terminating
                   // '\0' character).
};

// String - a UTF-8 string class.
//
// We cannot use std::string as Microsoft's STL implementation in
// Visual C++ 7.1 has problems when exception is disabled.  There is a
// hack to work around this, but we've seen cases where the hack fails
// to work.
//
// Also, String is different from std::string in that it can represent
// both NULL and the empty string, while std::string cannot represent
// NULL.
//
// NULL and the empty string are considered different.  NULL is less
// than anything (including the empty string) except itself.
//
// This class only provides minimum functionality necessary for
// implementing Google Test.  We do not intend to implement a full-fledged
// string class here.
//
// Since the purpose of this class is to provide a substitute for
// std::string on platforms where it cannot be used, we define a copy
// constructor and assignment operators such that we don't need
// conditional compilation in a lot of places.
//
// In order to make the representation efficient, the d'tor of String
// is not virtual.  Therefore DO NOT INHERIT FROM String.
class String {
 public:
  // Static utility methods

  // Returns the input enclosed in double quotes if it's not NULL;
  // otherwise returns "(null)".  For example, "\"Hello\"" is returned
  // for input "Hello".
  //
  // This is useful for printing a C string in the syntax of a literal.
  //
  // Known issue: escape sequences are not handled yet.
  static String ShowCStringQuoted(const char* c_str);

  // Clones a 0-terminated C string, allocating memory using new.  The
  // caller is responsible for deleting the return value using
  // delete[].  Returns the cloned string, or NULL if the input is
  // NULL.
  //
  // This is different from strdup() in string.h, which allocates
  // memory using malloc().
  static const char* CloneCString(const char* c_str);

#ifdef _WIN32_WCE
  // Windows CE does not have the 'ANSI' versions of Win32 APIs. To be
  // able to pass strings to Win32 APIs on CE we need to convert them
  // to 'Unicode', UTF-16.

  // Creates a UTF-16 wide string from the given ANSI string, allocating
  // memory using new. The caller is responsible for deleting the return
  // value using delete[]. Returns the wide string, or NULL if the
  // input is NULL.
  //
  // The wide string is created using the ANSI codepage (CP_ACP) to
  // match the behaviour of the ANSI versions of Win32 calls and the
  // C runtime.
  static LPCWSTR AnsiToUtf16(const char* c_str);

  // Creates an ANSI string from the given wide string, allocating
  // memory using new. The caller is responsible for deleting the return
  // value using delete[]. Returns the ANSI string, or NULL if the
  // input is NULL.
  //
  // The returned string is created using the ANSI codepage (CP_ACP) to
  // match the behaviour of the ANSI versions of Win32 calls and the
  // C runtime.
  static const char* Utf16ToAnsi(LPCWSTR utf16_str);
#endif

  // Compares two C strings.  Returns true iff they have the same content.
  //
  // Unlike strcmp(), this function can handle NULL argument(s).  A
  // NULL C string is considered different to any non-NULL C string,
  // including the empty string.
  static bool CStringEquals(const char* lhs, const char* rhs);

  // Converts a wide C string to a String using the UTF-8 encoding.
  // NULL will be converted to "(null)".  If an error occurred during
  // the conversion, "(failed to convert from wide string)" is
  // returned.
  static String ShowWideCString(const wchar_t* wide_c_str);

  // Similar to ShowWideCString(), except that this function encloses
  // the converted string in double quotes.
  static String ShowWideCStringQuoted(const wchar_t* wide_c_str);

  // Compares two wide C strings.  Returns true iff they have the same
  // content.
  //
  // Unlike wcscmp(), this function can handle NULL argument(s).  A
  // NULL C string is considered different to any non-NULL C string,
  // including the empty string.
  static bool WideCStringEquals(const wchar_t* lhs, const wchar_t* rhs);

  // Compares two C strings, ignoring case.  Returns true iff they
  // have the same content.
  //
  // Unlike strcasecmp(), this function can handle NULL argument(s).
  // A NULL C string is considered different to any non-NULL C string,
  // including the empty string.
  static bool CaseInsensitiveCStringEquals(const char* lhs,
                                           const char* rhs);

  // Compares two wide C strings, ignoring case.  Returns true iff they
  // have the same content.
  //
  // Unlike wcscasecmp(), this function can handle NULL argument(s).
  // A NULL C string is considered different to any non-NULL wide C string,
  // including the empty string.
  // NB: The implementations on different platforms slightly differ.
  // On windows, this method uses _wcsicmp which compares according to LC_CTYPE
  // environment variable. On GNU platform this method uses wcscasecmp
  // which compares according to LC_CTYPE category of the current locale.
  // On MacOS X, it uses towlower, which also uses LC_CTYPE category of the
  // current locale.
  static bool CaseInsensitiveWideCStringEquals(const wchar_t* lhs,
                                               const wchar_t* rhs);

  // Formats a list of arguments to a String, using the same format
  // spec string as for printf.
  //
  // We do not use the StringPrintf class as it is not universally
  // available.
  //
  // The result is limited to 4096 characters (including the tailing
  // 0).  If 4096 characters are not enough to format the input,
  // "<buffer exceeded>" is returned.
  static String Format(const char* format, ...);

  // C'tors

  // The default c'tor constructs a NULL string, which is represented
  // by data_ being NULL.
  String() : data_(NULL) {}

  // Constructs a String by cloning a 0-terminated C string.
  String(const char* c_str) {  // NOLINT
    if (c_str == NULL) {
      data_ = NULL;
    } else {
      ConstructNonNull(c_str, strlen(c_str));
    }
  }

  // Constructs a String by copying a given number of chars from a
  // buffer.  E.g. String("hello", 3) creates the string "hel",
  // String("a\0bcd", 4) creates "a\0bc", String(NULL, 0) creates "",
  // and String(NULL, 1) results in access violation.
  String(const char* buffer, size_t length) {
    ConstructNonNull(buffer, length);
  }

  // The copy c'tor creates a new copy of the string.  The two
  // String objects do not share content.
  String(const String& str) : data_(NULL) { *this = str; }

  // D'tor.  String is intended to be a final class, so the d'tor
  // doesn't need to be virtual.
  ~String() {
    delete data_;
  }

  // Allows a String to be implicitly converted to an ::std::string or
  // ::string, and vice versa.  Converting a String containing a NULL
  // pointer to ::std::string or ::string is undefined behavior.
  // Converting a ::std::string or ::string containing an embedded NUL
  // character to a String will result in the prefix up to the first
  // NUL character.
#if GTEST_HAS_STD_STRING
  String(const ::std::string& str) {
    ConstructNonNull(str.c_str(), str.length());
  }

  operator ::std::string() const { return ::std::string(c_str(), length()); }
#endif  // GTEST_HAS_STD_STRING

#if GTEST_HAS_GLOBAL_STRING
  String(const ::string& str) {
    ConstructNonNull(str.c_str(), str.length());
  }

  operator ::string() const { return ::string(c_str(), length()); }
#endif  // GTEST_HAS_GLOBAL_STRING

  // Returns true iff this is an empty string (i.e. "").
  bool empty() const { return (c_str() != NULL) && (length() == 0); }

  // Compares this with another String.
  // Returns < 0 if this is less than rhs, 0 if this is equal to rhs, or > 0
  // if this is greater than rhs.
  int Compare(const String& rhs) const;

  // Returns true iff this String equals the given C string.  A NULL
  // string and a non-NULL string are considered not equal.
  bool operator==(const char* c_str) const { return Compare(c_str) == 0; }

  // Returns true iff this String is less than the given String.  A
  // NULL string is considered less than "".
  bool operator<(const String& rhs) const { return Compare(rhs) < 0; }

  // Returns true iff this String doesn't equal the given C string.  A NULL
  // string and a non-NULL string are considered not equal.
  bool operator!=(const char* c_str) const { return !(*this == c_str); }

  // Returns true iff this String ends with the given suffix.  *Any*
  // String is considered to end with a NULL or empty suffix.
  bool EndsWith(const char* suffix) const;

  // Returns true iff this String ends with the given suffix, not considering
  // case. Any String is considered to end with a NULL or empty suffix.
  bool EndsWithCaseInsensitive(const char* suffix) const;

  // Returns the length of the encapsulated string, or 0 if the
  // string is NULL.
  size_t length() const { return (data_ == NULL) ? 0 : data_->length_; }

  // Gets the 0-terminated C string this String object represents.
  // The String object still owns the string.  Therefore the caller
  // should NOT delete the return value.
  const char* c_str() const { return (data_ == NULL) ? NULL : data_->c_str_; }

  // Assigns a C string to this object.  Self-assignment works.
  const String& operator=(const char* c_str) { return *this = String(c_str); }

  // Assigns a String object to this object.  Self-assignment works.
  const String& operator=(const String& rhs) {
    if (this != &rhs) {
      delete data_;
      data_ = NULL;
      if (rhs.data_ != NULL) {
        ConstructNonNull(rhs.data_->c_str_, rhs.data_->length_);
      }
    }

    return *this;
  }

 private:
  // Constructs a non-NULL String from the given content.  This
  // function can only be called when data_ has not been allocated.
  // ConstructNonNull(NULL, 0) results in an empty string ("").
  // ConstructNonNull(NULL, non_zero) is undefined behavior.
  void ConstructNonNull(const char* buffer, size_t length) {
    data_ = new StringData;
    char* const str = new char[length + 1];
    memcpy(str, buffer, length);
    str[length] = '\0';
    data_->c_str_ = str;
    data_->length_ = length;
  }

  // Points to the representation of the String.  A NULL String is
  // represented by data_ == NULL.
  StringData* data_;
};  // class String

// Streams a String to an ostream.  Each '\0' character in the String
// is replaced with "\\0".
inline ::std::ostream& operator<<(::std::ostream& os, const String& str) {
  if (str.c_str() == NULL) {
    os << "(null)";
  } else {
    const char* const c_str = str.c_str();
    for (size_t i = 0; i != str.length(); i++) {
      if (c_str[i] == '\0') {
        os << "\\0";
      } else {
        os << c_str[i];
      }
    }
  }
  return os;
}

// Gets the content of the StrStream's buffer as a String.  Each '\0'
// character in the buffer is replaced with "\\0".
String StrStreamToString(StrStream* stream);

// Converts a streamable value to a String.  A NULL pointer is
// converted to "(null)".  When the input value is a ::string,
// ::std::string, ::wstring, or ::std::wstring object, each NUL
// character in it is replaced with "\\0".

// Declared here but defined in gtest.h, so that it has access
// to the definition of the Message class, required by the ARM
// compiler.
template <typename T>
String StreamableToString(const T& streamable);

}  // namespace internal
}  // namespace testing

#endif  // GTEST_INCLUDE_GTEST_INTERNAL_GTEST_STRING_H_
