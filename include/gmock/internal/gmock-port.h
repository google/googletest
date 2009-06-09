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
// Author: vadimb@google.com (Vadim Berman)
//
// Low-level types and utilities for porting Google Mock to various
// platforms.  They are subject to change without notice.  DO NOT USE
// THEM IN USER CODE.

#ifndef GMOCK_INCLUDE_GMOCK_INTERNAL_GMOCK_PORT_H_
#define GMOCK_INCLUDE_GMOCK_INTERNAL_GMOCK_PORT_H_

#include <assert.h>
#include <stdlib.h>
#include <iostream>

// Most of the types needed for porting Google Mock are also required
// for Google Test and are defined in gtest-port.h.
#include <gtest/internal/gtest-linked_ptr.h>
#include <gtest/internal/gtest-port.h>

// To avoid conditional compilation everywhere, we make it
// gmock-port.h's responsibility to #include the header implementing
// tr1/tuple.  gmock-port.h does this via gtest-port.h, which is
// guaranteed to pull in the tuple header.

#if GTEST_OS_LINUX

// On some platforms, <regex.h> needs someone to define size_t, and
// won't compile otherwise.  We can #include it here as we already
// included <stdlib.h>, which is guaranteed to define size_t through
// <stddef.h>.
#include <regex.h>  // NOLINT

// Defines this iff Google Mock uses the enhanced POSIX regular
// expression syntax.  This is public as it affects how a user uses
// regular expression matchers.
#define GMOCK_USES_POSIX_RE 1

#endif  // GTEST_OS_LINUX

#if defined(GMOCK_USES_PCRE) || defined(GMOCK_USES_POSIX_RE)
// Defines this iff regular expression matchers are supported.  This
// is public as it tells a user whether he can use regular expression
// matchers.
#define GMOCK_HAS_REGEX 1
#endif  // defined(GMOCK_USES_PCRE) || defined(GMOCK_USES_POSIX_RE)

namespace testing {
namespace internal {

// For Windows, check the compiler version. At least VS 2005 SP1 is
// required to compile Google Mock.
#if GTEST_OS_WINDOWS

#if _MSC_VER < 1400
#error "At least Visual Studio 2005 SP1 is required to compile Google Mock."
#elif _MSC_VER == 1400

// Unfortunately there is no unique _MSC_VER number for SP1. So for VS 2005
// we have to check if it has SP1 by checking whether a bug fixed in SP1
// is present. The bug in question is
// http://connect.microsoft.com/VisualStudio/feedback/ViewFeedback.aspx?FeedbackID=101702
// where the compiler incorrectly reports sizeof(poiter to an array).

class TestForSP1 {
 private:  // GCC complains if x_ is used by sizeof before defining it.
  static char x_[100];
  // VS 2005 RTM incorrectly reports sizeof(&x) as 100, and that value
  // is used to trigger 'invalid negative array size' error. If you
  // see this error, upgrade to VS 2005 SP1 since Google Mock will not
  // compile in VS 2005 RTM.
  static char Google_Mock_requires_Visual_Studio_2005_SP1_or_later_to_compile_[
      sizeof(&x_) != 100 ? 1 : -1];
};

#endif  // _MSC_VER
#endif  // GTEST_OS_WINDOWS

// Use implicit_cast as a safe version of static_cast or const_cast
// for upcasting in the type hierarchy (i.e. casting a pointer to Foo
// to a pointer to SuperclassOfFoo or casting a pointer to Foo to
// a const pointer to Foo).
// When you use implicit_cast, the compiler checks that the cast is safe.
// Such explicit implicit_casts are necessary in surprisingly many
// situations where C++ demands an exact type match instead of an
// argument type convertable to a target type.
//
// The From type can be inferred, so the preferred syntax for using
// implicit_cast is the same as for static_cast etc.:
//
//   implicit_cast<ToType>(expr)
//
// implicit_cast would have been part of the C++ standard library,
// but the proposal was submitted too late.  It will probably make
// its way into the language in the future.
template<typename To, typename From>
inline To implicit_cast(From const &f) {
  return f;
}

// When you upcast (that is, cast a pointer from type Foo to type
// SuperclassOfFoo), it's fine to use implicit_cast<>, since upcasts
// always succeed.  When you downcast (that is, cast a pointer from
// type Foo to type SubclassOfFoo), static_cast<> isn't safe, because
// how do you know the pointer is really of type SubclassOfFoo?  It
// could be a bare Foo, or of type DifferentSubclassOfFoo.  Thus,
// when you downcast, you should use this macro.  In debug mode, we
// use dynamic_cast<> to double-check the downcast is legal (we die
// if it's not).  In normal mode, we do the efficient static_cast<>
// instead.  Thus, it's important to test in debug mode to make sure
// the cast is legal!
//    This is the only place in the code we should use dynamic_cast<>.
// In particular, you SHOULDN'T be using dynamic_cast<> in order to
// do RTTI (eg code like this:
//    if (dynamic_cast<Subclass1>(foo)) HandleASubclass1Object(foo);
//    if (dynamic_cast<Subclass2>(foo)) HandleASubclass2Object(foo);
// You should design the code some other way not to need this.
template<typename To, typename From>  // use like this: down_cast<T*>(foo);
inline To down_cast(From* f) {  // so we only accept pointers
  // Ensures that To is a sub-type of From *.  This test is here only
  // for compile-time type checking, and has no overhead in an
  // optimized build at run-time, as it will be optimized away
  // completely.
  if (false) {
    implicit_cast<From*, To>(0);
  }

#if GTEST_HAS_RTTI
  assert(f == NULL || dynamic_cast<To>(f) != NULL);  // RTTI: debug mode only!
#endif
  return static_cast<To>(f);
}

// The GMOCK_COMPILE_ASSERT_ macro can be used to verify that a compile time
// expression is true. For example, you could use it to verify the
// size of a static array:
//
//   GMOCK_COMPILE_ASSERT_(ARRAYSIZE(content_type_names) == CONTENT_NUM_TYPES,
//                         content_type_names_incorrect_size);
//
// or to make sure a struct is smaller than a certain size:
//
//   GMOCK_COMPILE_ASSERT_(sizeof(foo) < 128, foo_too_large);
//
// The second argument to the macro is the name of the variable. If
// the expression is false, most compilers will issue a warning/error
// containing the name of the variable.

template <bool>
struct CompileAssert {
};

#define GMOCK_COMPILE_ASSERT_(expr, msg) \
  typedef ::testing::internal::CompileAssert<(bool(expr))> \
      msg[bool(expr) ? 1 : -1]

// Implementation details of GMOCK_COMPILE_ASSERT_:
//
// - GMOCK_COMPILE_ASSERT_ works by defining an array type that has -1
//   elements (and thus is invalid) when the expression is false.
//
// - The simpler definition
//
//    #define GMOCK_COMPILE_ASSERT_(expr, msg) typedef char msg[(expr) ? 1 : -1]
//
//   does not work, as gcc supports variable-length arrays whose sizes
//   are determined at run-time (this is gcc's extension and not part
//   of the C++ standard).  As a result, gcc fails to reject the
//   following code with the simple definition:
//
//     int foo;
//     GMOCK_COMPILE_ASSERT_(foo, msg); // not supposed to compile as foo is
//                                      // not a compile-time constant.
//
// - By using the type CompileAssert<(bool(expr))>, we ensures that
//   expr is a compile-time constant.  (Template arguments must be
//   determined at compile-time.)
//
// - The outter parentheses in CompileAssert<(bool(expr))> are necessary
//   to work around a bug in gcc 3.4.4 and 4.0.1.  If we had written
//
//     CompileAssert<bool(expr)>
//
//   instead, these compilers will refuse to compile
//
//     GMOCK_COMPILE_ASSERT_(5 > 0, some_message);
//
//   (They seem to think the ">" in "5 > 0" marks the end of the
//   template argument list.)
//
// - The array size is (bool(expr) ? 1 : -1), instead of simply
//
//     ((expr) ? 1 : -1).
//
//   This is to avoid running into a bug in MS VC 7.1, which
//   causes ((0.0) ? 1 : -1) to incorrectly evaluate to 1.

#if GTEST_HAS_GLOBAL_STRING
typedef ::string string;
#elif GTEST_HAS_STD_STRING
typedef ::std::string string;
#else
#error "Google Mock requires ::std::string to compile."
#endif  // GTEST_HAS_GLOBAL_STRING

#if GTEST_HAS_GLOBAL_WSTRING
typedef ::wstring wstring;
#elif GTEST_HAS_STD_WSTRING
typedef ::std::wstring wstring;
#endif  // GTEST_HAS_GLOBAL_WSTRING

// Prints the file location in the format native to the compiler.
inline void FormatFileLocation(const char* file, int line, ::std::ostream* os) {
  if (file == NULL)
    file = "unknown file";
  if (line < 0) {
    *os << file << ":";
  } else {
#if _MSC_VER
    *os << file << "(" << line << "):";
#else
    *os << file << ":" << line << ":";
#endif
  }
}

// INTERNAL IMPLEMENTATION - DO NOT USE.
//
// GMOCK_CHECK_ is an all mode assert. It aborts the program if the condition
// is not satisfied.
//  Synopsys:
//    GMOCK_CHECK_(boolean_condition);
//     or
//    GMOCK_CHECK_(boolean_condition) << "Additional message";
//
//    This checks the condition and if the condition is not satisfied
//    it prints message about the condition violation, including the
//    condition itself, plus additional message streamed into it, if any,
//    and then it aborts the program. It aborts the program irrespective of
//    whether it is built in the debug mode or not.

class GMockCheckProvider {
 public:
  GMockCheckProvider(const char* condition, const char* file, int line) {
    FormatFileLocation(file, line, &::std::cerr);
    ::std::cerr << " ERROR: Condition " << condition << " failed. ";
  }
  ~GMockCheckProvider() {
    ::std::cerr << ::std::endl;
    abort();
  }
  ::std::ostream& GetStream() { return ::std::cerr; }
};
#define GMOCK_CHECK_(condition) \
    GTEST_AMBIGUOUS_ELSE_BLOCKER_ \
    if (condition) \
      ; \
    else \
      ::testing::internal::GMockCheckProvider(\
          #condition, __FILE__, __LINE__).GetStream()

}  // namespace internal
}  // namespace testing

// Macro for referencing flags.  This is public as we want the user to
// use this syntax to reference Google Mock flags.
#define GMOCK_FLAG(name) FLAGS_gmock_##name

// Macros for declaring flags.
#define GMOCK_DECLARE_bool_(name) extern bool GMOCK_FLAG(name)
#define GMOCK_DECLARE_int32_(name) \
    extern ::testing::internal::Int32 GMOCK_FLAG(name)
#define GMOCK_DECLARE_string_(name) \
    extern ::testing::internal::String GMOCK_FLAG(name)

// Macros for defining flags.
#define GMOCK_DEFINE_bool_(name, default_val, doc) \
    bool GMOCK_FLAG(name) = (default_val)
#define GMOCK_DEFINE_int32_(name, default_val, doc) \
    ::testing::internal::Int32 GMOCK_FLAG(name) = (default_val)
#define GMOCK_DEFINE_string_(name, default_val, doc) \
    ::testing::internal::String GMOCK_FLAG(name) = (default_val)

#endif  // GMOCK_INCLUDE_GMOCK_INTERNAL_GMOCK_PORT_H_
