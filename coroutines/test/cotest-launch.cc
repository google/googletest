#include <string>

#include "cotest/cotest.h"
#include "gtest/gtest-spi.h"

using namespace std;
using namespace testing;
using ::testing::StrictMock;

////////////////////////////////////////////
// Code under test

class ClassToMock {
   public:
    virtual ~ClassToMock() {}
    virtual int Mock1() const = 0;
};

class ExampleClass {
   public:
    int Example1() { return 6; }
    int Example2(int i) { return i * 3; }
    void Example3() {}
    int m_i = 99;
    int& Example4() { return m_i; }
    int operator++() { return 7; }
    int Example6(int i, int j) { return i * 3 - j; }
};

////////////////////////////////////////////
// Mocking assets

class MockClass : public ClassToMock {
   public:
    MOCK_METHOD(int, Mock1, (), (const, override));
};

//////////////////////////////////////////////
// The actual tests

TEST(LaunchTest, Simple) {
    StrictMock<MockClass> mock_object;
    ExampleClass example;

    auto coro = COROUTINE() {
        auto d = LAUNCH(example.Example2(4));
        auto e = NEXT_EVENT();
        EXPECT_FALSE(e.IS_CALL());
        EXPECT_FALSE(e.IS_CALL(mock_object));
        EXPECT_FALSE(e.IS_CALL(mock_object, Mock1));
        EXPECT_TRUE(e.IS_RESULT());
        EXPECT_TRUE(e.IS_RESULT(d));
        EXPECT_EQ(e(d), 12);
    };
}

TEST(LaunchTest, VoidReturn1) {
    StrictMock<MockClass> mock_object;
    ExampleClass example;

    auto coro = COROUTINE() {
        auto d = LAUNCH(example.Example3());
        auto e = NEXT_EVENT();
        EXPECT_FALSE(e.IS_CALL());
        EXPECT_FALSE(e.IS_CALL(mock_object));
        EXPECT_FALSE(e.IS_CALL(mock_object, Mock1));
        EXPECT_TRUE(e.IS_RESULT());
        EXPECT_TRUE(e.IS_RESULT(d));
    };
}

TEST(LaunchTest, VoidReturn2) {
    StrictMock<MockClass> mock_object;
    ExampleClass example;

    auto coro = COROUTINE() {
        auto d = LAUNCH(example.Example3());
        auto e = NEXT_EVENT();
        EXPECT_FALSE(e.IS_CALL());
        EXPECT_FALSE(e.IS_CALL(mock_object));
        EXPECT_FALSE(e.IS_CALL(mock_object, Mock1));
        EXPECT_TRUE(e.IS_RESULT());
        EXPECT_TRUE(e.IS_RESULT(d));
        e(d);  // this is OK but evaluates to void
    };
}

TEST(LaunchTest, RefReturn) {
    StrictMock<MockClass> mock_object;
    ExampleClass example;

    auto coro = COROUTINE() {
        auto d = LAUNCH(example.Example4());
        auto e = NEXT_EVENT();
        EXPECT_FALSE(e.IS_CALL());
        EXPECT_FALSE(e.IS_CALL(mock_object));
        EXPECT_FALSE(e.IS_CALL(mock_object, Mock1));
        EXPECT_TRUE(e.IS_RESULT());
        EXPECT_TRUE(e.IS_RESULT(d));
        EXPECT_EQ(e(d), 99);
    };
}

TEST(LaunchTest, NestedEasy) {
    StrictMock<MockClass> mock_object;
    ExampleClass example;

    auto coro = COROUTINE() {
        auto d = LAUNCH(example.Example1());
        auto e = NEXT_EVENT();
        EXPECT_FALSE(e.IS_CALL());
        EXPECT_FALSE(e.IS_CALL(mock_object));
        EXPECT_FALSE(e.IS_CALL(mock_object, Mock1));
        EXPECT_TRUE(e.IS_RESULT());
        EXPECT_TRUE(e.IS_RESULT(d));
        EXPECT_EQ(e(d), 6);

        // Easy to support nesting between return value extraction and cleanup
        // (but useful: this is when by-reference return objects are valid).
        auto d2 = LAUNCH(example.Example2(5));
        auto e2 = NEXT_EVENT();
        EXPECT_FALSE(e2.IS_CALL());
        EXPECT_FALSE(e2.IS_CALL(mock_object));
        EXPECT_FALSE(e2.IS_CALL(mock_object, Mock1));
        EXPECT_TRUE(e2.IS_RESULT());
        EXPECT_FALSE(e2.IS_RESULT(d));
        EXPECT_TRUE(e2.IS_RESULT(d2));
        EXPECT_EQ(e2(d2), 15);
    };
}

TEST(LaunchTest, NestedHard) {
    StrictMock<MockClass> mock_object;
    ExampleClass example;

    auto coro = COROUTINE() {
        auto d = LAUNCH(example.Example1());
        auto e = NEXT_EVENT();

        auto d2 = LAUNCH(example.Example2(5));

        EXPECT_FALSE(e.IS_CALL());
        EXPECT_FALSE(e.IS_CALL(mock_object));
        EXPECT_FALSE(e.IS_CALL(mock_object, Mock1));
        EXPECT_TRUE(e.IS_RESULT());
        EXPECT_TRUE(e.IS_RESULT(d));
        EXPECT_EQ(e(d), 6);

        auto e2 = NEXT_EVENT();
        EXPECT_FALSE(e2.IS_CALL());
        EXPECT_FALSE(e2.IS_CALL(mock_object));
        EXPECT_FALSE(e2.IS_CALL(mock_object, Mock1));
        EXPECT_TRUE(e2.IS_RESULT());
        EXPECT_FALSE(e2.IS_RESULT(d));
        EXPECT_TRUE(e2.IS_RESULT(d2));
        EXPECT_EQ(e2(d2), 15);
    };
}

TEST(LaunchTest, Operator) {
    StrictMock<MockClass> mock_object;
    ExampleClass example;

    auto coro = COROUTINE() {
        auto d = LAUNCH(++example);
        auto e = NEXT_EVENT();
        EXPECT_FALSE(e.IS_CALL());
        EXPECT_FALSE(e.IS_CALL(mock_object));
        EXPECT_FALSE(e.IS_CALL(mock_object, Mock1));
        EXPECT_TRUE(e.IS_RESULT());
        EXPECT_TRUE(e.IS_RESULT(d));
        EXPECT_EQ(e(d), 7);
    };
}

TEST(LaunchTest, Exit1) {
    GTEST_SKIP() << "Not allowed: exiting with a launch session that may not "
                    "have completed";

    StrictMock<MockClass> mock_object;
    ExampleClass example;

    auto coro = COROUTINE() { auto d = LAUNCH(example.Example2(4)); };
}

TEST(LaunchTest, Exit2) {
    GTEST_SKIP() << "Not allowed: exiting with an incompleted event session";

    StrictMock<MockClass> mock_object;
    ExampleClass example;

    auto coro = COROUTINE() {
        auto d = LAUNCH(example.Example2(4));
        auto e = NEXT_EVENT();
    };
}

TEST(LaunchTest, ReturnOverlapCase1) {
    StrictMock<MockClass> mock_object;
    ExampleClass example;

    auto coro = COROUTINE() {
        auto d = LAUNCH(example.Example1());
        auto e = NEXT_EVENT();

        auto d2 = LAUNCH(example.Example2(5));
        auto e2 = NEXT_EVENT();

        EXPECT_TRUE(e.IS_RESULT(d));
        EXPECT_TRUE(e2.IS_RESULT(d2));
        EXPECT_EQ(e(d), 6);
        EXPECT_EQ(e2(d2), 15);
    };
}

TEST(LaunchTest, ReturnOverlapCase2) {
    StrictMock<MockClass> mock_object;
    ExampleClass example;

    auto coro = COROUTINE() {
        auto d = LAUNCH(example.Example1());
        auto e = NEXT_EVENT();
        EXPECT_TRUE(e.IS_RESULT(d));

        auto d2 = LAUNCH(example.Example2(5));
        auto e2 = NEXT_EVENT();
        EXPECT_TRUE(e2.IS_RESULT(d2));

        EXPECT_EQ(e(d), 6);
        EXPECT_EQ(e2(d2), 15);
    };
}

TEST(LaunchTest, SimpleIR) {
    StrictMock<MockClass> mock_object;
    ExampleClass example;

    auto coro = COROUTINE() {
        auto d = LAUNCH(example.Example2(4));
        auto e = NEXT_EVENT();
        EXPECT_FALSE(e.IS_CALL());
        EXPECT_FALSE(e.IS_CALL(mock_object));
        EXPECT_FALSE(e.IS_CALL(mock_object, Mock1));
        EXPECT_TRUE(e.IS_RESULT());
        // Demonstrate use of the return of IS_RESULT
        EXPECT_EQ(e.IS_RESULT()(d), 12);
    };
}

TEST(LaunchTest, FromMain) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE(MethodName) {
        auto cs = WAIT_FOR_CALL(mock_object, Mock1);
        EXPECT_TRUE(cs.FromMain());
        cs.RETURN(10);
    };

    coro.WATCH_CALL(mock_object, Mock1);

    EXPECT_EQ(mock_object.Mock1(), 10);
}

TEST(LaunchTest, CommaInExpr) {
    StrictMock<MockClass> mock_object;
    ExampleClass example;

    auto coro = COROUTINE() {
        auto d = LAUNCH(example.Example6(4, 44));
        auto e = NEXT_EVENT();
        EXPECT_FALSE(e.IS_CALL());
        EXPECT_FALSE(e.IS_CALL(mock_object));
        EXPECT_FALSE(e.IS_CALL(mock_object, Mock1));
        EXPECT_TRUE(e.IS_RESULT());
        EXPECT_TRUE(e.IS_RESULT(d));
        EXPECT_EQ(e(d), 12 - 44);
    };
}

TEST(LaunchTest, ShortForm) {
    ExampleClass example;

    auto coro = COROUTINE() {
        auto d = LAUNCH(example.Example6(4, 44));
        auto e = WAIT_FOR_RESULT();
        EXPECT_EQ(e(d), 12 - 44);
    };
}

TEST(LaunchTest, ShorterForm) {
    ExampleClass example;

    auto coro = COROUTINE() { EXPECT_EQ(WAIT_FOR_RESULT()(LAUNCH(example.Example6(4, 44))), 12 - 44); };
}

TEST(LaunchTest, ShortForm2) {
    ExampleClass example;

    auto coro = COROUTINE() {
        auto d = LAUNCH(example.Example6(4, 44));
        EXPECT_EQ(NEXT_EVENT()(d), 12 - 44);
    };
}

COTEST(LaunchTest, VeryShortForm) { EXPECT_EQ(NEXT_EVENT()(LAUNCH(ExampleClass().Example6(4, 44))), 12 - 44); }
