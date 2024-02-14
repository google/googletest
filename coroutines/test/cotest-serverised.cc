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
    virtual void Mock1(int i) const = 0;
    // rule: call to this must be followed by call to MockExtra()
    virtual void Mock2(int i, int j) const = 0;
    virtual void Mock3(int i) const = 0;
    virtual void MockExtra() const = 0;
};

class ExampleClass {
   public:
    ExampleClass(ClassToMock *dep_) : dep(dep_) {}
    int Example1() {
        // In order
        dep->Mock1(1);
        dep->Mock2(2, 10);
        dep->MockExtra();
        dep->Mock3(3);

        // More calls, in a random order but obeying rule
        dep->Mock3(3);
        dep->Mock1(1);
        dep->Mock2(2, 10);
        dep->MockExtra();
        dep->Mock1(1);
        dep->Mock3(3);
        dep->Mock2(2, 10);
        dep->MockExtra();
        dep->Mock2(2, 10);
        dep->MockExtra();

        return 100;
    }
    int Example2() {
        // In order
        dep->Mock1(1);
        dep->Mock1(2);
        dep->MockExtra();
        dep->Mock1(3);

        // More calls, in a random order but obeying rule
        dep->Mock1(3);
        dep->Mock1(1);
        dep->Mock1(2);
        dep->MockExtra();
        dep->Mock1(3);
        dep->Mock1(2);
        dep->MockExtra();
        dep->Mock1(2);
        dep->MockExtra();
        dep->Mock1(3);
        dep->Mock1(1);
        dep->Mock1(1);
        dep->Mock1(1);
        dep->Mock1(3);

        return 101;
    }

   private:
    ClassToMock *const dep;
};

//////////////////////////////////////////////
// Mocking assets

class MockClass : public ClassToMock {
   public:
    MOCK_METHOD(void, Mock1, (int i), (const, override));
    MOCK_METHOD(void, Mock2, (int i, int j), (const, override));
    MOCK_METHOD(void, Mock3, (int i), (const, override));
    MOCK_METHOD(void, MockExtra, (), (const, override));
};

//////////////////////////////////////////////
// The actual tests

COTEST(ServerisedTest, Example1) {
    StrictMock<MockClass> mock_object;
    ExampleClass example(&mock_object);

    EXPECT_CALL(mock_object, Mock3(3)).WillRepeatedly(Return());
    WATCH_CALL();

    auto l = LAUNCH(example.Example1());

    while (true) {
        auto e = NEXT_EVENT();

        if (auto e1 = e.IS_CALL(mock_object, Mock1)) {
            // Mock1() is accepted, checked and returned
            EXPECT_EQ(e1.GetArg<0>(), 1);
            e1.RETURN();
        } else if (auto e2 = e.IS_CALL(mock_object, Mock2)) {
            // Mock2() is accepted, checked and returned, but we require
            // it to be followed by a call to MockExtra()
            EXPECT_EQ(e2.GetArg<0>(), 2);
            e2.RETURN();
            WAIT_FOR_CALL(mock_object, MockExtra).RETURN();
        } else if (auto e3 = e.IS_CALL(mock_object, Mock3)) {
            // Mock3 is dropped and the expectation deals with it
            e3.DROP();
        } else if (e.IS_RESULT()) {
            EXPECT_EQ(e(l), 100);
            // Avoid using return, for C++20 coro compatibility.
            EXIT_COROUTINE();
        } else {
            EXPECT_TRUE(!"unexpected event");
        }
    }
}

COTEST(ServerisedTest, Example2) {
    StrictMock<MockClass> mock_object;
    ExampleClass example(&mock_object);

    EXPECT_CALL(mock_object, Mock1(3)).WillRepeatedly(Return());
    WATCH_CALL();

    auto l = LAUNCH(example.Example2());

    while (true) {
        auto e = NEXT_EVENT();

        if (auto e1 = e.IS_CALL(mock_object, Mock1)) {
            switch (e1.GetArg<0>()) {
                case 1:
                    // When arg is 1, return
                    e1.RETURN();
                    break;
                case 2:
                    // When arg is 2, return and expect extra call
                    e1.RETURN();
                    WAIT_FOR_CALL(mock_object, MockExtra).RETURN();
                    break;
                case 3:
                    // When arg is 3, drop and the expectation deals with it
                    e1.DROP();
                    break;
                default:
                    EXPECT_TRUE(!"unexpected event");
                    break;
            }
        } else if (e.IS_RESULT()) {
            EXPECT_EQ(e(l), 101);
            // Avoid using return, for C++20 coro compatibility.
            EXIT_COROUTINE();
        } else {
            EXPECT_TRUE(!"unexpected event");
        }
    }
}

// TODO example in which we build our own version of WAIT_FOR_CALL that bakes in the
// special behaviours seen in these examples (requiring extra mock call, dropping)
