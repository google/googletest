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
#include <utility>
#include <vector>

#include "gmock/internal/gmock-internal-utils.h"
#include "gmock/internal/gmock-port.h"
#include "gtest/gtest.h"

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

// MatchResultListener is an abstract class.  Its << operator can be
// used by a matcher to explain why a value matches or doesn't match.
//
// TODO(wan@google.com): add method
//   bool InterestedInWhy(bool result) const;
// to indicate whether the listener is interested in why the match
// result is 'result'.
class MatchResultListener {
 public:
  // Creates a listener object with the given underlying ostream.  The
  // listener does not own the ostream.
  explicit MatchResultListener(::std::ostream* os) : stream_(os) {}
  virtual ~MatchResultListener() = 0;  // Makes this class abstract.

  // Streams x to the underlying ostream; does nothing if the ostream
  // is NULL.
  template <typename T>
  MatchResultListener& operator<<(const T& x) {
    if (stream_ != NULL)
      *stream_ << x;
    return *this;
  }

  // Returns the underlying ostream.
  ::std::ostream* stream() { return stream_; }

  // Returns true iff the listener is interested in an explanation of
  // the match result.  A matcher's MatchAndExplain() method can use
  // this information to avoid generating the explanation when no one
  // intends to hear it.
  bool IsInterested() const { return stream_ != NULL; }

 private:
  ::std::ostream* const stream_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(MatchResultListener);
};

inline MatchResultListener::~MatchResultListener() {
}

// The implementation of a matcher.
template <typename T>
class MatcherInterface {
 public:
  virtual ~MatcherInterface() {}

  // Returns true iff the matcher matches x; also explains the match
  // result to 'listener', in the form of a non-restrictive relative
  // clause ("which ...", "whose ...", etc) that describes x.  For
  // example, the MatchAndExplain() method of the Pointee(...) matcher
  // should generate an explanation like "which points to ...".
  //
  // You should override this method when defining a new matcher.
  //
  // It's the responsibility of the caller (Google Mock) to guarantee
  // that 'listener' is not NULL.  This helps to simplify a matcher's
  // implementation when it doesn't care about the performance, as it
  // can talk to 'listener' without checking its validity first.
  // However, in order to implement dummy listeners efficiently,
  // listener->stream() may be NULL.
  virtual bool MatchAndExplain(T x, MatchResultListener* listener) const = 0;

  // Describes this matcher to an ostream.  The function should print
  // a verb phrase that describes the property a value matching this
  // matcher should have.  The subject of the verb phrase is the value
  // being matched.  For example, the DescribeTo() method of the Gt(7)
  // matcher prints "is greater than 7".
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
};

namespace internal {

// A match result listener that ignores the explanation.
class DummyMatchResultListener : public MatchResultListener {
 public:
  DummyMatchResultListener() : MatchResultListener(NULL) {}

 private:
  GTEST_DISALLOW_COPY_AND_ASSIGN_(DummyMatchResultListener);
};

// A match result listener that forwards the explanation to a given
// ostream.  The difference between this and MatchResultListener is
// that the former is concrete.
class StreamMatchResultListener : public MatchResultListener {
 public:
  explicit StreamMatchResultListener(::std::ostream* os)
      : MatchResultListener(os) {}

 private:
  GTEST_DISALLOW_COPY_AND_ASSIGN_(StreamMatchResultListener);
};

// A match result listener that stores the explanation in a string.
class StringMatchResultListener : public MatchResultListener {
 public:
  StringMatchResultListener() : MatchResultListener(&ss_) {}

  // Returns the explanation heard so far.
  internal::string str() const { return ss_.str(); }

 private:
  ::std::stringstream ss_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(StringMatchResultListener);
};

// An internal class for implementing Matcher<T>, which will derive
// from it.  We put functionalities common to all Matcher<T>
// specializations here to avoid code duplication.
template <typename T>
class MatcherBase {
 public:
  // Returns true iff the matcher matches x; also explains the match
  // result to 'listener'.
  bool MatchAndExplain(T x, MatchResultListener* listener) const {
    return impl_->MatchAndExplain(x, listener);
  }

  // Returns true iff this matcher matches x.
  bool Matches(T x) const {
    DummyMatchResultListener dummy;
    return MatchAndExplain(x, &dummy);
  }

  // Describes this matcher to an ostream.
  void DescribeTo(::std::ostream* os) const { impl_->DescribeTo(os); }

  // Describes the negation of this matcher to an ostream.
  void DescribeNegationTo(::std::ostream* os) const {
    impl_->DescribeNegationTo(os);
  }

  // Explains why x matches, or doesn't match, the matcher.
  void ExplainMatchResultTo(T x, ::std::ostream* os) const {
    StreamMatchResultListener listener(os);
    MatchAndExplain(x, &listener);
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

}  // namespace internal

// A Matcher<T> is a copyable and IMMUTABLE (except by assignment)
// object that can check whether a value of type T matches.  The
// implementation of Matcher<T> is just a linked_ptr to const
// MatcherInterface<T>, so copying is fairly cheap.  Don't inherit
// from Matcher!
template <typename T>
class Matcher : public internal::MatcherBase<T> {
 public:
  // Constructs a null matcher.  Needed for storing Matcher objects in STL
  // containers.  A default-constructed matcher is not yet initialized.  You
  // cannot use it until a valid value has been assigned to it.
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
class GTEST_API_ Matcher<const internal::string&>
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
class GTEST_API_ Matcher<internal::string>
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
// To define a polymorphic matcher, a user should provide an Impl
// class that has a DescribeTo() method and a DescribeNegationTo()
// method, and define a member function (or member function template)
//
//   bool MatchAndExplain(const Value& value,
//                        MatchResultListener* listener) const;
//
// See the definition of NotNull() for a complete example.
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

    virtual void DescribeTo(::std::ostream* os) const {
      impl_.DescribeTo(os);
    }

    virtual void DescribeNegationTo(::std::ostream* os) const {
      impl_.DescribeNegationTo(os);
    }

    virtual bool MatchAndExplain(T x, MatchResultListener* listener) const {
      return impl_.MatchAndExplain(x, listener);
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

// Anything inside the 'internal' namespace IS INTERNAL IMPLEMENTATION
// and MUST NOT BE USED IN USER CODE!!!
namespace internal {

// The MatcherCastImpl class template is a helper for implementing
// MatcherCast().  We need this helper in order to partially
// specialize the implementation of MatcherCast() (C++ allows
// class/struct templates to be partially specialized, but not
// function templates.).

// This general version is used when MatcherCast()'s argument is a
// polymorphic matcher (i.e. something that can be converted to a
// Matcher but is not one yet; for example, Eq(value)) or a value (for
// example, "hello").
template <typename T, typename M>
class MatcherCastImpl {
 public:
  static Matcher<T> Cast(M polymorphic_matcher_or_value) {
    // M can be a polymorhic matcher, in which case we want to use
    // its conversion operator to create Matcher<T>.  Or it can be a value
    // that should be passed to the Matcher<T>'s constructor.
    //
    // We can't call Matcher<T>(polymorphic_matcher_or_value) when M is a
    // polymorphic matcher because it'll be ambiguous if T has an implicit
    // constructor from M (this usually happens when T has an implicit
    // constructor from any type).
    //
    // It won't work to unconditionally implict_cast
    // polymorphic_matcher_or_value to Matcher<T> because it won't trigger
    // a user-defined conversion from M to T if one exists (assuming M is
    // a value).
    return CastImpl(
        polymorphic_matcher_or_value,
        BooleanConstant<
            internal::ImplicitlyConvertible<M, Matcher<T> >::value>());
  }

 private:
  static Matcher<T> CastImpl(M value, BooleanConstant<false>) {
    // M can't be implicitly converted to Matcher<T>, so M isn't a polymorphic
    // matcher.  It must be a value then.  Use direct initialization to create
    // a matcher.
    return Matcher<T>(ImplicitCast_<T>(value));
  }

  static Matcher<T> CastImpl(M polymorphic_matcher_or_value,
                             BooleanConstant<true>) {
    // M is implicitly convertible to Matcher<T>, which means that either
    // M is a polymorhpic matcher or Matcher<T> has an implicit constructor
    // from M.  In both cases using the implicit conversion will produce a
    // matcher.
    //
    // Even if T has an implicit constructor from M, it won't be called because
    // creating Matcher<T> would require a chain of two user-defined conversions
    // (first to create T from M and then to create Matcher<T> from T).
    return polymorphic_matcher_or_value;
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
    virtual bool MatchAndExplain(T x, MatchResultListener* listener) const {
      return source_matcher_.MatchAndExplain(static_cast<U>(x), listener);
    }

    virtual void DescribeTo(::std::ostream* os) const {
      source_matcher_.DescribeTo(os);
    }

    virtual void DescribeNegationTo(::std::ostream* os) const {
      source_matcher_.DescribeNegationTo(os);
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

}  // namespace internal

// In order to be safe and clear, casting between different matcher
// types is done explicitly via MatcherCast<T>(m), which takes a
// matcher m and returns a Matcher<T>.  It compiles only when T can be
// statically converted to the argument type of m.
template <typename T, typename M>
inline Matcher<T> MatcherCast(M matcher) {
  return internal::MatcherCastImpl<T, M>::Cast(matcher);
}

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
  // This overload handles polymorphic matchers and values only since
  // monomorphic matchers are handled by the next one.
  template <typename M>
  static inline Matcher<T> Cast(M polymorphic_matcher_or_value) {
    return internal::MatcherCastImpl<T, M>::Cast(polymorphic_matcher_or_value);
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
    GTEST_COMPILE_ASSERT_((internal::ImplicitlyConvertible<T, U>::value),
                          T_must_be_implicitly_convertible_to_U);
    // Enforce that we are not converting a non-reference type T to a reference
    // type U.
    GTEST_COMPILE_ASSERT_(
        internal::is_reference<T>::value || !internal::is_reference<U>::value,
        cannot_convert_non_referentce_arg_to_reference);
    // In case both T and U are arithmetic types, enforce that the
    // conversion is not lossy.
    typedef GTEST_REMOVE_REFERENCE_AND_CONST_(T) RawT;
    typedef GTEST_REMOVE_REFERENCE_AND_CONST_(U) RawU;
    const bool kTIsOther = GMOCK_KIND_OF_(RawT) == internal::kOther;
    const bool kUIsOther = GMOCK_KIND_OF_(RawU) == internal::kOther;
    GTEST_COMPILE_ASSERT_(
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

// If the explanation is not empty, prints it to the ostream.
inline void PrintIfNotEmpty(const internal::string& explanation,
                            std::ostream* os) {
  if (explanation != "" && os != NULL) {
    *os << ", " << explanation;
  }
}

// Returns true if the given type name is easy to read by a human.
// This is used to decide whether printing the type of a value might
// be helpful.
inline bool IsReadableTypeName(const string& type_name) {
  // We consider a type name readable if it's short or doesn't contain
  // a template or function type.
  return (type_name.length() <= 20 ||
          type_name.find_first_of("<(") == string::npos);
}

// Matches the value against the given matcher, prints the value and explains
// the match result to the listener. Returns the match result.
// 'listener' must not be NULL.
// Value cannot be passed by const reference, because some matchers take a
// non-const argument.
template <typename Value, typename T>
bool MatchPrintAndExplain(Value& value, const Matcher<T>& matcher,
                          MatchResultListener* listener) {
  if (!listener->IsInterested()) {
    // If the listener is not interested, we do not need to construct the
    // inner explanation.
    return matcher.Matches(value);
  }

  StringMatchResultListener inner_listener;
  const bool match = matcher.MatchAndExplain(value, &inner_listener);

  UniversalPrint(value, listener->stream());
#if GTEST_HAS_RTTI
  const string& type_name = GetTypeName<Value>();
  if (IsReadableTypeName(type_name))
    *listener->stream() << " (of type " << type_name << ")";
#endif
  PrintIfNotEmpty(inner_listener.str(), listener->stream());

  return match;
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

  // TuplePrefix<N>::ExplainMatchFailuresTo(matchers, values, os)
  // describes failures in matching the first N fields of matchers
  // against the first N fields of values.  If there is no failure,
  // nothing will be streamed to os.
  template <typename MatcherTuple, typename ValueTuple>
  static void ExplainMatchFailuresTo(const MatcherTuple& matchers,
                                     const ValueTuple& values,
                                     ::std::ostream* os) {
    using ::std::tr1::tuple_element;
    using ::std::tr1::get;

    // First, describes failures in the first N - 1 fields.
    TuplePrefix<N - 1>::ExplainMatchFailuresTo(matchers, values, os);

    // Then describes the failure (if any) in the (N - 1)-th (0-based)
    // field.
    typename tuple_element<N - 1, MatcherTuple>::type matcher =
        get<N - 1>(matchers);
    typedef typename tuple_element<N - 1, ValueTuple>::type Value;
    Value value = get<N - 1>(values);
    StringMatchResultListener listener;
    if (!matcher.MatchAndExplain(value, &listener)) {
      // TODO(wan): include in the message the name of the parameter
      // as used in MOCK_METHOD*() when possible.
      *os << "  Expected arg #" << N - 1 << ": ";
      get<N - 1>(matchers).DescribeTo(os);
      *os << "\n           Actual: ";
      // We remove the reference in type Value to prevent the
      // universal printer from printing the address of value, which
      // isn't interesting to the user most of the time.  The
      // matcher's MatchAndExplain() method handles the case when
      // the address is interesting.
      internal::UniversalPrint(value, os);
      PrintIfNotEmpty(listener.str(), os);
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
  static void ExplainMatchFailuresTo(const MatcherTuple& /* matchers */,
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
  GTEST_COMPILE_ASSERT_(tuple_size<MatcherTuple>::value ==
                        tuple_size<ValueTuple>::value,
                        matcher_and_value_have_different_numbers_of_fields);
  return TuplePrefix<tuple_size<ValueTuple>::value>::
      Matches(matcher_tuple, value_tuple);
}

// Describes failures in matching matchers against values.  If there
// is no failure, nothing will be streamed to os.
template <typename MatcherTuple, typename ValueTuple>
void ExplainMatchFailureTupleTo(const MatcherTuple& matchers,
                                const ValueTuple& values,
                                ::std::ostream* os) {
  using ::std::tr1::tuple_size;
  TuplePrefix<tuple_size<MatcherTuple>::value>::ExplainMatchFailuresTo(
      matchers, values, os);
}

// Implements A<T>().
template <typename T>
class AnyMatcherImpl : public MatcherInterface<T> {
 public:
  virtual bool MatchAndExplain(
      T /* x */, MatchResultListener* /* listener */) const { return true; }
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
#define GMOCK_IMPLEMENT_COMPARISON_MATCHER_( \
    name, op, relation, negated_relation) \
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
      virtual bool MatchAndExplain(\
          Lhs lhs, MatchResultListener* /* listener */) const { \
        return lhs op rhs_; \
      } \
      virtual void DescribeTo(::std::ostream* os) const { \
        *os << relation  " "; \
        UniversalPrint(rhs_, os); \
      } \
      virtual void DescribeNegationTo(::std::ostream* os) const { \
        *os << negated_relation  " "; \
        UniversalPrint(rhs_, os); \
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
GMOCK_IMPLEMENT_COMPARISON_MATCHER_(Eq, ==, "is equal to", "isn't equal to");
GMOCK_IMPLEMENT_COMPARISON_MATCHER_(Ge, >=, "is >=", "isn't >=");
GMOCK_IMPLEMENT_COMPARISON_MATCHER_(Gt, >, "is >", "isn't >");
GMOCK_IMPLEMENT_COMPARISON_MATCHER_(Le, <=, "is <=", "isn't <=");
GMOCK_IMPLEMENT_COMPARISON_MATCHER_(Lt, <, "is <", "isn't <");
GMOCK_IMPLEMENT_COMPARISON_MATCHER_(Ne, !=, "isn't equal to", "is equal to");

#undef GMOCK_IMPLEMENT_COMPARISON_MATCHER_

// Implements the polymorphic IsNull() matcher, which matches any raw or smart
// pointer that is NULL.
class IsNullMatcher {
 public:
  template <typename Pointer>
  bool MatchAndExplain(const Pointer& p,
                       MatchResultListener* /* listener */) const {
    return GetRawPointer(p) == NULL;
  }

  void DescribeTo(::std::ostream* os) const { *os << "is NULL"; }
  void DescribeNegationTo(::std::ostream* os) const {
    *os << "isn't NULL";
  }
};

// Implements the polymorphic NotNull() matcher, which matches any raw or smart
// pointer that is not NULL.
class NotNullMatcher {
 public:
  template <typename Pointer>
  bool MatchAndExplain(const Pointer& p,
                       MatchResultListener* /* listener */) const {
    return GetRawPointer(p) != NULL;
  }

  void DescribeTo(::std::ostream* os) const { *os << "isn't NULL"; }
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

    // MatchAndExplain() takes a Super& (as opposed to const Super&)
    // in order to match the interface MatcherInterface<Super&>.
    virtual bool MatchAndExplain(
        Super& x, MatchResultListener* listener) const {
      *listener << "which is located @" << static_cast<const void*>(&x);
      return &x == &object_;
    }

    virtual void DescribeTo(::std::ostream* os) const {
      *os << "references the variable ";
      UniversalPrinter<Super&>::Print(object_, os);
    }

    virtual void DescribeNegationTo(::std::ostream* os) const {
      *os << "does not reference the variable ";
      UniversalPrinter<Super&>::Print(object_, os);
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
  StrEqualityMatcher(const StringType& str, bool expect_eq,
                     bool case_sensitive)
      : string_(str), expect_eq_(expect_eq), case_sensitive_(case_sensitive) {}

  // Accepts pointer types, particularly:
  //   const char*
  //   char*
  //   const wchar_t*
  //   wchar_t*
  template <typename CharType>
  bool MatchAndExplain(CharType* s, MatchResultListener* listener) const {
    if (s == NULL) {
      return !expect_eq_;
    }
    return MatchAndExplain(StringType(s), listener);
  }

  // Matches anything that can convert to StringType.
  //
  // This is a template, not just a plain function with const StringType&,
  // because StringPiece has some interfering non-explicit constructors.
  template <typename MatcheeStringType>
  bool MatchAndExplain(const MatcheeStringType& s,
                       MatchResultListener* /* listener */) const {
    const StringType& s2(s);
    const bool eq = case_sensitive_ ? s2 == string_ :
        CaseInsensitiveStringEquals(s2, string_);
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
    *os << (expect_eq ? "is " : "isn't ");
    *os << "equal to ";
    if (!case_sensitive_) {
      *os << "(ignoring case) ";
    }
    UniversalPrint(string_, os);
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
  explicit HasSubstrMatcher(const StringType& substring)
      : substring_(substring) {}

  // Accepts pointer types, particularly:
  //   const char*
  //   char*
  //   const wchar_t*
  //   wchar_t*
  template <typename CharType>
  bool MatchAndExplain(CharType* s, MatchResultListener* listener) const {
    return s != NULL && MatchAndExplain(StringType(s), listener);
  }

  // Matches anything that can convert to StringType.
  //
  // This is a template, not just a plain function with const StringType&,
  // because StringPiece has some interfering non-explicit constructors.
  template <typename MatcheeStringType>
  bool MatchAndExplain(const MatcheeStringType& s,
                       MatchResultListener* /* listener */) const {
    const StringType& s2(s);
    return s2.find(substring_) != StringType::npos;
  }

  // Describes what this matcher matches.
  void DescribeTo(::std::ostream* os) const {
    *os << "has substring ";
    UniversalPrint(substring_, os);
  }

  void DescribeNegationTo(::std::ostream* os) const {
    *os << "has no substring ";
    UniversalPrint(substring_, os);
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
  explicit StartsWithMatcher(const StringType& prefix) : prefix_(prefix) {
  }

  // Accepts pointer types, particularly:
  //   const char*
  //   char*
  //   const wchar_t*
  //   wchar_t*
  template <typename CharType>
  bool MatchAndExplain(CharType* s, MatchResultListener* listener) const {
    return s != NULL && MatchAndExplain(StringType(s), listener);
  }

  // Matches anything that can convert to StringType.
  //
  // This is a template, not just a plain function with const StringType&,
  // because StringPiece has some interfering non-explicit constructors.
  template <typename MatcheeStringType>
  bool MatchAndExplain(const MatcheeStringType& s,
                       MatchResultListener* /* listener */) const {
    const StringType& s2(s);
    return s2.length() >= prefix_.length() &&
        s2.substr(0, prefix_.length()) == prefix_;
  }

  void DescribeTo(::std::ostream* os) const {
    *os << "starts with ";
    UniversalPrint(prefix_, os);
  }

  void DescribeNegationTo(::std::ostream* os) const {
    *os << "doesn't start with ";
    UniversalPrint(prefix_, os);
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
  explicit EndsWithMatcher(const StringType& suffix) : suffix_(suffix) {}

  // Accepts pointer types, particularly:
  //   const char*
  //   char*
  //   const wchar_t*
  //   wchar_t*
  template <typename CharType>
  bool MatchAndExplain(CharType* s, MatchResultListener* listener) const {
    return s != NULL && MatchAndExplain(StringType(s), listener);
  }

  // Matches anything that can convert to StringType.
  //
  // This is a template, not just a plain function with const StringType&,
  // because StringPiece has some interfering non-explicit constructors.
  template <typename MatcheeStringType>
  bool MatchAndExplain(const MatcheeStringType& s,
                       MatchResultListener* /* listener */) const {
    const StringType& s2(s);
    return s2.length() >= suffix_.length() &&
        s2.substr(s2.length() - suffix_.length()) == suffix_;
  }

  void DescribeTo(::std::ostream* os) const {
    *os << "ends with ";
    UniversalPrint(suffix_, os);
  }

  void DescribeNegationTo(::std::ostream* os) const {
    *os << "doesn't end with ";
    UniversalPrint(suffix_, os);
  }

 private:
  const StringType suffix_;

  GTEST_DISALLOW_ASSIGN_(EndsWithMatcher);
};

// Implements polymorphic matchers MatchesRegex(regex) and
// ContainsRegex(regex), which can be used as a Matcher<T> as long as
// T can be converted to a string.
class MatchesRegexMatcher {
 public:
  MatchesRegexMatcher(const RE* regex, bool full_match)
      : regex_(regex), full_match_(full_match) {}

  // Accepts pointer types, particularly:
  //   const char*
  //   char*
  //   const wchar_t*
  //   wchar_t*
  template <typename CharType>
  bool MatchAndExplain(CharType* s, MatchResultListener* listener) const {
    return s != NULL && MatchAndExplain(internal::string(s), listener);
  }

  // Matches anything that can convert to internal::string.
  //
  // This is a template, not just a plain function with const internal::string&,
  // because StringPiece has some interfering non-explicit constructors.
  template <class MatcheeStringType>
  bool MatchAndExplain(const MatcheeStringType& s,
                       MatchResultListener* /* listener */) const {
    const internal::string& s2(s);
    return full_match_ ? RE::FullMatch(s2, *regex_) :
        RE::PartialMatch(s2, *regex_);
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
#define GMOCK_IMPLEMENT_COMPARISON2_MATCHER_(name, op, relation) \
  class name##2Matcher { \
   public: \
    template <typename T1, typename T2> \
    operator Matcher< ::std::tr1::tuple<T1, T2> >() const { \
      return MakeMatcher(new Impl< ::std::tr1::tuple<T1, T2> >); \
    } \
    template <typename T1, typename T2> \
    operator Matcher<const ::std::tr1::tuple<T1, T2>&>() const { \
      return MakeMatcher(new Impl<const ::std::tr1::tuple<T1, T2>&>); \
    } \
   private: \
    template <typename Tuple> \
    class Impl : public MatcherInterface<Tuple> { \
     public: \
      virtual bool MatchAndExplain( \
          Tuple args, \
          MatchResultListener* /* listener */) const { \
        return ::std::tr1::get<0>(args) op ::std::tr1::get<1>(args); \
      } \
      virtual void DescribeTo(::std::ostream* os) const { \
        *os << "are " relation;                                 \
      } \
      virtual void DescribeNegationTo(::std::ostream* os) const { \
        *os << "aren't " relation; \
      } \
    }; \
  }

// Implements Eq(), Ge(), Gt(), Le(), Lt(), and Ne() respectively.
GMOCK_IMPLEMENT_COMPARISON2_MATCHER_(Eq, ==, "an equal pair");
GMOCK_IMPLEMENT_COMPARISON2_MATCHER_(
    Ge, >=, "a pair where the first >= the second");
GMOCK_IMPLEMENT_COMPARISON2_MATCHER_(
    Gt, >, "a pair where the first > the second");
GMOCK_IMPLEMENT_COMPARISON2_MATCHER_(
    Le, <=, "a pair where the first <= the second");
GMOCK_IMPLEMENT_COMPARISON2_MATCHER_(
    Lt, <, "a pair where the first < the second");
GMOCK_IMPLEMENT_COMPARISON2_MATCHER_(Ne, !=, "an unequal pair");

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

  virtual bool MatchAndExplain(T x, MatchResultListener* listener) const {
    return !matcher_.MatchAndExplain(x, listener);
  }

  virtual void DescribeTo(::std::ostream* os) const {
    matcher_.DescribeNegationTo(os);
  }

  virtual void DescribeNegationTo(::std::ostream* os) const {
    matcher_.DescribeTo(os);
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

  virtual void DescribeTo(::std::ostream* os) const {
    *os << "(";
    matcher1_.DescribeTo(os);
    *os << ") and (";
    matcher2_.DescribeTo(os);
    *os << ")";
  }

  virtual void DescribeNegationTo(::std::ostream* os) const {
    *os << "(";
    matcher1_.DescribeNegationTo(os);
    *os << ") or (";
    matcher2_.DescribeNegationTo(os);
    *os << ")";
  }

  virtual bool MatchAndExplain(T x, MatchResultListener* listener) const {
    // If either matcher1_ or matcher2_ doesn't match x, we only need
    // to explain why one of them fails.
    StringMatchResultListener listener1;
    if (!matcher1_.MatchAndExplain(x, &listener1)) {
      *listener << listener1.str();
      return false;
    }

    StringMatchResultListener listener2;
    if (!matcher2_.MatchAndExplain(x, &listener2)) {
      *listener << listener2.str();
      return false;
    }

    // Otherwise we need to explain why *both* of them match.
    const internal::string s1 = listener1.str();
    const internal::string s2 = listener2.str();

    if (s1 == "") {
      *listener << s2;
    } else {
      *listener << s1;
      if (s2 != "") {
        *listener << ", and " << s2;
      }
    }
    return true;
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

  virtual void DescribeTo(::std::ostream* os) const {
    *os << "(";
    matcher1_.DescribeTo(os);
    *os << ") or (";
    matcher2_.DescribeTo(os);
    *os << ")";
  }

  virtual void DescribeNegationTo(::std::ostream* os) const {
    *os << "(";
    matcher1_.DescribeNegationTo(os);
    *os << ") and (";
    matcher2_.DescribeNegationTo(os);
    *os << ")";
  }

  virtual bool MatchAndExplain(T x, MatchResultListener* listener) const {
    // If either matcher1_ or matcher2_ matches x, we just need to
    // explain why *one* of them matches.
    StringMatchResultListener listener1;
    if (matcher1_.MatchAndExplain(x, &listener1)) {
      *listener << listener1.str();
      return true;
    }

    StringMatchResultListener listener2;
    if (matcher2_.MatchAndExplain(x, &listener2)) {
      *listener << listener2.str();
      return true;
    }

    // Otherwise we need to explain why *both* of them fail.
    const internal::string s1 = listener1.str();
    const internal::string s2 = listener2.str();

    if (s1 == "") {
      *listener << s2;
    } else {
      *listener << s1;
      if (s2 != "") {
        *listener << ", and " << s2;
      }
    }
    return false;
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
  bool MatchAndExplain(T& x,  // NOLINT
                       MatchResultListener* /* listener */) const {
    // Without the if-statement, MSVC sometimes warns about converting
    // a value to bool (warning 4800).
    //
    // We cannot write 'return !!predicate_(x);' as that doesn't work
    // when predicate_(x) returns a class convertible to bool but
    // having no operator!().
    if (predicate_(x))
      return true;
    return false;
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
    StringMatchResultListener listener;
    if (MatchPrintAndExplain(x, matcher, &listener))
      return AssertionSuccess();

    ::std::stringstream ss;
    ss << "Value of: " << value_text << "\n"
       << "Expected: ";
    matcher.DescribeTo(&ss);
    ss << "\n  Actual: " << listener.str();
    return AssertionFailure() << ss.str();
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

    virtual bool MatchAndExplain(T value,
                                 MatchResultListener* /* listener */) const {
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
          *os << "isn't NaN";
        } else {
          *os << "is anything";
        }
      } else {
        *os << "isn't approximately " << rhs_;
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
    typedef typename PointeeOf<GTEST_REMOVE_CONST_(  // NOLINT
        GTEST_REMOVE_REFERENCE_(Pointer))>::type Pointee;

    explicit Impl(const InnerMatcher& matcher)
        : matcher_(MatcherCast<const Pointee&>(matcher)) {}

    virtual void DescribeTo(::std::ostream* os) const {
      *os << "points to a value that ";
      matcher_.DescribeTo(os);
    }

    virtual void DescribeNegationTo(::std::ostream* os) const {
      *os << "does not point to a value that ";
      matcher_.DescribeTo(os);
    }

    virtual bool MatchAndExplain(Pointer pointer,
                                 MatchResultListener* listener) const {
      if (GetRawPointer(pointer) == NULL)
        return false;

      *listener << "which points to ";
      return MatchPrintAndExplain(*pointer, matcher_, listener);
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

  void DescribeTo(::std::ostream* os) const {
    *os << "is an object whose given field ";
    matcher_.DescribeTo(os);
  }

  void DescribeNegationTo(::std::ostream* os) const {
    *os << "is an object whose given field ";
    matcher_.DescribeNegationTo(os);
  }

  template <typename T>
  bool MatchAndExplain(const T& value, MatchResultListener* listener) const {
    return MatchAndExplainImpl(
        typename ::testing::internal::
            is_pointer<GTEST_REMOVE_CONST_(T)>::type(),
        value, listener);
  }

 private:
  // The first argument of MatchAndExplainImpl() is needed to help
  // Symbian's C++ compiler choose which overload to use.  Its type is
  // true_type iff the Field() matcher is used to match a pointer.
  bool MatchAndExplainImpl(false_type /* is_not_pointer */, const Class& obj,
                           MatchResultListener* listener) const {
    *listener << "whose given field is ";
    return MatchPrintAndExplain(obj.*field_, matcher_, listener);
  }

  bool MatchAndExplainImpl(true_type /* is_pointer */, const Class* p,
                           MatchResultListener* listener) const {
    if (p == NULL)
      return false;

    *listener << "which points to an object ";
    // Since *p has a field, it must be a class/struct/union type and
    // thus cannot be a pointer.  Therefore we pass false_type() as
    // the first argument.
    return MatchAndExplainImpl(false_type(), *p, listener);
  }

  const FieldType Class::*field_;
  const Matcher<const FieldType&> matcher_;

  GTEST_DISALLOW_ASSIGN_(FieldMatcher);
};

// Implements the Property() matcher for matching a property
// (i.e. return value of a getter method) of an object.
template <typename Class, typename PropertyType>
class PropertyMatcher {
 public:
  // The property may have a reference type, so 'const PropertyType&'
  // may cause double references and fail to compile.  That's why we
  // need GTEST_REFERENCE_TO_CONST, which works regardless of
  // PropertyType being a reference or not.
  typedef GTEST_REFERENCE_TO_CONST_(PropertyType) RefToConstProperty;

  PropertyMatcher(PropertyType (Class::*property)() const,
                  const Matcher<RefToConstProperty>& matcher)
      : property_(property), matcher_(matcher) {}

  void DescribeTo(::std::ostream* os) const {
    *os << "is an object whose given property ";
    matcher_.DescribeTo(os);
  }

  void DescribeNegationTo(::std::ostream* os) const {
    *os << "is an object whose given property ";
    matcher_.DescribeNegationTo(os);
  }

  template <typename T>
  bool MatchAndExplain(const T&value, MatchResultListener* listener) const {
    return MatchAndExplainImpl(
        typename ::testing::internal::
            is_pointer<GTEST_REMOVE_CONST_(T)>::type(),
        value, listener);
  }

 private:
  // The first argument of MatchAndExplainImpl() is needed to help
  // Symbian's C++ compiler choose which overload to use.  Its type is
  // true_type iff the Property() matcher is used to match a pointer.
  bool MatchAndExplainImpl(false_type /* is_not_pointer */, const Class& obj,
                           MatchResultListener* listener) const {
    *listener << "whose given property is ";
    // Cannot pass the return value (for example, int) to MatchPrintAndExplain,
    // which takes a non-const reference as argument.
    RefToConstProperty result = (obj.*property_)();
    return MatchPrintAndExplain(result, matcher_, listener);
  }

  bool MatchAndExplainImpl(true_type /* is_pointer */, const Class* p,
                           MatchResultListener* listener) const {
    if (p == NULL)
      return false;

    *listener << "which points to an object ";
    // Since *p has a property method, it must be a class/struct/union
    // type and thus cannot be a pointer.  Therefore we pass
    // false_type() as the first argument.
    return MatchAndExplainImpl(false_type(), *p, listener);
  }

  PropertyType (Class::*property_)() const;
  const Matcher<RefToConstProperty> matcher_;

  GTEST_DISALLOW_ASSIGN_(PropertyMatcher);
};

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

    virtual void DescribeTo(::std::ostream* os) const {
      *os << "is mapped by the given callable to a value that ";
      matcher_.DescribeTo(os);
    }

    virtual void DescribeNegationTo(::std::ostream* os) const {
      *os << "is mapped by the given callable to a value that ";
      matcher_.DescribeNegationTo(os);
    }

    virtual bool MatchAndExplain(T obj, MatchResultListener* listener) const {
      *listener << "which is mapped by the given callable to ";
      // Cannot pass the return value (for example, int) to
      // MatchPrintAndExplain, which takes a non-const reference as argument.
      ResultType result =
          CallableTraits<Callable>::template Invoke<T>(callable_, obj);
      return MatchPrintAndExplain(result, matcher_, listener);
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
    (void)testing::StaticAssertTypeEq<Container,
        GTEST_REMOVE_REFERENCE_AND_CONST_(Container)>();
  }

  void DescribeTo(::std::ostream* os) const {
    *os << "equals ";
    UniversalPrint(rhs_, os);
  }
  void DescribeNegationTo(::std::ostream* os) const {
    *os << "does not equal ";
    UniversalPrint(rhs_, os);
  }

  template <typename LhsContainer>
  bool MatchAndExplain(const LhsContainer& lhs,
                       MatchResultListener* listener) const {
    // GTEST_REMOVE_CONST_() is needed to work around an MSVC 8.0 bug
    // that causes LhsContainer to be a const type sometimes.
    typedef internal::StlContainerView<GTEST_REMOVE_CONST_(LhsContainer)>
        LhsView;
    typedef typename LhsView::type LhsStlContainer;
    StlContainerReference lhs_stl_container = LhsView::ConstReference(lhs);
    if (lhs_stl_container == rhs_)
      return true;

    ::std::ostream* const os = listener->stream();
    if (os != NULL) {
      // Something is different. Check for extra values first.
      bool printed_header = false;
      for (typename LhsStlContainer::const_iterator it =
               lhs_stl_container.begin();
           it != lhs_stl_container.end(); ++it) {
        if (internal::ArrayAwareFind(rhs_.begin(), rhs_.end(), *it) ==
            rhs_.end()) {
          if (printed_header) {
            *os << ", ";
          } else {
            *os << "which has these unexpected elements: ";
            printed_header = true;
          }
          UniversalPrint(*it, os);
        }
      }

      // Now check for missing values.
      bool printed_header2 = false;
      for (typename StlContainer::const_iterator it = rhs_.begin();
           it != rhs_.end(); ++it) {
        if (internal::ArrayAwareFind(
                lhs_stl_container.begin(), lhs_stl_container.end(), *it) ==
            lhs_stl_container.end()) {
          if (printed_header2) {
            *os << ", ";
          } else {
            *os << (printed_header ? ",\nand" : "which")
                << " doesn't have these expected elements: ";
            printed_header2 = true;
          }
          UniversalPrint(*it, os);
        }
      }
    }

    return false;
  }

 private:
  const StlContainer rhs_;

  GTEST_DISALLOW_ASSIGN_(ContainerEqMatcher);
};

// A comparator functor that uses the < operator to compare two values.
struct LessComparator {
  template <typename T, typename U>
  bool operator()(const T& lhs, const U& rhs) const { return lhs < rhs; }
};

// Implements WhenSortedBy(comparator, container_matcher).
template <typename Comparator, typename ContainerMatcher>
class WhenSortedByMatcher {
 public:
  WhenSortedByMatcher(const Comparator& comparator,
                      const ContainerMatcher& matcher)
      : comparator_(comparator), matcher_(matcher) {}

  template <typename LhsContainer>
  operator Matcher<LhsContainer>() const {
    return MakeMatcher(new Impl<LhsContainer>(comparator_, matcher_));
  }

  template <typename LhsContainer>
  class Impl : public MatcherInterface<LhsContainer> {
   public:
    typedef internal::StlContainerView<
         GTEST_REMOVE_REFERENCE_AND_CONST_(LhsContainer)> LhsView;
    typedef typename LhsView::type LhsStlContainer;
    typedef typename LhsView::const_reference LhsStlContainerReference;
    typedef typename LhsStlContainer::value_type LhsValue;

    Impl(const Comparator& comparator, const ContainerMatcher& matcher)
        : comparator_(comparator), matcher_(matcher) {}

    virtual void DescribeTo(::std::ostream* os) const {
      *os << "(when sorted) ";
      matcher_.DescribeTo(os);
    }

    virtual void DescribeNegationTo(::std::ostream* os) const {
      *os << "(when sorted) ";
      matcher_.DescribeNegationTo(os);
    }

    virtual bool MatchAndExplain(LhsContainer lhs,
                                 MatchResultListener* listener) const {
      LhsStlContainerReference lhs_stl_container = LhsView::ConstReference(lhs);
      std::vector<LhsValue> sorted_container(lhs_stl_container.begin(),
                                             lhs_stl_container.end());
      std::sort(sorted_container.begin(), sorted_container.end(), comparator_);

      if (!listener->IsInterested()) {
        // If the listener is not interested, we do not need to
        // construct the inner explanation.
        return matcher_.Matches(sorted_container);
      }

      *listener << "which is ";
      UniversalPrint(sorted_container, listener->stream());
      *listener << " when sorted";

      StringMatchResultListener inner_listener;
      const bool match = matcher_.MatchAndExplain(sorted_container,
                                                  &inner_listener);
      PrintIfNotEmpty(inner_listener.str(), listener->stream());
      return match;
    }

   private:
    const Comparator comparator_;
    const Matcher<const std::vector<LhsValue>&> matcher_;

    GTEST_DISALLOW_COPY_AND_ASSIGN_(Impl);
  };

 private:
  const Comparator comparator_;
  const ContainerMatcher matcher_;

  GTEST_DISALLOW_ASSIGN_(WhenSortedByMatcher);
};

// Implements Pointwise(tuple_matcher, rhs_container).  tuple_matcher
// must be able to be safely cast to Matcher<tuple<const T1&, const
// T2&> >, where T1 and T2 are the types of elements in the LHS
// container and the RHS container respectively.
template <typename TupleMatcher, typename RhsContainer>
class PointwiseMatcher {
 public:
  typedef internal::StlContainerView<RhsContainer> RhsView;
  typedef typename RhsView::type RhsStlContainer;
  typedef typename RhsStlContainer::value_type RhsValue;

  // Like ContainerEq, we make a copy of rhs in case the elements in
  // it are modified after this matcher is created.
  PointwiseMatcher(const TupleMatcher& tuple_matcher, const RhsContainer& rhs)
      : tuple_matcher_(tuple_matcher), rhs_(RhsView::Copy(rhs)) {
    // Makes sure the user doesn't instantiate this class template
    // with a const or reference type.
    (void)testing::StaticAssertTypeEq<RhsContainer,
        GTEST_REMOVE_REFERENCE_AND_CONST_(RhsContainer)>();
  }

  template <typename LhsContainer>
  operator Matcher<LhsContainer>() const {
    return MakeMatcher(new Impl<LhsContainer>(tuple_matcher_, rhs_));
  }

  template <typename LhsContainer>
  class Impl : public MatcherInterface<LhsContainer> {
   public:
    typedef internal::StlContainerView<
         GTEST_REMOVE_REFERENCE_AND_CONST_(LhsContainer)> LhsView;
    typedef typename LhsView::type LhsStlContainer;
    typedef typename LhsView::const_reference LhsStlContainerReference;
    typedef typename LhsStlContainer::value_type LhsValue;
    // We pass the LHS value and the RHS value to the inner matcher by
    // reference, as they may be expensive to copy.  We must use tuple
    // instead of pair here, as a pair cannot hold references (C++ 98,
    // 20.2.2 [lib.pairs]).
    typedef std::tr1::tuple<const LhsValue&, const RhsValue&> InnerMatcherArg;

    Impl(const TupleMatcher& tuple_matcher, const RhsStlContainer& rhs)
        // mono_tuple_matcher_ holds a monomorphic version of the tuple matcher.
        : mono_tuple_matcher_(SafeMatcherCast<InnerMatcherArg>(tuple_matcher)),
          rhs_(rhs) {}

    virtual void DescribeTo(::std::ostream* os) const {
      *os << "contains " << rhs_.size()
          << " values, where each value and its corresponding value in ";
      UniversalPrinter<RhsStlContainer>::Print(rhs_, os);
      *os << " ";
      mono_tuple_matcher_.DescribeTo(os);
    }
    virtual void DescribeNegationTo(::std::ostream* os) const {
      *os << "doesn't contain exactly " << rhs_.size()
          << " values, or contains a value x at some index i"
          << " where x and the i-th value of ";
      UniversalPrint(rhs_, os);
      *os << " ";
      mono_tuple_matcher_.DescribeNegationTo(os);
    }

    virtual bool MatchAndExplain(LhsContainer lhs,
                                 MatchResultListener* listener) const {
      LhsStlContainerReference lhs_stl_container = LhsView::ConstReference(lhs);
      const size_t actual_size = lhs_stl_container.size();
      if (actual_size != rhs_.size()) {
        *listener << "which contains " << actual_size << " values";
        return false;
      }

      typename LhsStlContainer::const_iterator left = lhs_stl_container.begin();
      typename RhsStlContainer::const_iterator right = rhs_.begin();
      for (size_t i = 0; i != actual_size; ++i, ++left, ++right) {
        const InnerMatcherArg value_pair(*left, *right);

        if (listener->IsInterested()) {
          StringMatchResultListener inner_listener;
          if (!mono_tuple_matcher_.MatchAndExplain(
                  value_pair, &inner_listener)) {
            *listener << "where the value pair (";
            UniversalPrint(*left, listener->stream());
            *listener << ", ";
            UniversalPrint(*right, listener->stream());
            *listener << ") at index #" << i << " don't match";
            PrintIfNotEmpty(inner_listener.str(), listener->stream());
            return false;
          }
        } else {
          if (!mono_tuple_matcher_.Matches(value_pair))
            return false;
        }
      }

      return true;
    }

   private:
    const Matcher<InnerMatcherArg> mono_tuple_matcher_;
    const RhsStlContainer rhs_;

    GTEST_DISALLOW_ASSIGN_(Impl);
  };

 private:
  const TupleMatcher tuple_matcher_;
  const RhsStlContainer rhs_;

  GTEST_DISALLOW_ASSIGN_(PointwiseMatcher);
};

// Holds the logic common to ContainsMatcherImpl and EachMatcherImpl.
template <typename Container>
class QuantifierMatcherImpl : public MatcherInterface<Container> {
 public:
  typedef GTEST_REMOVE_REFERENCE_AND_CONST_(Container) RawContainer;
  typedef StlContainerView<RawContainer> View;
  typedef typename View::type StlContainer;
  typedef typename View::const_reference StlContainerReference;
  typedef typename StlContainer::value_type Element;

  template <typename InnerMatcher>
  explicit QuantifierMatcherImpl(InnerMatcher inner_matcher)
      : inner_matcher_(
           testing::SafeMatcherCast<const Element&>(inner_matcher)) {}

  // Checks whether:
  // * All elements in the container match, if all_elements_should_match.
  // * Any element in the container matches, if !all_elements_should_match.
  bool MatchAndExplainImpl(bool all_elements_should_match,
                           Container container,
                           MatchResultListener* listener) const {
    StlContainerReference stl_container = View::ConstReference(container);
    size_t i = 0;
    for (typename StlContainer::const_iterator it = stl_container.begin();
         it != stl_container.end(); ++it, ++i) {
      StringMatchResultListener inner_listener;
      const bool matches = inner_matcher_.MatchAndExplain(*it, &inner_listener);

      if (matches != all_elements_should_match) {
        *listener << "whose element #" << i
                  << (matches ? " matches" : " doesn't match");
        PrintIfNotEmpty(inner_listener.str(), listener->stream());
        return !all_elements_should_match;
      }
    }
    return all_elements_should_match;
  }

 protected:
  const Matcher<const Element&> inner_matcher_;

  GTEST_DISALLOW_ASSIGN_(QuantifierMatcherImpl);
};

// Implements Contains(element_matcher) for the given argument type Container.
// Symmetric to EachMatcherImpl.
template <typename Container>
class ContainsMatcherImpl : public QuantifierMatcherImpl<Container> {
 public:
  template <typename InnerMatcher>
  explicit ContainsMatcherImpl(InnerMatcher inner_matcher)
      : QuantifierMatcherImpl<Container>(inner_matcher) {}

  // Describes what this matcher does.
  virtual void DescribeTo(::std::ostream* os) const {
    *os << "contains at least one element that ";
    this->inner_matcher_.DescribeTo(os);
  }

  virtual void DescribeNegationTo(::std::ostream* os) const {
    *os << "doesn't contain any element that ";
    this->inner_matcher_.DescribeTo(os);
  }

  virtual bool MatchAndExplain(Container container,
                               MatchResultListener* listener) const {
    return this->MatchAndExplainImpl(false, container, listener);
  }

 private:
  GTEST_DISALLOW_ASSIGN_(ContainsMatcherImpl);
};

// Implements Each(element_matcher) for the given argument type Container.
// Symmetric to ContainsMatcherImpl.
template <typename Container>
class EachMatcherImpl : public QuantifierMatcherImpl<Container> {
 public:
  template <typename InnerMatcher>
  explicit EachMatcherImpl(InnerMatcher inner_matcher)
      : QuantifierMatcherImpl<Container>(inner_matcher) {}

  // Describes what this matcher does.
  virtual void DescribeTo(::std::ostream* os) const {
    *os << "only contains elements that ";
    this->inner_matcher_.DescribeTo(os);
  }

  virtual void DescribeNegationTo(::std::ostream* os) const {
    *os << "contains some element that ";
    this->inner_matcher_.DescribeNegationTo(os);
  }

  virtual bool MatchAndExplain(Container container,
                               MatchResultListener* listener) const {
    return this->MatchAndExplainImpl(true, container, listener);
  }

 private:
  GTEST_DISALLOW_ASSIGN_(EachMatcherImpl);
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

// Implements polymorphic Each(element_matcher).
template <typename M>
class EachMatcher {
 public:
  explicit EachMatcher(M m) : inner_matcher_(m) {}

  template <typename Container>
  operator Matcher<Container>() const {
    return MakeMatcher(new EachMatcherImpl<Container>(inner_matcher_));
  }

 private:
  const M inner_matcher_;

  GTEST_DISALLOW_ASSIGN_(EachMatcher);
};

// Implements Key(inner_matcher) for the given argument pair type.
// Key(inner_matcher) matches an std::pair whose 'first' field matches
// inner_matcher.  For example, Contains(Key(Ge(5))) can be used to match an
// std::map that contains at least one element whose key is >= 5.
template <typename PairType>
class KeyMatcherImpl : public MatcherInterface<PairType> {
 public:
  typedef GTEST_REMOVE_REFERENCE_AND_CONST_(PairType) RawPairType;
  typedef typename RawPairType::first_type KeyType;

  template <typename InnerMatcher>
  explicit KeyMatcherImpl(InnerMatcher inner_matcher)
      : inner_matcher_(
          testing::SafeMatcherCast<const KeyType&>(inner_matcher)) {
  }

  // Returns true iff 'key_value.first' (the key) matches the inner matcher.
  virtual bool MatchAndExplain(PairType key_value,
                               MatchResultListener* listener) const {
    StringMatchResultListener inner_listener;
    const bool match = inner_matcher_.MatchAndExplain(key_value.first,
                                                      &inner_listener);
    const internal::string explanation = inner_listener.str();
    if (explanation != "") {
      *listener << "whose first field is a value " << explanation;
    }
    return match;
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
  typedef GTEST_REMOVE_REFERENCE_AND_CONST_(PairType) RawPairType;
  typedef typename RawPairType::first_type FirstType;
  typedef typename RawPairType::second_type SecondType;

  template <typename FirstMatcher, typename SecondMatcher>
  PairMatcherImpl(FirstMatcher first_matcher, SecondMatcher second_matcher)
      : first_matcher_(
            testing::SafeMatcherCast<const FirstType&>(first_matcher)),
        second_matcher_(
            testing::SafeMatcherCast<const SecondType&>(second_matcher)) {
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

  // Returns true iff 'a_pair.first' matches first_matcher and 'a_pair.second'
  // matches second_matcher.
  virtual bool MatchAndExplain(PairType a_pair,
                               MatchResultListener* listener) const {
    if (!listener->IsInterested()) {
      // If the listener is not interested, we don't need to construct the
      // explanation.
      return first_matcher_.Matches(a_pair.first) &&
             second_matcher_.Matches(a_pair.second);
    }
    StringMatchResultListener first_inner_listener;
    if (!first_matcher_.MatchAndExplain(a_pair.first,
                                        &first_inner_listener)) {
      *listener << "whose first field does not match";
      PrintIfNotEmpty(first_inner_listener.str(), listener->stream());
      return false;
    }
    StringMatchResultListener second_inner_listener;
    if (!second_matcher_.MatchAndExplain(a_pair.second,
                                         &second_inner_listener)) {
      *listener << "whose second field does not match";
      PrintIfNotEmpty(second_inner_listener.str(), listener->stream());
      return false;
    }
    ExplainSuccess(first_inner_listener.str(), second_inner_listener.str(),
                   listener);
    return true;
  }

 private:
  void ExplainSuccess(const internal::string& first_explanation,
                      const internal::string& second_explanation,
                      MatchResultListener* listener) const {
    *listener << "whose both fields match";
    if (first_explanation != "") {
      *listener << ", where the first field is a value " << first_explanation;
    }
    if (second_explanation != "") {
      *listener << ", ";
      if (first_explanation != "") {
        *listener << "and ";
      } else {
        *listener << "where ";
      }
      *listener << "the second field is a value " << second_explanation;
    }
  }

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
  typedef GTEST_REMOVE_REFERENCE_AND_CONST_(Container) RawContainer;
  typedef internal::StlContainerView<RawContainer> View;
  typedef typename View::type StlContainer;
  typedef typename View::const_reference StlContainerReference;
  typedef typename StlContainer::value_type Element;

  // Constructs the matcher from a sequence of element values or
  // element matchers.
  template <typename InputIter>
  ElementsAreMatcherImpl(InputIter first, InputIter last) {
    while (first != last) {
      matchers_.push_back(MatcherCast<const Element&>(*first++));
    }
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
        *os << "element #" << i << " ";
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
      *os << "isn't empty";
      return;
    }

    *os << "doesn't have " << Elements(count()) << ", or\n";
    for (size_t i = 0; i != count(); ++i) {
      *os << "element #" << i << " ";
      matchers_[i].DescribeNegationTo(os);
      if (i + 1 < count()) {
        *os << ", or\n";
      }
    }
  }

  virtual bool MatchAndExplain(Container container,
                               MatchResultListener* listener) const {
    StlContainerReference stl_container = View::ConstReference(container);
    const size_t actual_count = stl_container.size();
    if (actual_count != count()) {
      // The element count doesn't match.  If the container is empty,
      // there's no need to explain anything as Google Mock already
      // prints the empty container.  Otherwise we just need to show
      // how many elements there actually are.
      if (actual_count != 0) {
        *listener << "which has " << Elements(actual_count);
      }
      return false;
    }

    typename StlContainer::const_iterator it = stl_container.begin();
    // explanations[i] is the explanation of the element at index i.
    std::vector<internal::string> explanations(count());
    for (size_t i = 0; i != count();  ++it, ++i) {
      StringMatchResultListener s;
      if (matchers_[i].MatchAndExplain(*it, &s)) {
        explanations[i] = s.str();
      } else {
        // The container has the right size but the i-th element
        // doesn't match its expectation.
        *listener << "whose element #" << i << " doesn't match";
        PrintIfNotEmpty(s.str(), listener->stream());
        return false;
      }
    }

    // Every element matches its expectation.  We need to explain why
    // (the obvious ones can be skipped).
    bool reason_printed = false;
    for (size_t i = 0; i != count(); ++i) {
      const internal::string& s = explanations[i];
      if (!s.empty()) {
        if (reason_printed) {
          *listener << ",\nand ";
        }
        *listener << "whose element #" << i << " matches, " << s;
        reason_printed = true;
      }
    }

    return true;
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
    typedef GTEST_REMOVE_REFERENCE_AND_CONST_(Container) RawContainer;
    typedef typename internal::StlContainerView<RawContainer>::type::value_type
        Element;

    const Matcher<const Element&>* const matchers = NULL;
    return MakeMatcher(new ElementsAreMatcherImpl<Container>(matchers,
                                                             matchers));
  }
};

// Implements ElementsAreArray().
template <typename T>
class ElementsAreArrayMatcher {
 public:
  template <typename Iter>
  ElementsAreArrayMatcher(Iter first, Iter last) : matchers_(first, last) {}

  template <typename Container>
  operator Matcher<Container>() const {
    return MakeMatcher(new ElementsAreMatcherImpl<Container>(
        matchers_.begin(), matchers_.end()));
  }

 private:
  const std::vector<T> matchers_;

  GTEST_DISALLOW_ASSIGN_(ElementsAreArrayMatcher);
};

// Returns the description for a matcher defined using the MATCHER*()
// macro where the user-supplied description string is "", if
// 'negation' is false; otherwise returns the description of the
// negation of the matcher.  'param_values' contains a list of strings
// that are the print-out of the matcher's parameters.
GTEST_API_ string FormatMatcherDescription(bool negation,
                                           const char* matcher_name,
                                           const Strings& param_values);

}  // namespace internal

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
          MatcherCast<GTEST_REFERENCE_TO_CONST_(PropertyType)>(matcher)));
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
inline PolymorphicMatcher<internal::ContainerEqMatcher<  // NOLINT
                            GTEST_REMOVE_CONST_(Container)> >
    ContainerEq(const Container& rhs) {
  // This following line is for working around a bug in MSVC 8.0,
  // which causes Container to be a const type sometimes.
  typedef GTEST_REMOVE_CONST_(Container) RawContainer;
  return MakePolymorphicMatcher(
      internal::ContainerEqMatcher<RawContainer>(rhs));
}

// Returns a matcher that matches a container that, when sorted using
// the given comparator, matches container_matcher.
template <typename Comparator, typename ContainerMatcher>
inline internal::WhenSortedByMatcher<Comparator, ContainerMatcher>
WhenSortedBy(const Comparator& comparator,
             const ContainerMatcher& container_matcher) {
  return internal::WhenSortedByMatcher<Comparator, ContainerMatcher>(
      comparator, container_matcher);
}

// Returns a matcher that matches a container that, when sorted using
// the < operator, matches container_matcher.
template <typename ContainerMatcher>
inline internal::WhenSortedByMatcher<internal::LessComparator, ContainerMatcher>
WhenSorted(const ContainerMatcher& container_matcher) {
  return
      internal::WhenSortedByMatcher<internal::LessComparator, ContainerMatcher>(
          internal::LessComparator(), container_matcher);
}

// Matches an STL-style container or a native array that contains the
// same number of elements as in rhs, where its i-th element and rhs's
// i-th element (as a pair) satisfy the given pair matcher, for all i.
// TupleMatcher must be able to be safely cast to Matcher<tuple<const
// T1&, const T2&> >, where T1 and T2 are the types of elements in the
// LHS container and the RHS container respectively.
template <typename TupleMatcher, typename Container>
inline internal::PointwiseMatcher<TupleMatcher,
                                  GTEST_REMOVE_CONST_(Container)>
Pointwise(const TupleMatcher& tuple_matcher, const Container& rhs) {
  // This following line is for working around a bug in MSVC 8.0,
  // which causes Container to be a const type sometimes.
  typedef GTEST_REMOVE_CONST_(Container) RawContainer;
  return internal::PointwiseMatcher<TupleMatcher, RawContainer>(
      tuple_matcher, rhs);
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

// Matches an STL-style container or a native array that contains only
// elements matching the given value or matcher.
//
// Each(m) is semantically equivalent to Not(Contains(Not(m))). Only
// the messages are different.
//
// Examples:
//   ::std::set<int> page_ids;
//   // Each(m) matches an empty container, regardless of what m is.
//   EXPECT_THAT(page_ids, Each(Eq(1)));
//   EXPECT_THAT(page_ids, Each(Eq(77)));
//
//   page_ids.insert(3);
//   EXPECT_THAT(page_ids, Each(Gt(0)));
//   EXPECT_THAT(page_ids, Not(Each(Gt(4))));
//   page_ids.insert(1);
//   EXPECT_THAT(page_ids, Not(Each(Lt(2))));
//
//   ::std::map<int, size_t> page_lengths;
//   page_lengths[1] = 100;
//   page_lengths[2] = 200;
//   page_lengths[3] = 300;
//   EXPECT_THAT(page_lengths, Not(Each(Pair(1, 100))));
//   EXPECT_THAT(page_lengths, Each(Key(Le(3))));
//
//   const char* user_ids[] = { "joe", "mike", "tom" };
//   EXPECT_THAT(user_ids, Not(Each(Eq(::std::string("tom")))));
template <typename M>
inline internal::EachMatcher<M> Each(M matcher) {
  return internal::EachMatcher<M>(matcher);
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

// Matches the value against the given matcher and explains the match
// result to listener.
template <typename T, typename M>
inline bool ExplainMatchResult(
    M matcher, const T& value, MatchResultListener* listener) {
  return SafeMatcherCast<const T&>(matcher).MatchAndExplain(value, listener);
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
