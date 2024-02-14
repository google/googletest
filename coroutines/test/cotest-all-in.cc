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
    virtual int Mock1(int i) = 0;
    virtual int Mock2(int i, int j) const = 0;
    virtual int Mock3(int i) const = 0;
    virtual int Mock4(int i) const = 0;
    virtual int Mock4(int i) = 0;
};

class ExampleClass {
   public:
    ExampleClass(ClassToMock *dep_) : dep(dep_) {}

    int Example1(int a) { return dep->Mock1(a + 1) * 2; }

    int Example2(int a) { return dep->Mock1(a + 1) * dep->Mock1(0); }

   private:
    ClassToMock *const dep;
};

//////////////////////////////////////////////
// Mocking assets

class MockClass : public ClassToMock {
   public:
    MOCK_METHOD(int, Mock1, (int i), (override));
    MOCK_METHOD(int, Mock2, (int i, int j), (const, override));
    MOCK_METHOD(int, Mock3, (int i), (const, override));
    MOCK_METHOD(int, Mock4, (int i), (const, override));
    MOCK_METHOD(int, Mock4, (int i), (override));
};

//////////////////////////////////////////////
// The actual tests

StrictMock<MockClass> *mock_object_p = nullptr;

/*
 * Why this works
 *
 * Expectations, watches etc are created in the coro but under shared_ptr<>
 * and are held alive by the mockers in the mock object. In this test case,
 * the mockers outlast the coro itself, so we end up using DetachCoroutine()
 * etc.
 */
TEST(AllInTest, WatchInside) {
    StrictMock<MockClass> mock_object;
    StrictMock<MockClass> mock_object2;

    auto coro = COROUTINE() {
        WATCH_CALL(mock_object, Mock1);
        WATCH_CALL(mock_object, Mock2);

        // See later tests in which LAUNCH() is used to run code-under-test
        // from within coroutine.
        mock_object_p = &mock_object;

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

    ASSERT_TRUE(mock_object_p);

    EXPECT_EQ(mock_object_p->Mock1(200), 20);
    EXPECT_EQ(mock_object_p->Mock2(200, 400), 30);

    mock_object_p = nullptr;
}

TEST(AllInTest, LaunchMock) {
    auto coro = COROUTINE() {
        StrictMock<MockClass> mock_object;
        ExampleClass example(&mock_object);

        WATCH_CALL();

        auto d = LAUNCH(example.Example1(4));

        auto e = NEXT_EVENT();
        EXPECT_TRUE(e.IS_CALL(mock_object, Mock1(5)).RETURN(1000));

        e = NEXT_EVENT();
        EXPECT_TRUE(e.IS_RESULT(d));
        EXPECT_EQ(e(d), 2000);
    };
}

COTEST(AllInTest, CotestMacro) {
    StrictMock<MockClass> mock_object;
    ExampleClass example(&mock_object);

    WATCH_CALL();

    auto d = LAUNCH(example.Example1(4));

    auto cs = WAIT_FOR_CALL(mock_object, Mock1);
    EXPECT_TRUE(cs.GetArg<0>() == 5);
    cs.RETURN(1000);

    auto e = WAIT_FOR_RESULT();
    EXPECT_EQ(e(d), 2000);
}

COTEST(AllInTest, CotestMacroWithExpect) {
    StrictMock<MockClass> mock_object;
    ExampleClass example(&mock_object);

    // EXPECT_CALL() is fine inside a COTEST(). Semantics are the
    // same.
    EXPECT_CALL(mock_object, Mock1(0)).WillRepeatedly(Return(1));
    WATCH_CALL();

    auto d = LAUNCH(example.Example2(4));

    auto cs = WAIT_FOR_CALL(mock_object, Mock1(Gt(0)));
    EXPECT_TRUE(cs.GetArg<0>() == 5);
    cs.RETURN(1000);

    auto e = WAIT_FOR_RESULT();
    EXPECT_EQ(e(d), 1000);
}
