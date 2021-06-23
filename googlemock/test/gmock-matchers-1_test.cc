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


// Google Mock - a framework for writing C++ mock classes.
//
// This file tests some commonly used argument matchers.

// Silence warning C4244: 'initializing': conversion from 'int' to 'short',
// possible loss of data and C4100, unreferenced local parameter
#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4244)
# pragma warning(disable:4100)
#endif

#include "gmock/gmock-matchers.h"

#include <string.h>
#include <time.h>

#include <array>
#include <cstdint>
#include <deque>
#include <forward_list>
#include <functional>
#include <iostream>
#include <iterator>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "gmock/gmock-more-matchers.h"
#include "gmock/gmock.h"
#include "gtest/gtest-spi.h"
#include "gtest/gtest.h"

namespace testing {
namespace gmock_matchers_1_test {
namespace {

using std::greater;
using std::less;
using std::list;
using std::make_pair;
using std::map;
using std::multimap;
using std::multiset;
using std::ostream;
using std::pair;
using std::set;
using std::stringstream;
using std::vector;
using testing::internal::DummyMatchResultListener;
using testing::internal::ElementMatcherPair;
using testing::internal::ElementMatcherPairs;
using testing::internal::ElementsAreArrayMatcher;
using testing::internal::ExplainMatchFailureTupleTo;
using testing::internal::FloatingEqMatcher;
using testing::internal::FormatMatcherDescription;
using testing::internal::IsReadableTypeName;
using testing::internal::MatchMatrix;
using testing::internal::PredicateFormatterFromMatcher;
using testing::internal::RE;
using testing::internal::StreamMatchResultListener;
using testing::internal::Strings;

// Helper for testing container-valued matchers in mock method context. It is
// important to test matchers in this context, since it requires additional type
// deduction beyond what EXPECT_THAT does, thus making it more restrictive.
struct ContainerHelper {
  MOCK_METHOD1(Call, void(std::vector<std::unique_ptr<int>>));
};

std::vector<std::unique_ptr<int>> MakeUniquePtrs(const std::vector<int>& ints) {
  std::vector<std::unique_ptr<int>> pointers;
  for (int i : ints) pointers.emplace_back(new int(i));
  return pointers;
}

// For testing ExplainMatchResultTo().
class GreaterThanMatcher : public MatcherInterface<int> {
 public:
  explicit GreaterThanMatcher(int rhs) : rhs_(rhs) {}

  void DescribeTo(ostream* os) const override { *os << "is > " << rhs_; }

  bool MatchAndExplain(int lhs, MatchResultListener* listener) const override {
    const int diff = lhs - rhs_;
    if (diff > 0) {
      *listener << "which is " << diff << " more than " << rhs_;
    } else if (diff == 0) {
      *listener << "which is the same as " << rhs_;
    } else {
      *listener << "which is " << -diff << " less than " << rhs_;
    }

    return lhs > rhs_;
  }

 private:
  int rhs_;
};

Matcher<int> GreaterThan(int n) {
  return MakeMatcher(new GreaterThanMatcher(n));
}

std::string OfType(const std::string& type_name) {
#if GTEST_HAS_RTTI
  return IsReadableTypeName(type_name) ? " (of type " + type_name + ")" : "";
#else
  return "";
#endif
}

// Returns the description of the given matcher.
template <typename T>
std::string Describe(const Matcher<T>& m) {
  return DescribeMatcher<T>(m);
}

// Returns the description of the negation of the given matcher.
template <typename T>
std::string DescribeNegation(const Matcher<T>& m) {
  return DescribeMatcher<T>(m, true);
}

// Returns the reason why x matches, or doesn't match, m.
template <typename MatcherType, typename Value>
std::string Explain(const MatcherType& m, const Value& x) {
  StringMatchResultListener listener;
  ExplainMatchResult(m, x, &listener);
  return listener.str();
}

TEST(MonotonicMatcherTest, IsPrintable) {
  stringstream ss;
  ss << GreaterThan(5);
  EXPECT_EQ("is > 5", ss.str());
}

TEST(MatchResultListenerTest, StreamingWorks) {
  StringMatchResultListener listener;
  listener << "hi" << 5;
  EXPECT_EQ("hi5", listener.str());

  listener.Clear();
  EXPECT_EQ("", listener.str());

  listener << 42;
  EXPECT_EQ("42", listener.str());

  // Streaming shouldn't crash when the underlying ostream is NULL.
  DummyMatchResultListener dummy;
  dummy << "hi" << 5;
}

TEST(MatchResultListenerTest, CanAccessUnderlyingStream) {
  EXPECT_TRUE(DummyMatchResultListener().stream() == nullptr);
  EXPECT_TRUE(StreamMatchResultListener(nullptr).stream() == nullptr);

  EXPECT_EQ(&std::cout, StreamMatchResultListener(&std::cout).stream());
}

TEST(MatchResultListenerTest, IsInterestedWorks) {
  EXPECT_TRUE(StringMatchResultListener().IsInterested());
  EXPECT_TRUE(StreamMatchResultListener(&std::cout).IsInterested());

  EXPECT_FALSE(DummyMatchResultListener().IsInterested());
  EXPECT_FALSE(StreamMatchResultListener(nullptr).IsInterested());
}

// Makes sure that the MatcherInterface<T> interface doesn't
// change.
class EvenMatcherImpl : public MatcherInterface<int> {
 public:
  bool MatchAndExplain(int x,
                       MatchResultListener* /* listener */) const override {
    return x % 2 == 0;
  }

  void DescribeTo(ostream* os) const override { *os << "is an even number"; }

  // We deliberately don't define DescribeNegationTo() and
  // ExplainMatchResultTo() here, to make sure the definition of these
  // two methods is optional.
};

// Makes sure that the MatcherInterface API doesn't change.
TEST(MatcherInterfaceTest, CanBeImplementedUsingPublishedAPI) {
  EvenMatcherImpl m;
}

// Tests implementing a monomorphic matcher using MatchAndExplain().

class NewEvenMatcherImpl : public MatcherInterface<int> {
 public:
  bool MatchAndExplain(int x, MatchResultListener* listener) const override {
    const bool match = x % 2 == 0;
    // Verifies that we can stream to a listener directly.
    *listener << "value % " << 2;
    if (listener->stream() != nullptr) {
      // Verifies that we can stream to a listener's underlying stream
      // too.
      *listener->stream() << " == " << (x % 2);
    }
    return match;
  }

  void DescribeTo(ostream* os) const override { *os << "is an even number"; }
};

TEST(MatcherInterfaceTest, CanBeImplementedUsingNewAPI) {
  Matcher<int> m = MakeMatcher(new NewEvenMatcherImpl);
  EXPECT_TRUE(m.Matches(2));
  EXPECT_FALSE(m.Matches(3));
  EXPECT_EQ("value % 2 == 0", Explain(m, 2));
  EXPECT_EQ("value % 2 == 1", Explain(m, 3));
}

// Tests default-constructing a matcher.
TEST(MatcherTest, CanBeDefaultConstructed) {
  Matcher<double> m;
}

// Tests that Matcher<T> can be constructed from a MatcherInterface<T>*.
TEST(MatcherTest, CanBeConstructedFromMatcherInterface) {
  const MatcherInterface<int>* impl = new EvenMatcherImpl;
  Matcher<int> m(impl);
  EXPECT_TRUE(m.Matches(4));
  EXPECT_FALSE(m.Matches(5));
}

// Tests that value can be used in place of Eq(value).
TEST(MatcherTest, CanBeImplicitlyConstructedFromValue) {
  Matcher<int> m1 = 5;
  EXPECT_TRUE(m1.Matches(5));
  EXPECT_FALSE(m1.Matches(6));
}

// Tests that NULL can be used in place of Eq(NULL).
TEST(MatcherTest, CanBeImplicitlyConstructedFromNULL) {
  Matcher<int*> m1 = nullptr;
  EXPECT_TRUE(m1.Matches(nullptr));
  int n = 0;
  EXPECT_FALSE(m1.Matches(&n));
}

// Tests that matchers can be constructed from a variable that is not properly
// defined. This should be illegal, but many users rely on this accidentally.
struct Undefined {
  virtual ~Undefined() = 0;
  static const int kInt = 1;
};

TEST(MatcherTest, CanBeConstructedFromUndefinedVariable) {
  Matcher<int> m1 = Undefined::kInt;
  EXPECT_TRUE(m1.Matches(1));
  EXPECT_FALSE(m1.Matches(2));
}

// Test that a matcher parameterized with an abstract class compiles.
TEST(MatcherTest, CanAcceptAbstractClass) { Matcher<const Undefined&> m = _; }

// Tests that matchers are copyable.
TEST(MatcherTest, IsCopyable) {
  // Tests the copy constructor.
  Matcher<bool> m1 = Eq(false);
  EXPECT_TRUE(m1.Matches(false));
  EXPECT_FALSE(m1.Matches(true));

  // Tests the assignment operator.
  m1 = Eq(true);
  EXPECT_TRUE(m1.Matches(true));
  EXPECT_FALSE(m1.Matches(false));
}

// Tests that Matcher<T>::DescribeTo() calls
// MatcherInterface<T>::DescribeTo().
TEST(MatcherTest, CanDescribeItself) {
  EXPECT_EQ("is an even number",
            Describe(Matcher<int>(new EvenMatcherImpl)));
}

// Tests Matcher<T>::MatchAndExplain().
TEST(MatcherTest, MatchAndExplain) {
  Matcher<int> m = GreaterThan(0);
  StringMatchResultListener listener1;
  EXPECT_TRUE(m.MatchAndExplain(42, &listener1));
  EXPECT_EQ("which is 42 more than 0", listener1.str());

  StringMatchResultListener listener2;
  EXPECT_FALSE(m.MatchAndExplain(-9, &listener2));
  EXPECT_EQ("which is 9 less than 0", listener2.str());
}

// Tests that a C-string literal can be implicitly converted to a
// Matcher<std::string> or Matcher<const std::string&>.
TEST(StringMatcherTest, CanBeImplicitlyConstructedFromCStringLiteral) {
  Matcher<std::string> m1 = "hi";
  EXPECT_TRUE(m1.Matches("hi"));
  EXPECT_FALSE(m1.Matches("hello"));

  Matcher<const std::string&> m2 = "hi";
  EXPECT_TRUE(m2.Matches("hi"));
  EXPECT_FALSE(m2.Matches("hello"));
}

// Tests that a string object can be implicitly converted to a
// Matcher<std::string> or Matcher<const std::string&>.
TEST(StringMatcherTest, CanBeImplicitlyConstructedFromString) {
  Matcher<std::string> m1 = std::string("hi");
  EXPECT_TRUE(m1.Matches("hi"));
  EXPECT_FALSE(m1.Matches("hello"));

  Matcher<const std::string&> m2 = std::string("hi");
  EXPECT_TRUE(m2.Matches("hi"));
  EXPECT_FALSE(m2.Matches("hello"));
}

#if GTEST_INTERNAL_HAS_STRING_VIEW
// Tests that a C-string literal can be implicitly converted to a
// Matcher<StringView> or Matcher<const StringView&>.
TEST(StringViewMatcherTest, CanBeImplicitlyConstructedFromCStringLiteral) {
  Matcher<internal::StringView> m1 = "cats";
  EXPECT_TRUE(m1.Matches("cats"));
  EXPECT_FALSE(m1.Matches("dogs"));

  Matcher<const internal::StringView&> m2 = "cats";
  EXPECT_TRUE(m2.Matches("cats"));
  EXPECT_FALSE(m2.Matches("dogs"));
}

// Tests that a std::string object can be implicitly converted to a
// Matcher<StringView> or Matcher<const StringView&>.
TEST(StringViewMatcherTest, CanBeImplicitlyConstructedFromString) {
  Matcher<internal::StringView> m1 = std::string("cats");
  EXPECT_TRUE(m1.Matches("cats"));
  EXPECT_FALSE(m1.Matches("dogs"));

  Matcher<const internal::StringView&> m2 = std::string("cats");
  EXPECT_TRUE(m2.Matches("cats"));
  EXPECT_FALSE(m2.Matches("dogs"));
}

// Tests that a StringView object can be implicitly converted to a
// Matcher<StringView> or Matcher<const StringView&>.
TEST(StringViewMatcherTest, CanBeImplicitlyConstructedFromStringView) {
  Matcher<internal::StringView> m1 = internal::StringView("cats");
  EXPECT_TRUE(m1.Matches("cats"));
  EXPECT_FALSE(m1.Matches("dogs"));

  Matcher<const internal::StringView&> m2 = internal::StringView("cats");
  EXPECT_TRUE(m2.Matches("cats"));
  EXPECT_FALSE(m2.Matches("dogs"));
}
#endif  // GTEST_INTERNAL_HAS_STRING_VIEW

// Tests that a std::reference_wrapper<std::string> object can be implicitly
// converted to a Matcher<std::string> or Matcher<const std::string&> via Eq().
TEST(StringMatcherTest,
     CanBeImplicitlyConstructedFromEqReferenceWrapperString) {
  std::string value = "cats";
  Matcher<std::string> m1 = Eq(std::ref(value));
  EXPECT_TRUE(m1.Matches("cats"));
  EXPECT_FALSE(m1.Matches("dogs"));

  Matcher<const std::string&> m2 = Eq(std::ref(value));
  EXPECT_TRUE(m2.Matches("cats"));
  EXPECT_FALSE(m2.Matches("dogs"));
}

// Tests that MakeMatcher() constructs a Matcher<T> from a
// MatcherInterface* without requiring the user to explicitly
// write the type.
TEST(MakeMatcherTest, ConstructsMatcherFromMatcherInterface) {
  const MatcherInterface<int>* dummy_impl = new EvenMatcherImpl;
  Matcher<int> m = MakeMatcher(dummy_impl);
}

// Tests that MakePolymorphicMatcher() can construct a polymorphic
// matcher from its implementation using the old API.
const int g_bar = 1;
class ReferencesBarOrIsZeroImpl {
 public:
  template <typename T>
  bool MatchAndExplain(const T& x,
                       MatchResultListener* /* listener */) const {
    const void* p = &x;
    return p == &g_bar || x == 0;
  }

  void DescribeTo(ostream* os) const { *os << "g_bar or zero"; }

  void DescribeNegationTo(ostream* os) const {
    *os << "doesn't reference g_bar and is not zero";
  }
};

// This function verifies that MakePolymorphicMatcher() returns a
// PolymorphicMatcher<T> where T is the argument's type.
PolymorphicMatcher<ReferencesBarOrIsZeroImpl> ReferencesBarOrIsZero() {
  return MakePolymorphicMatcher(ReferencesBarOrIsZeroImpl());
}

TEST(MakePolymorphicMatcherTest, ConstructsMatcherUsingOldAPI) {
  // Using a polymorphic matcher to match a reference type.
  Matcher<const int&> m1 = ReferencesBarOrIsZero();
  EXPECT_TRUE(m1.Matches(0));
  // Verifies that the identity of a by-reference argument is preserved.
  EXPECT_TRUE(m1.Matches(g_bar));
  EXPECT_FALSE(m1.Matches(1));
  EXPECT_EQ("g_bar or zero", Describe(m1));

  // Using a polymorphic matcher to match a value type.
  Matcher<double> m2 = ReferencesBarOrIsZero();
  EXPECT_TRUE(m2.Matches(0.0));
  EXPECT_FALSE(m2.Matches(0.1));
  EXPECT_EQ("g_bar or zero", Describe(m2));
}

// Tests implementing a polymorphic matcher using MatchAndExplain().

class PolymorphicIsEvenImpl {
 public:
  void DescribeTo(ostream* os) const { *os << "is even"; }

  void DescribeNegationTo(ostream* os) const {
    *os << "is odd";
  }

  template <typename T>
  bool MatchAndExplain(const T& x, MatchResultListener* listener) const {
    // Verifies that we can stream to the listener directly.
    *listener << "% " << 2;
    if (listener->stream() != nullptr) {
      // Verifies that we can stream to the listener's underlying stream
      // too.
      *listener->stream() << " == " << (x % 2);
    }
    return (x % 2) == 0;
  }
};

PolymorphicMatcher<PolymorphicIsEvenImpl> PolymorphicIsEven() {
  return MakePolymorphicMatcher(PolymorphicIsEvenImpl());
}

TEST(MakePolymorphicMatcherTest, ConstructsMatcherUsingNewAPI) {
  // Using PolymorphicIsEven() as a Matcher<int>.
  const Matcher<int> m1 = PolymorphicIsEven();
  EXPECT_TRUE(m1.Matches(42));
  EXPECT_FALSE(m1.Matches(43));
  EXPECT_EQ("is even", Describe(m1));

  const Matcher<int> not_m1 = Not(m1);
  EXPECT_EQ("is odd", Describe(not_m1));

  EXPECT_EQ("% 2 == 0", Explain(m1, 42));

  // Using PolymorphicIsEven() as a Matcher<char>.
  const Matcher<char> m2 = PolymorphicIsEven();
  EXPECT_TRUE(m2.Matches('\x42'));
  EXPECT_FALSE(m2.Matches('\x43'));
  EXPECT_EQ("is even", Describe(m2));

  const Matcher<char> not_m2 = Not(m2);
  EXPECT_EQ("is odd", Describe(not_m2));

  EXPECT_EQ("% 2 == 0", Explain(m2, '\x42'));
}

// Tests that MatcherCast<T>(m) works when m is a polymorphic matcher.
TEST(MatcherCastTest, FromPolymorphicMatcher) {
  Matcher<int> m = MatcherCast<int>(Eq(5));
  EXPECT_TRUE(m.Matches(5));
  EXPECT_FALSE(m.Matches(6));
}

// For testing casting matchers between compatible types.
class IntValue {
 public:
  // An int can be statically (although not implicitly) cast to a
  // IntValue.
  explicit IntValue(int a_value) : value_(a_value) {}

  int value() const { return value_; }
 private:
  int value_;
};

// For testing casting matchers between compatible types.
bool IsPositiveIntValue(const IntValue& foo) {
  return foo.value() > 0;
}

// Tests that MatcherCast<T>(m) works when m is a Matcher<U> where T
// can be statically converted to U.
TEST(MatcherCastTest, FromCompatibleType) {
  Matcher<double> m1 = Eq(2.0);
  Matcher<int> m2 = MatcherCast<int>(m1);
  EXPECT_TRUE(m2.Matches(2));
  EXPECT_FALSE(m2.Matches(3));

  Matcher<IntValue> m3 = Truly(IsPositiveIntValue);
  Matcher<int> m4 = MatcherCast<int>(m3);
  // In the following, the arguments 1 and 0 are statically converted
  // to IntValue objects, and then tested by the IsPositiveIntValue()
  // predicate.
  EXPECT_TRUE(m4.Matches(1));
  EXPECT_FALSE(m4.Matches(0));
}

// Tests that MatcherCast<T>(m) works when m is a Matcher<const T&>.
TEST(MatcherCastTest, FromConstReferenceToNonReference) {
  Matcher<const int&> m1 = Eq(0);
  Matcher<int> m2 = MatcherCast<int>(m1);
  EXPECT_TRUE(m2.Matches(0));
  EXPECT_FALSE(m2.Matches(1));
}

// Tests that MatcherCast<T>(m) works when m is a Matcher<T&>.
TEST(MatcherCastTest, FromReferenceToNonReference) {
  Matcher<int&> m1 = Eq(0);
  Matcher<int> m2 = MatcherCast<int>(m1);
  EXPECT_TRUE(m2.Matches(0));
  EXPECT_FALSE(m2.Matches(1));
}

// Tests that MatcherCast<const T&>(m) works when m is a Matcher<T>.
TEST(MatcherCastTest, FromNonReferenceToConstReference) {
  Matcher<int> m1 = Eq(0);
  Matcher<const int&> m2 = MatcherCast<const int&>(m1);
  EXPECT_TRUE(m2.Matches(0));
  EXPECT_FALSE(m2.Matches(1));
}

// Tests that MatcherCast<T&>(m) works when m is a Matcher<T>.
TEST(MatcherCastTest, FromNonReferenceToReference) {
  Matcher<int> m1 = Eq(0);
  Matcher<int&> m2 = MatcherCast<int&>(m1);
  int n = 0;
  EXPECT_TRUE(m2.Matches(n));
  n = 1;
  EXPECT_FALSE(m2.Matches(n));
}

// Tests that MatcherCast<T>(m) works when m is a Matcher<T>.
TEST(MatcherCastTest, FromSameType) {
  Matcher<int> m1 = Eq(0);
  Matcher<int> m2 = MatcherCast<int>(m1);
  EXPECT_TRUE(m2.Matches(0));
  EXPECT_FALSE(m2.Matches(1));
}

// Tests that MatcherCast<T>(m) works when m is a value of the same type as the
// value type of the Matcher.
TEST(MatcherCastTest, FromAValue) {
  Matcher<int> m = MatcherCast<int>(42);
  EXPECT_TRUE(m.Matches(42));
  EXPECT_FALSE(m.Matches(239));
}

// Tests that MatcherCast<T>(m) works when m is a value of the type implicitly
// convertible to the value type of the Matcher.
TEST(MatcherCastTest, FromAnImplicitlyConvertibleValue) {
  const int kExpected = 'c';
  Matcher<int> m = MatcherCast<int>('c');
  EXPECT_TRUE(m.Matches(kExpected));
  EXPECT_FALSE(m.Matches(kExpected + 1));
}

struct NonImplicitlyConstructibleTypeWithOperatorEq {
  friend bool operator==(
      const NonImplicitlyConstructibleTypeWithOperatorEq& /* ignored */,
      int rhs) {
    return 42 == rhs;
  }
  friend bool operator==(
      int lhs,
      const NonImplicitlyConstructibleTypeWithOperatorEq& /* ignored */) {
    return lhs == 42;
  }
};

// Tests that MatcherCast<T>(m) works when m is a neither a matcher nor
// implicitly convertible to the value type of the Matcher, but the value type
// of the matcher has operator==() overload accepting m.
TEST(MatcherCastTest, NonImplicitlyConstructibleTypeWithOperatorEq) {
  Matcher<NonImplicitlyConstructibleTypeWithOperatorEq> m1 =
      MatcherCast<NonImplicitlyConstructibleTypeWithOperatorEq>(42);
  EXPECT_TRUE(m1.Matches(NonImplicitlyConstructibleTypeWithOperatorEq()));

  Matcher<NonImplicitlyConstructibleTypeWithOperatorEq> m2 =
      MatcherCast<NonImplicitlyConstructibleTypeWithOperatorEq>(239);
  EXPECT_FALSE(m2.Matches(NonImplicitlyConstructibleTypeWithOperatorEq()));

  // When updating the following lines please also change the comment to
  // namespace convertible_from_any.
  Matcher<int> m3 =
      MatcherCast<int>(NonImplicitlyConstructibleTypeWithOperatorEq());
  EXPECT_TRUE(m3.Matches(42));
  EXPECT_FALSE(m3.Matches(239));
}

// ConvertibleFromAny does not work with MSVC. resulting in
// error C2440: 'initializing': cannot convert from 'Eq' to 'M'
// No constructor could take the source type, or constructor overload
// resolution was ambiguous

#if !defined _MSC_VER

// The below ConvertibleFromAny struct is implicitly constructible from anything
// and when in the same namespace can interact with other tests. In particular,
// if it is in the same namespace as other tests and one removes
//   NonImplicitlyConstructibleTypeWithOperatorEq::operator==(int lhs, ...);
// then the corresponding test still compiles (and it should not!) by implicitly
// converting NonImplicitlyConstructibleTypeWithOperatorEq to ConvertibleFromAny
// in m3.Matcher().
namespace convertible_from_any {
// Implicitly convertible from any type.
struct ConvertibleFromAny {
  ConvertibleFromAny(int a_value) : value(a_value) {}
  template <typename T>
  ConvertibleFromAny(const T& /*a_value*/) : value(-1) {
    ADD_FAILURE() << "Conversion constructor called";
  }
  int value;
};

bool operator==(const ConvertibleFromAny& a, const ConvertibleFromAny& b) {
  return a.value == b.value;
}

ostream& operator<<(ostream& os, const ConvertibleFromAny& a) {
  return os << a.value;
}

TEST(MatcherCastTest, ConversionConstructorIsUsed) {
  Matcher<ConvertibleFromAny> m = MatcherCast<ConvertibleFromAny>(1);
  EXPECT_TRUE(m.Matches(ConvertibleFromAny(1)));
  EXPECT_FALSE(m.Matches(ConvertibleFromAny(2)));
}

TEST(MatcherCastTest, FromConvertibleFromAny) {
  Matcher<ConvertibleFromAny> m =
      MatcherCast<ConvertibleFromAny>(Eq(ConvertibleFromAny(1)));
  EXPECT_TRUE(m.Matches(ConvertibleFromAny(1)));
  EXPECT_FALSE(m.Matches(ConvertibleFromAny(2)));
}
}  // namespace convertible_from_any

#endif  // !defined _MSC_VER

struct IntReferenceWrapper {
  IntReferenceWrapper(const int& a_value) : value(&a_value) {}
  const int* value;
};

bool operator==(const IntReferenceWrapper& a, const IntReferenceWrapper& b) {
  return a.value == b.value;
}

TEST(MatcherCastTest, ValueIsNotCopied) {
  int n = 42;
  Matcher<IntReferenceWrapper> m = MatcherCast<IntReferenceWrapper>(n);
  // Verify that the matcher holds a reference to n, not to its temporary copy.
  EXPECT_TRUE(m.Matches(n));
}

class Base {
 public:
  virtual ~Base() {}
  Base() {}
 private:
  GTEST_DISALLOW_COPY_AND_ASSIGN_(Base);
};

class Derived : public Base {
 public:
  Derived() : Base() {}
  int i;
};

class OtherDerived : public Base {};

// Tests that SafeMatcherCast<T>(m) works when m is a polymorphic matcher.
TEST(SafeMatcherCastTest, FromPolymorphicMatcher) {
  Matcher<char> m2 = SafeMatcherCast<char>(Eq(32));
  EXPECT_TRUE(m2.Matches(' '));
  EXPECT_FALSE(m2.Matches('\n'));
}

// Tests that SafeMatcherCast<T>(m) works when m is a Matcher<U> where
// T and U are arithmetic types and T can be losslessly converted to
// U.
TEST(SafeMatcherCastTest, FromLosslesslyConvertibleArithmeticType) {
  Matcher<double> m1 = DoubleEq(1.0);
  Matcher<float> m2 = SafeMatcherCast<float>(m1);
  EXPECT_TRUE(m2.Matches(1.0f));
  EXPECT_FALSE(m2.Matches(2.0f));

  Matcher<char> m3 = SafeMatcherCast<char>(TypedEq<int>('a'));
  EXPECT_TRUE(m3.Matches('a'));
  EXPECT_FALSE(m3.Matches('b'));
}

// Tests that SafeMatcherCast<T>(m) works when m is a Matcher<U> where T and U
// are pointers or references to a derived and a base class, correspondingly.
TEST(SafeMatcherCastTest, FromBaseClass) {
  Derived d, d2;
  Matcher<Base*> m1 = Eq(&d);
  Matcher<Derived*> m2 = SafeMatcherCast<Derived*>(m1);
  EXPECT_TRUE(m2.Matches(&d));
  EXPECT_FALSE(m2.Matches(&d2));

  Matcher<Base&> m3 = Ref(d);
  Matcher<Derived&> m4 = SafeMatcherCast<Derived&>(m3);
  EXPECT_TRUE(m4.Matches(d));
  EXPECT_FALSE(m4.Matches(d2));
}

// Tests that SafeMatcherCast<T&>(m) works when m is a Matcher<const T&>.
TEST(SafeMatcherCastTest, FromConstReferenceToReference) {
  int n = 0;
  Matcher<const int&> m1 = Ref(n);
  Matcher<int&> m2 = SafeMatcherCast<int&>(m1);
  int n1 = 0;
  EXPECT_TRUE(m2.Matches(n));
  EXPECT_FALSE(m2.Matches(n1));
}

// Tests that MatcherCast<const T&>(m) works when m is a Matcher<T>.
TEST(SafeMatcherCastTest, FromNonReferenceToConstReference) {
  Matcher<std::unique_ptr<int>> m1 = IsNull();
  Matcher<const std::unique_ptr<int>&> m2 =
      SafeMatcherCast<const std::unique_ptr<int>&>(m1);
  EXPECT_TRUE(m2.Matches(std::unique_ptr<int>()));
  EXPECT_FALSE(m2.Matches(std::unique_ptr<int>(new int)));
}

// Tests that SafeMatcherCast<T&>(m) works when m is a Matcher<T>.
TEST(SafeMatcherCastTest, FromNonReferenceToReference) {
  Matcher<int> m1 = Eq(0);
  Matcher<int&> m2 = SafeMatcherCast<int&>(m1);
  int n = 0;
  EXPECT_TRUE(m2.Matches(n));
  n = 1;
  EXPECT_FALSE(m2.Matches(n));
}

// Tests that SafeMatcherCast<T>(m) works when m is a Matcher<T>.
TEST(SafeMatcherCastTest, FromSameType) {
  Matcher<int> m1 = Eq(0);
  Matcher<int> m2 = SafeMatcherCast<int>(m1);
  EXPECT_TRUE(m2.Matches(0));
  EXPECT_FALSE(m2.Matches(1));
}

#if !defined _MSC_VER

namespace convertible_from_any {
TEST(SafeMatcherCastTest, ConversionConstructorIsUsed) {
  Matcher<ConvertibleFromAny> m = SafeMatcherCast<ConvertibleFromAny>(1);
  EXPECT_TRUE(m.Matches(ConvertibleFromAny(1)));
  EXPECT_FALSE(m.Matches(ConvertibleFromAny(2)));
}

TEST(SafeMatcherCastTest, FromConvertibleFromAny) {
  Matcher<ConvertibleFromAny> m =
      SafeMatcherCast<ConvertibleFromAny>(Eq(ConvertibleFromAny(1)));
  EXPECT_TRUE(m.Matches(ConvertibleFromAny(1)));
  EXPECT_FALSE(m.Matches(ConvertibleFromAny(2)));
}
}  // namespace convertible_from_any

#endif  // !defined _MSC_VER

TEST(SafeMatcherCastTest, ValueIsNotCopied) {
  int n = 42;
  Matcher<IntReferenceWrapper> m = SafeMatcherCast<IntReferenceWrapper>(n);
  // Verify that the matcher holds a reference to n, not to its temporary copy.
  EXPECT_TRUE(m.Matches(n));
}

TEST(ExpectThat, TakesLiterals) {
  EXPECT_THAT(1, 1);
  EXPECT_THAT(1.0, 1.0);
  EXPECT_THAT(std::string(), "");
}

TEST(ExpectThat, TakesFunctions) {
  struct Helper {
    static void Func() {}
  };
  void (*func)() = Helper::Func;
  EXPECT_THAT(func, Helper::Func);
  EXPECT_THAT(func, &Helper::Func);
}

// Tests that A<T>() matches any value of type T.
TEST(ATest, MatchesAnyValue) {
  // Tests a matcher for a value type.
  Matcher<double> m1 = A<double>();
  EXPECT_TRUE(m1.Matches(91.43));
  EXPECT_TRUE(m1.Matches(-15.32));

  // Tests a matcher for a reference type.
  int a = 2;
  int b = -6;
  Matcher<int&> m2 = A<int&>();
  EXPECT_TRUE(m2.Matches(a));
  EXPECT_TRUE(m2.Matches(b));
}

TEST(ATest, WorksForDerivedClass) {
  Base base;
  Derived derived;
  EXPECT_THAT(&base, A<Base*>());
  // This shouldn't compile: EXPECT_THAT(&base, A<Derived*>());
  EXPECT_THAT(&derived, A<Base*>());
  EXPECT_THAT(&derived, A<Derived*>());
}

// Tests that A<T>() describes itself properly.
TEST(ATest, CanDescribeSelf) {
  EXPECT_EQ("is anything", Describe(A<bool>()));
}

// Tests that An<T>() matches any value of type T.
TEST(AnTest, MatchesAnyValue) {
  // Tests a matcher for a value type.
  Matcher<int> m1 = An<int>();
  EXPECT_TRUE(m1.Matches(9143));
  EXPECT_TRUE(m1.Matches(-1532));

  // Tests a matcher for a reference type.
  int a = 2;
  int b = -6;
  Matcher<int&> m2 = An<int&>();
  EXPECT_TRUE(m2.Matches(a));
  EXPECT_TRUE(m2.Matches(b));
}

// Tests that An<T>() describes itself properly.
TEST(AnTest, CanDescribeSelf) {
  EXPECT_EQ("is anything", Describe(An<int>()));
}

// Tests that _ can be used as a matcher for any type and matches any
// value of that type.
TEST(UnderscoreTest, MatchesAnyValue) {
  // Uses _ as a matcher for a value type.
  Matcher<int> m1 = _;
  EXPECT_TRUE(m1.Matches(123));
  EXPECT_TRUE(m1.Matches(-242));

  // Uses _ as a matcher for a reference type.
  bool a = false;
  const bool b = true;
  Matcher<const bool&> m2 = _;
  EXPECT_TRUE(m2.Matches(a));
  EXPECT_TRUE(m2.Matches(b));
}

// Tests that _ describes itself properly.
TEST(UnderscoreTest, CanDescribeSelf) {
  Matcher<int> m = _;
  EXPECT_EQ("is anything", Describe(m));
}

// Tests that Eq(x) matches any value equal to x.
TEST(EqTest, MatchesEqualValue) {
  // 2 C-strings with same content but different addresses.
  const char a1[] = "hi";
  const char a2[] = "hi";

  Matcher<const char*> m1 = Eq(a1);
  EXPECT_TRUE(m1.Matches(a1));
  EXPECT_FALSE(m1.Matches(a2));
}

// Tests that Eq(v) describes itself properly.

class Unprintable {
 public:
  Unprintable() : c_('a') {}

  bool operator==(const Unprintable& /* rhs */) const { return true; }
  // -Wunused-private-field: dummy accessor for `c_`.
  char dummy_c() { return c_; }
 private:
  char c_;
};

TEST(EqTest, CanDescribeSelf) {
  Matcher<Unprintable> m = Eq(Unprintable());
  EXPECT_EQ("is equal to 1-byte object <61>", Describe(m));
}

// Tests that Eq(v) can be used to match any type that supports
// comparing with type T, where T is v's type.
TEST(EqTest, IsPolymorphic) {
  Matcher<int> m1 = Eq(1);
  EXPECT_TRUE(m1.Matches(1));
  EXPECT_FALSE(m1.Matches(2));

  Matcher<char> m2 = Eq(1);
  EXPECT_TRUE(m2.Matches('\1'));
  EXPECT_FALSE(m2.Matches('a'));
}

// Tests that TypedEq<T>(v) matches values of type T that's equal to v.
TEST(TypedEqTest, ChecksEqualityForGivenType) {
  Matcher<char> m1 = TypedEq<char>('a');
  EXPECT_TRUE(m1.Matches('a'));
  EXPECT_FALSE(m1.Matches('b'));

  Matcher<int> m2 = TypedEq<int>(6);
  EXPECT_TRUE(m2.Matches(6));
  EXPECT_FALSE(m2.Matches(7));
}

// Tests that TypedEq(v) describes itself properly.
TEST(TypedEqTest, CanDescribeSelf) {
  EXPECT_EQ("is equal to 2", Describe(TypedEq<int>(2)));
}

// Tests that TypedEq<T>(v) has type Matcher<T>.

// Type<T>::IsTypeOf(v) compiles if and only if the type of value v is T, where
// T is a "bare" type (i.e. not in the form of const U or U&).  If v's type is
// not T, the compiler will generate a message about "undefined reference".
template <typename T>
struct Type {
  static bool IsTypeOf(const T& /* v */) { return true; }

  template <typename T2>
  static void IsTypeOf(T2 v);
};

TEST(TypedEqTest, HasSpecifiedType) {
  // Verfies that the type of TypedEq<T>(v) is Matcher<T>.
  Type<Matcher<int> >::IsTypeOf(TypedEq<int>(5));
  Type<Matcher<double> >::IsTypeOf(TypedEq<double>(5));
}

// Tests that Ge(v) matches anything >= v.
TEST(GeTest, ImplementsGreaterThanOrEqual) {
  Matcher<int> m1 = Ge(0);
  EXPECT_TRUE(m1.Matches(1));
  EXPECT_TRUE(m1.Matches(0));
  EXPECT_FALSE(m1.Matches(-1));
}

// Tests that Ge(v) describes itself properly.
TEST(GeTest, CanDescribeSelf) {
  Matcher<int> m = Ge(5);
  EXPECT_EQ("is >= 5", Describe(m));
}

// Tests that Gt(v) matches anything > v.
TEST(GtTest, ImplementsGreaterThan) {
  Matcher<double> m1 = Gt(0);
  EXPECT_TRUE(m1.Matches(1.0));
  EXPECT_FALSE(m1.Matches(0.0));
  EXPECT_FALSE(m1.Matches(-1.0));
}

// Tests that Gt(v) describes itself properly.
TEST(GtTest, CanDescribeSelf) {
  Matcher<int> m = Gt(5);
  EXPECT_EQ("is > 5", Describe(m));
}

// Tests that Le(v) matches anything <= v.
TEST(LeTest, ImplementsLessThanOrEqual) {
  Matcher<char> m1 = Le('b');
  EXPECT_TRUE(m1.Matches('a'));
  EXPECT_TRUE(m1.Matches('b'));
  EXPECT_FALSE(m1.Matches('c'));
}

// Tests that Le(v) describes itself properly.
TEST(LeTest, CanDescribeSelf) {
  Matcher<int> m = Le(5);
  EXPECT_EQ("is <= 5", Describe(m));
}

// Tests that Lt(v) matches anything < v.
TEST(LtTest, ImplementsLessThan) {
  Matcher<const std::string&> m1 = Lt("Hello");
  EXPECT_TRUE(m1.Matches("Abc"));
  EXPECT_FALSE(m1.Matches("Hello"));
  EXPECT_FALSE(m1.Matches("Hello, world!"));
}

// Tests that Lt(v) describes itself properly.
TEST(LtTest, CanDescribeSelf) {
  Matcher<int> m = Lt(5);
  EXPECT_EQ("is < 5", Describe(m));
}

// Tests that Ne(v) matches anything != v.
TEST(NeTest, ImplementsNotEqual) {
  Matcher<int> m1 = Ne(0);
  EXPECT_TRUE(m1.Matches(1));
  EXPECT_TRUE(m1.Matches(-1));
  EXPECT_FALSE(m1.Matches(0));
}

// Tests that Ne(v) describes itself properly.
TEST(NeTest, CanDescribeSelf) {
  Matcher<int> m = Ne(5);
  EXPECT_EQ("isn't equal to 5", Describe(m));
}

class MoveOnly {
 public:
  explicit MoveOnly(int i) : i_(i) {}
  MoveOnly(const MoveOnly&) = delete;
  MoveOnly(MoveOnly&&) = default;
  MoveOnly& operator=(const MoveOnly&) = delete;
  MoveOnly& operator=(MoveOnly&&) = default;

  bool operator==(const MoveOnly& other) const { return i_ == other.i_; }
  bool operator!=(const MoveOnly& other) const { return i_ != other.i_; }
  bool operator<(const MoveOnly& other) const { return i_ < other.i_; }
  bool operator<=(const MoveOnly& other) const { return i_ <= other.i_; }
  bool operator>(const MoveOnly& other) const { return i_ > other.i_; }
  bool operator>=(const MoveOnly& other) const { return i_ >= other.i_; }

 private:
  int i_;
};

struct MoveHelper {
  MOCK_METHOD1(Call, void(MoveOnly));
};

// Disable this test in VS 2015 (version 14), where it fails when SEH is enabled
#if defined(_MSC_VER) && (_MSC_VER < 1910)
TEST(ComparisonBaseTest, DISABLED_WorksWithMoveOnly) {
#else
TEST(ComparisonBaseTest, WorksWithMoveOnly) {
#endif
  MoveOnly m{0};
  MoveHelper helper;

  EXPECT_CALL(helper, Call(Eq(ByRef(m))));
  helper.Call(MoveOnly(0));
  EXPECT_CALL(helper, Call(Ne(ByRef(m))));
  helper.Call(MoveOnly(1));
  EXPECT_CALL(helper, Call(Le(ByRef(m))));
  helper.Call(MoveOnly(0));
  EXPECT_CALL(helper, Call(Lt(ByRef(m))));
  helper.Call(MoveOnly(-1));
  EXPECT_CALL(helper, Call(Ge(ByRef(m))));
  helper.Call(MoveOnly(0));
  EXPECT_CALL(helper, Call(Gt(ByRef(m))));
  helper.Call(MoveOnly(1));
}

// Tests that IsNull() matches any NULL pointer of any type.
TEST(IsNullTest, MatchesNullPointer) {
  Matcher<int*> m1 = IsNull();
  int* p1 = nullptr;
  int n = 0;
  EXPECT_TRUE(m1.Matches(p1));
  EXPECT_FALSE(m1.Matches(&n));

  Matcher<const char*> m2 = IsNull();
  const char* p2 = nullptr;
  EXPECT_TRUE(m2.Matches(p2));
  EXPECT_FALSE(m2.Matches("hi"));

  Matcher<void*> m3 = IsNull();
  void* p3 = nullptr;
  EXPECT_TRUE(m3.Matches(p3));
  EXPECT_FALSE(m3.Matches(reinterpret_cast<void*>(0xbeef)));
}

TEST(IsNullTest, StdFunction) {
  const Matcher<std::function<void()>> m = IsNull();

  EXPECT_TRUE(m.Matches(std::function<void()>()));
  EXPECT_FALSE(m.Matches([]{}));
}

// Tests that IsNull() describes itself properly.
TEST(IsNullTest, CanDescribeSelf) {
  Matcher<int*> m = IsNull();
  EXPECT_EQ("is NULL", Describe(m));
  EXPECT_EQ("isn't NULL", DescribeNegation(m));
}

// Tests that NotNull() matches any non-NULL pointer of any type.
TEST(NotNullTest, MatchesNonNullPointer) {
  Matcher<int*> m1 = NotNull();
  int* p1 = nullptr;
  int n = 0;
  EXPECT_FALSE(m1.Matches(p1));
  EXPECT_TRUE(m1.Matches(&n));

  Matcher<const char*> m2 = NotNull();
  const char* p2 = nullptr;
  EXPECT_FALSE(m2.Matches(p2));
  EXPECT_TRUE(m2.Matches("hi"));
}

TEST(NotNullTest, LinkedPtr) {
  const Matcher<std::shared_ptr<int>> m = NotNull();
  const std::shared_ptr<int> null_p;
  const std::shared_ptr<int> non_null_p(new int);

  EXPECT_FALSE(m.Matches(null_p));
  EXPECT_TRUE(m.Matches(non_null_p));
}

TEST(NotNullTest, ReferenceToConstLinkedPtr) {
  const Matcher<const std::shared_ptr<double>&> m = NotNull();
  const std::shared_ptr<double> null_p;
  const std::shared_ptr<double> non_null_p(new double);

  EXPECT_FALSE(m.Matches(null_p));
  EXPECT_TRUE(m.Matches(non_null_p));
}

TEST(NotNullTest, StdFunction) {
  const Matcher<std::function<void()>> m = NotNull();

  EXPECT_TRUE(m.Matches([]{}));
  EXPECT_FALSE(m.Matches(std::function<void()>()));
}

// Tests that NotNull() describes itself properly.
TEST(NotNullTest, CanDescribeSelf) {
  Matcher<int*> m = NotNull();
  EXPECT_EQ("isn't NULL", Describe(m));
}

// Tests that Ref(variable) matches an argument that references
// 'variable'.
TEST(RefTest, MatchesSameVariable) {
  int a = 0;
  int b = 0;
  Matcher<int&> m = Ref(a);
  EXPECT_TRUE(m.Matches(a));
  EXPECT_FALSE(m.Matches(b));
}

// Tests that Ref(variable) describes itself properly.
TEST(RefTest, CanDescribeSelf) {
  int n = 5;
  Matcher<int&> m = Ref(n);
  stringstream ss;
  ss << "references the variable @" << &n << " 5";
  EXPECT_EQ(ss.str(), Describe(m));
}

// Test that Ref(non_const_varialbe) can be used as a matcher for a
// const reference.
TEST(RefTest, CanBeUsedAsMatcherForConstReference) {
  int a = 0;
  int b = 0;
  Matcher<const int&> m = Ref(a);
  EXPECT_TRUE(m.Matches(a));
  EXPECT_FALSE(m.Matches(b));
}

// Tests that Ref(variable) is covariant, i.e. Ref(derived) can be
// used wherever Ref(base) can be used (Ref(derived) is a sub-type
// of Ref(base), but not vice versa.

TEST(RefTest, IsCovariant) {
  Base base, base2;
  Derived derived;
  Matcher<const Base&> m1 = Ref(base);
  EXPECT_TRUE(m1.Matches(base));
  EXPECT_FALSE(m1.Matches(base2));
  EXPECT_FALSE(m1.Matches(derived));

  m1 = Ref(derived);
  EXPECT_TRUE(m1.Matches(derived));
  EXPECT_FALSE(m1.Matches(base));
  EXPECT_FALSE(m1.Matches(base2));
}

TEST(RefTest, ExplainsResult) {
  int n = 0;
  EXPECT_THAT(Explain(Matcher<const int&>(Ref(n)), n),
              StartsWith("which is located @"));

  int m = 0;
  EXPECT_THAT(Explain(Matcher<const int&>(Ref(n)), m),
              StartsWith("which is located @"));
}

// Tests string comparison matchers.

template <typename T = std::string>
std::string FromStringLike(internal::StringLike<T> str) {
  return std::string(str);
}

TEST(StringLike, TestConversions) {
  EXPECT_EQ("foo", FromStringLike("foo"));
  EXPECT_EQ("foo", FromStringLike(std::string("foo")));
#if GTEST_INTERNAL_HAS_STRING_VIEW
  EXPECT_EQ("foo", FromStringLike(internal::StringView("foo")));
#endif  // GTEST_INTERNAL_HAS_STRING_VIEW

  // Non deducible types.
  EXPECT_EQ("", FromStringLike({}));
  EXPECT_EQ("foo", FromStringLike({'f', 'o', 'o'}));
  const char buf[] = "foo";
  EXPECT_EQ("foo", FromStringLike({buf, buf + 3}));
}

TEST(StrEqTest, MatchesEqualString) {
  Matcher<const char*> m = StrEq(std::string("Hello"));
  EXPECT_TRUE(m.Matches("Hello"));
  EXPECT_FALSE(m.Matches("hello"));
  EXPECT_FALSE(m.Matches(nullptr));

  Matcher<const std::string&> m2 = StrEq("Hello");
  EXPECT_TRUE(m2.Matches("Hello"));
  EXPECT_FALSE(m2.Matches("Hi"));

#if GTEST_INTERNAL_HAS_STRING_VIEW
  Matcher<const internal::StringView&> m3 =
      StrEq(internal::StringView("Hello"));
  EXPECT_TRUE(m3.Matches(internal::StringView("Hello")));
  EXPECT_FALSE(m3.Matches(internal::StringView("hello")));
  EXPECT_FALSE(m3.Matches(internal::StringView()));

  Matcher<const internal::StringView&> m_empty = StrEq("");
  EXPECT_TRUE(m_empty.Matches(internal::StringView("")));
  EXPECT_TRUE(m_empty.Matches(internal::StringView()));
  EXPECT_FALSE(m_empty.Matches(internal::StringView("hello")));
#endif  // GTEST_INTERNAL_HAS_STRING_VIEW
}

TEST(StrEqTest, CanDescribeSelf) {
  Matcher<std::string> m = StrEq("Hi-\'\"?\\\a\b\f\n\r\t\v\xD3");
  EXPECT_EQ("is equal to \"Hi-\'\\\"?\\\\\\a\\b\\f\\n\\r\\t\\v\\xD3\"",
      Describe(m));

  std::string str("01204500800");
  str[3] = '\0';
  Matcher<std::string> m2 = StrEq(str);
  EXPECT_EQ("is equal to \"012\\04500800\"", Describe(m2));
  str[0] = str[6] = str[7] = str[9] = str[10] = '\0';
  Matcher<std::string> m3 = StrEq(str);
  EXPECT_EQ("is equal to \"\\012\\045\\0\\08\\0\\0\"", Describe(m3));
}

TEST(StrNeTest, MatchesUnequalString) {
  Matcher<const char*> m = StrNe("Hello");
  EXPECT_TRUE(m.Matches(""));
  EXPECT_TRUE(m.Matches(nullptr));
  EXPECT_FALSE(m.Matches("Hello"));

  Matcher<std::string> m2 = StrNe(std::string("Hello"));
  EXPECT_TRUE(m2.Matches("hello"));
  EXPECT_FALSE(m2.Matches("Hello"));

#if GTEST_INTERNAL_HAS_STRING_VIEW
  Matcher<const internal::StringView> m3 = StrNe(internal::StringView("Hello"));
  EXPECT_TRUE(m3.Matches(internal::StringView("")));
  EXPECT_TRUE(m3.Matches(internal::StringView()));
  EXPECT_FALSE(m3.Matches(internal::StringView("Hello")));
#endif  // GTEST_INTERNAL_HAS_STRING_VIEW
}

TEST(StrNeTest, CanDescribeSelf) {
  Matcher<const char*> m = StrNe("Hi");
  EXPECT_EQ("isn't equal to \"Hi\"", Describe(m));
}

TEST(StrCaseEqTest, MatchesEqualStringIgnoringCase) {
  Matcher<const char*> m = StrCaseEq(std::string("Hello"));
  EXPECT_TRUE(m.Matches("Hello"));
  EXPECT_TRUE(m.Matches("hello"));
  EXPECT_FALSE(m.Matches("Hi"));
  EXPECT_FALSE(m.Matches(nullptr));

  Matcher<const std::string&> m2 = StrCaseEq("Hello");
  EXPECT_TRUE(m2.Matches("hello"));
  EXPECT_FALSE(m2.Matches("Hi"));

#if GTEST_INTERNAL_HAS_STRING_VIEW
  Matcher<const internal::StringView&> m3 =
      StrCaseEq(internal::StringView("Hello"));
  EXPECT_TRUE(m3.Matches(internal::StringView("Hello")));
  EXPECT_TRUE(m3.Matches(internal::StringView("hello")));
  EXPECT_FALSE(m3.Matches(internal::StringView("Hi")));
  EXPECT_FALSE(m3.Matches(internal::StringView()));
#endif  // GTEST_INTERNAL_HAS_STRING_VIEW
}

TEST(StrCaseEqTest, MatchesEqualStringWith0IgnoringCase) {
  std::string str1("oabocdooeoo");
  std::string str2("OABOCDOOEOO");
  Matcher<const std::string&> m0 = StrCaseEq(str1);
  EXPECT_FALSE(m0.Matches(str2 + std::string(1, '\0')));

  str1[3] = str2[3] = '\0';
  Matcher<const std::string&> m1 = StrCaseEq(str1);
  EXPECT_TRUE(m1.Matches(str2));

  str1[0] = str1[6] = str1[7] = str1[10] = '\0';
  str2[0] = str2[6] = str2[7] = str2[10] = '\0';
  Matcher<const std::string&> m2 = StrCaseEq(str1);
  str1[9] = str2[9] = '\0';
  EXPECT_FALSE(m2.Matches(str2));

  Matcher<const std::string&> m3 = StrCaseEq(str1);
  EXPECT_TRUE(m3.Matches(str2));

  EXPECT_FALSE(m3.Matches(str2 + "x"));
  str2.append(1, '\0');
  EXPECT_FALSE(m3.Matches(str2));
  EXPECT_FALSE(m3.Matches(std::string(str2, 0, 9)));
}

TEST(StrCaseEqTest, CanDescribeSelf) {
  Matcher<std::string> m = StrCaseEq("Hi");
  EXPECT_EQ("is equal to (ignoring case) \"Hi\"", Describe(m));
}

TEST(StrCaseNeTest, MatchesUnequalStringIgnoringCase) {
  Matcher<const char*> m = StrCaseNe("Hello");
  EXPECT_TRUE(m.Matches("Hi"));
  EXPECT_TRUE(m.Matches(nullptr));
  EXPECT_FALSE(m.Matches("Hello"));
  EXPECT_FALSE(m.Matches("hello"));

  Matcher<std::string> m2 = StrCaseNe(std::string("Hello"));
  EXPECT_TRUE(m2.Matches(""));
  EXPECT_FALSE(m2.Matches("Hello"));

#if GTEST_INTERNAL_HAS_STRING_VIEW
  Matcher<const internal::StringView> m3 =
      StrCaseNe(internal::StringView("Hello"));
  EXPECT_TRUE(m3.Matches(internal::StringView("Hi")));
  EXPECT_TRUE(m3.Matches(internal::StringView()));
  EXPECT_FALSE(m3.Matches(internal::StringView("Hello")));
  EXPECT_FALSE(m3.Matches(internal::StringView("hello")));
#endif  // GTEST_INTERNAL_HAS_STRING_VIEW
}

TEST(StrCaseNeTest, CanDescribeSelf) {
  Matcher<const char*> m = StrCaseNe("Hi");
  EXPECT_EQ("isn't equal to (ignoring case) \"Hi\"", Describe(m));
}

// Tests that HasSubstr() works for matching string-typed values.
TEST(HasSubstrTest, WorksForStringClasses) {
  const Matcher<std::string> m1 = HasSubstr("foo");
  EXPECT_TRUE(m1.Matches(std::string("I love food.")));
  EXPECT_FALSE(m1.Matches(std::string("tofo")));

  const Matcher<const std::string&> m2 = HasSubstr("foo");
  EXPECT_TRUE(m2.Matches(std::string("I love food.")));
  EXPECT_FALSE(m2.Matches(std::string("tofo")));

  const Matcher<std::string> m_empty = HasSubstr("");
  EXPECT_TRUE(m_empty.Matches(std::string()));
  EXPECT_TRUE(m_empty.Matches(std::string("not empty")));
}

// Tests that HasSubstr() works for matching C-string-typed values.
TEST(HasSubstrTest, WorksForCStrings) {
  const Matcher<char*> m1 = HasSubstr("foo");
  EXPECT_TRUE(m1.Matches(const_cast<char*>("I love food.")));
  EXPECT_FALSE(m1.Matches(const_cast<char*>("tofo")));
  EXPECT_FALSE(m1.Matches(nullptr));

  const Matcher<const char*> m2 = HasSubstr("foo");
  EXPECT_TRUE(m2.Matches("I love food."));
  EXPECT_FALSE(m2.Matches("tofo"));
  EXPECT_FALSE(m2.Matches(nullptr));

  const Matcher<const char*> m_empty = HasSubstr("");
  EXPECT_TRUE(m_empty.Matches("not empty"));
  EXPECT_TRUE(m_empty.Matches(""));
  EXPECT_FALSE(m_empty.Matches(nullptr));
}

#if GTEST_INTERNAL_HAS_STRING_VIEW
// Tests that HasSubstr() works for matching StringView-typed values.
TEST(HasSubstrTest, WorksForStringViewClasses) {
  const Matcher<internal::StringView> m1 =
      HasSubstr(internal::StringView("foo"));
  EXPECT_TRUE(m1.Matches(internal::StringView("I love food.")));
  EXPECT_FALSE(m1.Matches(internal::StringView("tofo")));
  EXPECT_FALSE(m1.Matches(internal::StringView()));

  const Matcher<const internal::StringView&> m2 = HasSubstr("foo");
  EXPECT_TRUE(m2.Matches(internal::StringView("I love food.")));
  EXPECT_FALSE(m2.Matches(internal::StringView("tofo")));
  EXPECT_FALSE(m2.Matches(internal::StringView()));

  const Matcher<const internal::StringView&> m3 = HasSubstr("");
  EXPECT_TRUE(m3.Matches(internal::StringView("foo")));
  EXPECT_TRUE(m3.Matches(internal::StringView("")));
  EXPECT_TRUE(m3.Matches(internal::StringView()));
}
#endif  // GTEST_INTERNAL_HAS_STRING_VIEW

// Tests that HasSubstr(s) describes itself properly.
TEST(HasSubstrTest, CanDescribeSelf) {
  Matcher<std::string> m = HasSubstr("foo\n\"");
  EXPECT_EQ("has substring \"foo\\n\\\"\"", Describe(m));
}

TEST(KeyTest, CanDescribeSelf) {
  Matcher<const pair<std::string, int>&> m = Key("foo");
  EXPECT_EQ("has a key that is equal to \"foo\"", Describe(m));
  EXPECT_EQ("doesn't have a key that is equal to \"foo\"", DescribeNegation(m));
}

TEST(KeyTest, ExplainsResult) {
  Matcher<pair<int, bool> > m = Key(GreaterThan(10));
  EXPECT_EQ("whose first field is a value which is 5 less than 10",
            Explain(m, make_pair(5, true)));
  EXPECT_EQ("whose first field is a value which is 5 more than 10",
            Explain(m, make_pair(15, true)));
}

TEST(KeyTest, MatchesCorrectly) {
  pair<int, std::string> p(25, "foo");
  EXPECT_THAT(p, Key(25));
  EXPECT_THAT(p, Not(Key(42)));
  EXPECT_THAT(p, Key(Ge(20)));
  EXPECT_THAT(p, Not(Key(Lt(25))));
}

TEST(KeyTest, WorksWithMoveOnly) {
  pair<std::unique_ptr<int>, std::unique_ptr<int>> p;
  EXPECT_THAT(p, Key(Eq(nullptr)));
}

template <size_t I>
struct Tag {};

struct PairWithGet {
  int member_1;
  std::string member_2;
  using first_type = int;
  using second_type = std::string;

  const int& GetImpl(Tag<0>) const { return member_1; }
  const std::string& GetImpl(Tag<1>) const { return member_2; }
};
template <size_t I>
auto get(const PairWithGet& value) -> decltype(value.GetImpl(Tag<I>())) {
  return value.GetImpl(Tag<I>());
}
TEST(PairTest, MatchesPairWithGetCorrectly) {
  PairWithGet p{25, "foo"};
  EXPECT_THAT(p, Key(25));
  EXPECT_THAT(p, Not(Key(42)));
  EXPECT_THAT(p, Key(Ge(20)));
  EXPECT_THAT(p, Not(Key(Lt(25))));

  std::vector<PairWithGet> v = {{11, "Foo"}, {29, "gMockIsBestMock"}};
  EXPECT_THAT(v, Contains(Key(29)));
}

TEST(KeyTest, SafelyCastsInnerMatcher) {
  Matcher<int> is_positive = Gt(0);
  Matcher<int> is_negative = Lt(0);
  pair<char, bool> p('a', true);
  EXPECT_THAT(p, Key(is_positive));
  EXPECT_THAT(p, Not(Key(is_negative)));
}

TEST(KeyTest, InsideContainsUsingMap) {
  map<int, char> container;
  container.insert(make_pair(1, 'a'));
  container.insert(make_pair(2, 'b'));
  container.insert(make_pair(4, 'c'));
  EXPECT_THAT(container, Contains(Key(1)));
  EXPECT_THAT(container, Not(Contains(Key(3))));
}

TEST(KeyTest, InsideContainsUsingMultimap) {
  multimap<int, char> container;
  container.insert(make_pair(1, 'a'));
  container.insert(make_pair(2, 'b'));
  container.insert(make_pair(4, 'c'));

  EXPECT_THAT(container, Not(Contains(Key(25))));
  container.insert(make_pair(25, 'd'));
  EXPECT_THAT(container, Contains(Key(25)));
  container.insert(make_pair(25, 'e'));
  EXPECT_THAT(container, Contains(Key(25)));

  EXPECT_THAT(container, Contains(Key(1)));
  EXPECT_THAT(container, Not(Contains(Key(3))));
}

TEST(PairTest, Typing) {
  // Test verifies the following type conversions can be compiled.
  Matcher<const pair<const char*, int>&> m1 = Pair("foo", 42);
  Matcher<const pair<const char*, int> > m2 = Pair("foo", 42);
  Matcher<pair<const char*, int> > m3 = Pair("foo", 42);

  Matcher<pair<int, const std::string> > m4 = Pair(25, "42");
  Matcher<pair<const std::string, int> > m5 = Pair("25", 42);
}

TEST(PairTest, CanDescribeSelf) {
  Matcher<const pair<std::string, int>&> m1 = Pair("foo", 42);
  EXPECT_EQ("has a first field that is equal to \"foo\""
            ", and has a second field that is equal to 42",
            Describe(m1));
  EXPECT_EQ("has a first field that isn't equal to \"foo\""
            ", or has a second field that isn't equal to 42",
            DescribeNegation(m1));
  // Double and triple negation (1 or 2 times not and description of negation).
  Matcher<const pair<int, int>&> m2 = Not(Pair(Not(13), 42));
  EXPECT_EQ("has a first field that isn't equal to 13"
            ", and has a second field that is equal to 42",
            DescribeNegation(m2));
}

TEST(PairTest, CanExplainMatchResultTo) {
  // If neither field matches, Pair() should explain about the first
  // field.
  const Matcher<pair<int, int> > m = Pair(GreaterThan(0), GreaterThan(0));
  EXPECT_EQ("whose first field does not match, which is 1 less than 0",
            Explain(m, make_pair(-1, -2)));

  // If the first field matches but the second doesn't, Pair() should
  // explain about the second field.
  EXPECT_EQ("whose second field does not match, which is 2 less than 0",
            Explain(m, make_pair(1, -2)));

  // If the first field doesn't match but the second does, Pair()
  // should explain about the first field.
  EXPECT_EQ("whose first field does not match, which is 1 less than 0",
            Explain(m, make_pair(-1, 2)));

  // If both fields match, Pair() should explain about them both.
  EXPECT_EQ("whose both fields match, where the first field is a value "
            "which is 1 more than 0, and the second field is a value "
            "which is 2 more than 0",
            Explain(m, make_pair(1, 2)));

  // If only the first match has an explanation, only this explanation should
  // be printed.
  const Matcher<pair<int, int> > explain_first = Pair(GreaterThan(0), 0);
  EXPECT_EQ("whose both fields match, where the first field is a value "
            "which is 1 more than 0",
            Explain(explain_first, make_pair(1, 0)));

  // If only the second match has an explanation, only this explanation should
  // be printed.
  const Matcher<pair<int, int> > explain_second = Pair(0, GreaterThan(0));
  EXPECT_EQ("whose both fields match, where the second field is a value "
            "which is 1 more than 0",
            Explain(explain_second, make_pair(0, 1)));
}

TEST(PairTest, MatchesCorrectly) {
  pair<int, std::string> p(25, "foo");

  // Both fields match.
  EXPECT_THAT(p, Pair(25, "foo"));
  EXPECT_THAT(p, Pair(Ge(20), HasSubstr("o")));

  // 'first' doesnt' match, but 'second' matches.
  EXPECT_THAT(p, Not(Pair(42, "foo")));
  EXPECT_THAT(p, Not(Pair(Lt(25), "foo")));

  // 'first' matches, but 'second' doesn't match.
  EXPECT_THAT(p, Not(Pair(25, "bar")));
  EXPECT_THAT(p, Not(Pair(25, Not("foo"))));

  // Neither field matches.
  EXPECT_THAT(p, Not(Pair(13, "bar")));
  EXPECT_THAT(p, Not(Pair(Lt(13), HasSubstr("a"))));
}

TEST(PairTest, WorksWithMoveOnly) {
  pair<std::unique_ptr<int>, std::unique_ptr<int>> p;
  p.second.reset(new int(7));
  EXPECT_THAT(p, Pair(Eq(nullptr), Ne(nullptr)));
}

TEST(PairTest, SafelyCastsInnerMatchers) {
  Matcher<int> is_positive = Gt(0);
  Matcher<int> is_negative = Lt(0);
  pair<char, bool> p('a', true);
  EXPECT_THAT(p, Pair(is_positive, _));
  EXPECT_THAT(p, Not(Pair(is_negative, _)));
  EXPECT_THAT(p, Pair(_, is_positive));
  EXPECT_THAT(p, Not(Pair(_, is_negative)));
}

TEST(PairTest, InsideContainsUsingMap) {
  map<int, char> container;
  container.insert(make_pair(1, 'a'));
  container.insert(make_pair(2, 'b'));
  container.insert(make_pair(4, 'c'));
  EXPECT_THAT(container, Contains(Pair(1, 'a')));
  EXPECT_THAT(container, Contains(Pair(1, _)));
  EXPECT_THAT(container, Contains(Pair(_, 'a')));
  EXPECT_THAT(container, Not(Contains(Pair(3, _))));
}

TEST(FieldsAreTest, MatchesCorrectly) {
  std::tuple<int, std::string, double> p(25, "foo", .5);

  // All fields match.
  EXPECT_THAT(p, FieldsAre(25, "foo", .5));
  EXPECT_THAT(p, FieldsAre(Ge(20), HasSubstr("o"), DoubleEq(.5)));

  // Some don't match.
  EXPECT_THAT(p, Not(FieldsAre(26, "foo", .5)));
  EXPECT_THAT(p, Not(FieldsAre(25, "fo", .5)));
  EXPECT_THAT(p, Not(FieldsAre(25, "foo", .6)));
}

TEST(FieldsAreTest, CanDescribeSelf) {
  Matcher<const pair<std::string, int>&> m1 = FieldsAre("foo", 42);
  EXPECT_EQ(
      "has field #0 that is equal to \"foo\""
      ", and has field #1 that is equal to 42",
      Describe(m1));
  EXPECT_EQ(
      "has field #0 that isn't equal to \"foo\""
      ", or has field #1 that isn't equal to 42",
      DescribeNegation(m1));
}

TEST(FieldsAreTest, CanExplainMatchResultTo) {
  // The first one that fails is the one that gives the error.
  Matcher<std::tuple<int, int, int>> m =
      FieldsAre(GreaterThan(0), GreaterThan(0), GreaterThan(0));

  EXPECT_EQ("whose field #0 does not match, which is 1 less than 0",
            Explain(m, std::make_tuple(-1, -2, -3)));
  EXPECT_EQ("whose field #1 does not match, which is 2 less than 0",
            Explain(m, std::make_tuple(1, -2, -3)));
  EXPECT_EQ("whose field #2 does not match, which is 3 less than 0",
            Explain(m, std::make_tuple(1, 2, -3)));

  // If they all match, we get a long explanation of success.
  EXPECT_EQ(
      "whose all elements match, "
      "where field #0 is a value which is 1 more than 0"
      ", and field #1 is a value which is 2 more than 0"
      ", and field #2 is a value which is 3 more than 0",
      Explain(m, std::make_tuple(1, 2, 3)));

  // Only print those that have an explanation.
  m = FieldsAre(GreaterThan(0), 0, GreaterThan(0));
  EXPECT_EQ(
      "whose all elements match, "
      "where field #0 is a value which is 1 more than 0"
      ", and field #2 is a value which is 3 more than 0",
      Explain(m, std::make_tuple(1, 0, 3)));

  // If only one has an explanation, then print that one.
  m = FieldsAre(0, GreaterThan(0), 0);
  EXPECT_EQ(
      "whose all elements match, "
      "where field #1 is a value which is 1 more than 0",
      Explain(m, std::make_tuple(0, 1, 0)));
}

#if defined(__cpp_structured_bindings) && __cpp_structured_bindings >= 201606
TEST(FieldsAreTest, StructuredBindings) {
  // testing::FieldsAre can also match aggregates and such with C++17 and up.
  struct MyType {
    int i;
    std::string str;
  };
  EXPECT_THAT((MyType{17, "foo"}), FieldsAre(Eq(17), HasSubstr("oo")));

  // Test all the supported arities.
  struct MyVarType1 {
    int a;
  };
  EXPECT_THAT(MyVarType1{}, FieldsAre(0));
  struct MyVarType2 {
    int a, b;
  };
  EXPECT_THAT(MyVarType2{}, FieldsAre(0, 0));
  struct MyVarType3 {
    int a, b, c;
  };
  EXPECT_THAT(MyVarType3{}, FieldsAre(0, 0, 0));
  struct MyVarType4 {
    int a, b, c, d;
  };
  EXPECT_THAT(MyVarType4{}, FieldsAre(0, 0, 0, 0));
  struct MyVarType5 {
    int a, b, c, d, e;
  };
  EXPECT_THAT(MyVarType5{}, FieldsAre(0, 0, 0, 0, 0));
  struct MyVarType6 {
    int a, b, c, d, e, f;
  };
  EXPECT_THAT(MyVarType6{}, FieldsAre(0, 0, 0, 0, 0, 0));
  struct MyVarType7 {
    int a, b, c, d, e, f, g;
  };
  EXPECT_THAT(MyVarType7{}, FieldsAre(0, 0, 0, 0, 0, 0, 0));
  struct MyVarType8 {
    int a, b, c, d, e, f, g, h;
  };
  EXPECT_THAT(MyVarType8{}, FieldsAre(0, 0, 0, 0, 0, 0, 0, 0));
  struct MyVarType9 {
    int a, b, c, d, e, f, g, h, i;
  };
  EXPECT_THAT(MyVarType9{}, FieldsAre(0, 0, 0, 0, 0, 0, 0, 0, 0));
  struct MyVarType10 {
    int a, b, c, d, e, f, g, h, i, j;
  };
  EXPECT_THAT(MyVarType10{}, FieldsAre(0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
  struct MyVarType11 {
    int a, b, c, d, e, f, g, h, i, j, k;
  };
  EXPECT_THAT(MyVarType11{}, FieldsAre(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
  struct MyVarType12 {
    int a, b, c, d, e, f, g, h, i, j, k, l;
  };
  EXPECT_THAT(MyVarType12{}, FieldsAre(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
  struct MyVarType13 {
    int a, b, c, d, e, f, g, h, i, j, k, l, m;
  };
  EXPECT_THAT(MyVarType13{}, FieldsAre(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
  struct MyVarType14 {
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n;
  };
  EXPECT_THAT(MyVarType14{},
              FieldsAre(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
  struct MyVarType15 {
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o;
  };
  EXPECT_THAT(MyVarType15{},
              FieldsAre(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
  struct MyVarType16 {
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p;
  };
  EXPECT_THAT(MyVarType16{},
              FieldsAre(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
}
#endif

TEST(ContainsTest, WorksWithMoveOnly) {
  ContainerHelper helper;
  EXPECT_CALL(helper, Call(Contains(Pointee(2))));
  helper.Call(MakeUniquePtrs({1, 2}));
}

TEST(PairTest, UseGetInsteadOfMembers) {
  PairWithGet pair{7, "ABC"};
  EXPECT_THAT(pair, Pair(7, "ABC"));
  EXPECT_THAT(pair, Pair(Ge(7), HasSubstr("AB")));
  EXPECT_THAT(pair, Not(Pair(Lt(7), "ABC")));

  std::vector<PairWithGet> v = {{11, "Foo"}, {29, "gMockIsBestMock"}};
  EXPECT_THAT(v,
              ElementsAre(Pair(11, std::string("Foo")), Pair(Ge(10), Not(""))));
}

// Tests StartsWith(s).

TEST(StartsWithTest, MatchesStringWithGivenPrefix) {
  const Matcher<const char*> m1 = StartsWith(std::string(""));
  EXPECT_TRUE(m1.Matches("Hi"));
  EXPECT_TRUE(m1.Matches(""));
  EXPECT_FALSE(m1.Matches(nullptr));

  const Matcher<const std::string&> m2 = StartsWith("Hi");
  EXPECT_TRUE(m2.Matches("Hi"));
  EXPECT_TRUE(m2.Matches("Hi Hi!"));
  EXPECT_TRUE(m2.Matches("High"));
  EXPECT_FALSE(m2.Matches("H"));
  EXPECT_FALSE(m2.Matches(" Hi"));

#if GTEST_INTERNAL_HAS_STRING_VIEW
  const Matcher<internal::StringView> m_empty =
      StartsWith(internal::StringView(""));
  EXPECT_TRUE(m_empty.Matches(internal::StringView()));
  EXPECT_TRUE(m_empty.Matches(internal::StringView("")));
  EXPECT_TRUE(m_empty.Matches(internal::StringView("not empty")));
#endif  // GTEST_INTERNAL_HAS_STRING_VIEW
}

TEST(StartsWithTest, CanDescribeSelf) {
  Matcher<const std::string> m = StartsWith("Hi");
  EXPECT_EQ("starts with \"Hi\"", Describe(m));
}

// Tests EndsWith(s).

TEST(EndsWithTest, MatchesStringWithGivenSuffix) {
  const Matcher<const char*> m1 = EndsWith("");
  EXPECT_TRUE(m1.Matches("Hi"));
  EXPECT_TRUE(m1.Matches(""));
  EXPECT_FALSE(m1.Matches(nullptr));

  const Matcher<const std::string&> m2 = EndsWith(std::string("Hi"));
  EXPECT_TRUE(m2.Matches("Hi"));
  EXPECT_TRUE(m2.Matches("Wow Hi Hi"));
  EXPECT_TRUE(m2.Matches("Super Hi"));
  EXPECT_FALSE(m2.Matches("i"));
  EXPECT_FALSE(m2.Matches("Hi "));

#if GTEST_INTERNAL_HAS_STRING_VIEW
  const Matcher<const internal::StringView&> m4 =
      EndsWith(internal::StringView(""));
  EXPECT_TRUE(m4.Matches("Hi"));
  EXPECT_TRUE(m4.Matches(""));
  EXPECT_TRUE(m4.Matches(internal::StringView()));
  EXPECT_TRUE(m4.Matches(internal::StringView("")));
#endif  // GTEST_INTERNAL_HAS_STRING_VIEW
}

TEST(EndsWithTest, CanDescribeSelf) {
  Matcher<const std::string> m = EndsWith("Hi");
  EXPECT_EQ("ends with \"Hi\"", Describe(m));
}

// Tests MatchesRegex().

TEST(MatchesRegexTest, MatchesStringMatchingGivenRegex) {
  const Matcher<const char*> m1 = MatchesRegex("a.*z");
  EXPECT_TRUE(m1.Matches("az"));
  EXPECT_TRUE(m1.Matches("abcz"));
  EXPECT_FALSE(m1.Matches(nullptr));

  const Matcher<const std::string&> m2 = MatchesRegex(new RE("a.*z"));
  EXPECT_TRUE(m2.Matches("azbz"));
  EXPECT_FALSE(m2.Matches("az1"));
  EXPECT_FALSE(m2.Matches("1az"));

#if GTEST_INTERNAL_HAS_STRING_VIEW
  const Matcher<const internal::StringView&> m3 = MatchesRegex("a.*z");
  EXPECT_TRUE(m3.Matches(internal::StringView("az")));
  EXPECT_TRUE(m3.Matches(internal::StringView("abcz")));
  EXPECT_FALSE(m3.Matches(internal::StringView("1az")));
  EXPECT_FALSE(m3.Matches(internal::StringView()));
  const Matcher<const internal::StringView&> m4 =
      MatchesRegex(internal::StringView(""));
  EXPECT_TRUE(m4.Matches(internal::StringView("")));
  EXPECT_TRUE(m4.Matches(internal::StringView()));
#endif  // GTEST_INTERNAL_HAS_STRING_VIEW
}

TEST(MatchesRegexTest, CanDescribeSelf) {
  Matcher<const std::string> m1 = MatchesRegex(std::string("Hi.*"));
  EXPECT_EQ("matches regular expression \"Hi.*\"", Describe(m1));

  Matcher<const char*> m2 = MatchesRegex(new RE("a.*"));
  EXPECT_EQ("matches regular expression \"a.*\"", Describe(m2));

#if GTEST_INTERNAL_HAS_STRING_VIEW
  Matcher<const internal::StringView> m3 = MatchesRegex(new RE("0.*"));
  EXPECT_EQ("matches regular expression \"0.*\"", Describe(m3));
#endif  // GTEST_INTERNAL_HAS_STRING_VIEW
}

// Tests ContainsRegex().

TEST(ContainsRegexTest, MatchesStringContainingGivenRegex) {
  const Matcher<const char*> m1 = ContainsRegex(std::string("a.*z"));
  EXPECT_TRUE(m1.Matches("az"));
  EXPECT_TRUE(m1.Matches("0abcz1"));
  EXPECT_FALSE(m1.Matches(nullptr));

  const Matcher<const std::string&> m2 = ContainsRegex(new RE("a.*z"));
  EXPECT_TRUE(m2.Matches("azbz"));
  EXPECT_TRUE(m2.Matches("az1"));
  EXPECT_FALSE(m2.Matches("1a"));

#if GTEST_INTERNAL_HAS_STRING_VIEW
  const Matcher<const internal::StringView&> m3 =
      ContainsRegex(new RE("a.*z"));
  EXPECT_TRUE(m3.Matches(internal::StringView("azbz")));
  EXPECT_TRUE(m3.Matches(internal::StringView("az1")));
  EXPECT_FALSE(m3.Matches(internal::StringView("1a")));
  EXPECT_FALSE(m3.Matches(internal::StringView()));
  const Matcher<const internal::StringView&> m4 =
      ContainsRegex(internal::StringView(""));
  EXPECT_TRUE(m4.Matches(internal::StringView("")));
  EXPECT_TRUE(m4.Matches(internal::StringView()));
#endif  // GTEST_INTERNAL_HAS_STRING_VIEW
}

TEST(ContainsRegexTest, CanDescribeSelf) {
  Matcher<const std::string> m1 = ContainsRegex("Hi.*");
  EXPECT_EQ("contains regular expression \"Hi.*\"", Describe(m1));

  Matcher<const char*> m2 = ContainsRegex(new RE("a.*"));
  EXPECT_EQ("contains regular expression \"a.*\"", Describe(m2));

#if GTEST_INTERNAL_HAS_STRING_VIEW
  Matcher<const internal::StringView> m3 = ContainsRegex(new RE("0.*"));
  EXPECT_EQ("contains regular expression \"0.*\"", Describe(m3));
#endif  // GTEST_INTERNAL_HAS_STRING_VIEW
}

// Tests for wide strings.
#if GTEST_HAS_STD_WSTRING
TEST(StdWideStrEqTest, MatchesEqual) {
  Matcher<const wchar_t*> m = StrEq(::std::wstring(L"Hello"));
  EXPECT_TRUE(m.Matches(L"Hello"));
  EXPECT_FALSE(m.Matches(L"hello"));
  EXPECT_FALSE(m.Matches(nullptr));

  Matcher<const ::std::wstring&> m2 = StrEq(L"Hello");
  EXPECT_TRUE(m2.Matches(L"Hello"));
  EXPECT_FALSE(m2.Matches(L"Hi"));

  Matcher<const ::std::wstring&> m3 = StrEq(L"\xD3\x576\x8D3\xC74D");
  EXPECT_TRUE(m3.Matches(L"\xD3\x576\x8D3\xC74D"));
  EXPECT_FALSE(m3.Matches(L"\xD3\x576\x8D3\xC74E"));

  ::std::wstring str(L"01204500800");
  str[3] = L'\0';
  Matcher<const ::std::wstring&> m4 = StrEq(str);
  EXPECT_TRUE(m4.Matches(str));
  str[0] = str[6] = str[7] = str[9] = str[10] = L'\0';
  Matcher<const ::std::wstring&> m5 = StrEq(str);
  EXPECT_TRUE(m5.Matches(str));
}

TEST(StdWideStrEqTest, CanDescribeSelf) {
  Matcher< ::std::wstring> m = StrEq(L"Hi-\'\"?\\\a\b\f\n\r\t\v");
  EXPECT_EQ("is equal to L\"Hi-\'\\\"?\\\\\\a\\b\\f\\n\\r\\t\\v\"",
    Describe(m));

  Matcher< ::std::wstring> m2 = StrEq(L"\xD3\x576\x8D3\xC74D");
  EXPECT_EQ("is equal to L\"\\xD3\\x576\\x8D3\\xC74D\"",
    Describe(m2));

  ::std::wstring str(L"01204500800");
  str[3] = L'\0';
  Matcher<const ::std::wstring&> m4 = StrEq(str);
  EXPECT_EQ("is equal to L\"012\\04500800\"", Describe(m4));
  str[0] = str[6] = str[7] = str[9] = str[10] = L'\0';
  Matcher<const ::std::wstring&> m5 = StrEq(str);
  EXPECT_EQ("is equal to L\"\\012\\045\\0\\08\\0\\0\"", Describe(m5));
}

TEST(StdWideStrNeTest, MatchesUnequalString) {
  Matcher<const wchar_t*> m = StrNe(L"Hello");
  EXPECT_TRUE(m.Matches(L""));
  EXPECT_TRUE(m.Matches(nullptr));
  EXPECT_FALSE(m.Matches(L"Hello"));

  Matcher< ::std::wstring> m2 = StrNe(::std::wstring(L"Hello"));
  EXPECT_TRUE(m2.Matches(L"hello"));
  EXPECT_FALSE(m2.Matches(L"Hello"));
}

TEST(StdWideStrNeTest, CanDescribeSelf) {
  Matcher<const wchar_t*> m = StrNe(L"Hi");
  EXPECT_EQ("isn't equal to L\"Hi\"", Describe(m));
}

TEST(StdWideStrCaseEqTest, MatchesEqualStringIgnoringCase) {
  Matcher<const wchar_t*> m = StrCaseEq(::std::wstring(L"Hello"));
  EXPECT_TRUE(m.Matches(L"Hello"));
  EXPECT_TRUE(m.Matches(L"hello"));
  EXPECT_FALSE(m.Matches(L"Hi"));
  EXPECT_FALSE(m.Matches(nullptr));

  Matcher<const ::std::wstring&> m2 = StrCaseEq(L"Hello");
  EXPECT_TRUE(m2.Matches(L"hello"));
  EXPECT_FALSE(m2.Matches(L"Hi"));
}

TEST(StdWideStrCaseEqTest, MatchesEqualStringWith0IgnoringCase) {
  ::std::wstring str1(L"oabocdooeoo");
  ::std::wstring str2(L"OABOCDOOEOO");
  Matcher<const ::std::wstring&> m0 = StrCaseEq(str1);
  EXPECT_FALSE(m0.Matches(str2 + ::std::wstring(1, L'\0')));

  str1[3] = str2[3] = L'\0';
  Matcher<const ::std::wstring&> m1 = StrCaseEq(str1);
  EXPECT_TRUE(m1.Matches(str2));

  str1[0] = str1[6] = str1[7] = str1[10] = L'\0';
  str2[0] = str2[6] = str2[7] = str2[10] = L'\0';
  Matcher<const ::std::wstring&> m2 = StrCaseEq(str1);
  str1[9] = str2[9] = L'\0';
  EXPECT_FALSE(m2.Matches(str2));

  Matcher<const ::std::wstring&> m3 = StrCaseEq(str1);
  EXPECT_TRUE(m3.Matches(str2));

  EXPECT_FALSE(m3.Matches(str2 + L"x"));
  str2.append(1, L'\0');
  EXPECT_FALSE(m3.Matches(str2));
  EXPECT_FALSE(m3.Matches(::std::wstring(str2, 0, 9)));
}

TEST(StdWideStrCaseEqTest, CanDescribeSelf) {
  Matcher< ::std::wstring> m = StrCaseEq(L"Hi");
  EXPECT_EQ("is equal to (ignoring case) L\"Hi\"", Describe(m));
}

TEST(StdWideStrCaseNeTest, MatchesUnequalStringIgnoringCase) {
  Matcher<const wchar_t*> m = StrCaseNe(L"Hello");
  EXPECT_TRUE(m.Matches(L"Hi"));
  EXPECT_TRUE(m.Matches(nullptr));
  EXPECT_FALSE(m.Matches(L"Hello"));
  EXPECT_FALSE(m.Matches(L"hello"));

  Matcher< ::std::wstring> m2 = StrCaseNe(::std::wstring(L"Hello"));
  EXPECT_TRUE(m2.Matches(L""));
  EXPECT_FALSE(m2.Matches(L"Hello"));
}

TEST(StdWideStrCaseNeTest, CanDescribeSelf) {
  Matcher<const wchar_t*> m = StrCaseNe(L"Hi");
  EXPECT_EQ("isn't equal to (ignoring case) L\"Hi\"", Describe(m));
}

// Tests that HasSubstr() works for matching wstring-typed values.
TEST(StdWideHasSubstrTest, WorksForStringClasses) {
  const Matcher< ::std::wstring> m1 = HasSubstr(L"foo");
  EXPECT_TRUE(m1.Matches(::std::wstring(L"I love food.")));
  EXPECT_FALSE(m1.Matches(::std::wstring(L"tofo")));

  const Matcher<const ::std::wstring&> m2 = HasSubstr(L"foo");
  EXPECT_TRUE(m2.Matches(::std::wstring(L"I love food.")));
  EXPECT_FALSE(m2.Matches(::std::wstring(L"tofo")));
}

// Tests that HasSubstr() works for matching C-wide-string-typed values.
TEST(StdWideHasSubstrTest, WorksForCStrings) {
  const Matcher<wchar_t*> m1 = HasSubstr(L"foo");
  EXPECT_TRUE(m1.Matches(const_cast<wchar_t*>(L"I love food.")));
  EXPECT_FALSE(m1.Matches(const_cast<wchar_t*>(L"tofo")));
  EXPECT_FALSE(m1.Matches(nullptr));

  const Matcher<const wchar_t*> m2 = HasSubstr(L"foo");
  EXPECT_TRUE(m2.Matches(L"I love food."));
  EXPECT_FALSE(m2.Matches(L"tofo"));
  EXPECT_FALSE(m2.Matches(nullptr));
}

// Tests that HasSubstr(s) describes itself properly.
TEST(StdWideHasSubstrTest, CanDescribeSelf) {
  Matcher< ::std::wstring> m = HasSubstr(L"foo\n\"");
  EXPECT_EQ("has substring L\"foo\\n\\\"\"", Describe(m));
}

// Tests StartsWith(s).

TEST(StdWideStartsWithTest, MatchesStringWithGivenPrefix) {
  const Matcher<const wchar_t*> m1 = StartsWith(::std::wstring(L""));
  EXPECT_TRUE(m1.Matches(L"Hi"));
  EXPECT_TRUE(m1.Matches(L""));
  EXPECT_FALSE(m1.Matches(nullptr));

  const Matcher<const ::std::wstring&> m2 = StartsWith(L"Hi");
  EXPECT_TRUE(m2.Matches(L"Hi"));
  EXPECT_TRUE(m2.Matches(L"Hi Hi!"));
  EXPECT_TRUE(m2.Matches(L"High"));
  EXPECT_FALSE(m2.Matches(L"H"));
  EXPECT_FALSE(m2.Matches(L" Hi"));
}

TEST(StdWideStartsWithTest, CanDescribeSelf) {
  Matcher<const ::std::wstring> m = StartsWith(L"Hi");
  EXPECT_EQ("starts with L\"Hi\"", Describe(m));
}

// Tests EndsWith(s).

TEST(StdWideEndsWithTest, MatchesStringWithGivenSuffix) {
  const Matcher<const wchar_t*> m1 = EndsWith(L"");
  EXPECT_TRUE(m1.Matches(L"Hi"));
  EXPECT_TRUE(m1.Matches(L""));
  EXPECT_FALSE(m1.Matches(nullptr));

  const Matcher<const ::std::wstring&> m2 = EndsWith(::std::wstring(L"Hi"));
  EXPECT_TRUE(m2.Matches(L"Hi"));
  EXPECT_TRUE(m2.Matches(L"Wow Hi Hi"));
  EXPECT_TRUE(m2.Matches(L"Super Hi"));
  EXPECT_FALSE(m2.Matches(L"i"));
  EXPECT_FALSE(m2.Matches(L"Hi "));
}

TEST(StdWideEndsWithTest, CanDescribeSelf) {
  Matcher<const ::std::wstring> m = EndsWith(L"Hi");
  EXPECT_EQ("ends with L\"Hi\"", Describe(m));
}

#endif  // GTEST_HAS_STD_WSTRING

typedef ::std::tuple<long, int> Tuple2;  // NOLINT

// Tests that Eq() matches a 2-tuple where the first field == the
// second field.
TEST(Eq2Test, MatchesEqualArguments) {
  Matcher<const Tuple2&> m = Eq();
  EXPECT_TRUE(m.Matches(Tuple2(5L, 5)));
  EXPECT_FALSE(m.Matches(Tuple2(5L, 6)));
}

// Tests that Eq() describes itself properly.
TEST(Eq2Test, CanDescribeSelf) {
  Matcher<const Tuple2&> m = Eq();
  EXPECT_EQ("are an equal pair", Describe(m));
}

// Tests that Ge() matches a 2-tuple where the first field >= the
// second field.
TEST(Ge2Test, MatchesGreaterThanOrEqualArguments) {
  Matcher<const Tuple2&> m = Ge();
  EXPECT_TRUE(m.Matches(Tuple2(5L, 4)));
  EXPECT_TRUE(m.Matches(Tuple2(5L, 5)));
  EXPECT_FALSE(m.Matches(Tuple2(5L, 6)));
}

// Tests that Ge() describes itself properly.
TEST(Ge2Test, CanDescribeSelf) {
  Matcher<const Tuple2&> m = Ge();
  EXPECT_EQ("are a pair where the first >= the second", Describe(m));
}

// Tests that Gt() matches a 2-tuple where the first field > the
// second field.
TEST(Gt2Test, MatchesGreaterThanArguments) {
  Matcher<const Tuple2&> m = Gt();
  EXPECT_TRUE(m.Matches(Tuple2(5L, 4)));
  EXPECT_FALSE(m.Matches(Tuple2(5L, 5)));
  EXPECT_FALSE(m.Matches(Tuple2(5L, 6)));
}

// Tests that Gt() describes itself properly.
TEST(Gt2Test, CanDescribeSelf) {
  Matcher<const Tuple2&> m = Gt();
  EXPECT_EQ("are a pair where the first > the second", Describe(m));
}

// Tests that Le() matches a 2-tuple where the first field <= the
// second field.
TEST(Le2Test, MatchesLessThanOrEqualArguments) {
  Matcher<const Tuple2&> m = Le();
  EXPECT_TRUE(m.Matches(Tuple2(5L, 6)));
  EXPECT_TRUE(m.Matches(Tuple2(5L, 5)));
  EXPECT_FALSE(m.Matches(Tuple2(5L, 4)));
}

// Tests that Le() describes itself properly.
TEST(Le2Test, CanDescribeSelf) {
  Matcher<const Tuple2&> m = Le();
  EXPECT_EQ("are a pair where the first <= the second", Describe(m));
}

// Tests that Lt() matches a 2-tuple where the first field < the
// second field.
TEST(Lt2Test, MatchesLessThanArguments) {
  Matcher<const Tuple2&> m = Lt();
  EXPECT_TRUE(m.Matches(Tuple2(5L, 6)));
  EXPECT_FALSE(m.Matches(Tuple2(5L, 5)));
  EXPECT_FALSE(m.Matches(Tuple2(5L, 4)));
}

// Tests that Lt() describes itself properly.
TEST(Lt2Test, CanDescribeSelf) {
  Matcher<const Tuple2&> m = Lt();
  EXPECT_EQ("are a pair where the first < the second", Describe(m));
}

// Tests that Ne() matches a 2-tuple where the first field != the
// second field.
TEST(Ne2Test, MatchesUnequalArguments) {
  Matcher<const Tuple2&> m = Ne();
  EXPECT_TRUE(m.Matches(Tuple2(5L, 6)));
  EXPECT_TRUE(m.Matches(Tuple2(5L, 4)));
  EXPECT_FALSE(m.Matches(Tuple2(5L, 5)));
}

// Tests that Ne() describes itself properly.
TEST(Ne2Test, CanDescribeSelf) {
  Matcher<const Tuple2&> m = Ne();
  EXPECT_EQ("are an unequal pair", Describe(m));
}

TEST(PairMatchBaseTest, WorksWithMoveOnly) {
  using Pointers = std::tuple<std::unique_ptr<int>, std::unique_ptr<int>>;
  Matcher<Pointers> matcher = Eq();
  Pointers pointers;
  // Tested values don't matter; the point is that matcher does not copy the
  // matched values.
  EXPECT_TRUE(matcher.Matches(pointers));
}

// Tests that IsNan() matches a NaN, with float.
TEST(IsNan, FloatMatchesNan) {
  float quiet_nan = std::numeric_limits<float>::quiet_NaN();
  float other_nan = std::nanf("1");
  float real_value = 1.0f;

  Matcher<float> m = IsNan();
  EXPECT_TRUE(m.Matches(quiet_nan));
  EXPECT_TRUE(m.Matches(other_nan));
  EXPECT_FALSE(m.Matches(real_value));

  Matcher<float&> m_ref = IsNan();
  EXPECT_TRUE(m_ref.Matches(quiet_nan));
  EXPECT_TRUE(m_ref.Matches(other_nan));
  EXPECT_FALSE(m_ref.Matches(real_value));

  Matcher<const float&> m_cref = IsNan();
  EXPECT_TRUE(m_cref.Matches(quiet_nan));
  EXPECT_TRUE(m_cref.Matches(other_nan));
  EXPECT_FALSE(m_cref.Matches(real_value));
}

// Tests that IsNan() matches a NaN, with double.
TEST(IsNan, DoubleMatchesNan) {
  double quiet_nan = std::numeric_limits<double>::quiet_NaN();
  double other_nan = std::nan("1");
  double real_value = 1.0;

  Matcher<double> m = IsNan();
  EXPECT_TRUE(m.Matches(quiet_nan));
  EXPECT_TRUE(m.Matches(other_nan));
  EXPECT_FALSE(m.Matches(real_value));

  Matcher<double&> m_ref = IsNan();
  EXPECT_TRUE(m_ref.Matches(quiet_nan));
  EXPECT_TRUE(m_ref.Matches(other_nan));
  EXPECT_FALSE(m_ref.Matches(real_value));

  Matcher<const double&> m_cref = IsNan();
  EXPECT_TRUE(m_cref.Matches(quiet_nan));
  EXPECT_TRUE(m_cref.Matches(other_nan));
  EXPECT_FALSE(m_cref.Matches(real_value));
}

// Tests that IsNan() matches a NaN, with long double.
TEST(IsNan, LongDoubleMatchesNan) {
  long double quiet_nan = std::numeric_limits<long double>::quiet_NaN();
  long double other_nan = std::nan("1");
  long double real_value = 1.0;

  Matcher<long double> m = IsNan();
  EXPECT_TRUE(m.Matches(quiet_nan));
  EXPECT_TRUE(m.Matches(other_nan));
  EXPECT_FALSE(m.Matches(real_value));

  Matcher<long double&> m_ref = IsNan();
  EXPECT_TRUE(m_ref.Matches(quiet_nan));
  EXPECT_TRUE(m_ref.Matches(other_nan));
  EXPECT_FALSE(m_ref.Matches(real_value));

  Matcher<const long double&> m_cref = IsNan();
  EXPECT_TRUE(m_cref.Matches(quiet_nan));
  EXPECT_TRUE(m_cref.Matches(other_nan));
  EXPECT_FALSE(m_cref.Matches(real_value));
}

// Tests that IsNan() works with Not.
TEST(IsNan, NotMatchesNan) {
  Matcher<float> mf = Not(IsNan());
  EXPECT_FALSE(mf.Matches(std::numeric_limits<float>::quiet_NaN()));
  EXPECT_FALSE(mf.Matches(std::nanf("1")));
  EXPECT_TRUE(mf.Matches(1.0));

  Matcher<double> md = Not(IsNan());
  EXPECT_FALSE(md.Matches(std::numeric_limits<double>::quiet_NaN()));
  EXPECT_FALSE(md.Matches(std::nan("1")));
  EXPECT_TRUE(md.Matches(1.0));

  Matcher<long double> mld = Not(IsNan());
  EXPECT_FALSE(mld.Matches(std::numeric_limits<long double>::quiet_NaN()));
  EXPECT_FALSE(mld.Matches(std::nanl("1")));
  EXPECT_TRUE(mld.Matches(1.0));
}

// Tests that IsNan() can describe itself.
TEST(IsNan, CanDescribeSelf) {
  Matcher<float> mf = IsNan();
  EXPECT_EQ("is NaN", Describe(mf));

  Matcher<double> md = IsNan();
  EXPECT_EQ("is NaN", Describe(md));

  Matcher<long double> mld = IsNan();
  EXPECT_EQ("is NaN", Describe(mld));
}

// Tests that IsNan() can describe itself with Not.
TEST(IsNan, CanDescribeSelfWithNot) {
  Matcher<float> mf = Not(IsNan());
  EXPECT_EQ("isn't NaN", Describe(mf));

  Matcher<double> md = Not(IsNan());
  EXPECT_EQ("isn't NaN", Describe(md));

  Matcher<long double> mld = Not(IsNan());
  EXPECT_EQ("isn't NaN", Describe(mld));
}

// Tests that FloatEq() matches a 2-tuple where
// FloatEq(first field) matches the second field.
TEST(FloatEq2Test, MatchesEqualArguments) {
  typedef ::std::tuple<float, float> Tpl;
  Matcher<const Tpl&> m = FloatEq();
  EXPECT_TRUE(m.Matches(Tpl(1.0f, 1.0f)));
  EXPECT_TRUE(m.Matches(Tpl(0.3f, 0.1f + 0.1f + 0.1f)));
  EXPECT_FALSE(m.Matches(Tpl(1.1f, 1.0f)));
}

// Tests that FloatEq() describes itself properly.
TEST(FloatEq2Test, CanDescribeSelf) {
  Matcher<const ::std::tuple<float, float>&> m = FloatEq();
  EXPECT_EQ("are an almost-equal pair", Describe(m));
}

// Tests that NanSensitiveFloatEq() matches a 2-tuple where
// NanSensitiveFloatEq(first field) matches the second field.
TEST(NanSensitiveFloatEqTest, MatchesEqualArgumentsWithNaN) {
  typedef ::std::tuple<float, float> Tpl;
  Matcher<const Tpl&> m = NanSensitiveFloatEq();
  EXPECT_TRUE(m.Matches(Tpl(1.0f, 1.0f)));
  EXPECT_TRUE(m.Matches(Tpl(std::numeric_limits<float>::quiet_NaN(),
                            std::numeric_limits<float>::quiet_NaN())));
  EXPECT_FALSE(m.Matches(Tpl(1.1f, 1.0f)));
  EXPECT_FALSE(m.Matches(Tpl(1.0f, std::numeric_limits<float>::quiet_NaN())));
  EXPECT_FALSE(m.Matches(Tpl(std::numeric_limits<float>::quiet_NaN(), 1.0f)));
}

// Tests that NanSensitiveFloatEq() describes itself properly.
TEST(NanSensitiveFloatEqTest, CanDescribeSelfWithNaNs) {
  Matcher<const ::std::tuple<float, float>&> m = NanSensitiveFloatEq();
  EXPECT_EQ("are an almost-equal pair", Describe(m));
}

// Tests that DoubleEq() matches a 2-tuple where
// DoubleEq(first field) matches the second field.
TEST(DoubleEq2Test, MatchesEqualArguments) {
  typedef ::std::tuple<double, double> Tpl;
  Matcher<const Tpl&> m = DoubleEq();
  EXPECT_TRUE(m.Matches(Tpl(1.0, 1.0)));
  EXPECT_TRUE(m.Matches(Tpl(0.3, 0.1 + 0.1 + 0.1)));
  EXPECT_FALSE(m.Matches(Tpl(1.1, 1.0)));
}

// Tests that DoubleEq() describes itself properly.
TEST(DoubleEq2Test, CanDescribeSelf) {
  Matcher<const ::std::tuple<double, double>&> m = DoubleEq();
  EXPECT_EQ("are an almost-equal pair", Describe(m));
}

// Tests that NanSensitiveDoubleEq() matches a 2-tuple where
// NanSensitiveDoubleEq(first field) matches the second field.
TEST(NanSensitiveDoubleEqTest, MatchesEqualArgumentsWithNaN) {
  typedef ::std::tuple<double, double> Tpl;
  Matcher<const Tpl&> m = NanSensitiveDoubleEq();
  EXPECT_TRUE(m.Matches(Tpl(1.0f, 1.0f)));
  EXPECT_TRUE(m.Matches(Tpl(std::numeric_limits<double>::quiet_NaN(),
                            std::numeric_limits<double>::quiet_NaN())));
  EXPECT_FALSE(m.Matches(Tpl(1.1f, 1.0f)));
  EXPECT_FALSE(m.Matches(Tpl(1.0f, std::numeric_limits<double>::quiet_NaN())));
  EXPECT_FALSE(m.Matches(Tpl(std::numeric_limits<double>::quiet_NaN(), 1.0f)));
}

// Tests that DoubleEq() describes itself properly.
TEST(NanSensitiveDoubleEqTest, CanDescribeSelfWithNaNs) {
  Matcher<const ::std::tuple<double, double>&> m = NanSensitiveDoubleEq();
  EXPECT_EQ("are an almost-equal pair", Describe(m));
}

// Tests that FloatEq() matches a 2-tuple where
// FloatNear(first field, max_abs_error) matches the second field.
TEST(FloatNear2Test, MatchesEqualArguments) {
  typedef ::std::tuple<float, float> Tpl;
  Matcher<const Tpl&> m = FloatNear(0.5f);
  EXPECT_TRUE(m.Matches(Tpl(1.0f, 1.0f)));
  EXPECT_TRUE(m.Matches(Tpl(1.3f, 1.0f)));
  EXPECT_FALSE(m.Matches(Tpl(1.8f, 1.0f)));
}

// Tests that FloatNear() describes itself properly.
TEST(FloatNear2Test, CanDescribeSelf) {
  Matcher<const ::std::tuple<float, float>&> m = FloatNear(0.5f);
  EXPECT_EQ("are an almost-equal pair", Describe(m));
}

// Tests that NanSensitiveFloatNear() matches a 2-tuple where
// NanSensitiveFloatNear(first field) matches the second field.
TEST(NanSensitiveFloatNearTest, MatchesNearbyArgumentsWithNaN) {
  typedef ::std::tuple<float, float> Tpl;
  Matcher<const Tpl&> m = NanSensitiveFloatNear(0.5f);
  EXPECT_TRUE(m.Matches(Tpl(1.0f, 1.0f)));
  EXPECT_TRUE(m.Matches(Tpl(1.1f, 1.0f)));
  EXPECT_TRUE(m.Matches(Tpl(std::numeric_limits<float>::quiet_NaN(),
                            std::numeric_limits<float>::quiet_NaN())));
  EXPECT_FALSE(m.Matches(Tpl(1.6f, 1.0f)));
  EXPECT_FALSE(m.Matches(Tpl(1.0f, std::numeric_limits<float>::quiet_NaN())));
  EXPECT_FALSE(m.Matches(Tpl(std::numeric_limits<float>::quiet_NaN(), 1.0f)));
}

// Tests that NanSensitiveFloatNear() describes itself properly.
TEST(NanSensitiveFloatNearTest, CanDescribeSelfWithNaNs) {
  Matcher<const ::std::tuple<float, float>&> m = NanSensitiveFloatNear(0.5f);
  EXPECT_EQ("are an almost-equal pair", Describe(m));
}

// Tests that FloatEq() matches a 2-tuple where
// DoubleNear(first field, max_abs_error) matches the second field.
TEST(DoubleNear2Test, MatchesEqualArguments) {
  typedef ::std::tuple<double, double> Tpl;
  Matcher<const Tpl&> m = DoubleNear(0.5);
  EXPECT_TRUE(m.Matches(Tpl(1.0, 1.0)));
  EXPECT_TRUE(m.Matches(Tpl(1.3, 1.0)));
  EXPECT_FALSE(m.Matches(Tpl(1.8, 1.0)));
}

// Tests that DoubleNear() describes itself properly.
TEST(DoubleNear2Test, CanDescribeSelf) {
  Matcher<const ::std::tuple<double, double>&> m = DoubleNear(0.5);
  EXPECT_EQ("are an almost-equal pair", Describe(m));
}

// Tests that NanSensitiveDoubleNear() matches a 2-tuple where
// NanSensitiveDoubleNear(first field) matches the second field.
TEST(NanSensitiveDoubleNearTest, MatchesNearbyArgumentsWithNaN) {
  typedef ::std::tuple<double, double> Tpl;
  Matcher<const Tpl&> m = NanSensitiveDoubleNear(0.5f);
  EXPECT_TRUE(m.Matches(Tpl(1.0f, 1.0f)));
  EXPECT_TRUE(m.Matches(Tpl(1.1f, 1.0f)));
  EXPECT_TRUE(m.Matches(Tpl(std::numeric_limits<double>::quiet_NaN(),
                            std::numeric_limits<double>::quiet_NaN())));
  EXPECT_FALSE(m.Matches(Tpl(1.6f, 1.0f)));
  EXPECT_FALSE(m.Matches(Tpl(1.0f, std::numeric_limits<double>::quiet_NaN())));
  EXPECT_FALSE(m.Matches(Tpl(std::numeric_limits<double>::quiet_NaN(), 1.0f)));
}

// Tests that NanSensitiveDoubleNear() describes itself properly.
TEST(NanSensitiveDoubleNearTest, CanDescribeSelfWithNaNs) {
  Matcher<const ::std::tuple<double, double>&> m = NanSensitiveDoubleNear(0.5f);
  EXPECT_EQ("are an almost-equal pair", Describe(m));
}

// Tests that Not(m) matches any value that doesn't match m.
TEST(NotTest, NegatesMatcher) {
  Matcher<int> m;
  m = Not(Eq(2));
  EXPECT_TRUE(m.Matches(3));
  EXPECT_FALSE(m.Matches(2));
}

// Tests that Not(m) describes itself properly.
TEST(NotTest, CanDescribeSelf) {
  Matcher<int> m = Not(Eq(5));
  EXPECT_EQ("isn't equal to 5", Describe(m));
}

// Tests that monomorphic matchers are safely cast by the Not matcher.
TEST(NotTest, NotMatcherSafelyCastsMonomorphicMatchers) {
  // greater_than_5 is a monomorphic matcher.
  Matcher<int> greater_than_5 = Gt(5);

  Matcher<const int&> m = Not(greater_than_5);
  Matcher<int&> m2 = Not(greater_than_5);
  Matcher<int&> m3 = Not(m);
}

// Helper to allow easy testing of AllOf matchers with num parameters.
void AllOfMatches(int num, const Matcher<int>& m) {
  SCOPED_TRACE(Describe(m));
  EXPECT_TRUE(m.Matches(0));
  for (int i = 1; i <= num; ++i) {
    EXPECT_FALSE(m.Matches(i));
  }
  EXPECT_TRUE(m.Matches(num + 1));
}

// Tests that AllOf(m1, ..., mn) matches any value that matches all of
// the given matchers.
TEST(AllOfTest, MatchesWhenAllMatch) {
  Matcher<int> m;
  m = AllOf(Le(2), Ge(1));
  EXPECT_TRUE(m.Matches(1));
  EXPECT_TRUE(m.Matches(2));
  EXPECT_FALSE(m.Matches(0));
  EXPECT_FALSE(m.Matches(3));

  m = AllOf(Gt(0), Ne(1), Ne(2));
  EXPECT_TRUE(m.Matches(3));
  EXPECT_FALSE(m.Matches(2));
  EXPECT_FALSE(m.Matches(1));
  EXPECT_FALSE(m.Matches(0));

  m = AllOf(Gt(0), Ne(1), Ne(2), Ne(3));
  EXPECT_TRUE(m.Matches(4));
  EXPECT_FALSE(m.Matches(3));
  EXPECT_FALSE(m.Matches(2));
  EXPECT_FALSE(m.Matches(1));
  EXPECT_FALSE(m.Matches(0));

  m = AllOf(Ge(0), Lt(10), Ne(3), Ne(5), Ne(7));
  EXPECT_TRUE(m.Matches(0));
  EXPECT_TRUE(m.Matches(1));
  EXPECT_FALSE(m.Matches(3));

  // The following tests for varying number of sub-matchers. Due to the way
  // the sub-matchers are handled it is enough to test every sub-matcher once
  // with sub-matchers using the same matcher type. Varying matcher types are
  // checked for above.
  AllOfMatches(2, AllOf(Ne(1), Ne(2)));
  AllOfMatches(3, AllOf(Ne(1), Ne(2), Ne(3)));
  AllOfMatches(4, AllOf(Ne(1), Ne(2), Ne(3), Ne(4)));
  AllOfMatches(5, AllOf(Ne(1), Ne(2), Ne(3), Ne(4), Ne(5)));
  AllOfMatches(6, AllOf(Ne(1), Ne(2), Ne(3), Ne(4), Ne(5), Ne(6)));
  AllOfMatches(7, AllOf(Ne(1), Ne(2), Ne(3), Ne(4), Ne(5), Ne(6), Ne(7)));
  AllOfMatches(8, AllOf(Ne(1), Ne(2), Ne(3), Ne(4), Ne(5), Ne(6), Ne(7),
                        Ne(8)));
  AllOfMatches(9, AllOf(Ne(1), Ne(2), Ne(3), Ne(4), Ne(5), Ne(6), Ne(7),
                        Ne(8), Ne(9)));
  AllOfMatches(10, AllOf(Ne(1), Ne(2), Ne(3), Ne(4), Ne(5), Ne(6), Ne(7), Ne(8),
                         Ne(9), Ne(10)));
  AllOfMatches(
      50, AllOf(Ne(1), Ne(2), Ne(3), Ne(4), Ne(5), Ne(6), Ne(7), Ne(8), Ne(9),
                Ne(10), Ne(11), Ne(12), Ne(13), Ne(14), Ne(15), Ne(16), Ne(17),
                Ne(18), Ne(19), Ne(20), Ne(21), Ne(22), Ne(23), Ne(24), Ne(25),
                Ne(26), Ne(27), Ne(28), Ne(29), Ne(30), Ne(31), Ne(32), Ne(33),
                Ne(34), Ne(35), Ne(36), Ne(37), Ne(38), Ne(39), Ne(40), Ne(41),
                Ne(42), Ne(43), Ne(44), Ne(45), Ne(46), Ne(47), Ne(48), Ne(49),
                Ne(50)));
}


// Tests that AllOf(m1, ..., mn) describes itself properly.
TEST(AllOfTest, CanDescribeSelf) {
  Matcher<int> m;
  m = AllOf(Le(2), Ge(1));
  EXPECT_EQ("(is <= 2) and (is >= 1)", Describe(m));

  m = AllOf(Gt(0), Ne(1), Ne(2));
  std::string expected_descr1 =
      "(is > 0) and (isn't equal to 1) and (isn't equal to 2)";
  EXPECT_EQ(expected_descr1, Describe(m));

  m = AllOf(Gt(0), Ne(1), Ne(2), Ne(3));
  std::string expected_descr2 =
      "(is > 0) and (isn't equal to 1) and (isn't equal to 2) and (isn't equal "
      "to 3)";
  EXPECT_EQ(expected_descr2, Describe(m));

  m = AllOf(Ge(0), Lt(10), Ne(3), Ne(5), Ne(7));
  std::string expected_descr3 =
      "(is >= 0) and (is < 10) and (isn't equal to 3) and (isn't equal to 5) "
      "and (isn't equal to 7)";
  EXPECT_EQ(expected_descr3, Describe(m));
}

// Tests that AllOf(m1, ..., mn) describes its negation properly.
TEST(AllOfTest, CanDescribeNegation) {
  Matcher<int> m;
  m = AllOf(Le(2), Ge(1));
  std::string expected_descr4 = "(isn't <= 2) or (isn't >= 1)";
  EXPECT_EQ(expected_descr4, DescribeNegation(m));

  m = AllOf(Gt(0), Ne(1), Ne(2));
  std::string expected_descr5 =
      "(isn't > 0) or (is equal to 1) or (is equal to 2)";
  EXPECT_EQ(expected_descr5, DescribeNegation(m));

  m = AllOf(Gt(0), Ne(1), Ne(2), Ne(3));
  std::string expected_descr6 =
      "(isn't > 0) or (is equal to 1) or (is equal to 2) or (is equal to 3)";
  EXPECT_EQ(expected_descr6, DescribeNegation(m));

  m = AllOf(Ge(0), Lt(10), Ne(3), Ne(5), Ne(7));
  std::string expected_desr7 =
      "(isn't >= 0) or (isn't < 10) or (is equal to 3) or (is equal to 5) or "
      "(is equal to 7)";
  EXPECT_EQ(expected_desr7, DescribeNegation(m));

  m = AllOf(Ne(1), Ne(2), Ne(3), Ne(4), Ne(5), Ne(6), Ne(7), Ne(8), Ne(9),
            Ne(10), Ne(11));
  AllOf(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
  EXPECT_THAT(Describe(m), EndsWith("and (isn't equal to 11)"));
  AllOfMatches(11, m);
}

// Tests that monomorphic matchers are safely cast by the AllOf matcher.
TEST(AllOfTest, AllOfMatcherSafelyCastsMonomorphicMatchers) {
  // greater_than_5 and less_than_10 are monomorphic matchers.
  Matcher<int> greater_than_5 = Gt(5);
  Matcher<int> less_than_10 = Lt(10);

  Matcher<const int&> m = AllOf(greater_than_5, less_than_10);
  Matcher<int&> m2 = AllOf(greater_than_5, less_than_10);
  Matcher<int&> m3 = AllOf(greater_than_5, m2);

  // Tests that BothOf works when composing itself.
  Matcher<const int&> m4 = AllOf(greater_than_5, less_than_10, less_than_10);
  Matcher<int&> m5 = AllOf(greater_than_5, less_than_10, less_than_10);
}

TEST(AllOfTest, ExplainsResult) {
  Matcher<int> m;

  // Successful match.  Both matchers need to explain.  The second
  // matcher doesn't give an explanation, so only the first matcher's
  // explanation is printed.
  m = AllOf(GreaterThan(10), Lt(30));
  EXPECT_EQ("which is 15 more than 10", Explain(m, 25));

  // Successful match.  Both matchers need to explain.
  m = AllOf(GreaterThan(10), GreaterThan(20));
  EXPECT_EQ("which is 20 more than 10, and which is 10 more than 20",
            Explain(m, 30));

  // Successful match.  All matchers need to explain.  The second
  // matcher doesn't given an explanation.
  m = AllOf(GreaterThan(10), Lt(30), GreaterThan(20));
  EXPECT_EQ("which is 15 more than 10, and which is 5 more than 20",
            Explain(m, 25));

  // Successful match.  All matchers need to explain.
  m = AllOf(GreaterThan(10), GreaterThan(20), GreaterThan(30));
  EXPECT_EQ("which is 30 more than 10, and which is 20 more than 20, "
            "and which is 10 more than 30",
            Explain(m, 40));

  // Failed match.  The first matcher, which failed, needs to
  // explain.
  m = AllOf(GreaterThan(10), GreaterThan(20));
  EXPECT_EQ("which is 5 less than 10", Explain(m, 5));

  // Failed match.  The second matcher, which failed, needs to
  // explain.  Since it doesn't given an explanation, nothing is
  // printed.
  m = AllOf(GreaterThan(10), Lt(30));
  EXPECT_EQ("", Explain(m, 40));

  // Failed match.  The second matcher, which failed, needs to
  // explain.
  m = AllOf(GreaterThan(10), GreaterThan(20));
  EXPECT_EQ("which is 5 less than 20", Explain(m, 15));
}

// Helper to allow easy testing of AnyOf matchers with num parameters.
static void AnyOfMatches(int num, const Matcher<int>& m) {
  SCOPED_TRACE(Describe(m));
  EXPECT_FALSE(m.Matches(0));
  for (int i = 1; i <= num; ++i) {
    EXPECT_TRUE(m.Matches(i));
  }
  EXPECT_FALSE(m.Matches(num + 1));
}

static void AnyOfStringMatches(int num, const Matcher<std::string>& m) {
  SCOPED_TRACE(Describe(m));
  EXPECT_FALSE(m.Matches(std::to_string(0)));

  for (int i = 1; i <= num; ++i) {
    EXPECT_TRUE(m.Matches(std::to_string(i)));
  }
  EXPECT_FALSE(m.Matches(std::to_string(num + 1)));
}

// Tests that AnyOf(m1, ..., mn) matches any value that matches at
// least one of the given matchers.
TEST(AnyOfTest, MatchesWhenAnyMatches) {
  Matcher<int> m;
  m = AnyOf(Le(1), Ge(3));
  EXPECT_TRUE(m.Matches(1));
  EXPECT_TRUE(m.Matches(4));
  EXPECT_FALSE(m.Matches(2));

  m = AnyOf(Lt(0), Eq(1), Eq(2));
  EXPECT_TRUE(m.Matches(-1));
  EXPECT_TRUE(m.Matches(1));
  EXPECT_TRUE(m.Matches(2));
  EXPECT_FALSE(m.Matches(0));

  m = AnyOf(Lt(0), Eq(1), Eq(2), Eq(3));
  EXPECT_TRUE(m.Matches(-1));
  EXPECT_TRUE(m.Matches(1));
  EXPECT_TRUE(m.Matches(2));
  EXPECT_TRUE(m.Matches(3));
  EXPECT_FALSE(m.Matches(0));

  m = AnyOf(Le(0), Gt(10), 3, 5, 7);
  EXPECT_TRUE(m.Matches(0));
  EXPECT_TRUE(m.Matches(11));
  EXPECT_TRUE(m.Matches(3));
  EXPECT_FALSE(m.Matches(2));

  // The following tests for varying number of sub-matchers. Due to the way
  // the sub-matchers are handled it is enough to test every sub-matcher once
  // with sub-matchers using the same matcher type. Varying matcher types are
  // checked for above.
  AnyOfMatches(2, AnyOf(1, 2));
  AnyOfMatches(3, AnyOf(1, 2, 3));
  AnyOfMatches(4, AnyOf(1, 2, 3, 4));
  AnyOfMatches(5, AnyOf(1, 2, 3, 4, 5));
  AnyOfMatches(6, AnyOf(1, 2, 3, 4, 5, 6));
  AnyOfMatches(7, AnyOf(1, 2, 3, 4, 5, 6, 7));
  AnyOfMatches(8, AnyOf(1, 2, 3, 4, 5, 6, 7, 8));
  AnyOfMatches(9, AnyOf(1, 2, 3, 4, 5, 6, 7, 8, 9));
  AnyOfMatches(10, AnyOf(1, 2, 3, 4, 5, 6, 7, 8, 9, 10));
}

// Tests the variadic version of the AnyOfMatcher.
TEST(AnyOfTest, VariadicMatchesWhenAnyMatches) {
  // Also make sure AnyOf is defined in the right namespace and does not depend
  // on ADL.
  Matcher<int> m = ::testing::AnyOf(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);

  EXPECT_THAT(Describe(m), EndsWith("or (is equal to 11)"));
  AnyOfMatches(11, m);
  AnyOfMatches(50, AnyOf(1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
                         11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
                         21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
                         31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
                         41, 42, 43, 44, 45, 46, 47, 48, 49, 50));
  AnyOfStringMatches(
      50, AnyOf("1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12",
                "13", "14", "15", "16", "17", "18", "19", "20", "21", "22",
                "23", "24", "25", "26", "27", "28", "29", "30", "31", "32",
                "33", "34", "35", "36", "37", "38", "39", "40", "41", "42",
                "43", "44", "45", "46", "47", "48", "49", "50"));
}

// Tests the variadic version of the ElementsAreMatcher
TEST(ElementsAreTest, HugeMatcher) {
  vector<int> test_vector{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

  EXPECT_THAT(test_vector,
              ElementsAre(Eq(1), Eq(2), Lt(13), Eq(4), Eq(5), Eq(6), Eq(7),
                          Eq(8), Eq(9), Eq(10), Gt(1), Eq(12)));
}

// Tests the variadic version of the UnorderedElementsAreMatcher
TEST(ElementsAreTest, HugeMatcherStr) {
  vector<std::string> test_vector{
      "literal_string", "", "", "", "", "", "", "", "", "", "", ""};

  EXPECT_THAT(test_vector, UnorderedElementsAre("literal_string", _, _, _, _, _,
                                                _, _, _, _, _, _));
}

// Tests the variadic version of the UnorderedElementsAreMatcher
TEST(ElementsAreTest, HugeMatcherUnordered) {
  vector<int> test_vector{2, 1, 8, 5, 4, 6, 7, 3, 9, 12, 11, 10};

  EXPECT_THAT(test_vector, UnorderedElementsAre(
                               Eq(2), Eq(1), Gt(7), Eq(5), Eq(4), Eq(6), Eq(7),
                               Eq(3), Eq(9), Eq(12), Eq(11), Ne(122)));
}


// Tests that AnyOf(m1, ..., mn) describes itself properly.
TEST(AnyOfTest, CanDescribeSelf) {
  Matcher<int> m;
  m = AnyOf(Le(1), Ge(3));

  EXPECT_EQ("(is <= 1) or (is >= 3)",
            Describe(m));

  m = AnyOf(Lt(0), Eq(1), Eq(2));
  EXPECT_EQ("(is < 0) or (is equal to 1) or (is equal to 2)", Describe(m));

  m = AnyOf(Lt(0), Eq(1), Eq(2), Eq(3));
  EXPECT_EQ("(is < 0) or (is equal to 1) or (is equal to 2) or (is equal to 3)",
            Describe(m));

  m = AnyOf(Le(0), Gt(10), 3, 5, 7);
  EXPECT_EQ(
      "(is <= 0) or (is > 10) or (is equal to 3) or (is equal to 5) or (is "
      "equal to 7)",
      Describe(m));
}

// Tests that AnyOf(m1, ..., mn) describes its negation properly.
TEST(AnyOfTest, CanDescribeNegation) {
  Matcher<int> m;
  m = AnyOf(Le(1), Ge(3));
  EXPECT_EQ("(isn't <= 1) and (isn't >= 3)",
            DescribeNegation(m));

  m = AnyOf(Lt(0), Eq(1), Eq(2));
  EXPECT_EQ("(isn't < 0) and (isn't equal to 1) and (isn't equal to 2)",
            DescribeNegation(m));

  m = AnyOf(Lt(0), Eq(1), Eq(2), Eq(3));
  EXPECT_EQ(
      "(isn't < 0) and (isn't equal to 1) and (isn't equal to 2) and (isn't "
      "equal to 3)",
      DescribeNegation(m));

  m = AnyOf(Le(0), Gt(10), 3, 5, 7);
  EXPECT_EQ(
      "(isn't <= 0) and (isn't > 10) and (isn't equal to 3) and (isn't equal "
      "to 5) and (isn't equal to 7)",
      DescribeNegation(m));
}

// Tests that monomorphic matchers are safely cast by the AnyOf matcher.
TEST(AnyOfTest, AnyOfMatcherSafelyCastsMonomorphicMatchers) {
  // greater_than_5 and less_than_10 are monomorphic matchers.
  Matcher<int> greater_than_5 = Gt(5);
  Matcher<int> less_than_10 = Lt(10);

  Matcher<const int&> m = AnyOf(greater_than_5, less_than_10);
  Matcher<int&> m2 = AnyOf(greater_than_5, less_than_10);
  Matcher<int&> m3 = AnyOf(greater_than_5, m2);

  // Tests that EitherOf works when composing itself.
  Matcher<const int&> m4 = AnyOf(greater_than_5, less_than_10, less_than_10);
  Matcher<int&> m5 = AnyOf(greater_than_5, less_than_10, less_than_10);
}

TEST(AnyOfTest, ExplainsResult) {
  Matcher<int> m;

  // Failed match.  Both matchers need to explain.  The second
  // matcher doesn't give an explanation, so only the first matcher's
  // explanation is printed.
  m = AnyOf(GreaterThan(10), Lt(0));
  EXPECT_EQ("which is 5 less than 10", Explain(m, 5));

  // Failed match.  Both matchers need to explain.
  m = AnyOf(GreaterThan(10), GreaterThan(20));
  EXPECT_EQ("which is 5 less than 10, and which is 15 less than 20",
            Explain(m, 5));

  // Failed match.  All matchers need to explain.  The second
  // matcher doesn't given an explanation.
  m = AnyOf(GreaterThan(10), Gt(20), GreaterThan(30));
  EXPECT_EQ("which is 5 less than 10, and which is 25 less than 30",
            Explain(m, 5));

  // Failed match.  All matchers need to explain.
  m = AnyOf(GreaterThan(10), GreaterThan(20), GreaterThan(30));
  EXPECT_EQ("which is 5 less than 10, and which is 15 less than 20, "
            "and which is 25 less than 30",
            Explain(m, 5));

  // Successful match.  The first matcher, which succeeded, needs to
  // explain.
  m = AnyOf(GreaterThan(10), GreaterThan(20));
  EXPECT_EQ("which is 5 more than 10", Explain(m, 15));

  // Successful match.  The second matcher, which succeeded, needs to
  // explain.  Since it doesn't given an explanation, nothing is
  // printed.
  m = AnyOf(GreaterThan(10), Lt(30));
  EXPECT_EQ("", Explain(m, 0));

  // Successful match.  The second matcher, which succeeded, needs to
  // explain.
  m = AnyOf(GreaterThan(30), GreaterThan(20));
  EXPECT_EQ("which is 5 more than 20", Explain(m, 25));
}

// The following predicate function and predicate functor are for
// testing the Truly(predicate) matcher.

// Returns non-zero if the input is positive.  Note that the return
// type of this function is not bool.  It's OK as Truly() accepts any
// unary function or functor whose return type can be implicitly
// converted to bool.
int IsPositive(double x) {
  return x > 0 ? 1 : 0;
}

// This functor returns true if the input is greater than the given
// number.
class IsGreaterThan {
 public:
  explicit IsGreaterThan(int threshold) : threshold_(threshold) {}

  bool operator()(int n) const { return n > threshold_; }

 private:
  int threshold_;
};

// For testing Truly().
const int foo = 0;

// This predicate returns true if and only if the argument references foo and
// has a zero value.
bool ReferencesFooAndIsZero(const int& n) {
  return (&n == &foo) && (n == 0);
}

// Tests that Truly(predicate) matches what satisfies the given
// predicate.
TEST(TrulyTest, MatchesWhatSatisfiesThePredicate) {
  Matcher<double> m = Truly(IsPositive);
  EXPECT_TRUE(m.Matches(2.0));
  EXPECT_FALSE(m.Matches(-1.5));
}

// Tests that Truly(predicate_functor) works too.
TEST(TrulyTest, CanBeUsedWithFunctor) {
  Matcher<int> m = Truly(IsGreaterThan(5));
  EXPECT_TRUE(m.Matches(6));
  EXPECT_FALSE(m.Matches(4));
}

// A class that can be implicitly converted to bool.
class ConvertibleToBool {
 public:
  explicit ConvertibleToBool(int number) : number_(number) {}
  operator bool() const { return number_ != 0; }

 private:
  int number_;
};

ConvertibleToBool IsNotZero(int number) {
  return ConvertibleToBool(number);
}

// Tests that the predicate used in Truly() may return a class that's
// implicitly convertible to bool, even when the class has no
// operator!().
TEST(TrulyTest, PredicateCanReturnAClassConvertibleToBool) {
  Matcher<int> m = Truly(IsNotZero);
  EXPECT_TRUE(m.Matches(1));
  EXPECT_FALSE(m.Matches(0));
}

// Tests that Truly(predicate) can describe itself properly.
TEST(TrulyTest, CanDescribeSelf) {
  Matcher<double> m = Truly(IsPositive);
  EXPECT_EQ("satisfies the given predicate",
            Describe(m));
}

// Tests that Truly(predicate) works when the matcher takes its
// argument by reference.
TEST(TrulyTest, WorksForByRefArguments) {
  Matcher<const int&> m = Truly(ReferencesFooAndIsZero);
  EXPECT_TRUE(m.Matches(foo));
  int n = 0;
  EXPECT_FALSE(m.Matches(n));
}

// Tests that Truly(predicate) provides a helpful reason when it fails.
TEST(TrulyTest, ExplainsFailures) {
  StringMatchResultListener listener;
  EXPECT_FALSE(ExplainMatchResult(Truly(IsPositive), -1, &listener));
  EXPECT_EQ(listener.str(), "didn't satisfy the given predicate");
}

// Tests that Matches(m) is a predicate satisfied by whatever that
// matches matcher m.
TEST(MatchesTest, IsSatisfiedByWhatMatchesTheMatcher) {
  EXPECT_TRUE(Matches(Ge(0))(1));
  EXPECT_FALSE(Matches(Eq('a'))('b'));
}

// Tests that Matches(m) works when the matcher takes its argument by
// reference.
TEST(MatchesTest, WorksOnByRefArguments) {
  int m = 0, n = 0;
  EXPECT_TRUE(Matches(AllOf(Ref(n), Eq(0)))(n));
  EXPECT_FALSE(Matches(Ref(m))(n));
}

// Tests that a Matcher on non-reference type can be used in
// Matches().
TEST(MatchesTest, WorksWithMatcherOnNonRefType) {
  Matcher<int> eq5 = Eq(5);
  EXPECT_TRUE(Matches(eq5)(5));
  EXPECT_FALSE(Matches(eq5)(2));
}

// Tests Value(value, matcher).  Since Value() is a simple wrapper for
// Matches(), which has been tested already, we don't spend a lot of
// effort on testing Value().
TEST(ValueTest, WorksWithPolymorphicMatcher) {
  EXPECT_TRUE(Value("hi", StartsWith("h")));
  EXPECT_FALSE(Value(5, Gt(10)));
}

TEST(ValueTest, WorksWithMonomorphicMatcher) {
  const Matcher<int> is_zero = Eq(0);
  EXPECT_TRUE(Value(0, is_zero));
  EXPECT_FALSE(Value('a', is_zero));

  int n = 0;
  const Matcher<const int&> ref_n = Ref(n);
  EXPECT_TRUE(Value(n, ref_n));
  EXPECT_FALSE(Value(1, ref_n));
}

TEST(ExplainMatchResultTest, WorksWithPolymorphicMatcher) {
  StringMatchResultListener listener1;
  EXPECT_TRUE(ExplainMatchResult(PolymorphicIsEven(), 42, &listener1));
  EXPECT_EQ("% 2 == 0", listener1.str());

  StringMatchResultListener listener2;
  EXPECT_FALSE(ExplainMatchResult(Ge(42), 1.5, &listener2));
  EXPECT_EQ("", listener2.str());
}

TEST(ExplainMatchResultTest, WorksWithMonomorphicMatcher) {
  const Matcher<int> is_even = PolymorphicIsEven();
  StringMatchResultListener listener1;
  EXPECT_TRUE(ExplainMatchResult(is_even, 42, &listener1));
  EXPECT_EQ("% 2 == 0", listener1.str());

  const Matcher<const double&> is_zero = Eq(0);
  StringMatchResultListener listener2;
  EXPECT_FALSE(ExplainMatchResult(is_zero, 1.5, &listener2));
  EXPECT_EQ("", listener2.str());
}

MATCHER(ConstructNoArg, "") { return true; }
MATCHER_P(Construct1Arg, arg1, "") { return true; }
MATCHER_P2(Construct2Args, arg1, arg2, "") { return true; }

TEST(MatcherConstruct, ExplicitVsImplicit) {
  {
    // No arg constructor can be constructed with empty brace.
    ConstructNoArgMatcher m = {};
    (void)m;
    // And with no args
    ConstructNoArgMatcher m2;
    (void)m2;
  }
  {
    // The one arg constructor has an explicit constructor.
    // This is to prevent the implicit conversion.
    using M = Construct1ArgMatcherP<int>;
    EXPECT_TRUE((std::is_constructible<M, int>::value));
    EXPECT_FALSE((std::is_convertible<int, M>::value));
  }
  {
    // Multiple arg matchers can be constructed with an implicit construction.
    Construct2ArgsMatcherP2<int, double> m = {1, 2.2};
    (void)m;
  }
}

MATCHER_P(Really, inner_matcher, "") {
  return ExplainMatchResult(inner_matcher, arg, result_listener);
}

TEST(ExplainMatchResultTest, WorksInsideMATCHER) {
  EXPECT_THAT(0, Really(Eq(0)));
}

TEST(DescribeMatcherTest, WorksWithValue) {
  EXPECT_EQ("is equal to 42", DescribeMatcher<int>(42));
  EXPECT_EQ("isn't equal to 42", DescribeMatcher<int>(42, true));
}

TEST(DescribeMatcherTest, WorksWithMonomorphicMatcher) {
  const Matcher<int> monomorphic = Le(0);
  EXPECT_EQ("is <= 0", DescribeMatcher<int>(monomorphic));
  EXPECT_EQ("isn't <= 0", DescribeMatcher<int>(monomorphic, true));
}

TEST(DescribeMatcherTest, WorksWithPolymorphicMatcher) {
  EXPECT_EQ("is even", DescribeMatcher<int>(PolymorphicIsEven()));
  EXPECT_EQ("is odd", DescribeMatcher<int>(PolymorphicIsEven(), true));
}

TEST(AllArgsTest, WorksForTuple) {
  EXPECT_THAT(std::make_tuple(1, 2L), AllArgs(Lt()));
  EXPECT_THAT(std::make_tuple(2L, 1), Not(AllArgs(Lt())));
}

TEST(AllArgsTest, WorksForNonTuple) {
  EXPECT_THAT(42, AllArgs(Gt(0)));
  EXPECT_THAT('a', Not(AllArgs(Eq('b'))));
}

class AllArgsHelper {
 public:
  AllArgsHelper() {}

  MOCK_METHOD2(Helper, int(char x, int y));

 private:
  GTEST_DISALLOW_COPY_AND_ASSIGN_(AllArgsHelper);
};

TEST(AllArgsTest, WorksInWithClause) {
  AllArgsHelper helper;
  ON_CALL(helper, Helper(_, _))
      .With(AllArgs(Lt()))
      .WillByDefault(Return(1));
  EXPECT_CALL(helper, Helper(_, _));
  EXPECT_CALL(helper, Helper(_, _))
      .With(AllArgs(Gt()))
      .WillOnce(Return(2));

  EXPECT_EQ(1, helper.Helper('\1', 2));
  EXPECT_EQ(2, helper.Helper('a', 1));
}

class OptionalMatchersHelper {
 public:
  OptionalMatchersHelper() {}

  MOCK_METHOD0(NoArgs, int());

  MOCK_METHOD1(OneArg, int(int y));

  MOCK_METHOD2(TwoArgs, int(char x, int y));

  MOCK_METHOD1(Overloaded, int(char x));
  MOCK_METHOD2(Overloaded, int(char x, int y));

 private:
  GTEST_DISALLOW_COPY_AND_ASSIGN_(OptionalMatchersHelper);
};

TEST(AllArgsTest, WorksWithoutMatchers) {
  OptionalMatchersHelper helper;

  ON_CALL(helper, NoArgs).WillByDefault(Return(10));
  ON_CALL(helper, OneArg).WillByDefault(Return(20));
  ON_CALL(helper, TwoArgs).WillByDefault(Return(30));

  EXPECT_EQ(10, helper.NoArgs());
  EXPECT_EQ(20, helper.OneArg(1));
  EXPECT_EQ(30, helper.TwoArgs('\1', 2));

  EXPECT_CALL(helper, NoArgs).Times(1);
  EXPECT_CALL(helper, OneArg).WillOnce(Return(100));
  EXPECT_CALL(helper, OneArg(17)).WillOnce(Return(200));
  EXPECT_CALL(helper, TwoArgs).Times(0);

  EXPECT_EQ(10, helper.NoArgs());
  EXPECT_EQ(100, helper.OneArg(1));
  EXPECT_EQ(200, helper.OneArg(17));
}

// Tests that ASSERT_THAT() and EXPECT_THAT() work when the value
// matches the matcher.
TEST(MatcherAssertionTest, WorksWhenMatcherIsSatisfied) {
  ASSERT_THAT(5, Ge(2)) << "This should succeed.";
  ASSERT_THAT("Foo", EndsWith("oo"));
  EXPECT_THAT(2, AllOf(Le(7), Ge(0))) << "This should succeed too.";
  EXPECT_THAT("Hello", StartsWith("Hell"));
}

// Tests that ASSERT_THAT() and EXPECT_THAT() work when the value
// doesn't match the matcher.
TEST(MatcherAssertionTest, WorksWhenMatcherIsNotSatisfied) {
  // 'n' must be static as it is used in an EXPECT_FATAL_FAILURE(),
  // which cannot reference auto variables.
  static unsigned short n;  // NOLINT
  n = 5;

  EXPECT_FATAL_FAILURE(ASSERT_THAT(n, Gt(10)),
                       "Value of: n\n"
                       "Expected: is > 10\n"
                       "  Actual: 5" + OfType("unsigned short"));
  n = 0;
  EXPECT_NONFATAL_FAILURE(
      EXPECT_THAT(n, AllOf(Le(7), Ge(5))),
      "Value of: n\n"
      "Expected: (is <= 7) and (is >= 5)\n"
      "  Actual: 0" + OfType("unsigned short"));
}

// Tests that ASSERT_THAT() and EXPECT_THAT() work when the argument
// has a reference type.
TEST(MatcherAssertionTest, WorksForByRefArguments) {
  // We use a static variable here as EXPECT_FATAL_FAILURE() cannot
  // reference auto variables.
  static int n;
  n = 0;
  EXPECT_THAT(n, AllOf(Le(7), Ref(n)));
  EXPECT_FATAL_FAILURE(ASSERT_THAT(n, Not(Ref(n))),
                       "Value of: n\n"
                       "Expected: does not reference the variable @");
  // Tests the "Actual" part.
  EXPECT_FATAL_FAILURE(ASSERT_THAT(n, Not(Ref(n))),
                       "Actual: 0" + OfType("int") + ", which is located @");
}

// Tests that ASSERT_THAT() and EXPECT_THAT() work when the matcher is
// monomorphic.
TEST(MatcherAssertionTest, WorksForMonomorphicMatcher) {
  Matcher<const char*> starts_with_he = StartsWith("he");
  ASSERT_THAT("hello", starts_with_he);

  Matcher<const std::string&> ends_with_ok = EndsWith("ok");
  ASSERT_THAT("book", ends_with_ok);
  const std::string bad = "bad";
  EXPECT_NONFATAL_FAILURE(EXPECT_THAT(bad, ends_with_ok),
                          "Value of: bad\n"
                          "Expected: ends with \"ok\"\n"
                          "  Actual: \"bad\"");
  Matcher<int> is_greater_than_5 = Gt(5);
  EXPECT_NONFATAL_FAILURE(EXPECT_THAT(5, is_greater_than_5),
                          "Value of: 5\n"
                          "Expected: is > 5\n"
                          "  Actual: 5" + OfType("int"));
}

// Tests floating-point matchers.
template <typename RawType>
class FloatingPointTest : public testing::Test {
 protected:
  typedef testing::internal::FloatingPoint<RawType> Floating;
  typedef typename Floating::Bits Bits;

  FloatingPointTest()
      : max_ulps_(Floating::kMaxUlps),
        zero_bits_(Floating(0).bits()),
        one_bits_(Floating(1).bits()),
        infinity_bits_(Floating(Floating::Infinity()).bits()),
        close_to_positive_zero_(
            Floating::ReinterpretBits(zero_bits_ + max_ulps_/2)),
        close_to_negative_zero_(
            -Floating::ReinterpretBits(zero_bits_ + max_ulps_ - max_ulps_/2)),
        further_from_negative_zero_(-Floating::ReinterpretBits(
            zero_bits_ + max_ulps_ + 1 - max_ulps_/2)),
        close_to_one_(Floating::ReinterpretBits(one_bits_ + max_ulps_)),
        further_from_one_(Floating::ReinterpretBits(one_bits_ + max_ulps_ + 1)),
        infinity_(Floating::Infinity()),
        close_to_infinity_(
            Floating::ReinterpretBits(infinity_bits_ - max_ulps_)),
        further_from_infinity_(
            Floating::ReinterpretBits(infinity_bits_ - max_ulps_ - 1)),
        max_(Floating::Max()),
        nan1_(Floating::ReinterpretBits(Floating::kExponentBitMask | 1)),
        nan2_(Floating::ReinterpretBits(Floating::kExponentBitMask | 200)) {
  }

  void TestSize() {
    EXPECT_EQ(sizeof(RawType), sizeof(Bits));
  }

  // A battery of tests for FloatingEqMatcher::Matches.
  // matcher_maker is a pointer to a function which creates a FloatingEqMatcher.
  void TestMatches(
      testing::internal::FloatingEqMatcher<RawType> (*matcher_maker)(RawType)) {
    Matcher<RawType> m1 = matcher_maker(0.0);
    EXPECT_TRUE(m1.Matches(-0.0));
    EXPECT_TRUE(m1.Matches(close_to_positive_zero_));
    EXPECT_TRUE(m1.Matches(close_to_negative_zero_));
    EXPECT_FALSE(m1.Matches(1.0));

    Matcher<RawType> m2 = matcher_maker(close_to_positive_zero_);
    EXPECT_FALSE(m2.Matches(further_from_negative_zero_));

    Matcher<RawType> m3 = matcher_maker(1.0);
    EXPECT_TRUE(m3.Matches(close_to_one_));
    EXPECT_FALSE(m3.Matches(further_from_one_));

    // Test commutativity: matcher_maker(0.0).Matches(1.0) was tested above.
    EXPECT_FALSE(m3.Matches(0.0));

    Matcher<RawType> m4 = matcher_maker(-infinity_);
    EXPECT_TRUE(m4.Matches(-close_to_infinity_));

    Matcher<RawType> m5 = matcher_maker(infinity_);
    EXPECT_TRUE(m5.Matches(close_to_infinity_));

    // This is interesting as the representations of infinity_ and nan1_
    // are only 1 DLP apart.
    EXPECT_FALSE(m5.Matches(nan1_));

    // matcher_maker can produce a Matcher<const RawType&>, which is needed in
    // some cases.
    Matcher<const RawType&> m6 = matcher_maker(0.0);
    EXPECT_TRUE(m6.Matches(-0.0));
    EXPECT_TRUE(m6.Matches(close_to_positive_zero_));
    EXPECT_FALSE(m6.Matches(1.0));

    // matcher_maker can produce a Matcher<RawType&>, which is needed in some
    // cases.
    Matcher<RawType&> m7 = matcher_maker(0.0);
    RawType x = 0.0;
    EXPECT_TRUE(m7.Matches(x));
    x = 0.01f;
    EXPECT_FALSE(m7.Matches(x));
  }

  // Pre-calculated numbers to be used by the tests.

  const Bits max_ulps_;

  const Bits zero_bits_;  // The bits that represent 0.0.
  const Bits one_bits_;  // The bits that represent 1.0.
  const Bits infinity_bits_;  // The bits that represent +infinity.

  // Some numbers close to 0.0.
  const RawType close_to_positive_zero_;
  const RawType close_to_negative_zero_;
  const RawType further_from_negative_zero_;

  // Some numbers close to 1.0.
  const RawType close_to_one_;
  const RawType further_from_one_;

  // Some numbers close to +infinity.
  const RawType infinity_;
  const RawType close_to_infinity_;
  const RawType further_from_infinity_;

  // Maximum representable value that's not infinity.
  const RawType max_;

  // Some NaNs.
  const RawType nan1_;
  const RawType nan2_;
};

// Tests floating-point matchers with fixed epsilons.
template <typename RawType>
class FloatingPointNearTest : public FloatingPointTest<RawType> {
 protected:
  typedef FloatingPointTest<RawType> ParentType;

  // A battery of tests for FloatingEqMatcher::Matches with a fixed epsilon.
  // matcher_maker is a pointer to a function which creates a FloatingEqMatcher.
  void TestNearMatches(
      testing::internal::FloatingEqMatcher<RawType>
          (*matcher_maker)(RawType, RawType)) {
    Matcher<RawType> m1 = matcher_maker(0.0, 0.0);
    EXPECT_TRUE(m1.Matches(0.0));
    EXPECT_TRUE(m1.Matches(-0.0));
    EXPECT_FALSE(m1.Matches(ParentType::close_to_positive_zero_));
    EXPECT_FALSE(m1.Matches(ParentType::close_to_negative_zero_));
    EXPECT_FALSE(m1.Matches(1.0));

    Matcher<RawType> m2 = matcher_maker(0.0, 1.0);
    EXPECT_TRUE(m2.Matches(0.0));
    EXPECT_TRUE(m2.Matches(-0.0));
    EXPECT_TRUE(m2.Matches(1.0));
    EXPECT_TRUE(m2.Matches(-1.0));
    EXPECT_FALSE(m2.Matches(ParentType::close_to_one_));
    EXPECT_FALSE(m2.Matches(-ParentType::close_to_one_));

    // Check that inf matches inf, regardless of the of the specified max
    // absolute error.
    Matcher<RawType> m3 = matcher_maker(ParentType::infinity_, 0.0);
    EXPECT_TRUE(m3.Matches(ParentType::infinity_));
    EXPECT_FALSE(m3.Matches(ParentType::close_to_infinity_));
    EXPECT_FALSE(m3.Matches(-ParentType::infinity_));

    Matcher<RawType> m4 = matcher_maker(-ParentType::infinity_, 0.0);
    EXPECT_TRUE(m4.Matches(-ParentType::infinity_));
    EXPECT_FALSE(m4.Matches(-ParentType::close_to_infinity_));
    EXPECT_FALSE(m4.Matches(ParentType::infinity_));

    // Test various overflow scenarios.
    Matcher<RawType> m5 = matcher_maker(ParentType::max_, ParentType::max_);
    EXPECT_TRUE(m5.Matches(ParentType::max_));
    EXPECT_FALSE(m5.Matches(-ParentType::max_));

    Matcher<RawType> m6 = matcher_maker(-ParentType::max_, ParentType::max_);
    EXPECT_FALSE(m6.Matches(ParentType::max_));
    EXPECT_TRUE(m6.Matches(-ParentType::max_));

    Matcher<RawType> m7 = matcher_maker(ParentType::max_, 0);
    EXPECT_TRUE(m7.Matches(ParentType::max_));
    EXPECT_FALSE(m7.Matches(-ParentType::max_));

    Matcher<RawType> m8 = matcher_maker(-ParentType::max_, 0);
    EXPECT_FALSE(m8.Matches(ParentType::max_));
    EXPECT_TRUE(m8.Matches(-ParentType::max_));

    // The difference between max() and -max() normally overflows to infinity,
    // but it should still match if the max_abs_error is also infinity.
    Matcher<RawType> m9 = matcher_maker(
        ParentType::max_, ParentType::infinity_);
    EXPECT_TRUE(m8.Matches(-ParentType::max_));

    // matcher_maker can produce a Matcher<const RawType&>, which is needed in
    // some cases.
    Matcher<const RawType&> m10 = matcher_maker(0.0, 1.0);
    EXPECT_TRUE(m10.Matches(-0.0));
    EXPECT_TRUE(m10.Matches(ParentType::close_to_positive_zero_));
    EXPECT_FALSE(m10.Matches(ParentType::close_to_one_));

    // matcher_maker can produce a Matcher<RawType&>, which is needed in some
    // cases.
    Matcher<RawType&> m11 = matcher_maker(0.0, 1.0);
    RawType x = 0.0;
    EXPECT_TRUE(m11.Matches(x));
    x = 1.0f;
    EXPECT_TRUE(m11.Matches(x));
    x = -1.0f;
    EXPECT_TRUE(m11.Matches(x));
    x = 1.1f;
    EXPECT_FALSE(m11.Matches(x));
    x = -1.1f;
    EXPECT_FALSE(m11.Matches(x));
  }
};

// Instantiate FloatingPointTest for testing floats.
typedef FloatingPointTest<float> FloatTest;

TEST_F(FloatTest, FloatEqApproximatelyMatchesFloats) {
  TestMatches(&FloatEq);
}

TEST_F(FloatTest, NanSensitiveFloatEqApproximatelyMatchesFloats) {
  TestMatches(&NanSensitiveFloatEq);
}

TEST_F(FloatTest, FloatEqCannotMatchNaN) {
  // FloatEq never matches NaN.
  Matcher<float> m = FloatEq(nan1_);
  EXPECT_FALSE(m.Matches(nan1_));
  EXPECT_FALSE(m.Matches(nan2_));
  EXPECT_FALSE(m.Matches(1.0));
}

TEST_F(FloatTest, NanSensitiveFloatEqCanMatchNaN) {
  // NanSensitiveFloatEq will match NaN.
  Matcher<float> m = NanSensitiveFloatEq(nan1_);
  EXPECT_TRUE(m.Matches(nan1_));
  EXPECT_TRUE(m.Matches(nan2_));
  EXPECT_FALSE(m.Matches(1.0));
}

TEST_F(FloatTest, FloatEqCanDescribeSelf) {
  Matcher<float> m1 = FloatEq(2.0f);
  EXPECT_EQ("is approximately 2", Describe(m1));
  EXPECT_EQ("isn't approximately 2", DescribeNegation(m1));

  Matcher<float> m2 = FloatEq(0.5f);
  EXPECT_EQ("is approximately 0.5", Describe(m2));
  EXPECT_EQ("isn't approximately 0.5", DescribeNegation(m2));

  Matcher<float> m3 = FloatEq(nan1_);
  EXPECT_EQ("never matches", Describe(m3));
  EXPECT_EQ("is anything", DescribeNegation(m3));
}

TEST_F(FloatTest, NanSensitiveFloatEqCanDescribeSelf) {
  Matcher<float> m1 = NanSensitiveFloatEq(2.0f);
  EXPECT_EQ("is approximately 2", Describe(m1));
  EXPECT_EQ("isn't approximately 2", DescribeNegation(m1));

  Matcher<float> m2 = NanSensitiveFloatEq(0.5f);
  EXPECT_EQ("is approximately 0.5", Describe(m2));
  EXPECT_EQ("isn't approximately 0.5", DescribeNegation(m2));

  Matcher<float> m3 = NanSensitiveFloatEq(nan1_);
  EXPECT_EQ("is NaN", Describe(m3));
  EXPECT_EQ("isn't NaN", DescribeNegation(m3));
}

// Instantiate FloatingPointTest for testing floats with a user-specified
// max absolute error.
typedef FloatingPointNearTest<float> FloatNearTest;

TEST_F(FloatNearTest, FloatNearMatches) {
  TestNearMatches(&FloatNear);
}

TEST_F(FloatNearTest, NanSensitiveFloatNearApproximatelyMatchesFloats) {
  TestNearMatches(&NanSensitiveFloatNear);
}

TEST_F(FloatNearTest, FloatNearCanDescribeSelf) {
  Matcher<float> m1 = FloatNear(2.0f, 0.5f);
  EXPECT_EQ("is approximately 2 (absolute error <= 0.5)", Describe(m1));
  EXPECT_EQ(
      "isn't approximately 2 (absolute error > 0.5)", DescribeNegation(m1));

  Matcher<float> m2 = FloatNear(0.5f, 0.5f);
  EXPECT_EQ("is approximately 0.5 (absolute error <= 0.5)", Describe(m2));
  EXPECT_EQ(
      "isn't approximately 0.5 (absolute error > 0.5)", DescribeNegation(m2));

  Matcher<float> m3 = FloatNear(nan1_, 0.0);
  EXPECT_EQ("never matches", Describe(m3));
  EXPECT_EQ("is anything", DescribeNegation(m3));
}

TEST_F(FloatNearTest, NanSensitiveFloatNearCanDescribeSelf) {
  Matcher<float> m1 = NanSensitiveFloatNear(2.0f, 0.5f);
  EXPECT_EQ("is approximately 2 (absolute error <= 0.5)", Describe(m1));
  EXPECT_EQ(
      "isn't approximately 2 (absolute error > 0.5)", DescribeNegation(m1));

  Matcher<float> m2 = NanSensitiveFloatNear(0.5f, 0.5f);
  EXPECT_EQ("is approximately 0.5 (absolute error <= 0.5)", Describe(m2));
  EXPECT_EQ(
      "isn't approximately 0.5 (absolute error > 0.5)", DescribeNegation(m2));

  Matcher<float> m3 = NanSensitiveFloatNear(nan1_, 0.1f);
  EXPECT_EQ("is NaN", Describe(m3));
  EXPECT_EQ("isn't NaN", DescribeNegation(m3));
}

TEST_F(FloatNearTest, FloatNearCannotMatchNaN) {
  // FloatNear never matches NaN.
  Matcher<float> m = FloatNear(ParentType::nan1_, 0.1f);
  EXPECT_FALSE(m.Matches(nan1_));
  EXPECT_FALSE(m.Matches(nan2_));
  EXPECT_FALSE(m.Matches(1.0));
}

TEST_F(FloatNearTest, NanSensitiveFloatNearCanMatchNaN) {
  // NanSensitiveFloatNear will match NaN.
  Matcher<float> m = NanSensitiveFloatNear(nan1_, 0.1f);
  EXPECT_TRUE(m.Matches(nan1_));
  EXPECT_TRUE(m.Matches(nan2_));
  EXPECT_FALSE(m.Matches(1.0));
}

// Instantiate FloatingPointTest for testing doubles.
typedef FloatingPointTest<double> DoubleTest;

TEST_F(DoubleTest, DoubleEqApproximatelyMatchesDoubles) {
  TestMatches(&DoubleEq);
}

TEST_F(DoubleTest, NanSensitiveDoubleEqApproximatelyMatchesDoubles) {
  TestMatches(&NanSensitiveDoubleEq);
}

TEST_F(DoubleTest, DoubleEqCannotMatchNaN) {
  // DoubleEq never matches NaN.
  Matcher<double> m = DoubleEq(nan1_);
  EXPECT_FALSE(m.Matches(nan1_));
  EXPECT_FALSE(m.Matches(nan2_));
  EXPECT_FALSE(m.Matches(1.0));
}

TEST_F(DoubleTest, NanSensitiveDoubleEqCanMatchNaN) {
  // NanSensitiveDoubleEq will match NaN.
  Matcher<double> m = NanSensitiveDoubleEq(nan1_);
  EXPECT_TRUE(m.Matches(nan1_));
  EXPECT_TRUE(m.Matches(nan2_));
  EXPECT_FALSE(m.Matches(1.0));
}

TEST_F(DoubleTest, DoubleEqCanDescribeSelf) {
  Matcher<double> m1 = DoubleEq(2.0);
  EXPECT_EQ("is approximately 2", Describe(m1));
  EXPECT_EQ("isn't approximately 2", DescribeNegation(m1));

  Matcher<double> m2 = DoubleEq(0.5);
  EXPECT_EQ("is approximately 0.5", Describe(m2));
  EXPECT_EQ("isn't approximately 0.5", DescribeNegation(m2));

  Matcher<double> m3 = DoubleEq(nan1_);
  EXPECT_EQ("never matches", Describe(m3));
  EXPECT_EQ("is anything", DescribeNegation(m3));
}

TEST_F(DoubleTest, NanSensitiveDoubleEqCanDescribeSelf) {
  Matcher<double> m1 = NanSensitiveDoubleEq(2.0);
  EXPECT_EQ("is approximately 2", Describe(m1));
  EXPECT_EQ("isn't approximately 2", DescribeNegation(m1));

  Matcher<double> m2 = NanSensitiveDoubleEq(0.5);
  EXPECT_EQ("is approximately 0.5", Describe(m2));
  EXPECT_EQ("isn't approximately 0.5", DescribeNegation(m2));

  Matcher<double> m3 = NanSensitiveDoubleEq(nan1_);
  EXPECT_EQ("is NaN", Describe(m3));
  EXPECT_EQ("isn't NaN", DescribeNegation(m3));
}

// Instantiate FloatingPointTest for testing floats with a user-specified
// max absolute error.
typedef FloatingPointNearTest<double> DoubleNearTest;

TEST_F(DoubleNearTest, DoubleNearMatches) {
  TestNearMatches(&DoubleNear);
}

TEST_F(DoubleNearTest, NanSensitiveDoubleNearApproximatelyMatchesDoubles) {
  TestNearMatches(&NanSensitiveDoubleNear);
}

TEST_F(DoubleNearTest, DoubleNearCanDescribeSelf) {
  Matcher<double> m1 = DoubleNear(2.0, 0.5);
  EXPECT_EQ("is approximately 2 (absolute error <= 0.5)", Describe(m1));
  EXPECT_EQ(
      "isn't approximately 2 (absolute error > 0.5)", DescribeNegation(m1));

  Matcher<double> m2 = DoubleNear(0.5, 0.5);
  EXPECT_EQ("is approximately 0.5 (absolute error <= 0.5)", Describe(m2));
  EXPECT_EQ(
      "isn't approximately 0.5 (absolute error > 0.5)", DescribeNegation(m2));

  Matcher<double> m3 = DoubleNear(nan1_, 0.0);
  EXPECT_EQ("never matches", Describe(m3));
  EXPECT_EQ("is anything", DescribeNegation(m3));
}

TEST_F(DoubleNearTest, ExplainsResultWhenMatchFails) {
  EXPECT_EQ("", Explain(DoubleNear(2.0, 0.1), 2.05));
  EXPECT_EQ("which is 0.2 from 2", Explain(DoubleNear(2.0, 0.1), 2.2));
  EXPECT_EQ("which is -0.3 from 2", Explain(DoubleNear(2.0, 0.1), 1.7));

  const std::string explanation =
      Explain(DoubleNear(2.1, 1e-10), 2.1 + 1.2e-10);
  // Different C++ implementations may print floating-point numbers
  // slightly differently.
  EXPECT_TRUE(explanation == "which is 1.2e-10 from 2.1" ||  // GCC
              explanation == "which is 1.2e-010 from 2.1")   // MSVC
      << " where explanation is \"" << explanation << "\".";
}

TEST_F(DoubleNearTest, NanSensitiveDoubleNearCanDescribeSelf) {
  Matcher<double> m1 = NanSensitiveDoubleNear(2.0, 0.5);
  EXPECT_EQ("is approximately 2 (absolute error <= 0.5)", Describe(m1));
  EXPECT_EQ(
      "isn't approximately 2 (absolute error > 0.5)", DescribeNegation(m1));

  Matcher<double> m2 = NanSensitiveDoubleNear(0.5, 0.5);
  EXPECT_EQ("is approximately 0.5 (absolute error <= 0.5)", Describe(m2));
  EXPECT_EQ(
      "isn't approximately 0.5 (absolute error > 0.5)", DescribeNegation(m2));

  Matcher<double> m3 = NanSensitiveDoubleNear(nan1_, 0.1);
  EXPECT_EQ("is NaN", Describe(m3));
  EXPECT_EQ("isn't NaN", DescribeNegation(m3));
}

TEST_F(DoubleNearTest, DoubleNearCannotMatchNaN) {
  // DoubleNear never matches NaN.
  Matcher<double> m = DoubleNear(ParentType::nan1_, 0.1);
  EXPECT_FALSE(m.Matches(nan1_));
  EXPECT_FALSE(m.Matches(nan2_));
  EXPECT_FALSE(m.Matches(1.0));
}

TEST_F(DoubleNearTest, NanSensitiveDoubleNearCanMatchNaN) {
  // NanSensitiveDoubleNear will match NaN.
  Matcher<double> m = NanSensitiveDoubleNear(nan1_, 0.1);
  EXPECT_TRUE(m.Matches(nan1_));
  EXPECT_TRUE(m.Matches(nan2_));
  EXPECT_FALSE(m.Matches(1.0));
}

TEST(PointeeTest, RawPointer) {
  const Matcher<int*> m = Pointee(Ge(0));

  int n = 1;
  EXPECT_TRUE(m.Matches(&n));
  n = -1;
  EXPECT_FALSE(m.Matches(&n));
  EXPECT_FALSE(m.Matches(nullptr));
}

TEST(PointeeTest, RawPointerToConst) {
  const Matcher<const double*> m = Pointee(Ge(0));

  double x = 1;
  EXPECT_TRUE(m.Matches(&x));
  x = -1;
  EXPECT_FALSE(m.Matches(&x));
  EXPECT_FALSE(m.Matches(nullptr));
}

TEST(PointeeTest, ReferenceToConstRawPointer) {
  const Matcher<int* const &> m = Pointee(Ge(0));

  int n = 1;
  EXPECT_TRUE(m.Matches(&n));
  n = -1;
  EXPECT_FALSE(m.Matches(&n));
  EXPECT_FALSE(m.Matches(nullptr));
}

TEST(PointeeTest, ReferenceToNonConstRawPointer) {
  const Matcher<double* &> m = Pointee(Ge(0));

  double x = 1.0;
  double* p = &x;
  EXPECT_TRUE(m.Matches(p));
  x = -1;
  EXPECT_FALSE(m.Matches(p));
  p = nullptr;
  EXPECT_FALSE(m.Matches(p));
}

TEST(PointeeTest, SmartPointer) {
  const Matcher<std::unique_ptr<int>> m = Pointee(Ge(0));

  std::unique_ptr<int> n(new int(1));
  EXPECT_TRUE(m.Matches(n));
}

TEST(PointeeTest, SmartPointerToConst) {
  const Matcher<std::unique_ptr<const int>> m = Pointee(Ge(0));

  // There's no implicit conversion from unique_ptr<int> to const
  // unique_ptr<const int>, so we must pass a unique_ptr<const int> into the
  // matcher.
  std::unique_ptr<const int> n(new int(1));
  EXPECT_TRUE(m.Matches(n));
}

TEST(PointerTest, RawPointer) {
  int n = 1;
  const Matcher<int*> m = Pointer(Eq(&n));

  EXPECT_TRUE(m.Matches(&n));

  int* p = nullptr;
  EXPECT_FALSE(m.Matches(p));
  EXPECT_FALSE(m.Matches(nullptr));
}

TEST(PointerTest, RawPointerToConst) {
  int n = 1;
  const Matcher<const int*> m = Pointer(Eq(&n));

  EXPECT_TRUE(m.Matches(&n));

  int* p = nullptr;
  EXPECT_FALSE(m.Matches(p));
  EXPECT_FALSE(m.Matches(nullptr));
}

TEST(PointerTest, SmartPointer) {
  std::unique_ptr<int> n(new int(10));
  int* raw_n = n.get();
  const Matcher<std::unique_ptr<int>> m = Pointer(Eq(raw_n));

  EXPECT_TRUE(m.Matches(n));
}

TEST(PointerTest, SmartPointerToConst) {
  std::unique_ptr<const int> n(new int(10));
  const int* raw_n = n.get();
  const Matcher<std::unique_ptr<const int>> m = Pointer(Eq(raw_n));

  // There's no implicit conversion from unique_ptr<int> to const
  // unique_ptr<const int>, so we must pass a unique_ptr<const int> into the
  // matcher.
  std::unique_ptr<const int> p(new int(10));
  EXPECT_FALSE(m.Matches(p));
}

TEST(AddressTest, NonConst) {
  int n = 1;
  const Matcher<int> m = Address(Eq(&n));

  EXPECT_TRUE(m.Matches(n));

  int other = 5;

  EXPECT_FALSE(m.Matches(other));

  int& n_ref = n;

  EXPECT_TRUE(m.Matches(n_ref));
}

TEST(AddressTest, Const) {
  const int n = 1;
  const Matcher<int> m = Address(Eq(&n));

  EXPECT_TRUE(m.Matches(n));

  int other = 5;

  EXPECT_FALSE(m.Matches(other));
}

TEST(AddressTest, MatcherDoesntCopy) {
  std::unique_ptr<int> n(new int(1));
  const Matcher<std::unique_ptr<int>> m = Address(Eq(&n));

  EXPECT_TRUE(m.Matches(n));
}

TEST(AddressTest, Describe) {
  Matcher<int> matcher = Address(_);
  EXPECT_EQ("has address that is anything", Describe(matcher));
  EXPECT_EQ("does not have address that is anything",
            DescribeNegation(matcher));
}

MATCHER_P(FieldIIs, inner_matcher, "") {
  return ExplainMatchResult(inner_matcher, arg.i, result_listener);
}

#if GTEST_HAS_RTTI
TEST(WhenDynamicCastToTest, SameType) {
  Derived derived;
  derived.i = 4;

  // Right type. A pointer is passed down.
  Base* as_base_ptr = &derived;
  EXPECT_THAT(as_base_ptr, WhenDynamicCastTo<Derived*>(Not(IsNull())));
  EXPECT_THAT(as_base_ptr, WhenDynamicCastTo<Derived*>(Pointee(FieldIIs(4))));
  EXPECT_THAT(as_base_ptr,
              Not(WhenDynamicCastTo<Derived*>(Pointee(FieldIIs(5)))));
}

TEST(WhenDynamicCastToTest, WrongTypes) {
  Base base;
  Derived derived;
  OtherDerived other_derived;

  // Wrong types. NULL is passed.
  EXPECT_THAT(&base, Not(WhenDynamicCastTo<Derived*>(Pointee(_))));
  EXPECT_THAT(&base, WhenDynamicCastTo<Derived*>(IsNull()));
  Base* as_base_ptr = &derived;
  EXPECT_THAT(as_base_ptr, Not(WhenDynamicCastTo<OtherDerived*>(Pointee(_))));
  EXPECT_THAT(as_base_ptr, WhenDynamicCastTo<OtherDerived*>(IsNull()));
  as_base_ptr = &other_derived;
  EXPECT_THAT(as_base_ptr, Not(WhenDynamicCastTo<Derived*>(Pointee(_))));
  EXPECT_THAT(as_base_ptr, WhenDynamicCastTo<Derived*>(IsNull()));
}

TEST(WhenDynamicCastToTest, AlreadyNull) {
  // Already NULL.
  Base* as_base_ptr = nullptr;
  EXPECT_THAT(as_base_ptr, WhenDynamicCastTo<Derived*>(IsNull()));
}

struct AmbiguousCastTypes {
  class VirtualDerived : public virtual Base {};
  class DerivedSub1 : public VirtualDerived {};
  class DerivedSub2 : public VirtualDerived {};
  class ManyDerivedInHierarchy : public DerivedSub1, public DerivedSub2 {};
};

TEST(WhenDynamicCastToTest, AmbiguousCast) {
  AmbiguousCastTypes::DerivedSub1 sub1;
  AmbiguousCastTypes::ManyDerivedInHierarchy many_derived;
  // Multiply derived from Base. dynamic_cast<> returns NULL.
  Base* as_base_ptr =
      static_cast<AmbiguousCastTypes::DerivedSub1*>(&many_derived);
  EXPECT_THAT(as_base_ptr,
              WhenDynamicCastTo<AmbiguousCastTypes::VirtualDerived*>(IsNull()));
  as_base_ptr = &sub1;
  EXPECT_THAT(
      as_base_ptr,
      WhenDynamicCastTo<AmbiguousCastTypes::VirtualDerived*>(Not(IsNull())));
}

TEST(WhenDynamicCastToTest, Describe) {
  Matcher<Base*> matcher = WhenDynamicCastTo<Derived*>(Pointee(_));
  const std::string prefix =
      "when dynamic_cast to " + internal::GetTypeName<Derived*>() + ", ";
  EXPECT_EQ(prefix + "points to a value that is anything", Describe(matcher));
  EXPECT_EQ(prefix + "does not point to a value that is anything",
            DescribeNegation(matcher));
}

TEST(WhenDynamicCastToTest, Explain) {
  Matcher<Base*> matcher = WhenDynamicCastTo<Derived*>(Pointee(_));
  Base* null = nullptr;
  EXPECT_THAT(Explain(matcher, null), HasSubstr("NULL"));
  Derived derived;
  EXPECT_TRUE(matcher.Matches(&derived));
  EXPECT_THAT(Explain(matcher, &derived), HasSubstr("which points to "));

  // With references, the matcher itself can fail. Test for that one.
  Matcher<const Base&> ref_matcher = WhenDynamicCastTo<const OtherDerived&>(_);
  EXPECT_THAT(Explain(ref_matcher, derived),
              HasSubstr("which cannot be dynamic_cast"));
}

TEST(WhenDynamicCastToTest, GoodReference) {
  Derived derived;
  derived.i = 4;
  Base& as_base_ref = derived;
  EXPECT_THAT(as_base_ref, WhenDynamicCastTo<const Derived&>(FieldIIs(4)));
  EXPECT_THAT(as_base_ref, WhenDynamicCastTo<const Derived&>(Not(FieldIIs(5))));
}

TEST(WhenDynamicCastToTest, BadReference) {
  Derived derived;
  Base& as_base_ref = derived;
  EXPECT_THAT(as_base_ref, Not(WhenDynamicCastTo<const OtherDerived&>(_)));
}
#endif  // GTEST_HAS_RTTI

// Minimal const-propagating pointer.
template <typename T>
class ConstPropagatingPtr {
 public:
  typedef T element_type;

  ConstPropagatingPtr() : val_() {}
  explicit ConstPropagatingPtr(T* t) : val_(t) {}
  ConstPropagatingPtr(const ConstPropagatingPtr& other) : val_(other.val_) {}

  T* get() { return val_; }
  T& operator*() { return *val_; }
  // Most smart pointers return non-const T* and T& from the next methods.
  const T* get() const { return val_; }
  const T& operator*() const { return *val_; }

 private:
  T* val_;
};

TEST(PointeeTest, WorksWithConstPropagatingPointers) {
  const Matcher< ConstPropagatingPtr<int> > m = Pointee(Lt(5));
  int three = 3;
  const ConstPropagatingPtr<int> co(&three);
  ConstPropagatingPtr<int> o(&three);
  EXPECT_TRUE(m.Matches(o));
  EXPECT_TRUE(m.Matches(co));
  *o = 6;
  EXPECT_FALSE(m.Matches(o));
  EXPECT_FALSE(m.Matches(ConstPropagatingPtr<int>()));
}

TEST(PointeeTest, NeverMatchesNull) {
  const Matcher<const char*> m = Pointee(_);
  EXPECT_FALSE(m.Matches(nullptr));
}

// Tests that we can write Pointee(value) instead of Pointee(Eq(value)).
TEST(PointeeTest, MatchesAgainstAValue) {
  const Matcher<int*> m = Pointee(5);

  int n = 5;
  EXPECT_TRUE(m.Matches(&n));
  n = -1;
  EXPECT_FALSE(m.Matches(&n));
  EXPECT_FALSE(m.Matches(nullptr));
}

TEST(PointeeTest, CanDescribeSelf) {
  const Matcher<int*> m = Pointee(Gt(3));
  EXPECT_EQ("points to a value that is > 3", Describe(m));
  EXPECT_EQ("does not point to a value that is > 3",
            DescribeNegation(m));
}

TEST(PointeeTest, CanExplainMatchResult) {
  const Matcher<const std::string*> m = Pointee(StartsWith("Hi"));

  EXPECT_EQ("", Explain(m, static_cast<const std::string*>(nullptr)));

  const Matcher<long*> m2 = Pointee(GreaterThan(1));  // NOLINT
  long n = 3;  // NOLINT
  EXPECT_EQ("which points to 3" + OfType("long") + ", which is 2 more than 1",
            Explain(m2, &n));
}

TEST(PointeeTest, AlwaysExplainsPointee) {
  const Matcher<int*> m = Pointee(0);
  int n = 42;
  EXPECT_EQ("which points to 42" + OfType("int"), Explain(m, &n));
}

// An uncopyable class.
class Uncopyable {
 public:
  Uncopyable() : value_(-1) {}
  explicit Uncopyable(int a_value) : value_(a_value) {}

  int value() const { return value_; }
  void set_value(int i) { value_ = i; }

 private:
  int value_;
  GTEST_DISALLOW_COPY_AND_ASSIGN_(Uncopyable);
};

// Returns true if and only if x.value() is positive.
bool ValueIsPositive(const Uncopyable& x) { return x.value() > 0; }

MATCHER_P(UncopyableIs, inner_matcher, "") {
  return ExplainMatchResult(inner_matcher, arg.value(), result_listener);
}

// A user-defined struct for testing Field().
struct AStruct {
  AStruct() : x(0), y(1.0), z(5), p(nullptr) {}
  AStruct(const AStruct& rhs)
      : x(rhs.x), y(rhs.y), z(rhs.z.value()), p(rhs.p) {}

  int x;           // A non-const field.
  const double y;  // A const field.
  Uncopyable z;    // An uncopyable field.
  const char* p;   // A pointer field.
};

// A derived struct for testing Field().
struct DerivedStruct : public AStruct {
  char ch;
};

// Tests that Field(&Foo::field, ...) works when field is non-const.
TEST(FieldTest, WorksForNonConstField) {
  Matcher<AStruct> m = Field(&AStruct::x, Ge(0));
  Matcher<AStruct> m_with_name = Field("x", &AStruct::x, Ge(0));

  AStruct a;
  EXPECT_TRUE(m.Matches(a));
  EXPECT_TRUE(m_with_name.Matches(a));
  a.x = -1;
  EXPECT_FALSE(m.Matches(a));
  EXPECT_FALSE(m_with_name.Matches(a));
}

// Tests that Field(&Foo::field, ...) works when field is const.
TEST(FieldTest, WorksForConstField) {
  AStruct a;

  Matcher<AStruct> m = Field(&AStruct::y, Ge(0.0));
  Matcher<AStruct> m_with_name = Field("y", &AStruct::y, Ge(0.0));
  EXPECT_TRUE(m.Matches(a));
  EXPECT_TRUE(m_with_name.Matches(a));
  m = Field(&AStruct::y, Le(0.0));
  m_with_name = Field("y", &AStruct::y, Le(0.0));
  EXPECT_FALSE(m.Matches(a));
  EXPECT_FALSE(m_with_name.Matches(a));
}

// Tests that Field(&Foo::field, ...) works when field is not copyable.
TEST(FieldTest, WorksForUncopyableField) {
  AStruct a;

  Matcher<AStruct> m = Field(&AStruct::z, Truly(ValueIsPositive));
  EXPECT_TRUE(m.Matches(a));
  m = Field(&AStruct::z, Not(Truly(ValueIsPositive)));
  EXPECT_FALSE(m.Matches(a));
}

// Tests that Field(&Foo::field, ...) works when field is a pointer.
TEST(FieldTest, WorksForPointerField) {
  // Matching against NULL.
  Matcher<AStruct> m = Field(&AStruct::p, static_cast<const char*>(nullptr));
  AStruct a;
  EXPECT_TRUE(m.Matches(a));
  a.p = "hi";
  EXPECT_FALSE(m.Matches(a));

  // Matching a pointer that is not NULL.
  m = Field(&AStruct::p, StartsWith("hi"));
  a.p = "hill";
  EXPECT_TRUE(m.Matches(a));
  a.p = "hole";
  EXPECT_FALSE(m.Matches(a));
}

// Tests that Field() works when the object is passed by reference.
TEST(FieldTest, WorksForByRefArgument) {
  Matcher<const AStruct&> m = Field(&AStruct::x, Ge(0));

  AStruct a;
  EXPECT_TRUE(m.Matches(a));
  a.x = -1;
  EXPECT_FALSE(m.Matches(a));
}

// Tests that Field(&Foo::field, ...) works when the argument's type
// is a sub-type of Foo.
TEST(FieldTest, WorksForArgumentOfSubType) {
  // Note that the matcher expects DerivedStruct but we say AStruct
  // inside Field().
  Matcher<const DerivedStruct&> m = Field(&AStruct::x, Ge(0));

  DerivedStruct d;
  EXPECT_TRUE(m.Matches(d));
  d.x = -1;
  EXPECT_FALSE(m.Matches(d));
}

// Tests that Field(&Foo::field, m) works when field's type and m's
// argument type are compatible but not the same.
TEST(FieldTest, WorksForCompatibleMatcherType) {
  // The field is an int, but the inner matcher expects a signed char.
  Matcher<const AStruct&> m = Field(&AStruct::x,
                                    Matcher<signed char>(Ge(0)));

  AStruct a;
  EXPECT_TRUE(m.Matches(a));
  a.x = -1;
  EXPECT_FALSE(m.Matches(a));
}

// Tests that Field() can describe itself.
TEST(FieldTest, CanDescribeSelf) {
  Matcher<const AStruct&> m = Field(&AStruct::x, Ge(0));

  EXPECT_EQ("is an object whose given field is >= 0", Describe(m));
  EXPECT_EQ("is an object whose given field isn't >= 0", DescribeNegation(m));
}

TEST(FieldTest, CanDescribeSelfWithFieldName) {
  Matcher<const AStruct&> m = Field("field_name", &AStruct::x, Ge(0));

  EXPECT_EQ("is an object whose field `field_name` is >= 0", Describe(m));
  EXPECT_EQ("is an object whose field `field_name` isn't >= 0",
            DescribeNegation(m));
}

// Tests that Field() can explain the match result.
TEST(FieldTest, CanExplainMatchResult) {
  Matcher<const AStruct&> m = Field(&AStruct::x, Ge(0));

  AStruct a;
  a.x = 1;
  EXPECT_EQ("whose given field is 1" + OfType("int"), Explain(m, a));

  m = Field(&AStruct::x, GreaterThan(0));
  EXPECT_EQ(
      "whose given field is 1" + OfType("int") + ", which is 1 more than 0",
      Explain(m, a));
}

TEST(FieldTest, CanExplainMatchResultWithFieldName) {
  Matcher<const AStruct&> m = Field("field_name", &AStruct::x, Ge(0));

  AStruct a;
  a.x = 1;
  EXPECT_EQ("whose field `field_name` is 1" + OfType("int"), Explain(m, a));

  m = Field("field_name", &AStruct::x, GreaterThan(0));
  EXPECT_EQ("whose field `field_name` is 1" + OfType("int") +
                ", which is 1 more than 0",
            Explain(m, a));
}

// Tests that Field() works when the argument is a pointer to const.
TEST(FieldForPointerTest, WorksForPointerToConst) {
  Matcher<const AStruct*> m = Field(&AStruct::x, Ge(0));

  AStruct a;
  EXPECT_TRUE(m.Matches(&a));
  a.x = -1;
  EXPECT_FALSE(m.Matches(&a));
}

// Tests that Field() works when the argument is a pointer to non-const.
TEST(FieldForPointerTest, WorksForPointerToNonConst) {
  Matcher<AStruct*> m = Field(&AStruct::x, Ge(0));

  AStruct a;
  EXPECT_TRUE(m.Matches(&a));
  a.x = -1;
  EXPECT_FALSE(m.Matches(&a));
}

// Tests that Field() works when the argument is a reference to a const pointer.
TEST(FieldForPointerTest, WorksForReferenceToConstPointer) {
  Matcher<AStruct* const&> m = Field(&AStruct::x, Ge(0));

  AStruct a;
  EXPECT_TRUE(m.Matches(&a));
  a.x = -1;
  EXPECT_FALSE(m.Matches(&a));
}

// Tests that Field() does not match the NULL pointer.
TEST(FieldForPointerTest, DoesNotMatchNull) {
  Matcher<const AStruct*> m = Field(&AStruct::x, _);
  EXPECT_FALSE(m.Matches(nullptr));
}

// Tests that Field(&Foo::field, ...) works when the argument's type
// is a sub-type of const Foo*.
TEST(FieldForPointerTest, WorksForArgumentOfSubType) {
  // Note that the matcher expects DerivedStruct but we say AStruct
  // inside Field().
  Matcher<DerivedStruct*> m = Field(&AStruct::x, Ge(0));

  DerivedStruct d;
  EXPECT_TRUE(m.Matches(&d));
  d.x = -1;
  EXPECT_FALSE(m.Matches(&d));
}

// Tests that Field() can describe itself when used to match a pointer.
TEST(FieldForPointerTest, CanDescribeSelf) {
  Matcher<const AStruct*> m = Field(&AStruct::x, Ge(0));

  EXPECT_EQ("is an object whose given field is >= 0", Describe(m));
  EXPECT_EQ("is an object whose given field isn't >= 0", DescribeNegation(m));
}

TEST(FieldForPointerTest, CanDescribeSelfWithFieldName) {
  Matcher<const AStruct*> m = Field("field_name", &AStruct::x, Ge(0));

  EXPECT_EQ("is an object whose field `field_name` is >= 0", Describe(m));
  EXPECT_EQ("is an object whose field `field_name` isn't >= 0",
            DescribeNegation(m));
}

// Tests that Field() can explain the result of matching a pointer.
TEST(FieldForPointerTest, CanExplainMatchResult) {
  Matcher<const AStruct*> m = Field(&AStruct::x, Ge(0));

  AStruct a;
  a.x = 1;
  EXPECT_EQ("", Explain(m, static_cast<const AStruct*>(nullptr)));
  EXPECT_EQ("which points to an object whose given field is 1" + OfType("int"),
            Explain(m, &a));

  m = Field(&AStruct::x, GreaterThan(0));
  EXPECT_EQ("which points to an object whose given field is 1" + OfType("int") +
            ", which is 1 more than 0", Explain(m, &a));
}

TEST(FieldForPointerTest, CanExplainMatchResultWithFieldName) {
  Matcher<const AStruct*> m = Field("field_name", &AStruct::x, Ge(0));

  AStruct a;
  a.x = 1;
  EXPECT_EQ("", Explain(m, static_cast<const AStruct*>(nullptr)));
  EXPECT_EQ(
      "which points to an object whose field `field_name` is 1" + OfType("int"),
      Explain(m, &a));

  m = Field("field_name", &AStruct::x, GreaterThan(0));
  EXPECT_EQ("which points to an object whose field `field_name` is 1" +
                OfType("int") + ", which is 1 more than 0",
            Explain(m, &a));
}

}  // namespace
}  // namespace gmock_matchers_1_test
}  // namespace testing

#ifdef _MSC_VER
# pragma warning(pop)
#endif
