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
// A user can teach this function how to print a class type T by
// defining either operator<<() or PrintTo() in the namespace that
// defines T.  More specifically, the FIRST defined function in the
// following list will be used (assuming T is defined in namespace
// foo):
//
//   1. foo::PrintTo(const T&, ostream*)
//   2. operator<<(ostream&, const T&) defined in either foo or the
//      global namespace.
//
// If none of the above is defined, it will print the debug string of
// the value if it is a protocol buffer, or print the raw bytes in the
// value otherwise.
//
// To aid debugging: when T is a reference type, the address of the
// value is also printed; when T is a (const) char pointer, both the
// pointer value and the NUL-terminated string it points to are
// printed.
//
// We also provide some convenient wrappers:
//
//   // Prints a value as the given type to a string.
//   string ::testing::internal::UniversalPrinter<T>::PrintToString(value);
//
//   // Prints a value tersely: for a reference type, the referenced
//   // value (but not the address) is printed; for a (const) char
//   // pointer, the NUL-terminated string (but not the pointer) is
//   // printed.
//   void ::testing::internal::UniversalTersePrint(const T& value, ostream*);
//
//   // Prints the fields of a tuple tersely to a string vector, one
//   // element for each field.
//   std::vector<string> UniversalTersePrintTupleFieldsToStrings(
//       const Tuple& value);

#ifndef GMOCK_INCLUDE_GMOCK_GMOCK_PRINTERS_H_
#define GMOCK_INCLUDE_GMOCK_GMOCK_PRINTERS_H_

#include <ostream>  // NOLINT
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <gmock/internal/gmock-internal-utils.h>
#include <gmock/internal/gmock-port.h>
#include <gtest/gtest.h>

namespace testing {

// Definitions in the 'internal' and 'internal2' name spaces are
// subject to change without notice.  DO NOT USE THEM IN USER CODE!
namespace internal2 {

// Prints the given number of bytes in the given object to the given
// ostream.
void PrintBytesInObjectTo(const unsigned char* obj_bytes,
                          size_t count,
                          ::std::ostream* os);

// TypeWithoutFormatter<T, kIsProto>::PrintValue(value, os) is called
// by the universal printer to print a value of type T when neither
// operator<< nor PrintTo() is defined for type T.  When T is
// ProtocolMessage, proto2::Message, or a subclass of those, kIsProto
// will be true and the short debug string of the protocol message
// value will be printed; otherwise kIsProto will be false and the
// bytes in the value will be printed.
template <typename T, bool kIsProto>
class TypeWithoutFormatter {
 public:
  static void PrintValue(const T& value, ::std::ostream* os) {
    PrintBytesInObjectTo(reinterpret_cast<const unsigned char*>(&value),
                         sizeof(value), os);
  }
};
template <typename T>
class TypeWithoutFormatter<T, true> {
 public:
  static void PrintValue(const T& value, ::std::ostream* os) {
    // Both ProtocolMessage and proto2::Message have the
    // ShortDebugString() method, so the same implementation works for
    // both.
    ::std::operator<<(*os, "<" + value.ShortDebugString() + ">");
  }
};

// Prints the given value to the given ostream.  If the value is a
// protocol message, its short debug string is printed; otherwise the
// bytes in the value are printed.  This is what
// UniversalPrinter<T>::Print() does when it knows nothing about type
// T and T has no << operator.
//
// A user can override this behavior for a class type Foo by defining
// a << operator in the namespace where Foo is defined.
//
// We put this operator in namespace 'internal2' instead of 'internal'
// to simplify the implementation, as much code in 'internal' needs to
// use << in STL, which would conflict with our own << were it defined
// in 'internal'.
//
// Note that this operator<< takes a generic std::basic_ostream<Char,
// CharTraits> type instead of the more restricted std::ostream.  If
// we define it to take an std::ostream instead, we'll get an
// "ambiguous overloads" compiler error when trying to print a type
// Foo that supports streaming to std::basic_ostream<Char,
// CharTraits>, as the compiler cannot tell whether
// operator<<(std::ostream&, const T&) or
// operator<<(std::basic_stream<Char, CharTraits>, const Foo&) is more
// specific.
template <typename Char, typename CharTraits, typename T>
::std::basic_ostream<Char, CharTraits>& operator<<(
    ::std::basic_ostream<Char, CharTraits>& os, const T& x) {
  TypeWithoutFormatter<T, ::testing::internal::IsAProtocolMessage<T>::value>::
      PrintValue(x, &os);
  return os;
}

}  // namespace internal2
}  // namespace testing

// This namespace MUST NOT BE NESTED IN ::testing, or the name look-up
// magic needed for implementing UniversalPrinter won't work.
namespace testing_internal {

// Used to print a value that is not an STL-style container when the
// user doesn't define PrintTo() for it.
template <typename T>
void DefaultPrintNonContainerTo(const T& value, ::std::ostream* os) {
  // With the following statement, during unqualified name lookup,
  // testing::internal2::operator<< appears as if it was declared in
  // the nearest enclosing namespace that contains both
  // ::testing_internal and ::testing::internal2, i.e. the global
  // namespace.  For more details, refer to the C++ Standard section
  // 7.3.4-1 [namespace.udir].  This allows us to fall back onto
  // testing::internal2::operator<< in case T doesn't come with a <<
  // operator.
  //
  // We cannot write 'using ::testing::internal2::operator<<;', which
  // gcc 3.3 fails to compile due to a compiler bug.
  using namespace ::testing::internal2;  // NOLINT

  // Assuming T is defined in namespace foo, in the next statement,
  // the compiler will consider all of:
  //
  //   1. foo::operator<< (thanks to Koenig look-up),
  //   2. ::operator<< (as the current namespace is enclosed in ::),
  //   3. testing::internal2::operator<< (thanks to the using statement above).
  //
  // The operator<< whose type matches T best will be picked.
  //
  // We deliberately allow #2 to be a candidate, as sometimes it's
  // impossible to define #1 (e.g. when foo is ::std, defining
  // anything in it is undefined behavior unless you are a compiler
  // vendor.).
  *os << value;
}

}  // namespace testing_internal

namespace testing {
namespace internal {

// UniversalPrinter<T>::Print(value, ostream_ptr) prints the given
// value to the given ostream.  The caller must ensure that
// 'ostream_ptr' is not NULL, or the behavior is undefined.
//
// We define UniversalPrinter as a class template (as opposed to a
// function template), as we need to partially specialize it for
// reference types, which cannot be done with function templates.
template <typename T>
class UniversalPrinter;

// Used to print an STL-style container when the user doesn't define
// a PrintTo() for it.
template <typename C>
void DefaultPrintTo(IsContainer /* dummy */,
                    false_type /* is not a pointer */,
                    const C& container, ::std::ostream* os) {
  const size_t kMaxCount = 32;  // The maximum number of elements to print.
  *os << '{';
  size_t count = 0;
  for (typename C::const_iterator it = container.begin();
       it != container.end(); ++it, ++count) {
    if (count > 0) {
      *os << ',';
      if (count == kMaxCount) {  // Enough has been printed.
        *os << " ...";
        break;
      }
    }
    *os << ' ';
    PrintTo(*it, os);
  }

  if (count > 0) {
    *os << ' ';
  }
  *os << '}';
}

// Used to print a pointer that is neither a char pointer nor a member
// pointer, when the user doesn't define PrintTo() for it.  (A member
// variable pointer or member function pointer doesn't really point to
// a location in the address space.  Their representation is
// implementation-defined.  Therefore they will be printed as raw
// bytes.)
template <typename T>
void DefaultPrintTo(IsNotContainer /* dummy */,
                    true_type /* is a pointer */,
                    T* p, ::std::ostream* os) {
  if (p == NULL) {
    *os << "NULL";
  } else {
    // We cannot use implicit_cast or static_cast here, as they don't
    // work when p is a function pointer.
    *os << reinterpret_cast<const void*>(p);
  }
}

// Used to print a non-container, non-pointer value when the user
// doesn't define PrintTo() for it.
template <typename T>
void DefaultPrintTo(IsNotContainer /* dummy */,
                    false_type /* is not a pointer */,
                    const T& value, ::std::ostream* os) {
  ::testing_internal::DefaultPrintNonContainerTo(value, os);
}

// Prints the given value using the << operator if it has one;
// otherwise prints the bytes in it.  This is what
// UniversalPrinter<T>::Print() does when PrintTo() is not specialized
// or overloaded for type T.
//
// A user can override this behavior for a class type Foo by defining
// an overload of PrintTo() in the namespace where Foo is defined.  We
// give the user this option as sometimes defining a << operator for
// Foo is not desirable (e.g. the coding style may prevent doing it,
// or there is already a << operator but it doesn't do what the user
// wants).
template <typename T>
void PrintTo(const T& value, ::std::ostream* os) {
  // DefaultPrintTo() is overloaded.  The type of its first two
  // arguments determine which version will be picked.  If T is an
  // STL-style container, the version for container will be called; if
  // T is a pointer, the pointer version will be called; otherwise the
  // generic version will be called.
  //
  // Note that we check for container types here, prior to we check
  // for protocol message types in our operator<<.  The rationale is:
  //
  // For protocol messages, we want to give people a chance to
  // override Google Mock's format by defining a PrintTo() or
  // operator<<.  For STL containers, other formats can be
  // incompatible with Google Mock's format for the container
  // elements; therefore we check for container types here to ensure
  // that our format is used.
  //
  // The second argument of DefaultPrintTo() is needed to bypass a bug
  // in Symbian's C++ compiler that prevents it from picking the right
  // overload between:
  //
  //   PrintTo(const T& x, ...);
  //   PrintTo(T* x, ...);
  DefaultPrintTo(IsContainerTest<T>(0), is_pointer<T>(), value, os);
}

// The following list of PrintTo() overloads tells
// UniversalPrinter<T>::Print() how to print standard types (built-in
// types, strings, plain arrays, and pointers).

// Overloads for various char types.
void PrintCharTo(char c, int char_code, ::std::ostream* os);
inline void PrintTo(unsigned char c, ::std::ostream* os) {
  PrintCharTo(c, c, os);
}
inline void PrintTo(signed char c, ::std::ostream* os) {
  PrintCharTo(c, c, os);
}
inline void PrintTo(char c, ::std::ostream* os) {
  // When printing a plain char, we always treat it as unsigned.  This
  // way, the output won't be affected by whether the compiler thinks
  // char is signed or not.
  PrintTo(static_cast<unsigned char>(c), os);
}

// Overloads for other simple built-in types.
inline void PrintTo(bool x, ::std::ostream* os) {
  *os << (x ? "true" : "false");
}

// Overload for wchar_t type.
// Prints a wchar_t as a symbol if it is printable or as its internal
// code otherwise and also as its decimal code (except for L'\0').
// The L'\0' char is printed as "L'\\0'". The decimal code is printed
// as signed integer when wchar_t is implemented by the compiler
// as a signed type and is printed as an unsigned integer when wchar_t
// is implemented as an unsigned type.
void PrintTo(wchar_t wc, ::std::ostream* os);

// Overloads for C strings.
void PrintTo(const char* s, ::std::ostream* os);
inline void PrintTo(char* s, ::std::ostream* os) {
  PrintTo(implicit_cast<const char*>(s), os);
}

// MSVC can be configured to define wchar_t as a typedef of unsigned
// short.  It defines _NATIVE_WCHAR_T_DEFINED when wchar_t is a native
// type.  When wchar_t is a typedef, defining an overload for const
// wchar_t* would cause unsigned short* be printed as a wide string,
// possibly causing invalid memory accesses.
#if !defined(_MSC_VER) || defined(_NATIVE_WCHAR_T_DEFINED)
// Overloads for wide C strings
void PrintTo(const wchar_t* s, ::std::ostream* os);
inline void PrintTo(wchar_t* s, ::std::ostream* os) {
  PrintTo(implicit_cast<const wchar_t*>(s), os);
}
#endif

// Overload for C arrays.  Multi-dimensional arrays are printed
// properly.

// Prints the given number of elements in an array, without printing
// the curly braces.
template <typename T>
void PrintRawArrayTo(const T a[], size_t count, ::std::ostream* os) {
  UniversalPrinter<T>::Print(a[0], os);
  for (size_t i = 1; i != count; i++) {
    *os << ", ";
    UniversalPrinter<T>::Print(a[i], os);
  }
}

// Overloads for ::string and ::std::string.
#if GTEST_HAS_GLOBAL_STRING
void PrintStringTo(const ::string&s, ::std::ostream* os);
inline void PrintTo(const ::string& s, ::std::ostream* os) {
  PrintStringTo(s, os);
}
#endif  // GTEST_HAS_GLOBAL_STRING

#if GTEST_HAS_STD_STRING
void PrintStringTo(const ::std::string&s, ::std::ostream* os);
inline void PrintTo(const ::std::string& s, ::std::ostream* os) {
  PrintStringTo(s, os);
}
#endif  // GTEST_HAS_STD_STRING

// Overloads for ::wstring and ::std::wstring.
#if GTEST_HAS_GLOBAL_WSTRING
void PrintWideStringTo(const ::wstring&s, ::std::ostream* os);
inline void PrintTo(const ::wstring& s, ::std::ostream* os) {
  PrintWideStringTo(s, os);
}
#endif  // GTEST_HAS_GLOBAL_WSTRING

#if GTEST_HAS_STD_WSTRING
void PrintWideStringTo(const ::std::wstring&s, ::std::ostream* os);
inline void PrintTo(const ::std::wstring& s, ::std::ostream* os) {
  PrintWideStringTo(s, os);
}
#endif  // GTEST_HAS_STD_WSTRING

// Overload for ::std::tr1::tuple.  Needed for printing function
// arguments, which are packed as tuples.

typedef ::std::vector<string> Strings;

// This helper template allows PrintTo() for tuples and
// UniversalTersePrintTupleFieldsToStrings() to be defined by
// induction on the number of tuple fields.  The idea is that
// TuplePrefixPrinter<N>::PrintPrefixTo(t, os) prints the first N
// fields in tuple t, and can be defined in terms of
// TuplePrefixPrinter<N - 1>.

// The inductive case.
template <size_t N>
struct TuplePrefixPrinter {
  // Prints the first N fields of a tuple.
  template <typename Tuple>
  static void PrintPrefixTo(const Tuple& t, ::std::ostream* os) {
    TuplePrefixPrinter<N - 1>::PrintPrefixTo(t, os);
    *os << ", ";
    UniversalPrinter<typename ::std::tr1::tuple_element<N - 1, Tuple>::type>
        ::Print(::std::tr1::get<N - 1>(t), os);
  }

  // Tersely prints the first N fields of a tuple to a string vector,
  // one element for each field.
  template <typename Tuple>
  static void TersePrintPrefixToStrings(const Tuple& t, Strings* strings) {
    TuplePrefixPrinter<N - 1>::TersePrintPrefixToStrings(t, strings);
    ::std::stringstream ss;
    UniversalTersePrint(::std::tr1::get<N - 1>(t), &ss);
    strings->push_back(ss.str());
  }
};

// Base cases.
template <>
struct TuplePrefixPrinter<0> {
  template <typename Tuple>
  static void PrintPrefixTo(const Tuple&, ::std::ostream*) {}

  template <typename Tuple>
  static void TersePrintPrefixToStrings(const Tuple&, Strings*) {}
};
template <>
template <typename Tuple>
void TuplePrefixPrinter<1>::PrintPrefixTo(const Tuple& t, ::std::ostream* os) {
  UniversalPrinter<typename ::std::tr1::tuple_element<0, Tuple>::type>::
      Print(::std::tr1::get<0>(t), os);
}

// Helper function for printing a tuple.  T must be instantiated with
// a tuple type.
template <typename T>
void PrintTupleTo(const T& t, ::std::ostream* os) {
  *os << "(";
  TuplePrefixPrinter< ::std::tr1::tuple_size<T>::value>::
      PrintPrefixTo(t, os);
  *os << ")";
}

// Overloaded PrintTo() for tuples of various arities.  We support
// tuples of up-to 10 fields.  The following implementation works
// regardless of whether tr1::tuple is implemented using the
// non-standard variadic template feature or not.

inline void PrintTo(const ::std::tr1::tuple<>& t, ::std::ostream* os) {
  PrintTupleTo(t, os);
}

template <typename T1>
void PrintTo(const ::std::tr1::tuple<T1>& t, ::std::ostream* os) {
  PrintTupleTo(t, os);
}

template <typename T1, typename T2>
void PrintTo(const ::std::tr1::tuple<T1, T2>& t, ::std::ostream* os) {
  PrintTupleTo(t, os);
}

template <typename T1, typename T2, typename T3>
void PrintTo(const ::std::tr1::tuple<T1, T2, T3>& t, ::std::ostream* os) {
  PrintTupleTo(t, os);
}

template <typename T1, typename T2, typename T3, typename T4>
void PrintTo(const ::std::tr1::tuple<T1, T2, T3, T4>& t, ::std::ostream* os) {
  PrintTupleTo(t, os);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5>
void PrintTo(const ::std::tr1::tuple<T1, T2, T3, T4, T5>& t,
             ::std::ostream* os) {
  PrintTupleTo(t, os);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5,
          typename T6>
void PrintTo(const ::std::tr1::tuple<T1, T2, T3, T4, T5, T6>& t,
             ::std::ostream* os) {
  PrintTupleTo(t, os);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5,
          typename T6, typename T7>
void PrintTo(const ::std::tr1::tuple<T1, T2, T3, T4, T5, T6, T7>& t,
             ::std::ostream* os) {
  PrintTupleTo(t, os);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5,
          typename T6, typename T7, typename T8>
void PrintTo(const ::std::tr1::tuple<T1, T2, T3, T4, T5, T6, T7, T8>& t,
             ::std::ostream* os) {
  PrintTupleTo(t, os);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5,
          typename T6, typename T7, typename T8, typename T9>
void PrintTo(const ::std::tr1::tuple<T1, T2, T3, T4, T5, T6, T7, T8, T9>& t,
             ::std::ostream* os) {
  PrintTupleTo(t, os);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5,
          typename T6, typename T7, typename T8, typename T9, typename T10>
void PrintTo(
    const ::std::tr1::tuple<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10>& t,
    ::std::ostream* os) {
  PrintTupleTo(t, os);
}

// Overload for std::pair.
template <typename T1, typename T2>
void PrintTo(const ::std::pair<T1, T2>& value, ::std::ostream* os) {
  *os << '(';
  UniversalPrinter<T1>::Print(value.first, os);
  *os << ", ";
  UniversalPrinter<T2>::Print(value.second, os);
  *os << ')';
}

// Implements printing a non-reference type T by letting the compiler
// pick the right overload of PrintTo() for T.
template <typename T>
class UniversalPrinter {
 public:
  // MSVC warns about adding const to a function type, so we want to
  // disable the warning.
#ifdef _MSC_VER
#pragma warning(push)          // Saves the current warning state.
#pragma warning(disable:4180)  // Temporarily disables warning 4180.
#endif  // _MSC_VER

  // Note: we deliberately don't call this PrintTo(), as that name
  // conflicts with ::testing::internal::PrintTo in the body of the
  // function.
  static void Print(const T& value, ::std::ostream* os) {
    // By default, ::testing::internal::PrintTo() is used for printing
    // the value.
    //
    // Thanks to Koenig look-up, if T is a class and has its own
    // PrintTo() function defined in its namespace, that function will
    // be visible here.  Since it is more specific than the generic ones
    // in ::testing::internal, it will be picked by the compiler in the
    // following statement - exactly what we want.
    PrintTo(value, os);
  }

  // A convenient wrapper for Print() that returns the print-out as a
  // string.
  static string PrintToString(const T& value) {
    ::std::stringstream ss;
    Print(value, &ss);
    return ss.str();
  }

#ifdef _MSC_VER
#pragma warning(pop)           // Restores the warning state.
#endif  // _MSC_VER
};

// Implements printing an array type T[N].
template <typename T, size_t N>
class UniversalPrinter<T[N]> {
 public:
  // Prints the given array, omitting some elements when there are too
  // many.
  static void Print(const T (&a)[N], ::std::ostream* os) {
    // Prints a char array as a C string.  Note that we compare 'const
    // T' with 'const char' instead of comparing T with char, in case
    // that T is already a const type.
    if (internal::type_equals<const T, const char>::value) {
      UniversalPrinter<const T*>::Print(a, os);
      return;
    }

    if (N == 0) {
      *os << "{}";
    } else {
      *os << "{ ";
      const size_t kThreshold = 18;
      const size_t kChunkSize = 8;
      // If the array has more than kThreshold elements, we'll have to
      // omit some details by printing only the first and the last
      // kChunkSize elements.
      // TODO(wan): let the user control the threshold using a flag.
      if (N <= kThreshold) {
        PrintRawArrayTo(a, N, os);
      } else {
        PrintRawArrayTo(a, kChunkSize, os);
        *os << ", ..., ";
        PrintRawArrayTo(a + N - kChunkSize, kChunkSize, os);
      }
      *os << " }";
    }
  }

  // A convenient wrapper for Print() that returns the print-out as a
  // string.
  static string PrintToString(const T (&a)[N]) {
    ::std::stringstream ss;
    Print(a, &ss);
    return ss.str();
  }
};

// Implements printing a reference type T&.
template <typename T>
class UniversalPrinter<T&> {
 public:
  // MSVC warns about adding const to a function type, so we want to
  // disable the warning.
#ifdef _MSC_VER
#pragma warning(push)          // Saves the current warning state.
#pragma warning(disable:4180)  // Temporarily disables warning 4180.
#endif  // _MSC_VER

  static void Print(const T& value, ::std::ostream* os) {
    // Prints the address of the value.  We use reinterpret_cast here
    // as static_cast doesn't compile when T is a function type.
    *os << "@" << reinterpret_cast<const void*>(&value) << " ";

    // Then prints the value itself.
    UniversalPrinter<T>::Print(value, os);
  }

  // A convenient wrapper for Print() that returns the print-out as a
  // string.
  static string PrintToString(const T& value) {
    ::std::stringstream ss;
    Print(value, &ss);
    return ss.str();
  }

#ifdef _MSC_VER
#pragma warning(pop)           // Restores the warning state.
#endif  // _MSC_VER
};

// Prints a value tersely: for a reference type, the referenced value
// (but not the address) is printed; for a (const) char pointer, the
// NUL-terminated string (but not the pointer) is printed.
template <typename T>
void UniversalTersePrint(const T& value, ::std::ostream* os) {
  UniversalPrinter<T>::Print(value, os);
}
inline void UniversalTersePrint(const char* str, ::std::ostream* os) {
  if (str == NULL) {
    *os << "NULL";
  } else {
    UniversalPrinter<string>::Print(string(str), os);
  }
}
inline void UniversalTersePrint(char* str, ::std::ostream* os) {
  UniversalTersePrint(static_cast<const char*>(str), os);
}

// Prints the fields of a tuple tersely to a string vector, one
// element for each field.  See the comment before
// UniversalTersePrint() for how we define "tersely".
template <typename Tuple>
Strings UniversalTersePrintTupleFieldsToStrings(const Tuple& value) {
  Strings result;
  TuplePrefixPrinter< ::std::tr1::tuple_size<Tuple>::value>::
      TersePrintPrefixToStrings(value, &result);
  return result;
}

}  // namespace internal
}  // namespace testing

#endif  // GMOCK_INCLUDE_GMOCK_GMOCK_PRINTERS_H_
