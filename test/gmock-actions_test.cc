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
// This file tests the built-in actions.

#include <gmock/gmock-actions.h>
#include <algorithm>
#include <iterator>
#include <string>
#include <gmock/gmock.h>
#include <gmock/internal/gmock-port.h>
#include <gtest/gtest.h>
#include <gtest/gtest-spi.h>

namespace {

using ::std::tr1::get;
using ::std::tr1::make_tuple;
using ::std::tr1::tuple;
using ::std::tr1::tuple_element;
using testing::internal::BuiltInDefaultValue;
using testing::internal::Int64;
using testing::internal::UInt64;
// This list should be kept sorted.
using testing::_;
using testing::Action;
using testing::ActionInterface;
using testing::Assign;
using testing::DefaultValue;
using testing::DoDefault;
using testing::IgnoreResult;
using testing::Invoke;
using testing::InvokeWithoutArgs;
using testing::MakePolymorphicAction;
using testing::Ne;
using testing::PolymorphicAction;
using testing::Return;
using testing::ReturnNull;
using testing::ReturnRef;
using testing::SetArgumentPointee;
using testing::SetArrayArgument;

#ifndef _WIN32_WCE
using testing::SetErrnoAndReturn;
#endif  // _WIN32_WCE

#if GMOCK_HAS_PROTOBUF_
using testing::internal::TestMessage;
#endif  // GMOCK_HAS_PROTOBUF_

// Tests that BuiltInDefaultValue<T*>::Get() returns NULL.
TEST(BuiltInDefaultValueTest, IsNullForPointerTypes) {
  EXPECT_TRUE(BuiltInDefaultValue<int*>::Get() == NULL);
  EXPECT_TRUE(BuiltInDefaultValue<const char*>::Get() == NULL);
  EXPECT_TRUE(BuiltInDefaultValue<void*>::Get() == NULL);
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
  EXPECT_EQ(0, BuiltInDefaultValue<unsigned char>::Get());
  EXPECT_EQ(0, BuiltInDefaultValue<signed char>::Get());
  EXPECT_EQ(0, BuiltInDefaultValue<char>::Get());
#if !GTEST_OS_WINDOWS
  EXPECT_EQ(0, BuiltInDefaultValue<unsigned wchar_t>::Get());
  EXPECT_EQ(0, BuiltInDefaultValue<signed wchar_t>::Get());
#endif  // !GTEST_OS_WINDOWS
  EXPECT_EQ(0, BuiltInDefaultValue<wchar_t>::Get());
  EXPECT_EQ(0, BuiltInDefaultValue<unsigned short>::Get());  // NOLINT
  EXPECT_EQ(0, BuiltInDefaultValue<signed short>::Get());  // NOLINT
  EXPECT_EQ(0, BuiltInDefaultValue<short>::Get());  // NOLINT
  EXPECT_EQ(0, BuiltInDefaultValue<unsigned int>::Get());
  EXPECT_EQ(0, BuiltInDefaultValue<signed int>::Get());
  EXPECT_EQ(0, BuiltInDefaultValue<int>::Get());
  EXPECT_EQ(0, BuiltInDefaultValue<unsigned long>::Get());  // NOLINT
  EXPECT_EQ(0, BuiltInDefaultValue<signed long>::Get());  // NOLINT
  EXPECT_EQ(0, BuiltInDefaultValue<long>::Get());  // NOLINT
  EXPECT_EQ(0, BuiltInDefaultValue<UInt64>::Get());
  EXPECT_EQ(0, BuiltInDefaultValue<Int64>::Get());
  EXPECT_EQ(0, BuiltInDefaultValue<float>::Get());
  EXPECT_EQ(0, BuiltInDefaultValue<double>::Get());
}

// Tests that BuiltInDefaultValue<T>::Exists() returns true when T is a
// built-in numeric type.
TEST(BuiltInDefaultValueTest, ExistsForNumericTypes) {
  EXPECT_TRUE(BuiltInDefaultValue<unsigned char>::Exists());
  EXPECT_TRUE(BuiltInDefaultValue<signed char>::Exists());
  EXPECT_TRUE(BuiltInDefaultValue<char>::Exists());
#if !GTEST_OS_WINDOWS
  EXPECT_TRUE(BuiltInDefaultValue<unsigned wchar_t>::Exists());
  EXPECT_TRUE(BuiltInDefaultValue<signed wchar_t>::Exists());
#endif  // !GTEST_OS_WINDOWS
  EXPECT_TRUE(BuiltInDefaultValue<wchar_t>::Exists());
  EXPECT_TRUE(BuiltInDefaultValue<unsigned short>::Exists());  // NOLINT
  EXPECT_TRUE(BuiltInDefaultValue<signed short>::Exists());  // NOLINT
  EXPECT_TRUE(BuiltInDefaultValue<short>::Exists());  // NOLINT
  EXPECT_TRUE(BuiltInDefaultValue<unsigned int>::Exists());
  EXPECT_TRUE(BuiltInDefaultValue<signed int>::Exists());
  EXPECT_TRUE(BuiltInDefaultValue<int>::Exists());
  EXPECT_TRUE(BuiltInDefaultValue<unsigned long>::Exists());  // NOLINT
  EXPECT_TRUE(BuiltInDefaultValue<signed long>::Exists());  // NOLINT
  EXPECT_TRUE(BuiltInDefaultValue<long>::Exists());  // NOLINT
  EXPECT_TRUE(BuiltInDefaultValue<UInt64>::Exists());
  EXPECT_TRUE(BuiltInDefaultValue<Int64>::Exists());
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
#if GTEST_HAS_GLOBAL_STRING
  EXPECT_EQ("", BuiltInDefaultValue< ::string>::Get());
#endif  // GTEST_HAS_GLOBAL_STRING

#if GTEST_HAS_STD_STRING
  EXPECT_EQ("", BuiltInDefaultValue< ::std::string>::Get());
#endif  // GTEST_HAS_STD_STRING
}

// Tests that BuiltInDefaultValue<T>::Exists() returns true when T is a
// string type.
TEST(BuiltInDefaultValueTest, ExistsForString) {
#if GTEST_HAS_GLOBAL_STRING
  EXPECT_TRUE(BuiltInDefaultValue< ::string>::Exists());
#endif  // GTEST_HAS_GLOBAL_STRING

#if GTEST_HAS_STD_STRING
  EXPECT_TRUE(BuiltInDefaultValue< ::std::string>::Exists());
#endif  // GTEST_HAS_STD_STRING
}

// Tests that BuiltInDefaultValue<const T>::Get() returns the same
// value as BuiltInDefaultValue<T>::Get() does.
TEST(BuiltInDefaultValueTest, WorksForConstTypes) {
  EXPECT_EQ("", BuiltInDefaultValue<const std::string>::Get());
  EXPECT_EQ(0, BuiltInDefaultValue<const int>::Get());
  EXPECT_TRUE(BuiltInDefaultValue<char* const>::Get() == NULL);
  EXPECT_FALSE(BuiltInDefaultValue<const bool>::Get());
}

// Tests that BuiltInDefaultValue<T>::Get() aborts the program with
// the correct error message when T is a user-defined type.
struct UserType {
  UserType() : value(0) {}

  int value;
};

TEST(BuiltInDefaultValueTest, UserTypeHasNoDefault) {
  EXPECT_FALSE(BuiltInDefaultValue<UserType>::Exists());
}

#if GTEST_HAS_DEATH_TEST

// Tests that BuiltInDefaultValue<T&>::Get() aborts the program.
TEST(BuiltInDefaultValueDeathTest, IsUndefinedForReferences) {
  EXPECT_DEATH({  // NOLINT
    BuiltInDefaultValue<int&>::Get();
  }, "");
  EXPECT_DEATH({  // NOLINT
    BuiltInDefaultValue<const char&>::Get();
  }, "");
}

TEST(BuiltInDefaultValueDeathTest, IsUndefinedForUserTypes) {
  EXPECT_DEATH({  // NOLINT
    BuiltInDefaultValue<UserType>::Get();
  }, "");
}

#endif  // GTEST_HAS_DEATH_TEST

// Tests that DefaultValue<T>::IsSet() is false initially.
TEST(DefaultValueTest, IsInitiallyUnset) {
  EXPECT_FALSE(DefaultValue<int>::IsSet());
  EXPECT_FALSE(DefaultValue<const UserType>::IsSet());
}

// Tests that DefaultValue<T> can be set and then unset.
TEST(DefaultValueTest, CanBeSetAndUnset) {
  EXPECT_TRUE(DefaultValue<int>::Exists());
  EXPECT_FALSE(DefaultValue<const UserType>::Exists());

  DefaultValue<int>::Set(1);
  DefaultValue<const UserType>::Set(UserType());

  EXPECT_EQ(1, DefaultValue<int>::Get());
  EXPECT_EQ(0, DefaultValue<const UserType>::Get().value);

  EXPECT_TRUE(DefaultValue<int>::Exists());
  EXPECT_TRUE(DefaultValue<const UserType>::Exists());

  DefaultValue<int>::Clear();
  DefaultValue<const UserType>::Clear();

  EXPECT_FALSE(DefaultValue<int>::IsSet());
  EXPECT_FALSE(DefaultValue<const UserType>::IsSet());

  EXPECT_TRUE(DefaultValue<int>::Exists());
  EXPECT_FALSE(DefaultValue<const UserType>::Exists());
}

// Tests that DefaultValue<T>::Get() returns the
// BuiltInDefaultValue<T>::Get() when DefaultValue<T>::IsSet() is
// false.
TEST(DefaultValueDeathTest, GetReturnsBuiltInDefaultValueWhenUnset) {
  EXPECT_FALSE(DefaultValue<int>::IsSet());
  EXPECT_TRUE(DefaultValue<int>::Exists());
  EXPECT_FALSE(DefaultValue<UserType>::IsSet());
  EXPECT_FALSE(DefaultValue<UserType>::Exists());

  EXPECT_EQ(0, DefaultValue<int>::Get());

#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH({  // NOLINT
    DefaultValue<UserType>::Get();
  }, "");
#endif  // GTEST_HAS_DEATH_TEST
}

// Tests that DefaultValue<void>::Get() returns void.
TEST(DefaultValueTest, GetWorksForVoid) {
  return DefaultValue<void>::Get();
}

// Tests using DefaultValue with a reference type.

// Tests that DefaultValue<T&>::IsSet() is false initially.
TEST(DefaultValueOfReferenceTest, IsInitiallyUnset) {
  EXPECT_FALSE(DefaultValue<int&>::IsSet());
  EXPECT_FALSE(DefaultValue<UserType&>::IsSet());
}

// Tests that DefaultValue<T&>::Exists is false initiallly.
TEST(DefaultValueOfReferenceTest, IsInitiallyNotExisting) {
  EXPECT_FALSE(DefaultValue<int&>::Exists());
  EXPECT_FALSE(DefaultValue<UserType&>::Exists());
}

// Tests that DefaultValue<T&> can be set and then unset.
TEST(DefaultValueOfReferenceTest, CanBeSetAndUnset) {
  int n = 1;
  DefaultValue<const int&>::Set(n);
  UserType u;
  DefaultValue<UserType&>::Set(u);

  EXPECT_TRUE(DefaultValue<const int&>::Exists());
  EXPECT_TRUE(DefaultValue<UserType&>::Exists());

  EXPECT_EQ(&n, &(DefaultValue<const int&>::Get()));
  EXPECT_EQ(&u, &(DefaultValue<UserType&>::Get()));

  DefaultValue<const int&>::Clear();
  DefaultValue<UserType&>::Clear();

  EXPECT_FALSE(DefaultValue<const int&>::Exists());
  EXPECT_FALSE(DefaultValue<UserType&>::Exists());

  EXPECT_FALSE(DefaultValue<const int&>::IsSet());
  EXPECT_FALSE(DefaultValue<UserType&>::IsSet());
}

// Tests that DefaultValue<T&>::Get() returns the
// BuiltInDefaultValue<T&>::Get() when DefaultValue<T&>::IsSet() is
// false.
TEST(DefaultValueOfReferenceDeathTest, GetReturnsBuiltInDefaultValueWhenUnset) {
  EXPECT_FALSE(DefaultValue<int&>::IsSet());
  EXPECT_FALSE(DefaultValue<UserType&>::IsSet());

#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH({  // NOLINT
    DefaultValue<int&>::Get();
  }, "");
  EXPECT_DEATH({  // NOLINT
    DefaultValue<UserType>::Get();
  }, "");
#endif  // GTEST_HAS_DEATH_TEST
}

// Tests that ActionInterface can be implemented by defining the
// Perform method.

typedef int MyFunction(bool, int);

class MyActionImpl : public ActionInterface<MyFunction> {
 public:
  virtual int Perform(const tuple<bool, int>& args) {
    return get<0>(args) ? get<1>(args) : 0;
  }
};

TEST(ActionInterfaceTest, CanBeImplementedByDefiningPerform) {
  MyActionImpl my_action_impl;

  EXPECT_FALSE(my_action_impl.IsDoDefault());
}

TEST(ActionInterfaceTest, MakeAction) {
  Action<MyFunction> action = MakeAction(new MyActionImpl);

  // When exercising the Perform() method of Action<F>, we must pass
  // it a tuple whose size and type are compatible with F's argument
  // types.  For example, if F is int(), then Perform() takes a
  // 0-tuple; if F is void(bool, int), then Perform() takes a
  // tuple<bool, int>, and so on.
  EXPECT_EQ(5, action.Perform(make_tuple(true, 5)));
}

// Tests that Action<F> can be contructed from a pointer to
// ActionInterface<F>.
TEST(ActionTest, CanBeConstructedFromActionInterface) {
  Action<MyFunction> action(new MyActionImpl);
}

// Tests that Action<F> delegates actual work to ActionInterface<F>.
TEST(ActionTest, DelegatesWorkToActionInterface) {
  const Action<MyFunction> action(new MyActionImpl);

  EXPECT_EQ(5, action.Perform(make_tuple(true, 5)));
  EXPECT_EQ(0, action.Perform(make_tuple(false, 1)));
}

// Tests that Action<F> can be copied.
TEST(ActionTest, IsCopyable) {
  Action<MyFunction> a1(new MyActionImpl);
  Action<MyFunction> a2(a1);  // Tests the copy constructor.

  // a1 should continue to work after being copied from.
  EXPECT_EQ(5, a1.Perform(make_tuple(true, 5)));
  EXPECT_EQ(0, a1.Perform(make_tuple(false, 1)));

  // a2 should work like the action it was copied from.
  EXPECT_EQ(5, a2.Perform(make_tuple(true, 5)));
  EXPECT_EQ(0, a2.Perform(make_tuple(false, 1)));

  a2 = a1;  // Tests the assignment operator.

  // a1 should continue to work after being copied from.
  EXPECT_EQ(5, a1.Perform(make_tuple(true, 5)));
  EXPECT_EQ(0, a1.Perform(make_tuple(false, 1)));

  // a2 should work like the action it was copied from.
  EXPECT_EQ(5, a2.Perform(make_tuple(true, 5)));
  EXPECT_EQ(0, a2.Perform(make_tuple(false, 1)));
}

// Tests that an Action<From> object can be converted to a
// compatible Action<To> object.

class IsNotZero : public ActionInterface<bool(int)> {  // NOLINT
 public:
  virtual bool Perform(const tuple<int>& arg) {
    return get<0>(arg) != 0;
  }
};

TEST(ActionTest, CanBeConvertedToOtherActionType) {
  const Action<bool(int)> a1(new IsNotZero);  // NOLINT
  const Action<int(char)> a2 = Action<int(char)>(a1);  // NOLINT
  EXPECT_EQ(1, a2.Perform(make_tuple('a')));
  EXPECT_EQ(0, a2.Perform(make_tuple('\0')));
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
  Result Perform(const ArgumentTuple& args) { return get<1>(args); }
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
  Result Perform(const tuple<>&) const { return 0; }
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
  EXPECT_EQ(5, a1.Perform(make_tuple(false, 5, 2.0)));
}

// Tests that MakePolymorphicAction() works when the implementation
// class' Perform() method template has only one template parameter.
TEST(MakePolymorphicActionTest, WorksWhenPerformHasOneTemplateParameter) {
  Action<int()> a1 = ReturnZeroFromNullaryFunction();
  EXPECT_EQ(0, a1.Perform(make_tuple()));

  Action<void*()> a2 = ReturnZeroFromNullaryFunction();
  EXPECT_TRUE(a2.Perform(make_tuple()) == NULL);
}

// Tests that Return() works as an action for void-returning
// functions.
TEST(ReturnTest, WorksForVoid) {
  const Action<void(int)> ret = Return();  // NOLINT
  return ret.Perform(make_tuple(1));
}

// Tests that Return(v) returns v.
TEST(ReturnTest, ReturnsGivenValue) {
  Action<int()> ret = Return(1);  // NOLINT
  EXPECT_EQ(1, ret.Perform(make_tuple()));

  ret = Return(-5);
  EXPECT_EQ(-5, ret.Perform(make_tuple()));
}

// Tests that Return("string literal") works.
TEST(ReturnTest, AcceptsStringLiteral) {
  Action<const char*()> a1 = Return("Hello");
  EXPECT_STREQ("Hello", a1.Perform(make_tuple()));

  Action<std::string()> a2 = Return("world");
  EXPECT_EQ("world", a2.Perform(make_tuple()));
}

// Tests that Return(v) is covaraint.

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
  EXPECT_EQ(&base, ret.Perform(make_tuple()));

  ret = Return(&derived);
  EXPECT_EQ(&derived, ret.Perform(make_tuple()));
}

// Tests that ReturnNull() returns NULL in a pointer-returning function.
TEST(ReturnNullTest, WorksInPointerReturningFunction) {
  const Action<int*()> a1 = ReturnNull();
  EXPECT_TRUE(a1.Perform(make_tuple()) == NULL);

  const Action<const char*(bool)> a2 = ReturnNull();  // NOLINT
  EXPECT_TRUE(a2.Perform(make_tuple(true)) == NULL);
}

// Tests that ReturnRef(v) works for reference types.
TEST(ReturnRefTest, WorksForReference) {
  const int n = 0;
  const Action<const int&(bool)> ret = ReturnRef(n);  // NOLINT

  EXPECT_EQ(&n, &ret.Perform(make_tuple(true)));
}

// Tests that ReturnRef(v) is covariant.
TEST(ReturnRefTest, IsCovariant) {
  Base base;
  Derived derived;
  Action<Base&()> a = ReturnRef(base);
  EXPECT_EQ(&base, &a.Perform(make_tuple()));

  a = ReturnRef(derived);
  EXPECT_EQ(&derived, &a.Perform(make_tuple()));
}

// Tests that DoDefault() does the default action for the mock method.

class MyClass {};

class MockClass {
 public:
  MOCK_METHOD1(IntFunc, int(bool flag));  // NOLINT
  MOCK_METHOD0(Foo, MyClass());
};

// Tests that DoDefault() returns the built-in default value for the
// return type by default.
TEST(DoDefaultTest, ReturnsBuiltInDefaultValueByDefault) {
  MockClass mock;
  EXPECT_CALL(mock, IntFunc(_))
      .WillOnce(DoDefault());
  EXPECT_EQ(0, mock.IntFunc(true));
}

#if GTEST_HAS_DEATH_TEST

// Tests that DoDefault() aborts the process when there is no built-in
// default value for the return type.
TEST(DoDefaultDeathTest, DiesForUnknowType) {
  MockClass mock;
  EXPECT_CALL(mock, Foo())
      .WillRepeatedly(DoDefault());
  EXPECT_DEATH({  // NOLINT
    mock.Foo();
  }, "");
}

// Tests that using DoDefault() inside a composite action leads to a
// run-time error.

void VoidFunc(bool flag) {}

TEST(DoDefaultDeathTest, DiesIfUsedInCompositeAction) {
  MockClass mock;
  EXPECT_CALL(mock, IntFunc(_))
      .WillRepeatedly(DoAll(Invoke(VoidFunc),
                            DoDefault()));

  // Ideally we should verify the error message as well.  Sadly,
  // EXPECT_DEATH() can only capture stderr, while Google Mock's
  // errors are printed on stdout.  Therefore we have to settle for
  // not verifying the message.
  EXPECT_DEATH({  // NOLINT
    mock.IntFunc(true);
  }, "");
}

#endif  // GTEST_HAS_DEATH_TEST

// Tests that DoDefault() returns the default value set by
// DefaultValue<T>::Set() when it's not overriden by an ON_CALL().
TEST(DoDefaultTest, ReturnsUserSpecifiedPerTypeDefaultValueWhenThereIsOne) {
  DefaultValue<int>::Set(1);
  MockClass mock;
  EXPECT_CALL(mock, IntFunc(_))
      .WillOnce(DoDefault());
  EXPECT_EQ(1, mock.IntFunc(false));
  DefaultValue<int>::Clear();
}

// Tests that DoDefault() does the action specified by ON_CALL().
TEST(DoDefaultTest, DoesWhatOnCallSpecifies) {
  MockClass mock;
  ON_CALL(mock, IntFunc(_))
      .WillByDefault(Return(2));
  EXPECT_CALL(mock, IntFunc(_))
      .WillOnce(DoDefault());
  EXPECT_EQ(2, mock.IntFunc(false));
}

// Tests that using DoDefault() in ON_CALL() leads to a run-time failure.
TEST(DoDefaultTest, CannotBeUsedInOnCall) {
  MockClass mock;
  EXPECT_NONFATAL_FAILURE({  // NOLINT
    ON_CALL(mock, IntFunc(_))
      .WillByDefault(DoDefault());
  }, "DoDefault() cannot be used in ON_CALL()");
}

// Tests that SetArgumentPointee<N>(v) sets the variable pointed to by
// the N-th (0-based) argument to v.
TEST(SetArgumentPointeeTest, SetsTheNthPointee) {
  typedef void MyFunction(bool, int*, char*);
  Action<MyFunction> a = SetArgumentPointee<1>(2);

  int n = 0;
  char ch = '\0';
  a.Perform(make_tuple(true, &n, &ch));
  EXPECT_EQ(2, n);
  EXPECT_EQ('\0', ch);

  a = SetArgumentPointee<2>('a');
  n = 0;
  ch = '\0';
  a.Perform(make_tuple(true, &n, &ch));
  EXPECT_EQ(0, n);
  EXPECT_EQ('a', ch);
}

#if GMOCK_HAS_PROTOBUF_

// Tests that SetArgumentPointee<N>(proto_buffer) sets the v1 protobuf
// variable pointed to by the N-th (0-based) argument to proto_buffer.
TEST(SetArgumentPointeeTest, SetsTheNthPointeeOfProtoBufferType) {
  TestMessage* const msg = new TestMessage;
  msg->set_member("yes");
  TestMessage orig_msg;
  orig_msg.CopyFrom(*msg);

  Action<void(bool, TestMessage*)> a = SetArgumentPointee<1>(*msg);
  // SetArgumentPointee<N>(proto_buffer) makes a copy of proto_buffer
  // s.t. the action works even when the original proto_buffer has
  // died.  We ensure this behavior by deleting msg before using the
  // action.
  delete msg;

  TestMessage dest;
  EXPECT_FALSE(orig_msg.Equals(dest));
  a.Perform(make_tuple(true, &dest));
  EXPECT_TRUE(orig_msg.Equals(dest));
}

// Tests that SetArgumentPointee<N>(proto_buffer) sets the
// ::ProtocolMessage variable pointed to by the N-th (0-based)
// argument to proto_buffer.
TEST(SetArgumentPointeeTest, SetsTheNthPointeeOfProtoBufferBaseType) {
  TestMessage* const msg = new TestMessage;
  msg->set_member("yes");
  TestMessage orig_msg;
  orig_msg.CopyFrom(*msg);

  Action<void(bool, ::ProtocolMessage*)> a = SetArgumentPointee<1>(*msg);
  // SetArgumentPointee<N>(proto_buffer) makes a copy of proto_buffer
  // s.t. the action works even when the original proto_buffer has
  // died.  We ensure this behavior by deleting msg before using the
  // action.
  delete msg;

  TestMessage dest;
  ::ProtocolMessage* const dest_base = &dest;
  EXPECT_FALSE(orig_msg.Equals(dest));
  a.Perform(make_tuple(true, dest_base));
  EXPECT_TRUE(orig_msg.Equals(dest));
}

// Tests that SetArgumentPointee<N>(proto2_buffer) sets the v2
// protobuf variable pointed to by the N-th (0-based) argument to
// proto2_buffer.
TEST(SetArgumentPointeeTest, SetsTheNthPointeeOfProto2BufferType) {
  using testing::internal::FooMessage;
  FooMessage* const msg = new FooMessage;
  msg->set_int_field(2);
  msg->set_string_field("hi");
  FooMessage orig_msg;
  orig_msg.CopyFrom(*msg);

  Action<void(bool, FooMessage*)> a = SetArgumentPointee<1>(*msg);
  // SetArgumentPointee<N>(proto2_buffer) makes a copy of
  // proto2_buffer s.t. the action works even when the original
  // proto2_buffer has died.  We ensure this behavior by deleting msg
  // before using the action.
  delete msg;

  FooMessage dest;
  dest.set_int_field(0);
  a.Perform(make_tuple(true, &dest));
  EXPECT_EQ(2, dest.int_field());
  EXPECT_EQ("hi", dest.string_field());
}

// Tests that SetArgumentPointee<N>(proto2_buffer) sets the
// proto2::Message variable pointed to by the N-th (0-based) argument
// to proto2_buffer.
TEST(SetArgumentPointeeTest, SetsTheNthPointeeOfProto2BufferBaseType) {
  using testing::internal::FooMessage;
  FooMessage* const msg = new FooMessage;
  msg->set_int_field(2);
  msg->set_string_field("hi");
  FooMessage orig_msg;
  orig_msg.CopyFrom(*msg);

  Action<void(bool, ::proto2::Message*)> a = SetArgumentPointee<1>(*msg);
  // SetArgumentPointee<N>(proto2_buffer) makes a copy of
  // proto2_buffer s.t. the action works even when the original
  // proto2_buffer has died.  We ensure this behavior by deleting msg
  // before using the action.
  delete msg;

  FooMessage dest;
  dest.set_int_field(0);
  ::proto2::Message* const dest_base = &dest;
  a.Perform(make_tuple(true, dest_base));
  EXPECT_EQ(2, dest.int_field());
  EXPECT_EQ("hi", dest.string_field());
}

#endif  // GMOCK_HAS_PROTOBUF_

// Tests that SetArrayArgument<N>(first, last) sets the elements of the array
// pointed to by the N-th (0-based) argument to values in range [first, last).
TEST(SetArrayArgumentTest, SetsTheNthArray) {
  typedef void MyFunction(bool, int*, char*);
  int numbers[] = { 1, 2, 3 };
  Action<MyFunction> a = SetArrayArgument<1>(numbers, numbers + 3);

  int n[4] = {};
  int* pn = n;
  char ch[4] = {};
  char* pch = ch;
  a.Perform(make_tuple(true, pn, pch));
  EXPECT_EQ(1, n[0]);
  EXPECT_EQ(2, n[1]);
  EXPECT_EQ(3, n[2]);
  EXPECT_EQ(0, n[3]);
  EXPECT_EQ('\0', ch[0]);
  EXPECT_EQ('\0', ch[1]);
  EXPECT_EQ('\0', ch[2]);
  EXPECT_EQ('\0', ch[3]);

  // Tests first and last are iterators.
  std::string letters = "abc";
  a = SetArrayArgument<2>(letters.begin(), letters.end());
  std::fill_n(n, 4, 0);
  std::fill_n(ch, 4, '\0');
  a.Perform(make_tuple(true, pn, pch));
  EXPECT_EQ(0, n[0]);
  EXPECT_EQ(0, n[1]);
  EXPECT_EQ(0, n[2]);
  EXPECT_EQ(0, n[3]);
  EXPECT_EQ('a', ch[0]);
  EXPECT_EQ('b', ch[1]);
  EXPECT_EQ('c', ch[2]);
  EXPECT_EQ('\0', ch[3]);
}

// Tests SetArrayArgument<N>(first, last) where first == last.
TEST(SetArrayArgumentTest, SetsTheNthArrayWithEmptyRange) {
  typedef void MyFunction(bool, int*);
  int numbers[] = { 1, 2, 3 };
  Action<MyFunction> a = SetArrayArgument<1>(numbers, numbers);

  int n[4] = {};
  int* pn = n;
  a.Perform(make_tuple(true, pn));
  EXPECT_EQ(0, n[0]);
  EXPECT_EQ(0, n[1]);
  EXPECT_EQ(0, n[2]);
  EXPECT_EQ(0, n[3]);
}

// Tests SetArrayArgument<N>(first, last) where *first is convertible
// (but not equal) to the argument type.
TEST(SetArrayArgumentTest, SetsTheNthArrayWithConvertibleType) {
  typedef void MyFunction(bool, char*);
  int codes[] = { 97, 98, 99 };
  Action<MyFunction> a = SetArrayArgument<1>(codes, codes + 3);

  char ch[4] = {};
  char* pch = ch;
  a.Perform(make_tuple(true, pch));
  EXPECT_EQ('a', ch[0]);
  EXPECT_EQ('b', ch[1]);
  EXPECT_EQ('c', ch[2]);
  EXPECT_EQ('\0', ch[3]);
}

// Test SetArrayArgument<N>(first, last) with iterator as argument.
TEST(SetArrayArgumentTest, SetsTheNthArrayWithIteratorArgument) {
  typedef void MyFunction(bool, std::back_insert_iterator<std::string>);
  std::string letters = "abc";
  Action<MyFunction> a = SetArrayArgument<1>(letters.begin(), letters.end());

  std::string s;
  a.Perform(make_tuple(true, back_inserter(s)));
  EXPECT_EQ(letters, s);
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

bool Unary(int x) { return x < 0; }

const char* Plus1(const char* s) { return s + 1; }

void VoidUnary(int n) { g_done = true; }

bool ByConstRef(const std::string& s) { return s == "Hi"; }

const double g_double = 0;
bool ReferencesGlobalDouble(const double& x) { return &x == &g_double; }

std::string ByNonConstRef(std::string& s) { return s += "+"; }  // NOLINT

struct UnaryFunctor {
  int operator()(bool x) { return x ? 1 : -1; }
};

const char* Binary(const char* input, short n) { return input + n; }  // NOLINT

void VoidBinary(int, char) { g_done = true; }

int Ternary(int x, char y, short z) { return x + y + z; }  // NOLINT

void VoidTernary(int, char, bool) { g_done = true; }

int SumOf4(int a, int b, int c, int d) { return a + b + c + d; }

void VoidFunctionWithFourArguments(char, int, float, double) { g_done = true; }

int SumOf5(int a, int b, int c, int d, int e) { return a + b + c + d + e; }

struct SumOf5Functor {
  int operator()(int a, int b, int c, int d, int e) {
    return a + b + c + d + e;
  }
};

int SumOf6(int a, int b, int c, int d, int e, int f) {
  return a + b + c + d + e + f;
}

struct SumOf6Functor {
  int operator()(int a, int b, int c, int d, int e, int f) {
    return a + b + c + d + e + f;
  }
};

class Foo {
 public:
  Foo() : value_(123) {}

  int Nullary() const { return value_; }
  short Unary(long x) { return static_cast<short>(value_ + x); }  // NOLINT
  std::string Binary(const std::string& str, char c) const { return str + c; }
  int Ternary(int x, bool y, char z) { return value_ + x + y*z; }
  int SumOf4(int a, int b, int c, int d) const {
    return a + b + c + d + value_;
  }
  int SumOf5(int a, int b, int c, int d, int e) { return a + b + c + d + e; }
  int SumOf6(int a, int b, int c, int d, int e, int f) {
    return a + b + c + d + e + f;
  }
 private:
  int value_;
};

// Tests InvokeWithoutArgs(function).
TEST(InvokeWithoutArgsTest, Function) {
  // As an action that takes one argument.
  Action<int(int)> a = InvokeWithoutArgs(Nullary);  // NOLINT
  EXPECT_EQ(1, a.Perform(make_tuple(2)));

  // As an action that takes two arguments.
  Action<short(int, double)> a2 = InvokeWithoutArgs(Nullary);  // NOLINT
  EXPECT_EQ(1, a2.Perform(make_tuple(2, 3.5)));

  // As an action that returns void.
  Action<void(int)> a3 = InvokeWithoutArgs(VoidNullary);  // NOLINT
  g_done = false;
  a3.Perform(make_tuple(1));
  EXPECT_TRUE(g_done);
}

// Tests InvokeWithoutArgs(functor).
TEST(InvokeWithoutArgsTest, Functor) {
  // As an action that takes no argument.
  Action<int()> a = InvokeWithoutArgs(NullaryFunctor());  // NOLINT
  EXPECT_EQ(2, a.Perform(make_tuple()));

  // As an action that takes three arguments.
  Action<short(int, double, char)> a2 =  // NOLINT
      InvokeWithoutArgs(NullaryFunctor());
  EXPECT_EQ(2, a2.Perform(make_tuple(3, 3.5, 'a')));

  // As an action that returns void.
  Action<void()> a3 = InvokeWithoutArgs(VoidNullaryFunctor());
  g_done = false;
  a3.Perform(make_tuple());
  EXPECT_TRUE(g_done);
}

// Tests InvokeWithoutArgs(obj_ptr, method).
TEST(InvokeWithoutArgsTest, Method) {
  Foo foo;
  Action<int(bool, char)> a =  // NOLINT
      InvokeWithoutArgs(&foo, &Foo::Nullary);
  EXPECT_EQ(123, a.Perform(make_tuple(true, 'a')));
}

// Tests using IgnoreResult() on a polymorphic action.
TEST(IgnoreResultTest, PolymorphicAction) {
  Action<void(int)> a = IgnoreResult(Return(5));  // NOLINT
  a.Perform(make_tuple(1));
}

// Tests using IgnoreResult() on a monomorphic action.

int ReturnOne() {
  g_done = true;
  return 1;
}

TEST(IgnoreResultTest, MonomorphicAction) {
  g_done = false;
  Action<void()> a = IgnoreResult(Invoke(ReturnOne));
  a.Perform(make_tuple());
  EXPECT_TRUE(g_done);
}

// Tests using IgnoreResult() on an action that returns a class type.

MyClass ReturnMyClass(double x) {
  g_done = true;
  return MyClass();
}

TEST(IgnoreResultTest, ActionReturningClass) {
  g_done = false;
  Action<void(int)> a = IgnoreResult(Invoke(ReturnMyClass));  // NOLINT
  a.Perform(make_tuple(2));
  EXPECT_TRUE(g_done);
}

TEST(AssignTest, Int) {
  int x = 0;
  Action<void(int)> a = Assign(&x, 5);
  a.Perform(make_tuple(0));
  EXPECT_EQ(5, x);
}

TEST(AssignTest, String) {
  ::std::string x;
  Action<void(void)> a = Assign(&x, "Hello, world");
  a.Perform(make_tuple());
  EXPECT_EQ("Hello, world", x);
}

TEST(AssignTest, CompatibleTypes) {
  double x = 0;
  Action<void(int)> a = Assign(&x, 5);
  a.Perform(make_tuple(0));
  EXPECT_DOUBLE_EQ(5, x);
}

#ifndef _WIN32_WCE

class SetErrnoAndReturnTest : public testing::Test {
 protected:
  virtual void SetUp() { errno = 0; }
  virtual void TearDown() { errno = 0; }
};

TEST_F(SetErrnoAndReturnTest, Int) {
  Action<int(void)> a = SetErrnoAndReturn(ENOTTY, -5);
  EXPECT_EQ(-5, a.Perform(make_tuple()));
  EXPECT_EQ(ENOTTY, errno);
}

TEST_F(SetErrnoAndReturnTest, Ptr) {
  int x;
  Action<int*(void)> a = SetErrnoAndReturn(ENOTTY, &x);
  EXPECT_EQ(&x, a.Perform(make_tuple()));
  EXPECT_EQ(ENOTTY, errno);
}

TEST_F(SetErrnoAndReturnTest, CompatibleTypes) {
  Action<double()> a = SetErrnoAndReturn(EINVAL, 5);
  EXPECT_DOUBLE_EQ(5.0, a.Perform(make_tuple()));
  EXPECT_EQ(EINVAL, errno);
}

#endif  // _WIN32_WCE

}  // Unnamed namespace
