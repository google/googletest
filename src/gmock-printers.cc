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
// This file implements a universal value printer that can print a
// value of any type T:
//
//   void ::testing::internal::UniversalPrinter<T>::Print(value, ostream_ptr);
//
// It uses the << operator when possible, and prints the bytes in the
// object otherwise.  A user can override its behavior for a class
// type Foo by defining either operator<<(::std::ostream&, const Foo&)
// or void PrintTo(const Foo&, ::std::ostream*) in the namespace that
// defines Foo.

#include <gmock/gmock-printers.h>
#include <ctype.h>
#include <stdio.h>
#include <ostream>  // NOLINT
#include <string>
#include <gmock/internal/gmock-port.h>

namespace testing {

namespace {

using ::std::ostream;

#ifdef _WIN32_WCE
#define snprintf _snprintf
#elif GTEST_OS_WINDOWS
#define snprintf _snprintf_s
#endif

// Prints a segment of bytes in the given object.
void PrintByteSegmentInObjectTo(const unsigned char* obj_bytes, size_t start,
                                size_t count, ostream* os) {
  char text[5] = "";
  for (size_t i = 0; i != count; i++) {
    const size_t j = start + i;
    if (i != 0) {
      // Organizes the bytes into groups of 2 for easy parsing by
      // human.
      if ((j % 2) == 0) {
        *os << " ";
      }
    }
    snprintf(text, sizeof(text), "%02X", obj_bytes[j]);
    *os << text;
  }
}

// Prints the bytes in the given value to the given ostream.
void PrintBytesInObjectToImpl(const unsigned char* obj_bytes, size_t count,
                              ostream* os) {
  // Tells the user how big the object is.
  *os << count << "-byte object <";

  const size_t kThreshold = 132;
  const size_t kChunkSize = 64;
  // If the object size is bigger than kThreshold, we'll have to omit
  // some details by printing only the first and the last kChunkSize
  // bytes.
  // TODO(wan): let the user control the threshold using a flag.
  if (count < kThreshold) {
    PrintByteSegmentInObjectTo(obj_bytes, 0, count, os);
  } else {
    PrintByteSegmentInObjectTo(obj_bytes, 0, kChunkSize, os);
    *os << " ... ";
    // Rounds up to 2-byte boundary.
    const size_t resume_pos = (count - kChunkSize + 1)/2*2;
    PrintByteSegmentInObjectTo(obj_bytes, resume_pos, count - resume_pos, os);
  }
  *os << ">";
}

}  // namespace

namespace internal2 {

// Delegates to PrintBytesInObjectToImpl() to print the bytes in the
// given object.  The delegation simplifies the implementation, which
// uses the << operator and thus is easier done outside of the
// ::testing::internal namespace, which contains a << operator that
// sometimes conflicts with the one in STL.
void PrintBytesInObjectTo(const unsigned char* obj_bytes, size_t count,
                          ostream* os) {
  PrintBytesInObjectToImpl(obj_bytes, count, os);
}

}  // namespace internal2

namespace internal {

// Prints a wide char as a char literal without the quotes, escaping it
// when necessary.
static void PrintAsWideCharLiteralTo(wchar_t c, ostream* os) {
  switch (c) {
    case L'\0':
      *os << "\\0";
      break;
    case L'\'':
      *os << "\\'";
      break;
    case L'\?':
      *os << "\\?";
      break;
    case L'\\':
      *os << "\\\\";
      break;
    case L'\a':
      *os << "\\a";
      break;
    case L'\b':
      *os << "\\b";
      break;
    case L'\f':
      *os << "\\f";
      break;
    case L'\n':
      *os << "\\n";
      break;
    case L'\r':
      *os << "\\r";
      break;
    case L'\t':
      *os << "\\t";
      break;
    case L'\v':
      *os << "\\v";
      break;
    default:
      // Checks whether c is printable or not. Printable characters are in
      // the range [0x20,0x7E].
      // We test the value of c directly instead of calling isprint(), as
      // isprint() is buggy on Windows mobile.
      if (0x20 <= c && c <= 0x7E) {
        *os << static_cast<char>(c);
      } else {
        // Buffer size enough for the maximum number of digits and \0.
        char text[2 * sizeof(unsigned long) + 1] = "";
        snprintf(text, sizeof(text), "%lX", static_cast<unsigned long>(c));
        *os << "\\x" << text;
      }
  }
}

// Prints a char as if it's part of a string literal, escaping it when
// necessary.
static void PrintAsWideStringLiteralTo(wchar_t c, ostream* os) {
  switch (c) {
    case L'\'':
      *os << "'";
      break;
    case L'"':
      *os << "\\\"";
      break;
    default:
      PrintAsWideCharLiteralTo(c, os);
  }
}

// Prints a char as a char literal without the quotes, escaping it
// when necessary.
static void PrintAsCharLiteralTo(char c, ostream* os) {
  PrintAsWideCharLiteralTo(static_cast<unsigned char>(c), os);
}

// Prints a char as if it's part of a string literal, escaping it when
// necessary.
static void PrintAsStringLiteralTo(char c, ostream* os) {
  PrintAsWideStringLiteralTo(static_cast<unsigned char>(c), os);
}

// Prints a char and its code.  The '\0' char is printed as "'\\0'",
// other unprintable characters are also properly escaped using the
// standard C++ escape sequence.
void PrintCharTo(char c, int char_code, ostream* os) {
  *os << "'";
  PrintAsCharLiteralTo(c, os);
  *os << "'";
  if (c != '\0')
    *os << " (" << char_code << ")";
}

// Prints a wchar_t as a symbol if it is printable or as its internal
// code otherwise and also as its decimal code (except for L'\0').
// The L'\0' char is printed as "L'\\0'". The decimal code is printed
// as signed integer when wchar_t is implemented by the compiler
// as a signed type and is printed as an unsigned integer when wchar_t
// is implemented as an unsigned type.
void PrintTo(wchar_t wc, ostream* os) {
  *os << "L'";
  PrintAsWideCharLiteralTo(wc, os);
  *os << "'";
  if (wc != L'\0') {
    // Type Int64 is used because it provides more storage than wchar_t thus
    // when the compiler converts signed or unsigned implementation of wchar_t
    // to Int64 it fills higher bits with either zeros or the sign bit
    // passing it to operator <<() as either signed or unsigned integer.
    *os << " (" << static_cast<Int64>(wc) << ")";
  }
}

// Prints the given array of characters to the ostream.
// The array starts at *begin, the length is len, it may include '\0' characters
// and may not be null-terminated.
static void PrintCharsAsStringTo(const char* begin, size_t len, ostream* os) {
  *os << "\"";
  for (size_t index = 0; index < len; ++index) {
    PrintAsStringLiteralTo(begin[index], os);
  }
  *os << "\"";
}

// Prints a (const) char array of 'len' elements, starting at address 'begin'.
void UniversalPrintArray(const char* begin, size_t len, ostream* os) {
  PrintCharsAsStringTo(begin, len, os);
}

// Prints the given array of wide characters to the ostream.
// The array starts at *begin, the length is len, it may include L'\0'
// characters and may not be null-terminated.
static void PrintWideCharsAsStringTo(const wchar_t* begin, size_t len,
                                     ostream* os) {
  *os << "L\"";
  for (size_t index = 0; index < len; ++index) {
    PrintAsWideStringLiteralTo(begin[index], os);
  }
  *os << "\"";
}

// Prints the given C string to the ostream.
void PrintTo(const char* s, ostream* os) {
  if (s == NULL) {
    *os << "NULL";
  } else {
    *os << implicit_cast<const void*>(s) << " pointing to ";
    PrintCharsAsStringTo(s, strlen(s), os);
  }
}

// MSVC compiler can be configured to define whar_t as a typedef
// of unsigned short. Defining an overload for const wchar_t* in that case
// would cause pointers to unsigned shorts be printed as wide strings,
// possibly accessing more memory than intended and causing invalid
// memory accesses. MSVC defines _NATIVE_WCHAR_T_DEFINED symbol when
// wchar_t is implemented as a native type.
#if !defined(_MSC_VER) || defined(_NATIVE_WCHAR_T_DEFINED)
// Prints the given wide C string to the ostream.
void PrintTo(const wchar_t* s, ostream* os) {
  if (s == NULL) {
    *os << "NULL";
  } else {
    *os << implicit_cast<const void*>(s) << " pointing to ";
    PrintWideCharsAsStringTo(s, wcslen(s), os);
  }
}
#endif  // wchar_t is native

// Prints a ::string object.
#if GTEST_HAS_GLOBAL_STRING
void PrintStringTo(const ::string& s, ostream* os) {
  PrintCharsAsStringTo(s.data(), s.size(), os);
}
#endif  // GTEST_HAS_GLOBAL_STRING

#if GTEST_HAS_STD_STRING
void PrintStringTo(const ::std::string& s, ostream* os) {
  PrintCharsAsStringTo(s.data(), s.size(), os);
}
#endif  // GTEST_HAS_STD_STRING

// Prints a ::wstring object.
#if GTEST_HAS_GLOBAL_WSTRING
void PrintWideStringTo(const ::wstring& s, ostream* os) {
  PrintWideCharsAsStringTo(s.data(), s.size(), os);
}
#endif  // GTEST_HAS_GLOBAL_WSTRING

#if GTEST_HAS_STD_WSTRING
void PrintWideStringTo(const ::std::wstring& s, ostream* os) {
  PrintWideCharsAsStringTo(s.data(), s.size(), os);
}
#endif  // GTEST_HAS_STD_WSTRING

}  // namespace internal

}  // namespace testing
