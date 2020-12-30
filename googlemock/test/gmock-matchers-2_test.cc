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
namespace gmock_matchers_2_test {
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

// A user-defined class for testing Property().
class AClass {
 public:
  AClass() : n_(0) {}

  // A getter that returns a non-reference.
  int n() const { return n_; }

  void set_n(int new_n) { n_ = new_n; }

  // A getter that returns a reference to const.
  const std::string& s() const { return s_; }

  const std::string& s_ref() const & { return s_; }

  void set_s(const std::string& new_s) { s_ = new_s; }

  // A getter that returns a reference to non-const.
  double& x() const { return x_; }

 private:
  int n_;
  std::string s_;

  static double x_;
};

double AClass::x_ = 0.0;

// A derived class for testing Property().
class DerivedClass : public AClass {
 public:
  int k() const { return k_; }
 private:
  int k_;
};

// Tests that Property(&Foo::property, ...) works when property()
// returns a non-reference.
TEST(PropertyTest, WorksForNonReferenceProperty) {
  Matcher<const AClass&> m = Property(&AClass::n, Ge(0));
  Matcher<const AClass&> m_with_name = Property("n", &AClass::n, Ge(0));

  AClass a;
  a.set_n(1);
  EXPECT_TRUE(m.Matches(a));
  EXPECT_TRUE(m_with_name.Matches(a));

  a.set_n(-1);
  EXPECT_FALSE(m.Matches(a));
  EXPECT_FALSE(m_with_name.Matches(a));
}

// Tests that Property(&Foo::property, ...) works when property()
// returns a reference to const.
TEST(PropertyTest, WorksForReferenceToConstProperty) {
  Matcher<const AClass&> m = Property(&AClass::s, StartsWith("hi"));
  Matcher<const AClass&> m_with_name =
      Property("s", &AClass::s, StartsWith("hi"));

  AClass a;
  a.set_s("hill");
  EXPECT_TRUE(m.Matches(a));
  EXPECT_TRUE(m_with_name.Matches(a));

  a.set_s("hole");
  EXPECT_FALSE(m.Matches(a));
  EXPECT_FALSE(m_with_name.Matches(a));
}

// Tests that Property(&Foo::property, ...) works when property() is
// ref-qualified.
TEST(PropertyTest, WorksForRefQualifiedProperty) {
  Matcher<const AClass&> m = Property(&AClass::s_ref, StartsWith("hi"));
  Matcher<const AClass&> m_with_name =
      Property("s", &AClass::s_ref, StartsWith("hi"));

  AClass a;
  a.set_s("hill");
  EXPECT_TRUE(m.Matches(a));
  EXPECT_TRUE(m_with_name.Matches(a));

  a.set_s("hole");
  EXPECT_FALSE(m.Matches(a));
  EXPECT_FALSE(m_with_name.Matches(a));
}

// Tests that Property(&Foo::property, ...) works when property()
// returns a reference to non-const.
TEST(PropertyTest, WorksForReferenceToNonConstProperty) {
  double x = 0.0;
  AClass a;

  Matcher<const AClass&> m = Property(&AClass::x, Ref(x));
  EXPECT_FALSE(m.Matches(a));

  m = Property(&AClass::x, Not(Ref(x)));
  EXPECT_TRUE(m.Matches(a));
}

// Tests that Property(&Foo::property, ...) works when the argument is
// passed by value.
TEST(PropertyTest, WorksForByValueArgument) {
  Matcher<AClass> m = Property(&AClass::s, StartsWith("hi"));

  AClass a;
  a.set_s("hill");
  EXPECT_TRUE(m.Matches(a));

  a.set_s("hole");
  EXPECT_FALSE(m.Matches(a));
}

// Tests that Property(&Foo::property, ...) works when the argument's
// type is a sub-type of Foo.
TEST(PropertyTest, WorksForArgumentOfSubType) {
  // The matcher expects a DerivedClass, but inside the Property() we
  // say AClass.
  Matcher<const DerivedClass&> m = Property(&AClass::n, Ge(0));

  DerivedClass d;
  d.set_n(1);
  EXPECT_TRUE(m.Matches(d));

  d.set_n(-1);
  EXPECT_FALSE(m.Matches(d));
}

// Tests that Property(&Foo::property, m) works when property()'s type
// and m's argument type are compatible but different.
TEST(PropertyTest, WorksForCompatibleMatcherType) {
  // n() returns an int but the inner matcher expects a signed char.
  Matcher<const AClass&> m = Property(&AClass::n,
                                      Matcher<signed char>(Ge(0)));

  Matcher<const AClass&> m_with_name =
      Property("n", &AClass::n, Matcher<signed char>(Ge(0)));

  AClass a;
  EXPECT_TRUE(m.Matches(a));
  EXPECT_TRUE(m_with_name.Matches(a));
  a.set_n(-1);
  EXPECT_FALSE(m.Matches(a));
  EXPECT_FALSE(m_with_name.Matches(a));
}

// Tests that Property() can describe itself.
TEST(PropertyTest, CanDescribeSelf) {
  Matcher<const AClass&> m = Property(&AClass::n, Ge(0));

  EXPECT_EQ("is an object whose given property is >= 0", Describe(m));
  EXPECT_EQ("is an object whose given property isn't >= 0",
            DescribeNegation(m));
}

TEST(PropertyTest, CanDescribeSelfWithPropertyName) {
  Matcher<const AClass&> m = Property("fancy_name", &AClass::n, Ge(0));

  EXPECT_EQ("is an object whose property `fancy_name` is >= 0", Describe(m));
  EXPECT_EQ("is an object whose property `fancy_name` isn't >= 0",
            DescribeNegation(m));
}

// Tests that Property() can explain the match result.
TEST(PropertyTest, CanExplainMatchResult) {
  Matcher<const AClass&> m = Property(&AClass::n, Ge(0));

  AClass a;
  a.set_n(1);
  EXPECT_EQ("whose given property is 1" + OfType("int"), Explain(m, a));

  m = Property(&AClass::n, GreaterThan(0));
  EXPECT_EQ(
      "whose given property is 1" + OfType("int") + ", which is 1 more than 0",
      Explain(m, a));
}

TEST(PropertyTest, CanExplainMatchResultWithPropertyName) {
  Matcher<const AClass&> m = Property("fancy_name", &AClass::n, Ge(0));

  AClass a;
  a.set_n(1);
  EXPECT_EQ("whose property `fancy_name` is 1" + OfType("int"), Explain(m, a));

  m = Property("fancy_name", &AClass::n, GreaterThan(0));
  EXPECT_EQ("whose property `fancy_name` is 1" + OfType("int") +
                ", which is 1 more than 0",
            Explain(m, a));
}

// Tests that Property() works when the argument is a pointer to const.
TEST(PropertyForPointerTest, WorksForPointerToConst) {
  Matcher<const AClass*> m = Property(&AClass::n, Ge(0));

  AClass a;
  a.set_n(1);
  EXPECT_TRUE(m.Matches(&a));

  a.set_n(-1);
  EXPECT_FALSE(m.Matches(&a));
}

// Tests that Property() works when the argument is a pointer to non-const.
TEST(PropertyForPointerTest, WorksForPointerToNonConst) {
  Matcher<AClass*> m = Property(&AClass::s, StartsWith("hi"));

  AClass a;
  a.set_s("hill");
  EXPECT_TRUE(m.Matches(&a));

  a.set_s("hole");
  EXPECT_FALSE(m.Matches(&a));
}

// Tests that Property() works when the argument is a reference to a
// const pointer.
TEST(PropertyForPointerTest, WorksForReferenceToConstPointer) {
  Matcher<AClass* const&> m = Property(&AClass::s, StartsWith("hi"));

  AClass a;
  a.set_s("hill");
  EXPECT_TRUE(m.Matches(&a));

  a.set_s("hole");
  EXPECT_FALSE(m.Matches(&a));
}

// Tests that Property() does not match the NULL pointer.
TEST(PropertyForPointerTest, WorksForReferenceToNonConstProperty) {
  Matcher<const AClass*> m = Property(&AClass::x, _);
  EXPECT_FALSE(m.Matches(nullptr));
}

// Tests that Property(&Foo::property, ...) works when the argument's
// type is a sub-type of const Foo*.
TEST(PropertyForPointerTest, WorksForArgumentOfSubType) {
  // The matcher expects a DerivedClass, but inside the Property() we
  // say AClass.
  Matcher<const DerivedClass*> m = Property(&AClass::n, Ge(0));

  DerivedClass d;
  d.set_n(1);
  EXPECT_TRUE(m.Matches(&d));

  d.set_n(-1);
  EXPECT_FALSE(m.Matches(&d));
}

// Tests that Property() can describe itself when used to match a pointer.
TEST(PropertyForPointerTest, CanDescribeSelf) {
  Matcher<const AClass*> m = Property(&AClass::n, Ge(0));

  EXPECT_EQ("is an object whose given property is >= 0", Describe(m));
  EXPECT_EQ("is an object whose given property isn't >= 0",
            DescribeNegation(m));
}

TEST(PropertyForPointerTest, CanDescribeSelfWithPropertyDescription) {
  Matcher<const AClass*> m = Property("fancy_name", &AClass::n, Ge(0));

  EXPECT_EQ("is an object whose property `fancy_name` is >= 0", Describe(m));
  EXPECT_EQ("is an object whose property `fancy_name` isn't >= 0",
            DescribeNegation(m));
}

// Tests that Property() can explain the result of matching a pointer.
TEST(PropertyForPointerTest, CanExplainMatchResult) {
  Matcher<const AClass*> m = Property(&AClass::n, Ge(0));

  AClass a;
  a.set_n(1);
  EXPECT_EQ("", Explain(m, static_cast<const AClass*>(nullptr)));
  EXPECT_EQ(
      "which points to an object whose given property is 1" + OfType("int"),
      Explain(m, &a));

  m = Property(&AClass::n, GreaterThan(0));
  EXPECT_EQ("which points to an object whose given property is 1" +
            OfType("int") + ", which is 1 more than 0",
            Explain(m, &a));
}

TEST(PropertyForPointerTest, CanExplainMatchResultWithPropertyName) {
  Matcher<const AClass*> m = Property("fancy_name", &AClass::n, Ge(0));

  AClass a;
  a.set_n(1);
  EXPECT_EQ("", Explain(m, static_cast<const AClass*>(nullptr)));
  EXPECT_EQ("which points to an object whose property `fancy_name` is 1" +
                OfType("int"),
            Explain(m, &a));

  m = Property("fancy_name", &AClass::n, GreaterThan(0));
  EXPECT_EQ("which points to an object whose property `fancy_name` is 1" +
                OfType("int") + ", which is 1 more than 0",
            Explain(m, &a));
}

// Tests ResultOf.

// Tests that ResultOf(f, ...) compiles and works as expected when f is a
// function pointer.
std::string IntToStringFunction(int input) {
  return input == 1 ? "foo" : "bar";
}

TEST(ResultOfTest, WorksForFunctionPointers) {
  Matcher<int> matcher = ResultOf(&IntToStringFunction, Eq(std::string("foo")));

  EXPECT_TRUE(matcher.Matches(1));
  EXPECT_FALSE(matcher.Matches(2));
}

// Tests that ResultOf() can describe itself.
TEST(ResultOfTest, CanDescribeItself) {
  Matcher<int> matcher = ResultOf(&IntToStringFunction, StrEq("foo"));

  EXPECT_EQ("is mapped by the given callable to a value that "
            "is equal to \"foo\"", Describe(matcher));
  EXPECT_EQ("is mapped by the given callable to a value that "
            "isn't equal to \"foo\"", DescribeNegation(matcher));
}

// Tests that ResultOf() can explain the match result.
int IntFunction(int input) { return input == 42 ? 80 : 90; }

TEST(ResultOfTest, CanExplainMatchResult) {
  Matcher<int> matcher = ResultOf(&IntFunction, Ge(85));
  EXPECT_EQ("which is mapped by the given callable to 90" + OfType("int"),
            Explain(matcher, 36));

  matcher = ResultOf(&IntFunction, GreaterThan(85));
  EXPECT_EQ("which is mapped by the given callable to 90" + OfType("int") +
            ", which is 5 more than 85", Explain(matcher, 36));
}

// Tests that ResultOf(f, ...) compiles and works as expected when f(x)
// returns a non-reference.
TEST(ResultOfTest, WorksForNonReferenceResults) {
  Matcher<int> matcher = ResultOf(&IntFunction, Eq(80));

  EXPECT_TRUE(matcher.Matches(42));
  EXPECT_FALSE(matcher.Matches(36));
}

// Tests that ResultOf(f, ...) compiles and works as expected when f(x)
// returns a reference to non-const.
double& DoubleFunction(double& input) { return input; }  // NOLINT

Uncopyable& RefUncopyableFunction(Uncopyable& obj) {  // NOLINT
  return obj;
}

TEST(ResultOfTest, WorksForReferenceToNonConstResults) {
  double x = 3.14;
  double x2 = x;
  Matcher<double&> matcher = ResultOf(&DoubleFunction, Ref(x));

  EXPECT_TRUE(matcher.Matches(x));
  EXPECT_FALSE(matcher.Matches(x2));

  // Test that ResultOf works with uncopyable objects
  Uncopyable obj(0);
  Uncopyable obj2(0);
  Matcher<Uncopyable&> matcher2 =
      ResultOf(&RefUncopyableFunction, Ref(obj));

  EXPECT_TRUE(matcher2.Matches(obj));
  EXPECT_FALSE(matcher2.Matches(obj2));
}

// Tests that ResultOf(f, ...) compiles and works as expected when f(x)
// returns a reference to const.
const std::string& StringFunction(const std::string& input) { return input; }

TEST(ResultOfTest, WorksForReferenceToConstResults) {
  std::string s = "foo";
  std::string s2 = s;
  Matcher<const std::string&> matcher = ResultOf(&StringFunction, Ref(s));

  EXPECT_TRUE(matcher.Matches(s));
  EXPECT_FALSE(matcher.Matches(s2));
}

// Tests that ResultOf(f, m) works when f(x) and m's
// argument types are compatible but different.
TEST(ResultOfTest, WorksForCompatibleMatcherTypes) {
  // IntFunction() returns int but the inner matcher expects a signed char.
  Matcher<int> matcher = ResultOf(IntFunction, Matcher<signed char>(Ge(85)));

  EXPECT_TRUE(matcher.Matches(36));
  EXPECT_FALSE(matcher.Matches(42));
}

// Tests that the program aborts when ResultOf is passed
// a NULL function pointer.
TEST(ResultOfDeathTest, DiesOnNullFunctionPointers) {
  EXPECT_DEATH_IF_SUPPORTED(
      ResultOf(static_cast<std::string (*)(int dummy)>(nullptr),
               Eq(std::string("foo"))),
      "NULL function pointer is passed into ResultOf\\(\\)\\.");
}

// Tests that ResultOf(f, ...) compiles and works as expected when f is a
// function reference.
TEST(ResultOfTest, WorksForFunctionReferences) {
  Matcher<int> matcher = ResultOf(IntToStringFunction, StrEq("foo"));
  EXPECT_TRUE(matcher.Matches(1));
  EXPECT_FALSE(matcher.Matches(2));
}

// Tests that ResultOf(f, ...) compiles and works as expected when f is a
// function object.
struct Functor {
  std::string operator()(int input) const {
    return IntToStringFunction(input);
  }
};

TEST(ResultOfTest, WorksForFunctors) {
  Matcher<int> matcher = ResultOf(Functor(), Eq(std::string("foo")));

  EXPECT_TRUE(matcher.Matches(1));
  EXPECT_FALSE(matcher.Matches(2));
}

// Tests that ResultOf(f, ...) compiles and works as expected when f is a
// functor with more than one operator() defined. ResultOf() must work
// for each defined operator().
struct PolymorphicFunctor {
  typedef int result_type;
  int operator()(int n) { return n; }
  int operator()(const char* s) { return static_cast<int>(strlen(s)); }
  std::string operator()(int *p) { return p ? "good ptr" : "null"; }
};

TEST(ResultOfTest, WorksForPolymorphicFunctors) {
  Matcher<int> matcher_int = ResultOf(PolymorphicFunctor(), Ge(5));

  EXPECT_TRUE(matcher_int.Matches(10));
  EXPECT_FALSE(matcher_int.Matches(2));

  Matcher<const char*> matcher_string = ResultOf(PolymorphicFunctor(), Ge(5));

  EXPECT_TRUE(matcher_string.Matches("long string"));
  EXPECT_FALSE(matcher_string.Matches("shrt"));
}

TEST(ResultOfTest, WorksForPolymorphicFunctorsIgnoringResultType) {
  Matcher<int*> matcher = ResultOf(PolymorphicFunctor(), "good ptr");

  int n = 0;
  EXPECT_TRUE(matcher.Matches(&n));
  EXPECT_FALSE(matcher.Matches(nullptr));
}

TEST(ResultOfTest, WorksForLambdas) {
  Matcher<int> matcher = ResultOf(
      [](int str_len) {
        return std::string(static_cast<size_t>(str_len), 'x');
      },
      "xxx");
  EXPECT_TRUE(matcher.Matches(3));
  EXPECT_FALSE(matcher.Matches(1));
}

TEST(ResultOfTest, WorksForNonCopyableArguments) {
  Matcher<std::unique_ptr<int>> matcher = ResultOf(
      [](const std::unique_ptr<int>& str_len) {
        return std::string(static_cast<size_t>(*str_len), 'x');
      },
      "xxx");
  EXPECT_TRUE(matcher.Matches(std::unique_ptr<int>(new int(3))));
  EXPECT_FALSE(matcher.Matches(std::unique_ptr<int>(new int(1))));
}

const int* ReferencingFunction(const int& n) { return &n; }

struct ReferencingFunctor {
  typedef const int* result_type;
  result_type operator()(const int& n) { return &n; }
};

TEST(ResultOfTest, WorksForReferencingCallables) {
  const int n = 1;
  const int n2 = 1;
  Matcher<const int&> matcher2 = ResultOf(ReferencingFunction, Eq(&n));
  EXPECT_TRUE(matcher2.Matches(n));
  EXPECT_FALSE(matcher2.Matches(n2));

  Matcher<const int&> matcher3 = ResultOf(ReferencingFunctor(), Eq(&n));
  EXPECT_TRUE(matcher3.Matches(n));
  EXPECT_FALSE(matcher3.Matches(n2));
}

class DivisibleByImpl {
 public:
  explicit DivisibleByImpl(int a_divider) : divider_(a_divider) {}

  // For testing using ExplainMatchResultTo() with polymorphic matchers.
  template <typename T>
  bool MatchAndExplain(const T& n, MatchResultListener* listener) const {
    *listener << "which is " << (n % divider_) << " modulo "
              << divider_;
    return (n % divider_) == 0;
  }

  void DescribeTo(ostream* os) const {
    *os << "is divisible by " << divider_;
  }

  void DescribeNegationTo(ostream* os) const {
    *os << "is not divisible by " << divider_;
  }

  void set_divider(int a_divider) { divider_ = a_divider; }
  int divider() const { return divider_; }

 private:
  int divider_;
};

PolymorphicMatcher<DivisibleByImpl> DivisibleBy(int n) {
  return MakePolymorphicMatcher(DivisibleByImpl(n));
}

// Tests that when AllOf() fails, only the first failing matcher is
// asked to explain why.
TEST(ExplainMatchResultTest, AllOf_False_False) {
  const Matcher<int> m = AllOf(DivisibleBy(4), DivisibleBy(3));
  EXPECT_EQ("which is 1 modulo 4", Explain(m, 5));
}

// Tests that when AllOf() fails, only the first failing matcher is
// asked to explain why.
TEST(ExplainMatchResultTest, AllOf_False_True) {
  const Matcher<int> m = AllOf(DivisibleBy(4), DivisibleBy(3));
  EXPECT_EQ("which is 2 modulo 4", Explain(m, 6));
}

// Tests that when AllOf() fails, only the first failing matcher is
// asked to explain why.
TEST(ExplainMatchResultTest, AllOf_True_False) {
  const Matcher<int> m = AllOf(Ge(1), DivisibleBy(3));
  EXPECT_EQ("which is 2 modulo 3", Explain(m, 5));
}

// Tests that when AllOf() succeeds, all matchers are asked to explain
// why.
TEST(ExplainMatchResultTest, AllOf_True_True) {
  const Matcher<int> m = AllOf(DivisibleBy(2), DivisibleBy(3));
  EXPECT_EQ("which is 0 modulo 2, and which is 0 modulo 3", Explain(m, 6));
}

TEST(ExplainMatchResultTest, AllOf_True_True_2) {
  const Matcher<int> m = AllOf(Ge(2), Le(3));
  EXPECT_EQ("", Explain(m, 2));
}

TEST(ExplainmatcherResultTest, MonomorphicMatcher) {
  const Matcher<int> m = GreaterThan(5);
  EXPECT_EQ("which is 1 more than 5", Explain(m, 6));
}

// The following two tests verify that values without a public copy
// ctor can be used as arguments to matchers like Eq(), Ge(), and etc
// with the help of ByRef().

class NotCopyable {
 public:
  explicit NotCopyable(int a_value) : value_(a_value) {}

  int value() const { return value_; }

  bool operator==(const NotCopyable& rhs) const {
    return value() == rhs.value();
  }

  bool operator>=(const NotCopyable& rhs) const {
    return value() >= rhs.value();
  }
 private:
  int value_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(NotCopyable);
};

TEST(ByRefTest, AllowsNotCopyableConstValueInMatchers) {
  const NotCopyable const_value1(1);
  const Matcher<const NotCopyable&> m = Eq(ByRef(const_value1));

  const NotCopyable n1(1), n2(2);
  EXPECT_TRUE(m.Matches(n1));
  EXPECT_FALSE(m.Matches(n2));
}

TEST(ByRefTest, AllowsNotCopyableValueInMatchers) {
  NotCopyable value2(2);
  const Matcher<NotCopyable&> m = Ge(ByRef(value2));

  NotCopyable n1(1), n2(2);
  EXPECT_FALSE(m.Matches(n1));
  EXPECT_TRUE(m.Matches(n2));
}

TEST(IsEmptyTest, ImplementsIsEmpty) {
  vector<int> container;
  EXPECT_THAT(container, IsEmpty());
  container.push_back(0);
  EXPECT_THAT(container, Not(IsEmpty()));
  container.push_back(1);
  EXPECT_THAT(container, Not(IsEmpty()));
}

TEST(IsEmptyTest, WorksWithString) {
  std::string text;
  EXPECT_THAT(text, IsEmpty());
  text = "foo";
  EXPECT_THAT(text, Not(IsEmpty()));
  text = std::string("\0", 1);
  EXPECT_THAT(text, Not(IsEmpty()));
}

TEST(IsEmptyTest, CanDescribeSelf) {
  Matcher<vector<int> > m = IsEmpty();
  EXPECT_EQ("is empty", Describe(m));
  EXPECT_EQ("isn't empty", DescribeNegation(m));
}

TEST(IsEmptyTest, ExplainsResult) {
  Matcher<vector<int> > m = IsEmpty();
  vector<int> container;
  EXPECT_EQ("", Explain(m, container));
  container.push_back(0);
  EXPECT_EQ("whose size is 1", Explain(m, container));
}

TEST(IsEmptyTest, WorksWithMoveOnly) {
  ContainerHelper helper;
  EXPECT_CALL(helper, Call(IsEmpty()));
  helper.Call({});
}

TEST(IsTrueTest, IsTrueIsFalse) {
  EXPECT_THAT(true, IsTrue());
  EXPECT_THAT(false, IsFalse());
  EXPECT_THAT(true, Not(IsFalse()));
  EXPECT_THAT(false, Not(IsTrue()));
  EXPECT_THAT(0, Not(IsTrue()));
  EXPECT_THAT(0, IsFalse());
  EXPECT_THAT(nullptr, Not(IsTrue()));
  EXPECT_THAT(nullptr, IsFalse());
  EXPECT_THAT(-1, IsTrue());
  EXPECT_THAT(-1, Not(IsFalse()));
  EXPECT_THAT(1, IsTrue());
  EXPECT_THAT(1, Not(IsFalse()));
  EXPECT_THAT(2, IsTrue());
  EXPECT_THAT(2, Not(IsFalse()));
  int a = 42;
  EXPECT_THAT(a, IsTrue());
  EXPECT_THAT(a, Not(IsFalse()));
  EXPECT_THAT(&a, IsTrue());
  EXPECT_THAT(&a, Not(IsFalse()));
  EXPECT_THAT(false, Not(IsTrue()));
  EXPECT_THAT(true, Not(IsFalse()));
  EXPECT_THAT(std::true_type(), IsTrue());
  EXPECT_THAT(std::true_type(), Not(IsFalse()));
  EXPECT_THAT(std::false_type(), IsFalse());
  EXPECT_THAT(std::false_type(), Not(IsTrue()));
  EXPECT_THAT(nullptr, Not(IsTrue()));
  EXPECT_THAT(nullptr, IsFalse());
  std::unique_ptr<int> null_unique;
  std::unique_ptr<int> nonnull_unique(new int(0));
  EXPECT_THAT(null_unique, Not(IsTrue()));
  EXPECT_THAT(null_unique, IsFalse());
  EXPECT_THAT(nonnull_unique, IsTrue());
  EXPECT_THAT(nonnull_unique, Not(IsFalse()));
}

TEST(SizeIsTest, ImplementsSizeIs) {
  vector<int> container;
  EXPECT_THAT(container, SizeIs(0));
  EXPECT_THAT(container, Not(SizeIs(1)));
  container.push_back(0);
  EXPECT_THAT(container, Not(SizeIs(0)));
  EXPECT_THAT(container, SizeIs(1));
  container.push_back(0);
  EXPECT_THAT(container, Not(SizeIs(0)));
  EXPECT_THAT(container, SizeIs(2));
}

TEST(SizeIsTest, WorksWithMap) {
  map<std::string, int> container;
  EXPECT_THAT(container, SizeIs(0));
  EXPECT_THAT(container, Not(SizeIs(1)));
  container.insert(make_pair("foo", 1));
  EXPECT_THAT(container, Not(SizeIs(0)));
  EXPECT_THAT(container, SizeIs(1));
  container.insert(make_pair("bar", 2));
  EXPECT_THAT(container, Not(SizeIs(0)));
  EXPECT_THAT(container, SizeIs(2));
}

TEST(SizeIsTest, WorksWithReferences) {
  vector<int> container;
  Matcher<const vector<int>&> m = SizeIs(1);
  EXPECT_THAT(container, Not(m));
  container.push_back(0);
  EXPECT_THAT(container, m);
}

TEST(SizeIsTest, WorksWithMoveOnly) {
  ContainerHelper helper;
  EXPECT_CALL(helper, Call(SizeIs(3)));
  helper.Call(MakeUniquePtrs({1, 2, 3}));
}

// SizeIs should work for any type that provides a size() member function.
// For example, a size_type member type should not need to be provided.
struct MinimalistCustomType {
  int size() const { return 1; }
};
TEST(SizeIsTest, WorksWithMinimalistCustomType) {
  MinimalistCustomType container;
  EXPECT_THAT(container, SizeIs(1));
  EXPECT_THAT(container, Not(SizeIs(0)));
}

TEST(SizeIsTest, CanDescribeSelf) {
  Matcher<vector<int> > m = SizeIs(2);
  EXPECT_EQ("size is equal to 2", Describe(m));
  EXPECT_EQ("size isn't equal to 2", DescribeNegation(m));
}

TEST(SizeIsTest, ExplainsResult) {
  Matcher<vector<int> > m1 = SizeIs(2);
  Matcher<vector<int> > m2 = SizeIs(Lt(2u));
  Matcher<vector<int> > m3 = SizeIs(AnyOf(0, 3));
  Matcher<vector<int> > m4 = SizeIs(Gt(1u));
  vector<int> container;
  EXPECT_EQ("whose size 0 doesn't match", Explain(m1, container));
  EXPECT_EQ("whose size 0 matches", Explain(m2, container));
  EXPECT_EQ("whose size 0 matches", Explain(m3, container));
  EXPECT_EQ("whose size 0 doesn't match", Explain(m4, container));
  container.push_back(0);
  container.push_back(0);
  EXPECT_EQ("whose size 2 matches", Explain(m1, container));
  EXPECT_EQ("whose size 2 doesn't match", Explain(m2, container));
  EXPECT_EQ("whose size 2 doesn't match", Explain(m3, container));
  EXPECT_EQ("whose size 2 matches", Explain(m4, container));
}

#if GTEST_HAS_TYPED_TEST
// Tests ContainerEq with different container types, and
// different element types.

template <typename T>
class ContainerEqTest : public testing::Test {};

typedef testing::Types<
    set<int>,
    vector<size_t>,
    multiset<size_t>,
    list<int> >
    ContainerEqTestTypes;

TYPED_TEST_SUITE(ContainerEqTest, ContainerEqTestTypes);

// Tests that the filled container is equal to itself.
TYPED_TEST(ContainerEqTest, EqualsSelf) {
  static const int vals[] = {1, 1, 2, 3, 5, 8};
  TypeParam my_set(vals, vals + 6);
  const Matcher<TypeParam> m = ContainerEq(my_set);
  EXPECT_TRUE(m.Matches(my_set));
  EXPECT_EQ("", Explain(m, my_set));
}

// Tests that missing values are reported.
TYPED_TEST(ContainerEqTest, ValueMissing) {
  static const int vals[] = {1, 1, 2, 3, 5, 8};
  static const int test_vals[] = {2, 1, 8, 5};
  TypeParam my_set(vals, vals + 6);
  TypeParam test_set(test_vals, test_vals + 4);
  const Matcher<TypeParam> m = ContainerEq(my_set);
  EXPECT_FALSE(m.Matches(test_set));
  EXPECT_EQ("which doesn't have these expected elements: 3",
            Explain(m, test_set));
}

// Tests that added values are reported.
TYPED_TEST(ContainerEqTest, ValueAdded) {
  static const int vals[] = {1, 1, 2, 3, 5, 8};
  static const int test_vals[] = {1, 2, 3, 5, 8, 46};
  TypeParam my_set(vals, vals + 6);
  TypeParam test_set(test_vals, test_vals + 6);
  const Matcher<const TypeParam&> m = ContainerEq(my_set);
  EXPECT_FALSE(m.Matches(test_set));
  EXPECT_EQ("which has these unexpected elements: 46", Explain(m, test_set));
}

// Tests that added and missing values are reported together.
TYPED_TEST(ContainerEqTest, ValueAddedAndRemoved) {
  static const int vals[] = {1, 1, 2, 3, 5, 8};
  static const int test_vals[] = {1, 2, 3, 8, 46};
  TypeParam my_set(vals, vals + 6);
  TypeParam test_set(test_vals, test_vals + 5);
  const Matcher<TypeParam> m = ContainerEq(my_set);
  EXPECT_FALSE(m.Matches(test_set));
  EXPECT_EQ("which has these unexpected elements: 46,\n"
            "and doesn't have these expected elements: 5",
            Explain(m, test_set));
}

// Tests duplicated value -- expect no explanation.
TYPED_TEST(ContainerEqTest, DuplicateDifference) {
  static const int vals[] = {1, 1, 2, 3, 5, 8};
  static const int test_vals[] = {1, 2, 3, 5, 8};
  TypeParam my_set(vals, vals + 6);
  TypeParam test_set(test_vals, test_vals + 5);
  const Matcher<const TypeParam&> m = ContainerEq(my_set);
  // Depending on the container, match may be true or false
  // But in any case there should be no explanation.
  EXPECT_EQ("", Explain(m, test_set));
}
#endif  // GTEST_HAS_TYPED_TEST

// Tests that multiple missing values are reported.
// Using just vector here, so order is predictable.
TEST(ContainerEqExtraTest, MultipleValuesMissing) {
  static const int vals[] = {1, 1, 2, 3, 5, 8};
  static const int test_vals[] = {2, 1, 5};
  vector<int> my_set(vals, vals + 6);
  vector<int> test_set(test_vals, test_vals + 3);
  const Matcher<vector<int> > m = ContainerEq(my_set);
  EXPECT_FALSE(m.Matches(test_set));
  EXPECT_EQ("which doesn't have these expected elements: 3, 8",
            Explain(m, test_set));
}

// Tests that added values are reported.
// Using just vector here, so order is predictable.
TEST(ContainerEqExtraTest, MultipleValuesAdded) {
  static const int vals[] = {1, 1, 2, 3, 5, 8};
  static const int test_vals[] = {1, 2, 92, 3, 5, 8, 46};
  list<size_t> my_set(vals, vals + 6);
  list<size_t> test_set(test_vals, test_vals + 7);
  const Matcher<const list<size_t>&> m = ContainerEq(my_set);
  EXPECT_FALSE(m.Matches(test_set));
  EXPECT_EQ("which has these unexpected elements: 92, 46",
            Explain(m, test_set));
}

// Tests that added and missing values are reported together.
TEST(ContainerEqExtraTest, MultipleValuesAddedAndRemoved) {
  static const int vals[] = {1, 1, 2, 3, 5, 8};
  static const int test_vals[] = {1, 2, 3, 92, 46};
  list<size_t> my_set(vals, vals + 6);
  list<size_t> test_set(test_vals, test_vals + 5);
  const Matcher<const list<size_t> > m = ContainerEq(my_set);
  EXPECT_FALSE(m.Matches(test_set));
  EXPECT_EQ("which has these unexpected elements: 92, 46,\n"
            "and doesn't have these expected elements: 5, 8",
            Explain(m, test_set));
}

// Tests to see that duplicate elements are detected,
// but (as above) not reported in the explanation.
TEST(ContainerEqExtraTest, MultiSetOfIntDuplicateDifference) {
  static const int vals[] = {1, 1, 2, 3, 5, 8};
  static const int test_vals[] = {1, 2, 3, 5, 8};
  vector<int> my_set(vals, vals + 6);
  vector<int> test_set(test_vals, test_vals + 5);
  const Matcher<vector<int> > m = ContainerEq(my_set);
  EXPECT_TRUE(m.Matches(my_set));
  EXPECT_FALSE(m.Matches(test_set));
  // There is nothing to report when both sets contain all the same values.
  EXPECT_EQ("", Explain(m, test_set));
}

// Tests that ContainerEq works for non-trivial associative containers,
// like maps.
TEST(ContainerEqExtraTest, WorksForMaps) {
  map<int, std::string> my_map;
  my_map[0] = "a";
  my_map[1] = "b";

  map<int, std::string> test_map;
  test_map[0] = "aa";
  test_map[1] = "b";

  const Matcher<const map<int, std::string>&> m = ContainerEq(my_map);
  EXPECT_TRUE(m.Matches(my_map));
  EXPECT_FALSE(m.Matches(test_map));

  EXPECT_EQ("which has these unexpected elements: (0, \"aa\"),\n"
            "and doesn't have these expected elements: (0, \"a\")",
            Explain(m, test_map));
}

TEST(ContainerEqExtraTest, WorksForNativeArray) {
  int a1[] = {1, 2, 3};
  int a2[] = {1, 2, 3};
  int b[] = {1, 2, 4};

  EXPECT_THAT(a1, ContainerEq(a2));
  EXPECT_THAT(a1, Not(ContainerEq(b)));
}

TEST(ContainerEqExtraTest, WorksForTwoDimensionalNativeArray) {
  const char a1[][3] = {"hi", "lo"};
  const char a2[][3] = {"hi", "lo"};
  const char b[][3] = {"lo", "hi"};

  // Tests using ContainerEq() in the first dimension.
  EXPECT_THAT(a1, ContainerEq(a2));
  EXPECT_THAT(a1, Not(ContainerEq(b)));

  // Tests using ContainerEq() in the second dimension.
  EXPECT_THAT(a1, ElementsAre(ContainerEq(a2[0]), ContainerEq(a2[1])));
  EXPECT_THAT(a1, ElementsAre(Not(ContainerEq(b[0])), ContainerEq(a2[1])));
}

TEST(ContainerEqExtraTest, WorksForNativeArrayAsTuple) {
  const int a1[] = {1, 2, 3};
  const int a2[] = {1, 2, 3};
  const int b[] = {1, 2, 3, 4};

  const int* const p1 = a1;
  EXPECT_THAT(std::make_tuple(p1, 3), ContainerEq(a2));
  EXPECT_THAT(std::make_tuple(p1, 3), Not(ContainerEq(b)));

  const int c[] = {1, 3, 2};
  EXPECT_THAT(std::make_tuple(p1, 3), Not(ContainerEq(c)));
}

TEST(ContainerEqExtraTest, CopiesNativeArrayParameter) {
  std::string a1[][3] = {
    {"hi", "hello", "ciao"},
    {"bye", "see you", "ciao"}
  };

  std::string a2[][3] = {
    {"hi", "hello", "ciao"},
    {"bye", "see you", "ciao"}
  };

  const Matcher<const std::string(&)[2][3]> m = ContainerEq(a2);
  EXPECT_THAT(a1, m);

  a2[0][0] = "ha";
  EXPECT_THAT(a1, m);
}

TEST(WhenSortedByTest, WorksForEmptyContainer) {
  const vector<int> numbers;
  EXPECT_THAT(numbers, WhenSortedBy(less<int>(), ElementsAre()));
  EXPECT_THAT(numbers, Not(WhenSortedBy(less<int>(), ElementsAre(1))));
}

TEST(WhenSortedByTest, WorksForNonEmptyContainer) {
  vector<unsigned> numbers;
  numbers.push_back(3);
  numbers.push_back(1);
  numbers.push_back(2);
  numbers.push_back(2);
  EXPECT_THAT(numbers, WhenSortedBy(greater<unsigned>(),
                                    ElementsAre(3, 2, 2, 1)));
  EXPECT_THAT(numbers, Not(WhenSortedBy(greater<unsigned>(),
                                        ElementsAre(1, 2, 2, 3))));
}

TEST(WhenSortedByTest, WorksForNonVectorContainer) {
  list<std::string> words;
  words.push_back("say");
  words.push_back("hello");
  words.push_back("world");
  EXPECT_THAT(words, WhenSortedBy(less<std::string>(),
                                  ElementsAre("hello", "say", "world")));
  EXPECT_THAT(words, Not(WhenSortedBy(less<std::string>(),
                                      ElementsAre("say", "hello", "world"))));
}

TEST(WhenSortedByTest, WorksForNativeArray) {
  const int numbers[] = {1, 3, 2, 4};
  const int sorted_numbers[] = {1, 2, 3, 4};
  EXPECT_THAT(numbers, WhenSortedBy(less<int>(), ElementsAre(1, 2, 3, 4)));
  EXPECT_THAT(numbers, WhenSortedBy(less<int>(),
                                    ElementsAreArray(sorted_numbers)));
  EXPECT_THAT(numbers, Not(WhenSortedBy(less<int>(), ElementsAre(1, 3, 2, 4))));
}

TEST(WhenSortedByTest, CanDescribeSelf) {
  const Matcher<vector<int> > m = WhenSortedBy(less<int>(), ElementsAre(1, 2));
  EXPECT_EQ("(when sorted) has 2 elements where\n"
            "element #0 is equal to 1,\n"
            "element #1 is equal to 2",
            Describe(m));
  EXPECT_EQ("(when sorted) doesn't have 2 elements, or\n"
            "element #0 isn't equal to 1, or\n"
            "element #1 isn't equal to 2",
            DescribeNegation(m));
}

TEST(WhenSortedByTest, ExplainsMatchResult) {
  const int a[] = {2, 1};
  EXPECT_EQ("which is { 1, 2 } when sorted, whose element #0 doesn't match",
            Explain(WhenSortedBy(less<int>(), ElementsAre(2, 3)), a));
  EXPECT_EQ("which is { 1, 2 } when sorted",
            Explain(WhenSortedBy(less<int>(), ElementsAre(1, 2)), a));
}

// WhenSorted() is a simple wrapper on WhenSortedBy().  Hence we don't
// need to test it as exhaustively as we test the latter.

TEST(WhenSortedTest, WorksForEmptyContainer) {
  const vector<int> numbers;
  EXPECT_THAT(numbers, WhenSorted(ElementsAre()));
  EXPECT_THAT(numbers, Not(WhenSorted(ElementsAre(1))));
}

TEST(WhenSortedTest, WorksForNonEmptyContainer) {
  list<std::string> words;
  words.push_back("3");
  words.push_back("1");
  words.push_back("2");
  words.push_back("2");
  EXPECT_THAT(words, WhenSorted(ElementsAre("1", "2", "2", "3")));
  EXPECT_THAT(words, Not(WhenSorted(ElementsAre("3", "1", "2", "2"))));
}

TEST(WhenSortedTest, WorksForMapTypes) {
  map<std::string, int> word_counts;
  word_counts["and"] = 1;
  word_counts["the"] = 1;
  word_counts["buffalo"] = 2;
  EXPECT_THAT(word_counts,
              WhenSorted(ElementsAre(Pair("and", 1), Pair("buffalo", 2),
                                     Pair("the", 1))));
  EXPECT_THAT(word_counts,
              Not(WhenSorted(ElementsAre(Pair("and", 1), Pair("the", 1),
                                         Pair("buffalo", 2)))));
}

TEST(WhenSortedTest, WorksForMultiMapTypes) {
    multimap<int, int> ifib;
    ifib.insert(make_pair(8, 6));
    ifib.insert(make_pair(2, 3));
    ifib.insert(make_pair(1, 1));
    ifib.insert(make_pair(3, 4));
    ifib.insert(make_pair(1, 2));
    ifib.insert(make_pair(5, 5));
    EXPECT_THAT(ifib, WhenSorted(ElementsAre(Pair(1, 1),
                                             Pair(1, 2),
                                             Pair(2, 3),
                                             Pair(3, 4),
                                             Pair(5, 5),
                                             Pair(8, 6))));
    EXPECT_THAT(ifib, Not(WhenSorted(ElementsAre(Pair(8, 6),
                                                 Pair(2, 3),
                                                 Pair(1, 1),
                                                 Pair(3, 4),
                                                 Pair(1, 2),
                                                 Pair(5, 5)))));
}

TEST(WhenSortedTest, WorksForPolymorphicMatcher) {
    std::deque<int> d;
    d.push_back(2);
    d.push_back(1);
    EXPECT_THAT(d, WhenSorted(ElementsAre(1, 2)));
    EXPECT_THAT(d, Not(WhenSorted(ElementsAre(2, 1))));
}

TEST(WhenSortedTest, WorksForVectorConstRefMatcher) {
    std::deque<int> d;
    d.push_back(2);
    d.push_back(1);
    Matcher<const std::vector<int>&> vector_match = ElementsAre(1, 2);
    EXPECT_THAT(d, WhenSorted(vector_match));
    Matcher<const std::vector<int>&> not_vector_match = ElementsAre(2, 1);
    EXPECT_THAT(d, Not(WhenSorted(not_vector_match)));
}

// Deliberately bare pseudo-container.
// Offers only begin() and end() accessors, yielding InputIterator.
template <typename T>
class Streamlike {
 private:
  class ConstIter;
 public:
  typedef ConstIter const_iterator;
  typedef T value_type;

  template <typename InIter>
  Streamlike(InIter first, InIter last) : remainder_(first, last) {}

  const_iterator begin() const {
    return const_iterator(this, remainder_.begin());
  }
  const_iterator end() const {
    return const_iterator(this, remainder_.end());
  }

 private:
  class ConstIter : public std::iterator<std::input_iterator_tag,
                                         value_type,
                                         ptrdiff_t,
                                         const value_type*,
                                         const value_type&> {
   public:
    ConstIter(const Streamlike* s,
              typename std::list<value_type>::iterator pos)
        : s_(s), pos_(pos) {}

    const value_type& operator*() const { return *pos_; }
    const value_type* operator->() const { return &*pos_; }
    ConstIter& operator++() {
      s_->remainder_.erase(pos_++);
      return *this;
    }

    // *iter++ is required to work (see std::istreambuf_iterator).
    // (void)iter++ is also required to work.
    class PostIncrProxy {
     public:
      explicit PostIncrProxy(const value_type& value) : value_(value) {}
      value_type operator*() const { return value_; }
     private:
      value_type value_;
    };
    PostIncrProxy operator++(int) {
      PostIncrProxy proxy(**this);
      ++(*this);
      return proxy;
    }

    friend bool operator==(const ConstIter& a, const ConstIter& b) {
      return a.s_ == b.s_ && a.pos_ == b.pos_;
    }
    friend bool operator!=(const ConstIter& a, const ConstIter& b) {
      return !(a == b);
    }

   private:
    const Streamlike* s_;
    typename std::list<value_type>::iterator pos_;
  };

  friend std::ostream& operator<<(std::ostream& os, const Streamlike& s) {
    os << "[";
    typedef typename std::list<value_type>::const_iterator Iter;
    const char* sep = "";
    for (Iter it = s.remainder_.begin(); it != s.remainder_.end(); ++it) {
      os << sep << *it;
      sep = ",";
    }
    os << "]";
    return os;
  }

  mutable std::list<value_type> remainder_;  // modified by iteration
};

TEST(StreamlikeTest, Iteration) {
  const int a[5] = {2, 1, 4, 5, 3};
  Streamlike<int> s(a, a + 5);
  Streamlike<int>::const_iterator it = s.begin();
  const int* ip = a;
  while (it != s.end()) {
    SCOPED_TRACE(ip - a);
    EXPECT_EQ(*ip++, *it++);
  }
}

TEST(BeginEndDistanceIsTest, WorksWithForwardList) {
  std::forward_list<int> container;
  EXPECT_THAT(container, BeginEndDistanceIs(0));
  EXPECT_THAT(container, Not(BeginEndDistanceIs(1)));
  container.push_front(0);
  EXPECT_THAT(container, Not(BeginEndDistanceIs(0)));
  EXPECT_THAT(container, BeginEndDistanceIs(1));
  container.push_front(0);
  EXPECT_THAT(container, Not(BeginEndDistanceIs(0)));
  EXPECT_THAT(container, BeginEndDistanceIs(2));
}

TEST(BeginEndDistanceIsTest, WorksWithNonStdList) {
  const int a[5] = {1, 2, 3, 4, 5};
  Streamlike<int> s(a, a + 5);
  EXPECT_THAT(s, BeginEndDistanceIs(5));
}

TEST(BeginEndDistanceIsTest, CanDescribeSelf) {
  Matcher<vector<int> > m = BeginEndDistanceIs(2);
  EXPECT_EQ("distance between begin() and end() is equal to 2", Describe(m));
  EXPECT_EQ("distance between begin() and end() isn't equal to 2",
            DescribeNegation(m));
}

TEST(BeginEndDistanceIsTest, WorksWithMoveOnly) {
  ContainerHelper helper;
  EXPECT_CALL(helper, Call(BeginEndDistanceIs(2)));
  helper.Call(MakeUniquePtrs({1, 2}));
}

TEST(BeginEndDistanceIsTest, ExplainsResult) {
  Matcher<vector<int> > m1 = BeginEndDistanceIs(2);
  Matcher<vector<int> > m2 = BeginEndDistanceIs(Lt(2));
  Matcher<vector<int> > m3 = BeginEndDistanceIs(AnyOf(0, 3));
  Matcher<vector<int> > m4 = BeginEndDistanceIs(GreaterThan(1));
  vector<int> container;
  EXPECT_EQ("whose distance between begin() and end() 0 doesn't match",
            Explain(m1, container));
  EXPECT_EQ("whose distance between begin() and end() 0 matches",
            Explain(m2, container));
  EXPECT_EQ("whose distance between begin() and end() 0 matches",
            Explain(m3, container));
  EXPECT_EQ(
      "whose distance between begin() and end() 0 doesn't match, which is 1 "
      "less than 1",
      Explain(m4, container));
  container.push_back(0);
  container.push_back(0);
  EXPECT_EQ("whose distance between begin() and end() 2 matches",
            Explain(m1, container));
  EXPECT_EQ("whose distance between begin() and end() 2 doesn't match",
            Explain(m2, container));
  EXPECT_EQ("whose distance between begin() and end() 2 doesn't match",
            Explain(m3, container));
  EXPECT_EQ(
      "whose distance between begin() and end() 2 matches, which is 1 more "
      "than 1",
      Explain(m4, container));
}

TEST(WhenSortedTest, WorksForStreamlike) {
  // Streamlike 'container' provides only minimal iterator support.
  // Its iterators are tagged with input_iterator_tag.
  const int a[5] = {2, 1, 4, 5, 3};
  Streamlike<int> s(std::begin(a), std::end(a));
  EXPECT_THAT(s, WhenSorted(ElementsAre(1, 2, 3, 4, 5)));
  EXPECT_THAT(s, Not(WhenSorted(ElementsAre(2, 1, 4, 5, 3))));
}

TEST(WhenSortedTest, WorksForVectorConstRefMatcherOnStreamlike) {
  const int a[] = {2, 1, 4, 5, 3};
  Streamlike<int> s(std::begin(a), std::end(a));
  Matcher<const std::vector<int>&> vector_match = ElementsAre(1, 2, 3, 4, 5);
  EXPECT_THAT(s, WhenSorted(vector_match));
  EXPECT_THAT(s, Not(WhenSorted(ElementsAre(2, 1, 4, 5, 3))));
}

TEST(IsSupersetOfTest, WorksForNativeArray) {
  const int subset[] = {1, 4};
  const int superset[] = {1, 2, 4};
  const int disjoint[] = {1, 0, 3};
  EXPECT_THAT(subset, IsSupersetOf(subset));
  EXPECT_THAT(subset, Not(IsSupersetOf(superset)));
  EXPECT_THAT(superset, IsSupersetOf(subset));
  EXPECT_THAT(subset, Not(IsSupersetOf(disjoint)));
  EXPECT_THAT(disjoint, Not(IsSupersetOf(subset)));
}

TEST(IsSupersetOfTest, WorksWithDuplicates) {
  const int not_enough[] = {1, 2};
  const int enough[] = {1, 1, 2};
  const int expected[] = {1, 1};
  EXPECT_THAT(not_enough, Not(IsSupersetOf(expected)));
  EXPECT_THAT(enough, IsSupersetOf(expected));
}

TEST(IsSupersetOfTest, WorksForEmpty) {
  vector<int> numbers;
  vector<int> expected;
  EXPECT_THAT(numbers, IsSupersetOf(expected));
  expected.push_back(1);
  EXPECT_THAT(numbers, Not(IsSupersetOf(expected)));
  expected.clear();
  numbers.push_back(1);
  numbers.push_back(2);
  EXPECT_THAT(numbers, IsSupersetOf(expected));
  expected.push_back(1);
  EXPECT_THAT(numbers, IsSupersetOf(expected));
  expected.push_back(2);
  EXPECT_THAT(numbers, IsSupersetOf(expected));
  expected.push_back(3);
  EXPECT_THAT(numbers, Not(IsSupersetOf(expected)));
}

TEST(IsSupersetOfTest, WorksForStreamlike) {
  const int a[5] = {1, 2, 3, 4, 5};
  Streamlike<int> s(std::begin(a), std::end(a));

  vector<int> expected;
  expected.push_back(1);
  expected.push_back(2);
  expected.push_back(5);
  EXPECT_THAT(s, IsSupersetOf(expected));

  expected.push_back(0);
  EXPECT_THAT(s, Not(IsSupersetOf(expected)));
}

TEST(IsSupersetOfTest, TakesStlContainer) {
  const int actual[] = {3, 1, 2};

  ::std::list<int> expected;
  expected.push_back(1);
  expected.push_back(3);
  EXPECT_THAT(actual, IsSupersetOf(expected));

  expected.push_back(4);
  EXPECT_THAT(actual, Not(IsSupersetOf(expected)));
}

TEST(IsSupersetOfTest, Describe) {
  typedef std::vector<int> IntVec;
  IntVec expected;
  expected.push_back(111);
  expected.push_back(222);
  expected.push_back(333);
  EXPECT_THAT(
      Describe<IntVec>(IsSupersetOf(expected)),
      Eq("a surjection from elements to requirements exists such that:\n"
         " - an element is equal to 111\n"
         " - an element is equal to 222\n"
         " - an element is equal to 333"));
}

TEST(IsSupersetOfTest, DescribeNegation) {
  typedef std::vector<int> IntVec;
  IntVec expected;
  expected.push_back(111);
  expected.push_back(222);
  expected.push_back(333);
  EXPECT_THAT(
      DescribeNegation<IntVec>(IsSupersetOf(expected)),
      Eq("no surjection from elements to requirements exists such that:\n"
         " - an element is equal to 111\n"
         " - an element is equal to 222\n"
         " - an element is equal to 333"));
}

TEST(IsSupersetOfTest, MatchAndExplain) {
  std::vector<int> v;
  v.push_back(2);
  v.push_back(3);
  std::vector<int> expected;
  expected.push_back(1);
  expected.push_back(2);
  StringMatchResultListener listener;
  ASSERT_FALSE(ExplainMatchResult(IsSupersetOf(expected), v, &listener))
      << listener.str();
  EXPECT_THAT(listener.str(),
              Eq("where the following matchers don't match any elements:\n"
                 "matcher #0: is equal to 1"));

  v.push_back(1);
  listener.Clear();
  ASSERT_TRUE(ExplainMatchResult(IsSupersetOf(expected), v, &listener))
      << listener.str();
  EXPECT_THAT(listener.str(), Eq("where:\n"
                                 " - element #0 is matched by matcher #1,\n"
                                 " - element #2 is matched by matcher #0"));
}

TEST(IsSupersetOfTest, WorksForRhsInitializerList) {
  const int numbers[] = {1, 3, 6, 2, 4, 5};
  EXPECT_THAT(numbers, IsSupersetOf({1, 2}));
  EXPECT_THAT(numbers, Not(IsSupersetOf({3, 0})));
}

TEST(IsSupersetOfTest, WorksWithMoveOnly) {
  ContainerHelper helper;
  EXPECT_CALL(helper, Call(IsSupersetOf({Pointee(1)})));
  helper.Call(MakeUniquePtrs({1, 2}));
  EXPECT_CALL(helper, Call(Not(IsSupersetOf({Pointee(1), Pointee(2)}))));
  helper.Call(MakeUniquePtrs({2}));
}

TEST(IsSubsetOfTest, WorksForNativeArray) {
  const int subset[] = {1, 4};
  const int superset[] = {1, 2, 4};
  const int disjoint[] = {1, 0, 3};
  EXPECT_THAT(subset, IsSubsetOf(subset));
  EXPECT_THAT(subset, IsSubsetOf(superset));
  EXPECT_THAT(superset, Not(IsSubsetOf(subset)));
  EXPECT_THAT(subset, Not(IsSubsetOf(disjoint)));
  EXPECT_THAT(disjoint, Not(IsSubsetOf(subset)));
}

TEST(IsSubsetOfTest, WorksWithDuplicates) {
  const int not_enough[] = {1, 2};
  const int enough[] = {1, 1, 2};
  const int actual[] = {1, 1};
  EXPECT_THAT(actual, Not(IsSubsetOf(not_enough)));
  EXPECT_THAT(actual, IsSubsetOf(enough));
}

TEST(IsSubsetOfTest, WorksForEmpty) {
  vector<int> numbers;
  vector<int> expected;
  EXPECT_THAT(numbers, IsSubsetOf(expected));
  expected.push_back(1);
  EXPECT_THAT(numbers, IsSubsetOf(expected));
  expected.clear();
  numbers.push_back(1);
  numbers.push_back(2);
  EXPECT_THAT(numbers, Not(IsSubsetOf(expected)));
  expected.push_back(1);
  EXPECT_THAT(numbers, Not(IsSubsetOf(expected)));
  expected.push_back(2);
  EXPECT_THAT(numbers, IsSubsetOf(expected));
  expected.push_back(3);
  EXPECT_THAT(numbers, IsSubsetOf(expected));
}

TEST(IsSubsetOfTest, WorksForStreamlike) {
  const int a[5] = {1, 2};
  Streamlike<int> s(std::begin(a), std::end(a));

  vector<int> expected;
  expected.push_back(1);
  EXPECT_THAT(s, Not(IsSubsetOf(expected)));
  expected.push_back(2);
  expected.push_back(5);
  EXPECT_THAT(s, IsSubsetOf(expected));
}

TEST(IsSubsetOfTest, TakesStlContainer) {
  const int actual[] = {3, 1, 2};

  ::std::list<int> expected;
  expected.push_back(1);
  expected.push_back(3);
  EXPECT_THAT(actual, Not(IsSubsetOf(expected)));

  expected.push_back(2);
  expected.push_back(4);
  EXPECT_THAT(actual, IsSubsetOf(expected));
}

TEST(IsSubsetOfTest, Describe) {
  typedef std::vector<int> IntVec;
  IntVec expected;
  expected.push_back(111);
  expected.push_back(222);
  expected.push_back(333);

  EXPECT_THAT(
      Describe<IntVec>(IsSubsetOf(expected)),
      Eq("an injection from elements to requirements exists such that:\n"
         " - an element is equal to 111\n"
         " - an element is equal to 222\n"
         " - an element is equal to 333"));
}

TEST(IsSubsetOfTest, DescribeNegation) {
  typedef std::vector<int> IntVec;
  IntVec expected;
  expected.push_back(111);
  expected.push_back(222);
  expected.push_back(333);
  EXPECT_THAT(
      DescribeNegation<IntVec>(IsSubsetOf(expected)),
      Eq("no injection from elements to requirements exists such that:\n"
         " - an element is equal to 111\n"
         " - an element is equal to 222\n"
         " - an element is equal to 333"));
}

TEST(IsSubsetOfTest, MatchAndExplain) {
  std::vector<int> v;
  v.push_back(2);
  v.push_back(3);
  std::vector<int> expected;
  expected.push_back(1);
  expected.push_back(2);
  StringMatchResultListener listener;
  ASSERT_FALSE(ExplainMatchResult(IsSubsetOf(expected), v, &listener))
      << listener.str();
  EXPECT_THAT(listener.str(),
              Eq("where the following elements don't match any matchers:\n"
                 "element #1: 3"));

  expected.push_back(3);
  listener.Clear();
  ASSERT_TRUE(ExplainMatchResult(IsSubsetOf(expected), v, &listener))
      << listener.str();
  EXPECT_THAT(listener.str(), Eq("where:\n"
                                 " - element #0 is matched by matcher #1,\n"
                                 " - element #1 is matched by matcher #2"));
}

TEST(IsSubsetOfTest, WorksForRhsInitializerList) {
  const int numbers[] = {1, 2, 3};
  EXPECT_THAT(numbers, IsSubsetOf({1, 2, 3, 4}));
  EXPECT_THAT(numbers, Not(IsSubsetOf({1, 2})));
}

TEST(IsSubsetOfTest, WorksWithMoveOnly) {
  ContainerHelper helper;
  EXPECT_CALL(helper, Call(IsSubsetOf({Pointee(1), Pointee(2)})));
  helper.Call(MakeUniquePtrs({1}));
  EXPECT_CALL(helper, Call(Not(IsSubsetOf({Pointee(1)}))));
  helper.Call(MakeUniquePtrs({2}));
}

// Tests using ElementsAre() and ElementsAreArray() with stream-like
// "containers".

TEST(ElemensAreStreamTest, WorksForStreamlike) {
  const int a[5] = {1, 2, 3, 4, 5};
  Streamlike<int> s(std::begin(a), std::end(a));
  EXPECT_THAT(s, ElementsAre(1, 2, 3, 4, 5));
  EXPECT_THAT(s, Not(ElementsAre(2, 1, 4, 5, 3)));
}

TEST(ElemensAreArrayStreamTest, WorksForStreamlike) {
  const int a[5] = {1, 2, 3, 4, 5};
  Streamlike<int> s(std::begin(a), std::end(a));

  vector<int> expected;
  expected.push_back(1);
  expected.push_back(2);
  expected.push_back(3);
  expected.push_back(4);
  expected.push_back(5);
  EXPECT_THAT(s, ElementsAreArray(expected));

  expected[3] = 0;
  EXPECT_THAT(s, Not(ElementsAreArray(expected)));
}

TEST(ElementsAreTest, WorksWithUncopyable) {
  Uncopyable objs[2];
  objs[0].set_value(-3);
  objs[1].set_value(1);
  EXPECT_THAT(objs, ElementsAre(UncopyableIs(-3), Truly(ValueIsPositive)));
}

TEST(ElementsAreTest, WorksWithMoveOnly) {
  ContainerHelper helper;
  EXPECT_CALL(helper, Call(ElementsAre(Pointee(1), Pointee(2))));
  helper.Call(MakeUniquePtrs({1, 2}));

  EXPECT_CALL(helper, Call(ElementsAreArray({Pointee(3), Pointee(4)})));
  helper.Call(MakeUniquePtrs({3, 4}));
}

TEST(ElementsAreTest, TakesStlContainer) {
  const int actual[] = {3, 1, 2};

  ::std::list<int> expected;
  expected.push_back(3);
  expected.push_back(1);
  expected.push_back(2);
  EXPECT_THAT(actual, ElementsAreArray(expected));

  expected.push_back(4);
  EXPECT_THAT(actual, Not(ElementsAreArray(expected)));
}

// Tests for UnorderedElementsAreArray()

TEST(UnorderedElementsAreArrayTest, SucceedsWhenExpected) {
  const int a[] = {0, 1, 2, 3, 4};
  std::vector<int> s(std::begin(a), std::end(a));
  do {
    StringMatchResultListener listener;
    EXPECT_TRUE(ExplainMatchResult(UnorderedElementsAreArray(a),
                                   s, &listener)) << listener.str();
  } while (std::next_permutation(s.begin(), s.end()));
}

TEST(UnorderedElementsAreArrayTest, VectorBool) {
  const bool a[] = {0, 1, 0, 1, 1};
  const bool b[] = {1, 0, 1, 1, 0};
  std::vector<bool> expected(std::begin(a), std::end(a));
  std::vector<bool> actual(std::begin(b), std::end(b));
  StringMatchResultListener listener;
  EXPECT_TRUE(ExplainMatchResult(UnorderedElementsAreArray(expected),
                                 actual, &listener)) << listener.str();
}

TEST(UnorderedElementsAreArrayTest, WorksForStreamlike) {
  // Streamlike 'container' provides only minimal iterator support.
  // Its iterators are tagged with input_iterator_tag, and it has no
  // size() or empty() methods.
  const int a[5] = {2, 1, 4, 5, 3};
  Streamlike<int> s(std::begin(a), std::end(a));

  ::std::vector<int> expected;
  expected.push_back(1);
  expected.push_back(2);
  expected.push_back(3);
  expected.push_back(4);
  expected.push_back(5);
  EXPECT_THAT(s, UnorderedElementsAreArray(expected));

  expected.push_back(6);
  EXPECT_THAT(s, Not(UnorderedElementsAreArray(expected)));
}

TEST(UnorderedElementsAreArrayTest, TakesStlContainer) {
  const int actual[] = {3, 1, 2};

  ::std::list<int> expected;
  expected.push_back(1);
  expected.push_back(2);
  expected.push_back(3);
  EXPECT_THAT(actual, UnorderedElementsAreArray(expected));

  expected.push_back(4);
  EXPECT_THAT(actual, Not(UnorderedElementsAreArray(expected)));
}


TEST(UnorderedElementsAreArrayTest, TakesInitializerList) {
  const int a[5] = {2, 1, 4, 5, 3};
  EXPECT_THAT(a, UnorderedElementsAreArray({1, 2, 3, 4, 5}));
  EXPECT_THAT(a, Not(UnorderedElementsAreArray({1, 2, 3, 4, 6})));
}

TEST(UnorderedElementsAreArrayTest, TakesInitializerListOfCStrings) {
  const std::string a[5] = {"a", "b", "c", "d", "e"};
  EXPECT_THAT(a, UnorderedElementsAreArray({"a", "b", "c", "d", "e"}));
  EXPECT_THAT(a, Not(UnorderedElementsAreArray({"a", "b", "c", "d", "ef"})));
}

TEST(UnorderedElementsAreArrayTest, TakesInitializerListOfSameTypedMatchers) {
  const int a[5] = {2, 1, 4, 5, 3};
  EXPECT_THAT(a, UnorderedElementsAreArray(
      {Eq(1), Eq(2), Eq(3), Eq(4), Eq(5)}));
  EXPECT_THAT(a, Not(UnorderedElementsAreArray(
      {Eq(1), Eq(2), Eq(3), Eq(4), Eq(6)})));
}

TEST(UnorderedElementsAreArrayTest,
     TakesInitializerListOfDifferentTypedMatchers) {
  const int a[5] = {2, 1, 4, 5, 3};
  // The compiler cannot infer the type of the initializer list if its
  // elements have different types.  We must explicitly specify the
  // unified element type in this case.
  EXPECT_THAT(a, UnorderedElementsAreArray<Matcher<int> >(
      {Eq(1), Ne(-2), Ge(3), Le(4), Eq(5)}));
  EXPECT_THAT(a, Not(UnorderedElementsAreArray<Matcher<int> >(
      {Eq(1), Ne(-2), Ge(3), Le(4), Eq(6)})));
}


TEST(UnorderedElementsAreArrayTest, WorksWithMoveOnly) {
  ContainerHelper helper;
  EXPECT_CALL(helper,
              Call(UnorderedElementsAreArray({Pointee(1), Pointee(2)})));
  helper.Call(MakeUniquePtrs({2, 1}));
}

class UnorderedElementsAreTest : public testing::Test {
 protected:
  typedef std::vector<int> IntVec;
};

TEST_F(UnorderedElementsAreTest, WorksWithUncopyable) {
  Uncopyable objs[2];
  objs[0].set_value(-3);
  objs[1].set_value(1);
  EXPECT_THAT(objs,
              UnorderedElementsAre(Truly(ValueIsPositive), UncopyableIs(-3)));
}

TEST_F(UnorderedElementsAreTest, SucceedsWhenExpected) {
  const int a[] = {1, 2, 3};
  std::vector<int> s(std::begin(a), std::end(a));
  do {
    StringMatchResultListener listener;
    EXPECT_TRUE(ExplainMatchResult(UnorderedElementsAre(1, 2, 3),
                                   s, &listener)) << listener.str();
  } while (std::next_permutation(s.begin(), s.end()));
}

TEST_F(UnorderedElementsAreTest, FailsWhenAnElementMatchesNoMatcher) {
  const int a[] = {1, 2, 3};
  std::vector<int> s(std::begin(a), std::end(a));
  std::vector<Matcher<int> > mv;
  mv.push_back(1);
  mv.push_back(2);
  mv.push_back(2);
  // The element with value '3' matches nothing: fail fast.
  StringMatchResultListener listener;
  EXPECT_FALSE(ExplainMatchResult(UnorderedElementsAreArray(mv),
                                  s, &listener)) << listener.str();
}

TEST_F(UnorderedElementsAreTest, WorksForStreamlike) {
  // Streamlike 'container' provides only minimal iterator support.
  // Its iterators are tagged with input_iterator_tag, and it has no
  // size() or empty() methods.
  const int a[5] = {2, 1, 4, 5, 3};
  Streamlike<int> s(std::begin(a), std::end(a));

  EXPECT_THAT(s, UnorderedElementsAre(1, 2, 3, 4, 5));
  EXPECT_THAT(s, Not(UnorderedElementsAre(2, 2, 3, 4, 5)));
}

TEST_F(UnorderedElementsAreTest, WorksWithMoveOnly) {
  ContainerHelper helper;
  EXPECT_CALL(helper, Call(UnorderedElementsAre(Pointee(1), Pointee(2))));
  helper.Call(MakeUniquePtrs({2, 1}));
}

// One naive implementation of the matcher runs in O(N!) time, which is too
// slow for many real-world inputs. This test shows that our matcher can match
// 100 inputs very quickly (a few milliseconds).  An O(100!) is 10^158
// iterations and obviously effectively incomputable.
// [ RUN      ] UnorderedElementsAreTest.Performance
// [       OK ] UnorderedElementsAreTest.Performance (4 ms)
TEST_F(UnorderedElementsAreTest, Performance) {
  std::vector<int> s;
  std::vector<Matcher<int> > mv;
  for (int i = 0; i < 100; ++i) {
    s.push_back(i);
    mv.push_back(_);
  }
  mv[50] = Eq(0);
  StringMatchResultListener listener;
  EXPECT_TRUE(ExplainMatchResult(UnorderedElementsAreArray(mv),
                                 s, &listener)) << listener.str();
}

// Another variant of 'Performance' with similar expectations.
// [ RUN      ] UnorderedElementsAreTest.PerformanceHalfStrict
// [       OK ] UnorderedElementsAreTest.PerformanceHalfStrict (4 ms)
TEST_F(UnorderedElementsAreTest, PerformanceHalfStrict) {
  std::vector<int> s;
  std::vector<Matcher<int> > mv;
  for (int i = 0; i < 100; ++i) {
    s.push_back(i);
    if (i & 1) {
      mv.push_back(_);
    } else {
      mv.push_back(i);
    }
  }
  StringMatchResultListener listener;
  EXPECT_TRUE(ExplainMatchResult(UnorderedElementsAreArray(mv),
                                 s, &listener)) << listener.str();
}

TEST_F(UnorderedElementsAreTest, FailMessageCountWrong) {
  std::vector<int> v;
  v.push_back(4);
  StringMatchResultListener listener;
  EXPECT_FALSE(ExplainMatchResult(UnorderedElementsAre(1, 2, 3),
                                  v, &listener)) << listener.str();
  EXPECT_THAT(listener.str(), Eq("which has 1 element"));
}

TEST_F(UnorderedElementsAreTest, FailMessageCountWrongZero) {
  std::vector<int> v;
  StringMatchResultListener listener;
  EXPECT_FALSE(ExplainMatchResult(UnorderedElementsAre(1, 2, 3),
                                  v, &listener)) << listener.str();
  EXPECT_THAT(listener.str(), Eq(""));
}

TEST_F(UnorderedElementsAreTest, FailMessageUnmatchedMatchers) {
  std::vector<int> v;
  v.push_back(1);
  v.push_back(1);
  StringMatchResultListener listener;
  EXPECT_FALSE(ExplainMatchResult(UnorderedElementsAre(1, 2),
                                  v, &listener)) << listener.str();
  EXPECT_THAT(
      listener.str(),
      Eq("where the following matchers don't match any elements:\n"
         "matcher #1: is equal to 2"));
}

TEST_F(UnorderedElementsAreTest, FailMessageUnmatchedElements) {
  std::vector<int> v;
  v.push_back(1);
  v.push_back(2);
  StringMatchResultListener listener;
  EXPECT_FALSE(ExplainMatchResult(UnorderedElementsAre(1, 1),
                                  v, &listener)) << listener.str();
  EXPECT_THAT(
      listener.str(),
      Eq("where the following elements don't match any matchers:\n"
         "element #1: 2"));
}

TEST_F(UnorderedElementsAreTest, FailMessageUnmatchedMatcherAndElement) {
  std::vector<int> v;
  v.push_back(2);
  v.push_back(3);
  StringMatchResultListener listener;
  EXPECT_FALSE(ExplainMatchResult(UnorderedElementsAre(1, 2),
                                  v, &listener)) << listener.str();
  EXPECT_THAT(
      listener.str(),
      Eq("where"
         " the following matchers don't match any elements:\n"
         "matcher #0: is equal to 1\n"
         "and"
         " where"
         " the following elements don't match any matchers:\n"
         "element #1: 3"));
}

// Test helper for formatting element, matcher index pairs in expectations.
static std::string EMString(int element, int matcher) {
  stringstream ss;
  ss << "(element #" << element << ", matcher #" << matcher << ")";
  return ss.str();
}

TEST_F(UnorderedElementsAreTest, FailMessageImperfectMatchOnly) {
  // A situation where all elements and matchers have a match
  // associated with them, but the max matching is not perfect.
  std::vector<std::string> v;
  v.push_back("a");
  v.push_back("b");
  v.push_back("c");
  StringMatchResultListener listener;
  EXPECT_FALSE(ExplainMatchResult(
      UnorderedElementsAre("a", "a", AnyOf("b", "c")), v, &listener))
      << listener.str();

  std::string prefix =
      "where no permutation of the elements can satisfy all matchers, "
      "and the closest match is 2 of 3 matchers with the "
      "pairings:\n";

  // We have to be a bit loose here, because there are 4 valid max matches.
  EXPECT_THAT(
      listener.str(),
      AnyOf(prefix + "{\n  " + EMString(0, 0) +
                     ",\n  " + EMString(1, 2) + "\n}",
            prefix + "{\n  " + EMString(0, 1) +
                     ",\n  " + EMString(1, 2) + "\n}",
            prefix + "{\n  " + EMString(0, 0) +
                     ",\n  " + EMString(2, 2) + "\n}",
            prefix + "{\n  " + EMString(0, 1) +
                     ",\n  " + EMString(2, 2) + "\n}"));
}

TEST_F(UnorderedElementsAreTest, Describe) {
  EXPECT_THAT(Describe<IntVec>(UnorderedElementsAre()),
              Eq("is empty"));
  EXPECT_THAT(
      Describe<IntVec>(UnorderedElementsAre(345)),
      Eq("has 1 element and that element is equal to 345"));
  EXPECT_THAT(
      Describe<IntVec>(UnorderedElementsAre(111, 222, 333)),
      Eq("has 3 elements and there exists some permutation "
         "of elements such that:\n"
         " - element #0 is equal to 111, and\n"
         " - element #1 is equal to 222, and\n"
         " - element #2 is equal to 333"));
}

TEST_F(UnorderedElementsAreTest, DescribeNegation) {
  EXPECT_THAT(DescribeNegation<IntVec>(UnorderedElementsAre()),
              Eq("isn't empty"));
  EXPECT_THAT(
      DescribeNegation<IntVec>(UnorderedElementsAre(345)),
      Eq("doesn't have 1 element, or has 1 element that isn't equal to 345"));
  EXPECT_THAT(
      DescribeNegation<IntVec>(UnorderedElementsAre(123, 234, 345)),
      Eq("doesn't have 3 elements, or there exists no permutation "
         "of elements such that:\n"
         " - element #0 is equal to 123, and\n"
         " - element #1 is equal to 234, and\n"
         " - element #2 is equal to 345"));
}

namespace {

// Used as a check on the more complex max flow method used in the
// real testing::internal::FindMaxBipartiteMatching. This method is
// compatible but runs in worst-case factorial time, so we only
// use it in testing for small problem sizes.
template <typename Graph>
class BacktrackingMaxBPMState {
 public:
  // Does not take ownership of 'g'.
  explicit BacktrackingMaxBPMState(const Graph* g) : graph_(g) { }

  ElementMatcherPairs Compute() {
    if (graph_->LhsSize() == 0 || graph_->RhsSize() == 0) {
      return best_so_far_;
    }
    lhs_used_.assign(graph_->LhsSize(), kUnused);
    rhs_used_.assign(graph_->RhsSize(), kUnused);
    for (size_t irhs = 0; irhs < graph_->RhsSize(); ++irhs) {
      matches_.clear();
      RecurseInto(irhs);
      if (best_so_far_.size() == graph_->RhsSize())
        break;
    }
    return best_so_far_;
  }

 private:
  static const size_t kUnused = static_cast<size_t>(-1);

  void PushMatch(size_t lhs, size_t rhs) {
    matches_.push_back(ElementMatcherPair(lhs, rhs));
    lhs_used_[lhs] = rhs;
    rhs_used_[rhs] = lhs;
    if (matches_.size() > best_so_far_.size()) {
      best_so_far_ = matches_;
    }
  }

  void PopMatch() {
    const ElementMatcherPair& back = matches_.back();
    lhs_used_[back.first] = kUnused;
    rhs_used_[back.second] = kUnused;
    matches_.pop_back();
  }

  bool RecurseInto(size_t irhs) {
    if (rhs_used_[irhs] != kUnused) {
      return true;
    }
    for (size_t ilhs = 0; ilhs < graph_->LhsSize(); ++ilhs) {
      if (lhs_used_[ilhs] != kUnused) {
        continue;
      }
      if (!graph_->HasEdge(ilhs, irhs)) {
        continue;
      }
      PushMatch(ilhs, irhs);
      if (best_so_far_.size() == graph_->RhsSize()) {
        return false;
      }
      for (size_t mi = irhs + 1; mi < graph_->RhsSize(); ++mi) {
        if (!RecurseInto(mi)) return false;
      }
      PopMatch();
    }
    return true;
  }

  const Graph* graph_;  // not owned
  std::vector<size_t> lhs_used_;
  std::vector<size_t> rhs_used_;
  ElementMatcherPairs matches_;
  ElementMatcherPairs best_so_far_;
};

template <typename Graph>
const size_t BacktrackingMaxBPMState<Graph>::kUnused;

}  // namespace

// Implement a simple backtracking algorithm to determine if it is possible
// to find one element per matcher, without reusing elements.
template <typename Graph>
ElementMatcherPairs
FindBacktrackingMaxBPM(const Graph& g) {
  return BacktrackingMaxBPMState<Graph>(&g).Compute();
}

class BacktrackingBPMTest : public ::testing::Test { };

// Tests the MaxBipartiteMatching algorithm with square matrices.
// The single int param is the # of nodes on each of the left and right sides.
class BipartiteTest : public ::testing::TestWithParam<size_t> {};

// Verify all match graphs up to some moderate number of edges.
TEST_P(BipartiteTest, Exhaustive) {
  size_t nodes = GetParam();
  MatchMatrix graph(nodes, nodes);
  do {
    ElementMatcherPairs matches =
        internal::FindMaxBipartiteMatching(graph);
    EXPECT_EQ(FindBacktrackingMaxBPM(graph).size(), matches.size())
        << "graph: " << graph.DebugString();
    // Check that all elements of matches are in the graph.
    // Check that elements of first and second are unique.
    std::vector<bool> seen_element(graph.LhsSize());
    std::vector<bool> seen_matcher(graph.RhsSize());
    SCOPED_TRACE(PrintToString(matches));
    for (size_t i = 0; i < matches.size(); ++i) {
      size_t ilhs = matches[i].first;
      size_t irhs = matches[i].second;
      EXPECT_TRUE(graph.HasEdge(ilhs, irhs));
      EXPECT_FALSE(seen_element[ilhs]);
      EXPECT_FALSE(seen_matcher[irhs]);
      seen_element[ilhs] = true;
      seen_matcher[irhs] = true;
    }
  } while (graph.NextGraph());
}

INSTANTIATE_TEST_SUITE_P(AllGraphs, BipartiteTest,
                         ::testing::Range(size_t{0}, size_t{5}));

// Parameterized by a pair interpreted as (LhsSize, RhsSize).
class BipartiteNonSquareTest
    : public ::testing::TestWithParam<std::pair<size_t, size_t> > {
};

TEST_F(BipartiteNonSquareTest, SimpleBacktracking) {
  //   .......
  // 0:-----\ :
  // 1:---\ | :
  // 2:---\ | :
  // 3:-\ | | :
  //  :.......:
  //    0 1 2
  MatchMatrix g(4, 3);
  constexpr std::array<std::array<size_t, 2>, 4> kEdges = {
      {{{0, 2}}, {{1, 1}}, {{2, 1}}, {{3, 0}}}};
  for (size_t i = 0; i < kEdges.size(); ++i) {
    g.SetEdge(kEdges[i][0], kEdges[i][1], true);
  }
  EXPECT_THAT(FindBacktrackingMaxBPM(g),
              ElementsAre(Pair(3, 0),
                          Pair(AnyOf(1, 2), 1),
                          Pair(0, 2))) << g.DebugString();
}

// Verify a few nonsquare matrices.
TEST_P(BipartiteNonSquareTest, Exhaustive) {
  size_t nlhs = GetParam().first;
  size_t nrhs = GetParam().second;
  MatchMatrix graph(nlhs, nrhs);
  do {
    EXPECT_EQ(FindBacktrackingMaxBPM(graph).size(),
              internal::FindMaxBipartiteMatching(graph).size())
        << "graph: " << graph.DebugString()
        << "\nbacktracking: "
        << PrintToString(FindBacktrackingMaxBPM(graph))
        << "\nmax flow: "
        << PrintToString(internal::FindMaxBipartiteMatching(graph));
  } while (graph.NextGraph());
}

INSTANTIATE_TEST_SUITE_P(AllGraphs, BipartiteNonSquareTest,
    testing::Values(
        std::make_pair(1, 2),
        std::make_pair(2, 1),
        std::make_pair(3, 2),
        std::make_pair(2, 3),
        std::make_pair(4, 1),
        std::make_pair(1, 4),
        std::make_pair(4, 3),
        std::make_pair(3, 4)));

class BipartiteRandomTest
    : public ::testing::TestWithParam<std::pair<int, int> > {
};

// Verifies a large sample of larger graphs.
TEST_P(BipartiteRandomTest, LargerNets) {
  int nodes = GetParam().first;
  int iters = GetParam().second;
  MatchMatrix graph(static_cast<size_t>(nodes), static_cast<size_t>(nodes));

  auto seed = static_cast<uint32_t>(GTEST_FLAG(random_seed));
  if (seed == 0) {
    seed = static_cast<uint32_t>(time(nullptr));
  }

  for (; iters > 0; --iters, ++seed) {
    srand(static_cast<unsigned int>(seed));
    graph.Randomize();
    EXPECT_EQ(FindBacktrackingMaxBPM(graph).size(),
              internal::FindMaxBipartiteMatching(graph).size())
        << " graph: " << graph.DebugString()
        << "\nTo reproduce the failure, rerun the test with the flag"
           " --" << GTEST_FLAG_PREFIX_ << "random_seed=" << seed;
  }
}

// Test argument is a std::pair<int, int> representing (nodes, iters).
INSTANTIATE_TEST_SUITE_P(Samples, BipartiteRandomTest,
    testing::Values(
        std::make_pair(5, 10000),
        std::make_pair(6, 5000),
        std::make_pair(7, 2000),
        std::make_pair(8, 500),
        std::make_pair(9, 100)));

// Tests IsReadableTypeName().

TEST(IsReadableTypeNameTest, ReturnsTrueForShortNames) {
  EXPECT_TRUE(IsReadableTypeName("int"));
  EXPECT_TRUE(IsReadableTypeName("const unsigned char*"));
  EXPECT_TRUE(IsReadableTypeName("MyMap<int, void*>"));
  EXPECT_TRUE(IsReadableTypeName("void (*)(int, bool)"));
}

TEST(IsReadableTypeNameTest, ReturnsTrueForLongNonTemplateNonFunctionNames) {
  EXPECT_TRUE(IsReadableTypeName("my_long_namespace::MyClassName"));
  EXPECT_TRUE(IsReadableTypeName("int [5][6][7][8][9][10][11]"));
  EXPECT_TRUE(IsReadableTypeName("my_namespace::MyOuterClass::MyInnerClass"));
}

TEST(IsReadableTypeNameTest, ReturnsFalseForLongTemplateNames) {
  EXPECT_FALSE(
      IsReadableTypeName("basic_string<char, std::char_traits<char> >"));
  EXPECT_FALSE(IsReadableTypeName("std::vector<int, std::alloc_traits<int> >"));
}

TEST(IsReadableTypeNameTest, ReturnsFalseForLongFunctionTypeNames) {
  EXPECT_FALSE(IsReadableTypeName("void (&)(int, bool, char, float)"));
}

// Tests FormatMatcherDescription().

TEST(FormatMatcherDescriptionTest, WorksForEmptyDescription) {
  EXPECT_EQ("is even",
            FormatMatcherDescription(false, "IsEven", Strings()));
  EXPECT_EQ("not (is even)",
            FormatMatcherDescription(true, "IsEven", Strings()));

  const char* params[] = {"5"};
  EXPECT_EQ("equals 5",
            FormatMatcherDescription(false, "Equals",
                                     Strings(params, params + 1)));

  const char* params2[] = {"5", "8"};
  EXPECT_EQ("is in range (5, 8)",
            FormatMatcherDescription(false, "IsInRange",
                                     Strings(params2, params2 + 2)));
}

// Tests PolymorphicMatcher::mutable_impl().
TEST(PolymorphicMatcherTest, CanAccessMutableImpl) {
  PolymorphicMatcher<DivisibleByImpl> m(DivisibleByImpl(42));
  DivisibleByImpl& impl = m.mutable_impl();
  EXPECT_EQ(42, impl.divider());

  impl.set_divider(0);
  EXPECT_EQ(0, m.mutable_impl().divider());
}

// Tests PolymorphicMatcher::impl().
TEST(PolymorphicMatcherTest, CanAccessImpl) {
  const PolymorphicMatcher<DivisibleByImpl> m(DivisibleByImpl(42));
  const DivisibleByImpl& impl = m.impl();
  EXPECT_EQ(42, impl.divider());
}

TEST(MatcherTupleTest, ExplainsMatchFailure) {
  stringstream ss1;
  ExplainMatchFailureTupleTo(
      std::make_tuple(Matcher<char>(Eq('a')), GreaterThan(5)),
      std::make_tuple('a', 10), &ss1);
  EXPECT_EQ("", ss1.str());  // Successful match.

  stringstream ss2;
  ExplainMatchFailureTupleTo(
      std::make_tuple(GreaterThan(5), Matcher<char>(Eq('a'))),
      std::make_tuple(2, 'b'), &ss2);
  EXPECT_EQ("  Expected arg #0: is > 5\n"
            "           Actual: 2, which is 3 less than 5\n"
            "  Expected arg #1: is equal to 'a' (97, 0x61)\n"
            "           Actual: 'b' (98, 0x62)\n",
            ss2.str());  // Failed match where both arguments need explanation.

  stringstream ss3;
  ExplainMatchFailureTupleTo(
      std::make_tuple(GreaterThan(5), Matcher<char>(Eq('a'))),
      std::make_tuple(2, 'a'), &ss3);
  EXPECT_EQ("  Expected arg #0: is > 5\n"
            "           Actual: 2, which is 3 less than 5\n",
            ss3.str());  // Failed match where only one argument needs
                         // explanation.
}

// Tests Each().

TEST(EachTest, ExplainsMatchResultCorrectly) {
  set<int> a;  // empty

  Matcher<set<int> > m = Each(2);
  EXPECT_EQ("", Explain(m, a));

  Matcher<const int(&)[1]> n = Each(1);  // NOLINT

  const int b[1] = {1};
  EXPECT_EQ("", Explain(n, b));

  n = Each(3);
  EXPECT_EQ("whose element #0 doesn't match", Explain(n, b));

  a.insert(1);
  a.insert(2);
  a.insert(3);
  m = Each(GreaterThan(0));
  EXPECT_EQ("", Explain(m, a));

  m = Each(GreaterThan(10));
  EXPECT_EQ("whose element #0 doesn't match, which is 9 less than 10",
            Explain(m, a));
}

TEST(EachTest, DescribesItselfCorrectly) {
  Matcher<vector<int> > m = Each(1);
  EXPECT_EQ("only contains elements that is equal to 1", Describe(m));

  Matcher<vector<int> > m2 = Not(m);
  EXPECT_EQ("contains some element that isn't equal to 1", Describe(m2));
}

TEST(EachTest, MatchesVectorWhenAllElementsMatch) {
  vector<int> some_vector;
  EXPECT_THAT(some_vector, Each(1));
  some_vector.push_back(3);
  EXPECT_THAT(some_vector, Not(Each(1)));
  EXPECT_THAT(some_vector, Each(3));
  some_vector.push_back(1);
  some_vector.push_back(2);
  EXPECT_THAT(some_vector, Not(Each(3)));
  EXPECT_THAT(some_vector, Each(Lt(3.5)));

  vector<std::string> another_vector;
  another_vector.push_back("fee");
  EXPECT_THAT(another_vector, Each(std::string("fee")));
  another_vector.push_back("fie");
  another_vector.push_back("foe");
  another_vector.push_back("fum");
  EXPECT_THAT(another_vector, Not(Each(std::string("fee"))));
}

TEST(EachTest, MatchesMapWhenAllElementsMatch) {
  map<const char*, int> my_map;
  const char* bar = "a string";
  my_map[bar] = 2;
  EXPECT_THAT(my_map, Each(make_pair(bar, 2)));

  map<std::string, int> another_map;
  EXPECT_THAT(another_map, Each(make_pair(std::string("fee"), 1)));
  another_map["fee"] = 1;
  EXPECT_THAT(another_map, Each(make_pair(std::string("fee"), 1)));
  another_map["fie"] = 2;
  another_map["foe"] = 3;
  another_map["fum"] = 4;
  EXPECT_THAT(another_map, Not(Each(make_pair(std::string("fee"), 1))));
  EXPECT_THAT(another_map, Not(Each(make_pair(std::string("fum"), 1))));
  EXPECT_THAT(another_map, Each(Pair(_, Gt(0))));
}

TEST(EachTest, AcceptsMatcher) {
  const int a[] = {1, 2, 3};
  EXPECT_THAT(a, Each(Gt(0)));
  EXPECT_THAT(a, Not(Each(Gt(1))));
}

TEST(EachTest, WorksForNativeArrayAsTuple) {
  const int a[] = {1, 2};
  const int* const pointer = a;
  EXPECT_THAT(std::make_tuple(pointer, 2), Each(Gt(0)));
  EXPECT_THAT(std::make_tuple(pointer, 2), Not(Each(Gt(1))));
}

TEST(EachTest, WorksWithMoveOnly) {
  ContainerHelper helper;
  EXPECT_CALL(helper, Call(Each(Pointee(Gt(0)))));
  helper.Call(MakeUniquePtrs({1, 2}));
}

// For testing Pointwise().
class IsHalfOfMatcher {
 public:
  template <typename T1, typename T2>
  bool MatchAndExplain(const std::tuple<T1, T2>& a_pair,
                       MatchResultListener* listener) const {
    if (std::get<0>(a_pair) == std::get<1>(a_pair) / 2) {
      *listener << "where the second is " << std::get<1>(a_pair);
      return true;
    } else {
      *listener << "where the second/2 is " << std::get<1>(a_pair) / 2;
      return false;
    }
  }

  void DescribeTo(ostream* os) const {
    *os << "are a pair where the first is half of the second";
  }

  void DescribeNegationTo(ostream* os) const {
    *os << "are a pair where the first isn't half of the second";
  }
};

PolymorphicMatcher<IsHalfOfMatcher> IsHalfOf() {
  return MakePolymorphicMatcher(IsHalfOfMatcher());
}

TEST(PointwiseTest, DescribesSelf) {
  vector<int> rhs;
  rhs.push_back(1);
  rhs.push_back(2);
  rhs.push_back(3);
  const Matcher<const vector<int>&> m = Pointwise(IsHalfOf(), rhs);
  EXPECT_EQ("contains 3 values, where each value and its corresponding value "
            "in { 1, 2, 3 } are a pair where the first is half of the second",
            Describe(m));
  EXPECT_EQ("doesn't contain exactly 3 values, or contains a value x at some "
            "index i where x and the i-th value of { 1, 2, 3 } are a pair "
            "where the first isn't half of the second",
            DescribeNegation(m));
}

TEST(PointwiseTest, MakesCopyOfRhs) {
  list<signed char> rhs;
  rhs.push_back(2);
  rhs.push_back(4);

  int lhs[] = {1, 2};
  const Matcher<const int (&)[2]> m = Pointwise(IsHalfOf(), rhs);
  EXPECT_THAT(lhs, m);

  // Changing rhs now shouldn't affect m, which made a copy of rhs.
  rhs.push_back(6);
  EXPECT_THAT(lhs, m);
}

TEST(PointwiseTest, WorksForLhsNativeArray) {
  const int lhs[] = {1, 2, 3};
  vector<int> rhs;
  rhs.push_back(2);
  rhs.push_back(4);
  rhs.push_back(6);
  EXPECT_THAT(lhs, Pointwise(Lt(), rhs));
  EXPECT_THAT(lhs, Not(Pointwise(Gt(), rhs)));
}

TEST(PointwiseTest, WorksForRhsNativeArray) {
  const int rhs[] = {1, 2, 3};
  vector<int> lhs;
  lhs.push_back(2);
  lhs.push_back(4);
  lhs.push_back(6);
  EXPECT_THAT(lhs, Pointwise(Gt(), rhs));
  EXPECT_THAT(lhs, Not(Pointwise(Lt(), rhs)));
}

// Test is effective only with sanitizers.
TEST(PointwiseTest, WorksForVectorOfBool) {
  vector<bool> rhs(3, false);
  rhs[1] = true;
  vector<bool> lhs = rhs;
  EXPECT_THAT(lhs, Pointwise(Eq(), rhs));
  rhs[0] = true;
  EXPECT_THAT(lhs, Not(Pointwise(Eq(), rhs)));
}


TEST(PointwiseTest, WorksForRhsInitializerList) {
  const vector<int> lhs{2, 4, 6};
  EXPECT_THAT(lhs, Pointwise(Gt(), {1, 2, 3}));
  EXPECT_THAT(lhs, Not(Pointwise(Lt(), {3, 3, 7})));
}


TEST(PointwiseTest, RejectsWrongSize) {
  const double lhs[2] = {1, 2};
  const int rhs[1] = {0};
  EXPECT_THAT(lhs, Not(Pointwise(Gt(), rhs)));
  EXPECT_EQ("which contains 2 values",
            Explain(Pointwise(Gt(), rhs), lhs));

  const int rhs2[3] = {0, 1, 2};
  EXPECT_THAT(lhs, Not(Pointwise(Gt(), rhs2)));
}

TEST(PointwiseTest, RejectsWrongContent) {
  const double lhs[3] = {1, 2, 3};
  const int rhs[3] = {2, 6, 4};
  EXPECT_THAT(lhs, Not(Pointwise(IsHalfOf(), rhs)));
  EXPECT_EQ("where the value pair (2, 6) at index #1 don't match, "
            "where the second/2 is 3",
            Explain(Pointwise(IsHalfOf(), rhs), lhs));
}

TEST(PointwiseTest, AcceptsCorrectContent) {
  const double lhs[3] = {1, 2, 3};
  const int rhs[3] = {2, 4, 6};
  EXPECT_THAT(lhs, Pointwise(IsHalfOf(), rhs));
  EXPECT_EQ("", Explain(Pointwise(IsHalfOf(), rhs), lhs));
}

TEST(PointwiseTest, AllowsMonomorphicInnerMatcher) {
  const double lhs[3] = {1, 2, 3};
  const int rhs[3] = {2, 4, 6};
  const Matcher<std::tuple<const double&, const int&>> m1 = IsHalfOf();
  EXPECT_THAT(lhs, Pointwise(m1, rhs));
  EXPECT_EQ("", Explain(Pointwise(m1, rhs), lhs));

  // This type works as a std::tuple<const double&, const int&> can be
  // implicitly cast to std::tuple<double, int>.
  const Matcher<std::tuple<double, int>> m2 = IsHalfOf();
  EXPECT_THAT(lhs, Pointwise(m2, rhs));
  EXPECT_EQ("", Explain(Pointwise(m2, rhs), lhs));
}

MATCHER(PointeeEquals, "Points to an equal value") {
  return ExplainMatchResult(::testing::Pointee(::testing::get<1>(arg)),
                            ::testing::get<0>(arg), result_listener);
}

TEST(PointwiseTest, WorksWithMoveOnly) {
  ContainerHelper helper;
  EXPECT_CALL(helper, Call(Pointwise(PointeeEquals(), std::vector<int>{1, 2})));
  helper.Call(MakeUniquePtrs({1, 2}));
}

TEST(UnorderedPointwiseTest, DescribesSelf) {
  vector<int> rhs;
  rhs.push_back(1);
  rhs.push_back(2);
  rhs.push_back(3);
  const Matcher<const vector<int>&> m = UnorderedPointwise(IsHalfOf(), rhs);
  EXPECT_EQ(
      "has 3 elements and there exists some permutation of elements such "
      "that:\n"
      " - element #0 and 1 are a pair where the first is half of the second, "
      "and\n"
      " - element #1 and 2 are a pair where the first is half of the second, "
      "and\n"
      " - element #2 and 3 are a pair where the first is half of the second",
      Describe(m));
  EXPECT_EQ(
      "doesn't have 3 elements, or there exists no permutation of elements "
      "such that:\n"
      " - element #0 and 1 are a pair where the first is half of the second, "
      "and\n"
      " - element #1 and 2 are a pair where the first is half of the second, "
      "and\n"
      " - element #2 and 3 are a pair where the first is half of the second",
      DescribeNegation(m));
}

TEST(UnorderedPointwiseTest, MakesCopyOfRhs) {
  list<signed char> rhs;
  rhs.push_back(2);
  rhs.push_back(4);

  int lhs[] = {2, 1};
  const Matcher<const int (&)[2]> m = UnorderedPointwise(IsHalfOf(), rhs);
  EXPECT_THAT(lhs, m);

  // Changing rhs now shouldn't affect m, which made a copy of rhs.
  rhs.push_back(6);
  EXPECT_THAT(lhs, m);
}

TEST(UnorderedPointwiseTest, WorksForLhsNativeArray) {
  const int lhs[] = {1, 2, 3};
  vector<int> rhs;
  rhs.push_back(4);
  rhs.push_back(6);
  rhs.push_back(2);
  EXPECT_THAT(lhs, UnorderedPointwise(Lt(), rhs));
  EXPECT_THAT(lhs, Not(UnorderedPointwise(Gt(), rhs)));
}

TEST(UnorderedPointwiseTest, WorksForRhsNativeArray) {
  const int rhs[] = {1, 2, 3};
  vector<int> lhs;
  lhs.push_back(4);
  lhs.push_back(2);
  lhs.push_back(6);
  EXPECT_THAT(lhs, UnorderedPointwise(Gt(), rhs));
  EXPECT_THAT(lhs, Not(UnorderedPointwise(Lt(), rhs)));
}


TEST(UnorderedPointwiseTest, WorksForRhsInitializerList) {
  const vector<int> lhs{2, 4, 6};
  EXPECT_THAT(lhs, UnorderedPointwise(Gt(), {5, 1, 3}));
  EXPECT_THAT(lhs, Not(UnorderedPointwise(Lt(), {1, 1, 7})));
}


TEST(UnorderedPointwiseTest, RejectsWrongSize) {
  const double lhs[2] = {1, 2};
  const int rhs[1] = {0};
  EXPECT_THAT(lhs, Not(UnorderedPointwise(Gt(), rhs)));
  EXPECT_EQ("which has 2 elements",
            Explain(UnorderedPointwise(Gt(), rhs), lhs));

  const int rhs2[3] = {0, 1, 2};
  EXPECT_THAT(lhs, Not(UnorderedPointwise(Gt(), rhs2)));
}

TEST(UnorderedPointwiseTest, RejectsWrongContent) {
  const double lhs[3] = {1, 2, 3};
  const int rhs[3] = {2, 6, 6};
  EXPECT_THAT(lhs, Not(UnorderedPointwise(IsHalfOf(), rhs)));
  EXPECT_EQ("where the following elements don't match any matchers:\n"
            "element #1: 2",
            Explain(UnorderedPointwise(IsHalfOf(), rhs), lhs));
}

TEST(UnorderedPointwiseTest, AcceptsCorrectContentInSameOrder) {
  const double lhs[3] = {1, 2, 3};
  const int rhs[3] = {2, 4, 6};
  EXPECT_THAT(lhs, UnorderedPointwise(IsHalfOf(), rhs));
}

TEST(UnorderedPointwiseTest, AcceptsCorrectContentInDifferentOrder) {
  const double lhs[3] = {1, 2, 3};
  const int rhs[3] = {6, 4, 2};
  EXPECT_THAT(lhs, UnorderedPointwise(IsHalfOf(), rhs));
}

TEST(UnorderedPointwiseTest, AllowsMonomorphicInnerMatcher) {
  const double lhs[3] = {1, 2, 3};
  const int rhs[3] = {4, 6, 2};
  const Matcher<std::tuple<const double&, const int&>> m1 = IsHalfOf();
  EXPECT_THAT(lhs, UnorderedPointwise(m1, rhs));

  // This type works as a std::tuple<const double&, const int&> can be
  // implicitly cast to std::tuple<double, int>.
  const Matcher<std::tuple<double, int>> m2 = IsHalfOf();
  EXPECT_THAT(lhs, UnorderedPointwise(m2, rhs));
}

TEST(UnorderedPointwiseTest, WorksWithMoveOnly) {
  ContainerHelper helper;
  EXPECT_CALL(helper, Call(UnorderedPointwise(PointeeEquals(),
                                              std::vector<int>{1, 2})));
  helper.Call(MakeUniquePtrs({2, 1}));
}

// Sample optional type implementation with minimal requirements for use with
// Optional matcher.
template <typename T>
class SampleOptional {
 public:
  using value_type = T;
  explicit SampleOptional(T value)
      : value_(std::move(value)), has_value_(true) {}
  SampleOptional() : value_(), has_value_(false) {}
  operator bool() const { return has_value_; }
  const T& operator*() const { return value_; }

 private:
  T value_;
  bool has_value_;
};

TEST(OptionalTest, DescribesSelf) {
  const Matcher<SampleOptional<int>> m = Optional(Eq(1));
  EXPECT_EQ("value is equal to 1", Describe(m));
}

TEST(OptionalTest, ExplainsSelf) {
  const Matcher<SampleOptional<int>> m = Optional(Eq(1));
  EXPECT_EQ("whose value 1 matches", Explain(m, SampleOptional<int>(1)));
  EXPECT_EQ("whose value 2 doesn't match", Explain(m, SampleOptional<int>(2)));
}

TEST(OptionalTest, MatchesNonEmptyOptional) {
  const Matcher<SampleOptional<int>> m1 = Optional(1);
  const Matcher<SampleOptional<int>> m2 = Optional(Eq(2));
  const Matcher<SampleOptional<int>> m3 = Optional(Lt(3));
  SampleOptional<int> opt(1);
  EXPECT_TRUE(m1.Matches(opt));
  EXPECT_FALSE(m2.Matches(opt));
  EXPECT_TRUE(m3.Matches(opt));
}

TEST(OptionalTest, DoesNotMatchNullopt) {
  const Matcher<SampleOptional<int>> m = Optional(1);
  SampleOptional<int> empty;
  EXPECT_FALSE(m.Matches(empty));
}

TEST(OptionalTest, WorksWithMoveOnly) {
  Matcher<SampleOptional<std::unique_ptr<int>>> m = Optional(Eq(nullptr));
  EXPECT_TRUE(m.Matches(SampleOptional<std::unique_ptr<int>>(nullptr)));
}

class SampleVariantIntString {
 public:
  SampleVariantIntString(int i) : i_(i), has_int_(true) {}
  SampleVariantIntString(const std::string& s) : s_(s), has_int_(false) {}

  template <typename T>
  friend bool holds_alternative(const SampleVariantIntString& value) {
    return value.has_int_ == std::is_same<T, int>::value;
  }

  template <typename T>
  friend const T& get(const SampleVariantIntString& value) {
    return value.get_impl(static_cast<T*>(nullptr));
  }

 private:
  const int& get_impl(int*) const { return i_; }
  const std::string& get_impl(std::string*) const { return s_; }

  int i_;
  std::string s_;
  bool has_int_;
};

TEST(VariantTest, DescribesSelf) {
  const Matcher<SampleVariantIntString> m = VariantWith<int>(Eq(1));
  EXPECT_THAT(Describe(m), ContainsRegex("is a variant<> with value of type "
                                         "'.*' and the value is equal to 1"));
}

TEST(VariantTest, ExplainsSelf) {
  const Matcher<SampleVariantIntString> m = VariantWith<int>(Eq(1));
  EXPECT_THAT(Explain(m, SampleVariantIntString(1)),
              ContainsRegex("whose value 1"));
  EXPECT_THAT(Explain(m, SampleVariantIntString("A")),
              HasSubstr("whose value is not of type '"));
  EXPECT_THAT(Explain(m, SampleVariantIntString(2)),
              "whose value 2 doesn't match");
}

TEST(VariantTest, FullMatch) {
  Matcher<SampleVariantIntString> m = VariantWith<int>(Eq(1));
  EXPECT_TRUE(m.Matches(SampleVariantIntString(1)));

  m = VariantWith<std::string>(Eq("1"));
  EXPECT_TRUE(m.Matches(SampleVariantIntString("1")));
}

TEST(VariantTest, TypeDoesNotMatch) {
  Matcher<SampleVariantIntString> m = VariantWith<int>(Eq(1));
  EXPECT_FALSE(m.Matches(SampleVariantIntString("1")));

  m = VariantWith<std::string>(Eq("1"));
  EXPECT_FALSE(m.Matches(SampleVariantIntString(1)));
}

TEST(VariantTest, InnerDoesNotMatch) {
  Matcher<SampleVariantIntString> m = VariantWith<int>(Eq(1));
  EXPECT_FALSE(m.Matches(SampleVariantIntString(2)));

  m = VariantWith<std::string>(Eq("1"));
  EXPECT_FALSE(m.Matches(SampleVariantIntString("2")));
}

class SampleAnyType {
 public:
  explicit SampleAnyType(int i) : index_(0), i_(i) {}
  explicit SampleAnyType(const std::string& s) : index_(1), s_(s) {}

  template <typename T>
  friend const T* any_cast(const SampleAnyType* any) {
    return any->get_impl(static_cast<T*>(nullptr));
  }

 private:
  int index_;
  int i_;
  std::string s_;

  const int* get_impl(int*) const { return index_ == 0 ? &i_ : nullptr; }
  const std::string* get_impl(std::string*) const {
    return index_ == 1 ? &s_ : nullptr;
  }
};

TEST(AnyWithTest, FullMatch) {
  Matcher<SampleAnyType> m = AnyWith<int>(Eq(1));
  EXPECT_TRUE(m.Matches(SampleAnyType(1)));
}

TEST(AnyWithTest, TestBadCastType) {
  Matcher<SampleAnyType> m = AnyWith<std::string>(Eq("fail"));
  EXPECT_FALSE(m.Matches(SampleAnyType(1)));
}

TEST(AnyWithTest, TestUseInContainers) {
  std::vector<SampleAnyType> a;
  a.emplace_back(1);
  a.emplace_back(2);
  a.emplace_back(3);
  EXPECT_THAT(
      a, ElementsAreArray({AnyWith<int>(1), AnyWith<int>(2), AnyWith<int>(3)}));

  std::vector<SampleAnyType> b;
  b.emplace_back("hello");
  b.emplace_back("merhaba");
  b.emplace_back("salut");
  EXPECT_THAT(b, ElementsAreArray({AnyWith<std::string>("hello"),
                                   AnyWith<std::string>("merhaba"),
                                   AnyWith<std::string>("salut")}));
}
TEST(AnyWithTest, TestCompare) {
  EXPECT_THAT(SampleAnyType(1), AnyWith<int>(Gt(0)));
}

TEST(AnyWithTest, DescribesSelf) {
  const Matcher<const SampleAnyType&> m = AnyWith<int>(Eq(1));
  EXPECT_THAT(Describe(m), ContainsRegex("is an 'any' type with value of type "
                                         "'.*' and the value is equal to 1"));
}

TEST(AnyWithTest, ExplainsSelf) {
  const Matcher<const SampleAnyType&> m = AnyWith<int>(Eq(1));

  EXPECT_THAT(Explain(m, SampleAnyType(1)), ContainsRegex("whose value 1"));
  EXPECT_THAT(Explain(m, SampleAnyType("A")),
              HasSubstr("whose value is not of type '"));
  EXPECT_THAT(Explain(m, SampleAnyType(2)), "whose value 2 doesn't match");
}

TEST(PointeeTest, WorksOnMoveOnlyType) {
  std::unique_ptr<int> p(new int(3));
  EXPECT_THAT(p, Pointee(Eq(3)));
  EXPECT_THAT(p, Not(Pointee(Eq(2))));
}

TEST(NotTest, WorksOnMoveOnlyType) {
  std::unique_ptr<int> p(new int(3));
  EXPECT_THAT(p, Pointee(Eq(3)));
  EXPECT_THAT(p, Not(Pointee(Eq(2))));
}

// Tests Args<k0, ..., kn>(m).

TEST(ArgsTest, AcceptsZeroTemplateArg) {
  const std::tuple<int, bool> t(5, true);
  EXPECT_THAT(t, Args<>(Eq(std::tuple<>())));
  EXPECT_THAT(t, Not(Args<>(Ne(std::tuple<>()))));
}

TEST(ArgsTest, AcceptsOneTemplateArg) {
  const std::tuple<int, bool> t(5, true);
  EXPECT_THAT(t, Args<0>(Eq(std::make_tuple(5))));
  EXPECT_THAT(t, Args<1>(Eq(std::make_tuple(true))));
  EXPECT_THAT(t, Not(Args<1>(Eq(std::make_tuple(false)))));
}

TEST(ArgsTest, AcceptsTwoTemplateArgs) {
  const std::tuple<short, int, long> t(4, 5, 6L);  // NOLINT

  EXPECT_THAT(t, (Args<0, 1>(Lt())));
  EXPECT_THAT(t, (Args<1, 2>(Lt())));
  EXPECT_THAT(t, Not(Args<0, 2>(Gt())));
}

TEST(ArgsTest, AcceptsRepeatedTemplateArgs) {
  const std::tuple<short, int, long> t(4, 5, 6L);  // NOLINT
  EXPECT_THAT(t, (Args<0, 0>(Eq())));
  EXPECT_THAT(t, Not(Args<1, 1>(Ne())));
}

TEST(ArgsTest, AcceptsDecreasingTemplateArgs) {
  const std::tuple<short, int, long> t(4, 5, 6L);  // NOLINT
  EXPECT_THAT(t, (Args<2, 0>(Gt())));
  EXPECT_THAT(t, Not(Args<2, 1>(Lt())));
}

MATCHER(SumIsZero, "") {
  return std::get<0>(arg) + std::get<1>(arg) + std::get<2>(arg) == 0;
}

TEST(ArgsTest, AcceptsMoreTemplateArgsThanArityOfOriginalTuple) {
  EXPECT_THAT(std::make_tuple(-1, 2), (Args<0, 0, 1>(SumIsZero())));
  EXPECT_THAT(std::make_tuple(1, 2), Not(Args<0, 0, 1>(SumIsZero())));
}

TEST(ArgsTest, CanBeNested) {
  const std::tuple<short, int, long, int> t(4, 5, 6L, 6);  // NOLINT
  EXPECT_THAT(t, (Args<1, 2, 3>(Args<1, 2>(Eq()))));
  EXPECT_THAT(t, (Args<0, 1, 3>(Args<0, 2>(Lt()))));
}

TEST(ArgsTest, CanMatchTupleByValue) {
  typedef std::tuple<char, int, int> Tuple3;
  const Matcher<Tuple3> m = Args<1, 2>(Lt());
  EXPECT_TRUE(m.Matches(Tuple3('a', 1, 2)));
  EXPECT_FALSE(m.Matches(Tuple3('b', 2, 2)));
}

TEST(ArgsTest, CanMatchTupleByReference) {
  typedef std::tuple<char, char, int> Tuple3;
  const Matcher<const Tuple3&> m = Args<0, 1>(Lt());
  EXPECT_TRUE(m.Matches(Tuple3('a', 'b', 2)));
  EXPECT_FALSE(m.Matches(Tuple3('b', 'b', 2)));
}

// Validates that arg is printed as str.
MATCHER_P(PrintsAs, str, "") {
  return testing::PrintToString(arg) == str;
}

TEST(ArgsTest, AcceptsTenTemplateArgs) {
  EXPECT_THAT(std::make_tuple(0, 1L, 2, 3L, 4, 5, 6, 7, 8, 9),
              (Args<9, 8, 7, 6, 5, 4, 3, 2, 1, 0>(
                  PrintsAs("(9, 8, 7, 6, 5, 4, 3, 2, 1, 0)"))));
  EXPECT_THAT(std::make_tuple(0, 1L, 2, 3L, 4, 5, 6, 7, 8, 9),
              Not(Args<9, 8, 7, 6, 5, 4, 3, 2, 1, 0>(
                  PrintsAs("(0, 8, 7, 6, 5, 4, 3, 2, 1, 0)"))));
}

TEST(ArgsTest, DescirbesSelfCorrectly) {
  const Matcher<std::tuple<int, bool, char> > m = Args<2, 0>(Lt());
  EXPECT_EQ("are a tuple whose fields (#2, #0) are a pair where "
            "the first < the second",
            Describe(m));
}

TEST(ArgsTest, DescirbesNestedArgsCorrectly) {
  const Matcher<const std::tuple<int, bool, char, int>&> m =
      Args<0, 2, 3>(Args<2, 0>(Lt()));
  EXPECT_EQ("are a tuple whose fields (#0, #2, #3) are a tuple "
            "whose fields (#2, #0) are a pair where the first < the second",
            Describe(m));
}

TEST(ArgsTest, DescribesNegationCorrectly) {
  const Matcher<std::tuple<int, char> > m = Args<1, 0>(Gt());
  EXPECT_EQ("are a tuple whose fields (#1, #0) aren't a pair "
            "where the first > the second",
            DescribeNegation(m));
}

TEST(ArgsTest, ExplainsMatchResultWithoutInnerExplanation) {
  const Matcher<std::tuple<bool, int, int> > m = Args<1, 2>(Eq());
  EXPECT_EQ("whose fields (#1, #2) are (42, 42)",
            Explain(m, std::make_tuple(false, 42, 42)));
  EXPECT_EQ("whose fields (#1, #2) are (42, 43)",
            Explain(m, std::make_tuple(false, 42, 43)));
}

// For testing Args<>'s explanation.
class LessThanMatcher : public MatcherInterface<std::tuple<char, int> > {
 public:
  void DescribeTo(::std::ostream* /*os*/) const override {}

  bool MatchAndExplain(std::tuple<char, int> value,
                       MatchResultListener* listener) const override {
    const int diff = std::get<0>(value) - std::get<1>(value);
    if (diff > 0) {
      *listener << "where the first value is " << diff
                << " more than the second";
    }
    return diff < 0;
  }
};

Matcher<std::tuple<char, int> > LessThan() {
  return MakeMatcher(new LessThanMatcher);
}

TEST(ArgsTest, ExplainsMatchResultWithInnerExplanation) {
  const Matcher<std::tuple<char, int, int> > m = Args<0, 2>(LessThan());
  EXPECT_EQ(
      "whose fields (#0, #2) are ('a' (97, 0x61), 42), "
      "where the first value is 55 more than the second",
      Explain(m, std::make_tuple('a', 42, 42)));
  EXPECT_EQ("whose fields (#0, #2) are ('\\0', 43)",
            Explain(m, std::make_tuple('\0', 42, 43)));
}

class PredicateFormatterFromMatcherTest : public ::testing::Test {
 protected:
  enum Behavior { kInitialSuccess, kAlwaysFail, kFlaky };

  // A matcher that can return different results when used multiple times on the
  // same input. No real matcher should do this; but this lets us test that we
  // detect such behavior and fail appropriately.
  class MockMatcher : public MatcherInterface<Behavior> {
   public:
    bool MatchAndExplain(Behavior behavior,
                         MatchResultListener* listener) const override {
      *listener << "[MatchAndExplain]";
      switch (behavior) {
        case kInitialSuccess:
          // The first call to MatchAndExplain should use a "not interested"
          // listener; so this is expected to return |true|. There should be no
          // subsequent calls.
          return !listener->IsInterested();

        case kAlwaysFail:
          return false;

        case kFlaky:
          // The first call to MatchAndExplain should use a "not interested"
          // listener; so this will return |false|. Subsequent calls should have
          // an "interested" listener; so this will return |true|, thus
          // simulating a flaky matcher.
          return listener->IsInterested();
      }

      GTEST_LOG_(FATAL) << "This should never be reached";
      return false;
    }

    void DescribeTo(ostream* os) const override { *os << "[DescribeTo]"; }

    void DescribeNegationTo(ostream* os) const override {
      *os << "[DescribeNegationTo]";
    }
  };

  AssertionResult RunPredicateFormatter(Behavior behavior) {
    auto matcher = MakeMatcher(new MockMatcher);
    PredicateFormatterFromMatcher<Matcher<Behavior>> predicate_formatter(
        matcher);
    return predicate_formatter("dummy-name", behavior);
  }
};

TEST_F(PredicateFormatterFromMatcherTest, ShortCircuitOnSuccess) {
  AssertionResult result = RunPredicateFormatter(kInitialSuccess);
  EXPECT_TRUE(result);  // Implicit cast to bool.
  std::string expect;
  EXPECT_EQ(expect, result.message());
}

TEST_F(PredicateFormatterFromMatcherTest, NoShortCircuitOnFailure) {
  AssertionResult result = RunPredicateFormatter(kAlwaysFail);
  EXPECT_FALSE(result);  // Implicit cast to bool.
  std::string expect =
      "Value of: dummy-name\nExpected: [DescribeTo]\n"
      "  Actual: 1" +
      OfType(internal::GetTypeName<Behavior>()) + ", [MatchAndExplain]";
  EXPECT_EQ(expect, result.message());
}

TEST_F(PredicateFormatterFromMatcherTest, DetectsFlakyShortCircuit) {
  AssertionResult result = RunPredicateFormatter(kFlaky);
  EXPECT_FALSE(result);  // Implicit cast to bool.
  std::string expect =
      "Value of: dummy-name\nExpected: [DescribeTo]\n"
      "  The matcher failed on the initial attempt; but passed when rerun to "
      "generate the explanation.\n"
      "  Actual: 2" +
      OfType(internal::GetTypeName<Behavior>()) + ", [MatchAndExplain]";
  EXPECT_EQ(expect, result.message());
}

// Tests for ElementsAre().

TEST(ElementsAreTest, CanDescribeExpectingNoElement) {
  Matcher<const vector<int>&> m = ElementsAre();
  EXPECT_EQ("is empty", Describe(m));
}

TEST(ElementsAreTest, CanDescribeExpectingOneElement) {
  Matcher<vector<int>> m = ElementsAre(Gt(5));
  EXPECT_EQ("has 1 element that is > 5", Describe(m));
}

TEST(ElementsAreTest, CanDescribeExpectingManyElements) {
  Matcher<list<std::string>> m = ElementsAre(StrEq("one"), "two");
  EXPECT_EQ(
      "has 2 elements where\n"
      "element #0 is equal to \"one\",\n"
      "element #1 is equal to \"two\"",
      Describe(m));
}

TEST(ElementsAreTest, CanDescribeNegationOfExpectingNoElement) {
  Matcher<vector<int>> m = ElementsAre();
  EXPECT_EQ("isn't empty", DescribeNegation(m));
}

TEST(ElementsAreTest, CanDescribeNegationOfExpectingOneElment) {
  Matcher<const list<int>&> m = ElementsAre(Gt(5));
  EXPECT_EQ(
      "doesn't have 1 element, or\n"
      "element #0 isn't > 5",
      DescribeNegation(m));
}

TEST(ElementsAreTest, CanDescribeNegationOfExpectingManyElements) {
  Matcher<const list<std::string>&> m = ElementsAre("one", "two");
  EXPECT_EQ(
      "doesn't have 2 elements, or\n"
      "element #0 isn't equal to \"one\", or\n"
      "element #1 isn't equal to \"two\"",
      DescribeNegation(m));
}

TEST(ElementsAreTest, DoesNotExplainTrivialMatch) {
  Matcher<const list<int>&> m = ElementsAre(1, Ne(2));

  list<int> test_list;
  test_list.push_back(1);
  test_list.push_back(3);
  EXPECT_EQ("", Explain(m, test_list));  // No need to explain anything.
}

TEST(ElementsAreTest, ExplainsNonTrivialMatch) {
  Matcher<const vector<int>&> m =
      ElementsAre(GreaterThan(1), 0, GreaterThan(2));

  const int a[] = {10, 0, 100};
  vector<int> test_vector(std::begin(a), std::end(a));
  EXPECT_EQ(
      "whose element #0 matches, which is 9 more than 1,\n"
      "and whose element #2 matches, which is 98 more than 2",
      Explain(m, test_vector));
}

TEST(ElementsAreTest, CanExplainMismatchWrongSize) {
  Matcher<const list<int>&> m = ElementsAre(1, 3);

  list<int> test_list;
  // No need to explain when the container is empty.
  EXPECT_EQ("", Explain(m, test_list));

  test_list.push_back(1);
  EXPECT_EQ("which has 1 element", Explain(m, test_list));
}

TEST(ElementsAreTest, CanExplainMismatchRightSize) {
  Matcher<const vector<int>&> m = ElementsAre(1, GreaterThan(5));

  vector<int> v;
  v.push_back(2);
  v.push_back(1);
  EXPECT_EQ("whose element #0 doesn't match", Explain(m, v));

  v[0] = 1;
  EXPECT_EQ("whose element #1 doesn't match, which is 4 less than 5",
            Explain(m, v));
}

TEST(ElementsAreTest, MatchesOneElementVector) {
  vector<std::string> test_vector;
  test_vector.push_back("test string");

  EXPECT_THAT(test_vector, ElementsAre(StrEq("test string")));
}

TEST(ElementsAreTest, MatchesOneElementList) {
  list<std::string> test_list;
  test_list.push_back("test string");

  EXPECT_THAT(test_list, ElementsAre("test string"));
}

TEST(ElementsAreTest, MatchesThreeElementVector) {
  vector<std::string> test_vector;
  test_vector.push_back("one");
  test_vector.push_back("two");
  test_vector.push_back("three");

  EXPECT_THAT(test_vector, ElementsAre("one", StrEq("two"), _));
}

TEST(ElementsAreTest, MatchesOneElementEqMatcher) {
  vector<int> test_vector;
  test_vector.push_back(4);

  EXPECT_THAT(test_vector, ElementsAre(Eq(4)));
}

TEST(ElementsAreTest, MatchesOneElementAnyMatcher) {
  vector<int> test_vector;
  test_vector.push_back(4);

  EXPECT_THAT(test_vector, ElementsAre(_));
}

TEST(ElementsAreTest, MatchesOneElementValue) {
  vector<int> test_vector;
  test_vector.push_back(4);

  EXPECT_THAT(test_vector, ElementsAre(4));
}

TEST(ElementsAreTest, MatchesThreeElementsMixedMatchers) {
  vector<int> test_vector;
  test_vector.push_back(1);
  test_vector.push_back(2);
  test_vector.push_back(3);

  EXPECT_THAT(test_vector, ElementsAre(1, Eq(2), _));
}

TEST(ElementsAreTest, MatchesTenElementVector) {
  const int a[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  vector<int> test_vector(std::begin(a), std::end(a));

  EXPECT_THAT(test_vector,
              // The element list can contain values and/or matchers
              // of different types.
              ElementsAre(0, Ge(0), _, 3, 4, Ne(2), Eq(6), 7, 8, _));
}

TEST(ElementsAreTest, DoesNotMatchWrongSize) {
  vector<std::string> test_vector;
  test_vector.push_back("test string");
  test_vector.push_back("test string");

  Matcher<vector<std::string>> m = ElementsAre(StrEq("test string"));
  EXPECT_FALSE(m.Matches(test_vector));
}

TEST(ElementsAreTest, DoesNotMatchWrongValue) {
  vector<std::string> test_vector;
  test_vector.push_back("other string");

  Matcher<vector<std::string>> m = ElementsAre(StrEq("test string"));
  EXPECT_FALSE(m.Matches(test_vector));
}

TEST(ElementsAreTest, DoesNotMatchWrongOrder) {
  vector<std::string> test_vector;
  test_vector.push_back("one");
  test_vector.push_back("three");
  test_vector.push_back("two");

  Matcher<vector<std::string>> m =
      ElementsAre(StrEq("one"), StrEq("two"), StrEq("three"));
  EXPECT_FALSE(m.Matches(test_vector));
}

TEST(ElementsAreTest, WorksForNestedContainer) {
  constexpr std::array<const char*, 2> strings = {{"Hi", "world"}};

  vector<list<char>> nested;
  for (const auto& s : strings) {
    nested.emplace_back(s, s + strlen(s));
  }

  EXPECT_THAT(nested, ElementsAre(ElementsAre('H', Ne('e')),
                                  ElementsAre('w', 'o', _, _, 'd')));
  EXPECT_THAT(nested, Not(ElementsAre(ElementsAre('H', 'e'),
                                      ElementsAre('w', 'o', _, _, 'd'))));
}

TEST(ElementsAreTest, WorksWithByRefElementMatchers) {
  int a[] = {0, 1, 2};
  vector<int> v(std::begin(a), std::end(a));

  EXPECT_THAT(v, ElementsAre(Ref(v[0]), Ref(v[1]), Ref(v[2])));
  EXPECT_THAT(v, Not(ElementsAre(Ref(v[0]), Ref(v[1]), Ref(a[2]))));
}

TEST(ElementsAreTest, WorksWithContainerPointerUsingPointee) {
  int a[] = {0, 1, 2};
  vector<int> v(std::begin(a), std::end(a));

  EXPECT_THAT(&v, Pointee(ElementsAre(0, 1, _)));
  EXPECT_THAT(&v, Not(Pointee(ElementsAre(0, _, 3))));
}

TEST(ElementsAreTest, WorksWithNativeArrayPassedByReference) {
  int array[] = {0, 1, 2};
  EXPECT_THAT(array, ElementsAre(0, 1, _));
  EXPECT_THAT(array, Not(ElementsAre(1, _, _)));
  EXPECT_THAT(array, Not(ElementsAre(0, _)));
}

class NativeArrayPassedAsPointerAndSize {
 public:
  NativeArrayPassedAsPointerAndSize() {}

  MOCK_METHOD(void, Helper, (int* array, int size));

 private:
  GTEST_DISALLOW_COPY_AND_ASSIGN_(NativeArrayPassedAsPointerAndSize);
};

TEST(ElementsAreTest, WorksWithNativeArrayPassedAsPointerAndSize) {
  int array[] = {0, 1};
  ::std::tuple<int*, size_t> array_as_tuple(array, 2);
  EXPECT_THAT(array_as_tuple, ElementsAre(0, 1));
  EXPECT_THAT(array_as_tuple, Not(ElementsAre(0)));

  NativeArrayPassedAsPointerAndSize helper;
  EXPECT_CALL(helper, Helper(_, _)).With(ElementsAre(0, 1));
  helper.Helper(array, 2);
}

TEST(ElementsAreTest, WorksWithTwoDimensionalNativeArray) {
  const char a2[][3] = {"hi", "lo"};
  EXPECT_THAT(a2, ElementsAre(ElementsAre('h', 'i', '\0'),
                              ElementsAre('l', 'o', '\0')));
  EXPECT_THAT(a2, ElementsAre(StrEq("hi"), StrEq("lo")));
  EXPECT_THAT(a2, ElementsAre(Not(ElementsAre('h', 'o', '\0')),
                              ElementsAre('l', 'o', '\0')));
}

TEST(ElementsAreTest, AcceptsStringLiteral) {
  std::string array[] = {"hi", "one", "two"};
  EXPECT_THAT(array, ElementsAre("hi", "one", "two"));
  EXPECT_THAT(array, Not(ElementsAre("hi", "one", "too")));
}

// Declared here with the size unknown.  Defined AFTER the following test.
extern const char kHi[];

TEST(ElementsAreTest, AcceptsArrayWithUnknownSize) {
  // The size of kHi is not known in this test, but ElementsAre() should
  // still accept it.

  std::string array1[] = {"hi"};
  EXPECT_THAT(array1, ElementsAre(kHi));

  std::string array2[] = {"ho"};
  EXPECT_THAT(array2, Not(ElementsAre(kHi)));
}

const char kHi[] = "hi";

TEST(ElementsAreTest, MakesCopyOfArguments) {
  int x = 1;
  int y = 2;
  // This should make a copy of x and y.
  ::testing::internal::ElementsAreMatcher<std::tuple<int, int>>
      polymorphic_matcher = ElementsAre(x, y);
  // Changing x and y now shouldn't affect the meaning of the above matcher.
  x = y = 0;
  const int array1[] = {1, 2};
  EXPECT_THAT(array1, polymorphic_matcher);
  const int array2[] = {0, 0};
  EXPECT_THAT(array2, Not(polymorphic_matcher));
}

// Tests for ElementsAreArray().  Since ElementsAreArray() shares most
// of the implementation with ElementsAre(), we don't test it as
// thoroughly here.

TEST(ElementsAreArrayTest, CanBeCreatedWithValueArray) {
  const int a[] = {1, 2, 3};

  vector<int> test_vector(std::begin(a), std::end(a));
  EXPECT_THAT(test_vector, ElementsAreArray(a));

  test_vector[2] = 0;
  EXPECT_THAT(test_vector, Not(ElementsAreArray(a)));
}

TEST(ElementsAreArrayTest, CanBeCreatedWithArraySize) {
  std::array<const char*, 3> a = {{"one", "two", "three"}};

  vector<std::string> test_vector(std::begin(a), std::end(a));
  EXPECT_THAT(test_vector, ElementsAreArray(a.data(), a.size()));

  const char** p = a.data();
  test_vector[0] = "1";
  EXPECT_THAT(test_vector, Not(ElementsAreArray(p, a.size())));
}

TEST(ElementsAreArrayTest, CanBeCreatedWithoutArraySize) {
  const char* a[] = {"one", "two", "three"};

  vector<std::string> test_vector(std::begin(a), std::end(a));
  EXPECT_THAT(test_vector, ElementsAreArray(a));

  test_vector[0] = "1";
  EXPECT_THAT(test_vector, Not(ElementsAreArray(a)));
}

TEST(ElementsAreArrayTest, CanBeCreatedWithMatcherArray) {
  const Matcher<std::string> kMatcherArray[] = {StrEq("one"), StrEq("two"),
                                                StrEq("three")};

  vector<std::string> test_vector;
  test_vector.push_back("one");
  test_vector.push_back("two");
  test_vector.push_back("three");
  EXPECT_THAT(test_vector, ElementsAreArray(kMatcherArray));

  test_vector.push_back("three");
  EXPECT_THAT(test_vector, Not(ElementsAreArray(kMatcherArray)));
}

TEST(ElementsAreArrayTest, CanBeCreatedWithVector) {
  const int a[] = {1, 2, 3};
  vector<int> test_vector(std::begin(a), std::end(a));
  const vector<int> expected(std::begin(a), std::end(a));
  EXPECT_THAT(test_vector, ElementsAreArray(expected));
  test_vector.push_back(4);
  EXPECT_THAT(test_vector, Not(ElementsAreArray(expected)));
}

TEST(ElementsAreArrayTest, TakesInitializerList) {
  const int a[5] = {1, 2, 3, 4, 5};
  EXPECT_THAT(a, ElementsAreArray({1, 2, 3, 4, 5}));
  EXPECT_THAT(a, Not(ElementsAreArray({1, 2, 3, 5, 4})));
  EXPECT_THAT(a, Not(ElementsAreArray({1, 2, 3, 4, 6})));
}

TEST(ElementsAreArrayTest, TakesInitializerListOfCStrings) {
  const std::string a[5] = {"a", "b", "c", "d", "e"};
  EXPECT_THAT(a, ElementsAreArray({"a", "b", "c", "d", "e"}));
  EXPECT_THAT(a, Not(ElementsAreArray({"a", "b", "c", "e", "d"})));
  EXPECT_THAT(a, Not(ElementsAreArray({"a", "b", "c", "d", "ef"})));
}

TEST(ElementsAreArrayTest, TakesInitializerListOfSameTypedMatchers) {
  const int a[5] = {1, 2, 3, 4, 5};
  EXPECT_THAT(a, ElementsAreArray({Eq(1), Eq(2), Eq(3), Eq(4), Eq(5)}));
  EXPECT_THAT(a, Not(ElementsAreArray({Eq(1), Eq(2), Eq(3), Eq(4), Eq(6)})));
}

TEST(ElementsAreArrayTest, TakesInitializerListOfDifferentTypedMatchers) {
  const int a[5] = {1, 2, 3, 4, 5};
  // The compiler cannot infer the type of the initializer list if its
  // elements have different types.  We must explicitly specify the
  // unified element type in this case.
  EXPECT_THAT(
      a, ElementsAreArray<Matcher<int>>({Eq(1), Ne(-2), Ge(3), Le(4), Eq(5)}));
  EXPECT_THAT(a, Not(ElementsAreArray<Matcher<int>>(
                     {Eq(1), Ne(-2), Ge(3), Le(4), Eq(6)})));
}

TEST(ElementsAreArrayTest, CanBeCreatedWithMatcherVector) {
  const int a[] = {1, 2, 3};
  const Matcher<int> kMatchers[] = {Eq(1), Eq(2), Eq(3)};
  vector<int> test_vector(std::begin(a), std::end(a));
  const vector<Matcher<int>> expected(std::begin(kMatchers),
                                      std::end(kMatchers));
  EXPECT_THAT(test_vector, ElementsAreArray(expected));
  test_vector.push_back(4);
  EXPECT_THAT(test_vector, Not(ElementsAreArray(expected)));
}

TEST(ElementsAreArrayTest, CanBeCreatedWithIteratorRange) {
  const int a[] = {1, 2, 3};
  const vector<int> test_vector(std::begin(a), std::end(a));
  const vector<int> expected(std::begin(a), std::end(a));
  EXPECT_THAT(test_vector, ElementsAreArray(expected.begin(), expected.end()));
  // Pointers are iterators, too.
  EXPECT_THAT(test_vector, ElementsAreArray(std::begin(a), std::end(a)));
  // The empty range of NULL pointers should also be okay.
  int* const null_int = nullptr;
  EXPECT_THAT(test_vector, Not(ElementsAreArray(null_int, null_int)));
  EXPECT_THAT((vector<int>()), ElementsAreArray(null_int, null_int));
}

// Since ElementsAre() and ElementsAreArray() share much of the
// implementation, we only do a sanity test for native arrays here.
TEST(ElementsAreArrayTest, WorksWithNativeArray) {
  ::std::string a[] = {"hi", "ho"};
  ::std::string b[] = {"hi", "ho"};

  EXPECT_THAT(a, ElementsAreArray(b));
  EXPECT_THAT(a, ElementsAreArray(b, 2));
  EXPECT_THAT(a, Not(ElementsAreArray(b, 1)));
}

TEST(ElementsAreArrayTest, SourceLifeSpan) {
  const int a[] = {1, 2, 3};
  vector<int> test_vector(std::begin(a), std::end(a));
  vector<int> expect(std::begin(a), std::end(a));
  ElementsAreArrayMatcher<int> matcher_maker =
      ElementsAreArray(expect.begin(), expect.end());
  EXPECT_THAT(test_vector, matcher_maker);
  // Changing in place the values that initialized matcher_maker should not
  // affect matcher_maker anymore. It should have made its own copy of them.
  for (int& i : expect) {
    i += 10;
  }
  EXPECT_THAT(test_vector, matcher_maker);
  test_vector.push_back(3);
  EXPECT_THAT(test_vector, Not(matcher_maker));
}

// Tests for the MATCHER*() macro family.

// Tests that a simple MATCHER() definition works.

MATCHER(IsEven, "") { return (arg % 2) == 0; }

TEST(MatcherMacroTest, Works) {
  const Matcher<int> m = IsEven();
  EXPECT_TRUE(m.Matches(6));
  EXPECT_FALSE(m.Matches(7));

  EXPECT_EQ("is even", Describe(m));
  EXPECT_EQ("not (is even)", DescribeNegation(m));
  EXPECT_EQ("", Explain(m, 6));
  EXPECT_EQ("", Explain(m, 7));
}

// This also tests that the description string can reference 'negation'.
MATCHER(IsEven2, negation ? "is odd" : "is even") {
  if ((arg % 2) == 0) {
    // Verifies that we can stream to result_listener, a listener
    // supplied by the MATCHER macro implicitly.
    *result_listener << "OK";
    return true;
  } else {
    *result_listener << "% 2 == " << (arg % 2);
    return false;
  }
}

// This also tests that the description string can reference matcher
// parameters.
MATCHER_P2(EqSumOf, x, y,
           std::string(negation ? "doesn't equal" : "equals") + " the sum of " +
               PrintToString(x) + " and " + PrintToString(y)) {
  if (arg == (x + y)) {
    *result_listener << "OK";
    return true;
  } else {
    // Verifies that we can stream to the underlying stream of
    // result_listener.
    if (result_listener->stream() != nullptr) {
      *result_listener->stream() << "diff == " << (x + y - arg);
    }
    return false;
  }
}

// Tests that the matcher description can reference 'negation' and the
// matcher parameters.
TEST(MatcherMacroTest, DescriptionCanReferenceNegationAndParameters) {
  const Matcher<int> m1 = IsEven2();
  EXPECT_EQ("is even", Describe(m1));
  EXPECT_EQ("is odd", DescribeNegation(m1));

  const Matcher<int> m2 = EqSumOf(5, 9);
  EXPECT_EQ("equals the sum of 5 and 9", Describe(m2));
  EXPECT_EQ("doesn't equal the sum of 5 and 9", DescribeNegation(m2));
}

// Tests explaining match result in a MATCHER* macro.
TEST(MatcherMacroTest, CanExplainMatchResult) {
  const Matcher<int> m1 = IsEven2();
  EXPECT_EQ("OK", Explain(m1, 4));
  EXPECT_EQ("% 2 == 1", Explain(m1, 5));

  const Matcher<int> m2 = EqSumOf(1, 2);
  EXPECT_EQ("OK", Explain(m2, 3));
  EXPECT_EQ("diff == -1", Explain(m2, 4));
}

// Tests that the body of MATCHER() can reference the type of the
// value being matched.

MATCHER(IsEmptyString, "") {
  StaticAssertTypeEq<::std::string, arg_type>();
  return arg.empty();
}

MATCHER(IsEmptyStringByRef, "") {
  StaticAssertTypeEq<const ::std::string&, arg_type>();
  return arg.empty();
}

TEST(MatcherMacroTest, CanReferenceArgType) {
  const Matcher<::std::string> m1 = IsEmptyString();
  EXPECT_TRUE(m1.Matches(""));

  const Matcher<const ::std::string&> m2 = IsEmptyStringByRef();
  EXPECT_TRUE(m2.Matches(""));
}

// Tests that MATCHER() can be used in a namespace.

namespace matcher_test {
MATCHER(IsOdd, "") { return (arg % 2) != 0; }
}  // namespace matcher_test

TEST(MatcherMacroTest, WorksInNamespace) {
  Matcher<int> m = matcher_test::IsOdd();
  EXPECT_FALSE(m.Matches(4));
  EXPECT_TRUE(m.Matches(5));
}

// Tests that Value() can be used to compose matchers.
MATCHER(IsPositiveOdd, "") {
  return Value(arg, matcher_test::IsOdd()) && arg > 0;
}

TEST(MatcherMacroTest, CanBeComposedUsingValue) {
  EXPECT_THAT(3, IsPositiveOdd());
  EXPECT_THAT(4, Not(IsPositiveOdd()));
  EXPECT_THAT(-1, Not(IsPositiveOdd()));
}

// Tests that a simple MATCHER_P() definition works.

MATCHER_P(IsGreaterThan32And, n, "") { return arg > 32 && arg > n; }

TEST(MatcherPMacroTest, Works) {
  const Matcher<int> m = IsGreaterThan32And(5);
  EXPECT_TRUE(m.Matches(36));
  EXPECT_FALSE(m.Matches(5));

  EXPECT_EQ("is greater than 32 and 5", Describe(m));
  EXPECT_EQ("not (is greater than 32 and 5)", DescribeNegation(m));
  EXPECT_EQ("", Explain(m, 36));
  EXPECT_EQ("", Explain(m, 5));
}

// Tests that the description is calculated correctly from the matcher name.
MATCHER_P(_is_Greater_Than32and_, n, "") { return arg > 32 && arg > n; }

TEST(MatcherPMacroTest, GeneratesCorrectDescription) {
  const Matcher<int> m = _is_Greater_Than32and_(5);

  EXPECT_EQ("is greater than 32 and 5", Describe(m));
  EXPECT_EQ("not (is greater than 32 and 5)", DescribeNegation(m));
  EXPECT_EQ("", Explain(m, 36));
  EXPECT_EQ("", Explain(m, 5));
}

// Tests that a MATCHER_P matcher can be explicitly instantiated with
// a reference parameter type.

class UncopyableFoo {
 public:
  explicit UncopyableFoo(char value) : value_(value) { (void)value_; }

  UncopyableFoo(const UncopyableFoo&) = delete;
  void operator=(const UncopyableFoo&) = delete;

 private:
  char value_;
};

MATCHER_P(ReferencesUncopyable, variable, "") { return &arg == &variable; }

TEST(MatcherPMacroTest, WorksWhenExplicitlyInstantiatedWithReference) {
  UncopyableFoo foo1('1'), foo2('2');
  const Matcher<const UncopyableFoo&> m =
      ReferencesUncopyable<const UncopyableFoo&>(foo1);

  EXPECT_TRUE(m.Matches(foo1));
  EXPECT_FALSE(m.Matches(foo2));

  // We don't want the address of the parameter printed, as most
  // likely it will just annoy the user.  If the address is
  // interesting, the user should consider passing the parameter by
  // pointer instead.
  EXPECT_EQ("references uncopyable 1-byte object <31>", Describe(m));
}

// Tests that the body of MATCHER_Pn() can reference the parameter
// types.

MATCHER_P3(ParamTypesAreIntLongAndChar, foo, bar, baz, "") {
  StaticAssertTypeEq<int, foo_type>();
  StaticAssertTypeEq<long, bar_type>();  // NOLINT
  StaticAssertTypeEq<char, baz_type>();
  return arg == 0;
}

TEST(MatcherPnMacroTest, CanReferenceParamTypes) {
  EXPECT_THAT(0, ParamTypesAreIntLongAndChar(10, 20L, 'a'));
}

// Tests that a MATCHER_Pn matcher can be explicitly instantiated with
// reference parameter types.

MATCHER_P2(ReferencesAnyOf, variable1, variable2, "") {
  return &arg == &variable1 || &arg == &variable2;
}

TEST(MatcherPnMacroTest, WorksWhenExplicitlyInstantiatedWithReferences) {
  UncopyableFoo foo1('1'), foo2('2'), foo3('3');
  const Matcher<const UncopyableFoo&> const_m =
      ReferencesAnyOf<const UncopyableFoo&, const UncopyableFoo&>(foo1, foo2);

  EXPECT_TRUE(const_m.Matches(foo1));
  EXPECT_TRUE(const_m.Matches(foo2));
  EXPECT_FALSE(const_m.Matches(foo3));

  const Matcher<UncopyableFoo&> m =
      ReferencesAnyOf<UncopyableFoo&, UncopyableFoo&>(foo1, foo2);

  EXPECT_TRUE(m.Matches(foo1));
  EXPECT_TRUE(m.Matches(foo2));
  EXPECT_FALSE(m.Matches(foo3));
}

TEST(MatcherPnMacroTest,
     GeneratesCorretDescriptionWhenExplicitlyInstantiatedWithReferences) {
  UncopyableFoo foo1('1'), foo2('2');
  const Matcher<const UncopyableFoo&> m =
      ReferencesAnyOf<const UncopyableFoo&, const UncopyableFoo&>(foo1, foo2);

  // We don't want the addresses of the parameters printed, as most
  // likely they will just annoy the user.  If the addresses are
  // interesting, the user should consider passing the parameters by
  // pointers instead.
  EXPECT_EQ("references any of (1-byte object <31>, 1-byte object <32>)",
            Describe(m));
}

// Tests that a simple MATCHER_P2() definition works.

MATCHER_P2(IsNotInClosedRange, low, hi, "") { return arg < low || arg > hi; }

TEST(MatcherPnMacroTest, Works) {
  const Matcher<const long&> m = IsNotInClosedRange(10, 20);  // NOLINT
  EXPECT_TRUE(m.Matches(36L));
  EXPECT_FALSE(m.Matches(15L));

  EXPECT_EQ("is not in closed range (10, 20)", Describe(m));
  EXPECT_EQ("not (is not in closed range (10, 20))", DescribeNegation(m));
  EXPECT_EQ("", Explain(m, 36L));
  EXPECT_EQ("", Explain(m, 15L));
}

// Tests that MATCHER*() definitions can be overloaded on the number
// of parameters; also tests MATCHER_Pn() where n >= 3.

MATCHER(EqualsSumOf, "") { return arg == 0; }
MATCHER_P(EqualsSumOf, a, "") { return arg == a; }
MATCHER_P2(EqualsSumOf, a, b, "") { return arg == a + b; }
MATCHER_P3(EqualsSumOf, a, b, c, "") { return arg == a + b + c; }
MATCHER_P4(EqualsSumOf, a, b, c, d, "") { return arg == a + b + c + d; }
MATCHER_P5(EqualsSumOf, a, b, c, d, e, "") { return arg == a + b + c + d + e; }
MATCHER_P6(EqualsSumOf, a, b, c, d, e, f, "") {
  return arg == a + b + c + d + e + f;
}
MATCHER_P7(EqualsSumOf, a, b, c, d, e, f, g, "") {
  return arg == a + b + c + d + e + f + g;
}
MATCHER_P8(EqualsSumOf, a, b, c, d, e, f, g, h, "") {
  return arg == a + b + c + d + e + f + g + h;
}
MATCHER_P9(EqualsSumOf, a, b, c, d, e, f, g, h, i, "") {
  return arg == a + b + c + d + e + f + g + h + i;
}
MATCHER_P10(EqualsSumOf, a, b, c, d, e, f, g, h, i, j, "") {
  return arg == a + b + c + d + e + f + g + h + i + j;
}

TEST(MatcherPnMacroTest, CanBeOverloadedOnNumberOfParameters) {
  EXPECT_THAT(0, EqualsSumOf());
  EXPECT_THAT(1, EqualsSumOf(1));
  EXPECT_THAT(12, EqualsSumOf(10, 2));
  EXPECT_THAT(123, EqualsSumOf(100, 20, 3));
  EXPECT_THAT(1234, EqualsSumOf(1000, 200, 30, 4));
  EXPECT_THAT(12345, EqualsSumOf(10000, 2000, 300, 40, 5));
  EXPECT_THAT("abcdef",
              EqualsSumOf(::std::string("a"), 'b', 'c', "d", "e", 'f'));
  EXPECT_THAT("abcdefg",
              EqualsSumOf(::std::string("a"), 'b', 'c', "d", "e", 'f', 'g'));
  EXPECT_THAT("abcdefgh", EqualsSumOf(::std::string("a"), 'b', 'c', "d", "e",
                                      'f', 'g', "h"));
  EXPECT_THAT("abcdefghi", EqualsSumOf(::std::string("a"), 'b', 'c', "d", "e",
                                       'f', 'g', "h", 'i'));
  EXPECT_THAT("abcdefghij",
              EqualsSumOf(::std::string("a"), 'b', 'c', "d", "e", 'f', 'g', "h",
                          'i', ::std::string("j")));

  EXPECT_THAT(1, Not(EqualsSumOf()));
  EXPECT_THAT(-1, Not(EqualsSumOf(1)));
  EXPECT_THAT(-12, Not(EqualsSumOf(10, 2)));
  EXPECT_THAT(-123, Not(EqualsSumOf(100, 20, 3)));
  EXPECT_THAT(-1234, Not(EqualsSumOf(1000, 200, 30, 4)));
  EXPECT_THAT(-12345, Not(EqualsSumOf(10000, 2000, 300, 40, 5)));
  EXPECT_THAT("abcdef ",
              Not(EqualsSumOf(::std::string("a"), 'b', 'c', "d", "e", 'f')));
  EXPECT_THAT("abcdefg ", Not(EqualsSumOf(::std::string("a"), 'b', 'c', "d",
                                          "e", 'f', 'g')));
  EXPECT_THAT("abcdefgh ", Not(EqualsSumOf(::std::string("a"), 'b', 'c', "d",
                                           "e", 'f', 'g', "h")));
  EXPECT_THAT("abcdefghi ", Not(EqualsSumOf(::std::string("a"), 'b', 'c', "d",
                                            "e", 'f', 'g', "h", 'i')));
  EXPECT_THAT("abcdefghij ",
              Not(EqualsSumOf(::std::string("a"), 'b', 'c', "d", "e", 'f', 'g',
                              "h", 'i', ::std::string("j"))));
}

// Tests that a MATCHER_Pn() definition can be instantiated with any
// compatible parameter types.
TEST(MatcherPnMacroTest, WorksForDifferentParameterTypes) {
  EXPECT_THAT(123, EqualsSumOf(100L, 20, static_cast<char>(3)));
  EXPECT_THAT("abcd", EqualsSumOf(::std::string("a"), "b", 'c', "d"));

  EXPECT_THAT(124, Not(EqualsSumOf(100L, 20, static_cast<char>(3))));
  EXPECT_THAT("abcde", Not(EqualsSumOf(::std::string("a"), "b", 'c', "d")));
}

// Tests that the matcher body can promote the parameter types.

MATCHER_P2(EqConcat, prefix, suffix, "") {
  // The following lines promote the two parameters to desired types.
  std::string prefix_str(prefix);
  char suffix_char = static_cast<char>(suffix);
  return arg == prefix_str + suffix_char;
}

TEST(MatcherPnMacroTest, SimpleTypePromotion) {
  Matcher<std::string> no_promo = EqConcat(std::string("foo"), 't');
  Matcher<const std::string&> promo = EqConcat("foo", static_cast<int>('t'));
  EXPECT_FALSE(no_promo.Matches("fool"));
  EXPECT_FALSE(promo.Matches("fool"));
  EXPECT_TRUE(no_promo.Matches("foot"));
  EXPECT_TRUE(promo.Matches("foot"));
}

// Verifies the type of a MATCHER*.

TEST(MatcherPnMacroTest, TypesAreCorrect) {
  // EqualsSumOf() must be assignable to a EqualsSumOfMatcher variable.
  EqualsSumOfMatcher a0 = EqualsSumOf();

  // EqualsSumOf(1) must be assignable to a EqualsSumOfMatcherP variable.
  EqualsSumOfMatcherP<int> a1 = EqualsSumOf(1);

  // EqualsSumOf(p1, ..., pk) must be assignable to a EqualsSumOfMatcherPk
  // variable, and so on.
  EqualsSumOfMatcherP2<int, char> a2 = EqualsSumOf(1, '2');
  EqualsSumOfMatcherP3<int, int, char> a3 = EqualsSumOf(1, 2, '3');
  EqualsSumOfMatcherP4<int, int, int, char> a4 = EqualsSumOf(1, 2, 3, '4');
  EqualsSumOfMatcherP5<int, int, int, int, char> a5 =
      EqualsSumOf(1, 2, 3, 4, '5');
  EqualsSumOfMatcherP6<int, int, int, int, int, char> a6 =
      EqualsSumOf(1, 2, 3, 4, 5, '6');
  EqualsSumOfMatcherP7<int, int, int, int, int, int, char> a7 =
      EqualsSumOf(1, 2, 3, 4, 5, 6, '7');
  EqualsSumOfMatcherP8<int, int, int, int, int, int, int, char> a8 =
      EqualsSumOf(1, 2, 3, 4, 5, 6, 7, '8');
  EqualsSumOfMatcherP9<int, int, int, int, int, int, int, int, char> a9 =
      EqualsSumOf(1, 2, 3, 4, 5, 6, 7, 8, '9');
  EqualsSumOfMatcherP10<int, int, int, int, int, int, int, int, int, char> a10 =
      EqualsSumOf(1, 2, 3, 4, 5, 6, 7, 8, 9, '0');

  // Avoid "unused variable" warnings.
  (void)a0;
  (void)a1;
  (void)a2;
  (void)a3;
  (void)a4;
  (void)a5;
  (void)a6;
  (void)a7;
  (void)a8;
  (void)a9;
  (void)a10;
}

// Tests that matcher-typed parameters can be used in Value() inside a
// MATCHER_Pn definition.

// Succeeds if arg matches exactly 2 of the 3 matchers.
MATCHER_P3(TwoOf, m1, m2, m3, "") {
  const int count = static_cast<int>(Value(arg, m1)) +
                    static_cast<int>(Value(arg, m2)) +
                    static_cast<int>(Value(arg, m3));
  return count == 2;
}

TEST(MatcherPnMacroTest, CanUseMatcherTypedParameterInValue) {
  EXPECT_THAT(42, TwoOf(Gt(0), Lt(50), Eq(10)));
  EXPECT_THAT(0, Not(TwoOf(Gt(-1), Lt(1), Eq(0))));
}

// Tests Contains().

TEST(ContainsTest, ListMatchesWhenElementIsInContainer) {
  list<int> some_list;
  some_list.push_back(3);
  some_list.push_back(1);
  some_list.push_back(2);
  EXPECT_THAT(some_list, Contains(1));
  EXPECT_THAT(some_list, Contains(Gt(2.5)));
  EXPECT_THAT(some_list, Contains(Eq(2.0f)));

  list<std::string> another_list;
  another_list.push_back("fee");
  another_list.push_back("fie");
  another_list.push_back("foe");
  another_list.push_back("fum");
  EXPECT_THAT(another_list, Contains(std::string("fee")));
}

TEST(ContainsTest, ListDoesNotMatchWhenElementIsNotInContainer) {
  list<int> some_list;
  some_list.push_back(3);
  some_list.push_back(1);
  EXPECT_THAT(some_list, Not(Contains(4)));
}

TEST(ContainsTest, SetMatchesWhenElementIsInContainer) {
  set<int> some_set;
  some_set.insert(3);
  some_set.insert(1);
  some_set.insert(2);
  EXPECT_THAT(some_set, Contains(Eq(1.0)));
  EXPECT_THAT(some_set, Contains(Eq(3.0f)));
  EXPECT_THAT(some_set, Contains(2));

  set<std::string> another_set;
  another_set.insert("fee");
  another_set.insert("fie");
  another_set.insert("foe");
  another_set.insert("fum");
  EXPECT_THAT(another_set, Contains(Eq(std::string("fum"))));
}

TEST(ContainsTest, SetDoesNotMatchWhenElementIsNotInContainer) {
  set<int> some_set;
  some_set.insert(3);
  some_set.insert(1);
  EXPECT_THAT(some_set, Not(Contains(4)));

  set<std::string> c_string_set;
  c_string_set.insert("hello");
  EXPECT_THAT(c_string_set, Not(Contains(std::string("goodbye"))));
}

TEST(ContainsTest, ExplainsMatchResultCorrectly) {
  const int a[2] = {1, 2};
  Matcher<const int(&)[2]> m = Contains(2);
  EXPECT_EQ("whose element #1 matches", Explain(m, a));

  m = Contains(3);
  EXPECT_EQ("", Explain(m, a));

  m = Contains(GreaterThan(0));
  EXPECT_EQ("whose element #0 matches, which is 1 more than 0", Explain(m, a));

  m = Contains(GreaterThan(10));
  EXPECT_EQ("", Explain(m, a));
}

TEST(ContainsTest, DescribesItselfCorrectly) {
  Matcher<vector<int>> m = Contains(1);
  EXPECT_EQ("contains at least one element that is equal to 1", Describe(m));

  Matcher<vector<int>> m2 = Not(m);
  EXPECT_EQ("doesn't contain any element that is equal to 1", Describe(m2));
}

TEST(ContainsTest, MapMatchesWhenElementIsInContainer) {
  map<std::string, int> my_map;
  const char* bar = "a string";
  my_map[bar] = 2;
  EXPECT_THAT(my_map, Contains(pair<const char* const, int>(bar, 2)));

  map<std::string, int> another_map;
  another_map["fee"] = 1;
  another_map["fie"] = 2;
  another_map["foe"] = 3;
  another_map["fum"] = 4;
  EXPECT_THAT(another_map,
              Contains(pair<const std::string, int>(std::string("fee"), 1)));
  EXPECT_THAT(another_map, Contains(pair<const std::string, int>("fie", 2)));
}

TEST(ContainsTest, MapDoesNotMatchWhenElementIsNotInContainer) {
  map<int, int> some_map;
  some_map[1] = 11;
  some_map[2] = 22;
  EXPECT_THAT(some_map, Not(Contains(pair<const int, int>(2, 23))));
}

TEST(ContainsTest, ArrayMatchesWhenElementIsInContainer) {
  const char* string_array[] = {"fee", "fie", "foe", "fum"};
  EXPECT_THAT(string_array, Contains(Eq(std::string("fum"))));
}

TEST(ContainsTest, ArrayDoesNotMatchWhenElementIsNotInContainer) {
  int int_array[] = {1, 2, 3, 4};
  EXPECT_THAT(int_array, Not(Contains(5)));
}

TEST(ContainsTest, AcceptsMatcher) {
  const int a[] = {1, 2, 3};
  EXPECT_THAT(a, Contains(Gt(2)));
  EXPECT_THAT(a, Not(Contains(Gt(4))));
}

TEST(ContainsTest, WorksForNativeArrayAsTuple) {
  const int a[] = {1, 2};
  const int* const pointer = a;
  EXPECT_THAT(std::make_tuple(pointer, 2), Contains(1));
  EXPECT_THAT(std::make_tuple(pointer, 2), Not(Contains(Gt(3))));
}

TEST(ContainsTest, WorksForTwoDimensionalNativeArray) {
  int a[][3] = {{1, 2, 3}, {4, 5, 6}};
  EXPECT_THAT(a, Contains(ElementsAre(4, 5, 6)));
  EXPECT_THAT(a, Contains(Contains(5)));
  EXPECT_THAT(a, Not(Contains(ElementsAre(3, 4, 5))));
  EXPECT_THAT(a, Contains(Not(Contains(5))));
}

TEST(AllOfArrayTest, BasicForms) {
  // Iterator
  std::vector<int> v0{};
  std::vector<int> v1{1};
  std::vector<int> v2{2, 3};
  std::vector<int> v3{4, 4, 4};
  EXPECT_THAT(0, AllOfArray(v0.begin(), v0.end()));
  EXPECT_THAT(1, AllOfArray(v1.begin(), v1.end()));
  EXPECT_THAT(2, Not(AllOfArray(v1.begin(), v1.end())));
  EXPECT_THAT(3, Not(AllOfArray(v2.begin(), v2.end())));
  EXPECT_THAT(4, AllOfArray(v3.begin(), v3.end()));
  // Pointer +  size
  int ar[6] = {1, 2, 3, 4, 4, 4};
  EXPECT_THAT(0, AllOfArray(ar, 0));
  EXPECT_THAT(1, AllOfArray(ar, 1));
  EXPECT_THAT(2, Not(AllOfArray(ar, 1)));
  EXPECT_THAT(3, Not(AllOfArray(ar + 1, 3)));
  EXPECT_THAT(4, AllOfArray(ar + 3, 3));
  // Array
  // int ar0[0];  Not usable
  int ar1[1] = {1};
  int ar2[2] = {2, 3};
  int ar3[3] = {4, 4, 4};
  // EXPECT_THAT(0, Not(AllOfArray(ar0)));  // Cannot work
  EXPECT_THAT(1, AllOfArray(ar1));
  EXPECT_THAT(2, Not(AllOfArray(ar1)));
  EXPECT_THAT(3, Not(AllOfArray(ar2)));
  EXPECT_THAT(4, AllOfArray(ar3));
  // Container
  EXPECT_THAT(0, AllOfArray(v0));
  EXPECT_THAT(1, AllOfArray(v1));
  EXPECT_THAT(2, Not(AllOfArray(v1)));
  EXPECT_THAT(3, Not(AllOfArray(v2)));
  EXPECT_THAT(4, AllOfArray(v3));
  // Initializer
  EXPECT_THAT(0, AllOfArray<int>({}));  // Requires template arg.
  EXPECT_THAT(1, AllOfArray({1}));
  EXPECT_THAT(2, Not(AllOfArray({1})));
  EXPECT_THAT(3, Not(AllOfArray({2, 3})));
  EXPECT_THAT(4, AllOfArray({4, 4, 4}));
}

TEST(AllOfArrayTest, Matchers) {
  // vector
  std::vector<Matcher<int>> matchers{Ge(1), Lt(2)};
  EXPECT_THAT(0, Not(AllOfArray(matchers)));
  EXPECT_THAT(1, AllOfArray(matchers));
  EXPECT_THAT(2, Not(AllOfArray(matchers)));
  // initializer_list
  EXPECT_THAT(0, Not(AllOfArray({Ge(0), Ge(1)})));
  EXPECT_THAT(1, AllOfArray({Ge(0), Ge(1)}));
}

TEST(AnyOfArrayTest, BasicForms) {
  // Iterator
  std::vector<int> v0{};
  std::vector<int> v1{1};
  std::vector<int> v2{2, 3};
  EXPECT_THAT(0, Not(AnyOfArray(v0.begin(), v0.end())));
  EXPECT_THAT(1, AnyOfArray(v1.begin(), v1.end()));
  EXPECT_THAT(2, Not(AnyOfArray(v1.begin(), v1.end())));
  EXPECT_THAT(3, AnyOfArray(v2.begin(), v2.end()));
  EXPECT_THAT(4, Not(AnyOfArray(v2.begin(), v2.end())));
  // Pointer +  size
  int ar[3] = {1, 2, 3};
  EXPECT_THAT(0, Not(AnyOfArray(ar, 0)));
  EXPECT_THAT(1, AnyOfArray(ar, 1));
  EXPECT_THAT(2, Not(AnyOfArray(ar, 1)));
  EXPECT_THAT(3, AnyOfArray(ar + 1, 2));
  EXPECT_THAT(4, Not(AnyOfArray(ar + 1, 2)));
  // Array
  // int ar0[0];  Not usable
  int ar1[1] = {1};
  int ar2[2] = {2, 3};
  // EXPECT_THAT(0, Not(AnyOfArray(ar0)));  // Cannot work
  EXPECT_THAT(1, AnyOfArray(ar1));
  EXPECT_THAT(2, Not(AnyOfArray(ar1)));
  EXPECT_THAT(3, AnyOfArray(ar2));
  EXPECT_THAT(4, Not(AnyOfArray(ar2)));
  // Container
  EXPECT_THAT(0, Not(AnyOfArray(v0)));
  EXPECT_THAT(1, AnyOfArray(v1));
  EXPECT_THAT(2, Not(AnyOfArray(v1)));
  EXPECT_THAT(3, AnyOfArray(v2));
  EXPECT_THAT(4, Not(AnyOfArray(v2)));
  // Initializer
  EXPECT_THAT(0, Not(AnyOfArray<int>({})));  // Requires template arg.
  EXPECT_THAT(1, AnyOfArray({1}));
  EXPECT_THAT(2, Not(AnyOfArray({1})));
  EXPECT_THAT(3, AnyOfArray({2, 3}));
  EXPECT_THAT(4, Not(AnyOfArray({2, 3})));
}

TEST(AnyOfArrayTest, Matchers) {
  // We negate test AllOfArrayTest.Matchers.
  // vector
  std::vector<Matcher<int>> matchers{Lt(1), Ge(2)};
  EXPECT_THAT(0, AnyOfArray(matchers));
  EXPECT_THAT(1, Not(AnyOfArray(matchers)));
  EXPECT_THAT(2, AnyOfArray(matchers));
  // initializer_list
  EXPECT_THAT(0, AnyOfArray({Lt(0), Lt(1)}));
  EXPECT_THAT(1, Not(AllOfArray({Lt(0), Lt(1)})));
}

TEST(AnyOfArrayTest, ExplainsMatchResultCorrectly) {
  // AnyOfArray and AllOfArry use the same underlying template-template,
  // thus it is sufficient to test one here.
  const std::vector<int> v0{};
  const std::vector<int> v1{1};
  const std::vector<int> v2{2, 3};
  const Matcher<int> m0 = AnyOfArray(v0);
  const Matcher<int> m1 = AnyOfArray(v1);
  const Matcher<int> m2 = AnyOfArray(v2);
  EXPECT_EQ("", Explain(m0, 0));
  EXPECT_EQ("", Explain(m1, 1));
  EXPECT_EQ("", Explain(m1, 2));
  EXPECT_EQ("", Explain(m2, 3));
  EXPECT_EQ("", Explain(m2, 4));
  EXPECT_EQ("()", Describe(m0));
  EXPECT_EQ("(is equal to 1)", Describe(m1));
  EXPECT_EQ("(is equal to 2) or (is equal to 3)", Describe(m2));
  EXPECT_EQ("()", DescribeNegation(m0));
  EXPECT_EQ("(isn't equal to 1)", DescribeNegation(m1));
  EXPECT_EQ("(isn't equal to 2) and (isn't equal to 3)", DescribeNegation(m2));
  // Explain with matchers
  const Matcher<int> g1 = AnyOfArray({GreaterThan(1)});
  const Matcher<int> g2 = AnyOfArray({GreaterThan(1), GreaterThan(2)});
  // Explains the first positiv match and all prior negative matches...
  EXPECT_EQ("which is 1 less than 1", Explain(g1, 0));
  EXPECT_EQ("which is the same as 1", Explain(g1, 1));
  EXPECT_EQ("which is 1 more than 1", Explain(g1, 2));
  EXPECT_EQ("which is 1 less than 1, and which is 2 less than 2",
            Explain(g2, 0));
  EXPECT_EQ("which is the same as 1, and which is 1 less than 2",
            Explain(g2, 1));
  EXPECT_EQ("which is 1 more than 1",  // Only the first
            Explain(g2, 2));
}

TEST(AllOfTest, HugeMatcher) {
  // Verify that using AllOf with many arguments doesn't cause
  // the compiler to exceed template instantiation depth limit.
  EXPECT_THAT(0, testing::AllOf(_, _, _, _, _, _, _, _, _,
                                testing::AllOf(_, _, _, _, _, _, _, _, _, _)));
}

TEST(AnyOfTest, HugeMatcher) {
  // Verify that using AnyOf with many arguments doesn't cause
  // the compiler to exceed template instantiation depth limit.
  EXPECT_THAT(0, testing::AnyOf(_, _, _, _, _, _, _, _, _,
                                testing::AnyOf(_, _, _, _, _, _, _, _, _, _)));
}

namespace adl_test {

// Verifies that the implementation of ::testing::AllOf and ::testing::AnyOf
// don't issue unqualified recursive calls.  If they do, the argument dependent
// name lookup will cause AllOf/AnyOf in the 'adl_test' namespace to be found
// as a candidate and the compilation will break due to an ambiguous overload.

// The matcher must be in the same namespace as AllOf/AnyOf to make argument
// dependent lookup find those.
MATCHER(M, "") {
  (void)arg;
  return true;
}

template <typename T1, typename T2>
bool AllOf(const T1& /*t1*/, const T2& /*t2*/) {
  return true;
}

TEST(AllOfTest, DoesNotCallAllOfUnqualified) {
  EXPECT_THAT(42,
              testing::AllOf(M(), M(), M(), M(), M(), M(), M(), M(), M(), M()));
}

template <typename T1, typename T2>
bool AnyOf(const T1&, const T2&) {
  return true;
}

TEST(AnyOfTest, DoesNotCallAnyOfUnqualified) {
  EXPECT_THAT(42,
              testing::AnyOf(M(), M(), M(), M(), M(), M(), M(), M(), M(), M()));
}

}  // namespace adl_test

TEST(AllOfTest, WorksOnMoveOnlyType) {
  std::unique_ptr<int> p(new int(3));
  EXPECT_THAT(p, AllOf(Pointee(Eq(3)), Pointee(Gt(0)), Pointee(Lt(5))));
  EXPECT_THAT(p, Not(AllOf(Pointee(Eq(3)), Pointee(Gt(0)), Pointee(Lt(3)))));
}

TEST(AnyOfTest, WorksOnMoveOnlyType) {
  std::unique_ptr<int> p(new int(3));
  EXPECT_THAT(p, AnyOf(Pointee(Eq(5)), Pointee(Lt(0)), Pointee(Lt(5))));
  EXPECT_THAT(p, Not(AnyOf(Pointee(Eq(5)), Pointee(Lt(0)), Pointee(Gt(5)))));
}

MATCHER(IsNotNull, "") { return arg != nullptr; }

// Verifies that a matcher defined using MATCHER() can work on
// move-only types.
TEST(MatcherMacroTest, WorksOnMoveOnlyType) {
  std::unique_ptr<int> p(new int(3));
  EXPECT_THAT(p, IsNotNull());
  EXPECT_THAT(std::unique_ptr<int>(), Not(IsNotNull()));
}

MATCHER_P(UniquePointee, pointee, "") { return *arg == pointee; }

// Verifies that a matcher defined using MATCHER_P*() can work on
// move-only types.
TEST(MatcherPMacroTest, WorksOnMoveOnlyType) {
  std::unique_ptr<int> p(new int(3));
  EXPECT_THAT(p, UniquePointee(3));
  EXPECT_THAT(p, Not(UniquePointee(2)));
}

#if GTEST_HAS_EXCEPTIONS

// std::function<void()> is used below for compatibility with older copies of
// GCC. Normally, a raw lambda is all that is needed.

// Test that examples from documentation compile
TEST(ThrowsTest, Examples) {
  EXPECT_THAT(
      std::function<void()>([]() { throw std::runtime_error("message"); }),
      Throws<std::runtime_error>());

  EXPECT_THAT(
      std::function<void()>([]() { throw std::runtime_error("message"); }),
      ThrowsMessage<std::runtime_error>(HasSubstr("message")));
}

TEST(ThrowsTest, DoesNotGenerateDuplicateCatchClauseWarning) {
  EXPECT_THAT(std::function<void()>([]() { throw std::exception(); }),
              Throws<std::exception>());
}

TEST(ThrowsTest, CallableExecutedExactlyOnce) {
  size_t a = 0;

  EXPECT_THAT(std::function<void()>([&a]() {
                a++;
                throw 10;
              }),
              Throws<int>());
  EXPECT_EQ(a, 1u);

  EXPECT_THAT(std::function<void()>([&a]() {
                a++;
                throw std::runtime_error("message");
              }),
              Throws<std::runtime_error>());
  EXPECT_EQ(a, 2u);

  EXPECT_THAT(std::function<void()>([&a]() {
                a++;
                throw std::runtime_error("message");
              }),
              ThrowsMessage<std::runtime_error>(HasSubstr("message")));
  EXPECT_EQ(a, 3u);

  EXPECT_THAT(std::function<void()>([&a]() {
                a++;
                throw std::runtime_error("message");
              }),
              Throws<std::runtime_error>(
                  Property(&std::runtime_error::what, HasSubstr("message"))));
  EXPECT_EQ(a, 4u);
}

TEST(ThrowsTest, Describe) {
  Matcher<std::function<void()>> matcher = Throws<std::runtime_error>();
  std::stringstream ss;
  matcher.DescribeTo(&ss);
  auto explanation = ss.str();
  EXPECT_THAT(explanation, HasSubstr("std::runtime_error"));
}

TEST(ThrowsTest, Success) {
  Matcher<std::function<void()>> matcher = Throws<std::runtime_error>();
  StringMatchResultListener listener;
  EXPECT_TRUE(matcher.MatchAndExplain(
      []() { throw std::runtime_error("error message"); }, &listener));
  EXPECT_THAT(listener.str(), HasSubstr("std::runtime_error"));
}

TEST(ThrowsTest, FailWrongType) {
  Matcher<std::function<void()>> matcher = Throws<std::runtime_error>();
  StringMatchResultListener listener;
  EXPECT_FALSE(matcher.MatchAndExplain(
      []() { throw std::logic_error("error message"); }, &listener));
  EXPECT_THAT(listener.str(), HasSubstr("std::logic_error"));
  EXPECT_THAT(listener.str(), HasSubstr("\"error message\""));
}

TEST(ThrowsTest, FailWrongTypeNonStd) {
  Matcher<std::function<void()>> matcher = Throws<std::runtime_error>();
  StringMatchResultListener listener;
  EXPECT_FALSE(matcher.MatchAndExplain([]() { throw 10; }, &listener));
  EXPECT_THAT(listener.str(),
              HasSubstr("throws an exception of an unknown type"));
}

TEST(ThrowsTest, FailNoThrow) {
  Matcher<std::function<void()>> matcher = Throws<std::runtime_error>();
  StringMatchResultListener listener;
  EXPECT_FALSE(matcher.MatchAndExplain([]() { (void)0; }, &listener));
  EXPECT_THAT(listener.str(), HasSubstr("does not throw any exception"));
}

class ThrowsPredicateTest
    : public TestWithParam<Matcher<std::function<void()>>> {};

TEST_P(ThrowsPredicateTest, Describe) {
  Matcher<std::function<void()>> matcher = GetParam();
  std::stringstream ss;
  matcher.DescribeTo(&ss);
  auto explanation = ss.str();
  EXPECT_THAT(explanation, HasSubstr("std::runtime_error"));
  EXPECT_THAT(explanation, HasSubstr("error message"));
}

TEST_P(ThrowsPredicateTest, Success) {
  Matcher<std::function<void()>> matcher = GetParam();
  StringMatchResultListener listener;
  EXPECT_TRUE(matcher.MatchAndExplain(
      []() { throw std::runtime_error("error message"); }, &listener));
  EXPECT_THAT(listener.str(), HasSubstr("std::runtime_error"));
}

TEST_P(ThrowsPredicateTest, FailWrongType) {
  Matcher<std::function<void()>> matcher = GetParam();
  StringMatchResultListener listener;
  EXPECT_FALSE(matcher.MatchAndExplain(
      []() { throw std::logic_error("error message"); }, &listener));
  EXPECT_THAT(listener.str(), HasSubstr("std::logic_error"));
  EXPECT_THAT(listener.str(), HasSubstr("\"error message\""));
}

TEST_P(ThrowsPredicateTest, FailWrongTypeNonStd) {
  Matcher<std::function<void()>> matcher = GetParam();
  StringMatchResultListener listener;
  EXPECT_FALSE(matcher.MatchAndExplain([]() { throw 10; }, &listener));
  EXPECT_THAT(listener.str(),
              HasSubstr("throws an exception of an unknown type"));
}

TEST_P(ThrowsPredicateTest, FailWrongMessage) {
  Matcher<std::function<void()>> matcher = GetParam();
  StringMatchResultListener listener;
  EXPECT_FALSE(matcher.MatchAndExplain(
      []() { throw std::runtime_error("wrong message"); }, &listener));
  EXPECT_THAT(listener.str(), HasSubstr("std::runtime_error"));
  EXPECT_THAT(listener.str(), Not(HasSubstr("wrong message")));
}

TEST_P(ThrowsPredicateTest, FailNoThrow) {
  Matcher<std::function<void()>> matcher = GetParam();
  StringMatchResultListener listener;
  EXPECT_FALSE(matcher.MatchAndExplain([]() {}, &listener));
  EXPECT_THAT(listener.str(), HasSubstr("does not throw any exception"));
}

INSTANTIATE_TEST_SUITE_P(
    AllMessagePredicates, ThrowsPredicateTest,
    Values(Matcher<std::function<void()>>(
        ThrowsMessage<std::runtime_error>(HasSubstr("error message")))));

// Tests that Throws<E1>(Matcher<E2>{}) compiles even when E2 != const E1&.
TEST(ThrowsPredicateCompilesTest, ExceptionMatcherAcceptsBroadType) {
  {
    Matcher<std::function<void()>> matcher =
        ThrowsMessage<std::runtime_error>(HasSubstr("error message"));
    EXPECT_TRUE(
        matcher.Matches([]() { throw std::runtime_error("error message"); }));
    EXPECT_FALSE(
        matcher.Matches([]() { throw std::runtime_error("wrong message"); }));
  }

  {
    Matcher<uint64_t> inner = Eq(10);
    Matcher<std::function<void()>> matcher = Throws<uint32_t>(inner);
    EXPECT_TRUE(matcher.Matches([]() { throw(uint32_t) 10; }));
    EXPECT_FALSE(matcher.Matches([]() { throw(uint32_t) 11; }));
  }
}

// Tests that ThrowsMessage("message") is equivalent
// to ThrowsMessage(Eq<std::string>("message")).
TEST(ThrowsPredicateCompilesTest, MessageMatcherAcceptsNonMatcher) {
  Matcher<std::function<void()>> matcher =
      ThrowsMessage<std::runtime_error>("error message");
  EXPECT_TRUE(
      matcher.Matches([]() { throw std::runtime_error("error message"); }));
  EXPECT_FALSE(matcher.Matches(
      []() { throw std::runtime_error("wrong error message"); }));
}

#endif  // GTEST_HAS_EXCEPTIONS

}  // namespace
}  // namespace gmock_matchers_2_test
}  // namespace testing

#ifdef _MSC_VER
# pragma warning(pop)
#endif
