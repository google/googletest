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
// This file tests the spec builder syntax.

#include <gmock/gmock-spec-builders.h>

#include <ostream>  // NOLINT
#include <sstream>
#include <string>

#include <gmock/gmock.h>
#include <gmock/internal/gmock-port.h>
#include <gtest/gtest.h>
#include <gtest/gtest-spi.h>

namespace testing {
namespace internal {

// Helper class for testing the Expectation class template.
class ExpectationTester {
 public:
  // Sets the call count of the given expectation to the given number.
  void SetCallCount(int n, ExpectationBase* exp) {
    exp->call_count_ = n;
  }
};

}  // namespace internal
}  // namespace testing

namespace {

using testing::_;
using testing::AnyNumber;
using testing::AtLeast;
using testing::AtMost;
using testing::Between;
using testing::Cardinality;
using testing::CardinalityInterface;
using testing::Const;
using testing::DoAll;
using testing::DoDefault;
using testing::GMOCK_FLAG(verbose);
using testing::Gt;
using testing::InSequence;
using testing::Invoke;
using testing::InvokeWithoutArgs;
using testing::IsSubstring;
using testing::Lt;
using testing::Message;
using testing::Mock;
using testing::Return;
using testing::Sequence;
using testing::internal::g_gmock_mutex;
using testing::internal::kErrorVerbosity;
using testing::internal::kInfoVerbosity;
using testing::internal::kWarningVerbosity;
using testing::internal::Expectation;
using testing::internal::ExpectationTester;
using testing::internal::string;

class Result {};

class MockA {
 public:
  MOCK_METHOD1(DoA, void(int n));  // NOLINT
  MOCK_METHOD1(ReturnResult, Result(int n));  // NOLINT
  MOCK_METHOD2(Binary, bool(int x, int y));  // NOLINT
  MOCK_METHOD2(ReturnInt, int(int x, int y));  // NOLINT
};

class MockB {
 public:
  MOCK_CONST_METHOD0(DoB, int());  // NOLINT
  MOCK_METHOD1(DoB, int(int n));  // NOLINT
};

// Tests that EXPECT_CALL and ON_CALL compile in a presence of macro
// redefining a mock method name. This could happen, for example, when
// the tested code #includes Win32 API headers which define many APIs
// as macros, e.g. #define TextOut TextOutW.

#define Method MethodW

class CC {
 public:
  virtual ~CC() {}
  virtual int Method() = 0;
};
class MockCC : public CC {
 public:
  MOCK_METHOD0(Method, int());
};

// Tests that a method with expanded name compiles.
TEST(OnCallSyntaxTest, CompilesWithMethodNameExpandedFromMacro) {
  MockCC cc;
  ON_CALL(cc, Method());
}

// Tests that the method with expanded name not only compiles but runs
// and returns a correct value, too.
TEST(OnCallSyntaxTest, WorksWithMethodNameExpandedFromMacro) {
  MockCC cc;
  ON_CALL(cc, Method()).WillByDefault(Return(42));
  EXPECT_EQ(42, cc.Method());
}

// Tests that a method with expanded name compiles.
TEST(ExpectCallSyntaxTest, CompilesWithMethodNameExpandedFromMacro) {
  MockCC cc;
  EXPECT_CALL(cc, Method());
  cc.Method();
}

// Tests that it works, too.
TEST(ExpectCallSyntaxTest, WorksWithMethodNameExpandedFromMacro) {
  MockCC cc;
  EXPECT_CALL(cc, Method()).WillOnce(Return(42));
  EXPECT_EQ(42, cc.Method());
}

#undef Method  // Done with macro redefinition tests.

// Tests that ON_CALL evaluates its arguments exactly once as promised
// by Google Mock.
TEST(OnCallSyntaxTest, EvaluatesFirstArgumentOnce) {
  MockA a;
  MockA* pa = &a;

  ON_CALL(*pa++, DoA(_));
  EXPECT_EQ(&a + 1, pa);
}

TEST(OnCallSyntaxTest, EvaluatesSecondArgumentOnce) {
  MockA a;
  int n = 0;

  ON_CALL(a, DoA(n++));
  EXPECT_EQ(1, n);
}

// Tests that the syntax of ON_CALL() is enforced at run time.

TEST(OnCallSyntaxTest, WithIsOptional) {
  MockA a;

  ON_CALL(a, DoA(5))
      .WillByDefault(Return());
  ON_CALL(a, DoA(_))
      .With(_)
      .WillByDefault(Return());
}

TEST(OnCallSyntaxTest, WithCanAppearAtMostOnce) {
  MockA a;

  EXPECT_NONFATAL_FAILURE({  // NOLINT
    ON_CALL(a, ReturnResult(_))
        .With(_)
        .With(_)
        .WillByDefault(Return(Result()));
  }, ".With() cannot appear more than once in an ON_CALL()");
}

#if GTEST_HAS_DEATH_TEST

TEST(OnCallSyntaxTest, WillByDefaultIsMandatory) {
  MockA a;

  EXPECT_DEATH({  // NOLINT
    ON_CALL(a, DoA(5));
    a.DoA(5);
  }, "");
}

#endif  // GTEST_HAS_DEATH_TEST

TEST(OnCallSyntaxTest, WillByDefaultCanAppearAtMostOnce) {
  MockA a;

  EXPECT_NONFATAL_FAILURE({  // NOLINT
    ON_CALL(a, DoA(5))
        .WillByDefault(Return())
        .WillByDefault(Return());
  }, ".WillByDefault() must appear exactly once in an ON_CALL()");
}

// Tests that EXPECT_CALL evaluates its arguments exactly once as
// promised by Google Mock.
TEST(ExpectCallSyntaxTest, EvaluatesFirstArgumentOnce) {
  MockA a;
  MockA* pa = &a;

  EXPECT_CALL(*pa++, DoA(_));
  a.DoA(0);
  EXPECT_EQ(&a + 1, pa);
}

TEST(ExpectCallSyntaxTest, EvaluatesSecondArgumentOnce) {
  MockA a;
  int n = 0;

  EXPECT_CALL(a, DoA(n++));
  a.DoA(0);
  EXPECT_EQ(1, n);
}

// Tests that the syntax of EXPECT_CALL() is enforced at run time.

TEST(ExpectCallSyntaxTest, WithIsOptional) {
  MockA a;

  EXPECT_CALL(a, DoA(5))
      .Times(0);
  EXPECT_CALL(a, DoA(6))
      .With(_)
      .Times(0);
}

TEST(ExpectCallSyntaxTest, WithCanAppearAtMostOnce) {
  MockA a;

  EXPECT_NONFATAL_FAILURE({  // NOLINT
    EXPECT_CALL(a, DoA(6))
        .With(_)
        .With(_);
  }, ".With() cannot appear more than once in an EXPECT_CALL()");

  a.DoA(6);
}

TEST(ExpectCallSyntaxTest, WithMustBeFirstClause) {
  MockA a;

  EXPECT_NONFATAL_FAILURE({  // NOLINT
    EXPECT_CALL(a, DoA(1))
        .Times(1)
        .With(_);
  }, ".With() must be the first clause in an EXPECT_CALL()");

  a.DoA(1);

  EXPECT_NONFATAL_FAILURE({  // NOLINT
    EXPECT_CALL(a, DoA(2))
        .WillOnce(Return())
        .With(_);
  }, ".With() must be the first clause in an EXPECT_CALL()");

  a.DoA(2);
}

TEST(ExpectCallSyntaxTest, TimesCanBeInferred) {
  MockA a;

  EXPECT_CALL(a, DoA(1))
      .WillOnce(Return());

  EXPECT_CALL(a, DoA(2))
      .WillOnce(Return())
      .WillRepeatedly(Return());

  a.DoA(1);
  a.DoA(2);
  a.DoA(2);
}

TEST(ExpectCallSyntaxTest, TimesCanAppearAtMostOnce) {
  MockA a;

  EXPECT_NONFATAL_FAILURE({  // NOLINT
    EXPECT_CALL(a, DoA(1))
        .Times(1)
        .Times(2);
  }, ".Times() cannot appear more than once in an EXPECT_CALL()");

  a.DoA(1);
  a.DoA(1);
}

TEST(ExpectCallSyntaxTest, TimesMustBeBeforeInSequence) {
  MockA a;
  Sequence s;

  EXPECT_NONFATAL_FAILURE({  // NOLINT
    EXPECT_CALL(a, DoA(1))
        .InSequence(s)
        .Times(1);
  }, ".Times() cannot appear after ");

  a.DoA(1);
}

TEST(ExpectCallSyntaxTest, InSequenceIsOptional) {
  MockA a;
  Sequence s;

  EXPECT_CALL(a, DoA(1));
  EXPECT_CALL(a, DoA(2))
      .InSequence(s);

  a.DoA(1);
  a.DoA(2);
}

TEST(ExpectCallSyntaxTest, InSequenceCanAppearMultipleTimes) {
  MockA a;
  Sequence s1, s2;

  EXPECT_CALL(a, DoA(1))
      .InSequence(s1, s2)
      .InSequence(s1);

  a.DoA(1);
}

TEST(ExpectCallSyntaxTest, InSequenceMustBeBeforeWill) {
  MockA a;
  Sequence s;

  EXPECT_NONFATAL_FAILURE({  // NOLINT
    EXPECT_CALL(a, DoA(1))
        .WillOnce(Return())
        .InSequence(s);
  }, ".InSequence() cannot appear after ");

  a.DoA(1);
}

TEST(ExpectCallSyntaxTest, WillIsOptional) {
  MockA a;

  EXPECT_CALL(a, DoA(1));
  EXPECT_CALL(a, DoA(2))
      .WillOnce(Return());

  a.DoA(1);
  a.DoA(2);
}

TEST(ExpectCallSyntaxTest, WillCanAppearMultipleTimes) {
  MockA a;

  EXPECT_CALL(a, DoA(1))
      .Times(AnyNumber())
      .WillOnce(Return())
      .WillOnce(Return())
      .WillOnce(Return());
}

TEST(ExpectCallSyntaxTest, WillMustBeBeforeWillRepeatedly) {
  MockA a;

  EXPECT_NONFATAL_FAILURE({  // NOLINT
    EXPECT_CALL(a, DoA(1))
        .WillRepeatedly(Return())
        .WillOnce(Return());
  }, ".WillOnce() cannot appear after ");

  a.DoA(1);
}

TEST(ExpectCallSyntaxTest, WillRepeatedlyIsOptional) {
  MockA a;

  EXPECT_CALL(a, DoA(1))
      .WillOnce(Return());
  EXPECT_CALL(a, DoA(2))
      .WillOnce(Return())
      .WillRepeatedly(Return());

  a.DoA(1);
  a.DoA(2);
  a.DoA(2);
}

TEST(ExpectCallSyntaxTest, WillRepeatedlyCannotAppearMultipleTimes) {
  MockA a;

  EXPECT_NONFATAL_FAILURE({  // NOLINT
    EXPECT_CALL(a, DoA(1))
        .WillRepeatedly(Return())
        .WillRepeatedly(Return());
  }, ".WillRepeatedly() cannot appear more than once in an "
     "EXPECT_CALL()");
}

TEST(ExpectCallSyntaxTest, WillRepeatedlyMustBeBeforeRetiresOnSaturation) {
  MockA a;

  EXPECT_NONFATAL_FAILURE({  // NOLINT
    EXPECT_CALL(a, DoA(1))
        .RetiresOnSaturation()
        .WillRepeatedly(Return());
  }, ".WillRepeatedly() cannot appear after ");
}

TEST(ExpectCallSyntaxTest, RetiresOnSaturationIsOptional) {
  MockA a;

  EXPECT_CALL(a, DoA(1));
  EXPECT_CALL(a, DoA(1))
      .RetiresOnSaturation();

  a.DoA(1);
  a.DoA(1);
}

TEST(ExpectCallSyntaxTest, RetiresOnSaturationCannotAppearMultipleTimes) {
  MockA a;

  EXPECT_NONFATAL_FAILURE({  // NOLINT
    EXPECT_CALL(a, DoA(1))
        .RetiresOnSaturation()
        .RetiresOnSaturation();
  }, ".RetiresOnSaturation() cannot appear more than once");

  a.DoA(1);
}

TEST(ExpectCallSyntaxTest, DefaultCardinalityIsOnce) {
  {
    MockA a;
    EXPECT_CALL(a, DoA(1));
    a.DoA(1);
  }
  EXPECT_NONFATAL_FAILURE({  // NOLINT
    MockA a;
    EXPECT_CALL(a, DoA(1));
  }, "to be called once");
  EXPECT_NONFATAL_FAILURE({  // NOLINT
    MockA a;
    EXPECT_CALL(a, DoA(1));
    a.DoA(1);
    a.DoA(1);
  }, "to be called once");
}

// TODO(wan@google.com): find a way to re-enable these tests.
#if 0

// Tests that Google Mock doesn't print a warning when the number of
// WillOnce() is adequate.
TEST(ExpectCallSyntaxTest, DoesNotWarnOnAdequateActionCount) {
  CaptureTestStdout();
  {
    MockB b;

    // It's always fine to omit WillOnce() entirely.
    EXPECT_CALL(b, DoB())
        .Times(0);
    EXPECT_CALL(b, DoB(1))
        .Times(AtMost(1));
    EXPECT_CALL(b, DoB(2))
        .Times(1)
        .WillRepeatedly(Return(1));

    // It's fine for the number of WillOnce()s to equal the upper bound.
    EXPECT_CALL(b, DoB(3))
        .Times(Between(1, 2))
        .WillOnce(Return(1))
        .WillOnce(Return(2));

    // It's fine for the number of WillOnce()s to be smaller than the
    // upper bound when there is a WillRepeatedly().
    EXPECT_CALL(b, DoB(4))
        .Times(AtMost(3))
        .WillOnce(Return(1))
        .WillRepeatedly(Return(2));

    // Satisfies the above expectations.
    b.DoB(2);
    b.DoB(3);
  }
  const string& output = GetCapturedTestStdout();
  EXPECT_EQ("", output);
}

// Tests that Google Mock warns on having too many actions in an
// expectation compared to its cardinality.
TEST(ExpectCallSyntaxTest, WarnsOnTooManyActions) {
  CaptureTestStdout();
  {
    MockB b;

    // Warns when the number of WillOnce()s is larger than the upper bound.
    EXPECT_CALL(b, DoB())
        .Times(0)
        .WillOnce(Return(1));  // #1
    EXPECT_CALL(b, DoB())
        .Times(AtMost(1))
        .WillOnce(Return(1))
        .WillOnce(Return(2));  // #2
    EXPECT_CALL(b, DoB(1))
        .Times(1)
        .WillOnce(Return(1))
        .WillOnce(Return(2))
        .RetiresOnSaturation();  // #3

    // Warns when the number of WillOnce()s equals the upper bound and
    // there is a WillRepeatedly().
    EXPECT_CALL(b, DoB())
        .Times(0)
        .WillRepeatedly(Return(1));  // #4
    EXPECT_CALL(b, DoB(2))
        .Times(1)
        .WillOnce(Return(1))
        .WillRepeatedly(Return(2));  // #5

    // Satisfies the above expectations.
    b.DoB(1);
    b.DoB(2);
  }
  const string& output = GetCapturedTestStdout();
  EXPECT_PRED_FORMAT2(IsSubstring,
                      "Too many actions specified.\n"
                      "Expected to be never called, but has 1 WillOnce().",
                      output);  // #1
  EXPECT_PRED_FORMAT2(IsSubstring,
                      "Too many actions specified.\n"
                      "Expected to be called at most once, "
                      "but has 2 WillOnce()s.",
                      output);  // #2
  EXPECT_PRED_FORMAT2(IsSubstring,
                      "Too many actions specified.\n"
                      "Expected to be called once, but has 2 WillOnce()s.",
                      output);  // #3
  EXPECT_PRED_FORMAT2(IsSubstring,
                      "Too many actions specified.\n"
                      "Expected to be never called, but has 0 WillOnce()s "
                      "and a WillRepeatedly().",
                      output);  // #4
  EXPECT_PRED_FORMAT2(IsSubstring,
                      "Too many actions specified.\n"
                      "Expected to be called once, but has 1 WillOnce() "
                      "and a WillRepeatedly().",
                      output);  // #5
}

// Tests that Google Mock warns on having too few actions in an
// expectation compared to its cardinality.
TEST(ExpectCallSyntaxTest, WarnsOnTooFewActions) {
  MockB b;

  EXPECT_CALL(b, DoB())
      .Times(Between(2, 3))
      .WillOnce(Return(1));

  CaptureTestStdout();
  b.DoB();
  const string& output = GetCapturedTestStdout();
  EXPECT_PRED_FORMAT2(IsSubstring,
                      "Too few actions specified.\n"
                      "Expected to be called between 2 and 3 times, "
                      "but has only 1 WillOnce().",
                      output);
  b.DoB();
}

#endif  // 0

// Tests the semantics of ON_CALL().

// Tests that the built-in default action is taken when no ON_CALL()
// is specified.
TEST(OnCallTest, TakesBuiltInDefaultActionWhenNoOnCall) {
  MockB b;
  EXPECT_CALL(b, DoB());

  EXPECT_EQ(0, b.DoB());
}

// Tests that the built-in default action is taken when no ON_CALL()
// matches the invocation.
TEST(OnCallTest, TakesBuiltInDefaultActionWhenNoOnCallMatches) {
  MockB b;
  ON_CALL(b, DoB(1))
      .WillByDefault(Return(1));
  EXPECT_CALL(b, DoB(_));

  EXPECT_EQ(0, b.DoB(2));
}

// Tests that the last matching ON_CALL() action is taken.
TEST(OnCallTest, PicksLastMatchingOnCall) {
  MockB b;
  ON_CALL(b, DoB(_))
      .WillByDefault(Return(3));
  ON_CALL(b, DoB(2))
      .WillByDefault(Return(2));
  ON_CALL(b, DoB(1))
      .WillByDefault(Return(1));
  EXPECT_CALL(b, DoB(_));

  EXPECT_EQ(2, b.DoB(2));
}

// Tests the semantics of EXPECT_CALL().

// Tests that any call is allowed when no EXPECT_CALL() is specified.
TEST(ExpectCallTest, AllowsAnyCallWhenNoSpec) {
  MockB b;
  EXPECT_CALL(b, DoB());
  // There is no expectation on DoB(int).

  b.DoB();

  // DoB(int) can be called any number of times.
  b.DoB(1);
  b.DoB(2);
}

// Tests that the last matching EXPECT_CALL() fires.
TEST(ExpectCallTest, PicksLastMatchingExpectCall) {
  MockB b;
  EXPECT_CALL(b, DoB(_))
      .WillRepeatedly(Return(2));
  EXPECT_CALL(b, DoB(1))
      .WillRepeatedly(Return(1));

  EXPECT_EQ(1, b.DoB(1));
}

// Tests lower-bound violation.
TEST(ExpectCallTest, CatchesTooFewCalls) {
  EXPECT_NONFATAL_FAILURE({  // NOLINT
    MockB b;
    EXPECT_CALL(b, DoB(5))
        .Times(AtLeast(2));

    b.DoB(5);
  }, "Actual function call count doesn't match this expectation.\n"
     "         Expected: to be called at least twice\n"
     "           Actual: called once - unsatisfied and active");
}

// Tests that the cardinality can be inferred when no Times(...) is
// specified.
TEST(ExpectCallTest, InfersCardinalityWhenThereIsNoWillRepeatedly) {
  {
    MockB b;
    EXPECT_CALL(b, DoB())
        .WillOnce(Return(1))
        .WillOnce(Return(2));

    EXPECT_EQ(1, b.DoB());
    EXPECT_EQ(2, b.DoB());
  }

  EXPECT_NONFATAL_FAILURE({  // NOLINT
    MockB b;
    EXPECT_CALL(b, DoB())
        .WillOnce(Return(1))
        .WillOnce(Return(2));

    EXPECT_EQ(1, b.DoB());
  }, "to be called twice");

  {  // NOLINT
    MockB b;
    EXPECT_CALL(b, DoB())
        .WillOnce(Return(1))
        .WillOnce(Return(2));

    EXPECT_EQ(1, b.DoB());
    EXPECT_EQ(2, b.DoB());
    EXPECT_NONFATAL_FAILURE(b.DoB(), "to be called twice");
  }
}

TEST(ExpectCallTest, InfersCardinality1WhenThereIsWillRepeatedly) {
  {
    MockB b;
    EXPECT_CALL(b, DoB())
        .WillOnce(Return(1))
        .WillRepeatedly(Return(2));

    EXPECT_EQ(1, b.DoB());
  }

  {  // NOLINT
    MockB b;
    EXPECT_CALL(b, DoB())
        .WillOnce(Return(1))
        .WillRepeatedly(Return(2));

    EXPECT_EQ(1, b.DoB());
    EXPECT_EQ(2, b.DoB());
    EXPECT_EQ(2, b.DoB());
  }

  EXPECT_NONFATAL_FAILURE({  // NOLINT
    MockB b;
    EXPECT_CALL(b, DoB())
        .WillOnce(Return(1))
        .WillRepeatedly(Return(2));
  }, "to be called at least once");
}

// Tests that the n-th action is taken for the n-th matching
// invocation.
TEST(ExpectCallTest, NthMatchTakesNthAction) {
  MockB b;
  EXPECT_CALL(b, DoB())
      .WillOnce(Return(1))
      .WillOnce(Return(2))
      .WillOnce(Return(3));

  EXPECT_EQ(1, b.DoB());
  EXPECT_EQ(2, b.DoB());
  EXPECT_EQ(3, b.DoB());
}

// TODO(wan@google.com): find a way to re-enable these tests.
#if 0

// Tests that the default action is taken when the WillOnce(...) list is
// exhausted and there is no WillRepeatedly().
TEST(ExpectCallTest, TakesDefaultActionWhenWillListIsExhausted) {
  MockB b;
  EXPECT_CALL(b, DoB(_))
      .Times(1);
  EXPECT_CALL(b, DoB())
      .Times(AnyNumber())
      .WillOnce(Return(1))
      .WillOnce(Return(2));

  CaptureTestStdout();
  EXPECT_EQ(0, b.DoB(1));  // Shouldn't generate a warning as the
                           // expectation has no action clause at all.
  EXPECT_EQ(1, b.DoB());
  EXPECT_EQ(2, b.DoB());
  const string& output1 = GetCapturedTestStdout();
  EXPECT_EQ("", output1);

  CaptureTestStdout();
  EXPECT_EQ(0, b.DoB());
  EXPECT_EQ(0, b.DoB());
  const string& output2 = GetCapturedTestStdout();
  EXPECT_PRED2(RE::PartialMatch, output2,
               "Actions ran out\\.\n"
               "Called 3 times, but only 2 WillOnce\\(\\)s are specified - "
               "returning default value\\.");
  EXPECT_PRED2(RE::PartialMatch, output2,
               "Actions ran out\\.\n"
               "Called 4 times, but only 2 WillOnce\\(\\)s are specified - "
               "returning default value\\.");
}

#endif  // 0

// Tests that the WillRepeatedly() action is taken when the WillOnce(...)
// list is exhausted.
TEST(ExpectCallTest, TakesRepeatedActionWhenWillListIsExhausted) {
  MockB b;
  EXPECT_CALL(b, DoB())
      .WillOnce(Return(1))
      .WillRepeatedly(Return(2));

  EXPECT_EQ(1, b.DoB());
  EXPECT_EQ(2, b.DoB());
  EXPECT_EQ(2, b.DoB());
}

// Tests that an uninteresting call performs the default action.
TEST(UninterestingCallTest, DoesDefaultAction) {
  // When there is an ON_CALL() statement, the action specified by it
  // should be taken.
  MockA a;
  ON_CALL(a, Binary(_, _))
      .WillByDefault(Return(true));
  EXPECT_TRUE(a.Binary(1, 2));

  // When there is no ON_CALL(), the default value for the return type
  // should be returned.
  MockB b;
  EXPECT_EQ(0, b.DoB());
}

// Tests that an unexpected call performs the default action.
TEST(UnexpectedCallTest, DoesDefaultAction) {
  // When there is an ON_CALL() statement, the action specified by it
  // should be taken.
  MockA a;
  ON_CALL(a, Binary(_, _))
      .WillByDefault(Return(true));
  EXPECT_CALL(a, Binary(0, 0));
  a.Binary(0, 0);
  bool result = false;
  EXPECT_NONFATAL_FAILURE(result = a.Binary(1, 2),
                          "Unexpected mock function call");
  EXPECT_TRUE(result);

  // When there is no ON_CALL(), the default value for the return type
  // should be returned.
  MockB b;
  EXPECT_CALL(b, DoB(0))
      .Times(0);
  int n = -1;
  EXPECT_NONFATAL_FAILURE(n = b.DoB(1),
                          "Unexpected mock function call");
  EXPECT_EQ(0, n);
}

// Tests that when an unexpected void function generates the right
// failure message.
TEST(UnexpectedCallTest, GeneratesFailureForVoidFunction) {
  // First, tests the message when there is only one EXPECT_CALL().
  MockA a1;
  EXPECT_CALL(a1, DoA(1));
  a1.DoA(1);
  // Ideally we should match the failure message against a regex, but
  // EXPECT_NONFATAL_FAILURE doesn't support that, so we test for
  // multiple sub-strings instead.
  EXPECT_NONFATAL_FAILURE(
      a1.DoA(9),
      "Unexpected mock function call - returning directly.\n"
      "    Function call: DoA(9)\n"
      "Google Mock tried the following 1 expectation, but it didn't match:");
  EXPECT_NONFATAL_FAILURE(
      a1.DoA(9),
      "  Expected arg #0: is equal to 1\n"
      "           Actual: 9\n"
      "         Expected: to be called once\n"
      "           Actual: called once - saturated and active");

  // Next, tests the message when there are more than one EXPECT_CALL().
  MockA a2;
  EXPECT_CALL(a2, DoA(1));
  EXPECT_CALL(a2, DoA(3));
  a2.DoA(1);
  EXPECT_NONFATAL_FAILURE(
      a2.DoA(2),
      "Unexpected mock function call - returning directly.\n"
      "    Function call: DoA(2)\n"
      "Google Mock tried the following 2 expectations, but none matched:");
  EXPECT_NONFATAL_FAILURE(
      a2.DoA(2),
      "tried expectation #0\n"
      "  Expected arg #0: is equal to 1\n"
      "           Actual: 2\n"
      "         Expected: to be called once\n"
      "           Actual: called once - saturated and active");
  EXPECT_NONFATAL_FAILURE(
      a2.DoA(2),
      "tried expectation #1\n"
      "  Expected arg #0: is equal to 3\n"
      "           Actual: 2\n"
      "         Expected: to be called once\n"
      "           Actual: never called - unsatisfied and active");
  a2.DoA(3);
}

// Tests that an unexpected non-void function generates the right
// failure message.
TEST(UnexpectedCallTest, GeneartesFailureForNonVoidFunction) {
  MockB b1;
  EXPECT_CALL(b1, DoB(1));
  b1.DoB(1);
  EXPECT_NONFATAL_FAILURE(
      b1.DoB(2),
      "Unexpected mock function call - returning default value.\n"
      "    Function call: DoB(2)\n"
      "          Returns: 0\n"
      "Google Mock tried the following 1 expectation, but it didn't match:");
  EXPECT_NONFATAL_FAILURE(
      b1.DoB(2),
      "  Expected arg #0: is equal to 1\n"
      "           Actual: 2\n"
      "         Expected: to be called once\n"
      "           Actual: called once - saturated and active");
}

// Tests that Google Mock explains that an retired expectation doesn't
// match the call.
TEST(UnexpectedCallTest, RetiredExpectation) {
  MockB b;
  EXPECT_CALL(b, DoB(1))
      .RetiresOnSaturation();

  b.DoB(1);
  EXPECT_NONFATAL_FAILURE(
      b.DoB(1),
      "         Expected: the expectation is active\n"
      "           Actual: it is retired");
}

// Tests that Google Mock explains that an expectation that doesn't
// match the arguments doesn't match the call.
TEST(UnexpectedCallTest, UnmatchedArguments) {
  MockB b;
  EXPECT_CALL(b, DoB(1));

  EXPECT_NONFATAL_FAILURE(
      b.DoB(2),
      "  Expected arg #0: is equal to 1\n"
      "           Actual: 2\n");
  b.DoB(1);
}

#ifdef GMOCK_HAS_REGEX

// Tests that Google Mock explains that an expectation with
// unsatisfied pre-requisites doesn't match the call.
TEST(UnexpectedCallTest, UnsatisifiedPrerequisites) {
  Sequence s1, s2;
  MockB b;
  EXPECT_CALL(b, DoB(1))
      .InSequence(s1);
  EXPECT_CALL(b, DoB(2))
      .Times(AnyNumber())
      .InSequence(s1);
  EXPECT_CALL(b, DoB(3))
      .InSequence(s2);
  EXPECT_CALL(b, DoB(4))
      .InSequence(s1, s2);

  ::testing::TestPartResultArray failures;
  {
    ::testing::ScopedFakeTestPartResultReporter reporter(&failures);
    b.DoB(4);
    // Now 'failures' contains the Google Test failures generated by
    // the above statement.
  }

  // There should be one non-fatal failure.
  ASSERT_EQ(1, failures.size());
  const ::testing::TestPartResult& r = failures.GetTestPartResult(0);
  EXPECT_EQ(::testing::TPRT_NONFATAL_FAILURE, r.type());

  // Verifies that the failure message contains the two unsatisfied
  // pre-requisites but not the satisfied one.
  const char* const pattern =
#if GMOCK_USES_PCRE
      // PCRE has trouble using (.|\n) to match any character, but
      // supports the (?s) prefix for using . to match any character.
      "(?s)the following immediate pre-requisites are not satisfied:\n"
      ".*: pre-requisite #0\n"
      ".*: pre-requisite #1";
#else
      // POSIX RE doesn't understand the (?s) prefix, but has no trouble
      // with (.|\n).
      "the following immediate pre-requisites are not satisfied:\n"
      "(.|\n)*: pre-requisite #0\n"
      "(.|\n)*: pre-requisite #1";
#endif  // GMOCK_USES_PCRE

  EXPECT_TRUE(
      ::testing::internal::RE::PartialMatch(r.message(), pattern))
              << " where the message is " << r.message();
  b.DoB(1);
  b.DoB(3);
  b.DoB(4);
}

#endif  // GMOCK_HAS_REGEX

#if GTEST_HAS_DEATH_TEST

TEST(UndefinedReturnValueTest, ReturnValueIsMandatory) {
  MockA a;
  // TODO(wan@google.com): We should really verify the output message,
  // but we cannot yet due to that EXPECT_DEATH only captures stderr
  // while Google Mock logs to stdout.
  EXPECT_DEATH(a.ReturnResult(1), "");
}

#endif  // GTEST_HAS_DEATH_TEST

// Tests that an excessive call (one whose arguments match the
// matchers but is called too many times) performs the default action.
TEST(ExcessiveCallTest, DoesDefaultAction) {
  // When there is an ON_CALL() statement, the action specified by it
  // should be taken.
  MockA a;
  ON_CALL(a, Binary(_, _))
      .WillByDefault(Return(true));
  EXPECT_CALL(a, Binary(0, 0));
  a.Binary(0, 0);
  bool result = false;
  EXPECT_NONFATAL_FAILURE(result = a.Binary(0, 0),
                          "Mock function called more times than expected");
  EXPECT_TRUE(result);

  // When there is no ON_CALL(), the default value for the return type
  // should be returned.
  MockB b;
  EXPECT_CALL(b, DoB(0))
      .Times(0);
  int n = -1;
  EXPECT_NONFATAL_FAILURE(n = b.DoB(0),
                          "Mock function called more times than expected");
  EXPECT_EQ(0, n);
}

// Tests that when a void function is called too many times,
// the failure message contains the argument values.
TEST(ExcessiveCallTest, GeneratesFailureForVoidFunction) {
  MockA a;
  EXPECT_CALL(a, DoA(_))
      .Times(0);
  EXPECT_NONFATAL_FAILURE(
      a.DoA(9),
      "Mock function called more times than expected - returning directly.\n"
      "    Function call: DoA(9)\n"
      "         Expected: to be never called\n"
      "           Actual: called once - over-saturated and active");
}

// Tests that when a non-void function is called too many times, the
// failure message contains the argument values and the return value.
TEST(ExcessiveCallTest, GeneratesFailureForNonVoidFunction) {
  MockB b;
  EXPECT_CALL(b, DoB(_));
  b.DoB(1);
  EXPECT_NONFATAL_FAILURE(
      b.DoB(2),
      "Mock function called more times than expected - "
      "returning default value.\n"
      "    Function call: DoB(2)\n"
      "          Returns: 0\n"
      "         Expected: to be called once\n"
      "           Actual: called twice - over-saturated and active");
}

// Tests using sequences.

TEST(InSequenceTest, AllExpectationInScopeAreInSequence) {
  MockA a;
  {
    InSequence dummy;

    EXPECT_CALL(a, DoA(1));
    EXPECT_CALL(a, DoA(2));
  }

  EXPECT_NONFATAL_FAILURE({  // NOLINT
    a.DoA(2);
  }, "Unexpected mock function call");

  a.DoA(1);
  a.DoA(2);
}

TEST(InSequenceTest, NestedInSequence) {
  MockA a;
  {
    InSequence dummy;

    EXPECT_CALL(a, DoA(1));
    {
      InSequence dummy2;

      EXPECT_CALL(a, DoA(2));
      EXPECT_CALL(a, DoA(3));
    }
  }

  EXPECT_NONFATAL_FAILURE({  // NOLINT
    a.DoA(1);
    a.DoA(3);
  }, "Unexpected mock function call");

  a.DoA(2);
  a.DoA(3);
}

TEST(InSequenceTest, ExpectationsOutOfScopeAreNotAffected) {
  MockA a;
  {
    InSequence dummy;

    EXPECT_CALL(a, DoA(1));
    EXPECT_CALL(a, DoA(2));
  }
  EXPECT_CALL(a, DoA(3));

  EXPECT_NONFATAL_FAILURE({  // NOLINT
    a.DoA(2);
  }, "Unexpected mock function call");

  a.DoA(3);
  a.DoA(1);
  a.DoA(2);
}

// Tests that any order is allowed when no sequence is used.
TEST(SequenceTest, AnyOrderIsOkByDefault) {
  {
    MockA a;
    MockB b;

    EXPECT_CALL(a, DoA(1));
    EXPECT_CALL(b, DoB())
        .Times(AnyNumber());

    a.DoA(1);
    b.DoB();
  }

  {  // NOLINT
    MockA a;
    MockB b;

    EXPECT_CALL(a, DoA(1));
    EXPECT_CALL(b, DoB())
        .Times(AnyNumber());

    b.DoB();
    a.DoA(1);
  }
}

#if GTEST_HAS_DEATH_TEST

// Tests that the calls must be in strict order when a complete order
// is specified.
TEST(SequenceTest, CallsMustBeInStrictOrderWhenSaidSo) {
  MockA a;
  Sequence s;

  EXPECT_CALL(a, ReturnResult(1))
      .InSequence(s)
      .WillOnce(Return(Result()));

  EXPECT_CALL(a, ReturnResult(2))
      .InSequence(s)
      .WillOnce(Return(Result()));

  EXPECT_CALL(a, ReturnResult(3))
      .InSequence(s)
      .WillOnce(Return(Result()));

  EXPECT_DEATH({  // NOLINT
    a.ReturnResult(1);
    a.ReturnResult(3);
    a.ReturnResult(2);
  }, "");

  EXPECT_DEATH({  // NOLINT
    a.ReturnResult(2);
    a.ReturnResult(1);
    a.ReturnResult(3);
  }, "");

  a.ReturnResult(1);
  a.ReturnResult(2);
  a.ReturnResult(3);
}

// Tests specifying a DAG using multiple sequences.
TEST(SequenceTest, CallsMustConformToSpecifiedDag) {
  MockA a;
  MockB b;
  Sequence x, y;

  EXPECT_CALL(a, ReturnResult(1))
      .InSequence(x)
      .WillOnce(Return(Result()));

  EXPECT_CALL(b, DoB())
      .Times(2)
      .InSequence(y);

  EXPECT_CALL(a, ReturnResult(2))
      .InSequence(x, y)
      .WillRepeatedly(Return(Result()));

  EXPECT_CALL(a, ReturnResult(3))
      .InSequence(x)
      .WillOnce(Return(Result()));

  EXPECT_DEATH({  // NOLINT
    a.ReturnResult(1);
    b.DoB();
    a.ReturnResult(2);
  }, "");

  EXPECT_DEATH({  // NOLINT
    a.ReturnResult(2);
  }, "");

  EXPECT_DEATH({  // NOLINT
    a.ReturnResult(3);
  }, "");

  EXPECT_DEATH({  // NOLINT
    a.ReturnResult(1);
    b.DoB();
    b.DoB();
    a.ReturnResult(3);
    a.ReturnResult(2);
  }, "");

  b.DoB();
  a.ReturnResult(1);
  b.DoB();
  a.ReturnResult(3);
}

#endif  // GTEST_HAS_DEATH_TEST

TEST(SequenceTest, Retirement) {
  MockA a;
  Sequence s;

  EXPECT_CALL(a, DoA(1))
      .InSequence(s);
  EXPECT_CALL(a, DoA(_))
      .InSequence(s)
      .RetiresOnSaturation();
  EXPECT_CALL(a, DoA(1))
      .InSequence(s);

  a.DoA(1);
  a.DoA(2);
  a.DoA(1);
}

// Tests that Google Mock correctly handles calls to mock functions
// after a mock object owning one of their pre-requisites has died.

// Tests that calls that satisfy the original spec are successful.
TEST(DeletingMockEarlyTest, Success1) {
  MockB* const b1 = new MockB;
  MockA* const a = new MockA;
  MockB* const b2 = new MockB;

  {
    InSequence dummy;
    EXPECT_CALL(*b1, DoB(_))
        .WillOnce(Return(1));
    EXPECT_CALL(*a, Binary(_, _))
        .Times(AnyNumber())
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*b2, DoB(_))
        .Times(AnyNumber())
        .WillRepeatedly(Return(2));
  }

  EXPECT_EQ(1, b1->DoB(1));
  delete b1;
  // a's pre-requisite has died.
  EXPECT_TRUE(a->Binary(0, 1));
  delete b2;
  // a's successor has died.
  EXPECT_TRUE(a->Binary(1, 2));
  delete a;
}

// Tests that calls that satisfy the original spec are successful.
TEST(DeletingMockEarlyTest, Success2) {
  MockB* const b1 = new MockB;
  MockA* const a = new MockA;
  MockB* const b2 = new MockB;

  {
    InSequence dummy;
    EXPECT_CALL(*b1, DoB(_))
        .WillOnce(Return(1));
    EXPECT_CALL(*a, Binary(_, _))
        .Times(AnyNumber());
    EXPECT_CALL(*b2, DoB(_))
        .Times(AnyNumber())
        .WillRepeatedly(Return(2));
  }

  delete a;  // a is trivially satisfied.
  EXPECT_EQ(1, b1->DoB(1));
  EXPECT_EQ(2, b2->DoB(2));
  delete b1;
  delete b2;
}

// Tests that it's OK to delete a mock object itself in its action.

ACTION_P(Delete, ptr) { delete ptr; }

TEST(DeletingMockEarlyTest, CanDeleteSelfInActionReturningVoid) {
  MockA* const a = new MockA;
  EXPECT_CALL(*a, DoA(_)).WillOnce(Delete(a));
  a->DoA(42);  // This will cause a to be deleted.
}

TEST(DeletingMockEarlyTest, CanDeleteSelfInActionReturningValue) {
  MockA* const a = new MockA;
  EXPECT_CALL(*a, ReturnResult(_))
      .WillOnce(DoAll(Delete(a), Return(Result())));
  a->ReturnResult(42);  // This will cause a to be deleted.
}

// Tests that calls that violate the original spec yield failures.
TEST(DeletingMockEarlyTest, Failure1) {
  MockB* const b1 = new MockB;
  MockA* const a = new MockA;
  MockB* const b2 = new MockB;

  {
    InSequence dummy;
    EXPECT_CALL(*b1, DoB(_))
        .WillOnce(Return(1));
    EXPECT_CALL(*a, Binary(_, _))
        .Times(AnyNumber());
    EXPECT_CALL(*b2, DoB(_))
        .Times(AnyNumber())
        .WillRepeatedly(Return(2));
  }

  delete a;  // a is trivially satisfied.
  EXPECT_NONFATAL_FAILURE({
    b2->DoB(2);
  }, "Unexpected mock function call");
  EXPECT_EQ(1, b1->DoB(1));
  delete b1;
  delete b2;
}

// Tests that calls that violate the original spec yield failures.
TEST(DeletingMockEarlyTest, Failure2) {
  MockB* const b1 = new MockB;
  MockA* const a = new MockA;
  MockB* const b2 = new MockB;

  {
    InSequence dummy;
    EXPECT_CALL(*b1, DoB(_));
    EXPECT_CALL(*a, Binary(_, _))
        .Times(AnyNumber());
    EXPECT_CALL(*b2, DoB(_))
        .Times(AnyNumber());
  }

  EXPECT_NONFATAL_FAILURE(delete b1,
                          "Actual: never called");
  EXPECT_NONFATAL_FAILURE(a->Binary(0, 1),
                          "Unexpected mock function call");
  EXPECT_NONFATAL_FAILURE(b2->DoB(1),
                          "Unexpected mock function call");
  delete a;
  delete b2;
}

class EvenNumberCardinality : public CardinalityInterface {
 public:
  // Returns true iff call_count calls will satisfy this cardinality.
  virtual bool IsSatisfiedByCallCount(int call_count) const {
    return call_count % 2 == 0;
  }

  // Returns true iff call_count calls will saturate this cardinality.
  virtual bool IsSaturatedByCallCount(int call_count) const { return false; }

  // Describes self to an ostream.
  virtual void DescribeTo(::std::ostream* os) const {
    *os << "called even number of times";
  }
};

Cardinality EvenNumber() {
  return Cardinality(new EvenNumberCardinality);
}

TEST(ExpectationBaseTest,
     AllPrerequisitesAreSatisfiedWorksForNonMonotonicCardinality) {
  MockA* a = new MockA;
  Sequence s;

  EXPECT_CALL(*a, DoA(1))
      .Times(EvenNumber())
      .InSequence(s);
  EXPECT_CALL(*a, DoA(2))
      .Times(AnyNumber())
      .InSequence(s);
  EXPECT_CALL(*a, DoA(3))
      .Times(AnyNumber());

  a->DoA(3);
  a->DoA(1);
  EXPECT_NONFATAL_FAILURE(a->DoA(2), "Unexpected mock function call");
  EXPECT_NONFATAL_FAILURE(delete a, "to be called even number of times");
}

// The following tests verify the message generated when a mock
// function is called.

struct Printable {
};

inline void operator<<(::std::ostream& os, const Printable&) {
  os << "Printable";
}

struct Unprintable {
  Unprintable() : value(0) {}
  int value;
};

class MockC {
 public:
  MOCK_METHOD6(VoidMethod, void(bool cond, int n, string s, void* p,
                                const Printable& x, Unprintable y));
  MOCK_METHOD0(NonVoidMethod, int());  // NOLINT
};

// TODO(wan@google.com): find a way to re-enable these tests.
#if 0

// Tests that an uninteresting mock function call generates a warning
// containing the stack trace.
TEST(FunctionCallMessageTest, UninterestingCallGeneratesFyiWithStackTrace) {
  MockC c;
  CaptureTestStdout();
  c.VoidMethod(false, 5, "Hi", NULL, Printable(), Unprintable());
  const string& output = GetCapturedTestStdout();
  EXPECT_PRED_FORMAT2(IsSubstring, "GMOCK WARNING", output);
  EXPECT_PRED_FORMAT2(IsSubstring, "Stack trace:", output);
#ifndef NDEBUG
  // We check the stack trace content in dbg-mode only, as opt-mode
  // may inline the call we are interested in seeing.

  // Verifies that a void mock function's name appears in the stack
  // trace.
  EXPECT_PRED_FORMAT2(IsSubstring, "::MockC::VoidMethod(", output);

  // Verifies that a non-void mock function's name appears in the
  // stack trace.
  CaptureTestStdout();
  c.NonVoidMethod();
  const string& output2 = GetCapturedTestStdout();
  EXPECT_PRED_FORMAT2(IsSubstring, "::MockC::NonVoidMethod(", output2);
#endif  // NDEBUG
}

// Tests that an uninteresting mock function call causes the function
// arguments and return value to be printed.
TEST(FunctionCallMessageTest, UninterestingCallPrintsArgumentsAndReturnValue) {
  // A non-void mock function.
  MockB b;
  CaptureTestStdout();
  b.DoB();
  const string& output1 = GetCapturedTestStdout();
  EXPECT_PRED_FORMAT2(
      IsSubstring,
      "Uninteresting mock function call - returning default value.\n"
      "    Function call: DoB()\n"
      "          Returns: 0\n", output1);
  // Makes sure the return value is printed.

  // A void mock function.
  MockC c;
  CaptureTestStdout();
  c.VoidMethod(false, 5, "Hi", NULL, Printable(), Unprintable());
  const string& output2 = GetCapturedTestStdout();
  EXPECT_PRED2(RE::PartialMatch, output2,
               "Uninteresting mock function call - returning directly\\.\n"
               "    Function call: VoidMethod"
               "\\(false, 5, \"Hi\", NULL, @0x\\w+ "
               "Printable, 4-byte object <0000 0000>\\)");
  // A void function has no return value to print.
}

// Tests how the --gmock_verbose flag affects Google Mock's output.

class GMockVerboseFlagTest : public testing::Test {
 public:
  // Verifies that the given Google Mock output is correct.  (When
  // should_print is true, the output should match the given regex and
  // contain the given function name in the stack trace.  When it's
  // false, the output should be empty.)
  void VerifyOutput(const string& output, bool should_print,
                    const string& regex,
                    const string& function_name) {
    if (should_print) {
      EXPECT_PRED2(RE::PartialMatch, output, regex);
#ifndef NDEBUG
      // We check the stack trace content in dbg-mode only, as opt-mode
      // may inline the call we are interested in seeing.
      EXPECT_PRED_FORMAT2(IsSubstring, function_name, output);
#endif  // NDEBUG
    } else {
      EXPECT_EQ("", output);
    }
  }

  // Tests how the flag affects expected calls.
  void TestExpectedCall(bool should_print) {
    MockA a;
    EXPECT_CALL(a, DoA(5));
    EXPECT_CALL(a, Binary(_, 1))
        .WillOnce(Return(true));

    // A void-returning function.
    CaptureTestStdout();
    a.DoA(5);
    VerifyOutput(
        GetCapturedTestStdout(),
        should_print,
        "Expected mock function call\\.\n"
        "    Function call: DoA\\(5\\)\n"
        "Stack trace:",
        "MockA::DoA");

    // A non-void-returning function.
    CaptureTestStdout();
    a.Binary(2, 1);
    VerifyOutput(
        GetCapturedTestStdout(),
        should_print,
        "Expected mock function call\\.\n"
        "    Function call: Binary\\(2, 1\\)\n"
        "          Returns: true\n"
        "Stack trace:",
        "MockA::Binary");
  }

  // Tests how the flag affects uninteresting calls.
  void TestUninterestingCall(bool should_print) {
    MockA a;

    // A void-returning function.
    CaptureTestStdout();
    a.DoA(5);
    VerifyOutput(
        GetCapturedTestStdout(),
        should_print,
        "\nGMOCK WARNING:\n"
        "Uninteresting mock function call - returning directly\\.\n"
        "    Function call: DoA\\(5\\)\n"
        "Stack trace:\n"
        "[\\s\\S]*",
        "MockA::DoA");

    // A non-void-returning function.
    CaptureTestStdout();
    a.Binary(2, 1);
    VerifyOutput(
        GetCapturedTestStdout(),
        should_print,
        "\nGMOCK WARNING:\n"
        "Uninteresting mock function call - returning default value\\.\n"
        "    Function call: Binary\\(2, 1\\)\n"
        "          Returns: false\n"
        "Stack trace:\n"
        "[\\s\\S]*",
        "MockA::Binary");
  }
};

// Tests that --gmock_verbose=info causes both expected and
// uninteresting calls to be reported.
TEST_F(GMockVerboseFlagTest, Info) {
  GMOCK_FLAG(verbose) = kInfoVerbosity;
  TestExpectedCall(true);
  TestUninterestingCall(true);
}

// Tests that --gmock_verbose=warning causes uninteresting calls to be
// reported.
TEST_F(GMockVerboseFlagTest, Warning) {
  GMOCK_FLAG(verbose) = kWarningVerbosity;
  TestExpectedCall(false);
  TestUninterestingCall(true);
}

// Tests that --gmock_verbose=warning causes neither expected nor
// uninteresting calls to be reported.
TEST_F(GMockVerboseFlagTest, Error) {
  GMOCK_FLAG(verbose) = kErrorVerbosity;
  TestExpectedCall(false);
  TestUninterestingCall(false);
}

// Tests that --gmock_verbose=SOME_INVALID_VALUE has the same effect
// as --gmock_verbose=warning.
TEST_F(GMockVerboseFlagTest, InvalidFlagIsTreatedAsWarning) {
  GMOCK_FLAG(verbose) = "invalid";  // Treated as "warning".
  TestExpectedCall(false);
  TestUninterestingCall(true);
}

#endif  // 0

// A helper class that generates a failure when printed.  We use it to
// ensure that Google Mock doesn't print a value (even to an internal
// buffer) when it is not supposed to do so.
class PrintMeNot {};

void PrintTo(PrintMeNot /* dummy */, ::std::ostream* /* os */) {
  ADD_FAILURE() << "Google Mock is printing a value that shouldn't be "
                << "printed even to an internal buffer.";
}

class LogTestHelper {
 public:
  MOCK_METHOD1(Foo, PrintMeNot(PrintMeNot));
};

class GMockLogTest : public ::testing::Test {
 protected:
  virtual void SetUp() { original_verbose_ = GMOCK_FLAG(verbose); }
  virtual void TearDown() { GMOCK_FLAG(verbose) = original_verbose_; }

  LogTestHelper helper_;
  string original_verbose_;
};

TEST_F(GMockLogTest, DoesNotPrintGoodCallInternallyIfVerbosityIsWarning) {
  GMOCK_FLAG(verbose) = kWarningVerbosity;
  EXPECT_CALL(helper_, Foo(_))
      .WillOnce(Return(PrintMeNot()));
  helper_.Foo(PrintMeNot());  // This is an expected call.
}

TEST_F(GMockLogTest, DoesNotPrintGoodCallInternallyIfVerbosityIsError) {
  GMOCK_FLAG(verbose) = kErrorVerbosity;
  EXPECT_CALL(helper_, Foo(_))
      .WillOnce(Return(PrintMeNot()));
  helper_.Foo(PrintMeNot());  // This is an expected call.
}

TEST_F(GMockLogTest, DoesNotPrintWarningInternallyIfVerbosityIsError) {
  GMOCK_FLAG(verbose) = kErrorVerbosity;
  ON_CALL(helper_, Foo(_))
      .WillByDefault(Return(PrintMeNot()));
  helper_.Foo(PrintMeNot());  // This should generate a warning.
}

// Tests Mock::AllowLeak().

TEST(AllowLeakTest, AllowsLeakingUnusedMockObject) {
  MockA* a = new MockA;
  Mock::AllowLeak(a);
}

TEST(AllowLeakTest, CanBeCalledBeforeOnCall) {
  MockA* a = new MockA;
  Mock::AllowLeak(a);
  ON_CALL(*a, DoA(_)).WillByDefault(Return());
  a->DoA(0);
}

TEST(AllowLeakTest, CanBeCalledAfterOnCall) {
  MockA* a = new MockA;
  ON_CALL(*a, DoA(_)).WillByDefault(Return());
  Mock::AllowLeak(a);
}

TEST(AllowLeakTest, CanBeCalledBeforeExpectCall) {
  MockA* a = new MockA;
  Mock::AllowLeak(a);
  EXPECT_CALL(*a, DoA(_));
  a->DoA(0);
}

TEST(AllowLeakTest, CanBeCalledAfterExpectCall) {
  MockA* a = new MockA;
  EXPECT_CALL(*a, DoA(_)).Times(AnyNumber());
  Mock::AllowLeak(a);
}

TEST(AllowLeakTest, WorksWhenBothOnCallAndExpectCallArePresent) {
  MockA* a = new MockA;
  ON_CALL(*a, DoA(_)).WillByDefault(Return());
  EXPECT_CALL(*a, DoA(_)).Times(AnyNumber());
  Mock::AllowLeak(a);
}

// Tests that we can verify and clear a mock object's expectations
// when none of its methods has expectations.
TEST(VerifyAndClearExpectationsTest, NoMethodHasExpectations) {
  MockB b;
  ASSERT_TRUE(Mock::VerifyAndClearExpectations(&b));

  // There should be no expectations on the methods now, so we can
  // freely call them.
  EXPECT_EQ(0, b.DoB());
  EXPECT_EQ(0, b.DoB(1));
}

// Tests that we can verify and clear a mock object's expectations
// when some, but not all, of its methods have expectations *and* the
// verification succeeds.
TEST(VerifyAndClearExpectationsTest, SomeMethodsHaveExpectationsAndSucceed) {
  MockB b;
  EXPECT_CALL(b, DoB())
      .WillOnce(Return(1));
  b.DoB();
  ASSERT_TRUE(Mock::VerifyAndClearExpectations(&b));

  // There should be no expectations on the methods now, so we can
  // freely call them.
  EXPECT_EQ(0, b.DoB());
  EXPECT_EQ(0, b.DoB(1));
}

// Tests that we can verify and clear a mock object's expectations
// when some, but not all, of its methods have expectations *and* the
// verification fails.
TEST(VerifyAndClearExpectationsTest, SomeMethodsHaveExpectationsAndFail) {
  MockB b;
  EXPECT_CALL(b, DoB())
      .WillOnce(Return(1));
  bool result;
  EXPECT_NONFATAL_FAILURE(result = Mock::VerifyAndClearExpectations(&b),
                          "Actual: never called");
  ASSERT_FALSE(result);

  // There should be no expectations on the methods now, so we can
  // freely call them.
  EXPECT_EQ(0, b.DoB());
  EXPECT_EQ(0, b.DoB(1));
}

// Tests that we can verify and clear a mock object's expectations
// when all of its methods have expectations.
TEST(VerifyAndClearExpectationsTest, AllMethodsHaveExpectations) {
  MockB b;
  EXPECT_CALL(b, DoB())
      .WillOnce(Return(1));
  EXPECT_CALL(b, DoB(_))
      .WillOnce(Return(2));
  b.DoB();
  b.DoB(1);
  ASSERT_TRUE(Mock::VerifyAndClearExpectations(&b));

  // There should be no expectations on the methods now, so we can
  // freely call them.
  EXPECT_EQ(0, b.DoB());
  EXPECT_EQ(0, b.DoB(1));
}

// Tests that we can verify and clear a mock object's expectations
// when a method has more than one expectation.
TEST(VerifyAndClearExpectationsTest, AMethodHasManyExpectations) {
  MockB b;
  EXPECT_CALL(b, DoB(0))
      .WillOnce(Return(1));
  EXPECT_CALL(b, DoB(_))
      .WillOnce(Return(2));
  b.DoB(1);
  bool result;
  EXPECT_NONFATAL_FAILURE(result = Mock::VerifyAndClearExpectations(&b),
                          "Actual: never called");
  ASSERT_FALSE(result);

  // There should be no expectations on the methods now, so we can
  // freely call them.
  EXPECT_EQ(0, b.DoB());
  EXPECT_EQ(0, b.DoB(1));
}

// Tests that we can call VerifyAndClearExpectations() on the same
// mock object multiple times.
TEST(VerifyAndClearExpectationsTest, CanCallManyTimes) {
  MockB b;
  EXPECT_CALL(b, DoB());
  b.DoB();
  Mock::VerifyAndClearExpectations(&b);

  EXPECT_CALL(b, DoB(_))
      .WillOnce(Return(1));
  b.DoB(1);
  Mock::VerifyAndClearExpectations(&b);
  Mock::VerifyAndClearExpectations(&b);

  // There should be no expectations on the methods now, so we can
  // freely call them.
  EXPECT_EQ(0, b.DoB());
  EXPECT_EQ(0, b.DoB(1));
}

// Tests that we can clear a mock object's default actions when none
// of its methods has default actions.
TEST(VerifyAndClearTest, NoMethodHasDefaultActions) {
  MockB b;
  // If this crashes or generates a failure, the test will catch it.
  Mock::VerifyAndClear(&b);
  EXPECT_EQ(0, b.DoB());
}

// Tests that we can clear a mock object's default actions when some,
// but not all of its methods have default actions.
TEST(VerifyAndClearTest, SomeMethodsHaveDefaultActions) {
  MockB b;
  ON_CALL(b, DoB())
      .WillByDefault(Return(1));

  Mock::VerifyAndClear(&b);

  // Verifies that the default action of int DoB() was removed.
  EXPECT_EQ(0, b.DoB());
}

// Tests that we can clear a mock object's default actions when all of
// its methods have default actions.
TEST(VerifyAndClearTest, AllMethodsHaveDefaultActions) {
  MockB b;
  ON_CALL(b, DoB())
      .WillByDefault(Return(1));
  ON_CALL(b, DoB(_))
      .WillByDefault(Return(2));

  Mock::VerifyAndClear(&b);

  // Verifies that the default action of int DoB() was removed.
  EXPECT_EQ(0, b.DoB());

  // Verifies that the default action of int DoB(int) was removed.
  EXPECT_EQ(0, b.DoB(0));
}

// Tests that we can clear a mock object's default actions when a
// method has more than one ON_CALL() set on it.
TEST(VerifyAndClearTest, AMethodHasManyDefaultActions) {
  MockB b;
  ON_CALL(b, DoB(0))
      .WillByDefault(Return(1));
  ON_CALL(b, DoB(_))
      .WillByDefault(Return(2));

  Mock::VerifyAndClear(&b);

  // Verifies that the default actions (there are two) of int DoB(int)
  // were removed.
  EXPECT_EQ(0, b.DoB(0));
  EXPECT_EQ(0, b.DoB(1));
}

// Tests that we can call VerifyAndClear() on a mock object multiple
// times.
TEST(VerifyAndClearTest, CanCallManyTimes) {
  MockB b;
  ON_CALL(b, DoB())
      .WillByDefault(Return(1));
  Mock::VerifyAndClear(&b);
  Mock::VerifyAndClear(&b);

  ON_CALL(b, DoB(_))
      .WillByDefault(Return(1));
  Mock::VerifyAndClear(&b);

  EXPECT_EQ(0, b.DoB());
  EXPECT_EQ(0, b.DoB(1));
}

// Tests that VerifyAndClear() works when the verification succeeds.
TEST(VerifyAndClearTest, Success) {
  MockB b;
  ON_CALL(b, DoB())
      .WillByDefault(Return(1));
  EXPECT_CALL(b, DoB(1))
      .WillOnce(Return(2));

  b.DoB();
  b.DoB(1);
  ASSERT_TRUE(Mock::VerifyAndClear(&b));

  // There should be no expectations on the methods now, so we can
  // freely call them.
  EXPECT_EQ(0, b.DoB());
  EXPECT_EQ(0, b.DoB(1));
}

// Tests that VerifyAndClear() works when the verification fails.
TEST(VerifyAndClearTest, Failure) {
  MockB b;
  ON_CALL(b, DoB(_))
      .WillByDefault(Return(1));
  EXPECT_CALL(b, DoB())
      .WillOnce(Return(2));

  b.DoB(1);
  bool result;
  EXPECT_NONFATAL_FAILURE(result = Mock::VerifyAndClear(&b),
                          "Actual: never called");
  ASSERT_FALSE(result);

  // There should be no expectations on the methods now, so we can
  // freely call them.
  EXPECT_EQ(0, b.DoB());
  EXPECT_EQ(0, b.DoB(1));
}

// Tests that VerifyAndClear() works when the default actions and
// expectations are set on a const mock object.
TEST(VerifyAndClearTest, Const) {
  MockB b;
  ON_CALL(Const(b), DoB())
      .WillByDefault(Return(1));

  EXPECT_CALL(Const(b), DoB())
      .WillOnce(DoDefault())
      .WillOnce(Return(2));

  b.DoB();
  b.DoB();
  ASSERT_TRUE(Mock::VerifyAndClear(&b));

  // There should be no expectations on the methods now, so we can
  // freely call them.
  EXPECT_EQ(0, b.DoB());
  EXPECT_EQ(0, b.DoB(1));
}

// Tests that we can set default actions and expectations on a mock
// object after VerifyAndClear() has been called on it.
TEST(VerifyAndClearTest, CanSetDefaultActionsAndExpectationsAfterwards) {
  MockB b;
  ON_CALL(b, DoB())
      .WillByDefault(Return(1));
  EXPECT_CALL(b, DoB(_))
      .WillOnce(Return(2));
  b.DoB(1);

  Mock::VerifyAndClear(&b);

  EXPECT_CALL(b, DoB())
      .WillOnce(Return(3));
  ON_CALL(b, DoB(_))
      .WillByDefault(Return(4));

  EXPECT_EQ(3, b.DoB());
  EXPECT_EQ(4, b.DoB(1));
}

// Tests that calling VerifyAndClear() on one mock object does not
// affect other mock objects (either of the same type or not).
TEST(VerifyAndClearTest, DoesNotAffectOtherMockObjects) {
  MockA a;
  MockB b1;
  MockB b2;

  ON_CALL(a, Binary(_, _))
      .WillByDefault(Return(true));
  EXPECT_CALL(a, Binary(_, _))
      .WillOnce(DoDefault())
      .WillOnce(Return(false));

  ON_CALL(b1, DoB())
      .WillByDefault(Return(1));
  EXPECT_CALL(b1, DoB(_))
      .WillOnce(Return(2));

  ON_CALL(b2, DoB())
      .WillByDefault(Return(3));
  EXPECT_CALL(b2, DoB(_));

  b2.DoB(0);
  Mock::VerifyAndClear(&b2);

  // Verifies that the default actions and expectations of a and b1
  // are still in effect.
  EXPECT_TRUE(a.Binary(0, 0));
  EXPECT_FALSE(a.Binary(0, 0));

  EXPECT_EQ(1, b1.DoB());
  EXPECT_EQ(2, b1.DoB(0));
}

// Tests that a mock function's action can call a mock function
// (either the same function or a different one) either as an explicit
// action or as a default action without causing a dead lock.  It
// verifies that the action is not performed inside the critical
// section.

void Helper(MockC* c) {
  c->NonVoidMethod();
}

}  // namespace

int main(int argc, char **argv) {
  testing::InitGoogleMock(&argc, argv);

  // Ensures that the tests pass no matter what value of
  // --gmock_catch_leaked_mocks and --gmock_verbose the user specifies.
  testing::GMOCK_FLAG(catch_leaked_mocks) = true;
  testing::GMOCK_FLAG(verbose) = testing::internal::kWarningVerbosity;

  return RUN_ALL_TESTS();
}
