#include <string>

#include "cotest/cotest.h"
#include "cotest/internal/cotest-integ-finder.h"

using namespace std;
using namespace testing;

/*
 * Note: at the time of writing, this test does not use wildcarded watches
 * and so will not engage the untyped expectation finding mechanism. So
 * internal::CotestMockHandlerPool::Finder() won't be being used by the test
 * harness while we're testing it.
 */

//////////////////////////////////////////////
// The actual tests

TEST(ExpectationFinderTest, NoSchemes) {
    MockFunction<std::function<bool(internal::ExpectationBase *)>> mockLambda;
    std::vector<const internal::MockHandlerScheme *> schemes;

    auto coro = COROUTINE(){
        // Empty coro oversaturates on any visible call
    };
    coro.WATCH_CALL(mockLambda);

    // No schemes, so no exps. "which" is undefined in this case.
    unsigned which;
    auto exp = internal::CotestMockHandlerPool::Finder(schemes, mockLambda.AsStdFunction(), &which);
    ASSERT_FALSE(exp);
}

TEST(ExpectationFinderTest, EmptyScheme) {
    MockFunction<std::function<bool(internal::ExpectationBase *)>> mockLambda;
    internal::MockHandlerScheme s1;
    std::vector<const internal::MockHandlerScheme *> schemes{&s1};

    auto coro = COROUTINE(){
        // Empty coro oversaturates on any visible call
    };
    coro.WATCH_CALL(mockLambda);

    // No exps. "which" is undefined in this case.
    unsigned which;
    auto exp = internal::CotestMockHandlerPool::Finder(schemes, mockLambda.AsStdFunction(), &which);
    ASSERT_FALSE(exp);
}

TEST(ExpectationFinderTest, SimpleSchemeNo) {
    MockFunction<std::function<bool(internal::ExpectationBase *)>> mockLambda;

    auto e1 = make_shared<internal::TypedExpectation<int()>>(nullptr, nullptr, 0, "",
                                                             internal::Function<int()>::ArgumentMatcherTuple());
    internal::MockHandlerScheme s1{e1};
    std::vector<const internal::MockHandlerScheme *> schemes{&s1};

    auto coro = COROUTINE() {
        // One exp was created, and it should be queried
        WAIT_FOR_CALL(mockLambda, Call(e1.get())).RETURN(false);
    };
    coro.WATCH_CALL(mockLambda);

    // The exp says no. "which" is undefined in this case.
    unsigned which;
    auto exp = internal::CotestMockHandlerPool::Finder(schemes, mockLambda.AsStdFunction(), &which);
    ASSERT_FALSE(exp);
}

TEST(ExpectationFinderTest, SimpleSchemeYes) {
    MockFunction<std::function<bool(internal::ExpectationBase *)>> mockLambda;

    auto e1 = make_shared<internal::TypedExpectation<int()>>(nullptr, nullptr, 0, "",
                                                             internal::Function<int()>::ArgumentMatcherTuple());
    internal::MockHandlerScheme s1{e1};
    std::vector<const internal::MockHandlerScheme *> schemes{&s1};

    auto coro = COROUTINE() {
        // One exp was created, and it should be queried
        WAIT_FOR_CALL(mockLambda, Call(e1.get())).RETURN(true);
    };
    coro.WATCH_CALL(mockLambda);

    // The exp says yes.
    unsigned which;
    auto exp = internal::CotestMockHandlerPool::Finder(schemes, mockLambda.AsStdFunction(), &which);
    ASSERT_TRUE(exp);
    ASSERT_EQ(which, 0);
}

TEST(ExpectationFinderTest, OneSchemeMultiExp) {
    MockFunction<std::function<bool(internal::ExpectationBase *)>> mockLambda;

    auto e1 = make_shared<internal::TypedExpectation<int()>>(nullptr, nullptr, 0, "",
                                                             internal::Function<int()>::ArgumentMatcherTuple());
    auto e2 = make_shared<internal::TypedExpectation<int()>>(nullptr, nullptr, 0, "",
                                                             internal::Function<int()>::ArgumentMatcherTuple());
    auto e3 = make_shared<internal::TypedExpectation<int()>>(nullptr, nullptr, 0, "",
                                                             internal::Function<int()>::ArgumentMatcherTuple());
    auto e4 = make_shared<internal::TypedExpectation<int()>>(nullptr, nullptr, 0, "",
                                                             internal::Function<int()>::ArgumentMatcherTuple());
    internal::MockHandlerScheme s1{e1, e2, e3, e4};
    std::vector<const internal::MockHandlerScheme *> schemes{&s1};

    auto coro = COROUTINE() {
        // Exps queried in reverse order until one says yes
        WAIT_FOR_CALL(mockLambda, Call(e4.get())).RETURN(false);
        WAIT_FOR_CALL(mockLambda, Call(e3.get())).RETURN(false);
        WAIT_FOR_CALL(mockLambda, Call(e2.get())).RETURN(true);
    };
    coro.WATCH_CALL(mockLambda);

    // The exp says yes.
    unsigned which;
    auto exp = internal::CotestMockHandlerPool::Finder(schemes, mockLambda.AsStdFunction(), &which);
    ASSERT_TRUE(exp);
    ASSERT_EQ(which, 0);
}

TEST(ExpectationFinderTest, OneSchemeMultiExpIncing) {
    MockFunction<std::function<bool(internal::ExpectationBase *)>> mockLambda;

    auto e1 = make_shared<internal::TypedExpectation<int()>>(nullptr, nullptr, 0, "",
                                                             internal::Function<int()>::ArgumentMatcherTuple());
    auto e2 = make_shared<internal::TypedExpectation<int()>>(nullptr, nullptr, 0, "",
                                                             internal::Function<int()>::ArgumentMatcherTuple());
    auto e3 = make_shared<internal::TypedExpectation<int()>>(nullptr, nullptr, 0, "",
                                                             internal::Function<int()>::ArgumentMatcherTuple());
    auto e4 = make_shared<internal::TypedExpectation<int()>>(nullptr, nullptr, 0, "",
                                                             internal::Function<int()>::ArgumentMatcherTuple());
    internal::MockHandlerScheme s1{e1, e2, e3, e4};
    std::vector<const internal::MockHandlerScheme *> schemes{&s1};

    auto coro = COROUTINE() {
        // Exps queried in reverse order until one says yes
        WAIT_FOR_CALL(mockLambda, Call(e4.get())).RETURN(false);
        WAIT_FOR_CALL(mockLambda, Call(e3.get())).RETURN(false);
        WAIT_FOR_CALL(mockLambda, Call(e2.get())).RETURN(true);
    };
    coro.WATCH_CALL(mockLambda);

    // The exp says yes.
    unsigned which;
    auto exp = internal::CotestMockHandlerPool::Finder(schemes, mockLambda.AsStdFunction(), &which);
    ASSERT_TRUE(exp);
    ASSERT_EQ(which, 0);
}

TEST(ExpectationFinderTest, MultiSchemeNone) {
    MockFunction<std::function<bool(internal::ExpectationBase *)>> mockLambda;

    auto e1 = make_shared<internal::TypedExpectation<int()>>(nullptr, nullptr, 0, "",
                                                             internal::Function<int()>::ArgumentMatcherTuple());
    internal::CotestMockHandlerPool::GetOrCreateInstance()->AddExpectation(
        []() {});  // bumps the static global priority
    auto e2 = make_shared<internal::TypedExpectation<int()>>(nullptr, nullptr, 0, "",
                                                             internal::Function<int()>::ArgumentMatcherTuple());
    internal::CotestMockHandlerPool::GetOrCreateInstance()->AddExpectation(
        []() {});  // bumps the static global priority
    auto e3 = make_shared<internal::TypedExpectation<int()>>(nullptr, nullptr, 0, "",
                                                             internal::Function<int()>::ArgumentMatcherTuple());
    internal::CotestMockHandlerPool::GetOrCreateInstance()->AddExpectation(
        []() {});  // bumps the static global priority
    auto e4 = make_shared<internal::TypedExpectation<int()>>(nullptr, nullptr, 0, "",
                                                             internal::Function<int()>::ArgumentMatcherTuple());
    internal::CotestMockHandlerPool::GetOrCreateInstance()->AddExpectation(
        []() {});  // bumps the static global priority
    auto e5 = make_shared<internal::TypedExpectation<int()>>(nullptr, nullptr, 0, "",
                                                             internal::Function<int()>::ArgumentMatcherTuple());
    internal::CotestMockHandlerPool::GetOrCreateInstance()->AddExpectation(
        []() {});  // bumps the static global priority
    internal::MockHandlerScheme s1{e1, e3, e5};
    internal::MockHandlerScheme s2{e2};
    internal::MockHandlerScheme s3{e4};
    std::vector<const internal::MockHandlerScheme *> schemes{&s1, &s2, &s3};

    auto coro = COROUTINE() {
        // Exps queried in reverse order until one says yes
        WAIT_FOR_CALL(mockLambda, Call(e5.get())).RETURN(false);
        WAIT_FOR_CALL(mockLambda, Call(e4.get())).RETURN(false);
        WAIT_FOR_CALL(mockLambda, Call(e3.get())).RETURN(false);
        WAIT_FOR_CALL(mockLambda, Call(e2.get())).RETURN(false);
        WAIT_FOR_CALL(mockLambda, Call(e1.get())).RETURN(false);
    };
    coro.WATCH_CALL(mockLambda);

    // The exps say no.
    unsigned which;
    auto exp = internal::CotestMockHandlerPool::Finder(schemes, mockLambda.AsStdFunction(), &which);
    ASSERT_FALSE(exp);
}

TEST(ExpectationFinderTest, MultiSchemeLate) {
    MockFunction<std::function<bool(internal::ExpectationBase *)>> mockLambda;

    auto e1 = make_shared<internal::TypedExpectation<int()>>(nullptr, nullptr, 0, "",
                                                             internal::Function<int()>::ArgumentMatcherTuple());
    internal::CotestMockHandlerPool::GetOrCreateInstance()->AddExpectation(
        []() {});  // bumps the static global priority
    auto e2 = make_shared<internal::TypedExpectation<int()>>(nullptr, nullptr, 0, "",
                                                             internal::Function<int()>::ArgumentMatcherTuple());
    internal::CotestMockHandlerPool::GetOrCreateInstance()->AddExpectation(
        []() {});  // bumps the static global priority
    auto e3 = make_shared<internal::TypedExpectation<int()>>(nullptr, nullptr, 0, "",
                                                             internal::Function<int()>::ArgumentMatcherTuple());
    internal::CotestMockHandlerPool::GetOrCreateInstance()->AddExpectation(
        []() {});  // bumps the static global priority
    auto e4 = make_shared<internal::TypedExpectation<int()>>(nullptr, nullptr, 0, "",
                                                             internal::Function<int()>::ArgumentMatcherTuple());
    internal::CotestMockHandlerPool::GetOrCreateInstance()->AddExpectation(
        []() {});  // bumps the static global priority
    auto e5 = make_shared<internal::TypedExpectation<int()>>(nullptr, nullptr, 0, "",
                                                             internal::Function<int()>::ArgumentMatcherTuple());
    internal::CotestMockHandlerPool::GetOrCreateInstance()->AddExpectation(
        []() {});  // bumps the static global priority
    internal::MockHandlerScheme s1{e1, e3, e5};
    internal::MockHandlerScheme s2{e2};
    internal::MockHandlerScheme s3{e4};
    std::vector<const internal::MockHandlerScheme *> schemes{&s1, &s2, &s3};

    auto coro = COROUTINE() {
        // Exps queried in reverse order until one says yes
        WAIT_FOR_CALL(mockLambda, Call(e5.get())).RETURN(false);
        WAIT_FOR_CALL(mockLambda, Call(e4.get())).RETURN(false);
        WAIT_FOR_CALL(mockLambda, Call(e3.get())).RETURN(false);
        WAIT_FOR_CALL(mockLambda, Call(e2.get())).RETURN(false);
        WAIT_FOR_CALL(mockLambda, Call(e1.get())).RETURN(true);
    };
    coro.WATCH_CALL(mockLambda);

    // The exp says yes.
    unsigned which;
    auto exp = internal::CotestMockHandlerPool::Finder(schemes, mockLambda.AsStdFunction(), &which);
    ASSERT_EQ(exp, e1.get());
    ASSERT_EQ(which, 0);
}

TEST(ExpectationFinderTest, MultiSchemeMid) {
    MockFunction<std::function<bool(internal::ExpectationBase *)>> mockLambda;

    auto e1 = make_shared<internal::TypedExpectation<int()>>(nullptr, nullptr, 0, "",
                                                             internal::Function<int()>::ArgumentMatcherTuple());
    internal::CotestMockHandlerPool::GetOrCreateInstance()->AddExpectation(
        []() {});  // bumps the static global priority
    auto e2 = make_shared<internal::TypedExpectation<int()>>(nullptr, nullptr, 0, "",
                                                             internal::Function<int()>::ArgumentMatcherTuple());
    internal::CotestMockHandlerPool::GetOrCreateInstance()->AddExpectation(
        []() {});  // bumps the static global priority
    auto e3 = make_shared<internal::TypedExpectation<int()>>(nullptr, nullptr, 0, "",
                                                             internal::Function<int()>::ArgumentMatcherTuple());
    internal::CotestMockHandlerPool::GetOrCreateInstance()->AddExpectation(
        []() {});  // bumps the static global priority
    auto e4 = make_shared<internal::TypedExpectation<int()>>(nullptr, nullptr, 0, "",
                                                             internal::Function<int()>::ArgumentMatcherTuple());
    internal::CotestMockHandlerPool::GetOrCreateInstance()->AddExpectation(
        []() {});  // bumps the static global priority
    auto e5 = make_shared<internal::TypedExpectation<int()>>(nullptr, nullptr, 0, "",
                                                             internal::Function<int()>::ArgumentMatcherTuple());
    internal::CotestMockHandlerPool::GetOrCreateInstance()->AddExpectation(
        []() {});  // bumps the static global priority
    internal::MockHandlerScheme s1{e1, e3, e5};
    internal::MockHandlerScheme s2{e2};
    internal::MockHandlerScheme s3{e4};
    std::vector<const internal::MockHandlerScheme *> schemes{&s1, &s2, &s3};

    auto coro = COROUTINE() {
        // Exps queried in reverse order until one says yes
        WAIT_FOR_CALL(mockLambda, Call(e5.get())).RETURN(false);
        WAIT_FOR_CALL(mockLambda, Call(e4.get())).RETURN(false);
        WAIT_FOR_CALL(mockLambda, Call(e3.get())).RETURN(false);
        WAIT_FOR_CALL(mockLambda, Call(e2.get())).RETURN(true);
    };
    coro.WATCH_CALL(mockLambda);

    // The exp says yes.
    unsigned which;
    auto exp = internal::CotestMockHandlerPool::Finder(schemes, mockLambda.AsStdFunction(), &which);
    ASSERT_EQ(exp, e2.get());
    ASSERT_EQ(which, 1);
}

TEST(ExpectationFinderTest, MultiSchemeEarly) {
    MockFunction<std::function<bool(internal::ExpectationBase *)>> mockLambda;

    auto e1 = make_shared<internal::TypedExpectation<int()>>(nullptr, nullptr, 0, "",
                                                             internal::Function<int()>::ArgumentMatcherTuple());
    internal::CotestMockHandlerPool::GetOrCreateInstance()->AddExpectation(
        []() {});  // bumps the static global priority
    auto e2 = make_shared<internal::TypedExpectation<int()>>(nullptr, nullptr, 0, "",
                                                             internal::Function<int()>::ArgumentMatcherTuple());
    internal::CotestMockHandlerPool::GetOrCreateInstance()->AddExpectation(
        []() {});  // bumps the static global priority
    auto e3 = make_shared<internal::TypedExpectation<int()>>(nullptr, nullptr, 0, "",
                                                             internal::Function<int()>::ArgumentMatcherTuple());
    internal::CotestMockHandlerPool::GetOrCreateInstance()->AddExpectation(
        []() {});  // bumps the static global priority
    auto e4 = make_shared<internal::TypedExpectation<int()>>(nullptr, nullptr, 0, "",
                                                             internal::Function<int()>::ArgumentMatcherTuple());
    internal::CotestMockHandlerPool::GetOrCreateInstance()->AddExpectation(
        []() {});  // bumps the static global priority
    auto e5 = make_shared<internal::TypedExpectation<int()>>(nullptr, nullptr, 0, "",
                                                             internal::Function<int()>::ArgumentMatcherTuple());
    internal::CotestMockHandlerPool::GetOrCreateInstance()->AddExpectation(
        []() {});  // bumps the static global priority
    internal::MockHandlerScheme s1{e1, e3, e5};
    internal::MockHandlerScheme s2{e2};
    internal::MockHandlerScheme s3{e4};
    std::vector<const internal::MockHandlerScheme *> schemes{&s1, &s2, &s3};

    auto coro = COROUTINE() {
        // Exps queried in reverse order until one says yes
        WAIT_FOR_CALL(mockLambda, Call(e5.get())).RETURN(true);
    };
    coro.WATCH_CALL(mockLambda);

    // The exp says yes.
    unsigned which;
    auto exp = internal::CotestMockHandlerPool::Finder(schemes, mockLambda.AsStdFunction(), &which);
    ASSERT_EQ(exp, e5.get());
    ASSERT_EQ(which, 0);
}
