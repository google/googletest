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
    virtual int Mock3(int i) const = 0;
    virtual int Mock4(int i) const = 0;
    virtual int Mock4(int i) = 0;
};
class MockClass : public ClassToMock {
   public:
    MOCK_METHOD(int, Mock1, (int i), (const, override));
    MOCK_METHOD(int, Mock2, (int i, int j), (const, override));
    MOCK_METHOD(int, Mock3, (int i), (const, override));
    MOCK_METHOD(int, Mock4, (int i), (const, override));
    MOCK_METHOD(int, Mock4, (int i), (override));
};

//////////////////////////////////////////////
// The actual tests
TEST(LambdaTest, Lambda) {
    StrictMock<MockClass> mock_object;
    StrictMock<MockClass> mock_object2;

    auto coro = COROUTINE(Lambda) {
        auto cg = WAIT_FOR_CALL();
        EXPECT_FALSE(cg.IS_CALL(mock_object, Mock1(201)));
        EXPECT_FALSE(cg.IS_CALL(mock_object2, Mock1));
        EXPECT_FALSE(cg.IS_CALL(mock_object2, Mock1(_)));
        EXPECT_FALSE(cg.IS_CALL(mock_object2, Mock1(200)));
        EXPECT_TRUE(cg.IS_CALL(mock_object, Mock1));
        EXPECT_TRUE(cg.IS_CALL(mock_object, Mock1(_)));
        EXPECT_TRUE(cg.IS_CALL(mock_object, Mock1(200)).RETURN(20));
        cg = WAIT_FOR_CALL();
        EXPECT_FALSE(cg.IS_CALL(mock_object, Mock2(_, 401)));
        EXPECT_TRUE(cg.IS_CALL(mock_object, Mock2));
        EXPECT_TRUE(cg.IS_CALL(mock_object, Mock2(_, _)));
        EXPECT_TRUE(cg.IS_CALL(mock_object, Mock2(_, 400)));
        EXPECT_TRUE(cg.IS_CALL(mock_object, Mock2(200, _)).RETURN(30));
    };

    coro.WATCH_CALL(mock_object, Mock1);
    coro.WATCH_CALL(mock_object, Mock2);

    EXPECT_EQ(mock_object.Mock1(200), 20);
    EXPECT_EQ(mock_object.Mock2(200, 400), 30);
}

TEST(LambdaTest, WatchInside) {
    StrictMock<MockClass> mock_object;
    StrictMock<MockClass> mock_object2;

    auto coro = COROUTINE()  // Test the no name given case
    {
        WATCH_CALL(mock_object, Mock1);
        WATCH_CALL(mock_object, Mock2);

        auto cg = WAIT_FOR_CALL();
        EXPECT_FALSE(cg.IS_CALL(mock_object, Mock1(201)));
        EXPECT_FALSE(cg.IS_CALL(mock_object2, Mock1));
        EXPECT_FALSE(cg.IS_CALL(mock_object2, Mock1(_)));
        EXPECT_FALSE(cg.IS_CALL(mock_object2, Mock1(200)));
        EXPECT_TRUE(cg.IS_CALL(mock_object, Mock1));
        EXPECT_TRUE(cg.IS_CALL(mock_object, Mock1(_)));
        EXPECT_TRUE(cg.IS_CALL(mock_object, Mock1(200)).RETURN(20));
        cg = WAIT_FOR_CALL();
        EXPECT_FALSE(cg.IS_CALL(mock_object, Mock2(_, 401)));
        EXPECT_TRUE(cg.IS_CALL(mock_object, Mock2));
        EXPECT_TRUE(cg.IS_CALL(mock_object, Mock2(_, _)));
        EXPECT_TRUE(cg.IS_CALL(mock_object, Mock2(_, 400)));
        EXPECT_TRUE(cg.IS_CALL(mock_object, Mock2(200, _)).RETURN(30));
    };

    EXPECT_EQ(mock_object.Mock1(200), 20);
    EXPECT_EQ(mock_object.Mock2(200, 400), 30);
}
