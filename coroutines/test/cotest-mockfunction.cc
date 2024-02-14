#include <string>

#include "cotest/cotest.h"
#include "gtest/gtest-spi.h"

using namespace std;
using ::testing::MockFunction;

//////////////////////////////////////////////
// The actual tests

TEST(MockFunctionTest, RunsCallbackWithBarArgument) {
    // 1. Create a mock object.
    MockFunction<int(string)> mock_function;

    // 2. Set expectations on Call() method.
    auto coro = COROUTINE() {
        auto e = NEXT_EVENT();
        auto e2 = e.IS_CALL(mock_function, Call("bar"));
        e2.RETURN(1);
        WAIT_FOR_CALL(mock_function, Call("foo")).RETURN(2);
        WAIT_FOR_CALL(mock_function, Call("Scooby")).RETURN(3);
        WAIT_FOR_CALL(mock_function, Call("Doo")).RETURN(4);
    };
    coro.WATCH_CALL(mock_function);

    // 3. Exercise code that uses std::function.
    EXPECT_EQ(mock_function.Call("bar"), 1);
    EXPECT_EQ(mock_function.AsStdFunction()("foo"), 2);
    std::function<int(string)> mf2 = mock_function.AsStdFunction();
    EXPECT_EQ(mf2("Scooby"), 3);
    EXPECT_EQ(mf2("Doo"), 4);
}

TEST(MockFunctionTest, MockFunc) {
    MockFunction<int(string)> mock_function;

    auto coro = COROUTINE() {
        WATCH_CALL(mock_function);
        auto d = LAUNCH(mock_function.Call("testing"));
        WAIT_FOR_CALL(mock_function, Call("testing")).RETURN(4);
        auto e = WAIT_FOR_RESULT();
        EXPECT_EQ(e(d), 4);
    };
}

TEST(MockFunctionTest, Minimal) {
    // Demonstrates use of GMock's MockFunction as a "semaphore" between
    // launch coroutine and test coroutine.
    MockFunction<void()> mock_function;

    auto coro = COROUTINE() {
        WATCH_CALL(mock_function);
        auto l = LAUNCH(mock_function.Call());  // Keep launch session in scope
        WAIT_FOR_CALL().RETURN();               // return type is void so signature not required
        WAIT_FOR_RESULT();
    };
}

COTEST(MockFunctionTest, HyperMinimal) {
    WATCH_CALL();
    LAUNCH(MockFunction<void()>().Call()),  // Temporary lasts to the semicolon.
        WAIT_FOR_CALL().RETURN(), WAIT_FOR_RESULT();
}
