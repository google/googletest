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

// Google Test - The Google C++ Testing Framework
//
// This file tests the universal value printer.

#include "gtest/gtest-printers.h"

#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <algorithm>
#include <deque>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "gtest/gtest.h"

// hash_map and hash_set are available on Windows.
#if GTEST_OS_WINDOWS
#define GTEST_HAS_HASH_MAP_ 1  // Indicates that hash_map is available.
#include <hash_map>            // NOLINT
#define GTEST_HAS_HASH_SET_ 1  // Indicates that hash_set is available.
#include <hash_set>            // NOLINT
#endif  // GTEST_OS_WINDOWS

// Some user-defined types for testing the universal value printer.

// An anonymous enum type.
enum AnonymousEnum {
  kAE1 = -1,
  kAE2 = 1
};

// An enum without a user-defined printer.
enum EnumWithoutPrinter {
  kEWP1 = -2,
  kEWP2 = 42
};

// An enum with a << operator.
enum EnumWithStreaming {
  kEWS1 = 10,
};

std::ostream& operator<<(std::ostream& os, EnumWithStreaming e) {
  return os << (e == kEWS1 ? "kEWS1" : "invalid");
}

// An enum with a PrintTo() function.
enum EnumWithPrintTo {
  kEWPT1 = 1,
};

void PrintTo(EnumWithPrintTo e, std::ostream* os) {
  *os << (e == kEWPT1 ? "kEWPT1" : "invalid");
}

// A class implicitly convertible to BiggestInt.
class BiggestIntConvertible {
 public:
  operator ::testing::internal::BiggestInt() const { return 42; }
};

// A user-defined unprintable class template in the global namespace.
template <typename T>
class UnprintableTemplateInGlobal {
 public:
  UnprintableTemplateInGlobal() : value_() {}
 private:
  T value_;
};

// A user-defined streamable type in the global namespace.
class StreamableInGlobal {
 public:
  virtual ~StreamableInGlobal() {}
};

inline void operator<<(::std::ostream& os, const StreamableInGlobal& /* x */) {
  os << "StreamableInGlobal";
}

void operator<<(::std::ostream& os, const StreamableInGlobal* /* x */) {
  os << "StreamableInGlobal*";
}

namespace foo {

// A user-defined unprintable type in a user namespace.
class UnprintableInFoo {
 public:
  UnprintableInFoo() : z_(0) { memcpy(xy_, "\xEF\x12\x0\x0\x34\xAB\x0\x0", 8); }
 private:
  char xy_[8];
  double z_;
};

// A user-defined printable type in a user-chosen namespace.
struct PrintableViaPrintTo {
  PrintableViaPrintTo() : value() {}
  int value;
};

void PrintTo(const PrintableViaPrintTo& x, ::std::ostream* os) {
  *os << "PrintableViaPrintTo: " << x.value;
}

// A type with a user-defined << for printing its pointer.
struct PointerPrintable {
};

::std::ostream& operator<<(::std::ostream& os,
                           const PointerPrintable* /* x */) {
  return os << "PointerPrintable*";
}

// A user-defined printable class template in a user-chosen namespace.
template <typename T>
class PrintableViaPrintToTemplate {
 public:
  explicit PrintableViaPrintToTemplate(const T& a_value) : value_(a_value) {}

  const T& value() const { return value_; }
 private:
  T value_;
};

template <typename T>
void PrintTo(const PrintableViaPrintToTemplate<T>& x, ::std::ostream* os) {
  *os << "PrintableViaPrintToTemplate: " << x.value();
}

// A user-defined streamable class template in a user namespace.
template <typename T>
class StreamableTemplateInFoo {
 public:
  StreamableTemplateInFoo() : value_() {}

  const T& value() const { return value_; }
 private:
  T value_;
};

template <typename T>
inline ::std::ostream& operator<<(::std::ostream& os,
                                  const StreamableTemplateInFoo<T>& x) {
  return os << "StreamableTemplateInFoo: " << x.value();
}

}  // namespace foo

namespace testing {
namespace gtest_printers_test {

using ::std::deque;
using ::std::list;
using ::std::make_pair;
using ::std::map;
using ::std::multimap;
using ::std::multiset;
using ::std::pair;
using ::std::set;
using ::std::vector;
using ::testing::PrintToString;
using ::testing::internal::NativeArray;
using ::testing::internal::RE;
using ::testing::internal::Strings;
using ::testing::internal::UniversalTersePrint;
using ::testing::internal::UniversalPrint;
using ::testing::internal::UniversalTersePrintTupleFieldsToStrings;
using ::testing::internal::UniversalPrinter;
using ::testing::internal::kReference;
using ::testing::internal::string;

#if GTEST_HAS_TR1_TUPLE
using ::std::tr1::make_tuple;
using ::std::tr1::tuple;
#endif

#if GTEST_OS_WINDOWS
// MSVC defines the following classes in the ::stdext namespace while
// gcc defines them in the :: namespace.  Note that they are not part
// of the C++ standard.

using ::stdext::hash_map;
using ::stdext::hash_set;
using ::stdext::hash_multimap;
using ::stdext::hash_multiset;

#endif  // GTEST_OS_WINDOWS

// Prints a value to a string using the universal value printer.  This
// is a helper for testing UniversalPrinter<T>::Print() for various types.
template <typename T>
string Print(const T& value) {
  ::std::stringstream ss;
  UniversalPrinter<T>::Print(value, &ss);
  return ss.str();
}

// Prints a value passed by reference to a string, using the universal
// value printer.  This is a helper for testing
// UniversalPrinter<T&>::Print() for various types.
template <typename T>
string PrintByRef(const T& value) {
  ::std::stringstream ss;
  UniversalPrinter<T&>::Print(value, &ss);
  return ss.str();
}

// Tests printing various enum types.

TEST(PrintEnumTest, AnonymousEnum) {
  EXPECT_EQ("-1", Print(kAE1));
  EXPECT_EQ("1", Print(kAE2));
}

TEST(PrintEnumTest, EnumWithoutPrinter) {
  EXPECT_EQ("-2", Print(kEWP1));
  EXPECT_EQ("42", Print(kEWP2));
}

TEST(PrintEnumTest, EnumWithStreaming) {
  EXPECT_EQ("kEWS1", Print(kEWS1));
  EXPECT_EQ("invalid", Print(static_cast<EnumWithStreaming>(0)));
}

TEST(PrintEnumTest, EnumWithPrintTo) {
  EXPECT_EQ("kEWPT1", Print(kEWPT1));
  EXPECT_EQ("invalid", Print(static_cast<EnumWithPrintTo>(0)));
}

// Tests printing a class implicitly convertible to BiggestInt.

TEST(PrintClassTest, BiggestIntConvertible) {
  EXPECT_EQ("42", Print(BiggestIntConvertible()));
}

// Tests printing various char types.

// char.
TEST(PrintCharTest, PlainChar) {
  EXPECT_EQ("'\\0'", Print('\0'));
  EXPECT_EQ("'\\'' (39, 0x27)", Print('\''));
  EXPECT_EQ("'\"' (34, 0x22)", Print('"'));
  EXPECT_EQ("'\\?' (63, 0x3F)", Print('\?'));
  EXPECT_EQ("'\\\\' (92, 0x5C)", Print('\\'));
  EXPECT_EQ("'\\a' (7)", Print('\a'));
  EXPECT_EQ("'\\b' (8)", Print('\b'));
  EXPECT_EQ("'\\f' (12, 0xC)", Print('\f'));
  EXPECT_EQ("'\\n' (10, 0xA)", Print('\n'));
  EXPECT_EQ("'\\r' (13, 0xD)", Print('\r'));
  EXPECT_EQ("'\\t' (9)", Print('\t'));
  EXPECT_EQ("'\\v' (11, 0xB)", Print('\v'));
  EXPECT_EQ("'\\x7F' (127)", Print('\x7F'));
  EXPECT_EQ("'\\xFF' (255)", Print('\xFF'));
  EXPECT_EQ("' ' (32, 0x20)", Print(' '));
  EXPECT_EQ("'a' (97, 0x61)", Print('a'));
}

// signed char.
TEST(PrintCharTest, SignedChar) {
  EXPECT_EQ("'\\0'", Print(static_cast<signed char>('\0')));
  EXPECT_EQ("'\\xCE' (-50)",
            Print(static_cast<signed char>(-50)));
}

// unsigned char.
TEST(PrintCharTest, UnsignedChar) {
  EXPECT_EQ("'\\0'", Print(static_cast<unsigned char>('\0')));
  EXPECT_EQ("'b' (98, 0x62)",
            Print(static_cast<unsigned char>('b')));
}

// Tests printing other simple, built-in types.

// bool.
TEST(PrintBuiltInTypeTest, Bool) {
  EXPECT_EQ("false", Print(false));
  EXPECT_EQ("true", Print(true));
}

// wchar_t.
TEST(PrintBuiltInTypeTest, Wchar_t) {
  EXPECT_EQ("L'\\0'", Print(L'\0'));
  EXPECT_EQ("L'\\'' (39, 0x27)", Print(L'\''));
  EXPECT_EQ("L'\"' (34, 0x22)", Print(L'"'));
  EXPECT_EQ("L'\\?' (63, 0x3F)", Print(L'\?'));
  EXPECT_EQ("L'\\\\' (92, 0x5C)", Print(L'\\'));
  EXPECT_EQ("L'\\a' (7)", Print(L'\a'));
  EXPECT_EQ("L'\\b' (8)", Print(L'\b'));
  EXPECT_EQ("L'\\f' (12, 0xC)", Print(L'\f'));
  EXPECT_EQ("L'\\n' (10, 0xA)", Print(L'\n'));
  EXPECT_EQ("L'\\r' (13, 0xD)", Print(L'\r'));
  EXPECT_EQ("L'\\t' (9)", Print(L'\t'));
  EXPECT_EQ("L'\\v' (11, 0xB)", Print(L'\v'));
  EXPECT_EQ("L'\\x7F' (127)", Print(L'\x7F'));
  EXPECT_EQ("L'\\xFF' (255)", Print(L'\xFF'));
  EXPECT_EQ("L' ' (32, 0x20)", Print(L' '));
  EXPECT_EQ("L'a' (97, 0x61)", Print(L'a'));
  EXPECT_EQ("L'\\x576' (1398)", Print(L'\x576'));
  EXPECT_EQ("L'\\xC74D' (51021)", Print(L'\xC74D'));
}

// Test that Int64 provides more storage than wchar_t.
TEST(PrintTypeSizeTest, Wchar_t) {
  EXPECT_LT(sizeof(wchar_t), sizeof(testing::internal::Int64));
}

// Various integer types.
TEST(PrintBuiltInTypeTest, Integer) {
  EXPECT_EQ("'\\xFF' (255)", Print(static_cast<unsigned char>(255)));  // uint8
  EXPECT_EQ("'\\x80' (-128)", Print(static_cast<signed char>(-128)));  // int8
  EXPECT_EQ("65535", Print(USHRT_MAX));  // uint16
  EXPECT_EQ("-32768", Print(SHRT_MIN));  // int16
  EXPECT_EQ("4294967295", Print(UINT_MAX));  // uint32
  EXPECT_EQ("-2147483648", Print(INT_MIN));  // int32
  EXPECT_EQ("18446744073709551615",
            Print(static_cast<testing::internal::UInt64>(-1)));  // uint64
  EXPECT_EQ("-9223372036854775808",
            Print(static_cast<testing::internal::Int64>(1) << 63));  // int64
}

// Size types.
TEST(PrintBuiltInTypeTest, Size_t) {
  EXPECT_EQ("1", Print(sizeof('a')));  // size_t.
#if !GTEST_OS_WINDOWS
  // Windows has no ssize_t type.
  EXPECT_EQ("-2", Print(static_cast<ssize_t>(-2)));  // ssize_t.
#endif  // !GTEST_OS_WINDOWS
}

// Floating-points.
TEST(PrintBuiltInTypeTest, FloatingPoints) {
  EXPECT_EQ("1.5", Print(1.5f));   // float
  EXPECT_EQ("-2.5", Print(-2.5));  // double
}

// Since ::std::stringstream::operator<<(const void *) formats the pointer
// output differently with different compilers, we have to create the expected
// output first and use it as our expectation.
static string PrintPointer(const void *p) {
  ::std::stringstream expected_result_stream;
  expected_result_stream << p;
  return expected_result_stream.str();
}

// Tests printing C strings.

// const char*.
TEST(PrintCStringTest, Const) {
  const char* p = "World";
  EXPECT_EQ(PrintPointer(p) + " pointing to \"World\"", Print(p));
}

// char*.
TEST(PrintCStringTest, NonConst) {
  char p[] = "Hi";
  EXPECT_EQ(PrintPointer(p) + " pointing to \"Hi\"",
            Print(static_cast<char*>(p)));
}

// NULL C string.
TEST(PrintCStringTest, Null) {
  const char* p = NULL;
  EXPECT_EQ("NULL", Print(p));
}

// Tests that C strings are escaped properly.
TEST(PrintCStringTest, EscapesProperly) {
  const char* p = "'\"\?\\\a\b\f\n\r\t\v\x7F\xFF a";
  EXPECT_EQ(PrintPointer(p) + " pointing to \"'\\\"\\?\\\\\\a\\b\\f"
            "\\n\\r\\t\\v\\x7F\\xFF a\"",
            Print(p));
}



// MSVC compiler can be configured to define whar_t as a typedef
// of unsigned short. Defining an overload for const wchar_t* in that case
// would cause pointers to unsigned shorts be printed as wide strings,
// possibly accessing more memory than intended and causing invalid
// memory accesses. MSVC defines _NATIVE_WCHAR_T_DEFINED symbol when
// wchar_t is implemented as a native type.
#if !defined(_MSC_VER) || defined(_NATIVE_WCHAR_T_DEFINED)

// const wchar_t*.
TEST(PrintWideCStringTest, Const) {
  const wchar_t* p = L"World";
  EXPECT_EQ(PrintPointer(p) + " pointing to L\"World\"", Print(p));
}

// wchar_t*.
TEST(PrintWideCStringTest, NonConst) {
  wchar_t p[] = L"Hi";
  EXPECT_EQ(PrintPointer(p) + " pointing to L\"Hi\"",
            Print(static_cast<wchar_t*>(p)));
}

// NULL wide C string.
TEST(PrintWideCStringTest, Null) {
  const wchar_t* p = NULL;
  EXPECT_EQ("NULL", Print(p));
}

// Tests that wide C strings are escaped properly.
TEST(PrintWideCStringTest, EscapesProperly) {
  const wchar_t* p = L"'\"\?\\\a\b\f\n\r\t\v\xD3\x576\x8D3\xC74D a";
  EXPECT_EQ(PrintPointer(p) + " pointing to L\"'\\\"\\?\\\\\\a\\b\\f"
            "\\n\\r\\t\\v\\xD3\\x576\\x8D3\\xC74D a\"",
            Print(p));
}
#endif  // native wchar_t

// Tests printing pointers to other char types.

// signed char*.
TEST(PrintCharPointerTest, SignedChar) {
  signed char* p = reinterpret_cast<signed char*>(0x1234);
  EXPECT_EQ(PrintPointer(p), Print(p));
  p = NULL;
  EXPECT_EQ("NULL", Print(p));
}

// const signed char*.
TEST(PrintCharPointerTest, ConstSignedChar) {
  signed char* p = reinterpret_cast<signed char*>(0x1234);
  EXPECT_EQ(PrintPointer(p), Print(p));
  p = NULL;
  EXPECT_EQ("NULL", Print(p));
}

// unsigned char*.
TEST(PrintCharPointerTest, UnsignedChar) {
  unsigned char* p = reinterpret_cast<unsigned char*>(0x1234);
  EXPECT_EQ(PrintPointer(p), Print(p));
  p = NULL;
  EXPECT_EQ("NULL", Print(p));
}

// const unsigned char*.
TEST(PrintCharPointerTest, ConstUnsignedChar) {
  const unsigned char* p = reinterpret_cast<const unsigned char*>(0x1234);
  EXPECT_EQ(PrintPointer(p), Print(p));
  p = NULL;
  EXPECT_EQ("NULL", Print(p));
}

// Tests printing pointers to simple, built-in types.

// bool*.
TEST(PrintPointerToBuiltInTypeTest, Bool) {
  bool* p = reinterpret_cast<bool*>(0xABCD);
  EXPECT_EQ(PrintPointer(p), Print(p));
  p = NULL;
  EXPECT_EQ("NULL", Print(p));
}

// void*.
TEST(PrintPointerToBuiltInTypeTest, Void) {
  void* p = reinterpret_cast<void*>(0xABCD);
  EXPECT_EQ(PrintPointer(p), Print(p));
  p = NULL;
  EXPECT_EQ("NULL", Print(p));
}

// const void*.
TEST(PrintPointerToBuiltInTypeTest, ConstVoid) {
  const void* p = reinterpret_cast<const void*>(0xABCD);
  EXPECT_EQ(PrintPointer(p), Print(p));
  p = NULL;
  EXPECT_EQ("NULL", Print(p));
}

// Tests printing pointers to pointers.
TEST(PrintPointerToPointerTest, IntPointerPointer) {
  int** p = reinterpret_cast<int**>(0xABCD);
  EXPECT_EQ(PrintPointer(p), Print(p));
  p = NULL;
  EXPECT_EQ("NULL", Print(p));
}

// Tests printing (non-member) function pointers.

void MyFunction(int /* n */) {}

TEST(PrintPointerTest, NonMemberFunctionPointer) {
  // We cannot directly cast &MyFunction to const void* because the
  // standard disallows casting between pointers to functions and
  // pointers to objects, and some compilers (e.g. GCC 3.4) enforce
  // this limitation.
  EXPECT_EQ(
      PrintPointer(reinterpret_cast<const void*>(
          reinterpret_cast<internal::BiggestInt>(&MyFunction))),
      Print(&MyFunction));
  int (*p)(bool) = NULL;  // NOLINT
  EXPECT_EQ("NULL", Print(p));
}

// An assertion predicate determining whether a one string is a prefix for
// another.
template <typename StringType>
AssertionResult HasPrefix(const StringType& str, const StringType& prefix) {
  if (str.find(prefix, 0) == 0)
    return AssertionSuccess();

  const bool is_wide_string = sizeof(prefix[0]) > 1;
  const char* const begin_string_quote = is_wide_string ? "L\"" : "\"";
  return AssertionFailure()
      << begin_string_quote << prefix << "\" is not a prefix of "
      << begin_string_quote << str << "\"\n";
}

// Tests printing member variable pointers.  Although they are called
// pointers, they don't point to a location in the address space.
// Their representation is implementation-defined.  Thus they will be
// printed as raw bytes.

struct Foo {
 public:
  virtual ~Foo() {}
  int MyMethod(char x) { return x + 1; }
  virtual char MyVirtualMethod(int /* n */) { return 'a'; }

  int value;
};

TEST(PrintPointerTest, MemberVariablePointer) {
  EXPECT_TRUE(HasPrefix(Print(&Foo::value),
                        Print(sizeof(&Foo::value)) + "-byte object "));
  int (Foo::*p) = NULL;  // NOLINT
  EXPECT_TRUE(HasPrefix(Print(p),
                        Print(sizeof(p)) + "-byte object "));
}

// Tests printing member function pointers.  Although they are called
// pointers, they don't point to a location in the address space.
// Their representation is implementation-defined.  Thus they will be
// printed as raw bytes.
TEST(PrintPointerTest, MemberFunctionPointer) {
  EXPECT_TRUE(HasPrefix(Print(&Foo::MyMethod),
                        Print(sizeof(&Foo::MyMethod)) + "-byte object "));
  EXPECT_TRUE(
      HasPrefix(Print(&Foo::MyVirtualMethod),
                Print(sizeof((&Foo::MyVirtualMethod))) + "-byte object "));
  int (Foo::*p)(char) = NULL;  // NOLINT
  EXPECT_TRUE(HasPrefix(Print(p),
                        Print(sizeof(p)) + "-byte object "));
}

// Tests printing C arrays.

// The difference between this and Print() is that it ensures that the
// argument is a reference to an array.
template <typename T, size_t N>
string PrintArrayHelper(T (&a)[N]) {
  return Print(a);
}

// One-dimensional array.
TEST(PrintArrayTest, OneDimensionalArray) {
  int a[5] = { 1, 2, 3, 4, 5 };
  EXPECT_EQ("{ 1, 2, 3, 4, 5 }", PrintArrayHelper(a));
}

// Two-dimensional array.
TEST(PrintArrayTest, TwoDimensionalArray) {
  int a[2][5] = {
    { 1, 2, 3, 4, 5 },
    { 6, 7, 8, 9, 0 }
  };
  EXPECT_EQ("{ { 1, 2, 3, 4, 5 }, { 6, 7, 8, 9, 0 } }", PrintArrayHelper(a));
}

// Array of const elements.
TEST(PrintArrayTest, ConstArray) {
  const bool a[1] = { false };
  EXPECT_EQ("{ false }", PrintArrayHelper(a));
}

// Char array.
TEST(PrintArrayTest, CharArray) {
  // Array a contains '\0' in the middle and doesn't end with '\0'.
  char a[3] = { 'H', '\0', 'i' };
  EXPECT_EQ("\"H\\0i\"", PrintArrayHelper(a));
}

// Const char array.
TEST(PrintArrayTest, ConstCharArray) {
  const char a[4] = "\0Hi";
  EXPECT_EQ("\"\\0Hi\\0\"", PrintArrayHelper(a));
}

// Array of objects.
TEST(PrintArrayTest, ObjectArray) {
  string a[3] = { "Hi", "Hello", "Ni hao" };
  EXPECT_EQ("{ \"Hi\", \"Hello\", \"Ni hao\" }", PrintArrayHelper(a));
}

// Array with many elements.
TEST(PrintArrayTest, BigArray) {
  int a[100] = { 1, 2, 3 };
  EXPECT_EQ("{ 1, 2, 3, 0, 0, 0, 0, 0, ..., 0, 0, 0, 0, 0, 0, 0, 0 }",
            PrintArrayHelper(a));
}

// Tests printing ::string and ::std::string.

#if GTEST_HAS_GLOBAL_STRING
// ::string.
TEST(PrintStringTest, StringInGlobalNamespace) {
  const char s[] = "'\"\?\\\a\b\f\n\0\r\t\v\x7F\xFF a";
  const ::string str(s, sizeof(s));
  EXPECT_EQ("\"'\\\"\\?\\\\\\a\\b\\f\\n\\0\\r\\t\\v\\x7F\\xFF a\\0\"",
            Print(str));
}
#endif  // GTEST_HAS_GLOBAL_STRING

// ::std::string.
TEST(PrintStringTest, StringInStdNamespace) {
  const char s[] = "'\"\?\\\a\b\f\n\0\r\t\v\x7F\xFF a";
  const ::std::string str(s, sizeof(s));
  EXPECT_EQ("\"'\\\"\\?\\\\\\a\\b\\f\\n\\0\\r\\t\\v\\x7F\\xFF a\\0\"",
            Print(str));
}

// Tests printing ::wstring and ::std::wstring.

#if GTEST_HAS_GLOBAL_WSTRING
// ::wstring.
TEST(PrintWideStringTest, StringInGlobalNamespace) {
  const wchar_t s[] = L"'\"\?\\\a\b\f\n\0\r\t\v\xD3\x576\x8D3\xC74D a";
  const ::wstring str(s, sizeof(s)/sizeof(wchar_t));
  EXPECT_EQ("L\"'\\\"\\?\\\\\\a\\b\\f\\n\\0\\r\\t\\v"
            "\\xD3\\x576\\x8D3\\xC74D a\\0\"",
            Print(str));
}
#endif  // GTEST_HAS_GLOBAL_WSTRING

#if GTEST_HAS_STD_WSTRING
// ::std::wstring.
TEST(PrintWideStringTest, StringInStdNamespace) {
  const wchar_t s[] = L"'\"\?\\\a\b\f\n\0\r\t\v\xD3\x576\x8D3\xC74D a";
  const ::std::wstring str(s, sizeof(s)/sizeof(wchar_t));
  EXPECT_EQ("L\"'\\\"\\?\\\\\\a\\b\\f\\n\\0\\r\\t\\v"
            "\\xD3\\x576\\x8D3\\xC74D a\\0\"",
            Print(str));
}
#endif  // GTEST_HAS_STD_WSTRING

// Tests printing types that support generic streaming (i.e. streaming
// to std::basic_ostream<Char, CharTraits> for any valid Char and
// CharTraits types).

// Tests printing a non-template type that supports generic streaming.

class AllowsGenericStreaming {};

template <typename Char, typename CharTraits>
std::basic_ostream<Char, CharTraits>& operator<<(
    std::basic_ostream<Char, CharTraits>& os,
    const AllowsGenericStreaming& /* a */) {
  return os << "AllowsGenericStreaming";
}

TEST(PrintTypeWithGenericStreamingTest, NonTemplateType) {
  AllowsGenericStreaming a;
  EXPECT_EQ("AllowsGenericStreaming", Print(a));
}

// Tests printing a template type that supports generic streaming.

template <typename T>
class AllowsGenericStreamingTemplate {};

template <typename Char, typename CharTraits, typename T>
std::basic_ostream<Char, CharTraits>& operator<<(
    std::basic_ostream<Char, CharTraits>& os,
    const AllowsGenericStreamingTemplate<T>& /* a */) {
  return os << "AllowsGenericStreamingTemplate";
}

TEST(PrintTypeWithGenericStreamingTest, TemplateType) {
  AllowsGenericStreamingTemplate<int> a;
  EXPECT_EQ("AllowsGenericStreamingTemplate", Print(a));
}

// Tests printing a type that supports generic streaming and can be
// implicitly converted to another printable type.

template <typename T>
class AllowsGenericStreamingAndImplicitConversionTemplate {
 public:
  operator bool() const { return false; }
};

template <typename Char, typename CharTraits, typename T>
std::basic_ostream<Char, CharTraits>& operator<<(
    std::basic_ostream<Char, CharTraits>& os,
    const AllowsGenericStreamingAndImplicitConversionTemplate<T>& /* a */) {
  return os << "AllowsGenericStreamingAndImplicitConversionTemplate";
}

TEST(PrintTypeWithGenericStreamingTest, TypeImplicitlyConvertible) {
  AllowsGenericStreamingAndImplicitConversionTemplate<int> a;
  EXPECT_EQ("AllowsGenericStreamingAndImplicitConversionTemplate", Print(a));
}

#if GTEST_HAS_STRING_PIECE_

// Tests printing StringPiece.

TEST(PrintStringPieceTest, SimpleStringPiece) {
  const StringPiece sp = "Hello";
  EXPECT_EQ("\"Hello\"", Print(sp));
}

TEST(PrintStringPieceTest, UnprintableCharacters) {
  const char str[] = "NUL (\0) and \r\t";
  const StringPiece sp(str, sizeof(str) - 1);
  EXPECT_EQ("\"NUL (\\0) and \\r\\t\"", Print(sp));
}

#endif  // GTEST_HAS_STRING_PIECE_

// Tests printing STL containers.

TEST(PrintStlContainerTest, EmptyDeque) {
  deque<char> empty;
  EXPECT_EQ("{}", Print(empty));
}

TEST(PrintStlContainerTest, NonEmptyDeque) {
  deque<int> non_empty;
  non_empty.push_back(1);
  non_empty.push_back(3);
  EXPECT_EQ("{ 1, 3 }", Print(non_empty));
}

#if GTEST_HAS_HASH_MAP_

TEST(PrintStlContainerTest, OneElementHashMap) {
  hash_map<int, char> map1;
  map1[1] = 'a';
  EXPECT_EQ("{ (1, 'a' (97, 0x61)) }", Print(map1));
}

TEST(PrintStlContainerTest, HashMultiMap) {
  hash_multimap<int, bool> map1;
  map1.insert(make_pair(5, true));
  map1.insert(make_pair(5, false));

  // Elements of hash_multimap can be printed in any order.
  const string result = Print(map1);
  EXPECT_TRUE(result == "{ (5, true), (5, false) }" ||
              result == "{ (5, false), (5, true) }")
                  << " where Print(map1) returns \"" << result << "\".";
}

#endif  // GTEST_HAS_HASH_MAP_

#if GTEST_HAS_HASH_SET_

TEST(PrintStlContainerTest, HashSet) {
  hash_set<string> set1;
  set1.insert("hello");
  EXPECT_EQ("{ \"hello\" }", Print(set1));
}

TEST(PrintStlContainerTest, HashMultiSet) {
  const int kSize = 5;
  int a[kSize] = { 1, 1, 2, 5, 1 };
  hash_multiset<int> set1(a, a + kSize);

  // Elements of hash_multiset can be printed in any order.
  const string result = Print(set1);
  const string expected_pattern = "{ d, d, d, d, d }";  // d means a digit.

  // Verifies the result matches the expected pattern; also extracts
  // the numbers in the result.
  ASSERT_EQ(expected_pattern.length(), result.length());
  std::vector<int> numbers;
  for (size_t i = 0; i != result.length(); i++) {
    if (expected_pattern[i] == 'd') {
      ASSERT_TRUE(isdigit(static_cast<unsigned char>(result[i])) != 0);
      numbers.push_back(result[i] - '0');
    } else {
      EXPECT_EQ(expected_pattern[i], result[i]) << " where result is "
                                                << result;
    }
  }

  // Makes sure the result contains the right numbers.
  std::sort(numbers.begin(), numbers.end());
  std::sort(a, a + kSize);
  EXPECT_TRUE(std::equal(a, a + kSize, numbers.begin()));
}

#endif  // GTEST_HAS_HASH_SET_

TEST(PrintStlContainerTest, List) {
  const char* a[] = {
    "hello",
    "world"
  };
  const list<string> strings(a, a + 2);
  EXPECT_EQ("{ \"hello\", \"world\" }", Print(strings));
}

TEST(PrintStlContainerTest, Map) {
  map<int, bool> map1;
  map1[1] = true;
  map1[5] = false;
  map1[3] = true;
  EXPECT_EQ("{ (1, true), (3, true), (5, false) }", Print(map1));
}

TEST(PrintStlContainerTest, MultiMap) {
  multimap<bool, int> map1;
  map1.insert(make_pair(true, 0));
  map1.insert(make_pair(true, 1));
  map1.insert(make_pair(false, 2));
  EXPECT_EQ("{ (false, 2), (true, 0), (true, 1) }", Print(map1));
}

TEST(PrintStlContainerTest, Set) {
  const unsigned int a[] = { 3, 0, 5 };
  set<unsigned int> set1(a, a + 3);
  EXPECT_EQ("{ 0, 3, 5 }", Print(set1));
}

TEST(PrintStlContainerTest, MultiSet) {
  const int a[] = { 1, 1, 2, 5, 1 };
  multiset<int> set1(a, a + 5);
  EXPECT_EQ("{ 1, 1, 1, 2, 5 }", Print(set1));
}

TEST(PrintStlContainerTest, Pair) {
  pair<const bool, int> p(true, 5);
  EXPECT_EQ("(true, 5)", Print(p));
}

TEST(PrintStlContainerTest, Vector) {
  vector<int> v;
  v.push_back(1);
  v.push_back(2);
  EXPECT_EQ("{ 1, 2 }", Print(v));
}

TEST(PrintStlContainerTest, LongSequence) {
  const int a[100] = { 1, 2, 3 };
  const vector<int> v(a, a + 100);
  EXPECT_EQ("{ 1, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "
            "0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ... }", Print(v));
}

TEST(PrintStlContainerTest, NestedContainer) {
  const int a1[] = { 1, 2 };
  const int a2[] = { 3, 4, 5 };
  const list<int> l1(a1, a1 + 2);
  const list<int> l2(a2, a2 + 3);

  vector<list<int> > v;
  v.push_back(l1);
  v.push_back(l2);
  EXPECT_EQ("{ { 1, 2 }, { 3, 4, 5 } }", Print(v));
}

TEST(PrintStlContainerTest, OneDimensionalNativeArray) {
  const int a[3] = { 1, 2, 3 };
  NativeArray<int> b(a, 3, kReference);
  EXPECT_EQ("{ 1, 2, 3 }", Print(b));
}

TEST(PrintStlContainerTest, TwoDimensionalNativeArray) {
  const int a[2][3] = { { 1, 2, 3 }, { 4, 5, 6 } };
  NativeArray<int[3]> b(a, 2, kReference);
  EXPECT_EQ("{ { 1, 2, 3 }, { 4, 5, 6 } }", Print(b));
}

#if GTEST_HAS_TR1_TUPLE
// Tests printing tuples.

// Tuples of various arities.
TEST(PrintTupleTest, VariousSizes) {
  tuple<> t0;
  EXPECT_EQ("()", Print(t0));

  tuple<int> t1(5);
  EXPECT_EQ("(5)", Print(t1));

  tuple<char, bool> t2('a', true);
  EXPECT_EQ("('a' (97, 0x61), true)", Print(t2));

  tuple<bool, int, int> t3(false, 2, 3);
  EXPECT_EQ("(false, 2, 3)", Print(t3));

  tuple<bool, int, int, int> t4(false, 2, 3, 4);
  EXPECT_EQ("(false, 2, 3, 4)", Print(t4));

  tuple<bool, int, int, int, bool> t5(false, 2, 3, 4, true);
  EXPECT_EQ("(false, 2, 3, 4, true)", Print(t5));

  tuple<bool, int, int, int, bool, int> t6(false, 2, 3, 4, true, 6);
  EXPECT_EQ("(false, 2, 3, 4, true, 6)", Print(t6));

  tuple<bool, int, int, int, bool, int, int> t7(false, 2, 3, 4, true, 6, 7);
  EXPECT_EQ("(false, 2, 3, 4, true, 6, 7)", Print(t7));

  tuple<bool, int, int, int, bool, int, int, bool> t8(
      false, 2, 3, 4, true, 6, 7, true);
  EXPECT_EQ("(false, 2, 3, 4, true, 6, 7, true)", Print(t8));

  tuple<bool, int, int, int, bool, int, int, bool, int> t9(
      false, 2, 3, 4, true, 6, 7, true, 9);
  EXPECT_EQ("(false, 2, 3, 4, true, 6, 7, true, 9)", Print(t9));

  const char* const str = "8";
  tuple<bool, char, short, testing::internal::Int32,  // NOLINT
      testing::internal::Int64, float, double, const char*, void*, string>
      t10(false, 'a', 3, 4, 5, 1.5F, -2.5, str, NULL, "10");
  EXPECT_EQ("(false, 'a' (97, 0x61), 3, 4, 5, 1.5, -2.5, " + PrintPointer(str) +
            " pointing to \"8\", NULL, \"10\")",
            Print(t10));
}

// Nested tuples.
TEST(PrintTupleTest, NestedTuple) {
  tuple<tuple<int, bool>, char> nested(make_tuple(5, true), 'a');
  EXPECT_EQ("((5, true), 'a' (97, 0x61))", Print(nested));
}

#endif  // GTEST_HAS_TR1_TUPLE

// Tests printing user-defined unprintable types.

// Unprintable types in the global namespace.
TEST(PrintUnprintableTypeTest, InGlobalNamespace) {
  EXPECT_EQ("1-byte object <00>",
            Print(UnprintableTemplateInGlobal<bool>()));
}

// Unprintable types in a user namespace.
TEST(PrintUnprintableTypeTest, InUserNamespace) {
  EXPECT_EQ("16-byte object <EF-12 00-00 34-AB 00-00 00-00 00-00 00-00 00-00>",
            Print(::foo::UnprintableInFoo()));
}

// Unprintable types are that too big to be printed completely.

struct Big {
  Big() { memset(array, 0, sizeof(array)); }
  char array[257];
};

TEST(PrintUnpritableTypeTest, BigObject) {
  EXPECT_EQ("257-byte object <00-00 00-00 00-00 00-00 00-00 00-00 "
            "00-00 00-00 00-00 00-00 00-00 00-00 00-00 00-00 00-00 00-00 "
            "00-00 00-00 00-00 00-00 00-00 00-00 00-00 00-00 00-00 00-00 "
            "00-00 00-00 00-00 00-00 00-00 00-00 ... 00-00 00-00 00-00 "
            "00-00 00-00 00-00 00-00 00-00 00-00 00-00 00-00 00-00 00-00 "
            "00-00 00-00 00-00 00-00 00-00 00-00 00-00 00-00 00-00 00-00 "
            "00-00 00-00 00-00 00-00 00-00 00-00 00-00 00-00 00>",
            Print(Big()));
}

// Tests printing user-defined streamable types.

// Streamable types in the global namespace.
TEST(PrintStreamableTypeTest, InGlobalNamespace) {
  StreamableInGlobal x;
  EXPECT_EQ("StreamableInGlobal", Print(x));
  EXPECT_EQ("StreamableInGlobal*", Print(&x));
}

// Printable template types in a user namespace.
TEST(PrintStreamableTypeTest, TemplateTypeInUserNamespace) {
  EXPECT_EQ("StreamableTemplateInFoo: 0",
            Print(::foo::StreamableTemplateInFoo<int>()));
}

// Tests printing user-defined types that have a PrintTo() function.
TEST(PrintPrintableTypeTest, InUserNamespace) {
  EXPECT_EQ("PrintableViaPrintTo: 0",
            Print(::foo::PrintableViaPrintTo()));
}

// Tests printing a pointer to a user-defined type that has a <<
// operator for its pointer.
TEST(PrintPrintableTypeTest, PointerInUserNamespace) {
  ::foo::PointerPrintable x;
  EXPECT_EQ("PointerPrintable*", Print(&x));
}

// Tests printing user-defined class template that have a PrintTo() function.
TEST(PrintPrintableTypeTest, TemplateInUserNamespace) {
  EXPECT_EQ("PrintableViaPrintToTemplate: 5",
            Print(::foo::PrintableViaPrintToTemplate<int>(5)));
}

#if GTEST_HAS_PROTOBUF_

// Tests printing a protocol message.
TEST(PrintProtocolMessageTest, PrintsShortDebugString) {
  testing::internal::TestMessage msg;
  msg.set_member("yes");
  EXPECT_EQ("<member:\"yes\">", Print(msg));
}

// Tests printing a short proto2 message.
TEST(PrintProto2MessageTest, PrintsShortDebugStringWhenItIsShort) {
  testing::internal::FooMessage msg;
  msg.set_int_field(2);
  msg.set_string_field("hello");
  EXPECT_PRED2(RE::FullMatch, Print(msg),
               "<int_field:\\s*2\\s+string_field:\\s*\"hello\">");
}

// Tests printing a long proto2 message.
TEST(PrintProto2MessageTest, PrintsDebugStringWhenItIsLong) {
  testing::internal::FooMessage msg;
  msg.set_int_field(2);
  msg.set_string_field("hello");
  msg.add_names("peter");
  msg.add_names("paul");
  msg.add_names("mary");
  EXPECT_PRED2(RE::FullMatch, Print(msg),
               "<\n"
               "int_field:\\s*2\n"
               "string_field:\\s*\"hello\"\n"
               "names:\\s*\"peter\"\n"
               "names:\\s*\"paul\"\n"
               "names:\\s*\"mary\"\n"
               ">");
}

#endif  // GTEST_HAS_PROTOBUF_

// Tests that the universal printer prints both the address and the
// value of a reference.
TEST(PrintReferenceTest, PrintsAddressAndValue) {
  int n = 5;
  EXPECT_EQ("@" + PrintPointer(&n) + " 5", PrintByRef(n));

  int a[2][3] = {
    { 0, 1, 2 },
    { 3, 4, 5 }
  };
  EXPECT_EQ("@" + PrintPointer(a) + " { { 0, 1, 2 }, { 3, 4, 5 } }",
            PrintByRef(a));

  const ::foo::UnprintableInFoo x;
  EXPECT_EQ("@" + PrintPointer(&x) + " 16-byte object "
            "<EF-12 00-00 34-AB 00-00 00-00 00-00 00-00 00-00>",
            PrintByRef(x));
}

// Tests that the universal printer prints a function pointer passed by
// reference.
TEST(PrintReferenceTest, HandlesFunctionPointer) {
  void (*fp)(int n) = &MyFunction;
  const string fp_pointer_string =
      PrintPointer(reinterpret_cast<const void*>(&fp));
  // We cannot directly cast &MyFunction to const void* because the
  // standard disallows casting between pointers to functions and
  // pointers to objects, and some compilers (e.g. GCC 3.4) enforce
  // this limitation.
  const string fp_string = PrintPointer(reinterpret_cast<const void*>(
      reinterpret_cast<internal::BiggestInt>(fp)));
  EXPECT_EQ("@" + fp_pointer_string + " " + fp_string,
            PrintByRef(fp));
}

// Tests that the universal printer prints a member function pointer
// passed by reference.
TEST(PrintReferenceTest, HandlesMemberFunctionPointer) {
  int (Foo::*p)(char ch) = &Foo::MyMethod;
  EXPECT_TRUE(HasPrefix(
      PrintByRef(p),
      "@" + PrintPointer(reinterpret_cast<const void*>(&p)) + " " +
          Print(sizeof(p)) + "-byte object "));

  char (Foo::*p2)(int n) = &Foo::MyVirtualMethod;
  EXPECT_TRUE(HasPrefix(
      PrintByRef(p2),
      "@" + PrintPointer(reinterpret_cast<const void*>(&p2)) + " " +
          Print(sizeof(p2)) + "-byte object "));
}

// Tests that the universal printer prints a member variable pointer
// passed by reference.
TEST(PrintReferenceTest, HandlesMemberVariablePointer) {
  int (Foo::*p) = &Foo::value;  // NOLINT
  EXPECT_TRUE(HasPrefix(
      PrintByRef(p),
      "@" + PrintPointer(&p) + " " + Print(sizeof(p)) + "-byte object "));
}

// Useful for testing PrintToString().  We cannot use EXPECT_EQ()
// there as its implementation uses PrintToString().  The caller must
// ensure that 'value' has no side effect.
#define EXPECT_PRINT_TO_STRING_(value, expected_string)         \
  EXPECT_TRUE(PrintToString(value) == (expected_string))        \
      << " where " #value " prints as " << (PrintToString(value))

TEST(PrintToStringTest, WorksForScalar) {
  EXPECT_PRINT_TO_STRING_(123, "123");
}

TEST(PrintToStringTest, WorksForPointerToConstChar) {
  const char* p = "hello";
  EXPECT_PRINT_TO_STRING_(p, "\"hello\"");
}

TEST(PrintToStringTest, WorksForPointerToNonConstChar) {
  char s[] = "hello";
  char* p = s;
  EXPECT_PRINT_TO_STRING_(p, "\"hello\"");
}

TEST(PrintToStringTest, WorksForArray) {
  int n[3] = { 1, 2, 3 };
  EXPECT_PRINT_TO_STRING_(n, "{ 1, 2, 3 }");
}

#undef EXPECT_PRINT_TO_STRING_

TEST(UniversalTersePrintTest, WorksForNonReference) {
  ::std::stringstream ss;
  UniversalTersePrint(123, &ss);
  EXPECT_EQ("123", ss.str());
}

TEST(UniversalTersePrintTest, WorksForReference) {
  const int& n = 123;
  ::std::stringstream ss;
  UniversalTersePrint(n, &ss);
  EXPECT_EQ("123", ss.str());
}

TEST(UniversalTersePrintTest, WorksForCString) {
  const char* s1 = "abc";
  ::std::stringstream ss1;
  UniversalTersePrint(s1, &ss1);
  EXPECT_EQ("\"abc\"", ss1.str());

  char* s2 = const_cast<char*>(s1);
  ::std::stringstream ss2;
  UniversalTersePrint(s2, &ss2);
  EXPECT_EQ("\"abc\"", ss2.str());

  const char* s3 = NULL;
  ::std::stringstream ss3;
  UniversalTersePrint(s3, &ss3);
  EXPECT_EQ("NULL", ss3.str());
}

TEST(UniversalPrintTest, WorksForNonReference) {
  ::std::stringstream ss;
  UniversalPrint(123, &ss);
  EXPECT_EQ("123", ss.str());
}

TEST(UniversalPrintTest, WorksForReference) {
  const int& n = 123;
  ::std::stringstream ss;
  UniversalPrint(n, &ss);
  EXPECT_EQ("123", ss.str());
}

TEST(UniversalPrintTest, WorksForCString) {
  const char* s1 = "abc";
  ::std::stringstream ss1;
  UniversalPrint(s1, &ss1);
  EXPECT_EQ(PrintPointer(s1) + " pointing to \"abc\"", string(ss1.str()));

  char* s2 = const_cast<char*>(s1);
  ::std::stringstream ss2;
  UniversalPrint(s2, &ss2);
  EXPECT_EQ(PrintPointer(s2) + " pointing to \"abc\"", string(ss2.str()));

  const char* s3 = NULL;
  ::std::stringstream ss3;
  UniversalPrint(s3, &ss3);
  EXPECT_EQ("NULL", ss3.str());
}


#if GTEST_HAS_TR1_TUPLE

TEST(UniversalTersePrintTupleFieldsToStringsTest, PrintsEmptyTuple) {
  Strings result = UniversalTersePrintTupleFieldsToStrings(make_tuple());
  EXPECT_EQ(0u, result.size());
}

TEST(UniversalTersePrintTupleFieldsToStringsTest, PrintsOneTuple) {
  Strings result = UniversalTersePrintTupleFieldsToStrings(make_tuple(1));
  ASSERT_EQ(1u, result.size());
  EXPECT_EQ("1", result[0]);
}

TEST(UniversalTersePrintTupleFieldsToStringsTest, PrintsTwoTuple) {
  Strings result = UniversalTersePrintTupleFieldsToStrings(make_tuple(1, 'a'));
  ASSERT_EQ(2u, result.size());
  EXPECT_EQ("1", result[0]);
  EXPECT_EQ("'a' (97, 0x61)", result[1]);
}

TEST(UniversalTersePrintTupleFieldsToStringsTest, PrintsTersely) {
  const int n = 1;
  Strings result = UniversalTersePrintTupleFieldsToStrings(
      tuple<const int&, const char*>(n, "a"));
  ASSERT_EQ(2u, result.size());
  EXPECT_EQ("1", result[0]);
  EXPECT_EQ("\"a\"", result[1]);
}

#endif  // GTEST_HAS_TR1_TUPLE

}  // namespace gtest_printers_test
}  // namespace testing
