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
// This file defines some utilities useful for implementing Google
// Mock.  They are subject to change without notice, so please DO NOT
// USE THEM IN USER CODE.

#ifndef GMOCK_INCLUDE_GMOCK_INTERNAL_GMOCK_INTERNAL_UTILS_H_
#define GMOCK_INCLUDE_GMOCK_INTERNAL_GMOCK_INTERNAL_UTILS_H_

#include <stdio.h>
#include <ostream>  // NOLINT
#include <string>

#include <gmock/internal/gmock-generated-internal-utils.h>
#include <gmock/internal/gmock-port.h>
#include <gtest/gtest.h>

// Concatenates two pre-processor symbols; works for concatenating
// built-in macros like __FILE__ and __LINE__.
#define GMOCK_CONCAT_TOKEN_IMPL_(foo, bar) foo##bar
#define GMOCK_CONCAT_TOKEN_(foo, bar) GMOCK_CONCAT_TOKEN_IMPL_(foo, bar)

#ifdef __GNUC__
#define GMOCK_ATTRIBUTE_UNUSED_ __attribute__ ((unused))
#else
#define GMOCK_ATTRIBUTE_UNUSED_
#endif  // __GNUC__

class ProtocolMessage;
namespace proto2 { class Message; }

namespace testing {
namespace internal {

// Converts an identifier name to a space-separated list of lower-case
// words.  Each maximum substring of the form [A-Za-z][a-z]*|\d+ is
// treated as one word.  For example, both "FooBar123" and
// "foo_bar_123" are converted to "foo bar 123".
string ConvertIdentifierNameToWords(const char* id_name);

// Defining a variable of type CompileAssertTypesEqual<T1, T2> will cause a
// compiler error iff T1 and T2 are different types.
template <typename T1, typename T2>
struct CompileAssertTypesEqual;

template <typename T>
struct CompileAssertTypesEqual<T, T> {
};

// Removes the reference from a type if it is a reference type,
// otherwise leaves it unchanged.  This is the same as
// tr1::remove_reference, which is not widely available yet.
template <typename T>
struct RemoveReference { typedef T type; };  // NOLINT
template <typename T>
struct RemoveReference<T&> { typedef T type; };  // NOLINT

// A handy wrapper around RemoveReference that works when the argument
// T depends on template parameters.
#define GMOCK_REMOVE_REFERENCE_(T) \
    typename ::testing::internal::RemoveReference<T>::type

// Removes const from a type if it is a const type, otherwise leaves
// it unchanged.  This is the same as tr1::remove_const, which is not
// widely available yet.
template <typename T>
struct RemoveConst { typedef T type; };  // NOLINT
template <typename T>
struct RemoveConst<const T> { typedef T type; };  // NOLINT

// MSVC 8.0 has a bug which causes the above definition to fail to
// remove the const in 'const int[3]'.  The following specialization
// works around the bug.  However, it causes trouble with gcc and thus
// needs to be conditionally compiled.
#ifdef _MSC_VER
template <typename T, size_t N>
struct RemoveConst<T[N]> {
  typedef typename RemoveConst<T>::type type[N];
};
#endif  // _MSC_VER

// A handy wrapper around RemoveConst that works when the argument
// T depends on template parameters.
#define GMOCK_REMOVE_CONST_(T) \
    typename ::testing::internal::RemoveConst<T>::type

// Adds reference to a type if it is not a reference type,
// otherwise leaves it unchanged.  This is the same as
// tr1::add_reference, which is not widely available yet.
template <typename T>
struct AddReference { typedef T& type; };  // NOLINT
template <typename T>
struct AddReference<T&> { typedef T& type; };  // NOLINT

// A handy wrapper around AddReference that works when the argument T
// depends on template parameters.
#define GMOCK_ADD_REFERENCE_(T) \
    typename ::testing::internal::AddReference<T>::type

// Adds a reference to const on top of T as necessary.  For example,
// it transforms
//
//   char         ==> const char&
//   const char   ==> const char&
//   char&        ==> const char&
//   const char&  ==> const char&
//
// The argument T must depend on some template parameters.
#define GMOCK_REFERENCE_TO_CONST_(T) \
    GMOCK_ADD_REFERENCE_(const GMOCK_REMOVE_REFERENCE_(T))

// PointeeOf<Pointer>::type is the type of a value pointed to by a
// Pointer, which can be either a smart pointer or a raw pointer.  The
// following default implementation is for the case where Pointer is a
// smart pointer.
template <typename Pointer>
struct PointeeOf {
  // Smart pointer classes define type element_type as the type of
  // their pointees.
  typedef typename Pointer::element_type type;
};
// This specialization is for the raw pointer case.
template <typename T>
struct PointeeOf<T*> { typedef T type; };  // NOLINT

// GetRawPointer(p) returns the raw pointer underlying p when p is a
// smart pointer, or returns p itself when p is already a raw pointer.
// The following default implementation is for the smart pointer case.
template <typename Pointer>
inline typename Pointer::element_type* GetRawPointer(const Pointer& p) {
  return p.get();
}
// This overloaded version is for the raw pointer case.
template <typename Element>
inline Element* GetRawPointer(Element* p) { return p; }

// This comparator allows linked_ptr to be stored in sets.
template <typename T>
struct LinkedPtrLessThan {
  bool operator()(const ::testing::internal::linked_ptr<T>& lhs,
                  const ::testing::internal::linked_ptr<T>& rhs) const {
    return lhs.get() < rhs.get();
  }
};

// ImplicitlyConvertible<From, To>::value is a compile-time bool
// constant that's true iff type From can be implicitly converted to
// type To.
template <typename From, typename To>
class ImplicitlyConvertible {
 private:
  // We need the following helper functions only for their types.
  // They have no implementations.

  // MakeFrom() is an expression whose type is From.  We cannot simply
  // use From(), as the type From may not have a public default
  // constructor.
  static From MakeFrom();

  // These two functions are overloaded.  Given an expression
  // Helper(x), the compiler will pick the first version if x can be
  // implicitly converted to type To; otherwise it will pick the
  // second version.
  //
  // The first version returns a value of size 1, and the second
  // version returns a value of size 2.  Therefore, by checking the
  // size of Helper(x), which can be done at compile time, we can tell
  // which version of Helper() is used, and hence whether x can be
  // implicitly converted to type To.
  static char Helper(To);
  static char (&Helper(...))[2];  // NOLINT

  // We have to put the 'public' section after the 'private' section,
  // or MSVC refuses to compile the code.
 public:
  // MSVC warns about implicitly converting from double to int for
  // possible loss of data, so we need to temporarily disable the
  // warning.
#ifdef _MSC_VER
#pragma warning(push)          // Saves the current warning state.
#pragma warning(disable:4244)  // Temporarily disables warning 4244.
  static const bool value =
      sizeof(Helper(ImplicitlyConvertible::MakeFrom())) == 1;
#pragma warning(pop)           // Restores the warning state.
#else
  static const bool value =
      sizeof(Helper(ImplicitlyConvertible::MakeFrom())) == 1;
#endif  // _MSV_VER
};
template <typename From, typename To>
const bool ImplicitlyConvertible<From, To>::value;

// In what follows, we use the term "kind" to indicate whether a type
// is bool, an integer type (excluding bool), a floating-point type,
// or none of them.  This categorization is useful for determining
// when a matcher argument type can be safely converted to another
// type in the implementation of SafeMatcherCast.
enum TypeKind {
  kBool, kInteger, kFloatingPoint, kOther
};

// KindOf<T>::value is the kind of type T.
template <typename T> struct KindOf {
  enum { value = kOther };  // The default kind.
};

// This macro declares that the kind of 'type' is 'kind'.
#define GMOCK_DECLARE_KIND_(type, kind) \
  template <> struct KindOf<type> { enum { value = kind }; }

GMOCK_DECLARE_KIND_(bool, kBool);

// All standard integer types.
GMOCK_DECLARE_KIND_(char, kInteger);
GMOCK_DECLARE_KIND_(signed char, kInteger);
GMOCK_DECLARE_KIND_(unsigned char, kInteger);
GMOCK_DECLARE_KIND_(short, kInteger);  // NOLINT
GMOCK_DECLARE_KIND_(unsigned short, kInteger);  // NOLINT
GMOCK_DECLARE_KIND_(int, kInteger);
GMOCK_DECLARE_KIND_(unsigned int, kInteger);
GMOCK_DECLARE_KIND_(long, kInteger);  // NOLINT
GMOCK_DECLARE_KIND_(unsigned long, kInteger);  // NOLINT

// MSVC can be configured to define wchar_t as a typedef of unsigned
// short. It defines _NATIVE_WCHAR_T_DEFINED symbol when wchar_t is a
// native type.
#if !defined(_MSC_VER) || defined(_NATIVE_WCHAR_T_DEFINED)
GMOCK_DECLARE_KIND_(wchar_t, kInteger);
#endif

// Non-standard integer types.
GMOCK_DECLARE_KIND_(Int64, kInteger);
GMOCK_DECLARE_KIND_(UInt64, kInteger);

// All standard floating-point types.
GMOCK_DECLARE_KIND_(float, kFloatingPoint);
GMOCK_DECLARE_KIND_(double, kFloatingPoint);
GMOCK_DECLARE_KIND_(long double, kFloatingPoint);

#undef GMOCK_DECLARE_KIND_

// Evaluates to the kind of 'type'.
#define GMOCK_KIND_OF_(type) \
  static_cast< ::testing::internal::TypeKind>( \
      ::testing::internal::KindOf<type>::value)

// Evaluates to true iff integer type T is signed.
#define GMOCK_IS_SIGNED_(T) (static_cast<T>(-1) < 0)

// LosslessArithmeticConvertibleImpl<kFromKind, From, kToKind, To>::value
// is true iff arithmetic type From can be losslessly converted to
// arithmetic type To.
//
// It's the user's responsibility to ensure that both From and To are
// raw (i.e. has no CV modifier, is not a pointer, and is not a
// reference) built-in arithmetic types, kFromKind is the kind of
// From, and kToKind is the kind of To; the value is
// implementation-defined when the above pre-condition is violated.
template <TypeKind kFromKind, typename From, TypeKind kToKind, typename To>
struct LosslessArithmeticConvertibleImpl : public false_type {};

// Converting bool to bool is lossless.
template <>
struct LosslessArithmeticConvertibleImpl<kBool, bool, kBool, bool>
    : public true_type {};  // NOLINT

// Converting bool to any integer type is lossless.
template <typename To>
struct LosslessArithmeticConvertibleImpl<kBool, bool, kInteger, To>
    : public true_type {};  // NOLINT

// Converting bool to any floating-point type is lossless.
template <typename To>
struct LosslessArithmeticConvertibleImpl<kBool, bool, kFloatingPoint, To>
    : public true_type {};  // NOLINT

// Converting an integer to bool is lossy.
template <typename From>
struct LosslessArithmeticConvertibleImpl<kInteger, From, kBool, bool>
    : public false_type {};  // NOLINT

// Converting an integer to another non-bool integer is lossless iff
// the target type's range encloses the source type's range.
template <typename From, typename To>
struct LosslessArithmeticConvertibleImpl<kInteger, From, kInteger, To>
    : public bool_constant<
      // When converting from a smaller size to a larger size, we are
      // fine as long as we are not converting from signed to unsigned.
      ((sizeof(From) < sizeof(To)) &&
       (!GMOCK_IS_SIGNED_(From) || GMOCK_IS_SIGNED_(To))) ||
      // When converting between the same size, the signedness must match.
      ((sizeof(From) == sizeof(To)) &&
       (GMOCK_IS_SIGNED_(From) == GMOCK_IS_SIGNED_(To)))> {};  // NOLINT

#undef GMOCK_IS_SIGNED_

// Converting an integer to a floating-point type may be lossy, since
// the format of a floating-point number is implementation-defined.
template <typename From, typename To>
struct LosslessArithmeticConvertibleImpl<kInteger, From, kFloatingPoint, To>
    : public false_type {};  // NOLINT

// Converting a floating-point to bool is lossy.
template <typename From>
struct LosslessArithmeticConvertibleImpl<kFloatingPoint, From, kBool, bool>
    : public false_type {};  // NOLINT

// Converting a floating-point to an integer is lossy.
template <typename From, typename To>
struct LosslessArithmeticConvertibleImpl<kFloatingPoint, From, kInteger, To>
    : public false_type {};  // NOLINT

// Converting a floating-point to another floating-point is lossless
// iff the target type is at least as big as the source type.
template <typename From, typename To>
struct LosslessArithmeticConvertibleImpl<
  kFloatingPoint, From, kFloatingPoint, To>
    : public bool_constant<sizeof(From) <= sizeof(To)> {};  // NOLINT

// LosslessArithmeticConvertible<From, To>::value is true iff arithmetic
// type From can be losslessly converted to arithmetic type To.
//
// It's the user's responsibility to ensure that both From and To are
// raw (i.e. has no CV modifier, is not a pointer, and is not a
// reference) built-in arithmetic types; the value is
// implementation-defined when the above pre-condition is violated.
template <typename From, typename To>
struct LosslessArithmeticConvertible
    : public LosslessArithmeticConvertibleImpl<
  GMOCK_KIND_OF_(From), From, GMOCK_KIND_OF_(To), To> {};  // NOLINT

// IsAProtocolMessage<T>::value is a compile-time bool constant that's
// true iff T is type ProtocolMessage, proto2::Message, or a subclass
// of those.
template <typename T>
struct IsAProtocolMessage
    : public bool_constant<
  ImplicitlyConvertible<const T*, const ::ProtocolMessage*>::value ||
  ImplicitlyConvertible<const T*, const ::proto2::Message*>::value> {
};

// When the compiler sees expression IsContainerTest<C>(0), the first
// overload of IsContainerTest will be picked if C is an STL-style
// container class (since C::const_iterator* is a valid type and 0 can
// be converted to it), while the second overload will be picked
// otherwise (since C::const_iterator will be an invalid type in this
// case).  Therefore, we can determine whether C is a container class
// by checking the type of IsContainerTest<C>(0).  The value of the
// expression is insignificant.
typedef int IsContainer;
template <class C>
IsContainer IsContainerTest(typename C::const_iterator*) { return 0; }

typedef char IsNotContainer;
template <class C>
IsNotContainer IsContainerTest(...) { return '\0'; }

// This interface knows how to report a Google Mock failure (either
// non-fatal or fatal).
class FailureReporterInterface {
 public:
  // The type of a failure (either non-fatal or fatal).
  enum FailureType {
    NONFATAL, FATAL
  };

  virtual ~FailureReporterInterface() {}

  // Reports a failure that occurred at the given source file location.
  virtual void ReportFailure(FailureType type, const char* file, int line,
                             const string& message) = 0;
};

// Returns the failure reporter used by Google Mock.
FailureReporterInterface* GetFailureReporter();

// Asserts that condition is true; aborts the process with the given
// message if condition is false.  We cannot use LOG(FATAL) or CHECK()
// as Google Mock might be used to mock the log sink itself.  We
// inline this function to prevent it from showing up in the stack
// trace.
inline void Assert(bool condition, const char* file, int line,
                   const string& msg) {
  if (!condition) {
    GetFailureReporter()->ReportFailure(FailureReporterInterface::FATAL,
                                        file, line, msg);
  }
}
inline void Assert(bool condition, const char* file, int line) {
  Assert(condition, file, line, "Assertion failed.");
}

// Verifies that condition is true; generates a non-fatal failure if
// condition is false.
inline void Expect(bool condition, const char* file, int line,
                   const string& msg) {
  if (!condition) {
    GetFailureReporter()->ReportFailure(FailureReporterInterface::NONFATAL,
                                        file, line, msg);
  }
}
inline void Expect(bool condition, const char* file, int line) {
  Expect(condition, file, line, "Expectation failed.");
}

// Severity level of a log.
enum LogSeverity {
  INFO = 0,
  WARNING = 1,
};

// Valid values for the --gmock_verbose flag.

// All logs (informational and warnings) are printed.
const char kInfoVerbosity[] = "info";
// Only warnings are printed.
const char kWarningVerbosity[] = "warning";
// No logs are printed.
const char kErrorVerbosity[] = "error";

// Returns true iff a log with the given severity is visible according
// to the --gmock_verbose flag.
bool LogIsVisible(LogSeverity severity);

// Prints the given message to stdout iff 'severity' >= the level
// specified by the --gmock_verbose flag.  If stack_frames_to_skip >=
// 0, also prints the stack trace excluding the top
// stack_frames_to_skip frames.  In opt mode, any positive
// stack_frames_to_skip is treated as 0, since we don't know which
// function calls will be inlined by the compiler and need to be
// conservative.
void Log(LogSeverity severity, const string& message, int stack_frames_to_skip);

// TODO(wan@google.com): group all type utilities together.

// Type traits.

// is_reference<T>::value is non-zero iff T is a reference type.
template <typename T> struct is_reference : public false_type {};
template <typename T> struct is_reference<T&> : public true_type {};

// type_equals<T1, T2>::value is non-zero iff T1 and T2 are the same type.
template <typename T1, typename T2> struct type_equals : public false_type {};
template <typename T> struct type_equals<T, T> : public true_type {};

// remove_reference<T>::type removes the reference from type T, if any.
template <typename T> struct remove_reference { typedef T type; };  // NOLINT
template <typename T> struct remove_reference<T&> { typedef T type; }; // NOLINT

// Invalid<T>() returns an invalid value of type T.  This is useful
// when a value of type T is needed for compilation, but the statement
// will not really be executed (or we don't care if the statement
// crashes).
template <typename T>
inline T Invalid() {
  return *static_cast<typename remove_reference<T>::type*>(NULL);
}
template <>
inline void Invalid<void>() {}

// Utilities for native arrays.

// ArrayEq() compares two k-dimensional native arrays using the
// elements' operator==, where k can be any integer >= 0.  When k is
// 0, ArrayEq() degenerates into comparing a single pair of values.

template <typename T, typename U>
bool ArrayEq(const T* lhs, size_t size, const U* rhs);

// This generic version is used when k is 0.
template <typename T, typename U>
inline bool ArrayEq(const T& lhs, const U& rhs) { return lhs == rhs; }

// This overload is used when k >= 1.
template <typename T, typename U, size_t N>
inline bool ArrayEq(const T(&lhs)[N], const U(&rhs)[N]) {
  return internal::ArrayEq(lhs, N, rhs);
}

// This helper reduces code bloat.  If we instead put its logic inside
// the previous ArrayEq() function, arrays with different sizes would
// lead to different copies of the template code.
template <typename T, typename U>
bool ArrayEq(const T* lhs, size_t size, const U* rhs) {
  for (size_t i = 0; i != size; i++) {
    if (!internal::ArrayEq(lhs[i], rhs[i]))
      return false;
  }
  return true;
}

// Finds the first element in the iterator range [begin, end) that
// equals elem.  Element may be a native array type itself.
template <typename Iter, typename Element>
Iter ArrayAwareFind(Iter begin, Iter end, const Element& elem) {
  for (Iter it = begin; it != end; ++it) {
    if (internal::ArrayEq(*it, elem))
      return it;
  }
  return end;
}

// CopyArray() copies a k-dimensional native array using the elements'
// operator=, where k can be any integer >= 0.  When k is 0,
// CopyArray() degenerates into copying a single value.

template <typename T, typename U>
void CopyArray(const T* from, size_t size, U* to);

// This generic version is used when k is 0.
template <typename T, typename U>
inline void CopyArray(const T& from, U* to) { *to = from; }

// This overload is used when k >= 1.
template <typename T, typename U, size_t N>
inline void CopyArray(const T(&from)[N], U(*to)[N]) {
  internal::CopyArray(from, N, *to);
}

// This helper reduces code bloat.  If we instead put its logic inside
// the previous CopyArray() function, arrays with different sizes
// would lead to different copies of the template code.
template <typename T, typename U>
void CopyArray(const T* from, size_t size, U* to) {
  for (size_t i = 0; i != size; i++) {
    internal::CopyArray(from[i], to + i);
  }
}

// The relation between an NativeArray object (see below) and the
// native array it represents.
enum RelationToSource {
  kReference,  // The NativeArray references the native array.
  kCopy        // The NativeArray makes a copy of the native array and
               // owns the copy.
};

// Adapts a native array to a read-only STL-style container.  Instead
// of the complete STL container concept, this adaptor only implements
// members useful for Google Mock's container matchers.  New members
// should be added as needed.  To simplify the implementation, we only
// support Element being a raw type (i.e. having no top-level const or
// reference modifier).  It's the client's responsibility to satisfy
// this requirement.  Element can be an array type itself (hence
// multi-dimensional arrays are supported).
template <typename Element>
class NativeArray {
 public:
  // STL-style container typedefs.
  typedef Element value_type;
  typedef const Element* const_iterator;

  // Constructs from a native array passed by reference.
  template <size_t N>
  NativeArray(const Element (&array)[N], RelationToSource relation) {
    Init(array, N, relation);
  }

  // Constructs from a native array passed by a pointer and a size.
  // For generality we don't artificially restrict the types of the
  // pointer and the size.
  template <typename Pointer, typename Size>
  NativeArray(const ::std::tr1::tuple<Pointer, Size>& array,
              RelationToSource relation) {
    Init(internal::GetRawPointer(::std::tr1::get<0>(array)),
         ::std::tr1::get<1>(array),
         relation);
  }

  // Copy constructor.
  NativeArray(const NativeArray& rhs) {
    Init(rhs.array_, rhs.size_, rhs.relation_to_source_);
  }

  ~NativeArray() {
    // Ensures that the user doesn't instantiate NativeArray with a
    // const or reference type.
    testing::StaticAssertTypeEq<Element,
        GMOCK_REMOVE_CONST_(GMOCK_REMOVE_REFERENCE_(Element))>();
    if (relation_to_source_ == kCopy)
      delete[] array_;
  }

  // STL-style container methods.
  size_t size() const { return size_; }
  const_iterator begin() const { return array_; }
  const_iterator end() const { return array_ + size_; }
  bool operator==(const NativeArray& rhs) const {
    return size() == rhs.size() &&
        ArrayEq(begin(), size(), rhs.begin());
  }

 private:
  // Not implemented as we don't want to support assignment.
  void operator=(const NativeArray& rhs);

  // Initializes this object; makes a copy of the input array if
  // 'relation' is kCopy.
  void Init(const Element* array, size_t size, RelationToSource relation) {
    if (relation == kReference) {
      array_ = array;
    } else {
      Element* const copy = new Element[size];
      CopyArray(array, size, copy);
      array_ = copy;
    }
    size_ = size;
    relation_to_source_ = relation;
  }

  const Element* array_;
  size_t size_;
  RelationToSource relation_to_source_;
};

// Given a raw type (i.e. having no top-level reference or const
// modifier) RawContainer that's either an STL-style container or a
// native array, class StlContainerView<RawContainer> has the
// following members:
//
//   - type is a type that provides an STL-style container view to
//     (i.e. implements the STL container concept for) RawContainer;
//   - const_reference is a type that provides a reference to a const
//     RawContainer;
//   - ConstReference(raw_container) returns a const reference to an STL-style
//     container view to raw_container, which is a RawContainer.
//   - Copy(raw_container) returns an STL-style container view of a
//     copy of raw_container, which is a RawContainer.
//
// This generic version is used when RawContainer itself is already an
// STL-style container.
template <class RawContainer>
class StlContainerView {
 public:
  typedef RawContainer type;
  typedef const type& const_reference;

  static const_reference ConstReference(const RawContainer& container) {
    // Ensures that RawContainer is not a const type.
    testing::StaticAssertTypeEq<RawContainer,
        GMOCK_REMOVE_CONST_(RawContainer)>();
    return container;
  }
  static type Copy(const RawContainer& container) { return container; }
};

// This specialization is used when RawContainer is a native array type.
template <typename Element, size_t N>
class StlContainerView<Element[N]> {
 public:
  typedef GMOCK_REMOVE_CONST_(Element) RawElement;
  typedef internal::NativeArray<RawElement> type;
  // NativeArray<T> can represent a native array either by value or by
  // reference (selected by a constructor argument), so 'const type'
  // can be used to reference a const native array.  We cannot
  // 'typedef const type& const_reference' here, as that would mean
  // ConstReference() has to return a reference to a local variable.
  typedef const type const_reference;

  static const_reference ConstReference(const Element (&array)[N]) {
    // Ensures that Element is not a const type.
    testing::StaticAssertTypeEq<Element, RawElement>();
    return type(array, kReference);
  }
  static type Copy(const Element (&array)[N]) {
    return type(array, kCopy);
  }
};

// This specialization is used when RawContainer is a native array
// represented as a (pointer, size) tuple.
template <typename ElementPointer, typename Size>
class StlContainerView< ::std::tr1::tuple<ElementPointer, Size> > {
 public:
  typedef GMOCK_REMOVE_CONST_(
      typename internal::PointeeOf<ElementPointer>::type) RawElement;
  typedef internal::NativeArray<RawElement> type;
  typedef const type const_reference;

  static const_reference ConstReference(
      const ::std::tr1::tuple<ElementPointer, Size>& array) {
    return type(array, kReference);
  }
  static type Copy(const ::std::tr1::tuple<ElementPointer, Size>& array) {
    return type(array, kCopy);
  }
};

// The following specialization prevents the user from instantiating
// StlContainer with a reference type.
template <typename T> class StlContainerView<T&>;

}  // namespace internal
}  // namespace testing

#endif  // GMOCK_INCLUDE_GMOCK_INTERNAL_GMOCK_INTERNAL_UTILS_H_
