#include <string>

#include "cotest/cotest.h"
#include "gtest/gtest-spi.h"

using namespace std;
using ::testing::StrictMock;
using namespace testing;

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
    virtual void Mock5(int i) const = 0;
    virtual void Mock6(int i, int j) const = 0;
};
class MockClass : public ClassToMock {
   public:
    MOCK_METHOD(int, Mock1, (int i), (const, override));
    MOCK_METHOD(int, Mock2, (int i, int j), (const, override));
    MOCK_METHOD(int, Mock3, (int i), (const, override));
    MOCK_METHOD(int, Mock4, (int i), (const, override));
    MOCK_METHOD(int, Mock4, (int i), (override));
    MOCK_METHOD(void, Mock5, (int i), (const, override));
    MOCK_METHOD(void, Mock6, (int i, int j), (const, override));
};

using ::testing::Return;

//////////////////////////////////////////////
// The actual tests

TEST(ExteriorWildcardTest, TwoMethodWaiting) {
    StrictMock<MockClass> mock_object;
    StrictMock<MockClass> mock_object2;

    // Try doing this early, to simulate a generic setup phase
    EXPECT_CALL(mock_object2, Mock1).WillRepeatedly(Return(-2));

    auto coro = COROUTINE(TwoMethodWaiting) {
        auto cg = WAIT_FOR_CALL(mock_object);
        EXPECT_TRUE(cg.IS_CALL(mock_object, Mock1(200)).RETURN(20));
        auto cg2 = WAIT_FOR_CALL(mock_object);
        EXPECT_TRUE(cg2.IS_CALL(mock_object, Mock2(200, 400)).RETURN(30));
    };

    EXPECT_CALL(mock_object2, Mock2).Times(2).WillRepeatedly(Return(-3));
    coro.WATCH_CALL();

    EXPECT_EQ(mock_object2.Mock1(500), -2);
    EXPECT_EQ(mock_object2.Mock2(500, 600), -3);
    EXPECT_EQ(mock_object.Mock1(200), 20);
    EXPECT_EQ(mock_object2.Mock1(501), -2);
    EXPECT_EQ(mock_object2.Mock2(501, 601), -3);
    EXPECT_EQ(mock_object.Mock2(200, 400), 30);
}

TEST(ExteriorWildcardTest, TwoMethodWaitingMO) {
    StrictMock<MockClass> mock_object;
    StrictMock<MockClass> mock_object2;

    auto coro = COROUTINE(TwoMethodWaiting) {
        auto cg = WAIT_FOR_CALL();
        EXPECT_TRUE(cg.IS_CALL(mock_object, Mock1(200)).RETURN(20));
        auto cg2 = WAIT_FOR_CALL();
        EXPECT_TRUE(cg2.IS_CALL(mock_object, Mock2(200, 400)).RETURN(30));
    };

    EXPECT_CALL(mock_object2, Mock1).Times(2).WillRepeatedly(Return(-2));
    EXPECT_CALL(mock_object2, Mock2).Times(2).WillRepeatedly(Return(-3));
    coro.WATCH_CALL(mock_object);

    EXPECT_EQ(mock_object2.Mock1(500), -2);
    EXPECT_EQ(mock_object2.Mock2(500, 600), -3);
    EXPECT_EQ(mock_object.Mock1(200), 20);
    EXPECT_EQ(mock_object2.Mock1(501), -2);
    EXPECT_EQ(mock_object2.Mock2(501, 601), -3);
    EXPECT_EQ(mock_object.Mock2(200, 400), 30);
}

TEST(ExteriorWildcardTest, TwoMethodWaitingPre) {
    StrictMock<MockClass> mock_object;
    StrictMock<MockClass> mock_object2;

    auto coro = COROUTINE(TwoMethodWaitingPre) {
        auto cg = WAIT_FOR_CALL(mock_object);
        EXPECT_TRUE(cg.IS_CALL(mock_object, Mock1(200)).RETURN(20));
        cg = WAIT_FOR_CALL(mock_object);
        EXPECT_TRUE(cg.IS_CALL(mock_object, Mock2(200, 400)).RETURN(30));
    };

    EXPECT_CALL(mock_object2, Mock1).Times(2).WillRepeatedly(Return(-2));
    EXPECT_CALL(mock_object2, Mock2).Times(2).WillRepeatedly(Return(-3));
    coro.WATCH_CALL();
    EXPECT_CALL(mock_object, Mock1(1000)).WillRepeatedly(Return(-10));
    EXPECT_CALL(mock_object2, Mock1(1100)).WillRepeatedly(Return(-11));

    EXPECT_EQ(mock_object2.Mock1(500), -2);
    EXPECT_EQ(mock_object2.Mock2(500, 600), -3);
    EXPECT_EQ(mock_object.Mock1(200), 20);
    EXPECT_EQ(mock_object.Mock1(1000), -10);
    EXPECT_EQ(mock_object2.Mock1(1100), -11);
    EXPECT_EQ(mock_object2.Mock1(501), -2);
    EXPECT_EQ(mock_object2.Mock2(501, 601), -3);
    EXPECT_EQ(mock_object.Mock2(200, 400), 30);
}

TEST(ExteriorWildcardTest, TwoMethodWaitingPreMO) {
    StrictMock<MockClass> mock_object;
    StrictMock<MockClass> mock_object2;

    auto coro = COROUTINE(TwoMethodWaitingPre) {
        auto cg = WAIT_FOR_CALL();
        EXPECT_TRUE(cg.IS_CALL(mock_object, Mock1(200)).RETURN(20));
        cg = WAIT_FOR_CALL();
        EXPECT_TRUE(cg.IS_CALL(mock_object, Mock2(200, 400)).RETURN(30));
    };

    EXPECT_CALL(mock_object2, Mock1).Times(2).WillRepeatedly(Return(-2));
    EXPECT_CALL(mock_object2, Mock2).Times(2).WillRepeatedly(Return(-3));
    coro.WATCH_CALL(mock_object);
    EXPECT_CALL(mock_object, Mock1(1000)).WillRepeatedly(Return(-10));
    EXPECT_CALL(mock_object2, Mock1(1100)).WillRepeatedly(Return(-11));

    EXPECT_EQ(mock_object2.Mock1(500), -2);
    EXPECT_EQ(mock_object2.Mock2(500, 600), -3);
    EXPECT_EQ(mock_object.Mock1(200), 20);
    EXPECT_EQ(mock_object.Mock1(1000), -10);
    EXPECT_EQ(mock_object2.Mock1(1100), -11);
    EXPECT_EQ(mock_object2.Mock1(501), -2);
    EXPECT_EQ(mock_object2.Mock2(501, 601), -3);
    EXPECT_EQ(mock_object.Mock2(200, 400), 30);
}

TEST(ExteriorWildcardTest, MultiPriority) {
    StrictMock<MockClass> mock_object;
    StrictMock<MockClass> mock_object2;

    auto coro1 = COROUTINE() {
        auto cg = WAIT_FOR_CALL(mock_object);
        EXPECT_TRUE(cg.IS_CALL(mock_object, Mock1(200)).RETURN(20));
        // Exit without RETIRE() is saturation
    };

    auto coro2 = COROUTINE() {
        auto cg = WAIT_FOR_CALL(mock_object2);
        EXPECT_TRUE(cg.IS_CALL(mock_object2, Mock2(_, 400)).With(Lt()).RETURN(30));
        RETIRE();
    };

    coro1.WATCH_CALL();
    EXPECT_CALL(mock_object, Mock1).WillOnce(Return(-10)).RetiresOnSaturation();  // #1
    EXPECT_CALL(mock_object2, Mock2).WillOnce(Return(-11));                       // #2
    coro2.WATCH_CALL();

    EXPECT_EQ(mock_object.Mock1(1000),
              -10);  // coro2's wait drops; expectation #1 matches and retires
    EXPECT_EQ(mock_object.Mock1(200),
              20);  // coro2's wait drops; expectation #1 has retired; coro1
                    // accepts and is saturated

    EXPECT_EQ(mock_object2.Mock2(200, 400), 30);  // coro2 accepts and retires
    EXPECT_EQ(mock_object2.Mock2(200, 400),
              -11);  // coro2 has retired; expectation #2 matches and is saturated
}

TEST(ExteriorWildcardTest, MultiPriorityMO) {
    StrictMock<MockClass> mock_object;
    StrictMock<MockClass> mock_object2;

    auto coro1 = COROUTINE() {
        auto cg = WAIT_FOR_CALL();
        EXPECT_TRUE(cg.IS_CALL(mock_object, Mock1(200)).RETURN(20));
        // Exit without RETIRE() is saturation
    };

    auto coro2 = COROUTINE() {
        auto cg = WAIT_FOR_CALL();
        EXPECT_TRUE(cg.IS_CALL(mock_object2, Mock2(_, 400)).With(Lt()).RETURN(30));
        RETIRE();
    };

    coro1.WATCH_CALL(mock_object);
    EXPECT_CALL(mock_object, Mock1).WillOnce(Return(-10)).RetiresOnSaturation();  // #1
    EXPECT_CALL(mock_object2, Mock2).WillOnce(Return(-11));                       // #2
    coro2.WATCH_CALL(mock_object2);

    EXPECT_EQ(mock_object.Mock1(1000),
              -10);  // coro2's wait drops; expectation #1 matches and retires
    EXPECT_EQ(mock_object.Mock1(200),
              20);  // coro2's wait drops; expectation #1 has retired; coro1
                    // accepts and is saturated

    EXPECT_EQ(mock_object2.Mock2(200, 400), 30);  // coro2 accepts and retires
    EXPECT_EQ(mock_object2.Mock2(200, 400),
              -11);  // coro2 has retired; expectation #2 matches and is saturated
}

TEST(ExteriorWildcardTest, MockObjectAddressAlias) {
    auto coro = COROUTINE() {
        WAIT_FOR_CALL().RETURN();
        WAIT_FOR_CALL().RETURN();
    };

    {
        StrictMock<MockClass> mock_object;
        coro.WATCH_CALL(mock_object);

        mock_object.Mock5(200);
    }

    {
        StrictMock<MockClass> mock_object2;

        // Known bug with WATCH_CALL( mock object )
        // This call should not make it into the coroutine
        // but it does because mock_object2 is at the same address as
        // the now-deleted mock_object1. Not easy to fix: consider if
        // we hadn't made any calls on mock_object - then the GMock code
        // that registers the mocker would not have run. If we assume
        // a call, then we could maybe use GMock's registry to recover
        // the relationship - then we'd also need DetachMocker() so that
        // the CotestWatcher can deduce that it needs to detach from the
        // mock object too.
        // However, the test case to cause this is very strange and seems
        // to require the coro to be outside the scope of the mock objects -
        // otherwise Watchers will be discarded before here.
        mock_object2.Mock6(200, 400);
    }
}

TEST(ExteriorWildcardTest, StackedCoros) {
    StrictMock<MockClass> mock_object;
    StrictMock<MockClass> mock_object2;

    auto coro1 = COROUTINE(coro1) {
        WATCH_CALL();  // watch all mock calls

        auto cg = WAIT_FOR_CALL();
        EXPECT_TRUE(cg.IS_CALL(mock_object, Mock1(1000)).RETURN(-10));
        cg = WAIT_FOR_CALL(mock_object);
        EXPECT_TRUE(cg.IS_CALL(mock_object, Mock1(200)).RETURN(20));
        auto cs = WAIT_FOR_CALL(mock_object, Mock2);
        EXPECT_TRUE(cs.IS_CALL(mock_object, Mock2(220, _)));
        cs.RETURN(-11);
        RETIRE();
    };

    auto coro2 = COROUTINE(coro2) {
        WATCH_CALL();  // watch all mock calls

        auto cg = WAIT_FOR_CALL(mock_object2);
        EXPECT_TRUE(cg.IS_CALL(mock_object2, Mock2(_, 400)).With(Lt()).RETURN(30));
        auto cs = WAIT_FOR_CALL(mock_object, Mock1(1100));
        cs.RETURN(-5);
        cg = WAIT_FOR_CALL();
        EXPECT_TRUE(cg.IS_CALL(mock_object2, Mock2(300, 350)).With(Lt()).RETURN(33));
        SATISFY();
        cg = WAIT_FOR_CALL(mock_object).RETURN();
    };

    EXPECT_EQ(mock_object.Mock1(1000),
              -10);  // c2 is looking for any mock call on mo2, so drops it and c1
                     // is looking for any so accepts, checks, returns
    EXPECT_EQ(mock_object2.Mock2(200, 400),
              30);  // c2 is looking for any mock call on mo2, so accepts, checks,
                    // returns
    EXPECT_EQ(mock_object.Mock1(200),
              20);  // c2 is now looking for mo.MM1 with arg==1100, so drops it and c1
                    // is now looking for any call on mo so accepts, checks, returns

    EXPECT_EQ(mock_object.Mock2(220, 400),
              -11);  // c2 is still looking for mo.MM1 with arg==1100, so drops it and
                     // c1 is now looking for mo.MM2 mo so accepts, checks, returns
    EXPECT_EQ(mock_object.Mock1(1100),
              -5);  // c2 is still looking for mo.MM1 with arg==1100, so accepts,
                    // checks, returns
    EXPECT_EQ(mock_object2.Mock2(300, 350),
              33);  // c2 is now looking for any call so accepts, checks, returns

    // c1 has retired, which means it will drop any further calls (none are made
    // here) c2 is left looking for any call on mo, but it's satisfied, so if the
    // call never arrives, there's no error
}
