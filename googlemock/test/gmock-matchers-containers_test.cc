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
#pragma warning(push)
#pragma warning(disable : 4244)
#pragma warning(disable : 4100)
#endif

#include "test/gmock-matchers_test.h"

namespace testing {
namespace gmock_matchers_test {
namespace {

std::vector<std::unique_ptr<int>> MakeUniquePtrs(const std::vector<int>& ints) {
  std::vector<std::unique_ptr<int>> pointers;
  for (int i : ints) pointers.emplace_back(new int(i));
  return pointers;
}

std::string OfType(const std::string& type_name) {
#if GTEST_HAS_RTTI
  return IsReadableTypeName(type_name) ? " (of type " + type_name + ")" : "";
#else
  return "";
#endif
}

TEST(ContainsTest, WorksWithMoveOnly) {
  ContainerHelper helper;
  EXPECT_CALL(helper, Call(Contains(Pointee(2))));
  helper.Call(MakeUniquePtrs({1, 2}));
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
                       "  Actual: 5" +
                           OfType("unsigned short"));
  n = 0;
  EXPECT_NONFATAL_FAILURE(EXPECT_THAT(n, AllOf(Le(7), Ge(5))),
                          "Value of: n\n"
                          "Expected: (is <= 7) and (is >= 5)\n"
                          "  Actual: 0" +
                              OfType("unsigned short"));
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
                          "  Actual: 5" +
                              OfType("int"));
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
  const Matcher<int* const&> m = Pointee(Ge(0));

  int n = 1;
  EXPECT_TRUE(m.Matches(&n));
  n = -1;
  EXPECT_FALSE(m.Matches(&n));
  EXPECT_FALSE(m.Matches(nullptr));
}

TEST(PointeeTest, ReferenceToNonConstRawPointer) {
  const Matcher<double*&> m = Pointee(Ge(0));

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
  const Matcher<ConstPropagatingPtr<int>> m = Pointee(Lt(5));
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
  EXPECT_EQ("does not point to a value that is > 3", DescribeNegation(m));
}

TEST(PointeeTest, CanExplainMatchResult) {
  const Matcher<const std::string*> m = Pointee(StartsWith("Hi"));

  EXPECT_EQ("", Explain(m, static_cast<const std::string*>(nullptr)));

  const Matcher<long*> m2 = Pointee(GreaterThan(1));  // NOLINT
  long n = 3;                                         // NOLINT
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
  Matcher<const AStruct&> m = Field(&AStruct::x, Matcher<signed char>(Ge(0)));

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
                ", which is 1 more than 0",
            Explain(m, &a));
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

// A user-defined class for testing Property().
class AClass {
 public:
  AClass() : n_(0) {}

  // A getter that returns a non-reference.
  int n() const { return n_; }

  void set_n(int new_n) { n_ = new_n; }

  // A getter that returns a reference to const.
  const std::string& s() const { return s_; }

  const std::string& s_ref() const& { return s_; }

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
  Matcher<const AClass&> m = Property(&AClass::n, Matcher<signed char>(Ge(0)));

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

  EXPECT_EQ(
      "is mapped by the given callable to a value that "
      "is equal to \"foo\"",
      Describe(matcher));
  EXPECT_EQ(
      "is mapped by the given callable to a value that "
      "isn't equal to \"foo\"",
      DescribeNegation(matcher));
}

// Tests that ResultOf() can describe itself when provided a result description.
TEST(ResultOfTest, CanDescribeItselfWithResultDescription) {
  Matcher<int> matcher =
      ResultOf("string conversion", &IntToStringFunction, StrEq("foo"));

  EXPECT_EQ("whose string conversion is equal to \"foo\"", Describe(matcher));
  EXPECT_EQ("whose string conversion isn't equal to \"foo\"",
            DescribeNegation(matcher));
}

// Tests that ResultOf() can explain the match result.
int IntFunction(int input) { return input == 42 ? 80 : 90; }

TEST(ResultOfTest, CanExplainMatchResult) {
  Matcher<int> matcher = ResultOf(&IntFunction, Ge(85));
  EXPECT_EQ("which is mapped by the given callable to 90" + OfType("int"),
            Explain(matcher, 36));

  matcher = ResultOf(&IntFunction, GreaterThan(85));
  EXPECT_EQ("which is mapped by the given callable to 90" + OfType("int") +
                ", which is 5 more than 85",
            Explain(matcher, 36));
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
  Matcher<Uncopyable&> matcher2 = ResultOf(&RefUncopyableFunction, Ref(obj));

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
  std::string operator()(int input) const { return IntToStringFunction(input); }
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
  std::string operator()(int* p) { return p ? "good ptr" : "null"; }
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
  Matcher<vector<int>> m = SizeIs(2);
  EXPECT_EQ("size is equal to 2", Describe(m));
  EXPECT_EQ("size isn't equal to 2", DescribeNegation(m));
}

TEST(SizeIsTest, ExplainsResult) {
  Matcher<vector<int>> m1 = SizeIs(2);
  Matcher<vector<int>> m2 = SizeIs(Lt(2u));
  Matcher<vector<int>> m3 = SizeIs(AnyOf(0, 3));
  Matcher<vector<int>> m4 = SizeIs(Gt(1u));
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
  EXPECT_THAT(numbers,
              WhenSortedBy(greater<unsigned>(), ElementsAre(3, 2, 2, 1)));
  EXPECT_THAT(numbers,
              Not(WhenSortedBy(greater<unsigned>(), ElementsAre(1, 2, 2, 3))));
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
  EXPECT_THAT(numbers,
              WhenSortedBy(less<int>(), ElementsAreArray(sorted_numbers)));
  EXPECT_THAT(numbers, Not(WhenSortedBy(less<int>(), ElementsAre(1, 3, 2, 4))));
}

TEST(WhenSortedByTest, CanDescribeSelf) {
  const Matcher<vector<int>> m = WhenSortedBy(less<int>(), ElementsAre(1, 2));
  EXPECT_EQ(
      "(when sorted) has 2 elements where\n"
      "element #0 is equal to 1,\n"
      "element #1 is equal to 2",
      Describe(m));
  EXPECT_EQ(
      "(when sorted) doesn't have 2 elements, or\n"
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
  EXPECT_THAT(ifib,
              WhenSorted(ElementsAre(Pair(1, 1), Pair(1, 2), Pair(2, 3),
                                     Pair(3, 4), Pair(5, 5), Pair(8, 6))));
  EXPECT_THAT(ifib,
              Not(WhenSorted(ElementsAre(Pair(8, 6), Pair(2, 3), Pair(1, 1),
                                         Pair(3, 4), Pair(1, 2), Pair(5, 5)))));
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
  const_iterator end() const { return const_iterator(this, remainder_.end()); }

 private:
  class ConstIter {
   public:
    using iterator_category = std::input_iterator_tag;
    using value_type = T;
    using difference_type = ptrdiff_t;
    using pointer = const value_type*;
    using reference = const value_type&;

    ConstIter(const Streamlike* s, typename std::list<value_type>::iterator pos)
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
  Matcher<vector<int>> m = BeginEndDistanceIs(2);
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
  Matcher<vector<int>> m1 = BeginEndDistanceIs(2);
  Matcher<vector<int>> m2 = BeginEndDistanceIs(Lt(2));
  Matcher<vector<int>> m3 = BeginEndDistanceIs(AnyOf(0, 3));
  Matcher<vector<int>> m4 = BeginEndDistanceIs(GreaterThan(1));
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
    EXPECT_TRUE(ExplainMatchResult(UnorderedElementsAreArray(a), s, &listener))
        << listener.str();
  } while (std::next_permutation(s.begin(), s.end()));
}

TEST(UnorderedElementsAreArrayTest, VectorBool) {
  const bool a[] = {0, 1, 0, 1, 1};
  const bool b[] = {1, 0, 1, 1, 0};
  std::vector<bool> expected(std::begin(a), std::end(a));
  std::vector<bool> actual(std::begin(b), std::end(b));
  StringMatchResultListener listener;
  EXPECT_TRUE(ExplainMatchResult(UnorderedElementsAreArray(expected), actual,
                                 &listener))
      << listener.str();
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
  EXPECT_THAT(a,
              UnorderedElementsAreArray({Eq(1), Eq(2), Eq(3), Eq(4), Eq(5)}));
  EXPECT_THAT(
      a, Not(UnorderedElementsAreArray({Eq(1), Eq(2), Eq(3), Eq(4), Eq(6)})));
}

TEST(UnorderedElementsAreArrayTest,
     TakesInitializerListOfDifferentTypedMatchers) {
  const int a[5] = {2, 1, 4, 5, 3};
  // The compiler cannot infer the type of the initializer list if its
  // elements have different types.  We must explicitly specify the
  // unified element type in this case.
  EXPECT_THAT(a, UnorderedElementsAreArray<Matcher<int>>(
                     {Eq(1), Ne(-2), Ge(3), Le(4), Eq(5)}));
  EXPECT_THAT(a, Not(UnorderedElementsAreArray<Matcher<int>>(
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
    EXPECT_TRUE(ExplainMatchResult(UnorderedElementsAre(1, 2, 3), s, &listener))
        << listener.str();
  } while (std::next_permutation(s.begin(), s.end()));
}

TEST_F(UnorderedElementsAreTest, FailsWhenAnElementMatchesNoMatcher) {
  const int a[] = {1, 2, 3};
  std::vector<int> s(std::begin(a), std::end(a));
  std::vector<Matcher<int>> mv;
  mv.push_back(1);
  mv.push_back(2);
  mv.push_back(2);
  // The element with value '3' matches nothing: fail fast.
  StringMatchResultListener listener;
  EXPECT_FALSE(ExplainMatchResult(UnorderedElementsAreArray(mv), s, &listener))
      << listener.str();
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
  std::vector<Matcher<int>> mv;
  for (int i = 0; i < 100; ++i) {
    s.push_back(i);
    mv.push_back(_);
  }
  mv[50] = Eq(0);
  StringMatchResultListener listener;
  EXPECT_TRUE(ExplainMatchResult(UnorderedElementsAreArray(mv), s, &listener))
      << listener.str();
}

// Another variant of 'Performance' with similar expectations.
// [ RUN      ] UnorderedElementsAreTest.PerformanceHalfStrict
// [       OK ] UnorderedElementsAreTest.PerformanceHalfStrict (4 ms)
TEST_F(UnorderedElementsAreTest, PerformanceHalfStrict) {
  std::vector<int> s;
  std::vector<Matcher<int>> mv;
  for (int i = 0; i < 100; ++i) {
    s.push_back(i);
    if (i & 1) {
      mv.push_back(_);
    } else {
      mv.push_back(i);
    }
  }
  StringMatchResultListener listener;
  EXPECT_TRUE(ExplainMatchResult(UnorderedElementsAreArray(mv), s, &listener))
      << listener.str();
}

TEST_F(UnorderedElementsAreTest, FailMessageCountWrong) {
  std::vector<int> v;
  v.push_back(4);
  StringMatchResultListener listener;
  EXPECT_FALSE(ExplainMatchResult(UnorderedElementsAre(1, 2, 3), v, &listener))
      << listener.str();
  EXPECT_THAT(listener.str(), Eq("which has 1 element"));
}

TEST_F(UnorderedElementsAreTest, FailMessageCountWrongZero) {
  std::vector<int> v;
  StringMatchResultListener listener;
  EXPECT_FALSE(ExplainMatchResult(UnorderedElementsAre(1, 2, 3), v, &listener))
      << listener.str();
  EXPECT_THAT(listener.str(), Eq(""));
}

TEST_F(UnorderedElementsAreTest, FailMessageUnmatchedMatchers) {
  std::vector<int> v;
  v.push_back(1);
  v.push_back(1);
  StringMatchResultListener listener;
  EXPECT_FALSE(ExplainMatchResult(UnorderedElementsAre(1, 2), v, &listener))
      << listener.str();
  EXPECT_THAT(listener.str(),
              Eq("where the following matchers don't match any elements:\n"
                 "matcher #1: is equal to 2"));
}

TEST_F(UnorderedElementsAreTest, FailMessageUnmatchedElements) {
  std::vector<int> v;
  v.push_back(1);
  v.push_back(2);
  StringMatchResultListener listener;
  EXPECT_FALSE(ExplainMatchResult(UnorderedElementsAre(1, 1), v, &listener))
      << listener.str();
  EXPECT_THAT(listener.str(),
              Eq("where the following elements don't match any matchers:\n"
                 "element #1: 2"));
}

TEST_F(UnorderedElementsAreTest, FailMessageUnmatchedMatcherAndElement) {
  std::vector<int> v;
  v.push_back(2);
  v.push_back(3);
  StringMatchResultListener listener;
  EXPECT_FALSE(ExplainMatchResult(UnorderedElementsAre(1, 2), v, &listener))
      << listener.str();
  EXPECT_THAT(listener.str(),
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
      AnyOf(
          prefix + "{\n  " + EMString(0, 0) + ",\n  " + EMString(1, 2) + "\n}",
          prefix + "{\n  " + EMString(0, 1) + ",\n  " + EMString(1, 2) + "\n}",
          prefix + "{\n  " + EMString(0, 0) + ",\n  " + EMString(2, 2) + "\n}",
          prefix + "{\n  " + EMString(0, 1) + ",\n  " + EMString(2, 2) +
              "\n}"));
}

TEST_F(UnorderedElementsAreTest, Describe) {
  EXPECT_THAT(Describe<IntVec>(UnorderedElementsAre()), Eq("is empty"));
  EXPECT_THAT(Describe<IntVec>(UnorderedElementsAre(345)),
              Eq("has 1 element and that element is equal to 345"));
  EXPECT_THAT(Describe<IntVec>(UnorderedElementsAre(111, 222, 333)),
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
  EXPECT_THAT(DescribeNegation<IntVec>(UnorderedElementsAre(123, 234, 345)),
              Eq("doesn't have 3 elements, or there exists no permutation "
                 "of elements such that:\n"
                 " - element #0 is equal to 123, and\n"
                 " - element #1 is equal to 234, and\n"
                 " - element #2 is equal to 345"));
}

// Tests Each().

TEST(EachTest, ExplainsMatchResultCorrectly) {
  set<int> a;  // empty

  Matcher<set<int>> m = Each(2);
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
  Matcher<vector<int>> m = Each(1);
  EXPECT_EQ("only contains elements that is equal to 1", Describe(m));

  Matcher<vector<int>> m2 = Not(m);
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
  EXPECT_EQ(
      "contains 3 values, where each value and its corresponding value "
      "in { 1, 2, 3 } are a pair where the first is half of the second",
      Describe(m));
  EXPECT_EQ(
      "doesn't contain exactly 3 values, or contains a value x at some "
      "index i where x and the i-th value of { 1, 2, 3 } are a pair "
      "where the first isn't half of the second",
      DescribeNegation(m));
}

TEST(PointwiseTest, MakesCopyOfRhs) {
  list<signed char> rhs;
  rhs.push_back(2);
  rhs.push_back(4);

  int lhs[] = {1, 2};
  const Matcher<const int(&)[2]> m = Pointwise(IsHalfOf(), rhs);
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
  EXPECT_EQ("which contains 2 values", Explain(Pointwise(Gt(), rhs), lhs));

  const int rhs2[3] = {0, 1, 2};
  EXPECT_THAT(lhs, Not(Pointwise(Gt(), rhs2)));
}

TEST(PointwiseTest, RejectsWrongContent) {
  const double lhs[3] = {1, 2, 3};
  const int rhs[3] = {2, 6, 4};
  EXPECT_THAT(lhs, Not(Pointwise(IsHalfOf(), rhs)));
  EXPECT_EQ(
      "where the value pair (2, 6) at index #1 don't match, "
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
  const Matcher<const int(&)[2]> m = UnorderedPointwise(IsHalfOf(), rhs);
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
  EXPECT_EQ(
      "where the following elements don't match any matchers:\n"
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

TEST(PointeeTest, WorksOnMoveOnlyType) {
  std::unique_ptr<int> p(new int(3));
  EXPECT_THAT(p, Pointee(Eq(3)));
  EXPECT_THAT(p, Not(Pointee(Eq(2))));
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

TEST(ElementsAreTest, CanDescribeNegationOfExpectingOneElement) {
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
// implementation, we only do a test for native arrays here.
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

// Tests Contains().

TEST(ContainsTest, ListMatchesWhenElementIsInContainer) {
  list<int> some_list;
  some_list.push_back(3);
  some_list.push_back(1);
  some_list.push_back(2);
  some_list.push_back(3);
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

}  // namespace
}  // namespace gmock_matchers_test
}  // namespace testing

#ifdef _MSC_VER
#pragma warning(pop)
#endif
