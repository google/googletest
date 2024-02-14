#include <string>

#include "cotest/cotest.h"
#include "gtest/gtest-spi.h"

using namespace std;
using namespace testing;
using ::testing::StrictMock;

////////////////////////////////////////////
// Code under test

struct ResultObjectDestructCounter {
    ~ResultObjectDestructCounter() { count++; }
    static int count;
};

int ResultObjectDestructCounter::count = 0;

class ClassToMock {
   public:
    virtual ~ClassToMock() {}
    virtual int Mock1() const = 0;
};

class ExampleClass {
   public:
    ResultObjectDestructCounter Example1() { return ResultObjectDestructCounter(); }
};

////////////////////////////////////////////
// Mocking assets

class MockClass : public ClassToMock {
   public:
    MOCK_METHOD(int, Mock1, (), (const, override));
};

//////////////////////////////////////////////
// The actual tests

TEST(ResultLifetimeTest, ReturnOverlapCase1) {
    StrictMock<MockClass> mock_object;
    ExampleClass example;

    COTEST_CLEANUP();
    ResultObjectDestructCounter::count = 0;

    auto coro = COROUTINE() {
        auto d = LAUNCH(example.Example1());
        auto e = NEXT_EVENT();

        auto d2 = LAUNCH(example.Example1());
        auto e2 = NEXT_EVENT();

        EXPECT_TRUE(e.IS_RESULT(d));
        EXPECT_TRUE(e2.IS_RESULT(d2));

        // Both launch sessions (d and d2) are still in scope and so the
        // returned objects should not have been destructed.
        EXPECT_EQ(ResultObjectDestructCounter::count, 0);
    };
}

TEST(ResultLifetimeTest, ReturnOverlapCase2) {
    StrictMock<MockClass> mock_object;
    ExampleClass example;

    COTEST_CLEANUP();
    ResultObjectDestructCounter::count = 0;

    auto coro = COROUTINE() {
        auto d = LAUNCH(example.Example1());
        auto e = NEXT_EVENT();
        EXPECT_TRUE(e.IS_RESULT(d));

        auto d2 = LAUNCH(example.Example1());
        auto e2 = NEXT_EVENT();
        EXPECT_TRUE(e2.IS_RESULT(d2));

        // Both launch sessions (d and d2) are still in scope and so the
        // returned objects should not have been destructed.
        EXPECT_EQ(ResultObjectDestructCounter::count, 0);
    };
}

TEST(ResultLifetimeTest, ReturnOverlapCase3) {
    StrictMock<MockClass> mock_object;
    ExampleClass example;

    COTEST_CLEANUP();
    ResultObjectDestructCounter::count = 0;

    {
        auto coro = COROUTINE() {
            {
                auto d = LAUNCH(example.Example1());
                auto e = NEXT_EVENT();
                EXPECT_TRUE(e.IS_RESULT(d));
            }
            auto d2 = LAUNCH(example.Example1());
            auto e2 = NEXT_EVENT();
            EXPECT_TRUE(e2.IS_RESULT(d2));

            // Check that at least one launch session (d2) has not had its
            // return object destructed. Note the _LE condition - destruction
            // is not required by the implementation.
            EXPECT_LE(ResultObjectDestructCounter::count, 1);
        };
    }
}
