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

class ClassToMockA {
   public:
    virtual ~ClassToMockA() {}
    virtual int Mock(int i) const = 0;
};
class MockClassA : public ClassToMockA {
   public:
    MOCK_METHOD(int, Mock, (int i), (const, override));
};

class ClassToMockB {
   public:
    virtual ~ClassToMockB() {}
    virtual int Mock(int i) const = 0;
};
class MockClassB : public ClassToMockB {
   public:
    MOCK_METHOD(int, Mock, (int i), (const, override));
};

//////////////////////////////////////////////
// The actual tests

TEST(InteriorFilteringTest, MethodBasic) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE(MethodBasic) {
        auto cg = WAIT_FOR_CALL();
        EXPECT_TRUE(cg.FromMain());
        EXPECT_FALSE(cg.IS_CALL(mock_object, Mock1(201)));
        EXPECT_TRUE(cg.IS_CALL(mock_object, Mock1));
        EXPECT_TRUE(cg.IS_CALL(mock_object, Mock1(_)));
        EXPECT_TRUE(cg.IS_CALL(mock_object, Mock1(200)).RETURN(10));
        cg = WAIT_FOR_CALL();
        EXPECT_FALSE(cg.IS_CALL(mock_object, Mock2(_, 401)));
        EXPECT_TRUE(cg.IS_CALL(mock_object, Mock2));
        EXPECT_TRUE(cg.IS_CALL(mock_object, Mock2(_, _)));
        EXPECT_TRUE(cg.IS_CALL(mock_object, Mock2(_, 400)));
        EXPECT_TRUE(cg.IS_CALL(mock_object, Mock2(200, _)).RETURN(20));
    };

    coro.WATCH_CALL(mock_object, Mock1);
    coro.WATCH_CALL(mock_object, Mock2);

    EXPECT_EQ(mock_object.Mock1(200), 10);
    EXPECT_EQ(mock_object.Mock2(200, 400), 20);
}

TEST(InteriorFilteringTest, MethodName) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE(MethodName) {
        auto cg = WAIT_FOR_CALL();
        EXPECT_FALSE(cg.IS_CALL(mock_object, Mock3));
        EXPECT_FALSE(cg.IS_CALL(mock_object, Mock3(_)));
        EXPECT_FALSE(cg.IS_CALL(mock_object, Mock3(200)));
        EXPECT_TRUE(cg.IS_CALL(mock_object, Mock1).RETURN(10));
    };

    coro.WATCH_CALL(mock_object, Mock1);

    EXPECT_EQ(mock_object.Mock1(200), 10);
}

TEST(InteriorFilteringTest, MethodConst) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE(MethodConst) {
        auto cg = WAIT_FOR_CALL();
        EXPECT_FALSE(cg.IS_CALL(Const(mock_object), Mock4(_)));
        EXPECT_TRUE(cg.IS_CALL(mock_object, Mock4(_)).RETURN(30));
        cg = WAIT_FOR_CALL();
        EXPECT_FALSE(cg.IS_CALL(mock_object, Mock4(_)));
        EXPECT_TRUE(cg.IS_CALL(Const(mock_object), Mock4(_)).RETURN(40));
    };

    const StrictMock<MockClass> &const_ref_mock_object(mock_object);
    coro.WATCH_CALL(mock_object, Mock4(_));
    coro.WATCH_CALL(Const(mock_object), Mock4(_));

    EXPECT_EQ(mock_object.Mock4(200), 30);
    EXPECT_EQ(const_ref_mock_object.Mock4(200), 40);
}

TEST(InteriorFilteringTest, TwoMethod) {
    StrictMock<MockClass> mock_object;
    StrictMock<MockClass> mock_object2;

    auto coro = COROUTINE(TwoMethod) {
        auto cg = WAIT_FOR_CALL();
        EXPECT_FALSE(cg.IS_CALL(mock_object, Mock1(201)));
        EXPECT_FALSE(cg.IS_CALL(mock_object2, Mock1));
        EXPECT_FALSE(cg.IS_CALL(mock_object2, Mock1(_)));
        EXPECT_FALSE(cg.IS_CALL(mock_object2, Mock1(200)));
        EXPECT_TRUE(cg.IS_CALL(mock_object));  // 1-arg IS_CALL() matches by mock object
        EXPECT_FALSE(cg.IS_CALL(mock_object2));
        EXPECT_TRUE(cg.IS_CALL(mock_object, Mock1));
        EXPECT_TRUE(cg.IS_CALL(mock_object, Mock1(_)));
        EXPECT_TRUE(cg.IS_CALL(mock_object, Mock1(200)).RETURN(20));
        cg = WAIT_FOR_CALL();
        EXPECT_FALSE(cg.IS_CALL(mock_object, Mock2(_, 401)));
        EXPECT_TRUE(cg.IS_CALL(mock_object, Mock2(Ne(201), Ne(399))));
        EXPECT_FALSE(cg.IS_CALL(mock_object, Mock2(Ne(200), _)));
        EXPECT_TRUE(cg.IS_CALL(mock_object));
        EXPECT_FALSE(cg.IS_CALL(mock_object2));
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

TEST(InteriorFilteringTest, MethodByClass) {
    StrictMock<MockClassA> mock_object_a;
    StrictMock<MockClassB> mock_object_b;

    auto coro = COROUTINE(MethodByClass) {
        auto cg = WAIT_FOR_CALL();
        EXPECT_FALSE(cg.IS_CALL(mock_object_b, Mock));
        EXPECT_TRUE(cg.IS_CALL(mock_object_a, Mock).RETURN(10));
    };
    coro.WATCH_CALL(mock_object_a, Mock);

    EXPECT_EQ(mock_object_a.Mock(200), 10);
}

TEST(InteriorFilteringTest, WithClause) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE(WithClause) {
        auto cg = WAIT_FOR_CALL();
        EXPECT_TRUE(cg.IS_CALL(mock_object, Mock2(_, _)).With(Lt()));
        EXPECT_FALSE(cg.IS_CALL(mock_object, Mock2(0, 0)).With(Lt()));  // With() on NULL call session must be safe
        auto cs = cg.IS_CALL(mock_object, Mock2);
        EXPECT_THAT(cs.GetArgs(), Lt());
        EXPECT_THAT(cs.GetArgs(), Not(Ge()));
        cs.RETURN(10);
        cg = WAIT_FOR_CALL();
        EXPECT_FALSE(cg.IS_CALL(mock_object, Mock2(_, _)).With(Lt()));
        cg.IS_CALL(mock_object, Mock2).With(Gt()).RETURN(20);
    };

    coro.WATCH_CALL(mock_object, Mock2);

    EXPECT_EQ(mock_object.Mock2(200, 400), 10);
    EXPECT_EQ(mock_object.Mock2(200, 100), 20);
}

MATCHER(IsEven, "") { return (arg % 2) == 0; }

TEST(InteriorFilteringTest, Complex) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE(Complex) {
        while (true) {
            auto cs = MOCK_CALL_HANDLE(mock_object, Mock1(_));
            auto cs2 = MOCK_CALL_HANDLE(mock_object, Mock1);

            cs = WAIT_FOR_CALL().IS_CALL(mock_object, Mock1(IsEven()));
            EXPECT_TRUE(cs);
            cs.RETURN(10);
            cs2 = WAIT_FOR_CALL().IS_CALL(mock_object, Mock1(Not(IsEven())));
            EXPECT_TRUE(cs2);
            cs2.RETURN(10);
            SATISFY();
        }
    };

    coro.WATCH_CALL(mock_object, Mock1);
    for (int i = 0; i < 10; i++) EXPECT_EQ(mock_object.Mock1(i), 10);
}

TEST(InteriorFilteringTest, ComplexWaiting) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE(ComplexWaiting) {
        while (true) {
            // Be a better neighbour by passing on unmatching calls rather than
            // generating an error here - but we may be passing on too many
            auto cs = WAIT_FOR_CALL(mock_object, Mock1(IsEven()));
            EXPECT_TRUE(cs);
            cs.RETURN(10);
            cs = WAIT_FOR_CALL(mock_object, Mock1(Not(IsEven())));
            EXPECT_TRUE(cs);
            cs.RETURN(10);
            SATISFY();
        }
    };

    coro.WATCH_CALL(mock_object, Mock1);
    for (int i = 0; i < 10; i++) EXPECT_EQ(mock_object.Mock1(i), 10);
}

TEST(InteriorFilteringTest, ComplexWaitingPrio) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE(ComplexWaitingPrio) {
        while (true) {
            // Only drop negative args
            auto cs = WAIT_FOR_CALL(mock_object, Mock1(Ge(0)));
            EXPECT_TRUE(cs.WithArg<0>(IsEven()));
            EXPECT_FALSE(cs.WithArg<0>(Not(IsEven())));
            cs.RETURN(10);
            cs = WAIT_FOR_CALL(mock_object, Mock1(Ge(0)));
            EXPECT_TRUE(cs.WithArg<0>(Not(IsEven())));
            EXPECT_FALSE(cs.WithArg<0>(IsEven()));
            cs.RETURN(10);
            SATISFY();
        }
    };

    // Of course, the EXPECT_CALL could come after the WATCH_CALL and
    // therefore have a higher priority, and thus do the filtering, but this way
    // tests the coro more.
    EXPECT_CALL(mock_object, Mock1(-1)).Times(3).WillRepeatedly(Return(-1));
    coro.WATCH_CALL(mock_object, Mock1);
    for (int i = 0; i < 10; i++) {
        EXPECT_EQ(mock_object.Mock1(i), 10);
        if (i % 7 == 0 || i % 5 == 0) {
            EXPECT_EQ(mock_object.Mock1(-1), -1);
        }
    }
}

TEST(InteriorFilteringTest, ComplexWaitingPrioET) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE(ComplexWaitingPrioET) {
        while (true) {
            // Pass on negative args
            auto cs = WAIT_FOR_CALL(mock_object, Mock1(Ge(0)));
            EXPECT_THAT(cs.GetArg<0>(), IsEven());
            cs.RETURN(10);
            cs = WAIT_FOR_CALL(mock_object, Mock1(Ge(0)));
            EXPECT_THAT(cs.GetArg<0>(), Not(IsEven()));
            EXPECT_THAT(cs.GetArg<0>() + 1,
                        IsEven());  // show that we have the underlying int
            cs.RETURN(10);
            SATISFY();
        }
    };

    EXPECT_CALL(mock_object, Mock1(-5)).Times(3).WillRepeatedly(Return(-2));
    coro.WATCH_CALL(mock_object, Mock1);
    for (int i = 0; i < 10; i++) {
        EXPECT_EQ(mock_object.Mock1(i), 10);
        if (i % 7 == 0 || i % 5 == 0) {
            EXPECT_EQ(mock_object.Mock1(-5), -2);
        }
    }
}

TEST(InteriorFilteringTest, TwoMethodWaiting) {
    StrictMock<MockClass> mock_object;
    StrictMock<MockClass> mock_object2;

    auto coro = COROUTINE(TwoMethodWaiting) {
        auto cg = WAIT_FOR_CALL(mock_object);
        EXPECT_TRUE(cg.IS_CALL(mock_object, Mock1(200)).RETURN(20));
        cg = WAIT_FOR_CALL(mock_object);
        EXPECT_TRUE(cg.IS_CALL(mock_object, Mock2(200, 400)).RETURN(30));
    };

    EXPECT_CALL(mock_object2, Mock1).Times(2).WillRepeatedly(Return(-2));
    EXPECT_CALL(mock_object2, Mock2).Times(2).WillRepeatedly(Return(-3));
    coro.WATCH_CALL(mock_object, Mock1);
    coro.WATCH_CALL(mock_object, Mock2);
    coro.WATCH_CALL(mock_object2, Mock1);
    coro.WATCH_CALL(mock_object2, Mock2);

    EXPECT_EQ(mock_object2.Mock1(500), -2);
    EXPECT_EQ(mock_object2.Mock2(500, 600), -3);
    EXPECT_EQ(mock_object.Mock1(200), 20);
    EXPECT_EQ(mock_object2.Mock1(501), -2);
    EXPECT_EQ(mock_object2.Mock2(501, 601), -3);
    EXPECT_EQ(mock_object.Mock2(200, 400), 30);
    // If we want to make further calls here,
    // we will need the coro to RETIRE() to avoid oversaturation,
    // see test case TwoMethodWaitingAndRetire
}

TEST(InteriorFilteringTest, TwoMethodWaitingAndRetire) {
    StrictMock<MockClass> mock_object;
    StrictMock<MockClass> mock_object2;

    auto coro = COROUTINE(TwoMethodWaitingAndRetire) {
        auto cg = WAIT_FOR_CALL(mock_object);
        EXPECT_TRUE(cg.IS_CALL(mock_object, Mock1(200)).RETURN(20));
        cg = WAIT_FOR_CALL(mock_object);
        EXPECT_TRUE(cg.IS_CALL(mock_object, Mock2(200, 400)).RETURN(30));
        // We believe we're done: retire to drop all subsequent calls
        RETIRE();
    };

    EXPECT_CALL(mock_object2, Mock1).Times(3).WillRepeatedly(Return(-2));
    EXPECT_CALL(mock_object2, Mock2).Times(3).WillRepeatedly(Return(-3));
    coro.WATCH_CALL(mock_object, Mock1);
    coro.WATCH_CALL(mock_object, Mock2);
    coro.WATCH_CALL(mock_object2, Mock1);
    coro.WATCH_CALL(mock_object2, Mock2);

    EXPECT_EQ(mock_object2.Mock1(500), -2);
    EXPECT_EQ(mock_object2.Mock2(500, 600), -3);
    EXPECT_EQ(mock_object.Mock1(200), 20);
    EXPECT_EQ(mock_object2.Mock1(501), -2);
    EXPECT_EQ(mock_object2.Mock2(501, 601), -3);
    EXPECT_EQ(mock_object.Mock2(200, 400), 30);
    // These actually reach the retired coroutine
    EXPECT_EQ(mock_object2.Mock1(502), -2);
    EXPECT_EQ(mock_object2.Mock2(502, 602), -3);
}
