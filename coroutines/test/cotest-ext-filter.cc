#include <string>

#include "cotest/cotest.h"
#include "gtest/gtest-spi.h"

using namespace std;
using namespace testing;
using ::testing::StrictMock;

//////////////////////////////////////////////
// Mocking assets

class ClassToMock {
   public:
    virtual ~ClassToMock() {}
    virtual int Mock1(int i) const = 0;
    virtual int Mock2(int i, int j) const = 0;
};
class MockClass : public ClassToMock {
   public:
    MOCK_METHOD(int, Mock1, (int i), (const, override));
    MOCK_METHOD(int, Mock2, (int i, int j), (const, override));
};

//////////////////////////////////////////////
// The actual tests

TEST(UserInterfaceTest, Method1_Base) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE(Method1) {
        auto cs = WAIT_FOR_CALL(mock_object, Mock1);
        EXPECT_EQ(cs.GetArg<0>(), 200);
        cs.RETURN(10);
    };

    coro.WATCH_CALL(mock_object, Mock1);

    EXPECT_EQ(mock_object.Mock1(200), 10);
}

TEST(UserInterfaceTest, Method1_Underscore) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE(Method1) {
        auto cs = WAIT_FOR_CALL(mock_object, Mock1);
        EXPECT_EQ(cs.GetArg<0>(), 200);
        cs.RETURN(10);
    };

    coro.WATCH_CALL(mock_object, Mock1(_));

    EXPECT_EQ(mock_object.Mock1(200), 10);
}

TEST(UserInterfaceTest, Method1_Match) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE(Method1) {
        auto cs = WAIT_FOR_CALL(mock_object, Mock1);
        EXPECT_EQ(cs.GetArg<0>(), 200);
        cs.RETURN(10);
    };

    coro.WATCH_CALL(mock_object, Mock1(200));

    EXPECT_EQ(mock_object.Mock1(200), 10);
}

TEST(UserInterfaceTest, Method1_NoMatch) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE(FailIfCall) {
        SATISFY();  // If we never get the call, we're fine
        WAIT_FOR_CALL();
        COTEST_ASSERT(false);
    };

    EXPECT_CALL(mock_object, Mock1(200)).WillOnce(Return(20));
    coro.WATCH_CALL(mock_object, Mock1(100));

    EXPECT_EQ(mock_object.Mock1(200), 20);
}

TEST(UserInterfaceTest, Method2_Base) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE(Method2) {
        auto cs = WAIT_FOR_CALL(mock_object, Mock2);
        EXPECT_EQ(cs.GetArg<0>(), 200);
        EXPECT_EQ(cs.GetArg<1>(), 300);
        cs.RETURN(10);
    };

    coro.WATCH_CALL(mock_object, Mock2);

    EXPECT_EQ(mock_object.Mock2(200, 300), 10);
}

TEST(UserInterfaceTest, Method2_Underscore) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE(Method2) {
        auto cs = WAIT_FOR_CALL(mock_object, Mock2);
        EXPECT_EQ(cs.GetArg<0>(), 200);
        EXPECT_EQ(cs.GetArg<1>(), 300);
        cs.RETURN(10);
    };

    coro.WATCH_CALL(mock_object, Mock2(_, _));

    EXPECT_EQ(mock_object.Mock2(200, 300), 10);
}

TEST(UserInterfaceTest, Method2_Mix1) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE(Method2) {
        auto cs = WAIT_FOR_CALL(mock_object, Mock2);
        EXPECT_EQ(cs.GetArg<0>(), 200);
        EXPECT_EQ(cs.GetArg<1>(), 300);
        cs.RETURN(10);
    };

    coro.WATCH_CALL(mock_object, Mock2(_, 300));

    EXPECT_EQ(mock_object.Mock2(200, 300), 10);
}

TEST(UserInterfaceTest, Method2_Mix2) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE(Method2) {
        auto cs = WAIT_FOR_CALL(mock_object, Mock2);
        EXPECT_EQ(cs.GetArg<0>(), 200);
        EXPECT_EQ(cs.GetArg<1>(), 300);
        cs.RETURN(10);
    };

    coro.WATCH_CALL(mock_object, Mock2(200, _));

    EXPECT_EQ(mock_object.Mock2(200, 300), 10);
}

TEST(UserInterfaceTest, Method2_Match) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE(Method2) {
        auto cs = WAIT_FOR_CALL(mock_object, Mock2);
        EXPECT_EQ(cs.GetArg<0>(), 200);
        EXPECT_EQ(cs.GetArg<1>(), 300);
        cs.RETURN(10);
    };

    coro.WATCH_CALL(mock_object, Mock2(200, 300));

    EXPECT_EQ(mock_object.Mock2(200, 300), 10);
}

TEST(UserInterfaceTest, Method2_Mix_Mismatch) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE(FailIfCall) {
        SATISFY();  // If we never get the call, we're fine
        WAIT_FOR_CALL();
        COTEST_ASSERT(false);
    };

    EXPECT_CALL(mock_object, Mock2(200, _)).WillOnce(Return(20));
    coro.WATCH_CALL(mock_object, Mock2(202, _));

    EXPECT_EQ(mock_object.Mock2(200, 300), 20);
}

TEST(UserInterfaceTest, Method2_Mismatch) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE(FailIfCall) {
        SATISFY();  // If we never get the call, we're fine
        WAIT_FOR_CALL();
        COTEST_ASSERT(false);
    };

    EXPECT_CALL(mock_object, Mock2(200, 300)).WillOnce(Return(20));
    coro.WATCH_CALL(mock_object, Mock2(200, 303));

    EXPECT_EQ(mock_object.Mock2(200, 300), 20);
}

TEST(UserInterfaceTest, Method2_With) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE(Method2_With) {
        auto cs = WAIT_FOR_CALL(mock_object, Mock2);
        EXPECT_EQ(cs.GetArg<0>(), 200);
        EXPECT_EQ(cs.GetArg<1>(), 300);
        cs.RETURN(10);
    };

    coro.WATCH_CALL(mock_object, Mock2).With(Lt());

    EXPECT_EQ(mock_object.Mock2(200, 300), 10);
}

TEST(UserInterfaceTest, Method2_With_Mismatch) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE(Method2_With_Mismatch) {
        SATISFY();  // If we never get the call, we're fine
        WAIT_FOR_CALL();
        COTEST_ASSERT(false);
    };

    EXPECT_CALL(mock_object, Mock2).WillOnce(Return(20));
    coro.WATCH_CALL(mock_object, Mock2).With(Gt());

    EXPECT_EQ(mock_object.Mock2(200, 300), 20);
}

TEST(UserInterfaceTest, OverlappingWatchers) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE() {
        auto e = NEXT_EVENT();
        e.DROP();
    };

    EXPECT_CALL(mock_object, Mock2).WillOnce(Return(20));
    // These two watchers overlap in scope: cotest must ensure the
    // coro only "sees" the mock call once.
    coro.WATCH_CALL(mock_object, Mock2);
    coro.WATCH_CALL();

    EXPECT_EQ(mock_object.Mock2(200, 300), 20);
}
