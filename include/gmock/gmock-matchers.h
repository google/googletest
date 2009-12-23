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
// This file implements some commonly used argument matchers.  More
// matchers can be defined by the user implementing the
// MatcherInterface<T> interface if necessary.

#ifndef GMOCK_INCLUDE_GMOCK_GMOCK_MATCHERS_H_
#define GMOCK_INCLUDE_GMOCK_GMOCK_MATCHERS_H_

#include <algorithm>
#include <limits>
#include <ostream>  // NOLINT
#include <sstream>
#include <string>
#include <vector>

#include <gmock/gmock-printers.h>
#include <gmock/internal/gmock-internal-utils.h>
#include <gmock/internal/gmock-port.h>
#include <gtest/gtest.h>

namespace testing {

// To implement a matcher Foo for type T, define:
//   1. a class FooMatcherImpl that implements the
//      MatcherInterface<T> interface, and
//   2. a factory function that creates a Matcher<T> object from a
//      FooMatcherImpl*.
//
// The two-level delegation design makes it possible to allow a user
// to write "v" instead of "Eq(v)" where a Matcher is expected, which
// is impossible if we pass matchers by pointers.  It also eases
// ownership management as Matcher objects can now be copied like
// plain values.

// The implementation of a matcher.
template <typename T>
class MatcherInterface {
 public:
  virtual ~MatcherInterface() {}

  // Returns true iff the matcher matches x.
  virtual bool Matches(T x) const = 0;

  // Describes this matcher to an ostream.
  virtual void DescribeTo(::std::ostream* os) const = 0;

  // Describes the negation of this matcher to an ostream.  For
  // example, if the description of this matcher is "is greater than
  // 7", the negated description could be "is not greater than 7".
  // You are not required to override this when implementing
  // MatcherInterface, but it is highly advised so that your matcher
  // can produce good error messages.
  virtual void DescribeNegationTo(::std::ostream* os) const {
    *os << "not (";
    DescribeTo(os);
    *os << ")";
  }

  // Explains why x matches, or doesn't match, the matcher.  Override
  // this to provide any additional information that helps a user
  // understand the match result.
  virtual void ExplainMatchResultTo(T /* x */, ::std::ostream* /* os */) const {
    // By default, nothing more needs to be explained, as Google Mock
    // has already printed the value of x when this function is
    // called.
  }
};

namespace internal {

// An internal class for implementing Matcher<T>, which will derive
// from it.  We put functionalities common to all Matcher<T>
// specializations here to avoid code duplication.
template <typename T>
class MatcherBase {
 public:
  // Returns true iff this matcher matches x.
  bool Matches(T x) const { return impl_->Matches(x); }

  // Describes this matcher to an ostream.
  void DescribeTo(::std::ostream* os) const { impl_->DescribeTo(os); }

  // Describes the negation of this matcher to an ostream.
  void DescribeNegationTo(::std::ostream* os) const {
    impl_->DescribeNegationTo(os);
  }

  // Explains why x matches, or doesn't match, the matcher.
  void ExplainMatchResultTo(T x, ::std::ostream* os) const {
    impl_->ExplainMatchResultTo(x, os);
  }

 protected:
  MatcherBase() {}

  // Constructs a matcher from its implementation.
  explicit MatcherBase(const MatcherInterface<T>* impl)
      : impl_(impl) {}

  virtual ~MatcherBase() {}

 private:
  // shared_ptr (util/gtl/shared_ptr.h) and linked_ptr have similar
  // interfaces.  The former dynamically allocates a chunk of memory
  // to hold the reference count, while the latter tracks all
  // references using a circular linked list without allocating
  // memory.  It has been observed that linked_ptr performs better in
  // typical scenarios.  However, shared_ptr can out-perform
  // linked_ptr when there are many more uses of the copy constructor
  // than the default constructor.
  //
  // If performance becomes a problem, we should see if using
  // shared_ptr helps.
  ::testing::internal::linked_ptr<const MatcherInterface<T> > impl_;
};

// The default implementation of ExplainMatchResultTo() for
// polymorphic matchers.
template <typename PolymorphicMatcherImpl, typename T>
inline void ExplainMatchResultTo(const PolymorphicMatcherImpl& /* impl */,
                                 const T& /* x */,
                                 ::std::ostream* /* os */) {
  // By default, nothing more needs to be said, as Google Mock already
  // prints the value of x elsewhere.
}

}  // namespace internal

// A Matcher<T> is a copyable and IMMUTABLE (except by assignment)
// object that can check whether a value of type T matches.  The
// implementation of Matcher<T> is just a linked_ptr to const
// MatcherInterface<T>, so copying is fairly cheap.  Don't inherit
// from Matcher!
template <typename T>
class Matcher : public internal::MatcherBase<T> {
 public:
  // Constructs a null matcher.  Needed for storing Matcher objects in
  // STL containers.
  Matcher() {}

  // Constructs a matcher from its implementation.
  explicit Matcher(const MatcherInterface<T>* impl)
      : internal::MatcherBase<T>(impl) {}

  // Implicit constructor here allows people to write
  // EXPECT_CALL(foo, Bar(5)) instead of EXPECT_CALL(foo, Bar(Eq(5))) sometimes
  Matcher(T value);  // NOLINT
};

// The following two specializations allow the user to write str
// instead of Eq(str) and "foo" instead of Eq("foo") when a string
// matcher is expected.
template <>
class Matcher<const internal::string&>
    : public internal::MatcherBase<const internal::string&> {
 public:
  Matcher() {}

  explicit Matcher(const MatcherInterface<const internal::string&>* impl)
      : internal::MatcherBase<const internal::string&>(impl) {}

  // Allows the user to write str instead of Eq(str) sometimes, where
  // str is a string object.
  Matcher(const internal::string& s);  // NOLINT

  // Allows the user to write "foo" instead of Eq("foo") sometimes.
  Matcher(const char* s);  // NOLINT
};

template <>
class Matcher<internal::string>
    : public internal::MatcherBase<internal::string> {
 public:
  Matcher() {}

  explicit Matcher(const MatcherInterface<internal::string>* impl)
      : internal::MatcherBase<internal::string>(impl) {}

  // Allows the user to write str instead of Eq(str) sometimes, where
  // str is a string object.
  Matcher(const internal::string& s);  // NOLINT

  // Allows the user to write "foo" instead of Eq("foo") sometimes.
  Matcher(const char* s);  // NOLINT
};

// The PolymorphicMatcher class template makes it easy to implement a
// polymorphic matcher (i.e. a matcher that can match values of more
// than one type, e.g. Eq(n) and NotNull()).
//
// To define a polymorphic matcher, a user first provides a Impl class
// that has a Matches() method, a DescribeTo() method, and a
// DescribeNegationTo() method.  The Matches() method is usually a
// method template (such that it works with multiple types).  Then the
// user creates the polymorphic matcher using
// MakePolymorphicMatcher().  To provide additional explanation to the
// match result, define a FREE function (or function template)
//
//   void ExplainMatchResultTo(const Impl& matcher, const Value& value,
//                             ::std::ostream* os);
//
// in the SAME NAME SPACE where Impl is defined.  See the definition
// of NotNull() for a complete example.
template <class Impl>
class PolymorphicMatcher {
 public:
  explicit PolymorphicMatcher(const Impl& an_impl) : impl_(an_impl) {}

  // Returns a mutable reference to the underlying matcher
  // implementation object.
  Impl& mutable_impl() { return impl_; }

  // Returns an immutable reference to the underlying matcher
  // implementation object.
  const Impl& impl() const { return impl_; }

  template <typename T>
  operator Matcher<T>() const {
    return Matcher<T>(new MonomorphicImpl<T>(impl_));
  }

 private:
  template <typename T>
  class MonomorphicImpl : public MatcherInterface<T> {
   public:
    explicit MonomorphicImpl(const Impl& impl) : impl_(impl) {}

    virtual bool Matches(T x) const { return impl_.Matches(x); }

    virtual void DescribeTo(::std::ostream* os) const {
      impl_.DescribeTo(os);
    }

    virtual void DescribeNegationTo(::std::ostream* os) const {
      impl_.DescribeNegationTo(os);
    }

    virtual void ExplainMatchResultTo(T x, ::std::ostream* os) const {
      using ::testing::internal::ExplainMatchResultTo;

      // C++ uses Argument-Dependent Look-up (aka Koenig Look-up) to
      // resolve the call to ExplainMatchResultTo() here.  This
      // means that if there's a ExplainMatchResultTo() function
      // defined in the name space where class Impl is defined, it
      // will be picked by the compiler as the better match.
      // Otherwise the default implementation of it in
      // ::testing::internal will be picked.
      //
      // This look-up rule lets a writer of a polymorphic matcher
      // customize the behavior of ExplainMatchResultTo() when he
      // cares to.  Nothing needs to be done by the writer if he
      // doesn't need to customize it.
      ExplainMatchResultTo(impl_, x, os);
    }

   private:
    const Impl impl_;

    GTEST_DISALLOW_ASSIGN_(MonomorphicImpl);
  };

  Impl impl_;

  GTEST_DISALLOW_ASSIGN_(PolymorphicMatcher);
};

// Creates a matcher from its implementation.  This is easier to use
// than the Matcher<T> constructor as it doesn't require you to
// explicitly write the template argument, e.g.
//
//   MakeMatcher(foo);
// vs
//   Matcher<const string&>(foo);
template <typename T>
inline Matcher<T> MakeMatcher(const MatcherInterface<T>* impl) {
  return Matcher<T>(impl);
};

// Creates a polymorphic matcher from its implementation.  This is
// easier to use than the PolymorphicMatcher<Impl> constructor as it
// doesn't require you to explicitly write the template argument, e.g.
//
//   MakePolymorphicMatcher(foo);
// vs
//   PolymorphicMatcher<TypeOfFoo>(foo);
template <class Impl>
inline PolymorphicMatcher<Impl> MakePolymorphicMatcher(const Impl& impl) {
  return PolymorphicMatcher<Impl>(impl);
}

// In order to be safe and clear, casting between different matcher
// types is done explicitly via MatcherCast<T>(m), which takes a
// matcher m and returns a Matcher<T>.  It compiles only when T can be
// statically converted to the argument type of m.
template <typename T, typename M>
Matcher<T> MatcherCast(M m);

// Implements SafeMatcherCast().
//
// We use an intermediate class to do the actual safe casting as Nokia's
// Symbian compiler cannot decide between
// template <T, M> ... (M) and
// template <T, U> ... (const Matcher<U>&)
// for function templates but can for member function templates.
template <typename T>
class SafeMatcherCastImpl {
 public:
  // This overload handles polymorphic matchers only since monomorphic
  // matchers are handled by the next one.
  template <typename M>
  static inline Matcher<T> Cast(M polymorphic_matcher) {
    return Matcher<T>(polymorphic_matcher);
  }

  // This overload handles monomorphic matchers.
  //
  // In general, if type T can be implicitly converted to type U, we can
  // safely convert a Matcher<U> to a Matcher<T> (i.e. Matcher is
  // contravariant): just keep a copy of the original Matcher<U>, convert the
  // argument from type T to U, and then pass it to the underlying Matcher<U>.
  // The only exception is when U is a reference and T is not, as the
  // underlying Matcher<U> may be interested in the argument's address, which
  // is not preserved in the conversion from T to U.
  template <typename U>
  static inline Matcher<T> Cast(const Matcher<U>& matcher) {
    // Enforce that T can be implicitly converted to U.
    GMOCK_COMPILE_ASSERT_((internal::ImplicitlyConvertible<T, U>::value),
                          T_must_be_implicitly_convertible_to_U);
    // Enforce that we are not converting a non-reference type T to a reference
    // type U.
    GMOCK_COMPILE_ASSERT_(
        internal::is_reference<T>::value || !internal::is_reference<U>::value,
        cannot_convert_non_referentce_arg_to_reference);
    // In case both T and U are arithmetic types, enforce that the
    // conversion is not lossy.
    typedef GMOCK_REMOVE_CONST_(GMOCK_REMOVE_REFERENCE_(T)) RawT;
    typedef GMOCK_REMOVE_CONST_(GMOCK_REMOVE_REFERENCE_(U)) RawU;
    const bool kTIsOther = GMOCK_KIND_OF_(RawT) == internal::kOther;
    const bool kUIsOther = GMOCK_KIND_OF_(RawU) == internal::kOther;
    GMOCK_COMPILE_ASSERT_(
        kTIsOther || kUIsOther ||
        (internal::LosslessArithmeticConvertible<RawT, RawU>::value),
        conversion_of_arithmetic_types_must_be_lossless);
    return MatcherCast<T>(matcher);
  }
};

template <typename T, typename M>
inline Matcher<T> SafeMatcherCast(const M& polymorphic_matcher) {
  return SafeMatcherCastImpl<T>::Cast(polymorphic_matcher);
}

// A<T>() returns a matcher that matches any value of type T.
template <typename T>
Matcher<T> A();

// Anything inside the 'internal' namespace IS INTERNAL IMPLEMENTATION
// and MUST NOT BE USED IN USER CODE!!!
namespace internal {

// Appends the explanation on the result of matcher.Matches(value) to
// os iff the explanation is not empty.
template <typename T>
void ExplainMatchResultAsNeededTo(const Matcher<T>& matcher, T value,
                                  ::std::ostream* os) {
  ::std::stringstream reason;
  matcher.ExplainMatchResultTo(value, &reason);
  const internal::string s = reason.str();
  if (s != "") {
    *os << " (" << s << ")";
  }
}

// An internal helper class for doing compile-time loop on a tuple's
// fields.
template <size_t N>
class TuplePrefix {
 public:
  // TuplePrefix<N>::Matches(matcher_tuple, value_tuple) returns true
  // iff the first N fields of matcher_tuple matches the first N
  // fields of value_tuple, respectively.
  template <typename MatcherTuple, typename ValueTuple>
  static bool Matches(const MatcherTuple& matcher_tuple,
                      const ValueTuple& value_tuple) {
    using ::std::tr1::get;
    return TuplePrefix<N - 1>::Matches(matcher_tuple, value_tuple)
        && get<N - 1>(matcher_tuple).Matches(get<N - 1>(value_tuple));
  }

  // TuplePrefix<N>::DescribeMatchFailuresTo(matchers, values, os)
  // describes failures in matching the first N fields of matchers
  // against the first N fields of values.  If there is no failure,
  // nothing will be streamed to os.
  template <typename MatcherTuple, typename ValueTuple>
  static void DescribeMatchFailuresTo(const MatcherTuple& matchers,
                                      const ValueTuple& values,
                                      ::std::ostream* os) {
    using ::std::tr1::tuple_element;
    using ::std::tr1::get;

    // First, describes failures in the first N - 1 fields.
    TuplePrefix<N - 1>::DescribeMatchFailuresTo(matchers, values, os);

    // Then describes the failure (if any) in the (N - 1)-th (0-based)
    // field.
    typename tuple_element<N - 1, MatcherTuple>::type matcher =
        get<N - 1>(matchers);
    typedef typename tuple_element<N - 1, ValueTuple>::type Value;
    Value value = get<N - 1>(values);
    if (!matcher.Matches(value)) {
      // TODO(wan): include in the message the name of the parameter
      // as used in MOCK_METHOD*() when possible.
      *os << "  Expected arg #" << N - 1 << ": ";
      get<N - 1>(matchers).DescribeTo(os);
      *os << "\n           Actual: ";
      // We remove the reference in type Value to prevent the
      // universal printer from printing the address of value, which
      // isn't interesting to the user most of the time.  The
      // matcher's ExplainMatchResultTo() method handles the case when
      // the address is interesting.
      internal::UniversalPrinter<GMOCK_REMOVE_REFERENCE_(Value)>::
          Print(value, os);
      ExplainMatchResultAsNeededTo<Value>(matcher, value, os);
      *os << "\n";
    }
  }
};

// The base case.
template <>
class TuplePrefix<0> {
 public:
  template <typename MatcherTuple, typename ValueTuple>
  static bool Matches(const MatcherTuple& /* matcher_tuple */,
                      const ValueTuple& /* value_tuple */) {
    return true;
  }

  template <typename MatcherTuple, typename ValueTuple>
  static void DescribeMatchFailuresTo(const MatcherTuple& /* matchers */,
                                      const ValueTuple& /* values */,
                                      ::std::ostream* /* os */) {}
};

// TupleMatches(matcher_tuple, value_tuple) returns true iff all
// matchers in matcher_tuple match the corresponding fields in
// value_tuple.  It is a compiler error if matcher_tuple and
// value_tuple have different number of fields or incompatible field
// types.
template <typename MatcherTuple, typename ValueTuple>
bool TupleMatches(const MatcherTuple& matcher_tuple,
                  const ValueTuple& value_tuple) {
  using ::std::tr1::tuple_size;
  // Makes sure that matcher_tuple and value_tuple have the same
  // number of fields.
  GMOCK_COMPILE_ASSERT_(tuple_size<MatcherTuple>::value ==
                        tuple_size<ValueTuple>::value,
                        matcher_and_value_have_different_numbers_of_fields);
  return TuplePrefix<tuple_size<ValueTuple>::value>::
      Matches(matcher_tuple, value_tuple);
}

// Describes failures in matching matchers against values.  If there
// is no failure, nothing will be streamed to os.
template <typename MatcherTuple, typename ValueTuple>
void DescribeMatchFailureTupleTo(const MatcherTuple& matchers,
                                 const ValueTuple& values,
                                 ::std::ostream* os) {
  using ::std::tr1::tuple_size;
  TuplePrefix<tuple_size<MatcherTuple>::value>::DescribeMatchFailuresTo(
      matchers, values, os);
}

// The MatcherCastImpl class template is a helper for implementing
// MatcherCast().  We need this helper in order to partially
// specialize the implementation of MatcherCast() (C++ allows
// class/struct templates to be partially specialized, but not
// function templates.).

// This general version is used when MatcherCast()'s argument is a
// polymorphic matcher (i.e. something that can be converted to a
// Matcher but is not one yet; for example, Eq(value)).
template <typename T, typename M>
class MatcherCastImpl {
 public:
  static Matcher<T> Cast(M polymorphic_matcher) {
    return Matcher<T>(polymorphic_matcher);
  }
};

// This more specialized version is used when MatcherCast()'s argument
// is already a Matcher.  This only compiles when type T can be
// statically converted to type U.
template <typename T, typename U>
class MatcherCastImpl<T, Matcher<U> > {
 public:
  static Matcher<T> Cast(const Matcher<U>& source_matcher) {
    return Matcher<T>(new Impl(source_matcher));
  }

 private:
  class Impl : public MatcherInterface<T> {
   public:
    explicit Impl(const Matcher<U>& source_matcher)
        : source_matcher_(source_matcher) {}

    // We delegate the matching logic to the source matcher.
    virtual bool Matches(T x) const {
      return source_matcher_.Matches(static_cast<U>(x));
    }

    virtual void DescribeTo(::std::ostream* os) const {
      source_matcher_.DescribeTo(os);
    }

    virtual void DescribeNegationTo(::std::ostream* os) const {
      source_matcher_.DescribeNegationTo(os);
    }

    virtual void ExplainMatchResultTo(T x, ::std::ostream* os) const {
      source_matcher_.ExplainMatchResultTo(static_cast<U>(x), os);
    }

   private:
    const Matcher<U> source_matcher_;

    GTEST_DISALLOW_ASSIGN_(Impl);
  };
};

// This even more specialized version is used for efficiently casting
// a matcher to its own type.
template <typename T>
class MatcherCastImpl<T, Matcher<T> > {
 public:
  static Matcher<T> Cast(const Matcher<T>& matcher) { return matcher; }
};

// Implements A<T>().
template <typename T>
class AnyMatcherImpl : public MatcherInterface<T> {
 public:
  virtual bool Matches(T /* x */) const { return true; }
  virtual void DescribeTo(::std::ostream* os) const { *os << "is anything"; }
  virtual void DescribeNegationTo(::std::ostream* os) const {
    // This is mostly for completeness' safe, as it's not very useful
    // to write Not(A<bool>()).  However we cannot completely rule out
    // such a possibility, and it doesn't hurt to be prepared.
    *os << "never matches";
  }
};

// Implements _, a matcher that matches any value of any
// type.  This is a polymorphic matcher, so we need a template type
// conversion operator to make it appearing as a Matcher<T> for any
// type T.
class AnythingMatcher {
 public:
  template <typename T>
  operator Matcher<T>() const { return A<T>(); }
};

// Implements a matcher that compares a given value with a
// pre-supplied value using one of the ==, <=, <, etc, operators.  The
// two values being compared don't have to have the same type.
//
// The matcher defined here is polymorphic (for example, Eq(5) can be
// used to match an int, a short, a double, etc).  Therefore we use
// a template type conversion operator in the implementation.
//
// We define this as a macro in order to eliminate duplicated source
// code.
//
// The following template definition assumes that the Rhs parameter is
// a "bare" type (i.e. neither 'const T' nor 'T&').
#define GMOCK_IMPLEMENT_COMPARISON_MATCHER_(name, op, relation) \
  template <typename Rhs> class name##Matcher { \
   public: \
    explicit name##Matcher(const Rhs& rhs) : rhs_(rhs) {} \
    template <typename Lhs> \
    operator Matcher<Lhs>() const { \
      return MakeMatcher(new Impl<Lhs>(rhs_)); \
    } \
   private: \
    template <typename Lhs> \
    class Impl : public MatcherInterface<Lhs> { \
     public: \
      explicit Impl(const Rhs& rhs) : rhs_(rhs) {} \
      virtual bool Matches(Lhs lhs) const { return lhs op rhs_; } \
      virtual void DescribeTo(::std::ostream* os) const { \
        *os << "is " relation  " "; \
        UniversalPrinter<Rhs>::Print(rhs_, os); \
      } \
      virtual void DescribeNegationTo(::std::ostream* os) const { \
        *os << "is not " relation  " "; \
        UniversalPrinter<Rhs>::Print(rhs_, os); \
      } \
     private: \
      Rhs rhs_; \
      GTEST_DISALLOW_ASSIGN_(Impl); \
    }; \
    Rhs rhs_; \
    GTEST_DISALLOW_ASSIGN_(name##Matcher); \
  }

// Implements Eq(v), Ge(v), Gt(v), Le(v), Lt(v), and Ne(v)
// respectively.
GMOCK_IMPLEMENT_COMPARISON_MATCHER_(Eq, ==, "equal to");
GMOCK_IMPLEMENT_COMPARISON_MATCHER_(Ge, >=, "greater than or equal to");
GMOCK_IMPLEMENT_COMPARISON_MATCHER_(Gt, >, "greater than");
GMOCK_IMPLEMENT_COMPARISON_MATCHER_(Le, <=, "less than or equal to");
GMOCK_IMPLEMENT_COMPARISON_MATCHER_(Lt, <, "less than");
GMOCK_IMPLEMENT_COMPARISON_MATCHER_(Ne, !=, "not equal to");

#undef GMOCK_IMPLEMENT_COMPARISON_MATCHER_

// Implements the polymorphic IsNull() matcher, which matches any raw or smart
// pointer that is NULL.
class IsNullMatcher {
 public:
  template <typename Pointer>
  bool Matches(const Pointer& p) const { return GetRawPointer(p) == NULL; }

  void DescribeTo(::std::ostream* os) const { *os << "is NULL"; }
  void DescribeNegationTo(::std::ostream* os) const {
    *os << "is not NULL";
  }
};

// Implements the polymorphic NotNull() matcher, which matches any raw or smart
// pointer that is not NULL.
class NotNullMatcher {
 public:
  template <typename Pointer>
  bool Matches(const Pointer& p) const { return GetRawPointer(p) != NULL; }

  void DescribeTo(::std::ostream* os) const { *os << "is not NULL"; }
  void DescribeNegationTo(::std::ostream* os) const {
    *os << "is NULL";
  }
};

// Ref(variable) matches any argument that is a reference to
// 'variable'.  This matcher is polymorphic as it can match any
// super type of the type of 'variable'.
//
// The RefMatcher template class implements Ref(variable).  It can
// only be instantiated with a reference type.  This prevents a user
// from mistakenly using Ref(x) to match a non-reference function
// argument.  For example, the following will righteously cause a
// compiler error:
//
//   int n;
//   Matcher<int> m1 = Ref(n);   // This won't compile.
//   Matcher<int&> m2 = Ref(n);  // This will compile.
template <typename T>
class RefMatcher;

template <typename T>
class RefMatcher<T&> {
  // Google Mock is a generic framework and thus needs to support
  // mocking any function types, including those that take non-const
  // reference arguments.  Therefore the template parameter T (and
  // Super below) can be instantiated to either a const type or a
  // non-const type.
 public:
  // RefMatcher() takes a T& instead of const T&, as we want the
  // compiler to catch using Ref(const_value) as a matcher for a
  // non-const reference.
  explicit RefMatcher(T& x) : object_(x) {}  // NOLINT

  template <typename Super>
  operator Matcher<Super&>() const {
    // By passing object_ (type T&) to Impl(), which expects a Super&,
    // we make sure that Super is a super type of T.  In particular,
    // this catches using Ref(const_value) as a matcher for a
    // non-const reference, as you cannot implicitly convert a const
    // reference to a non-const reference.
    return MakeMatcher(new Impl<Super>(object_));
  }

 private:
  template <typename Super>
  class Impl : public MatcherInterface<Super&> {
   public:
    explicit Impl(Super& x) : object_(x) {}  // NOLINT

    // Matches() takes a Super& (as opposed to const Super&) in
    // order to match the interface MatcherInterface<Super&>.
    virtual bool Matches(Super& x) const { return &x == &object_; }  // NOLINT

    virtual void DescribeTo(::std::ostream* os) const {
      *os << "references the variable ";
      UniversalPrinter<Super&>::Print(object_, os);
    }

    virtual void DescribeNegationTo(::std::ostream* os) const {
      *os << "does not reference the variable ";
      UniversalPrinter<Super&>::Print(object_, os);
    }

    virtual void ExplainMatchResultTo(Super& x,  // NOLINT
                                      ::std::ostream* os) const {
      *os << "is located @" << static_cast<const void*>(&x);
    }

   private:
    const Super& object_;

    GTEST_DISALLOW_ASSIGN_(Impl);
  };

  T& object_;

  GTEST_DISALLOW_ASSIGN_(RefMatcher);
};

// Polymorphic helper functions for narrow and wide string matchers.
inline bool CaseInsensitiveCStringEquals(const char* lhs, const char* rhs) {
  return String::CaseInsensitiveCStringEquals(lhs, rhs);
}

inline bool CaseInsensitiveCStringEquals(const wchar_t* lhs,
                                         const wchar_t* rhs) {
  return String::CaseInsensitiveWideCStringEquals(lhs, rhs);
}

// String comparison for narrow or wide strings that can have embedded NUL
// characters.
template <typename StringType>
bool CaseInsensitiveStringEquals(const StringType& s1,
                                 const StringType& s2) {
  // Are the heads equal?
  if (!CaseInsensitiveCStringEquals(s1.c_str(), s2.c_str())) {
    return false;
  }

  // Skip the equal heads.
  const typename StringType::value_type nul = 0;
  const size_t i1 = s1.find(nul), i2 = s2.find(nul);

  // Are we at the end of either s1 or s2?
  if (i1 == StringType::npos || i2 == StringType::npos) {
    return i1 == i2;
  }

  // Are the tails equal?
  return CaseInsensitiveStringEquals(s1.substr(i1 + 1), s2.substr(i2 + 1));
}

// String matchers.

// Implements equality-based string matchers like StrEq, StrCaseNe, and etc.
template <typename StringType>
class StrEqualityMatcher {
 public:
  typedef typename StringType::const_pointer ConstCharPointer;

  StrEqualityMatcher(const StringType& str, bool expect_eq,
                     bool case_sensitive)
      : string_(str), expect_eq_(expect_eq), case_sensitive_(case_sensitive) {}

  // When expect_eq_ is true, returns true iff s is equal to string_;
  // otherwise returns true iff s is not equal to string_.
  bool Matches(ConstCharPointer s) const {
    if (s == NULL) {
      return !expect_eq_;
    }
    return Matches(StringType(s));
  }

  bool Matches(const StringType& s) const {
    const bool eq = case_sensitive_ ? s == string_ :
        CaseInsensitiveStringEquals(s, string_);
    return expect_eq_ == eq;
  }

  void DescribeTo(::std::ostream* os) const {
    DescribeToHelper(expect_eq_, os);
  }

  void DescribeNegationTo(::std::ostream* os) const {
    DescribeToHelper(!expect_eq_, os);
  }

 private:
  void DescribeToHelper(bool expect_eq, ::std::ostream* os) const {
    *os << "is ";
    if (!expect_eq) {
      *os << "not ";
    }
    *os << "equal to ";
    if (!case_sensitive_) {
      *os << "(ignoring case) ";
    }
    UniversalPrinter<StringType>::Print(string_, os);
  }

  const StringType string_;
  const bool expect_eq_;
  const bool case_sensitive_;

  GTEST_DISALLOW_ASSIGN_(StrEqualityMatcher);
};

// Implements the polymorphic HasSubstr(substring) matcher, which
// can be used as a Matcher<T> as long as T can be converted to a
// string.
template <typename StringType>
class HasSubstrMatcher {
 public:
  typedef typename StringType::const_pointer ConstCharPointer;

  explicit HasSubstrMatcher(const StringType& substring)
      : substring_(substring) {}

  // These overloaded methods allow HasSubstr(substring) to be used as a
  // Matcher<T> as long as T can be converted to string.  Returns true
  // iff s contains substring_ as a substring.
  bool Matches(ConstCharPointer s) const {
    return s != NULL && Matches(StringType(s));
  }

  bool Matches(const StringType& s) const {
    return s.find(substring_) != StringType::npos;
  }

  // Describes what this matcher matches.
  void DescribeTo(::std::ostream* os) const {
    *os << "has substring ";
    UniversalPrinter<StringType>::Print(substring_, os);
  }

  void DescribeNegationTo(::std::ostream* os) const {
    *os << "has no substring ";
    UniversalPrinter<StringType>::Print(substring_, os);
  }

 private:
  const StringType substring_;

  GTEST_DISALLOW_ASSIGN_(HasSubstrMatcher);
};

// Implements the polymorphic StartsWith(substring) matcher, which
// can be used as a Matcher<T> as long as T can be converted to a
// string.
template <typename StringType>
class StartsWithMatcher {
 public:
  typedef typename StringType::const_pointer ConstCharPointer;

  explicit StartsWithMatcher(const StringType& prefix) : prefix_(prefix) {
  }

  // These overloaded methods allow StartsWith(prefix) to be used as a
  // Matcher<T> as long as T can be converted to string.  Returns true
  // iff s starts with prefix_.
  bool Matches(ConstCharPointer s) const {
    return s != NULL && Matches(StringType(s));
  }

  bool Matches(const StringType& s) const {
    return s.length() >= prefix_.length() &&
        s.substr(0, prefix_.length()) == prefix_;
  }

  void DescribeTo(::std::ostream* os) const {
    *os << "starts with ";
    UniversalPrinter<StringType>::Print(prefix_, os);
  }

  void DescribeNegationTo(::std::ostream* os) const {
    *os << "doesn't start with ";
    UniversalPrinter<StringType>::Print(prefix_, os);
  }

 private:
  const StringType prefix_;

  GTEST_DISALLOW_ASSIGN_(StartsWithMatcher);
};

// Implements the polymorphic EndsWith(substring) matcher, which
// can be used as a Matcher<T> as long as T can be converted to a
// string.
template <typename StringType>
class EndsWithMatcher {
 public:
  typedef typename StringType::const_pointer ConstCharPointer;

  explicit EndsWithMatcher(const StringType& suffix) : suffix_(suffix) {}

  // These overloaded methods allow EndsWith(suffix) to be used as a
  // Matcher<T> as long as T can be converted to string.  Returns true
  // iff s ends with suffix_.
  bool Matches(ConstCharPointer s) const {
    return s != NULL && Matches(StringType(s));
  }

  bool Matches(const StringType& s) const {
    return s.length() >= suffix_.length() &&
        s.substr(s.length() - suffix_.length()) == suffix_;
  }

  void DescribeTo(::std::ostream* os) const {
    *os << "ends with ";
    UniversalPrinter<StringType>::Print(suffix_, os);
  }

  void DescribeNegationTo(::std::ostream* os) const {
    *os << "doesn't end with ";
    UniversalPrinter<StringType>::Print(suffix_, os);
  }

 private:
  const StringType suffix_;

  GTEST_DISALLOW_ASSIGN_(EndsWithMatcher);
};

#if GMOCK_HAS_REGEX

// Implements polymorphic matchers MatchesRegex(regex) and
// ContainsRegex(regex), which can be used as a Matcher<T> as long as
// T can be converted to a string.
class MatchesRegexMatcher {
 public:
  MatchesRegexMatcher(const RE* regex, bool full_match)
      : regex_(regex), full_match_(full_match) {}

  // These overloaded methods allow MatchesRegex(regex) to be used as
  // a Matcher<T> as long as T can be converted to string.  Returns
  // true iff s matches regular expression regex.  When full_match_ is
  // true, a full match is done; otherwise a partial match is done.
  bool Matches(const char* s) const {
    return s != NULL && Matches(internal::string(s));
  }

  bool Matches(const internal::string& s) const {
    return full_match_ ? RE::FullMatch(s, *regex_) :
        RE::PartialMatch(s, *regex_);
  }

  void DescribeTo(::std::ostream* os) const {
    *os << (full_match_ ? "matches" : "contains")
        << " regular expression ";
    UniversalPrinter<internal::string>::Print(regex_->pattern(), os);
  }

  void DescribeNegationTo(::std::ostream* os) const {
    *os << "doesn't " << (full_match_ ? "match" : "contain")
        << " regular expression ";
    UniversalPrinter<internal::string>::Print(regex_->pattern(), os);
  }

 private:
  const internal::linked_ptr<const RE> regex_;
  const bool full_match_;

  GTEST_DISALLOW_ASSIGN_(MatchesRegexMatcher);
};

#endif  // GMOCK_HAS_REGEX

// Implements a matcher that compares the two fields of a 2-tuple
// using one of the ==, <=, <, etc, operators.  The two fields being
// compared don't have to have the same type.
//
// The matcher defined here is polymorphic (for example, Eq() can be
// used to match a tuple<int, short>, a tuple<const long&, double>,
// etc).  Therefore we use a template type conversion operator in the
// implementation.
//
// We define this as a macro in order to eliminate duplicated source
// code.
#define GMOCK_IMPLEMENT_COMPARISON2_MATCHER_(name, op) \
  class name##2Matcher { \
   public: \
    template <typename T1, typename T2> \
    operator Matcher<const ::std::tr1::tuple<T1, T2>&>() const { \
      return MakeMatcher(new Impl<T1, T2>); \
    } \
   private: \
    template <typename T1, typename T2> \
    class Impl : public MatcherInterface<const ::std::tr1::tuple<T1, T2>&> { \
     public: \
      virtual bool Matches(const ::std::tr1::tuple<T1, T2>& args) const { \
        return ::std::tr1::get<0>(args) op ::std::tr1::get<1>(args); \
      } \
      virtual void DescribeTo(::std::ostream* os) const { \
        *os << "are a pair (x, y) where x " #op " y"; \
      } \
      virtual void DescribeNegationTo(::std::ostream* os) const { \
        *os << "are a pair (x, y) where x " #op " y is false"; \
      } \
    }; \
  }

// Implements Eq(), Ge(), Gt(), Le(), Lt(), and Ne() respectively.
GMOCK_IMPLEMENT_COMPARISON2_MATCHER_(Eq, ==);
GMOCK_IMPLEMENT_COMPARISON2_MATCHER_(Ge, >=);
GMOCK_IMPLEMENT_COMPARISON2_MATCHER_(Gt, >);
GMOCK_IMPLEMENT_COMPARISON2_MATCHER_(Le, <=);
GMOCK_IMPLEMENT_COMPARISON2_MATCHER_(Lt, <);
GMOCK_IMPLEMENT_COMPARISON2_MATCHER_(Ne, !=);

#undef GMOCK_IMPLEMENT_COMPARISON2_MATCHER_

// Implements the Not(...) matcher for a particular argument type T.
// We do not nest it inside the NotMatcher class template, as that
// will prevent different instantiations of NotMatcher from sharing
// the same NotMatcherImpl<T> class.
template <typename T>
class NotMatcherImpl : public MatcherInterface<T> {
 public:
  explicit NotMatcherImpl(const Matcher<T>& matcher)
      : matcher_(matcher) {}

  virtual bool Matches(T x) const {
    return !matcher_.Matches(x);
  }

  virtual void DescribeTo(::std::ostream* os) const {
    matcher_.DescribeNegationTo(os);
  }

  virtual void DescribeNegationTo(::std::ostream* os) const {
    matcher_.DescribeTo(os);
  }

  virtual void ExplainMatchResultTo(T x, ::std::ostream* os) const {
    matcher_.ExplainMatchResultTo(x, os);
  }

 private:
  const Matcher<T> matcher_;

  GTEST_DISALLOW_ASSIGN_(NotMatcherImpl);
};

// Implements the Not(m) matcher, which matches a value that doesn't
// match matcher m.
template <typename InnerMatcher>
class NotMatcher {
 public:
  explicit NotMatcher(InnerMatcher matcher) : matcher_(matcher) {}

  // This template type conversion operator allows Not(m) to be used
  // to match any type m can match.
  template <typename T>
  operator Matcher<T>() const {
    return Matcher<T>(new NotMatcherImpl<T>(SafeMatcherCast<T>(matcher_)));
  }

 private:
  InnerMatcher matcher_;

  GTEST_DISALLOW_ASSIGN_(NotMatcher);
};

// Implements the AllOf(m1, m2) matcher for a particular argument type
// T. We do not nest it inside the BothOfMatcher class template, as
// that will prevent different instantiations of BothOfMatcher from
// sharing the same BothOfMatcherImpl<T> class.
template <typename T>
class BothOfMatcherImpl : public MatcherInterface<T> {
 public:
  BothOfMatcherImpl(const Matcher<T>& matcher1, const Matcher<T>& matcher2)
      : matcher1_(matcher1), matcher2_(matcher2) {}

  virtual bool Matches(T x) const {
    return matcher1_.Matches(x) && matcher2_.Matches(x);
  }

  virtual void DescribeTo(::std::ostream* os) const {
    *os << "(";
    matcher1_.DescribeTo(os);
    *os << ") and (";
    matcher2_.DescribeTo(os);
    *os << ")";
  }

  virtual void DescribeNegationTo(::std::ostream* os) const {
    *os << "not ";
    DescribeTo(os);
  }

  virtual void ExplainMatchResultTo(T x, ::std::ostream* os) const {
    if (Matches(x)) {
      // When both matcher1_ and matcher2_ match x, we need to
      // explain why *both* of them match.
      ::std::stringstream ss1;
      matcher1_.ExplainMatchResultTo(x, &ss1);
      const internal::string s1 = ss1.str();

      ::std::stringstream ss2;
      matcher2_.ExplainMatchResultTo(x, &ss2);
      const internal::string s2 = ss2.str();

      if (s1 == "") {
        *os << s2;
      } else {
        *os << s1;
        if (s2 != "") {
          *os << "; " << s2;
        }
      }
    } else {
      // Otherwise we only need to explain why *one* of them fails
      // to match.
      if (!matcher1_.Matches(x)) {
        matcher1_.ExplainMatchResultTo(x, os);
      } else {
        matcher2_.ExplainMatchResultTo(x, os);
      }
    }
  }

 private:
  const Matcher<T> matcher1_;
  const Matcher<T> matcher2_;

  GTEST_DISALLOW_ASSIGN_(BothOfMatcherImpl);
};

// Used for implementing the AllOf(m_1, ..., m_n) matcher, which
// matches a value that matches all of the matchers m_1, ..., and m_n.
template <typename Matcher1, typename Matcher2>
class BothOfMatcher {
 public:
  BothOfMatcher(Matcher1 matcher1, Matcher2 matcher2)
      : matcher1_(matcher1), matcher2_(matcher2) {}

  // This template type conversion operator allows a
  // BothOfMatcher<Matcher1, Matcher2> object to match any type that
  // both Matcher1 and Matcher2 can match.
  template <typename T>
  operator Matcher<T>() const {
    return Matcher<T>(new BothOfMatcherImpl<T>(SafeMatcherCast<T>(matcher1_),
                                               SafeMatcherCast<T>(matcher2_)));
  }

 private:
  Matcher1 matcher1_;
  Matcher2 matcher2_;

  GTEST_DISALLOW_ASSIGN_(BothOfMatcher);
};

// Implements the AnyOf(m1, m2) matcher for a particular argument type
// T.  We do not nest it inside the AnyOfMatcher class template, as
// that will prevent different instantiations of AnyOfMatcher from
// sharing the same EitherOfMatcherImpl<T> class.
template <typename T>
class EitherOfMatcherImpl : public MatcherInterface<T> {
 public:
  EitherOfMatcherImpl(const Matcher<T>& matcher1, const Matcher<T>& matcher2)
      : matcher1_(matcher1), matcher2_(matcher2) {}

  virtual bool Matches(T x) const {
    return matcher1_.Matches(x) || matcher2_.Matches(x);
  }

  virtual void DescribeTo(::std::ostream* os) const {
    *os << "(";
    matcher1_.DescribeTo(os);
    *os << ") or (";
    matcher2_.DescribeTo(os);
    *os << ")";
  }

  virtual void DescribeNegationTo(::std::ostream* os) const {
    *os << "not ";
    DescribeTo(os);
  }

  virtual void ExplainMatchResultTo(T x, ::std::ostream* os) const {
    if (Matches(x)) {
      // If either matcher1_ or matcher2_ matches x, we just need
      // to explain why *one* of them matches.
      if (matcher1_.Matches(x)) {
        matcher1_.ExplainMatchResultTo(x, os);
      } else {
        matcher2_.ExplainMatchResultTo(x, os);
      }
    } else {
      // Otherwise we need to explain why *neither* matches.
      ::std::stringstream ss1;
      matcher1_.ExplainMatchResultTo(x, &ss1);
      const internal::string s1 = ss1.str();

      ::std::stringstream ss2;
      matcher2_.ExplainMatchResultTo(x, &ss2);
      const internal::string s2 = ss2.str();

      if (s1 == "") {
        *os << s2;
      } else {
        *os << s1;
        if (s2 != "") {
          *os << "; " << s2;
        }
      }
    }
  }

 private:
  const Matcher<T> matcher1_;
  const Matcher<T> matcher2_;

  GTEST_DISALLOW_ASSIGN_(EitherOfMatcherImpl);
};

// Used for implementing the AnyOf(m_1, ..., m_n) matcher, which
// matches a value that matches at least one of the matchers m_1, ...,
// and m_n.
template <typename Matcher1, typename Matcher2>
class EitherOfMatcher {
 public:
  EitherOfMatcher(Matcher1 matcher1, Matcher2 matcher2)
      : matcher1_(matcher1), matcher2_(matcher2) {}

  // This template type conversion operator allows a
  // EitherOfMatcher<Matcher1, Matcher2> object to match any type that
  // both Matcher1 and Matcher2 can match.
  template <typename T>
  operator Matcher<T>() const {
    return Matcher<T>(new EitherOfMatcherImpl<T>(
        SafeMatcherCast<T>(matcher1_), SafeMatcherCast<T>(matcher2_)));
  }

 private:
  Matcher1 matcher1_;
  Matcher2 matcher2_;

  GTEST_DISALLOW_ASSIGN_(EitherOfMatcher);
};

// Used for implementing Truly(pred), which turns a predicate into a
// matcher.
template <typename Predicate>
class TrulyMatcher {
 public:
  explicit TrulyMatcher(Predicate pred) : predicate_(pred) {}

  // This method template allows Truly(pred) to be used as a matcher
  // for type T where T is the argument type of predicate 'pred'.  The
  // argument is passed by reference as the predicate may be
  // interested in the address of the argument.
  template <typename T>
  bool Matches(T& x) const {  // NOLINT
#if GTEST_OS_WINDOWS
    // MSVC warns about converting a value into bool (warning 4800).
#pragma warning(push)          // Saves the current warning state.
#pragma warning(disable:4800)  // Temporarily disables warning 4800.
#endif  // GTEST_OS_WINDOWS
    return predicate_(x);
#if GTEST_OS_WINDOWS
#pragma warning(pop)           // Restores the warning state.
#endif  // GTEST_OS_WINDOWS
  }

  void DescribeTo(::std::ostream* os) const {
    *os << "satisfies the given predicate";
  }

  void DescribeNegationTo(::std::ostream* os) const {
    *os << "doesn't satisfy the given predicate";
  }

 private:
  Predicate predicate_;

  GTEST_DISALLOW_ASSIGN_(TrulyMatcher);
};

// Used for implementing Matches(matcher), which turns a matcher into
// a predicate.
template <typename M>
class MatcherAsPredicate {
 public:
  explicit MatcherAsPredicate(M matcher) : matcher_(matcher) {}

  // This template operator() allows Matches(m) to be used as a
  // predicate on type T where m is a matcher on type T.
  //
  // The argument x is passed by reference instead of by value, as
  // some matcher may be interested in its address (e.g. as in
  // Matches(Ref(n))(x)).
  template <typename T>
  bool operator()(const T& x) const {
    // We let matcher_ commit to a particular type here instead of
    // when the MatcherAsPredicate object was constructed.  This
    // allows us to write Matches(m) where m is a polymorphic matcher
    // (e.g. Eq(5)).
    //
    // If we write Matcher<T>(matcher_).Matches(x) here, it won't
    // compile when matcher_ has type Matcher<const T&>; if we write
    // Matcher<const T&>(matcher_).Matches(x) here, it won't compile
    // when matcher_ has type Matcher<T>; if we just write
    // matcher_.Matches(x), it won't compile when matcher_ is
    // polymorphic, e.g. Eq(5).
    //
    // MatcherCast<const T&>() is necessary for making the code work
    // in all of the above situations.
    return MatcherCast<const T&>(matcher_).Matches(x);
  }

 private:
  M matcher_;

  GTEST_DISALLOW_ASSIGN_(MatcherAsPredicate);
};

// For implementing ASSERT_THAT() and EXPECT_THAT().  The template
// argument M must be a type that can be converted to a matcher.
template <typename M>
class PredicateFormatterFromMatcher {
 public:
  explicit PredicateFormatterFromMatcher(const M& m) : matcher_(m) {}

  // This template () operator allows a PredicateFormatterFromMatcher
  // object to act as a predicate-formatter suitable for using with
  // Google Test's EXPECT_PRED_FORMAT1() macro.
  template <typename T>
  AssertionResult operator()(const char* value_text, const T& x) const {
    // We convert matcher_ to a Matcher<const T&> *now* instead of
    // when the PredicateFormatterFromMatcher object was constructed,
    // as matcher_ may be polymorphic (e.g. NotNull()) and we won't
    // know which type to instantiate it to until we actually see the
    // type of x here.
    //
    // We write MatcherCast<const T&>(matcher_) instead of
    // Matcher<const T&>(matcher_), as the latter won't compile when
    // matcher_ has type Matcher<T> (e.g. An<int>()).
    const Matcher<const T&> matcher = MatcherCast<const T&>(matcher_);
    if (matcher.Matches(x)) {
      return AssertionSuccess();
    } else {
      ::std::stringstream ss;
      ss << "Value of: " << value_text << "\n"
         << "Expected: ";
      matcher.DescribeTo(&ss);
      ss << "\n  Actual: ";
      UniversalPrinter<T>::Print(x, &ss);
      ExplainMatchResultAsNeededTo<const T&>(matcher, x, &ss);
      return AssertionFailure(Message() << ss.str());
    }
  }

 private:
  const M matcher_;

  GTEST_DISALLOW_ASSIGN_(PredicateFormatterFromMatcher);
};

// A helper function for converting a matcher to a predicate-formatter
// without the user needing to explicitly write the type.  This is
// used for implementing ASSERT_THAT() and EXPECT_THAT().
template <typename M>
inline PredicateFormatterFromMatcher<M>
MakePredicateFormatterFromMatcher(const M& matcher) {
  return PredicateFormatterFromMatcher<M>(matcher);
}

// Implements the polymorphic floating point equality matcher, which
// matches two float values using ULP-based approximation.  The
// template is meant to be instantiated with FloatType being either
// float or double.
template <typename FloatType>
class FloatingEqMatcher {
 public:
  // Constructor for FloatingEqMatcher.
  // The matcher's input will be compared with rhs.  The matcher treats two
  // NANs as equal if nan_eq_nan is true.  Otherwise, under IEEE standards,
  // equality comparisons between NANs will always return false.
  FloatingEqMatcher(FloatType rhs, bool nan_eq_nan) :
    rhs_(rhs), nan_eq_nan_(nan_eq_nan) {}

  // Implements floating point equality matcher as a Matcher<T>.
  template <typename T>
  class Impl : public MatcherInterface<T> {
   public:
    Impl(FloatType rhs, bool nan_eq_nan) :
      rhs_(rhs), nan_eq_nan_(nan_eq_nan) {}

    virtual bool Matches(T value) const {
      const FloatingPoint<FloatType> lhs(value), rhs(rhs_);

      // Compares NaNs first, if nan_eq_nan_ is true.
      if (nan_eq_nan_ && lhs.is_nan()) {
        return rhs.is_nan();
      }

      return lhs.AlmostEquals(rhs);
    }

    virtual void DescribeTo(::std::ostream* os) const {
      // os->precision() returns the previously set precision, which we
      // store to restore the ostream to its original configuration
      // after outputting.
      const ::std::streamsize old_precision = os->precision(
          ::std::numeric_limits<FloatType>::digits10 + 2);
      if (FloatingPoint<FloatType>(rhs_).is_nan()) {
        if (nan_eq_nan_) {
          *os << "is NaN";
        } else {
          *os << "never matches";
        }
      } else {
        *os << "is approximately " << rhs_;
      }
      os->precision(old_precision);
    }

    virtual void DescribeNegationTo(::std::ostream* os) const {
      // As before, get original precision.
      const ::std::streamsize old_precision = os->precision(
          ::std::numeric_limits<FloatType>::digits10 + 2);
      if (FloatingPoint<FloatType>(rhs_).is_nan()) {
        if (nan_eq_nan_) {
          *os << "is not NaN";
        } else {
          *os << "is anything";
        }
      } else {
        *os << "is not approximately " << rhs_;
      }
      // Restore original precision.
      os->precision(old_precision);
    }

   private:
    const FloatType rhs_;
    const bool nan_eq_nan_;

    GTEST_DISALLOW_ASSIGN_(Impl);
  };

  // The following 3 type conversion operators allow FloatEq(rhs) and
  // NanSensitiveFloatEq(rhs) to be used as a Matcher<float>, a
  // Matcher<const float&>, or a Matcher<float&>, but nothing else.
  // (While Google's C++ coding style doesn't allow arguments passed
  // by non-const reference, we may see them in code not conforming to
  // the style.  Therefore Google Mock needs to support them.)
  operator Matcher<FloatType>() const {
    return MakeMatcher(new Impl<FloatType>(rhs_, nan_eq_nan_));
  }

  operator Matcher<const FloatType&>() const {
    return MakeMatcher(new Impl<const FloatType&>(rhs_, nan_eq_nan_));
  }

  operator Matcher<FloatType&>() const {
    return MakeMatcher(new Impl<FloatType&>(rhs_, nan_eq_nan_));
  }
 private:
  const FloatType rhs_;
  const bool nan_eq_nan_;

  GTEST_DISALLOW_ASSIGN_(FloatingEqMatcher);
};

// Implements the Pointee(m) matcher for matching a pointer whose
// pointee matches matcher m.  The pointer can be either raw or smart.
template <typename InnerMatcher>
class PointeeMatcher {
 public:
  explicit PointeeMatcher(const InnerMatcher& matcher) : matcher_(matcher) {}

  // This type conversion operator template allows Pointee(m) to be
  // used as a matcher for any pointer type whose pointee type is
  // compatible with the inner matcher, where type Pointer can be
  // either a raw pointer or a smart pointer.
  //
  // The reason we do this instead of relying on
  // MakePolymorphicMatcher() is that the latter is not flexible
  // enough for implementing the DescribeTo() method of Pointee().
  template <typename Pointer>
  operator Matcher<Pointer>() const {
    return MakeMatcher(new Impl<Pointer>(matcher_));
  }

 private:
  // The monomorphic implementation that works for a particular pointer type.
  template <typename Pointer>
  class Impl : public MatcherInterface<Pointer> {
   public:
    typedef typename PointeeOf<GMOCK_REMOVE_CONST_(  // NOLINT
        GMOCK_REMOVE_REFERENCE_(Pointer))>::type Pointee;

    explicit Impl(const InnerMatcher& matcher)
        : matcher_(MatcherCast<const Pointee&>(matcher)) {}

    virtual bool Matches(Pointer p) const {
      return GetRawPointer(p) != NULL && matcher_.Matches(*p);
    }

    virtual void DescribeTo(::std::ostream* os) const {
      *os << "points to a value that ";
      matcher_.DescribeTo(os);
    }

    virtual void DescribeNegationTo(::std::ostream* os) const {
      *os << "does not point to a value that ";
      matcher_.DescribeTo(os);
    }

    virtual void ExplainMatchResultTo(Pointer pointer,
                                      ::std::ostream* os) const {
      if (GetRawPointer(pointer) == NULL)
        return;

      ::std::stringstream ss;
      matcher_.ExplainMatchResultTo(*pointer, &ss);
      const internal::string s = ss.str();
      if (s != "") {
        *os << "points to a value that " << s;
      }
    }

   private:
    const Matcher<const Pointee&> matcher_;

    GTEST_DISALLOW_ASSIGN_(Impl);
  };

  const InnerMatcher matcher_;

  GTEST_DISALLOW_ASSIGN_(PointeeMatcher);
};

// Implements the Field() matcher for matching a field (i.e. member
// variable) of an object.
template <typename Class, typename FieldType>
class FieldMatcher {
 public:
  FieldMatcher(FieldType Class::*field,
               const Matcher<const FieldType&>& matcher)
      : field_(field), matcher_(matcher) {}

  // Returns true iff the inner matcher matches obj.field.
  bool Matches(const Class& obj) const {
    return matcher_.Matches(obj.*field_);
  }

  // Returns true iff the inner matcher matches obj->field.
  bool Matches(const Class* p) const {
    return (p != NULL) && matcher_.Matches(p->*field_);
  }

  void DescribeTo(::std::ostream* os) const {
    *os << "the given field ";
    matcher_.DescribeTo(os);
  }

  void DescribeNegationTo(::std::ostream* os) const {
    *os << "the given field ";
    matcher_.DescribeNegationTo(os);
  }

  // The first argument of ExplainMatchResultTo() is needed to help
  // Symbian's C++ compiler choose which overload to use.  Its type is
  // true_type iff the Field() matcher is used to match a pointer.
  void ExplainMatchResultTo(false_type /* is_not_pointer */, const Class& obj,
                            ::std::ostream* os) const {
    ::std::stringstream ss;
    matcher_.ExplainMatchResultTo(obj.*field_, &ss);
    const internal::string s = ss.str();
    if (s != "") {
      *os << "the given field " << s;
    }
  }

  void ExplainMatchResultTo(true_type /* is_pointer */, const Class* p,
                            ::std::ostream* os) const {
    if (p != NULL) {
      // Since *p has a field, it must be a class/struct/union type
      // and thus cannot be a pointer.  Therefore we pass false_type()
      // as the first argument.
      ExplainMatchResultTo(false_type(), *p, os);
    }
  }

 private:
  const FieldType Class::*field_;
  const Matcher<const FieldType&> matcher_;

  GTEST_DISALLOW_ASSIGN_(FieldMatcher);
};

// Explains the result of matching an object or pointer against a field matcher.
template <typename Class, typename FieldType, typename T>
void ExplainMatchResultTo(const FieldMatcher<Class, FieldType>& matcher,
                          const T& value, ::std::ostream* os) {
  matcher.ExplainMatchResultTo(
      typename ::testing::internal::is_pointer<T>::type(), value, os);
}

// Implements the Property() matcher for matching a property
// (i.e. return value of a getter method) of an object.
template <typename Class, typename PropertyType>
class PropertyMatcher {
 public:
  // The property may have a reference type, so 'const PropertyType&'
  // may cause double references and fail to compile.  That's why we
  // need GMOCK_REFERENCE_TO_CONST, which works regardless of
  // PropertyType being a reference or not.
  typedef GMOCK_REFERENCE_TO_CONST_(PropertyType) RefToConstProperty;

  PropertyMatcher(PropertyType (Class::*property)() const,
                  const Matcher<RefToConstProperty>& matcher)
      : property_(property), matcher_(matcher) {}

  // Returns true iff obj.property() matches the inner matcher.
  bool Matches(const Class& obj) const {
    return matcher_.Matches((obj.*property_)());
  }

  // Returns true iff p->property() matches the inner matcher.
  bool Matches(const Class* p) const {
    return (p != NULL) && matcher_.Matches((p->*property_)());
  }

  void DescribeTo(::std::ostream* os) const {
    *os << "the given property ";
    matcher_.DescribeTo(os);
  }

  void DescribeNegationTo(::std::ostream* os) const {
    *os << "the given property ";
    matcher_.DescribeNegationTo(os);
  }

  // The first argument of ExplainMatchResultTo() is needed to help
  // Symbian's C++ compiler choose which overload to use.  Its type is
  // true_type iff the Property() matcher is used to match a pointer.
  void ExplainMatchResultTo(false_type /* is_not_pointer */, const Class& obj,
                            ::std::ostream* os) const {
    ::std::stringstream ss;
    matcher_.ExplainMatchResultTo((obj.*property_)(), &ss);
    const internal::string s = ss.str();
    if (s != "") {
      *os << "the given property " << s;
    }
  }

  void ExplainMatchResultTo(true_type /* is_pointer */, const Class* p,
                            ::std::ostream* os) const {
    if (p != NULL) {
      // Since *p has a property method, it must be a
      // class/struct/union type and thus cannot be a pointer.
      // Therefore we pass false_type() as the first argument.
      ExplainMatchResultTo(false_type(), *p, os);
    }
  }

 private:
  PropertyType (Class::*property_)() const;
  const Matcher<RefToConstProperty> matcher_;

  GTEST_DISALLOW_ASSIGN_(PropertyMatcher);
};

// Explains the result of matching an object or pointer against a
// property matcher.
template <typename Class, typename PropertyType, typename T>
void ExplainMatchResultTo(const PropertyMatcher<Class, PropertyType>& matcher,
                          const T& value, ::std::ostream* os) {
  matcher.ExplainMatchResultTo(
      typename ::testing::internal::is_pointer<T>::type(), value, os);
}

// Type traits specifying various features of different functors for ResultOf.
// The default template specifies features for functor objects.
// Functor classes have to typedef argument_type and result_type
// to be compatible with ResultOf.
template <typename Functor>
struct CallableTraits {
  typedef typename Functor::result_type ResultType;
  typedef Functor StorageType;

  static void CheckIsValid(Functor /* functor */) {}
  template <typename T>
  static ResultType Invoke(Functor f, T arg) { return f(arg); }
};

// Specialization for function pointers.
template <typename ArgType, typename ResType>
struct CallableTraits<ResType(*)(ArgType)> {
  typedef ResType ResultType;
  typedef ResType(*StorageType)(ArgType);

  static void CheckIsValid(ResType(*f)(ArgType)) {
    GTEST_CHECK_(f != NULL)
        << "NULL function pointer is passed into ResultOf().";
  }
  template <typename T>
  static ResType Invoke(ResType(*f)(ArgType), T arg) {
    return (*f)(arg);
  }
};

// Implements the ResultOf() matcher for matching a return value of a
// unary function of an object.
template <typename Callable>
class ResultOfMatcher {
 public:
  typedef typename CallableTraits<Callable>::ResultType ResultType;

  ResultOfMatcher(Callable callable, const Matcher<ResultType>& matcher)
      : callable_(callable), matcher_(matcher) {
    CallableTraits<Callable>::CheckIsValid(callable_);
  }

  template <typename T>
  operator Matcher<T>() const {
    return Matcher<T>(new Impl<T>(callable_, matcher_));
  }

 private:
  typedef typename CallableTraits<Callable>::StorageType CallableStorageType;

  template <typename T>
  class Impl : public MatcherInterface<T> {
   public:
    Impl(CallableStorageType callable, const Matcher<ResultType>& matcher)
        : callable_(callable), matcher_(matcher) {}
    // Returns true iff callable_(obj) matches the inner matcher.
    // The calling syntax is different for different types of callables
    // so we abstract it in CallableTraits<Callable>::Invoke().
    virtual bool Matches(T obj) const {
      return matcher_.Matches(
          CallableTraits<Callable>::template Invoke<T>(callable_, obj));
    }

    virtual void DescribeTo(::std::ostream* os) const {
      *os << "result of the given callable ";
      matcher_.DescribeTo(os);
    }

    virtual void DescribeNegationTo(::std::ostream* os) const {
      *os << "result of the given callable ";
      matcher_.DescribeNegationTo(os);
    }

    virtual void ExplainMatchResultTo(T obj, ::std::ostream* os) const {
      ::std::stringstream ss;
      matcher_.ExplainMatchResultTo(
          CallableTraits<Callable>::template Invoke<T>(callable_, obj),
          &ss);
      const internal::string s = ss.str();
      if (s != "")
        *os << "result of the given callable " << s;
    }

   private:
    // Functors often define operator() as non-const method even though
    // they are actualy stateless. But we need to use them even when
    // 'this' is a const pointer. It's the user's responsibility not to
    // use stateful callables with ResultOf(), which does't guarantee
    // how many times the callable will be invoked.
    mutable CallableStorageType callable_;
    const Matcher<ResultType> matcher_;

    GTEST_DISALLOW_ASSIGN_(Impl);
  };  // class Impl

  const CallableStorageType callable_;
  const Matcher<ResultType> matcher_;

  GTEST_DISALLOW_ASSIGN_(ResultOfMatcher);
};

// Explains the result of matching a value against a functor matcher.
template <typename T, typename Callable>
void ExplainMatchResultTo(const ResultOfMatcher<Callable>& matcher,
                          T obj, ::std::ostream* os) {
  matcher.ExplainMatchResultTo(obj, os);
}

// Implements an equality matcher for any STL-style container whose elements
// support ==. This matcher is like Eq(), but its failure explanations provide
// more detailed information that is useful when the container is used as a set.
// The failure message reports elements that are in one of the operands but not
// the other. The failure messages do not report duplicate or out-of-order
// elements in the containers (which don't properly matter to sets, but can
// occur if the containers are vectors or lists, for example).
//
// Uses the container's const_iterator, value_type, operator ==,
// begin(), and end().
template <typename Container>
class ContainerEqMatcher {
 public:
  typedef internal::StlContainerView<Container> View;
  typedef typename View::type StlContainer;
  typedef typename View::const_reference StlContainerReference;

  // We make a copy of rhs in case the elements in it are modified
  // after this matcher is created.
  explicit ContainerEqMatcher(const Container& rhs) : rhs_(View::Copy(rhs)) {
    // Makes sure the user doesn't instantiate this class template
    // with a const or reference type.
    testing::StaticAssertTypeEq<Container,
        GMOCK_REMOVE_CONST_(GMOCK_REMOVE_REFERENCE_(Container))>();
  }

  template <typename LhsContainer>
  bool Matches(const LhsContainer& lhs) const {
    // GMOCK_REMOVE_CONST_() is needed to work around an MSVC 8.0 bug
    // that causes LhsContainer to be a const type sometimes.
    typedef internal::StlContainerView<GMOCK_REMOVE_CONST_(LhsContainer)>
        LhsView;
    StlContainerReference lhs_stl_container = LhsView::ConstReference(lhs);
    return lhs_stl_container == rhs_;
  }
  void DescribeTo(::std::ostream* os) const {
    *os << "equals ";
    UniversalPrinter<StlContainer>::Print(rhs_, os);
  }
  void DescribeNegationTo(::std::ostream* os) const {
    *os << "does not equal ";
    UniversalPrinter<StlContainer>::Print(rhs_, os);
  }

  template <typename LhsContainer>
  void ExplainMatchResultTo(const LhsContainer& lhs,
                            ::std::ostream* os) const {
    // GMOCK_REMOVE_CONST_() is needed to work around an MSVC 8.0 bug
    // that causes LhsContainer to be a const type sometimes.
    typedef internal::StlContainerView<GMOCK_REMOVE_CONST_(LhsContainer)>
        LhsView;
    typedef typename LhsView::type LhsStlContainer;
    StlContainerReference lhs_stl_container = LhsView::ConstReference(lhs);

    // Something is different. Check for missing values first.
    bool printed_header = false;
    for (typename LhsStlContainer::const_iterator it =
             lhs_stl_container.begin();
         it != lhs_stl_container.end(); ++it) {
      if (internal::ArrayAwareFind(rhs_.begin(), rhs_.end(), *it) ==
          rhs_.end()) {
        if (printed_header) {
          *os << ", ";
        } else {
          *os << "Only in actual: ";
          printed_header = true;
        }
        UniversalPrinter<typename LhsStlContainer::value_type>::Print(*it, os);
      }
    }

    // Now check for extra values.
    bool printed_header2 = false;
    for (typename StlContainer::const_iterator it = rhs_.begin();
         it != rhs_.end(); ++it) {
      if (internal::ArrayAwareFind(
              lhs_stl_container.begin(), lhs_stl_container.end(), *it) ==
          lhs_stl_container.end()) {
        if (printed_header2) {
          *os << ", ";
        } else {
          *os << (printed_header ? "; not" : "Not") << " in actual: ";
          printed_header2 = true;
        }
        UniversalPrinter<typename StlContainer::value_type>::Print(*it, os);
      }
    }
  }

 private:
  const StlContainer rhs_;

  GTEST_DISALLOW_ASSIGN_(ContainerEqMatcher);
};

template <typename LhsContainer, typename Container>
void ExplainMatchResultTo(const ContainerEqMatcher<Container>& matcher,
                          const LhsContainer& lhs,
                          ::std::ostream* os) {
  matcher.ExplainMatchResultTo(lhs, os);
}

// Implements Contains(element_matcher) for the given argument type Container.
template <typename Container>
class ContainsMatcherImpl : public MatcherInterface<Container> {
 public:
  typedef GMOCK_REMOVE_CONST_(GMOCK_REMOVE_REFERENCE_(Container)) RawContainer;
  typedef StlContainerView<RawContainer> View;
  typedef typename View::type StlContainer;
  typedef typename View::const_reference StlContainerReference;
  typedef typename StlContainer::value_type Element;

  template <typename InnerMatcher>
  explicit ContainsMatcherImpl(InnerMatcher inner_matcher)
      : inner_matcher_(
          testing::SafeMatcherCast<const Element&>(inner_matcher)) {}

  // Returns true iff 'container' matches.
  virtual bool Matches(Container container) const {
    StlContainerReference stl_container = View::ConstReference(container);
    for (typename StlContainer::const_iterator it = stl_container.begin();
         it != stl_container.end(); ++it) {
      if (inner_matcher_.Matches(*it))
        return true;
    }
    return false;
  }

  // Describes what this matcher does.
  virtual void DescribeTo(::std::ostream* os) const {
    *os << "contains at least one element that ";
    inner_matcher_.DescribeTo(os);
  }

  // Describes what the negation of this matcher does.
  virtual void DescribeNegationTo(::std::ostream* os) const {
    *os << "doesn't contain any element that ";
    inner_matcher_.DescribeTo(os);
  }

  // Explains why 'container' matches, or doesn't match, this matcher.
  virtual void ExplainMatchResultTo(Container container,
                                    ::std::ostream* os) const {
    StlContainerReference stl_container = View::ConstReference(container);

    // We need to explain which (if any) element matches inner_matcher_.
    typename StlContainer::const_iterator it = stl_container.begin();
    for (size_t i = 0; it != stl_container.end(); ++it, ++i) {
      if (inner_matcher_.Matches(*it)) {
        *os << "element " << i << " matches";
        return;
      }
    }
  }

 private:
  const Matcher<const Element&> inner_matcher_;

  GTEST_DISALLOW_ASSIGN_(ContainsMatcherImpl);
};

// Implements polymorphic Contains(element_matcher).
template <typename M>
class ContainsMatcher {
 public:
  explicit ContainsMatcher(M m) : inner_matcher_(m) {}

  template <typename Container>
  operator Matcher<Container>() const {
    return MakeMatcher(new ContainsMatcherImpl<Container>(inner_matcher_));
  }

 private:
  const M inner_matcher_;

  GTEST_DISALLOW_ASSIGN_(ContainsMatcher);
};

// Implements Key(inner_matcher) for the given argument pair type.
// Key(inner_matcher) matches an std::pair whose 'first' field matches
// inner_matcher.  For example, Contains(Key(Ge(5))) can be used to match an
// std::map that contains at least one element whose key is >= 5.
template <typename PairType>
class KeyMatcherImpl : public MatcherInterface<PairType> {
 public:
  typedef GMOCK_REMOVE_CONST_(GMOCK_REMOVE_REFERENCE_(PairType)) RawPairType;
  typedef typename RawPairType::first_type KeyType;

  template <typename InnerMatcher>
  explicit KeyMatcherImpl(InnerMatcher inner_matcher)
      : inner_matcher_(
          testing::SafeMatcherCast<const KeyType&>(inner_matcher)) {
  }

  // Returns true iff 'key_value.first' (the key) matches the inner matcher.
  virtual bool Matches(PairType key_value) const {
    return inner_matcher_.Matches(key_value.first);
  }

  // Describes what this matcher does.
  virtual void DescribeTo(::std::ostream* os) const {
    *os << "has a key that ";
    inner_matcher_.DescribeTo(os);
  }

  // Describes what the negation of this matcher does.
  virtual void DescribeNegationTo(::std::ostream* os) const {
    *os << "doesn't have a key that ";
    inner_matcher_.DescribeTo(os);
  }

  // Explains why 'key_value' matches, or doesn't match, this matcher.
  virtual void ExplainMatchResultTo(PairType key_value,
                                    ::std::ostream* os) const {
    inner_matcher_.ExplainMatchResultTo(key_value.first, os);
  }

 private:
  const Matcher<const KeyType&> inner_matcher_;

  GTEST_DISALLOW_ASSIGN_(KeyMatcherImpl);
};

// Implements polymorphic Key(matcher_for_key).
template <typename M>
class KeyMatcher {
 public:
  explicit KeyMatcher(M m) : matcher_for_key_(m) {}

  template <typename PairType>
  operator Matcher<PairType>() const {
    return MakeMatcher(new KeyMatcherImpl<PairType>(matcher_for_key_));
  }

 private:
  const M matcher_for_key_;

  GTEST_DISALLOW_ASSIGN_(KeyMatcher);
};

// Implements Pair(first_matcher, second_matcher) for the given argument pair
// type with its two matchers. See Pair() function below.
template <typename PairType>
class PairMatcherImpl : public MatcherInterface<PairType> {
 public:
  typedef GMOCK_REMOVE_CONST_(GMOCK_REMOVE_REFERENCE_(PairType)) RawPairType;
  typedef typename RawPairType::first_type FirstType;
  typedef typename RawPairType::second_type SecondType;

  template <typename FirstMatcher, typename SecondMatcher>
  PairMatcherImpl(FirstMatcher first_matcher, SecondMatcher second_matcher)
      : first_matcher_(
            testing::SafeMatcherCast<const FirstType&>(first_matcher)),
        second_matcher_(
            testing::SafeMatcherCast<const SecondType&>(second_matcher)) {
  }

  // Returns true iff 'a_pair.first' matches first_matcher and 'a_pair.second'
  // matches second_matcher.
  virtual bool Matches(PairType a_pair) const {
    return first_matcher_.Matches(a_pair.first) &&
           second_matcher_.Matches(a_pair.second);
  }

  // Describes what this matcher does.
  virtual void DescribeTo(::std::ostream* os) const {
    *os << "has a first field that ";
    first_matcher_.DescribeTo(os);
    *os << ", and has a second field that ";
    second_matcher_.DescribeTo(os);
  }

  // Describes what the negation of this matcher does.
  virtual void DescribeNegationTo(::std::ostream* os) const {
    *os << "has a first field that ";
    first_matcher_.DescribeNegationTo(os);
    *os << ", or has a second field that ";
    second_matcher_.DescribeNegationTo(os);
  }

  // Explains why 'a_pair' matches, or doesn't match, this matcher.
  virtual void ExplainMatchResultTo(PairType a_pair,
                                    ::std::ostream* os) const {
    ::std::stringstream ss1;
    first_matcher_.ExplainMatchResultTo(a_pair.first, &ss1);
    internal::string s1 = ss1.str();
    if (s1 != "") {
       s1 = "the first field " + s1;
    }

    ::std::stringstream ss2;
    second_matcher_.ExplainMatchResultTo(a_pair.second, &ss2);
    internal::string s2 = ss2.str();
    if (s2 != "") {
       s2 = "the second field " + s2;
    }

    *os << s1;
    if (s1 != "" && s2 != "") {
       *os << ", and ";
    }
    *os << s2;
  }

 private:
  const Matcher<const FirstType&> first_matcher_;
  const Matcher<const SecondType&> second_matcher_;

  GTEST_DISALLOW_ASSIGN_(PairMatcherImpl);
};

// Implements polymorphic Pair(first_matcher, second_matcher).
template <typename FirstMatcher, typename SecondMatcher>
class PairMatcher {
 public:
  PairMatcher(FirstMatcher first_matcher, SecondMatcher second_matcher)
      : first_matcher_(first_matcher), second_matcher_(second_matcher) {}

  template <typename PairType>
  operator Matcher<PairType> () const {
    return MakeMatcher(
        new PairMatcherImpl<PairType>(
            first_matcher_, second_matcher_));
  }

 private:
  const FirstMatcher first_matcher_;
  const SecondMatcher second_matcher_;

  GTEST_DISALLOW_ASSIGN_(PairMatcher);
};

// Implements ElementsAre() and ElementsAreArray().
template <typename Container>
class ElementsAreMatcherImpl : public MatcherInterface<Container> {
 public:
  typedef GMOCK_REMOVE_CONST_(GMOCK_REMOVE_REFERENCE_(Container)) RawContainer;
  typedef internal::StlContainerView<RawContainer> View;
  typedef typename View::type StlContainer;
  typedef typename View::const_reference StlContainerReference;
  typedef typename StlContainer::value_type Element;

  // Constructs the matcher from a sequence of element values or
  // element matchers.
  template <typename InputIter>
  ElementsAreMatcherImpl(InputIter first, size_t a_count) {
    matchers_.reserve(a_count);
    InputIter it = first;
    for (size_t i = 0; i != a_count; ++i, ++it) {
      matchers_.push_back(MatcherCast<const Element&>(*it));
    }
  }

  // Returns true iff 'container' matches.
  virtual bool Matches(Container container) const {
    StlContainerReference stl_container = View::ConstReference(container);
    if (stl_container.size() != count())
      return false;

    typename StlContainer::const_iterator it = stl_container.begin();
    for (size_t i = 0; i != count();  ++it, ++i) {
      if (!matchers_[i].Matches(*it))
        return false;
    }

    return true;
  }

  // Describes what this matcher does.
  virtual void DescribeTo(::std::ostream* os) const {
    if (count() == 0) {
      *os << "is empty";
    } else if (count() == 1) {
      *os << "has 1 element that ";
      matchers_[0].DescribeTo(os);
    } else {
      *os << "has " << Elements(count()) << " where\n";
      for (size_t i = 0; i != count(); ++i) {
        *os << "element " << i << " ";
        matchers_[i].DescribeTo(os);
        if (i + 1 < count()) {
          *os << ",\n";
        }
      }
    }
  }

  // Describes what the negation of this matcher does.
  virtual void DescribeNegationTo(::std::ostream* os) const {
    if (count() == 0) {
      *os << "is not empty";
      return;
    }

    *os << "does not have " << Elements(count()) << ", or\n";
    for (size_t i = 0; i != count(); ++i) {
      *os << "element " << i << " ";
      matchers_[i].DescribeNegationTo(os);
      if (i + 1 < count()) {
        *os << ", or\n";
      }
    }
  }

  // Explains why 'container' matches, or doesn't match, this matcher.
  virtual void ExplainMatchResultTo(Container container,
                                    ::std::ostream* os) const {
    StlContainerReference stl_container = View::ConstReference(container);
    if (Matches(container)) {
      // We need to explain why *each* element matches (the obvious
      // ones can be skipped).

      bool reason_printed = false;
      typename StlContainer::const_iterator it = stl_container.begin();
      for (size_t i = 0; i != count(); ++it, ++i) {
        ::std::stringstream ss;
        matchers_[i].ExplainMatchResultTo(*it, &ss);

        const string s = ss.str();
        if (!s.empty()) {
          if (reason_printed) {
            *os << ",\n";
          }
          *os << "element " << i << " " << s;
          reason_printed = true;
        }
      }
    } else {
      // We need to explain why the container doesn't match.
      const size_t actual_count = stl_container.size();
      if (actual_count != count()) {
        // The element count doesn't match.  If the container is
        // empty, there's no need to explain anything as Google Mock
        // already prints the empty container.  Otherwise we just need
        // to show how many elements there actually are.
        if (actual_count != 0) {
          *os << "has " << Elements(actual_count);
        }
        return;
      }

      // The container has the right size but at least one element
      // doesn't match expectation.  We need to find this element and
      // explain why it doesn't match.
      typename StlContainer::const_iterator it = stl_container.begin();
      for (size_t i = 0; i != count(); ++it, ++i) {
        if (matchers_[i].Matches(*it)) {
          continue;
        }

        *os << "element " << i << " doesn't match";

        ::std::stringstream ss;
        matchers_[i].ExplainMatchResultTo(*it, &ss);
        const string s = ss.str();
        if (!s.empty()) {
          *os << " (" << s << ")";
        }
        return;
      }
    }
  }

 private:
  static Message Elements(size_t count) {
    return Message() << count << (count == 1 ? " element" : " elements");
  }

  size_t count() const { return matchers_.size(); }
  std::vector<Matcher<const Element&> > matchers_;

  GTEST_DISALLOW_ASSIGN_(ElementsAreMatcherImpl);
};

// Implements ElementsAre() of 0 arguments.
class ElementsAreMatcher0 {
 public:
  ElementsAreMatcher0() {}

  template <typename Container>
  operator Matcher<Container>() const {
    typedef GMOCK_REMOVE_CONST_(GMOCK_REMOVE_REFERENCE_(Container))
        RawContainer;
    typedef typename internal::StlContainerView<RawContainer>::type::value_type
        Element;

    const Matcher<const Element&>* const matchers = NULL;
    return MakeMatcher(new ElementsAreMatcherImpl<Container>(matchers, 0));
  }
};

// Implements ElementsAreArray().
template <typename T>
class ElementsAreArrayMatcher {
 public:
  ElementsAreArrayMatcher(const T* first, size_t count) :
      first_(first), count_(count) {}

  template <typename Container>
  operator Matcher<Container>() const {
    typedef GMOCK_REMOVE_CONST_(GMOCK_REMOVE_REFERENCE_(Container))
        RawContainer;
    typedef typename internal::StlContainerView<RawContainer>::type::value_type
        Element;

    return MakeMatcher(new ElementsAreMatcherImpl<Container>(first_, count_));
  }

 private:
  const T* const first_;
  const size_t count_;

  GTEST_DISALLOW_ASSIGN_(ElementsAreArrayMatcher);
};

// Constants denoting interpolations in a matcher description string.
const int kTupleInterpolation = -1;    // "%(*)s"
const int kPercentInterpolation = -2;  // "%%"
const int kInvalidInterpolation = -3;  // "%" followed by invalid text

// Records the location and content of an interpolation.
struct Interpolation {
  Interpolation(const char* start, const char* end, int param)
      : start_pos(start), end_pos(end), param_index(param) {}

  // Points to the start of the interpolation (the '%' character).
  const char* start_pos;
  // Points to the first character after the interpolation.
  const char* end_pos;
  // 0-based index of the interpolated matcher parameter;
  // kTupleInterpolation for "%(*)s"; kPercentInterpolation for "%%".
  int param_index;
};

typedef ::std::vector<Interpolation> Interpolations;

// Parses a matcher description string and returns a vector of
// interpolations that appear in the string; generates non-fatal
// failures iff 'description' is an invalid matcher description.
// 'param_names' is a NULL-terminated array of parameter names in the
// order they appear in the MATCHER_P*() parameter list.
Interpolations ValidateMatcherDescription(
    const char* param_names[], const char* description);

// Returns the actual matcher description, given the matcher name,
// user-supplied description template string, interpolations in the
// string, and the printed values of the matcher parameters.
string FormatMatcherDescription(
    const char* matcher_name, const char* description,
    const Interpolations& interp, const Strings& param_values);

}  // namespace internal

// Implements MatcherCast().
template <typename T, typename M>
inline Matcher<T> MatcherCast(M matcher) {
  return internal::MatcherCastImpl<T, M>::Cast(matcher);
}

// _ is a matcher that matches anything of any type.
//
// This definition is fine as:
//
//   1. The C++ standard permits using the name _ in a namespace that
//      is not the global namespace or ::std.
//   2. The AnythingMatcher class has no data member or constructor,
//      so it's OK to create global variables of this type.
//   3. c-style has approved of using _ in this case.
const internal::AnythingMatcher _ = {};
// Creates a matcher that matches any value of the given type T.
template <typename T>
inline Matcher<T> A() { return MakeMatcher(new internal::AnyMatcherImpl<T>()); }

// Creates a matcher that matches any value of the given type T.
template <typename T>
inline Matcher<T> An() { return A<T>(); }

// Creates a polymorphic matcher that matches anything equal to x.
// Note: if the parameter of Eq() were declared as const T&, Eq("foo")
// wouldn't compile.
template <typename T>
inline internal::EqMatcher<T> Eq(T x) { return internal::EqMatcher<T>(x); }

// Constructs a Matcher<T> from a 'value' of type T.  The constructed
// matcher matches any value that's equal to 'value'.
template <typename T>
Matcher<T>::Matcher(T value) { *this = Eq(value); }

// Creates a monomorphic matcher that matches anything with type Lhs
// and equal to rhs.  A user may need to use this instead of Eq(...)
// in order to resolve an overloading ambiguity.
//
// TypedEq<T>(x) is just a convenient short-hand for Matcher<T>(Eq(x))
// or Matcher<T>(x), but more readable than the latter.
//
// We could define similar monomorphic matchers for other comparison
// operations (e.g. TypedLt, TypedGe, and etc), but decided not to do
// it yet as those are used much less than Eq() in practice.  A user
// can always write Matcher<T>(Lt(5)) to be explicit about the type,
// for example.
template <typename Lhs, typename Rhs>
inline Matcher<Lhs> TypedEq(const Rhs& rhs) { return Eq(rhs); }

// Creates a polymorphic matcher that matches anything >= x.
template <typename Rhs>
inline internal::GeMatcher<Rhs> Ge(Rhs x) {
  return internal::GeMatcher<Rhs>(x);
}

// Creates a polymorphic matcher that matches anything > x.
template <typename Rhs>
inline internal::GtMatcher<Rhs> Gt(Rhs x) {
  return internal::GtMatcher<Rhs>(x);
}

// Creates a polymorphic matcher that matches anything <= x.
template <typename Rhs>
inline internal::LeMatcher<Rhs> Le(Rhs x) {
  return internal::LeMatcher<Rhs>(x);
}

// Creates a polymorphic matcher that matches anything < x.
template <typename Rhs>
inline internal::LtMatcher<Rhs> Lt(Rhs x) {
  return internal::LtMatcher<Rhs>(x);
}

// Creates a polymorphic matcher that matches anything != x.
template <typename Rhs>
inline internal::NeMatcher<Rhs> Ne(Rhs x) {
  return internal::NeMatcher<Rhs>(x);
}

// Creates a polymorphic matcher that matches any NULL pointer.
inline PolymorphicMatcher<internal::IsNullMatcher > IsNull() {
  return MakePolymorphicMatcher(internal::IsNullMatcher());
}

// Creates a polymorphic matcher that matches any non-NULL pointer.
// This is convenient as Not(NULL) doesn't compile (the compiler
// thinks that that expression is comparing a pointer with an integer).
inline PolymorphicMatcher<internal::NotNullMatcher > NotNull() {
  return MakePolymorphicMatcher(internal::NotNullMatcher());
}

// Creates a polymorphic matcher that matches any argument that
// references variable x.
template <typename T>
inline internal::RefMatcher<T&> Ref(T& x) {  // NOLINT
  return internal::RefMatcher<T&>(x);
}

// Creates a matcher that matches any double argument approximately
// equal to rhs, where two NANs are considered unequal.
inline internal::FloatingEqMatcher<double> DoubleEq(double rhs) {
  return internal::FloatingEqMatcher<double>(rhs, false);
}

// Creates a matcher that matches any double argument approximately
// equal to rhs, including NaN values when rhs is NaN.
inline internal::FloatingEqMatcher<double> NanSensitiveDoubleEq(double rhs) {
  return internal::FloatingEqMatcher<double>(rhs, true);
}

// Creates a matcher that matches any float argument approximately
// equal to rhs, where two NANs are considered unequal.
inline internal::FloatingEqMatcher<float> FloatEq(float rhs) {
  return internal::FloatingEqMatcher<float>(rhs, false);
}

// Creates a matcher that matches any double argument approximately
// equal to rhs, including NaN values when rhs is NaN.
inline internal::FloatingEqMatcher<float> NanSensitiveFloatEq(float rhs) {
  return internal::FloatingEqMatcher<float>(rhs, true);
}

// Creates a matcher that matches a pointer (raw or smart) that points
// to a value that matches inner_matcher.
template <typename InnerMatcher>
inline internal::PointeeMatcher<InnerMatcher> Pointee(
    const InnerMatcher& inner_matcher) {
  return internal::PointeeMatcher<InnerMatcher>(inner_matcher);
}

// Creates a matcher that matches an object whose given field matches
// 'matcher'.  For example,
//   Field(&Foo::number, Ge(5))
// matches a Foo object x iff x.number >= 5.
template <typename Class, typename FieldType, typename FieldMatcher>
inline PolymorphicMatcher<
  internal::FieldMatcher<Class, FieldType> > Field(
    FieldType Class::*field, const FieldMatcher& matcher) {
  return MakePolymorphicMatcher(
      internal::FieldMatcher<Class, FieldType>(
          field, MatcherCast<const FieldType&>(matcher)));
  // The call to MatcherCast() is required for supporting inner
  // matchers of compatible types.  For example, it allows
  //   Field(&Foo::bar, m)
  // to compile where bar is an int32 and m is a matcher for int64.
}

// Creates a matcher that matches an object whose given property
// matches 'matcher'.  For example,
//   Property(&Foo::str, StartsWith("hi"))
// matches a Foo object x iff x.str() starts with "hi".
template <typename Class, typename PropertyType, typename PropertyMatcher>
inline PolymorphicMatcher<
  internal::PropertyMatcher<Class, PropertyType> > Property(
    PropertyType (Class::*property)() const, const PropertyMatcher& matcher) {
  return MakePolymorphicMatcher(
      internal::PropertyMatcher<Class, PropertyType>(
          property,
          MatcherCast<GMOCK_REFERENCE_TO_CONST_(PropertyType)>(matcher)));
  // The call to MatcherCast() is required for supporting inner
  // matchers of compatible types.  For example, it allows
  //   Property(&Foo::bar, m)
  // to compile where bar() returns an int32 and m is a matcher for int64.
}

// Creates a matcher that matches an object iff the result of applying
// a callable to x matches 'matcher'.
// For example,
//   ResultOf(f, StartsWith("hi"))
// matches a Foo object x iff f(x) starts with "hi".
// callable parameter can be a function, function pointer, or a functor.
// Callable has to satisfy the following conditions:
//   * It is required to keep no state affecting the results of
//     the calls on it and make no assumptions about how many calls
//     will be made. Any state it keeps must be protected from the
//     concurrent access.
//   * If it is a function object, it has to define type result_type.
//     We recommend deriving your functor classes from std::unary_function.
template <typename Callable, typename ResultOfMatcher>
internal::ResultOfMatcher<Callable> ResultOf(
    Callable callable, const ResultOfMatcher& matcher) {
  return internal::ResultOfMatcher<Callable>(
          callable,
          MatcherCast<typename internal::CallableTraits<Callable>::ResultType>(
              matcher));
  // The call to MatcherCast() is required for supporting inner
  // matchers of compatible types.  For example, it allows
  //   ResultOf(Function, m)
  // to compile where Function() returns an int32 and m is a matcher for int64.
}

// String matchers.

// Matches a string equal to str.
inline PolymorphicMatcher<internal::StrEqualityMatcher<internal::string> >
    StrEq(const internal::string& str) {
  return MakePolymorphicMatcher(internal::StrEqualityMatcher<internal::string>(
      str, true, true));
}

// Matches a string not equal to str.
inline PolymorphicMatcher<internal::StrEqualityMatcher<internal::string> >
    StrNe(const internal::string& str) {
  return MakePolymorphicMatcher(internal::StrEqualityMatcher<internal::string>(
      str, false, true));
}

// Matches a string equal to str, ignoring case.
inline PolymorphicMatcher<internal::StrEqualityMatcher<internal::string> >
    StrCaseEq(const internal::string& str) {
  return MakePolymorphicMatcher(internal::StrEqualityMatcher<internal::string>(
      str, true, false));
}

// Matches a string not equal to str, ignoring case.
inline PolymorphicMatcher<internal::StrEqualityMatcher<internal::string> >
    StrCaseNe(const internal::string& str) {
  return MakePolymorphicMatcher(internal::StrEqualityMatcher<internal::string>(
      str, false, false));
}

// Creates a matcher that matches any string, std::string, or C string
// that contains the given substring.
inline PolymorphicMatcher<internal::HasSubstrMatcher<internal::string> >
    HasSubstr(const internal::string& substring) {
  return MakePolymorphicMatcher(internal::HasSubstrMatcher<internal::string>(
      substring));
}

// Matches a string that starts with 'prefix' (case-sensitive).
inline PolymorphicMatcher<internal::StartsWithMatcher<internal::string> >
    StartsWith(const internal::string& prefix) {
  return MakePolymorphicMatcher(internal::StartsWithMatcher<internal::string>(
      prefix));
}

// Matches a string that ends with 'suffix' (case-sensitive).
inline PolymorphicMatcher<internal::EndsWithMatcher<internal::string> >
    EndsWith(const internal::string& suffix) {
  return MakePolymorphicMatcher(internal::EndsWithMatcher<internal::string>(
      suffix));
}

#ifdef GMOCK_HAS_REGEX

// Matches a string that fully matches regular expression 'regex'.
// The matcher takes ownership of 'regex'.
inline PolymorphicMatcher<internal::MatchesRegexMatcher> MatchesRegex(
    const internal::RE* regex) {
  return MakePolymorphicMatcher(internal::MatchesRegexMatcher(regex, true));
}
inline PolymorphicMatcher<internal::MatchesRegexMatcher> MatchesRegex(
    const internal::string& regex) {
  return MatchesRegex(new internal::RE(regex));
}

// Matches a string that contains regular expression 'regex'.
// The matcher takes ownership of 'regex'.
inline PolymorphicMatcher<internal::MatchesRegexMatcher> ContainsRegex(
    const internal::RE* regex) {
  return MakePolymorphicMatcher(internal::MatchesRegexMatcher(regex, false));
}
inline PolymorphicMatcher<internal::MatchesRegexMatcher> ContainsRegex(
    const internal::string& regex) {
  return ContainsRegex(new internal::RE(regex));
}

#endif  // GMOCK_HAS_REGEX

#if GTEST_HAS_GLOBAL_WSTRING || GTEST_HAS_STD_WSTRING
// Wide string matchers.

// Matches a string equal to str.
inline PolymorphicMatcher<internal::StrEqualityMatcher<internal::wstring> >
    StrEq(const internal::wstring& str) {
  return MakePolymorphicMatcher(internal::StrEqualityMatcher<internal::wstring>(
      str, true, true));
}

// Matches a string not equal to str.
inline PolymorphicMatcher<internal::StrEqualityMatcher<internal::wstring> >
    StrNe(const internal::wstring& str) {
  return MakePolymorphicMatcher(internal::StrEqualityMatcher<internal::wstring>(
      str, false, true));
}

// Matches a string equal to str, ignoring case.
inline PolymorphicMatcher<internal::StrEqualityMatcher<internal::wstring> >
    StrCaseEq(const internal::wstring& str) {
  return MakePolymorphicMatcher(internal::StrEqualityMatcher<internal::wstring>(
      str, true, false));
}

// Matches a string not equal to str, ignoring case.
inline PolymorphicMatcher<internal::StrEqualityMatcher<internal::wstring> >
    StrCaseNe(const internal::wstring& str) {
  return MakePolymorphicMatcher(internal::StrEqualityMatcher<internal::wstring>(
      str, false, false));
}

// Creates a matcher that matches any wstring, std::wstring, or C wide string
// that contains the given substring.
inline PolymorphicMatcher<internal::HasSubstrMatcher<internal::wstring> >
    HasSubstr(const internal::wstring& substring) {
  return MakePolymorphicMatcher(internal::HasSubstrMatcher<internal::wstring>(
      substring));
}

// Matches a string that starts with 'prefix' (case-sensitive).
inline PolymorphicMatcher<internal::StartsWithMatcher<internal::wstring> >
    StartsWith(const internal::wstring& prefix) {
  return MakePolymorphicMatcher(internal::StartsWithMatcher<internal::wstring>(
      prefix));
}

// Matches a string that ends with 'suffix' (case-sensitive).
inline PolymorphicMatcher<internal::EndsWithMatcher<internal::wstring> >
    EndsWith(const internal::wstring& suffix) {
  return MakePolymorphicMatcher(internal::EndsWithMatcher<internal::wstring>(
      suffix));
}

#endif  // GTEST_HAS_GLOBAL_WSTRING || GTEST_HAS_STD_WSTRING

// Creates a polymorphic matcher that matches a 2-tuple where the
// first field == the second field.
inline internal::Eq2Matcher Eq() { return internal::Eq2Matcher(); }

// Creates a polymorphic matcher that matches a 2-tuple where the
// first field >= the second field.
inline internal::Ge2Matcher Ge() { return internal::Ge2Matcher(); }

// Creates a polymorphic matcher that matches a 2-tuple where the
// first field > the second field.
inline internal::Gt2Matcher Gt() { return internal::Gt2Matcher(); }

// Creates a polymorphic matcher that matches a 2-tuple where the
// first field <= the second field.
inline internal::Le2Matcher Le() { return internal::Le2Matcher(); }

// Creates a polymorphic matcher that matches a 2-tuple where the
// first field < the second field.
inline internal::Lt2Matcher Lt() { return internal::Lt2Matcher(); }

// Creates a polymorphic matcher that matches a 2-tuple where the
// first field != the second field.
inline internal::Ne2Matcher Ne() { return internal::Ne2Matcher(); }

// Creates a matcher that matches any value of type T that m doesn't
// match.
template <typename InnerMatcher>
inline internal::NotMatcher<InnerMatcher> Not(InnerMatcher m) {
  return internal::NotMatcher<InnerMatcher>(m);
}

// Creates a matcher that matches any value that matches all of the
// given matchers.
//
// For now we only support up to 5 matchers.  Support for more
// matchers can be added as needed, or the user can use nested
// AllOf()s.
template <typename Matcher1, typename Matcher2>
inline internal::BothOfMatcher<Matcher1, Matcher2>
AllOf(Matcher1 m1, Matcher2 m2) {
  return internal::BothOfMatcher<Matcher1, Matcher2>(m1, m2);
}

template <typename Matcher1, typename Matcher2, typename Matcher3>
inline internal::BothOfMatcher<Matcher1,
           internal::BothOfMatcher<Matcher2, Matcher3> >
AllOf(Matcher1 m1, Matcher2 m2, Matcher3 m3) {
  return AllOf(m1, AllOf(m2, m3));
}

template <typename Matcher1, typename Matcher2, typename Matcher3,
          typename Matcher4>
inline internal::BothOfMatcher<Matcher1,
           internal::BothOfMatcher<Matcher2,
               internal::BothOfMatcher<Matcher3, Matcher4> > >
AllOf(Matcher1 m1, Matcher2 m2, Matcher3 m3, Matcher4 m4) {
  return AllOf(m1, AllOf(m2, m3, m4));
}

template <typename Matcher1, typename Matcher2, typename Matcher3,
          typename Matcher4, typename Matcher5>
inline internal::BothOfMatcher<Matcher1,
           internal::BothOfMatcher<Matcher2,
               internal::BothOfMatcher<Matcher3,
                   internal::BothOfMatcher<Matcher4, Matcher5> > > >
AllOf(Matcher1 m1, Matcher2 m2, Matcher3 m3, Matcher4 m4, Matcher5 m5) {
  return AllOf(m1, AllOf(m2, m3, m4, m5));
}

// Creates a matcher that matches any value that matches at least one
// of the given matchers.
//
// For now we only support up to 5 matchers.  Support for more
// matchers can be added as needed, or the user can use nested
// AnyOf()s.
template <typename Matcher1, typename Matcher2>
inline internal::EitherOfMatcher<Matcher1, Matcher2>
AnyOf(Matcher1 m1, Matcher2 m2) {
  return internal::EitherOfMatcher<Matcher1, Matcher2>(m1, m2);
}

template <typename Matcher1, typename Matcher2, typename Matcher3>
inline internal::EitherOfMatcher<Matcher1,
           internal::EitherOfMatcher<Matcher2, Matcher3> >
AnyOf(Matcher1 m1, Matcher2 m2, Matcher3 m3) {
  return AnyOf(m1, AnyOf(m2, m3));
}

template <typename Matcher1, typename Matcher2, typename Matcher3,
          typename Matcher4>
inline internal::EitherOfMatcher<Matcher1,
           internal::EitherOfMatcher<Matcher2,
               internal::EitherOfMatcher<Matcher3, Matcher4> > >
AnyOf(Matcher1 m1, Matcher2 m2, Matcher3 m3, Matcher4 m4) {
  return AnyOf(m1, AnyOf(m2, m3, m4));
}

template <typename Matcher1, typename Matcher2, typename Matcher3,
          typename Matcher4, typename Matcher5>
inline internal::EitherOfMatcher<Matcher1,
           internal::EitherOfMatcher<Matcher2,
               internal::EitherOfMatcher<Matcher3,
                   internal::EitherOfMatcher<Matcher4, Matcher5> > > >
AnyOf(Matcher1 m1, Matcher2 m2, Matcher3 m3, Matcher4 m4, Matcher5 m5) {
  return AnyOf(m1, AnyOf(m2, m3, m4, m5));
}

// Returns a matcher that matches anything that satisfies the given
// predicate.  The predicate can be any unary function or functor
// whose return type can be implicitly converted to bool.
template <typename Predicate>
inline PolymorphicMatcher<internal::TrulyMatcher<Predicate> >
Truly(Predicate pred) {
  return MakePolymorphicMatcher(internal::TrulyMatcher<Predicate>(pred));
}

// Returns a matcher that matches an equal container.
// This matcher behaves like Eq(), but in the event of mismatch lists the
// values that are included in one container but not the other. (Duplicate
// values and order differences are not explained.)
template <typename Container>
inline PolymorphicMatcher<internal::ContainerEqMatcher<
                            GMOCK_REMOVE_CONST_(Container)> >
    ContainerEq(const Container& rhs) {
  // This following line is for working around a bug in MSVC 8.0,
  // which causes Container to be a const type sometimes.
  typedef GMOCK_REMOVE_CONST_(Container) RawContainer;
  return MakePolymorphicMatcher(internal::ContainerEqMatcher<RawContainer>(rhs));
}

// Matches an STL-style container or a native array that contains at
// least one element matching the given value or matcher.
//
// Examples:
//   ::std::set<int> page_ids;
//   page_ids.insert(3);
//   page_ids.insert(1);
//   EXPECT_THAT(page_ids, Contains(1));
//   EXPECT_THAT(page_ids, Contains(Gt(2)));
//   EXPECT_THAT(page_ids, Not(Contains(4)));
//
//   ::std::map<int, size_t> page_lengths;
//   page_lengths[1] = 100;
//   EXPECT_THAT(page_lengths,
//               Contains(::std::pair<const int, size_t>(1, 100)));
//
//   const char* user_ids[] = { "joe", "mike", "tom" };
//   EXPECT_THAT(user_ids, Contains(Eq(::std::string("tom"))));
template <typename M>
inline internal::ContainsMatcher<M> Contains(M matcher) {
  return internal::ContainsMatcher<M>(matcher);
}

// Key(inner_matcher) matches an std::pair whose 'first' field matches
// inner_matcher.  For example, Contains(Key(Ge(5))) can be used to match an
// std::map that contains at least one element whose key is >= 5.
template <typename M>
inline internal::KeyMatcher<M> Key(M inner_matcher) {
  return internal::KeyMatcher<M>(inner_matcher);
}

// Pair(first_matcher, second_matcher) matches a std::pair whose 'first' field
// matches first_matcher and whose 'second' field matches second_matcher.  For
// example, EXPECT_THAT(map_type, ElementsAre(Pair(Ge(5), "foo"))) can be used
// to match a std::map<int, string> that contains exactly one element whose key
// is >= 5 and whose value equals "foo".
template <typename FirstMatcher, typename SecondMatcher>
inline internal::PairMatcher<FirstMatcher, SecondMatcher>
Pair(FirstMatcher first_matcher, SecondMatcher second_matcher) {
  return internal::PairMatcher<FirstMatcher, SecondMatcher>(
      first_matcher, second_matcher);
}

// Returns a predicate that is satisfied by anything that matches the
// given matcher.
template <typename M>
inline internal::MatcherAsPredicate<M> Matches(M matcher) {
  return internal::MatcherAsPredicate<M>(matcher);
}

// Returns true iff the value matches the matcher.
template <typename T, typename M>
inline bool Value(const T& value, M matcher) {
  return testing::Matches(matcher)(value);
}

// AllArgs(m) is a synonym of m.  This is useful in
//
//   EXPECT_CALL(foo, Bar(_, _)).With(AllArgs(Eq()));
//
// which is easier to read than
//
//   EXPECT_CALL(foo, Bar(_, _)).With(Eq());
template <typename InnerMatcher>
inline InnerMatcher AllArgs(const InnerMatcher& matcher) { return matcher; }

// These macros allow using matchers to check values in Google Test
// tests.  ASSERT_THAT(value, matcher) and EXPECT_THAT(value, matcher)
// succeed iff the value matches the matcher.  If the assertion fails,
// the value and the description of the matcher will be printed.
#define ASSERT_THAT(value, matcher) ASSERT_PRED_FORMAT1(\
    ::testing::internal::MakePredicateFormatterFromMatcher(matcher), value)
#define EXPECT_THAT(value, matcher) EXPECT_PRED_FORMAT1(\
    ::testing::internal::MakePredicateFormatterFromMatcher(matcher), value)

}  // namespace testing

#endif  // GMOCK_INCLUDE_GMOCK_GMOCK_MATCHERS_H_
