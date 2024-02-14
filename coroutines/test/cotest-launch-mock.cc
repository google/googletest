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
    virtual int Mock1(int x) = 0;
    virtual int Mock2(int x) = 0;
};

class ExampleClass {
   public:
    ExampleClass(ClassToMock *dep_) : dep(dep_) {}

    int Example1(int a) { return dep->Mock1(a + 1) * 2; }

    int Example1a(int a) { return dep->Mock1(a - 1) * 5; }

    int ExampleNoMock() { return 33; }

    int Example2(int a) { return dep->Mock2(a + 2) * 3; }

    int Example1And2(int a) {
        int b = dep->Mock1(a);
        return dep->Mock2(a + 1) + b * 2;
    }

    void Example1Void(int a) { (void)dep->Mock1(a + 11); }

   private:
    ClassToMock *const dep;
};

////////////////////////////////////////////
// Mocking assets

class MockClass : public ClassToMock {
   public:
    MOCK_METHOD(int, Mock1, (int x), (override));
    MOCK_METHOD(int, Mock2, (int x), (override));
};

//////////////////////////////////////////////
// The actual tests

TEST(LaunchWithMockTest, Simple) {
    StrictMock<MockClass> mock_object;
    ExampleClass example(&mock_object);

    auto coro = COROUTINE() {
        WATCH_CALL();  // Needs to be inside coro body so it executes before we
                       // start driving MUT calls

        auto d = LAUNCH(example.Example1(4));

        auto e = NEXT_EVENT().IS_CALL(mock_object, Mock1(5));
        EXPECT_TRUE(e);
        EXPECT_TRUE(e.From(d));
        EXPECT_FALSE(e.FromMain());
        e.RETURN(1000);

        auto e2 = NEXT_EVENT();
        EXPECT_TRUE(e2.IS_RESULT(d));
        EXPECT_EQ(e2(d), 2000);
    };
}

TEST(LaunchWithMockTest, SimpleVoidReturn) {
    StrictMock<MockClass> mock_object;
    ExampleClass example(&mock_object);

    auto coro = COROUTINE() {
        WATCH_CALL();  // Needs to be inside coro body so it executes before we
                       // start driving MUT calls

        auto d = LAUNCH(example.Example1Void(4));

        auto e = NEXT_EVENT().IS_CALL(mock_object, Mock1);
        EXPECT_TRUE(e.IS_CALL(mock_object, Mock1(15)));
        EXPECT_TRUE(e.From(d));
        e.RETURN(1000);

        auto e2 = NEXT_EVENT();
        EXPECT_TRUE(e2.IS_RESULT(d));
    };
}

TEST(LaunchWithMockTest, Dual) {
    StrictMock<MockClass> mock_object;
    ExampleClass example(&mock_object);

    auto coro = COROUTINE() {
        WATCH_CALL();  // Needs to be inside coro body so it executes before we
                       // start driving MUT calls

        auto d1 = LAUNCH(example.Example1(4));

        auto e3 = NEXT_EVENT().IS_CALL(mock_object, Mock1(5));
        EXPECT_TRUE(e3);  // from d
        e3.ACCEPT();      // required to avoid deadlocking on GMock's mutex
        EXPECT_TRUE(e3.From(d1));

        auto d2 = LAUNCH(example.Example2(3));
        auto e2 = NEXT_EVENT().IS_CALL(mock_object, Mock2(5));
        EXPECT_TRUE(e2);
        EXPECT_TRUE(e2.From(d2));
        EXPECT_FALSE(e2.From(d1));
        EXPECT_TRUE(e2.IS_CALL());
        e2.ACCEPT();  // required to avoid deadlocking on GMock's mutex

        e3.RETURN(1000);
        auto e = NEXT_EVENT();
        EXPECT_TRUE(e.IS_RESULT(d1));
        EXPECT_EQ(e(d1), 2000);

        e2.RETURN(2000);

        e = NEXT_EVENT();
        EXPECT_TRUE(e.IS_RESULT(d2));
        EXPECT_EQ(e(d2), 6000);
    };
}

TEST(LaunchWithMockTest, DualNestMock) {
    StrictMock<MockClass> mock_object;
    ExampleClass example(&mock_object);

    auto coro = COROUTINE() {
        WATCH_CALL();  // Needs to be inside coro body so it executes before we
                       // start driving MUT calls

        auto d = LAUNCH(example.Example1(4));

        auto e = NEXT_EVENT().IS_CALL(mock_object, Mock1(5));
        EXPECT_TRUE(e.From(d));
        e.ACCEPT();  // required to avoid deadlocking on GMock's mutex

        auto d2 = LAUNCH(example.Example1a(3));

        auto e2 = NEXT_EVENT();
        EXPECT_TRUE(e2.IS_CALL().From(d2));
        e2.ACCEPT();  // required to avoid deadlocking on GMock's mutex
        EXPECT_TRUE(e2);
        e2.IS_CALL(mock_object, Mock1(2)).RETURN(2000);

        e2 = NEXT_EVENT();
        EXPECT_TRUE(e2.IS_RESULT(d2));
        EXPECT_EQ(e2(d2), 10000);

        e.RETURN(1000);

        auto e3 = NEXT_EVENT();
        EXPECT_TRUE(e3.IS_RESULT(d));
        EXPECT_EQ(e3(d), 2000);
    };
}

TEST(LaunchWithMockTest, DualWaitFrom) {
    StrictMock<MockClass> mock_object;
    ExampleClass example(&mock_object);

    auto coro = COROUTINE() {
        WATCH_CALL();  // Needs to be inside coro body so it executes before we
                       // start driving MUT calls

        auto d = LAUNCH(example.Example1(4));
        auto e4 = WAIT_FOR_CALL_FROM(d).IS_CALL(mock_object, Mock1(5));
        EXPECT_TRUE(e4);
        EXPECT_TRUE(e4.From(d));

        auto d2 = LAUNCH(example.Example1a(3));
        auto e2 = WAIT_FOR_CALL_FROM(mock_object, Mock1(2), d2);
        EXPECT_TRUE(e2.From(d2));

        auto d3 = LAUNCH(example.Example1a(9));
        auto e3 = WAIT_FOR_CALL_FROM(mock_object, d3).IS_CALL(mock_object, Mock1(8));
        EXPECT_TRUE(e3);
        EXPECT_TRUE(e3.From(d3));

        e4.RETURN(1000);
        auto e = NEXT_EVENT();
        EXPECT_TRUE(e.IS_RESULT(d));
        EXPECT_EQ(e(d), 2000);

        e2.RETURN(2000);
        auto er2 = NEXT_EVENT();
        EXPECT_TRUE(er2.IS_RESULT(d2));
        EXPECT_EQ(er2(d2), 10000);

        e3.RETURN(1001);
        auto er3 = NEXT_EVENT();
        EXPECT_TRUE(er3.IS_RESULT(d3));
        EXPECT_EQ(er3(d3), 5005);
    };
}

TEST(LaunchWithMockTest, EffectOfUnseenMockBase) {
    StrictMock<MockClass> mock_object;
    ExampleClass example(&mock_object);

    auto coro = COROUTINE() {
        WATCH_CALL();
        // This seems like a strange place to add an expectation but
        // we need to add it after the WATCH_CALL() because we want it
        // to have a higher priority, and yet it must be added before the
        // coroutine runs, which is immediately after its declaration.
        // See DelayStartExample for another approach.
        EXPECT_CALL(mock_object, Mock1).WillRepeatedly(Return(99));

        auto d = LAUNCH(example.Example1(4));

        auto e = NEXT_EVENT();
        EXPECT_TRUE(e.IS_RESULT(d));
        EXPECT_EQ(e(d), 198);
    };
}

TEST(LaunchWithMockTest, EffectOfUnseenMock1) {
    StrictMock<MockClass> mock_object;
    ExampleClass example(&mock_object);

    auto coro = COROUTINE() {
        WATCH_CALL();
        EXPECT_CALL(mock_object, Mock1).WillRepeatedly(Return(99));

        auto d1 = LAUNCH(example.Example1(4));
        auto e = NEXT_EVENT();
        EXPECT_TRUE(e.IS_RESULT(d1));
        EXPECT_EQ(e(d1), 198);

        auto d2 = LAUNCH(example.ExampleNoMock());
        e = NEXT_EVENT();
        EXPECT_TRUE(e.IS_RESULT(d2));
        EXPECT_EQ(e(d2), 33);
    };
}

TEST(LaunchWithMockTest, EffectOfUnseenMock2) {
    StrictMock<MockClass> mock_object;
    ExampleClass example(&mock_object);

    auto coro = COROUTINE() {
        WATCH_CALL();
        EXPECT_CALL(mock_object, Mock1).WillRepeatedly(Return(99));

        auto d1 = LAUNCH(example.Example1And2(4));

        auto e3 = NEXT_EVENT().IS_CALL(mock_object, Mock2(5));
        EXPECT_TRUE(e3);
        EXPECT_TRUE(e3.From(d1));
        e3.ACCEPT();

        auto d2 = LAUNCH(example.ExampleNoMock());

        auto e2 = NEXT_EVENT();
        EXPECT_TRUE(e2.IS_RESULT(d2));
        EXPECT_EQ(e2(d2), 33);

        e3.RETURN(1000);

        auto e = NEXT_EVENT();
        EXPECT_TRUE(e.IS_RESULT(d1));
        EXPECT_EQ(e(d1), 1000 + 99 * 2);
    };
}

TEST(LaunchWithMockTest, NextEvent1) {
    StrictMock<MockClass> mock_object;
    ExampleClass example(&mock_object);

    auto coro = COROUTINE() {
        WATCH_CALL();

        auto d1 = LAUNCH(example.ExampleNoMock());  // no mock call

        // d1/EM3() makes no mock call, so we expect it to return immediately,
        // and so we expect to collect its return before dealing with d2/EM4()
        auto e = NEXT_EVENT();
        EXPECT_TRUE(e.IS_RESULT(d1));
        EXPECT_EQ(e(d1), 33);

        auto d2 = LAUNCH(example.Example2(6));  // ->Mockmethod2()

        auto e2 = NEXT_EVENT().IS_CALL(mock_object, Mock2(8));
        EXPECT_TRUE(e2);
        EXPECT_TRUE(e2.From(d2));
        e2.RETURN(1000);

        e = NEXT_EVENT();
        EXPECT_TRUE(e.IS_RESULT(d2));
        EXPECT_EQ(e(d2), 3000);
    };
}

TEST(LaunchWithMockTest, NextEvent2) {
    StrictMock<MockClass> mock_object;
    ExampleClass example(&mock_object);

    auto coro = COROUTINE() {
        WATCH_CALL();

        auto d1 = LAUNCH(example.Example1(4));  // ->Mock1()
        auto e3 = NEXT_EVENT().IS_CALL(mock_object, Mock1);
        EXPECT_TRUE(e3);
        EXPECT_TRUE(e3.From(d1));
        e3.ACCEPT();

        auto d2 = LAUNCH(example.Example2(6));  // ->Mock2()
        auto e2 = NEXT_EVENT().IS_CALL(mock_object, Mock2(8));
        EXPECT_TRUE(e2);
        EXPECT_TRUE(e2.From(d2));
        e2.ACCEPT();

        e3.RETURN(99);
        auto e = NEXT_EVENT();
        EXPECT_TRUE(e.IS_RESULT(d1));
        EXPECT_EQ(e(d1), 99 * 2);

        e2.RETURN(1000);

        e = NEXT_EVENT();
        EXPECT_TRUE(e.IS_RESULT(d2));
        EXPECT_EQ(e(d2), 3000);
    };
}

TEST(LaunchWithMockTest, DelayStartExample) {
    StrictMock<MockClass> mock_object;
    ExampleClass example(&mock_object);

    // This test serves as an example of an alternate way of adding
    // expectations/watches at a higher priority than the coroutine.
    // Declare a MockFunction to wait for.
    MockFunction<void()> delay_start;

    auto coro = COROUTINE() {
        // Wait for it and immediately return
        WAIT_FOR_CALL(delay_start).RETURN();

        auto d = LAUNCH(example.Example1(4));

        auto e = NEXT_EVENT();
        EXPECT_TRUE(e.IS_RESULT(d));
        EXPECT_EQ(e(d), 198);
    };

    coro.WATCH_CALL();
    EXPECT_CALL(mock_object, Mock1).WillRepeatedly(Return(99));

    // Only call it once all the expectations/watches are added.
    delay_start.Call();
}
