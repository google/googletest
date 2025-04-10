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
// This file tests the built-in actions.

#include "gmock/gmock-actions.h"

#include <algorithm>
#include <functional>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "gmock/gmock.h"
#include "gmock/internal/gmock-port.h"
#include "gtest/gtest-spi.h"
#include "gtest/gtest.h"
#include "gtest/internal/gtest-port.h"

// Silence C4100 (unreferenced formal parameter) and C4503 (decorated name
// length exceeded) for MSVC.
GTEST_DISABLE_MSC_WARNINGS_PUSH_(4100 4503)
#if defined(_MSC_VER) && (_MSC_VER == 1900)
// and silence C4800 (C4800: 'int *const ': forcing value
// to bool 'true' or 'false') for MSVC 15
GTEST_DISABLE_MSC_WARNINGS_PUSH_(4800)
#endif

namespace testing {
namespace {

using ::testing::internal::BuiltInDefaultValue;

TEST(TypeTraits, Negation) {
  // Direct use with std types.
  static_assert(std::is_base_of<std::false_type,
                                internal::negation<std::true_type>>::value,
                "");

  static_assert(std::is_base_of<std::true_type,
                                internal::negation<std::false_type>>::value,
                "");

  // With other types that fit the requirement of a value member that is
  // convertible to bool.
  static_assert(std::is_base_of<
                    std::true_type,
                    internal::negation<std::integral_constant<int, 0>>>::value,
                "");

  static_assert(std::is_base_of<
                    std::false_type,
                    internal::negation<std::integral_constant<int, 1>>>::value,
                "");

  static_assert(std::is_base_of<
                    std::false_type,
                    internal::negation<std::integral_constant<int, -1>>>::value,
                "");
}

// Weird false/true types that aren't actually bool constants (but should still
// be legal according to [meta.logical] because `bool(T::value)` is valid), are
// distinct from std::false_type and std::true_type, and are distinct from other
// instantiations of the same template.
//
// These let us check finicky details mandated by the standard like
// "std::conjunction should evaluate to a type that inherits from the first
// false-y input".
template <int>
struct MyFalse : std::integral_constant<int, 0> {};

template <int>
struct MyTrue : std::integral_constant<int, -1> {};

TEST(TypeTraits, Conjunction) {
  // Base case: always true.
  static_assert(std::is_base_of<std::true_type, internal::conjunction<>>::value,
                "");

  // One predicate: inherits from that predicate, regardless of value.
  static_assert(
      std::is_base_of<MyFalse<0>, internal::conjunction<MyFalse<0>>>::value,
      "");

  static_assert(
      std::is_base_of<MyTrue<0>, internal::conjunction<MyTrue<0>>>::value, "");

  // Multiple predicates, with at least one false: inherits from that one.
  static_assert(
      std::is_base_of<MyFalse<1>, internal::conjunction<MyTrue<0>, MyFalse<1>,
                                                        MyTrue<2>>>::value,
      "");

  static_assert(
      std::is_base_of<MyFalse<1>, internal::conjunction<MyTrue<0>, MyFalse<1>,
                                                        MyFalse<2>>>::value,
      "");

  // Short circuiting: in the case above, additional predicates need not even
  // define a value member.
  struct Empty {};
  static_assert(
      std::is_base_of<MyFalse<1>, internal::conjunction<MyTrue<0>, MyFalse<1>,
                                                        Empty>>::value,
      "");

  // All predicates true: inherits from the last.
  static_assert(
      std::is_base_of<MyTrue<2>, internal::conjunction<MyTrue<0>, MyTrue<1>,
                                                       MyTrue<2>>>::value,
      "");
}

TEST(TypeTraits, Disjunction) {
  // Base case: always false.
  static_assert(
      std::is_base_of<std::false_type, internal::disjunction<>>::value, "");

  // One predicate: inherits from that predicate, regardless of value.
  static_assert(
      std::is_base_of<MyFalse<0>, internal::disjunction<MyFalse<0>>>::value,
      "");

  static_assert(
      std::is_base_of<MyTrue<0>, internal::disjunction<MyTrue<0>>>::value, "");

  // Multiple predicates, with at least one true: inherits from that one.
  static_assert(
      std::is_base_of<MyTrue<1>, internal::disjunction<MyFalse<0>, MyTrue<1>,
                                                       MyFalse<2>>>::value,
      "");

  static_assert(
      std::is_base_of<MyTrue<1>, internal::disjunction<MyFalse<0>, MyTrue<1>,
                                                       MyTrue<2>>>::value,
      "");

  // Short circuiting: in the case above, additional predicates need not even
  // define a value member.
  struct Empty {};
  static_assert(
      std::is_base_of<MyTrue<1>, internal::disjunction<MyFalse<0>, MyTrue<1>,
                                                       Empty>>::value,
      "");

  // All predicates false: inherits from the last.
  static_assert(
      std::is_base_of<MyFalse<2>, internal::disjunction<MyFalse<0>, MyFalse<1>,
                                                        MyFalse<2>>>::value,
      "");
}

TEST(TypeTraits, IsInvocableRV) {
  struct C {
    int operator()() const { return 0; }
    void operator()(int) & {}
    std::string operator()(int) && { return ""; };
  };

  // The first overload is callable for const and non-const rvalues and lvalues.
  // It can be used to obtain an int, cv void, or anything int is convertible
  // to.
  static_assert(internal::is_callable_r<int, C>::value, "");
  static_assert(internal::is_callable_r<int, C&>::value, "");
  static_assert(internal::is_callable_r<int, const C>::value, "");
  static_assert(internal::is_callable_r<int, const C&>::value, "");

  static_assert(internal::is_callable_r<void, C>::value, "");
  static_assert(internal::is_callable_r<const volatile void, C>::value, "");
  static_assert(internal::is_callable_r<char, C>::value, "");

  // It's possible to provide an int. If it's given to an lvalue, the result is
  // void. Otherwise it is std::string (which is also treated as allowed for a
  // void result type).
  static_assert(internal::is_callable_r<void, C&, int>::value, "");
  static_assert(!internal::is_callable_r<int, C&, int>::value, "");
  static_assert(!internal::is_callable_r<std::string, C&, int>::value, "");
  static_assert(!internal::is_callable_r<void, const C&, int>::value, "");

  static_assert(internal::is_callable_r<std::string, C, int>::value, "");
  static_assert(internal::is_callable_r<void, C, int>::value, "");
  static_assert(!internal::is_callable_r<int, C, int>::value, "");

  // It's not possible to provide other arguments.
  static_assert(!internal::is_callable_r<void, C, std::string>::value, "");
  static_assert(!internal::is_callable_r<void, C, int, int>::value, "");

  // In C++17 and above, where it's guaranteed that functions can return
  // non-moveable objects, everything should work fine for non-moveable rsult
  // types too.
  // TODO(b/396121064) - Fix this test under MSVC
#ifndef _MSC_VER
  {
    struct NonMoveable {
      NonMoveable() = default;
      NonMoveable(NonMoveable&&) = delete;
    };

    static_assert(!std::is_move_constructible_v<NonMoveable>);

    struct Callable {
      NonMoveable operator()() { return NonMoveable(); }
    };

    static_assert(internal::is_callable_r<NonMoveable, Callable>::value);
    static_assert(internal::is_callable_r<void, Callable>::value);
    static_assert(
        internal::is_callable_r<const volatile void, Callable>::value);

    static_assert(!internal::is_callable_r<int, Callable>::value);
    static_assert(!internal::is_callable_r<NonMoveable, Callable, int>::value);
  }
#endif  // _MSC_VER

  // Nothing should choke when we try to call other arguments besides directly
  // callable objects, but they should not show up as callable.
  static_assert(!internal::is_callable_r<void, int>::value, "");
  static_assert(!internal::is_callable_r<void, void (C::*)()>::value, "");
  static_assert(!internal::is_callable_r<void, void (C::*)(), C*>::value, "");
}

// Tests that BuiltInDefaultValue<T*>::Get() returns NULL.
TEST(BuiltInDefaultValueTest, IsNullForPointerTypes) {
  EXPECT_TRUE(BuiltInDefaultValue<int*>::Get() == nullptr);
  EXPECT_TRUE(BuiltInDefaultValue<const char*>::Get() == nullptr);
  EXPECT_TRUE(BuiltInDefaultValue<void*>::Get() == nullptr);
}

// Tests that BuiltInDefaultValue<T*>::Exists() return true.
TEST(BuiltInDefaultValueTest, ExistsForPointerTypes) {
  EXPECT_TRUE(BuiltInDefaultValue<int*>::Exists());
  EXPECT_TRUE(BuiltInDefaultValue<const char*>::Exists());
  EXPECT_TRUE(BuiltInDefaultValue<void*>::Exists());
}

// Tests that BuiltInDefaultValue<T>::Get() returns 0 when T is a
// built-in numeric type.
TEST(BuiltInDefaultValueTest, IsZeroForNumericTypes) {
  EXPECT_EQ(0U, BuiltInDefaultValue<unsigned char>::Get());
  EXPECT_EQ(0, BuiltInDefaultValue<signed char>::Get());
  EXPECT_EQ(0, BuiltInDefaultValue<char>::Get());
#if GMOCK_WCHAR_T_IS_NATIVE_
#if !defined(__WCHAR_UNSIGNED__)
  EXPECT_EQ(0, BuiltInDefaultValue<wchar_t>::Get());
#else
  EXPECT_EQ(0U, BuiltInDefaultValue<wchar_t>::Get());
#endif
#endif
  EXPECT_EQ(0U, BuiltInDefaultValue<unsigned short>::Get());  // NOLINT
  EXPECT_EQ(0, BuiltInDefaultValue<signed short>::Get());     // NOLINT
  EXPECT_EQ(0, BuiltInDefaultValue<short>::Get());            // NOLINT
  EXPECT_EQ(0U, BuiltInDefaultValue<unsigned int>::Get());
  EXPECT_EQ(0, BuiltInDefaultValue<signed int>::Get());
  EXPECT_EQ(0, BuiltInDefaultValue<int>::Get());
  EXPECT_EQ(0U, BuiltInDefaultValue<unsigned long>::Get());       // NOLINT
  EXPECT_EQ(0, BuiltInDefaultValue<signed long>::Get());          // NOLINT
  EXPECT_EQ(0, BuiltInDefaultValue<long>::Get());                 // NOLINT
  EXPECT_EQ(0U, BuiltInDefaultValue<unsigned long long>::Get());  // NOLINT
  EXPECT_EQ(0, BuiltInDefaultValue<signed long long>::Get());     // NOLINT
  EXPECT_EQ(0, BuiltInDefaultValue<long long>::Get());            // NOLINT
  EXPECT_EQ(0, BuiltInDefaultValue<float>::Get());
  EXPECT_EQ(0, BuiltInDefaultValue<double>::Get());
}

// Tests that BuiltInDefaultValue<T>::Exists() returns true when T is a
// built-in numeric type.
TEST(BuiltInDefaultValueTest, ExistsForNumericTypes) {
  EXPECT_TRUE(BuiltInDefaultValue<unsigned char>::Exists());
  EXPECT_TRUE(BuiltInDefaultValue<signed char>::Exists());
  EXPECT_TRUE(BuiltInDefaultValue<char>::Exists());
#if GMOCK_WCHAR_T_IS_NATIVE_
  EXPECT_TRUE(BuiltInDefaultValue<wchar_t>::Exists());
#endif
  EXPECT_TRUE(BuiltInDefaultValue<unsigned short>::Exists());  // NOLINT
  EXPECT_TRUE(BuiltInDefaultValue<signed short>::Exists());    // NOLINT
  EXPECT_TRUE(BuiltInDefaultValue<short>::Exists());           // NOLINT
  EXPECT_TRUE(BuiltInDefaultValue<unsigned int>::Exists());
  EXPECT_TRUE(BuiltInDefaultValue<signed int>::Exists());
  EXPECT_TRUE(BuiltInDefaultValue<int>::Exists());
  EXPECT_TRUE(BuiltInDefaultValue<unsigned long>::Exists());       // NOLINT
  EXPECT_TRUE(BuiltInDefaultValue<signed long>::Exists());         // NOLINT
  EXPECT_TRUE(BuiltInDefaultValue<long>::Exists());                // NOLINT
  EXPECT_TRUE(BuiltInDefaultValue<unsigned long long>::Exists());  // NOLINT
  EXPECT_TRUE(BuiltInDefaultValue<signed long long>::Exists());    // NOLINT
  EXPECT_TRUE(BuiltInDefaultValue<long long>::Exists());           // NOLINT
  EXPECT_TRUE(BuiltInDefaultValue<float>::Exists());
  EXPECT_TRUE(BuiltInDefaultValue<double>::Exists());
}

// Tests that BuiltInDefaultValue<bool>::Get() returns false.
TEST(BuiltInDefaultValueTest, IsFalseForBool) {
  EXPECT_FALSE(BuiltInDefaultValue<bool>::Get());
}

// Tests that BuiltInDefaultValue<bool>::Exists() returns true.
TEST(BuiltInDefaultValueTest, BoolExists) {
  EXPECT_TRUE(BuiltInDefaultValue<bool>::Exists());
}

// Tests that BuiltInDefaultValue<T>::Get() returns "" when T is a
// string type.
TEST(BuiltInDefaultValueTest, IsEmptyStringForString) {
  EXPECT_EQ("", BuiltInDefaultValue<::std::string>::Get());
}

// Tests that BuiltInDefaultValue<T>::Exists() returns true when T is a
// string type.
TEST(BuiltInDefaultValueTest, ExistsForString) {
  EXPECT_TRUE(BuiltInDefaultValue<::std::string>::Exists());
}

// Tests that BuiltInDefaultValue<const T>::Get() returns the same
// value as BuiltInDefaultValue<T>::Get() does.
TEST(BuiltInDefaultValueTest, WorksForConstTypes) {
  EXPECT_EQ("", BuiltInDefaultValue<const std::string>::Get());
  EXPECT_EQ(0, BuiltInDefaultValue<const int>::Get());
  EXPECT_TRUE(BuiltInDefaultValue<char* const>::Get() == nullptr);
  EXPECT_FALSE(BuiltInDefaultValue<const bool>::Get());
}

// A type that's default constructible.
class MyDefaultConstructible {
 public:
  MyDefaultConstructible() : value_(42) {}

  int value() const { return value_; }

 private:
  int value_;
};

// A type that's not default constructible.
class MyNonDefaultConstructible {
 public:
  // Does not have a default ctor.
  explicit MyNonDefaultConstructible(int a_value) : value_(a_value) {}

  int value() const { return value_; }

 private:
  int value_;
};

TEST(BuiltInDefaultValueTest, ExistsForDefaultConstructibleType) {
  EXPECT_TRUE(BuiltInDefaultValue<MyDefaultConstructible>::Exists());
}

TEST(BuiltInDefaultValueTest, IsDefaultConstructedForDefaultConstructibleType) {
  EXPECT_EQ(42, BuiltInDefaultValue<MyDefaultConstructible>::Get().value());
}

TEST(BuiltInDefaultValueTest, DoesNotExistForNonDefaultConstructibleType) {
  EXPECT_FALSE(BuiltInDefaultValue<MyNonDefaultConstructible>::Exists());
}

// Tests that BuiltInDefaultValue<T&>::Get() aborts the program.
TEST(BuiltInDefaultValueDeathTest, IsUndefinedForReferences) {
  EXPECT_DEATH_IF_SUPPORTED({ BuiltInDefaultValue<int&>::Get(); }, "");
  EXPECT_DEATH_IF_SUPPORTED({ BuiltInDefaultValue<const char&>::Get(); }, "");
}

TEST(BuiltInDefaultValueDeathTest, IsUndefinedForNonDefaultConstructibleType) {
  EXPECT_DEATH_IF_SUPPORTED(
      { BuiltInDefaultValue<MyNonDefaultConstructible>::Get(); }, "");
}

// Tests that DefaultValue<T>::IsSet() is false initially.
TEST(DefaultValueTest, IsInitiallyUnset) {
  EXPECT_FALSE(DefaultValue<int>::IsSet());
  EXPECT_FALSE(DefaultValue<MyDefaultConstructible>::IsSet());
  EXPECT_FALSE(DefaultValue<const MyNonDefaultConstructible>::IsSet());
}

// Tests that DefaultValue<T> can be set and then unset.
TEST(DefaultValueTest, CanBeSetAndUnset) {
  EXPECT_TRUE(DefaultValue<int>::Exists());
  EXPECT_FALSE(DefaultValue<const MyNonDefaultConstructible>::Exists());

  DefaultValue<int>::Set(1);
  DefaultValue<const MyNonDefaultConstructible>::Set(
      MyNonDefaultConstructible(42));

  EXPECT_EQ(1, DefaultValue<int>::Get());
  EXPECT_EQ(42, DefaultValue<const MyNonDefaultConstructible>::Get().value());

  EXPECT_TRUE(DefaultValue<int>::Exists());
  EXPECT_TRUE(DefaultValue<const MyNonDefaultConstructible>::Exists());

  DefaultValue<int>::Clear();
  DefaultValue<const MyNonDefaultConstructible>::Clear();

  EXPECT_FALSE(DefaultValue<int>::IsSet());
  EXPECT_FALSE(DefaultValue<const MyNonDefaultConstructible>::IsSet());

  EXPECT_TRUE(DefaultValue<int>::Exists());
  EXPECT_FALSE(DefaultValue<const MyNonDefaultConstructible>::Exists());
}

// Tests that DefaultValue<T>::Get() returns the
// BuiltInDefaultValue<T>::Get() when DefaultValue<T>::IsSet() is
// false.
TEST(DefaultValueDeathTest, GetReturnsBuiltInDefaultValueWhenUnset) {
  EXPECT_FALSE(DefaultValue<int>::IsSet());
  EXPECT_TRUE(DefaultValue<int>::Exists());
  EXPECT_FALSE(DefaultValue<MyNonDefaultConstructible>::IsSet());
  EXPECT_FALSE(DefaultValue<MyNonDefaultConstructible>::Exists());

  EXPECT_EQ(0, DefaultValue<int>::Get());

  EXPECT_DEATH_IF_SUPPORTED(
      { DefaultValue<MyNonDefaultConstructible>::Get(); }, "");
}

TEST(DefaultValueTest, GetWorksForMoveOnlyIfSet) {
  EXPECT_TRUE(DefaultValue<std::unique_ptr<int>>::Exists());
  EXPECT_TRUE(DefaultValue<std::unique_ptr<int>>::Get() == nullptr);
  DefaultValue<std::unique_ptr<int>>::SetFactory(
      [] { return std::make_unique<int>(42); });
  EXPECT_TRUE(DefaultValue<std::unique_ptr<int>>::Exists());
  std::unique_ptr<int> i = DefaultValue<std::unique_ptr<int>>::Get();
  EXPECT_EQ(42, *i);
}

// Tests that DefaultValue<void>::Get() returns void.
TEST(DefaultValueTest, GetWorksForVoid) { return DefaultValue<void>::Get(); }

// Tests using DefaultValue with a reference type.

// Tests that DefaultValue<T&>::IsSet() is false initially.
TEST(DefaultValueOfReferenceTest, IsInitiallyUnset) {
  EXPECT_FALSE(DefaultValue<int&>::IsSet());
  EXPECT_FALSE(DefaultValue<MyDefaultConstructible&>::IsSet());
  EXPECT_FALSE(DefaultValue<MyNonDefaultConstructible&>::IsSet());
}

// Tests that DefaultValue<T&>::Exists is false initially.
TEST(DefaultValueOfReferenceTest, IsInitiallyNotExisting) {
  EXPECT_FALSE(DefaultValue<int&>::Exists());
  EXPECT_FALSE(DefaultValue<MyDefaultConstructible&>::Exists());
  EXPECT_FALSE(DefaultValue<MyNonDefaultConstructible&>::Exists());
}

// Tests that DefaultValue<T&> can be set and then unset.
TEST(DefaultValueOfReferenceTest, CanBeSetAndUnset) {
  int n = 1;
  DefaultValue<const int&>::Set(n);
  MyNonDefaultConstructible x(42);
  DefaultValue<MyNonDefaultConstructible&>::Set(x);

  EXPECT_TRUE(DefaultValue<const int&>::Exists());
  EXPECT_TRUE(DefaultValue<MyNonDefaultConstructible&>::Exists());

  EXPECT_EQ(&n, &(DefaultValue<const int&>::Get()));
  EXPECT_EQ(&x, &(DefaultValue<MyNonDefaultConstructible&>::Get()));

  DefaultValue<const int&>::Clear();
  DefaultValue<MyNonDefaultConstructible&>::Clear();

  EXPECT_FALSE(DefaultValue<const int&>::Exists());
  EXPECT_FALSE(DefaultValue<MyNonDefaultConstructible&>::Exists());

  EXPECT_FALSE(DefaultValue<const int&>::IsSet());
  EXPECT_FALSE(DefaultValue<MyNonDefaultConstructible&>::IsSet());
}

// Tests that DefaultValue<T&>::Get() returns the
// BuiltInDefaultValue<T&>::Get() when DefaultValue<T&>::IsSet() is
// false.
TEST(DefaultValueOfReferenceDeathTest, GetReturnsBuiltInDefaultValueWhenUnset) {
  EXPECT_FALSE(DefaultValue<int&>::IsSet());
  EXPECT_FALSE(DefaultValue<MyNonDefaultConstructible&>::IsSet());

  EXPECT_DEATH_IF_SUPPORTED({ DefaultValue<int&>::Get(); }, "");
  EXPECT_DEATH_IF_SUPPORTED(
      { DefaultValue<MyNonDefaultConstructible>::Get(); }, "");
}

// Tests that ActionInterface can be implemented by defining the
// Perform method.

typedef int MyGlobalFunction(bool, int);

class MyActionImpl : public ActionInterface<MyGlobalFunction> {
 public:
  int Perform(const std::tuple<bool, int>& args) override {
    return std::get<0>(args) ? std::get<1>(args) : 0;
  }
};

TEST(ActionInterfaceTest, CanBeImplementedByDefiningPerform) {
  MyActionImpl my_action_impl;
  (void)my_action_impl;
}

TEST(ActionInterfaceTest, MakeAction) {
  Action<MyGlobalFunction> action = MakeAction(new MyActionImpl);

  // When exercising the Perform() method of Action<F>, we must pass
  // it a tuple whose size and type are compatible with F's argument
  // types.  For example, if F is int(), then Perform() takes a
  // 0-tuple; if F is void(bool, int), then Perform() takes a
  // std::tuple<bool, int>, and so on.
  EXPECT_EQ(5, action.Perform(std::make_tuple(true, 5)));
}

// Tests that Action<F> can be constructed from a pointer to
// ActionInterface<F>.
TEST(ActionTest, CanBeConstructedFromActionInterface) {
  Action<MyGlobalFunction> action(new MyActionImpl);
}

// Tests that Action<F> delegates actual work to ActionInterface<F>.
TEST(ActionTest, DelegatesWorkToActionInterface) {
  const Action<MyGlobalFunction> action(new MyActionImpl);

  EXPECT_EQ(5, action.Perform(std::make_tuple(true, 5)));
  EXPECT_EQ(0, action.Perform(std::make_tuple(false, 1)));
}

// Tests that Action<F> can be copied.
TEST(ActionTest, IsCopyable) {
  Action<MyGlobalFunction> a1(new MyActionImpl);
  Action<MyGlobalFunction> a2(a1);  // Tests the copy constructor.

  // a1 should continue to work after being copied from.
  EXPECT_EQ(5, a1.Perform(std::make_tuple(true, 5)));
  EXPECT_EQ(0, a1.Perform(std::make_tuple(false, 1)));

  // a2 should work like the action it was copied from.
  EXPECT_EQ(5, a2.Perform(std::make_tuple(true, 5)));
  EXPECT_EQ(0, a2.Perform(std::make_tuple(false, 1)));

  a2 = a1;  // Tests the assignment operator.

  // a1 should continue to work after being copied from.
  EXPECT_EQ(5, a1.Perform(std::make_tuple(true, 5)));
  EXPECT_EQ(0, a1.Perform(std::make_tuple(false, 1)));

  // a2 should work like the action it was copied from.
  EXPECT_EQ(5, a2.Perform(std::make_tuple(true, 5)));
  EXPECT_EQ(0, a2.Perform(std::make_tuple(false, 1)));
}

// Tests that an Action<From> object can be converted to a
// compatible Action<To> object.

class IsNotZero : public ActionInterface<bool(int)> {  // NOLINT
 public:
  bool Perform(const std::tuple<int>& arg) override {
    return std::get<0>(arg) != 0;
  }
};

TEST(ActionTest, CanBeConvertedToOtherActionType) {
  const Action<bool(int)> a1(new IsNotZero);           // NOLINT
  const Action<int(char)> a2 = Action<int(char)>(a1);  // NOLINT
  EXPECT_EQ(1, a2.Perform(std::make_tuple('a')));
  EXPECT_EQ(0, a2.Perform(std::make_tuple('\0')));
}

// The following two classes are for testing MakePolymorphicAction().

// Implements a polymorphic action that returns the second of the
// arguments it receives.
class ReturnSecondArgumentAction {
 public:
  // We want to verify that MakePolymorphicAction() can work with a
  // polymorphic action whose Perform() method template is either
  // const or not.  This lets us verify the non-const case.
  template <typename Result, typename ArgumentTuple>
  Result Perform(const ArgumentTuple& args) {
    return std::get<1>(args);
  }
};

// Implements a polymorphic action that can be used in a nullary
// function to return 0.
class ReturnZeroFromNullaryFunctionAction {
 public:
  // For testing that MakePolymorphicAction() works when the
  // implementation class' Perform() method template takes only one
  // template parameter.
  //
  // We want to verify that MakePolymorphicAction() can work with a
  // polymorphic action whose Perform() method template is either
  // const or not.  This lets us verify the const case.
  template <typename Result>
  Result Perform(const std::tuple<>&) const {
    return 0;
  }
};

// These functions verify that MakePolymorphicAction() returns a
// PolymorphicAction<T> where T is the argument's type.

PolymorphicAction<ReturnSecondArgumentAction> ReturnSecondArgument() {
  return MakePolymorphicAction(ReturnSecondArgumentAction());
}

PolymorphicAction<ReturnZeroFromNullaryFunctionAction>
ReturnZeroFromNullaryFunction() {
  return MakePolymorphicAction(ReturnZeroFromNullaryFunctionAction());
}

// Tests that MakePolymorphicAction() turns a polymorphic action
// implementation class into a polymorphic action.
TEST(MakePolymorphicActionTest, ConstructsActionFromImpl) {
  Action<int(bool, int, double)> a1 = ReturnSecondArgument();  // NOLINT
  EXPECT_EQ(5, a1.Perform(std::make_tuple(false, 5, 2.0)));
}

// Tests that MakePolymorphicAction() works when the implementation
// class' Perform() method template has only one template parameter.
TEST(MakePolymorphicActionTest, WorksWhenPerformHasOneTemplateParameter) {
  Action<int()> a1 = ReturnZeroFromNullaryFunction();
  EXPECT_EQ(0, a1.Perform(std::make_tuple()));

  Action<void*()> a2 = ReturnZeroFromNullaryFunction();
  EXPECT_TRUE(a2.Perform(std::make_tuple()) == nullptr);
}

// Tests that Return() works as an action for void-returning
// functions.
TEST(ReturnTest, WorksForVoid) {
  const Action<void(int)> ret = Return();  // NOLINT
  return ret.Perform(std::make_tuple(1));
}

// Tests that Return(v) returns v.
TEST(ReturnTest, ReturnsGivenValue) {
  Action<int()> ret = Return(1);  // NOLINT
  EXPECT_EQ(1, ret.Perform(std::make_tuple()));

  ret = Return(-5);
  EXPECT_EQ(-5, ret.Perform(std::make_tuple()));
}

// Tests that Return("string literal") works.
TEST(ReturnTest, AcceptsStringLiteral) {
  Action<const char*()> a1 = Return("Hello");
  EXPECT_STREQ("Hello", a1.Perform(std::make_tuple()));

  Action<std::string()> a2 = Return("world");
  EXPECT_EQ("world", a2.Perform(std::make_tuple()));
}

// Return(x) should work fine when the mock function's return type is a
// reference-like wrapper for decltype(x), as when x is a std::string and the
// mock function returns std::string_view.
TEST(ReturnTest, SupportsReferenceLikeReturnType) {
  // A reference wrapper for std::vector<int>, implicitly convertible from it.
  struct Result {
    const std::vector<int>* v;
    Result(const std::vector<int>& vec) : v(&vec) {}  // NOLINT
  };

  // Set up an action for a mock function that returns the reference wrapper
  // type, initializing it with an actual vector.
  //
  // The returned wrapper should be initialized with a copy of that vector
  // that's embedded within the action itself (which should stay alive as long
  // as the mock object is alive), rather than e.g. a reference to the temporary
  // we feed to Return. This should work fine both for WillOnce and
  // WillRepeatedly.
  MockFunction<Result()> mock;
  EXPECT_CALL(mock, Call)
      .WillOnce(Return(std::vector<int>{17, 19, 23}))
      .WillRepeatedly(Return(std::vector<int>{29, 31, 37}));

  EXPECT_THAT(mock.AsStdFunction()(),
              Field(&Result::v, Pointee(ElementsAre(17, 19, 23))));

  EXPECT_THAT(mock.AsStdFunction()(),
              Field(&Result::v, Pointee(ElementsAre(29, 31, 37))));
}

TEST(ReturnTest, PrefersConversionOperator) {
  // Define types In and Out such that:
  //
  //  *  In is implicitly convertible to Out.
  //  *  Out also has an explicit constructor from In.
  //
  struct In;
  struct Out {
    int x;

    explicit Out(const int val) : x(val) {}
    explicit Out(const In&) : x(0) {}
  };

  struct In {
    operator Out() const { return Out{19}; }  // NOLINT
  };

  // Assumption check: the C++ language rules are such that a function that
  // returns Out which uses In a return statement will use the implicit
  // conversion path rather than the explicit constructor.
  EXPECT_THAT([]() -> Out { return In(); }(), Field(&Out::x, 19));

  // Return should work the same way: if the mock function's return type is Out
  // and we feed Return an In value, then the Out should be created through the
  // implicit conversion path rather than the explicit constructor.
  MockFunction<Out()> mock;
  EXPECT_CALL(mock, Call).WillOnce(Return(In()));
  EXPECT_THAT(mock.AsStdFunction()(), Field(&Out::x, 19));
}

// It should be possible to use Return(R) with a mock function result type U
// that is convertible from const R& but *not* R (such as
// std::reference_wrapper). This should work for both WillOnce and
// WillRepeatedly.
TEST(ReturnTest, ConversionRequiresConstLvalueReference) {
  using R = int;
  using U = std::reference_wrapper<const int>;

  static_assert(std::is_convertible<const R&, U>::value, "");
  static_assert(!std::is_convertible<R, U>::value, "");

  MockFunction<U()> mock;
  EXPECT_CALL(mock, Call).WillOnce(Return(17)).WillRepeatedly(Return(19));

  EXPECT_EQ(17, mock.AsStdFunction()());
  EXPECT_EQ(19, mock.AsStdFunction()());
}

// Return(x) should not be usable with a mock function result type that's
// implicitly convertible from decltype(x) but requires a non-const lvalue
// reference to the input. It doesn't make sense for the conversion operator to
// modify the input.
TEST(ReturnTest, ConversionRequiresMutableLvalueReference) {
  // Set up a type that is implicitly convertible from std::string&, but not
  // std::string&& or `const std::string&`.
  //
  // Avoid asserting about conversion from std::string on MSVC, which seems to
  // implement std::is_convertible incorrectly in this case.
  struct S {
    S(std::string&) {}  // NOLINT
  };

  static_assert(std::is_convertible<std::string&, S>::value, "");
#ifndef _MSC_VER
  static_assert(!std::is_convertible<std::string&&, S>::value, "");
#endif
  static_assert(!std::is_convertible<const std::string&, S>::value, "");

  // It shouldn't be possible to use the result of Return(std::string) in a
  // context where an S is needed.
  //
  // Here too we disable the assertion for MSVC, since its incorrect
  // implementation of is_convertible causes our SFINAE to be wrong.
  using RA = decltype(Return(std::string()));

  static_assert(!std::is_convertible<RA, Action<S()>>::value, "");
#ifndef _MSC_VER
  static_assert(!std::is_convertible<RA, OnceAction<S()>>::value, "");
#endif
}

TEST(ReturnTest, MoveOnlyResultType) {
  // Return should support move-only result types when used with WillOnce.
  {
    MockFunction<std::unique_ptr<int>()> mock;
    EXPECT_CALL(mock, Call)
        // NOLINTNEXTLINE
        .WillOnce(Return(std::unique_ptr<int>(new int(17))));

    EXPECT_THAT(mock.AsStdFunction()(), Pointee(17));
  }

  // The result of Return should not be convertible to Action (so it can't be
  // used with WillRepeatedly).
  static_assert(!std::is_convertible<decltype(Return(std::unique_ptr<int>())),
                                     Action<std::unique_ptr<int>()>>::value,
                "");
}

// Tests that Return(v) is covariant.

struct Base {
  bool operator==(const Base&) { return true; }
};

struct Derived : public Base {
  bool operator==(const Derived&) { return true; }
};

TEST(ReturnTest, IsCovariant) {
  Base base;
  Derived derived;
  Action<Base*()> ret = Return(&base);
  EXPECT_EQ(&base, ret.Perform(std::make_tuple()));

  ret = Return(&derived);
  EXPECT_EQ(&derived, ret.Perform(std::make_tuple()));
}

// Tests that the type of the value passed into Return is converted into T
// when the action is cast to Action<T(...)> rather than when the action is
// performed. See comments on testing::internal::ReturnAction in
// gmock-actions.h for more information.
class FromType {
 public:
  explicit FromType(bool* is_converted) : converted_(is_converted) {}
  bool* converted() const { return converted_; }

 private:
  bool* const converted_;
};

class ToType {
 public:
  // Must allow implicit conversion due to use in ImplicitCast_<T>.
  ToType(const FromType& x) { *x.converted() = true; }  // NOLINT
};

TEST(ReturnTest, ConvertsArgumentWhenConverted) {
  bool converted = false;
  FromType x(&converted);
  Action<ToType()> action(Return(x));
  EXPECT_TRUE(converted) << "Return must convert its argument in its own "
                         << "conversion operator.";
  converted = false;
  action.Perform(std::tuple<>());
  EXPECT_FALSE(converted) << "Action must NOT convert its argument "
                          << "when performed.";
}

// Tests that ReturnNull() returns NULL in a pointer-returning function.
TEST(ReturnNullTest, WorksInPointerReturningFunction) {
  const Action<int*()> a1 = ReturnNull();
  EXPECT_TRUE(a1.Perform(std::make_tuple()) == nullptr);

  const Action<const char*(bool)> a2 = ReturnNull();  // NOLINT
  EXPECT_TRUE(a2.Perform(std::make_tuple(true)) == nullptr);
}

// Tests that ReturnNull() returns NULL for shared_ptr and unique_ptr returning
// functions.
TEST(ReturnNullTest, WorksInSmartPointerReturningFunction) {
  const Action<std::unique_ptr<const int>()> a1 = ReturnNull();
  EXPECT_TRUE(a1.Perform(std::make_tuple()) == nullptr);

  const Action<std::shared_ptr<int>(std::string)> a2 = ReturnNull();
  EXPECT_TRUE(a2.Perform(std::make_tuple("foo")) == nullptr);
}

// Tests that ReturnRef(v) works for reference types.
TEST(ReturnRefTest, WorksForReference) {
  const int n = 0;
  const Action<const int&(bool)> ret = ReturnRef(n);  // NOLINT

  EXPECT_EQ(&n, &ret.Perform(std::make_tuple(true)));
}

// Tests that ReturnRef(v) is covariant.
TEST(ReturnRefTest, IsCovariant) {
  Base base;
  Derived derived;
  Action<Base&()> a = ReturnRef(base);
  EXPECT_EQ(&base, &a.Perform(std::make_tuple()));

  a = ReturnRef(derived);
  EXPECT_EQ(&derived, &a.Perform(std::make_tuple()));
}

template <typename T, typename = decltype(ReturnRef(std::declval<T&&>()))>
bool CanCallReturnRef(T&&) {
  return true;
}
bool CanCallReturnRef(Unused) { return false; }

// Tests that ReturnRef(v) is working with non-temporaries (T&)
TEST(ReturnRefTest, WorksForNonTemporary) {
  int scalar_value = 123;
  EXPECT_TRUE(CanCallReturnRef(scalar_value));

  std::string non_scalar_value("ABC");
  EXPECT_TRUE(CanCallReturnRef(non_scalar_value));

  const int const_scalar_value{321};
  EXPECT_TRUE(CanCallReturnRef(const_scalar_value));

  const std::string const_non_scalar_value("CBA");
  EXPECT_TRUE(CanCallReturnRef(const_non_scalar_value));
}

// Tests that ReturnRef(v) is not working with temporaries (T&&)
TEST(ReturnRefTest, DoesNotWorkForTemporary) {
  auto scalar_value = []() -> int { return 123; };
  EXPECT_FALSE(CanCallReturnRef(scalar_value()));

  auto non_scalar_value = []() -> std::string { return "ABC"; };
  EXPECT_FALSE(CanCallReturnRef(non_scalar_value()));

  // cannot use here callable returning "const scalar type",
  // because such const for scalar return type is ignored
  EXPECT_FALSE(CanCallReturnRef(static_cast<const int>(321)));

  auto const_non_scalar_value = []() -> const std::string { return "CBA"; };
  EXPECT_FALSE(CanCallReturnRef(const_non_scalar_value()));
}

// Tests that ReturnRefOfCopy(v) works for reference types.
TEST(ReturnRefOfCopyTest, WorksForReference) {
  int n = 42;
  const Action<const int&()> ret = ReturnRefOfCopy(n);

  EXPECT_NE(&n, &ret.Perform(std::make_tuple()));
  EXPECT_EQ(42, ret.Perform(std::make_tuple()));

  n = 43;
  EXPECT_NE(&n, &ret.Perform(std::make_tuple()));
  EXPECT_EQ(42, ret.Perform(std::make_tuple()));
}

// Tests that ReturnRefOfCopy(v) is covariant.
TEST(ReturnRefOfCopyTest, IsCovariant) {
  Base base;
  Derived derived;
  Action<Base&()> a = ReturnRefOfCopy(base);
  EXPECT_NE(&base, &a.Perform(std::make_tuple()));

  a = ReturnRefOfCopy(derived);
  EXPECT_NE(&derived, &a.Perform(std::make_tuple()));
}

// Tests that ReturnRoundRobin(v) works with initializer lists
TEST(ReturnRoundRobinTest, WorksForInitList) {
  Action<int()> ret = ReturnRoundRobin({1, 2, 3});

  EXPECT_EQ(1, ret.Perform(std::make_tuple()));
  EXPECT_EQ(2, ret.Perform(std::make_tuple()));
  EXPECT_EQ(3, ret.Perform(std::make_tuple()));
  EXPECT_EQ(1, ret.Perform(std::make_tuple()));
  EXPECT_EQ(2, ret.Perform(std::make_tuple()));
  EXPECT_EQ(3, ret.Perform(std::make_tuple()));
}

// Tests that ReturnRoundRobin(v) works with vectors
TEST(ReturnRoundRobinTest, WorksForVector) {
  std::vector<double> v = {4.4, 5.5, 6.6};
  Action<double()> ret = ReturnRoundRobin(v);

  EXPECT_EQ(4.4, ret.Perform(std::make_tuple()));
  EXPECT_EQ(5.5, ret.Perform(std::make_tuple()));
  EXPECT_EQ(6.6, ret.Perform(std::make_tuple()));
  EXPECT_EQ(4.4, ret.Perform(std::make_tuple()));
  EXPECT_EQ(5.5, ret.Perform(std::make_tuple()));
  EXPECT_EQ(6.6, ret.Perform(std::make_tuple()));
}

// Tests that DoDefault() does the default action for the mock method.

class MockClass {
 public:
  MockClass() = default;

  MOCK_METHOD1(IntFunc, int(bool flag));  // NOLINT
  MOCK_METHOD0(Foo, MyNonDefaultConstructible());
  MOCK_METHOD0(MakeUnique, std::unique_ptr<int>());
  MOCK_METHOD0(MakeUniqueBase, std::unique_ptr<Base>());
  MOCK_METHOD0(MakeVectorUnique, std::vector<std::unique_ptr<int>>());
  MOCK_METHOD1(TakeUnique, int(std::unique_ptr<int>));
  MOCK_METHOD2(TakeUnique,
               int(const std::unique_ptr<int>&, std::unique_ptr<int>));

 private:
  MockClass(const MockClass&) = delete;
  MockClass& operator=(const MockClass&) = delete;
};

// Tests that DoDefault() returns the built-in default value for the
// return type by default.
TEST(DoDefaultTest, ReturnsBuiltInDefaultValueByDefault) {
  MockClass mock;
  EXPECT_CALL(mock, IntFunc(_)).WillOnce(DoDefault());
  EXPECT_EQ(0, mock.IntFunc(true));
}

// Tests that DoDefault() throws (when exceptions are enabled) or aborts
// the process when there is no built-in default value for the return type.
TEST(DoDefaultDeathTest, DiesForUnknowType) {
  MockClass mock;
  EXPECT_CALL(mock, Foo()).WillRepeatedly(DoDefault());
#if GTEST_HAS_EXCEPTIONS
  EXPECT_ANY_THROW(mock.Foo());
#else
  EXPECT_DEATH_IF_SUPPORTED({ mock.Foo(); }, "");
#endif
}

// Tests that using DoDefault() inside a composite action leads to a
// run-time error.

void VoidFunc(bool /* flag */) {}

TEST(DoDefaultDeathTest, DiesIfUsedInCompositeAction) {
  MockClass mock;
  EXPECT_CALL(mock, IntFunc(_))
      .WillRepeatedly(DoAll(Invoke(VoidFunc), DoDefault()));

  // Ideally we should verify the error message as well.  Sadly,
  // EXPECT_DEATH() can only capture stderr, while Google Mock's
  // errors are printed on stdout.  Therefore we have to settle for
  // not verifying the message.
  EXPECT_DEATH_IF_SUPPORTED({ mock.IntFunc(true); }, "");
}

// Tests that DoDefault() returns the default value set by
// DefaultValue<T>::Set() when it's not overridden by an ON_CALL().
TEST(DoDefaultTest, ReturnsUserSpecifiedPerTypeDefaultValueWhenThereIsOne) {
  DefaultValue<int>::Set(1);
  MockClass mock;
  EXPECT_CALL(mock, IntFunc(_)).WillOnce(DoDefault());
  EXPECT_EQ(1, mock.IntFunc(false));
  DefaultValue<int>::Clear();
}

// Tests that DoDefault() does the action specified by ON_CALL().
TEST(DoDefaultTest, DoesWhatOnCallSpecifies) {
  MockClass mock;
  ON_CALL(mock, IntFunc(_)).WillByDefault(Return(2));
  EXPECT_CALL(mock, IntFunc(_)).WillOnce(DoDefault());
  EXPECT_EQ(2, mock.IntFunc(false));
}

// Tests that using DoDefault() in ON_CALL() leads to a run-time failure.
TEST(DoDefaultTest, CannotBeUsedInOnCall) {
  MockClass mock;
  EXPECT_NONFATAL_FAILURE(
      {  // NOLINT
        ON_CALL(mock, IntFunc(_)).WillByDefault(DoDefault());
      },
      "DoDefault() cannot be used in ON_CALL()");
}

// Tests that SetArgPointee<N>(v) sets the variable pointed to by
// the N-th (0-based) argument to v.
TEST(SetArgPointeeTest, SetsTheNthPointee) {
  typedef void MyFunction(bool, int*, char*);
  Action<MyFunction> a = SetArgPointee<1>(2);

  int n = 0;
  char ch = '\0';
  a.Perform(std::make_tuple(true, &n, &ch));
  EXPECT_EQ(2, n);
  EXPECT_EQ('\0', ch);

  a = SetArgPointee<2>('a');
  n = 0;
  ch = '\0';
  a.Perform(std::make_tuple(true, &n, &ch));
  EXPECT_EQ(0, n);
  EXPECT_EQ('a', ch);
}

// Tests that SetArgPointee<N>() accepts a string literal.
TEST(SetArgPointeeTest, AcceptsStringLiteral) {
  typedef void MyFunction(std::string*, const char**);
  Action<MyFunction> a = SetArgPointee<0>("hi");
  std::string str;
  const char* ptr = nullptr;
  a.Perform(std::make_tuple(&str, &ptr));
  EXPECT_EQ("hi", str);
  EXPECT_TRUE(ptr == nullptr);

  a = SetArgPointee<1>("world");
  str = "";
  a.Perform(std::make_tuple(&str, &ptr));
  EXPECT_EQ("", str);
  EXPECT_STREQ("world", ptr);
}

TEST(SetArgPointeeTest, AcceptsWideStringLiteral) {
  typedef void MyFunction(const wchar_t**);
  Action<MyFunction> a = SetArgPointee<0>(L"world");
  const wchar_t* ptr = nullptr;
  a.Perform(std::make_tuple(&ptr));
  EXPECT_STREQ(L"world", ptr);

#if GTEST_HAS_STD_WSTRING

  typedef void MyStringFunction(std::wstring*);
  Action<MyStringFunction> a2 = SetArgPointee<0>(L"world");
  std::wstring str = L"";
  a2.Perform(std::make_tuple(&str));
  EXPECT_EQ(L"world", str);

#endif
}

// Tests that SetArgPointee<N>() accepts a char pointer.
TEST(SetArgPointeeTest, AcceptsCharPointer) {
  typedef void MyFunction(bool, std::string*, const char**);
  const char* const hi = "hi";
  Action<MyFunction> a = SetArgPointee<1>(hi);
  std::string str;
  const char* ptr = nullptr;
  a.Perform(std::make_tuple(true, &str, &ptr));
  EXPECT_EQ("hi", str);
  EXPECT_TRUE(ptr == nullptr);

  char world_array[] = "world";
  char* const world = world_array;
  a = SetArgPointee<2>(world);
  str = "";
  a.Perform(std::make_tuple(true, &str, &ptr));
  EXPECT_EQ("", str);
  EXPECT_EQ(world, ptr);
}

TEST(SetArgPointeeTest, AcceptsWideCharPointer) {
  typedef void MyFunction(bool, const wchar_t**);
  const wchar_t* const hi = L"hi";
  Action<MyFunction> a = SetArgPointee<1>(hi);
  const wchar_t* ptr = nullptr;
  a.Perform(std::make_tuple(true, &ptr));
  EXPECT_EQ(hi, ptr);

#if GTEST_HAS_STD_WSTRING

  typedef void MyStringFunction(bool, std::wstring*);
  wchar_t world_array[] = L"world";
  wchar_t* const world = world_array;
  Action<MyStringFunction> a2 = SetArgPointee<1>(world);
  std::wstring str;
  a2.Perform(std::make_tuple(true, &str));
  EXPECT_EQ(world_array, str);
#endif
}

// Tests that SetArgumentPointee<N>(v) sets the variable pointed to by
// the N-th (0-based) argument to v.
TEST(SetArgumentPointeeTest, SetsTheNthPointee) {
  typedef void MyFunction(bool, int*, char*);
  Action<MyFunction> a = SetArgumentPointee<1>(2);

  int n = 0;
  char ch = '\0';
  a.Perform(std::make_tuple(true, &n, &ch));
  EXPECT_EQ(2, n);
  EXPECT_EQ('\0', ch);

  a = SetArgumentPointee<2>('a');
  n = 0;
  ch = '\0';
  a.Perform(std::make_tuple(true, &n, &ch));
  EXPECT_EQ(0, n);
  EXPECT_EQ('a', ch);
}

// Sample functions and functors for testing Invoke() and etc.
int Nullary() { return 1; }

class NullaryFunctor {
 public:
  int operator()() { return 2; }
};

bool g_done = false;
void VoidNullary() { g_done = true; }

class VoidNullaryFunctor {
 public:
  void operator()() { g_done = true; }
};

short Short(short n) { return n; }  // NOLINT
char Char(char ch) { return ch; }

const char* CharPtr(const char* s) { return s; }

bool Unary(int x) { return x < 0; }

const char* Binary(const char* input, short n) { return input + n; }  // NOLINT

void VoidBinary(int, char) { g_done = true; }

int Ternary(int x, char y, short z) { return x + y + z; }  // NOLINT

int SumOf4(int a, int b, int c, int d) { return a + b + c + d; }

class Foo {
 public:
  Foo() : value_(123) {}

  int Nullary() const { return value_; }

 private:
  int value_;
};

// Tests InvokeWithoutArgs(function).
TEST(InvokeWithoutArgsTest, Function) {
  // As an action that takes one argument.
  Action<int(int)> a = InvokeWithoutArgs(Nullary);  // NOLINT
  EXPECT_EQ(1, a.Perform(std::make_tuple(2)));

  // As an action that takes two arguments.
  Action<int(int, double)> a2 = InvokeWithoutArgs(Nullary);  // NOLINT
  EXPECT_EQ(1, a2.Perform(std::make_tuple(2, 3.5)));

  // As an action that returns void.
  Action<void(int)> a3 = InvokeWithoutArgs(VoidNullary);  // NOLINT
  g_done = false;
  a3.Perform(std::make_tuple(1));
  EXPECT_TRUE(g_done);
}

// Tests InvokeWithoutArgs(functor).
TEST(InvokeWithoutArgsTest, Functor) {
  // As an action that takes no argument.
  Action<int()> a = InvokeWithoutArgs(NullaryFunctor());  // NOLINT
  EXPECT_EQ(2, a.Perform(std::make_tuple()));

  // As an action that takes three arguments.
  Action<int(int, double, char)> a2 =  // NOLINT
      InvokeWithoutArgs(NullaryFunctor());
  EXPECT_EQ(2, a2.Perform(std::make_tuple(3, 3.5, 'a')));

  // As an action that returns void.
  Action<void()> a3 = InvokeWithoutArgs(VoidNullaryFunctor());
  g_done = false;
  a3.Perform(std::make_tuple());
  EXPECT_TRUE(g_done);
}

// Tests InvokeWithoutArgs(obj_ptr, method).
TEST(InvokeWithoutArgsTest, Method) {
  Foo foo;
  Action<int(bool, char)> a =  // NOLINT
      InvokeWithoutArgs(&foo, &Foo::Nullary);
  EXPECT_EQ(123, a.Perform(std::make_tuple(true, 'a')));
}

// Tests using IgnoreResult() on a polymorphic action.
TEST(IgnoreResultTest, PolymorphicAction) {
  Action<void(int)> a = IgnoreResult(Return(5));  // NOLINT
  a.Perform(std::make_tuple(1));
}

// Tests using IgnoreResult() on a monomorphic action.

int ReturnOne() {
  g_done = true;
  return 1;
}

TEST(IgnoreResultTest, MonomorphicAction) {
  g_done = false;
  Action<void()> a = IgnoreResult(Invoke(ReturnOne));
  a.Perform(std::make_tuple());
  EXPECT_TRUE(g_done);
}

// Tests using IgnoreResult() on an action that returns a class type.

MyNonDefaultConstructible ReturnMyNonDefaultConstructible(double /* x */) {
  g_done = true;
  return MyNonDefaultConstructible(42);
}

TEST(IgnoreResultTest, ActionReturningClass) {
  g_done = false;
  Action<void(int)> a =
      IgnoreResult(Invoke(ReturnMyNonDefaultConstructible));  // NOLINT
  a.Perform(std::make_tuple(2));
  EXPECT_TRUE(g_done);
}

TEST(AssignTest, Int) {
  int x = 0;
  Action<void(int)> a = Assign(&x, 5);
  a.Perform(std::make_tuple(0));
  EXPECT_EQ(5, x);
}

TEST(AssignTest, String) {
  ::std::string x;
  Action<void(void)> a = Assign(&x, "Hello, world");
  a.Perform(std::make_tuple());
  EXPECT_EQ("Hello, world", x);
}

TEST(AssignTest, CompatibleTypes) {
  double x = 0;
  Action<void(int)> a = Assign(&x, 5);
  a.Perform(std::make_tuple(0));
  EXPECT_DOUBLE_EQ(5, x);
}

// DoAll should support &&-qualified actions when used with WillOnce.
TEST(DoAll, SupportsRefQualifiedActions) {
  struct InitialAction {
    void operator()(const int arg) && { EXPECT_EQ(17, arg); }
  };

  struct FinalAction {
    int operator()() && { return 19; }
  };

  MockFunction<int(int)> mock;
  EXPECT_CALL(mock, Call).WillOnce(DoAll(InitialAction{}, FinalAction{}));
  EXPECT_EQ(19, mock.AsStdFunction()(17));
}

// DoAll should never provide rvalue references to the initial actions. If the
// mock action itself accepts an rvalue reference or a non-scalar object by
// value then the final action should receive an rvalue reference, but initial
// actions should receive only lvalue references.
TEST(DoAll, ProvidesLvalueReferencesToInitialActions) {
  struct Obj {};

  // Mock action accepts by value: the initial action should be fed a const
  // lvalue reference, and the final action an rvalue reference.
  {
    struct InitialAction {
      void operator()(Obj&) const { FAIL() << "Unexpected call"; }
      void operator()(const Obj&) const {}
      void operator()(Obj&&) const { FAIL() << "Unexpected call"; }
      void operator()(const Obj&&) const { FAIL() << "Unexpected call"; }
    };

    MockFunction<void(Obj)> mock;
    EXPECT_CALL(mock, Call)
        .WillOnce(DoAll(InitialAction{}, InitialAction{}, [](Obj&&) {}))
        .WillRepeatedly(DoAll(InitialAction{}, InitialAction{}, [](Obj&&) {}));

    mock.AsStdFunction()(Obj{});
    mock.AsStdFunction()(Obj{});
  }

  // Mock action accepts by const lvalue reference: both actions should receive
  // a const lvalue reference.
  {
    struct InitialAction {
      void operator()(Obj&) const { FAIL() << "Unexpected call"; }
      void operator()(const Obj&) const {}
      void operator()(Obj&&) const { FAIL() << "Unexpected call"; }
      void operator()(const Obj&&) const { FAIL() << "Unexpected call"; }
    };

    MockFunction<void(const Obj&)> mock;
    EXPECT_CALL(mock, Call)
        .WillOnce(DoAll(InitialAction{}, InitialAction{}, [](const Obj&) {}))
        .WillRepeatedly(
            DoAll(InitialAction{}, InitialAction{}, [](const Obj&) {}));

    mock.AsStdFunction()(Obj{});
    mock.AsStdFunction()(Obj{});
  }

  // Mock action accepts by non-const lvalue reference: both actions should get
  // a non-const lvalue reference if they want them.
  {
    struct InitialAction {
      void operator()(Obj&) const {}
      void operator()(Obj&&) const { FAIL() << "Unexpected call"; }
    };

    MockFunction<void(Obj&)> mock;
    EXPECT_CALL(mock, Call)
        .WillOnce(DoAll(InitialAction{}, InitialAction{}, [](Obj&) {}))
        .WillRepeatedly(DoAll(InitialAction{}, InitialAction{}, [](Obj&) {}));

    Obj obj;
    mock.AsStdFunction()(obj);
    mock.AsStdFunction()(obj);
  }

  // Mock action accepts by rvalue reference: the initial actions should receive
  // a non-const lvalue reference if it wants it, and the final action an rvalue
  // reference.
  {
    struct InitialAction {
      void operator()(Obj&) const {}
      void operator()(Obj&&) const { FAIL() << "Unexpected call"; }
    };

    MockFunction<void(Obj&&)> mock;
    EXPECT_CALL(mock, Call)
        .WillOnce(DoAll(InitialAction{}, InitialAction{}, [](Obj&&) {}))
        .WillRepeatedly(DoAll(InitialAction{}, InitialAction{}, [](Obj&&) {}));

    mock.AsStdFunction()(Obj{});
    mock.AsStdFunction()(Obj{});
  }

  // &&-qualified initial actions should also be allowed with WillOnce.
  {
    struct InitialAction {
      void operator()(Obj&) && {}
    };

    MockFunction<void(Obj&)> mock;
    EXPECT_CALL(mock, Call)
        .WillOnce(DoAll(InitialAction{}, InitialAction{}, [](Obj&) {}));

    Obj obj;
    mock.AsStdFunction()(obj);
  }

  {
    struct InitialAction {
      void operator()(Obj&) && {}
    };

    MockFunction<void(Obj&&)> mock;
    EXPECT_CALL(mock, Call)
        .WillOnce(DoAll(InitialAction{}, InitialAction{}, [](Obj&&) {}));

    mock.AsStdFunction()(Obj{});
  }
}

// DoAll should support being used with type-erased Action objects, both through
// WillOnce and WillRepeatedly.
TEST(DoAll, SupportsTypeErasedActions) {
  // With only type-erased actions.
  const Action<void()> initial_action = [] {};
  const Action<int()> final_action = [] { return 17; };

  MockFunction<int()> mock;
  EXPECT_CALL(mock, Call)
      .WillOnce(DoAll(initial_action, initial_action, final_action))
      .WillRepeatedly(DoAll(initial_action, initial_action, final_action));

  EXPECT_EQ(17, mock.AsStdFunction()());

  // With &&-qualified and move-only final action.
  {
    struct FinalAction {
      FinalAction() = default;
      FinalAction(FinalAction&&) = default;

      int operator()() && { return 17; }
    };

    EXPECT_CALL(mock, Call)
        .WillOnce(DoAll(initial_action, initial_action, FinalAction{}));

    EXPECT_EQ(17, mock.AsStdFunction()());
  }
}

// A DoAll action should be convertible to a OnceAction, even when its component
// sub-actions are user-provided types that define only an Action conversion
// operator. If they supposed being called more than once then they also support
// being called at most once.
TEST(DoAll, ConvertibleToOnceActionWithUserProvidedActionConversion) {
  // Simplest case: only one sub-action.
  struct CustomFinal final {
    operator Action<int()>() {  // NOLINT
      return Return(17);
    }

    operator Action<int(int, char)>() {  // NOLINT
      return Return(19);
    }
  };

  {
    OnceAction<int()> action = DoAll(CustomFinal{});
    EXPECT_EQ(17, std::move(action).Call());
  }

  {
    OnceAction<int(int, char)> action = DoAll(CustomFinal{});
    EXPECT_EQ(19, std::move(action).Call(0, 0));
  }

  // It should also work with multiple sub-actions.
  struct CustomInitial final {
    operator Action<void()>() {  // NOLINT
      return [] {};
    }

    operator Action<void(int, char)>() {  // NOLINT
      return [] {};
    }
  };

  {
    OnceAction<int()> action = DoAll(CustomInitial{}, CustomFinal{});
    EXPECT_EQ(17, std::move(action).Call());
  }

  {
    OnceAction<int(int, char)> action = DoAll(CustomInitial{}, CustomFinal{});
    EXPECT_EQ(19, std::move(action).Call(0, 0));
  }
}

// Tests using WithArgs and with an action that takes 1 argument.
TEST(WithArgsTest, OneArg) {
  Action<bool(double x, int n)> a = WithArgs<1>(Invoke(Unary));  // NOLINT
  EXPECT_TRUE(a.Perform(std::make_tuple(1.5, -1)));
  EXPECT_FALSE(a.Perform(std::make_tuple(1.5, 1)));
}

// Tests using WithArgs with an action that takes 2 arguments.
TEST(WithArgsTest, TwoArgs) {
  Action<const char*(const char* s, double x, short n)> a =  // NOLINT
      WithArgs<0, 2>(Invoke(Binary));
  const char s[] = "Hello";
  EXPECT_EQ(s + 2, a.Perform(std::make_tuple(CharPtr(s), 0.5, Short(2))));
}

struct ConcatAll {
  std::string operator()() const { return {}; }
  template <typename... I>
  std::string operator()(const char* a, I... i) const {
    return a + ConcatAll()(i...);
  }
};

// Tests using WithArgs with an action that takes 10 arguments.
TEST(WithArgsTest, TenArgs) {
  Action<std::string(const char*, const char*, const char*, const char*)> a =
      WithArgs<0, 1, 2, 3, 2, 1, 0, 1, 2, 3>(Invoke(ConcatAll{}));
  EXPECT_EQ("0123210123",
            a.Perform(std::make_tuple(CharPtr("0"), CharPtr("1"), CharPtr("2"),
                                      CharPtr("3"))));
}

// Tests using WithArgs with an action that is not Invoke().
class SubtractAction : public ActionInterface<int(int, int)> {
 public:
  int Perform(const std::tuple<int, int>& args) override {
    return std::get<0>(args) - std::get<1>(args);
  }
};

TEST(WithArgsTest, NonInvokeAction) {
  Action<int(const std::string&, int, int)> a =
      WithArgs<2, 1>(MakeAction(new SubtractAction));
  std::tuple<std::string, int, int> dummy =
      std::make_tuple(std::string("hi"), 2, 10);
  EXPECT_EQ(8, a.Perform(dummy));
}

// Tests using WithArgs to pass all original arguments in the original order.
TEST(WithArgsTest, Identity) {
  Action<int(int x, char y, short z)> a =  // NOLINT
      WithArgs<0, 1, 2>(Invoke(Ternary));
  EXPECT_EQ(123, a.Perform(std::make_tuple(100, Char(20), Short(3))));
}

// Tests using WithArgs with repeated arguments.
TEST(WithArgsTest, RepeatedArguments) {
  Action<int(bool, int m, int n)> a =  // NOLINT
      WithArgs<1, 1, 1, 1>(Invoke(SumOf4));
  EXPECT_EQ(4, a.Perform(std::make_tuple(false, 1, 10)));
}

// Tests using WithArgs with reversed argument order.
TEST(WithArgsTest, ReversedArgumentOrder) {
  Action<const char*(short n, const char* input)> a =  // NOLINT
      WithArgs<1, 0>(Invoke(Binary));
  const char s[] = "Hello";
  EXPECT_EQ(s + 2, a.Perform(std::make_tuple(Short(2), CharPtr(s))));
}

// Tests using WithArgs with compatible, but not identical, argument types.
TEST(WithArgsTest, ArgsOfCompatibleTypes) {
  Action<long(short x, char y, double z, char c)> a =  // NOLINT
      WithArgs<0, 1, 3>(Invoke(Ternary));
  EXPECT_EQ(123,
            a.Perform(std::make_tuple(Short(100), Char(20), 5.6, Char(3))));
}

// Tests using WithArgs with an action that returns void.
TEST(WithArgsTest, VoidAction) {
  Action<void(double x, char c, int n)> a = WithArgs<2, 1>(Invoke(VoidBinary));
  g_done = false;
  a.Perform(std::make_tuple(1.5, 'a', 3));
  EXPECT_TRUE(g_done);
}

TEST(WithArgsTest, ReturnReference) {
  Action<int&(int&, void*)> aa = WithArgs<0>([](int& a) -> int& { return a; });
  int i = 0;
  const int& res = aa.Perform(std::forward_as_tuple(i, nullptr));
  EXPECT_EQ(&i, &res);
}

TEST(WithArgsTest, InnerActionWithConversion) {
  Action<Derived*()> inner = [] { return nullptr; };

  MockFunction<Base*(double)> mock;
  EXPECT_CALL(mock, Call)
      .WillOnce(WithoutArgs(inner))
      .WillRepeatedly(WithoutArgs(inner));

  EXPECT_EQ(nullptr, mock.AsStdFunction()(1.1));
  EXPECT_EQ(nullptr, mock.AsStdFunction()(1.1));
}

// It should be possible to use an &&-qualified inner action as long as the
// whole shebang is used as an rvalue with WillOnce.
TEST(WithArgsTest, RefQualifiedInnerAction) {
  struct SomeAction {
    int operator()(const int arg) && {
      EXPECT_EQ(17, arg);
      return 19;
    }
  };

  MockFunction<int(int, int)> mock;
  EXPECT_CALL(mock, Call).WillOnce(WithArg<1>(SomeAction{}));
  EXPECT_EQ(19, mock.AsStdFunction()(0, 17));
}

// It should be fine to provide an lvalue WithArgsAction to WillOnce, even when
// the inner action only wants to convert to OnceAction.
TEST(WithArgsTest, ProvideAsLvalueToWillOnce) {
  struct SomeAction {
    operator OnceAction<int(int)>() const {  // NOLINT
      return [](const int arg) { return arg + 2; };
    }
  };

  const auto wa = WithArg<1>(SomeAction{});

  MockFunction<int(int, int)> mock;
  EXPECT_CALL(mock, Call).WillOnce(wa);
  EXPECT_EQ(19, mock.AsStdFunction()(0, 17));
}

#ifndef GTEST_OS_WINDOWS_MOBILE

class SetErrnoAndReturnTest : public testing::Test {
 protected:
  void SetUp() override { errno = 0; }
  void TearDown() override { errno = 0; }
};

TEST_F(SetErrnoAndReturnTest, Int) {
  Action<int(void)> a = SetErrnoAndReturn(ENOTTY, -5);
  EXPECT_EQ(-5, a.Perform(std::make_tuple()));
  EXPECT_EQ(ENOTTY, errno);
}

TEST_F(SetErrnoAndReturnTest, Ptr) {
  int x;
  Action<int*(void)> a = SetErrnoAndReturn(ENOTTY, &x);
  EXPECT_EQ(&x, a.Perform(std::make_tuple()));
  EXPECT_EQ(ENOTTY, errno);
}

TEST_F(SetErrnoAndReturnTest, CompatibleTypes) {
  Action<double()> a = SetErrnoAndReturn(EINVAL, 5);
  EXPECT_DOUBLE_EQ(5.0, a.Perform(std::make_tuple()));
  EXPECT_EQ(EINVAL, errno);
}

#endif  // !GTEST_OS_WINDOWS_MOBILE

// Tests ByRef().

// Tests that the result of ByRef() is copyable.
TEST(ByRefTest, IsCopyable) {
  const std::string s1 = "Hi";
  const std::string s2 = "Hello";

  auto ref_wrapper = ByRef(s1);
  const std::string& r1 = ref_wrapper;
  EXPECT_EQ(&s1, &r1);

  // Assigns a new value to ref_wrapper.
  ref_wrapper = ByRef(s2);
  const std::string& r2 = ref_wrapper;
  EXPECT_EQ(&s2, &r2);

  auto ref_wrapper1 = ByRef(s1);
  // Copies ref_wrapper1 to ref_wrapper.
  ref_wrapper = ref_wrapper1;
  const std::string& r3 = ref_wrapper;
  EXPECT_EQ(&s1, &r3);
}

// Tests using ByRef() on a const value.
TEST(ByRefTest, ConstValue) {
  const int n = 0;
  // int& ref = ByRef(n);  // This shouldn't compile - we have a
  // negative compilation test to catch it.
  const int& const_ref = ByRef(n);
  EXPECT_EQ(&n, &const_ref);
}

// Tests using ByRef() on a non-const value.
TEST(ByRefTest, NonConstValue) {
  int n = 0;

  // ByRef(n) can be used as either an int&,
  int& ref = ByRef(n);
  EXPECT_EQ(&n, &ref);

  // or a const int&.
  const int& const_ref = ByRef(n);
  EXPECT_EQ(&n, &const_ref);
}

// Tests explicitly specifying the type when using ByRef().
TEST(ByRefTest, ExplicitType) {
  int n = 0;
  const int& r1 = ByRef<const int>(n);
  EXPECT_EQ(&n, &r1);

  // ByRef<char>(n);  // This shouldn't compile - we have a negative
  // compilation test to catch it.

  Derived d;
  Derived& r2 = ByRef<Derived>(d);
  EXPECT_EQ(&d, &r2);

  const Derived& r3 = ByRef<const Derived>(d);
  EXPECT_EQ(&d, &r3);

  Base& r4 = ByRef<Base>(d);
  EXPECT_EQ(&d, &r4);

  const Base& r5 = ByRef<const Base>(d);
  EXPECT_EQ(&d, &r5);

  // The following shouldn't compile - we have a negative compilation
  // test for it.
  //
  // Base b;
  // ByRef<Derived>(b);
}

// Tests that Google Mock prints expression ByRef(x) as a reference to x.
TEST(ByRefTest, PrintsCorrectly) {
  int n = 42;
  ::std::stringstream expected, actual;
  testing::internal::UniversalPrinter<const int&>::Print(n, &expected);
  testing::internal::UniversalPrint(ByRef(n), &actual);
  EXPECT_EQ(expected.str(), actual.str());
}

struct UnaryConstructorClass {
  explicit UnaryConstructorClass(int v) : value(v) {}
  int value;
};

// Tests using ReturnNew() with a unary constructor.
TEST(ReturnNewTest, Unary) {
  Action<UnaryConstructorClass*()> a = ReturnNew<UnaryConstructorClass>(4000);
  UnaryConstructorClass* c = a.Perform(std::make_tuple());
  EXPECT_EQ(4000, c->value);
  delete c;
}

TEST(ReturnNewTest, UnaryWorksWhenMockMethodHasArgs) {
  Action<UnaryConstructorClass*(bool, int)> a =
      ReturnNew<UnaryConstructorClass>(4000);
  UnaryConstructorClass* c = a.Perform(std::make_tuple(false, 5));
  EXPECT_EQ(4000, c->value);
  delete c;
}

TEST(ReturnNewTest, UnaryWorksWhenMockMethodReturnsPointerToConst) {
  Action<const UnaryConstructorClass*()> a =
      ReturnNew<UnaryConstructorClass>(4000);
  const UnaryConstructorClass* c = a.Perform(std::make_tuple());
  EXPECT_EQ(4000, c->value);
  delete c;
}

class TenArgConstructorClass {
 public:
  TenArgConstructorClass(int a1, int a2, int a3, int a4, int a5, int a6, int a7,
                         int a8, int a9, int a10)
      : value_(a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10) {}
  int value_;
};

// Tests using ReturnNew() with a 10-argument constructor.
TEST(ReturnNewTest, ConstructorThatTakes10Arguments) {
  Action<TenArgConstructorClass*()> a = ReturnNew<TenArgConstructorClass>(
      1000000000, 200000000, 30000000, 4000000, 500000, 60000, 7000, 800, 90,
      0);
  TenArgConstructorClass* c = a.Perform(std::make_tuple());
  EXPECT_EQ(1234567890, c->value_);
  delete c;
}

std::unique_ptr<int> UniquePtrSource() { return std::make_unique<int>(19); }

std::vector<std::unique_ptr<int>> VectorUniquePtrSource() {
  std::vector<std::unique_ptr<int>> out;
  out.emplace_back(new int(7));
  return out;
}

TEST(MockMethodTest, CanReturnMoveOnlyValue_Return) {
  MockClass mock;
  std::unique_ptr<int> i(new int(19));
  EXPECT_CALL(mock, MakeUnique()).WillOnce(Return(ByMove(std::move(i))));
  EXPECT_CALL(mock, MakeVectorUnique())
      .WillOnce(Return(ByMove(VectorUniquePtrSource())));
  Derived* d = new Derived;
  EXPECT_CALL(mock, MakeUniqueBase())
      .WillOnce(Return(ByMove(std::unique_ptr<Derived>(d))));

  std::unique_ptr<int> result1 = mock.MakeUnique();
  EXPECT_EQ(19, *result1);

  std::vector<std::unique_ptr<int>> vresult = mock.MakeVectorUnique();
  EXPECT_EQ(1u, vresult.size());
  EXPECT_NE(nullptr, vresult[0]);
  EXPECT_EQ(7, *vresult[0]);

  std::unique_ptr<Base> result2 = mock.MakeUniqueBase();
  EXPECT_EQ(d, result2.get());
}

TEST(MockMethodTest, CanReturnMoveOnlyValue_DoAllReturn) {
  testing::MockFunction<void()> mock_function;
  MockClass mock;
  std::unique_ptr<int> i(new int(19));
  EXPECT_CALL(mock_function, Call());
  EXPECT_CALL(mock, MakeUnique())
      .WillOnce(DoAll(InvokeWithoutArgs(&mock_function,
                                        &testing::MockFunction<void()>::Call),
                      Return(ByMove(std::move(i)))));

  std::unique_ptr<int> result1 = mock.MakeUnique();
  EXPECT_EQ(19, *result1);
}

TEST(MockMethodTest, CanReturnMoveOnlyValue_Invoke) {
  MockClass mock;

  // Check default value
  DefaultValue<std::unique_ptr<int>>::SetFactory(
      [] { return std::make_unique<int>(42); });
  EXPECT_EQ(42, *mock.MakeUnique());

  EXPECT_CALL(mock, MakeUnique()).WillRepeatedly(Invoke(UniquePtrSource));
  EXPECT_CALL(mock, MakeVectorUnique())
      .WillRepeatedly(Invoke(VectorUniquePtrSource));
  std::unique_ptr<int> result1 = mock.MakeUnique();
  EXPECT_EQ(19, *result1);
  std::unique_ptr<int> result2 = mock.MakeUnique();
  EXPECT_EQ(19, *result2);
  EXPECT_NE(result1, result2);

  std::vector<std::unique_ptr<int>> vresult = mock.MakeVectorUnique();
  EXPECT_EQ(1u, vresult.size());
  EXPECT_NE(nullptr, vresult[0]);
  EXPECT_EQ(7, *vresult[0]);
}

TEST(MockMethodTest, CanTakeMoveOnlyValue) {
  MockClass mock;
  auto make = [](int i) { return std::make_unique<int>(i); };

  EXPECT_CALL(mock, TakeUnique(_)).WillRepeatedly([](std::unique_ptr<int> i) {
    return *i;
  });
  // DoAll() does not compile, since it would move from its arguments twice.
  // EXPECT_CALL(mock, TakeUnique(_, _))
  //     .WillRepeatedly(DoAll(Invoke([](std::unique_ptr<int> j) {}),
  //     Return(1)));
  EXPECT_CALL(mock, TakeUnique(testing::Pointee(7)))
      .WillOnce(Return(-7))
      .RetiresOnSaturation();
  EXPECT_CALL(mock, TakeUnique(testing::IsNull()))
      .WillOnce(Return(-1))
      .RetiresOnSaturation();

  EXPECT_EQ(5, mock.TakeUnique(make(5)));
  EXPECT_EQ(-7, mock.TakeUnique(make(7)));
  EXPECT_EQ(7, mock.TakeUnique(make(7)));
  EXPECT_EQ(7, mock.TakeUnique(make(7)));
  EXPECT_EQ(-1, mock.TakeUnique({}));

  // Some arguments are moved, some passed by reference.
  auto lvalue = make(6);
  EXPECT_CALL(mock, TakeUnique(_, _))
      .WillOnce([](const std::unique_ptr<int>& i, std::unique_ptr<int> j) {
        return *i * *j;
      });
  EXPECT_EQ(42, mock.TakeUnique(lvalue, make(7)));

  // The unique_ptr can be saved by the action.
  std::unique_ptr<int> saved;
  EXPECT_CALL(mock, TakeUnique(_)).WillOnce([&saved](std::unique_ptr<int> i) {
    saved = std::move(i);
    return 0;
  });
  EXPECT_EQ(0, mock.TakeUnique(make(42)));
  EXPECT_EQ(42, *saved);
}

// It should be possible to use callables with an &&-qualified call operator
// with WillOnce, since they will be called only once. This allows actions to
// contain and manipulate move-only types.
TEST(MockMethodTest, ActionHasRvalueRefQualifiedCallOperator) {
  struct Return17 {
    int operator()() && { return 17; }
  };

  // Action is directly compatible with mocked function type.
  {
    MockFunction<int()> mock;
    EXPECT_CALL(mock, Call).WillOnce(Return17());

    EXPECT_EQ(17, mock.AsStdFunction()());
  }

  // Action doesn't want mocked function arguments.
  {
    MockFunction<int(int)> mock;
    EXPECT_CALL(mock, Call).WillOnce(Return17());

    EXPECT_EQ(17, mock.AsStdFunction()(0));
  }
}

// Edge case: if an action has both a const-qualified and an &&-qualified call
// operator, there should be no "ambiguous call" errors. The &&-qualified
// operator should be used by WillOnce (since it doesn't need to retain the
// action beyond one call), and the const-qualified one by WillRepeatedly.
TEST(MockMethodTest, ActionHasMultipleCallOperators) {
  struct ReturnInt {
    int operator()() && { return 17; }
    int operator()() const& { return 19; }
  };

  // Directly compatible with mocked function type.
  {
    MockFunction<int()> mock;
    EXPECT_CALL(mock, Call).WillOnce(ReturnInt()).WillRepeatedly(ReturnInt());

    EXPECT_EQ(17, mock.AsStdFunction()());
    EXPECT_EQ(19, mock.AsStdFunction()());
    EXPECT_EQ(19, mock.AsStdFunction()());
  }

  // Ignores function arguments.
  {
    MockFunction<int(int)> mock;
    EXPECT_CALL(mock, Call).WillOnce(ReturnInt()).WillRepeatedly(ReturnInt());

    EXPECT_EQ(17, mock.AsStdFunction()(0));
    EXPECT_EQ(19, mock.AsStdFunction()(0));
    EXPECT_EQ(19, mock.AsStdFunction()(0));
  }
}

// WillOnce should have no problem coping with a move-only action, whether it is
// &&-qualified or not.
TEST(MockMethodTest, MoveOnlyAction) {
  // &&-qualified
  {
    struct Return17 {
      Return17() = default;
      Return17(Return17&&) = default;

      Return17(const Return17&) = delete;
      Return17 operator=(const Return17&) = delete;

      int operator()() && { return 17; }
    };

    MockFunction<int()> mock;
    EXPECT_CALL(mock, Call).WillOnce(Return17());
    EXPECT_EQ(17, mock.AsStdFunction()());
  }

  // Not &&-qualified
  {
    struct Return17 {
      Return17() = default;
      Return17(Return17&&) = default;

      Return17(const Return17&) = delete;
      Return17 operator=(const Return17&) = delete;

      int operator()() const { return 17; }
    };

    MockFunction<int()> mock;
    EXPECT_CALL(mock, Call).WillOnce(Return17());
    EXPECT_EQ(17, mock.AsStdFunction()());
  }
}

// It should be possible to use an action that returns a value with a mock
// function that doesn't, both through WillOnce and WillRepeatedly.
TEST(MockMethodTest, ActionReturnsIgnoredValue) {
  struct ReturnInt {
    int operator()() const { return 0; }
  };

  MockFunction<void()> mock;
  EXPECT_CALL(mock, Call).WillOnce(ReturnInt()).WillRepeatedly(ReturnInt());

  mock.AsStdFunction()();
  mock.AsStdFunction()();
}

// Despite the fanciness around move-only actions and so on, it should still be
// possible to hand an lvalue reference to a copyable action to WillOnce.
TEST(MockMethodTest, WillOnceCanAcceptLvalueReference) {
  MockFunction<int()> mock;

  const auto action = [] { return 17; };
  EXPECT_CALL(mock, Call).WillOnce(action);

  EXPECT_EQ(17, mock.AsStdFunction()());
}

// A callable that doesn't use SFINAE to restrict its call operator's overload
// set, but is still picky about which arguments it will accept.
struct StaticAssertSingleArgument {
  template <typename... Args>
  static constexpr bool CheckArgs() {
    static_assert(sizeof...(Args) == 1, "");
    return true;
  }

  template <typename... Args, bool = CheckArgs<Args...>()>
  int operator()(Args...) const {
    return 17;
  }
};

// WillOnce and WillRepeatedly should both work fine with naïve implementations
// of actions that don't use SFINAE to limit the overload set for their call
// operator. If they are compatible with the actual mocked signature, we
// shouldn't probe them with no arguments and trip a static_assert.
TEST(MockMethodTest, ActionSwallowsAllArguments) {
  MockFunction<int(int)> mock;
  EXPECT_CALL(mock, Call)
      .WillOnce(StaticAssertSingleArgument{})
      .WillRepeatedly(StaticAssertSingleArgument{});

  EXPECT_EQ(17, mock.AsStdFunction()(0));
  EXPECT_EQ(17, mock.AsStdFunction()(0));
}

struct ActionWithTemplatedConversionOperators {
  template <typename... Args>
  operator OnceAction<int(Args...)>() && {  // NOLINT
    return [] { return 17; };
  }

  template <typename... Args>
  operator Action<int(Args...)>() const {  // NOLINT
    return [] { return 19; };
  }
};

// It should be fine to hand both WillOnce and WillRepeatedly a function that
// defines templated conversion operators to OnceAction and Action. WillOnce
// should prefer the OnceAction version.
TEST(MockMethodTest, ActionHasTemplatedConversionOperators) {
  MockFunction<int()> mock;
  EXPECT_CALL(mock, Call)
      .WillOnce(ActionWithTemplatedConversionOperators{})
      .WillRepeatedly(ActionWithTemplatedConversionOperators{});

  EXPECT_EQ(17, mock.AsStdFunction()());
  EXPECT_EQ(19, mock.AsStdFunction()());
}

// Tests for std::function based action.

int Add(int val, int& ref, int* ptr) {  // NOLINT
  int result = val + ref + *ptr;
  ref = 42;
  *ptr = 43;
  return result;
}

int Deref(std::unique_ptr<int> ptr) { return *ptr; }

struct Double {
  template <typename T>
  T operator()(T t) {
    return 2 * t;
  }
};

std::unique_ptr<int> UniqueInt(int i) { return std::make_unique<int>(i); }

TEST(FunctorActionTest, ActionFromFunction) {
  Action<int(int, int&, int*)> a = &Add;
  int x = 1, y = 2, z = 3;
  EXPECT_EQ(6, a.Perform(std::forward_as_tuple(x, y, &z)));
  EXPECT_EQ(42, y);
  EXPECT_EQ(43, z);

  Action<int(std::unique_ptr<int>)> a1 = &Deref;
  EXPECT_EQ(7, a1.Perform(std::make_tuple(UniqueInt(7))));
}

TEST(FunctorActionTest, ActionFromLambda) {
  Action<int(bool, int)> a1 = [](bool b, int i) { return b ? i : 0; };
  EXPECT_EQ(5, a1.Perform(std::make_tuple(true, 5)));
  EXPECT_EQ(0, a1.Perform(std::make_tuple(false, 5)));

  std::unique_ptr<int> saved;
  Action<void(std::unique_ptr<int>)> a2 = [&saved](std::unique_ptr<int> p) {
    saved = std::move(p);
  };
  a2.Perform(std::make_tuple(UniqueInt(5)));
  EXPECT_EQ(5, *saved);
}

TEST(FunctorActionTest, PolymorphicFunctor) {
  Action<int(int)> ai = Double();
  EXPECT_EQ(2, ai.Perform(std::make_tuple(1)));
  Action<double(double)> ad = Double();  // Double? Double double!
  EXPECT_EQ(3.0, ad.Perform(std::make_tuple(1.5)));
}

TEST(FunctorActionTest, TypeConversion) {
  // Numeric promotions are allowed.
  const Action<bool(int)> a1 = [](int i) { return i > 1; };
  const Action<int(bool)> a2 = Action<int(bool)>(a1);
  EXPECT_EQ(1, a1.Perform(std::make_tuple(42)));
  EXPECT_EQ(0, a2.Perform(std::make_tuple(42)));

  // Implicit constructors are allowed.
  const Action<bool(std::string)> s1 = [](std::string s) { return !s.empty(); };
  const Action<int(const char*)> s2 = Action<int(const char*)>(s1);
  EXPECT_EQ(0, s2.Perform(std::make_tuple("")));
  EXPECT_EQ(1, s2.Perform(std::make_tuple("hello")));

  // Also between the lambda and the action itself.
  const Action<bool(std::string)> x1 = [](Unused) { return 42; };
  const Action<bool(std::string)> x2 = [] { return 42; };
  EXPECT_TRUE(x1.Perform(std::make_tuple("hello")));
  EXPECT_TRUE(x2.Perform(std::make_tuple("hello")));

  // Ensure decay occurs where required.
  std::function<int()> f = [] { return 7; };
  Action<int(int)> d = f;
  f = nullptr;
  EXPECT_EQ(7, d.Perform(std::make_tuple(1)));

  // Ensure creation of an empty action succeeds.
  Action<void(int)>(nullptr);
}

TEST(FunctorActionTest, UnusedArguments) {
  // Verify that users can ignore uninteresting arguments.
  Action<int(int, double y, double z)> a = [](int i, Unused, Unused) {
    return 2 * i;
  };
  std::tuple<int, double, double> dummy = std::make_tuple(3, 7.3, 9.44);
  EXPECT_EQ(6, a.Perform(dummy));
}

// Test that basic built-in actions work with move-only arguments.
TEST(MoveOnlyArgumentsTest, ReturningActions) {
  Action<int(std::unique_ptr<int>)> a = Return(1);
  EXPECT_EQ(1, a.Perform(std::make_tuple(nullptr)));

  a = testing::WithoutArgs([]() { return 7; });
  EXPECT_EQ(7, a.Perform(std::make_tuple(nullptr)));

  Action<void(std::unique_ptr<int>, int*)> a2 = testing::SetArgPointee<1>(3);
  int x = 0;
  a2.Perform(std::make_tuple(nullptr, &x));
  EXPECT_EQ(x, 3);
}

ACTION(ReturnArity) { return std::tuple_size<args_type>::value; }

TEST(ActionMacro, LargeArity) {
  EXPECT_EQ(
      1, testing::Action<int(int)>(ReturnArity()).Perform(std::make_tuple(0)));
  EXPECT_EQ(
      10,
      testing::Action<int(int, int, int, int, int, int, int, int, int, int)>(
          ReturnArity())
          .Perform(std::make_tuple(0, 1, 2, 3, 4, 5, 6, 7, 8, 9)));
  EXPECT_EQ(
      20,
      testing::Action<int(int, int, int, int, int, int, int, int, int, int, int,
                          int, int, int, int, int, int, int, int, int)>(
          ReturnArity())
          .Perform(std::make_tuple(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
                                   14, 15, 16, 17, 18, 19)));
}

}  // namespace
}  // namespace testing

#if defined(_MSC_VER) && (_MSC_VER == 1900)
GTEST_DISABLE_MSC_WARNINGS_POP_()  // 4800
#endif
GTEST_DISABLE_MSC_WARNINGS_POP_()  // 4100 4503
