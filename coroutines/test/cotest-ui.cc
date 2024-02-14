#include <string>

#include "cotest/cotest.h"
#include "gtest/gtest-spi.h"

using namespace std;
using ::testing::StrictMock;

//////////////////////////////////////////////
// Mocking assets

class ClassToMock {
   public:
    virtual ~ClassToMock() {}
    virtual int Mock1(int i) const = 0;
    virtual int Mock2() const = 0;
    virtual int Mock3(int x, const char *y, bool z) = 0;
    virtual int Mock4(int i) const = 0;
};
class MockClass : public ClassToMock {
   public:
    MOCK_METHOD(int, Mock1, (int i), (const, override));
    MOCK_METHOD(int, Mock2, (), (const, override));
    MOCK_METHOD(int, Mock3, (int x, const char *y, bool z), (override));
    MOCK_METHOD(int, Mock4, (int i), (const, override));
};

using ::testing::Return;

//////////////////////////////////////////////
// The actual tests

TEST(UserInterfaceTest, MethodSE) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE(MethodSE){{auto cs = WAIT_FOR_CALL(mock_object, Mock1);
    EXPECT_EQ(cs.GetArg<0>(), 100);
    cs.RETURN(10);
}

{
    auto cs = WAIT_FOR_CALL(mock_object, Mock3);
    EXPECT_EQ(cs.GetArg<0>(), 500);
    EXPECT_EQ(cs.GetArg<1>(), "abcd");
    EXPECT_FALSE(cs.GetArg<2>());
    cs.RETURN(30);
}
}
;
// Note that all mock methods are being sent to the same coroutine:
// Cannot use ON_CALL for these. ON_CALL sets behaviour on "uninteresting"
// calls which are ones with no expectations. But WATCH_CALL actually sets
// an expectation.

// absorb mock calls not accepted by coroutine
EXPECT_CALL(mock_object, Mock1).WillOnce(Return(-1));
EXPECT_CALL(mock_object, Mock2).WillOnce(Return(-1));
coro.WATCH_CALL(mock_object, Mock1);
coro.WATCH_CALL(mock_object, Mock2);
coro.WATCH_CALL(mock_object, Mock3);

// This is the body of the test case
EXPECT_EQ(mock_object.Mock2(), -1);
EXPECT_EQ(mock_object.Mock1(100), 10);
EXPECT_EQ(mock_object.Mock1(300), -1);
EXPECT_EQ(mock_object.Mock3(500, "abcd", false), 30);
}

TEST(UserInterfaceTest, MethodCheckNameSE) {
    StrictMock<MockClass> mock_object;
    auto coro = COROUTINE(MethodCheckNameSE){{auto cs = WAIT_FOR_CALL(mock_object, Mock4);
    EXPECT_EQ(cs.GetArg<0>(), 100);
    cs.RETURN(10);
}

{
    auto cs = WAIT_FOR_CALL(mock_object, Mock3);
    EXPECT_EQ(cs.GetArg<0>(), 500);
    EXPECT_EQ(cs.GetArg<1>(), "abcd");
    EXPECT_FALSE(cs.GetArg<2>());
    cs.RETURN(30);
}
}
;

// absorb mock calls not accepted by coroutine
EXPECT_CALL(mock_object, Mock1).WillOnce(Return(-1));
EXPECT_CALL(mock_object, Mock2).WillOnce(Return(-1));
coro.WATCH_CALL(mock_object, Mock1);
coro.WATCH_CALL(mock_object, Mock2);
coro.WATCH_CALL(mock_object, Mock4);
coro.WATCH_CALL(mock_object, Mock3);

// This is the body of the test case
// WAIT_FOR_MOCK_CLASS_SE(MockClass, Mock4); in coro requires Mock4 but MUT
// still calls Mock1 which has same signature. Mock1 should be rejected causing
// MUT to return false.
EXPECT_EQ(mock_object.Mock2(), -1);     // Passed by first wait due signature
EXPECT_EQ(mock_object.Mock1(200), -1);  // Passed due name (test expects Mock4)
EXPECT_EQ(mock_object.Mock4(100), 10);  // Accepted by first wait
EXPECT_EQ(mock_object.Mock3(500, "abcd", false),
          30);  // Accepted by second wait
}

TEST(UserInterfaceTest, NoMoveFromGenericCallSession) {
    StrictMock<MockClass> mock_object;
    auto coro = COROUTINE(NoMoveFromGenericCallSession) {
        auto cg = WAIT_FOR_CALL();
        EXPECT_TRUE(cg.IS_CALL(mock_object, Mock1(200)));
        COTEST_ASSERT(cg);  // should still be valid now we use shared_ptr
        cg.IS_CALL(mock_object, Mock1(200)).RETURN(10);
    };
    coro.WATCH_CALL(mock_object, Mock1);

    EXPECT_EQ(mock_object.Mock1(200), 10);
}
