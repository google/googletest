#include <string>

#include "cotest/internal/cotest-coro-thread.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

//#define EXPLICIT_FOR_GDB

using namespace std;
using namespace coro_impl;

using ::testing::StrictMock;

using CoroImplType = CoroOnThread;

//////////////////////////////////////////////
// Mocking assets

class ClassToMock {
   public:
    virtual ~ClassToMock() {}
    virtual void Mock2(int degrees) = 0;
    virtual int Mock1() const = 0;
};

class MockClass : public ClassToMock {
   public:
    MOCK_METHOD(void, Mock2, (int degrees), (override));
    MOCK_METHOD(int, Mock1, (), (const, override));
};

struct TestPayload : public Payload {
    TestPayload(int value_) : value(value_) {}
    std::string DebugString() const final { return "TestPayload(" + to_string(value) + ")"; }
    int value;
};

std::unique_ptr<Payload> from_coro;
struct IterateCoro0 {
    IterateCoro0(ExteriorInterface *coroutine_) : coroutine(coroutine_) {}
    int operator()() {
        from_coro = coroutine->Iterate(nullptr);
        return PeekPayload<TestPayload>(from_coro).value;
    }

    ExteriorInterface *const coroutine;
};

//////////////////////////////////////////////
// The actual tests

TEST(ActionFunctorTest, WithReturn) {
    StrictMock<MockClass> mock_object;

    auto cl = [](InteriorInterface *ii) {
        std::unique_ptr<Payload> from_main;
        from_main = ii->Yield(nullptr);  // waiting for the first mock call
        from_main = ii->Yield(MakePayload<TestPayload>(100));
        from_main = ii->Yield(MakePayload<TestPayload>(200));
        from_main = ii->Yield(MakePayload<TestPayload>(300));
    };
    CoroImplType coroutine(std::bind(cl, &coroutine), "coroutine");
#ifdef EXPLICIT_FOR_GDB
    const IterateCoro0 functor(&coroutine);
    const testing::Action<int()> action(functor);  // lands on Action(G&& fun) then void Init(G&& g,
                                                   // ::true_type)
    EXPECT_CALL(mock_object, Mock1()).Times(3).WillRepeatedly(action);
#else
    EXPECT_CALL(mock_object, Mock1()).Times(3).WillRepeatedly(IterateCoro0(&coroutine));
#endif
    from_coro = coroutine.Iterate(nullptr);
    EXPECT_FALSE(from_coro);

    // This is the body of the test case
    EXPECT_EQ(mock_object.Mock1(), 100);
    EXPECT_EQ(mock_object.Mock1(), 200);
    EXPECT_EQ(mock_object.Mock1(), 300);

    // Allow coroutine to exit
    from_coro = coroutine.Iterate(nullptr);
    EXPECT_FALSE(from_coro);
}

struct IterateCoro1 {
    IterateCoro1(ExteriorInterface *coroutine_) : coroutine(coroutine_) {}
    void operator()(int arg0) { from_coro = coroutine->Iterate(MakePayload<TestPayload>(arg0)); }

    ExteriorInterface *const coroutine;
};

TEST(ActionFunctorTest, WithArg) {
    StrictMock<MockClass> mock_object;

    auto cl = [](InteriorInterface *ii) {
        std::unique_ptr<Payload> from_main;
        from_main = ii->Yield(nullptr);
        EXPECT_EQ(PeekPayload<TestPayload>(from_main).value, 45);
        from_main = ii->Yield(nullptr);
        EXPECT_EQ(PeekPayload<TestPayload>(from_main).value, 90);
        from_main = ii->Yield(nullptr);
        EXPECT_EQ(PeekPayload<TestPayload>(from_main).value, 180);
        from_main = ii->Yield(nullptr);  // return from the last mock call
    };
    CoroImplType coroutine(std::bind(cl, &coroutine), "coroutine");
    EXPECT_CALL(mock_object, Mock2).Times(3).WillRepeatedly(IterateCoro1(&coroutine));

    from_coro = coroutine.Iterate(nullptr);
    EXPECT_FALSE(from_coro);

    // This is the body of the test case
    mock_object.Mock2(45);
    mock_object.Mock2(90);
    mock_object.Mock2(180);

    // Allow coroutine to exit
    from_coro = coroutine.Iterate(nullptr);
    EXPECT_FALSE(from_coro);
}
