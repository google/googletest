#include <string>

#include "cotest/internal/cotest-coro-thread.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace std;
using namespace coro_impl;

struct TestPayload : public Payload {
    TestPayload(int value_) : value(value_) {}
    std::string DebugString() const final { return "TestPayload(" + to_string(value) + ")"; }
    int value;
};

struct DestructorCheck {
    DestructorCheck() { undestructed_count++; }
    ~DestructorCheck() { undestructed_count--; }
    static int undestructed_count;
};

int DestructorCheck::undestructed_count = 0;

TEST(CoroTestThread, SimpleYieldingSequence) {
    string coro_stage = "init";
    auto cl = [&](InteriorInterface *ii) {
        DestructorCheck dc;
        std::unique_ptr<Payload> from_main;
        coro_stage = "point 1";
        from_main = ii->Yield(MakePayload<TestPayload>(10));
        EXPECT_TRUE(from_main);
        EXPECT_EQ(PeekPayload<TestPayload>(from_main).value, 100);
        coro_stage = "point 2";
        from_main = ii->Yield(MakePayload<TestPayload>(20));
        EXPECT_TRUE(from_main);
        EXPECT_EQ(PeekPayload<TestPayload>(from_main).value, 200);
        coro_stage = "point 3";
    };
    CoroOnThread coroutine(std::bind(cl, &coroutine), "coroutine");

    std::unique_ptr<Payload> from_coro;

    EXPECT_FALSE(coroutine.IsCoroutineExited());

    from_coro = coroutine.Iterate(nullptr);
    EXPECT_FALSE(coroutine.IsCoroutineExited());
    EXPECT_TRUE(from_coro);
    EXPECT_EQ(PeekPayload<TestPayload>(from_coro).value, 10);
    EXPECT_EQ(coro_stage, "point 1");

    from_coro = coroutine.Iterate(MakePayload<TestPayload>(100));
    EXPECT_TRUE(from_coro);
    EXPECT_EQ(PeekPayload<TestPayload>(from_coro).value, 20);
    EXPECT_FALSE(coroutine.IsCoroutineExited());
    EXPECT_EQ(coro_stage, "point 2");

    from_coro = coroutine.Iterate(MakePayload<TestPayload>(200));
    EXPECT_FALSE(from_coro);
    EXPECT_TRUE(coroutine.IsCoroutineExited());
    EXPECT_EQ(coro_stage, "point 3");

    EXPECT_EQ(DestructorCheck::undestructed_count, 0);
}

TEST(CoroTestThread, LongYieldingSequence) {
    int coro_stage = -1;
    const int n = 1000;
    auto cl = [&](InteriorInterface *ii) {
        DestructorCheck dc;
        std::unique_ptr<Payload> from_main;
        for (int i = 0; i < n; i++) {
            coro_stage = i;
            from_main = ii->Yield(MakePayload<TestPayload>(i * 10));
            EXPECT_TRUE(from_main);
            EXPECT_EQ(PeekPayload<TestPayload>(from_main).value, (i + 1) * 100);
        }
        coro_stage = n;
    };
    CoroOnThread coroutine(std::bind(cl, &coroutine), "coroutine");

    std::unique_ptr<Payload> from_coro;

    EXPECT_FALSE(coroutine.IsCoroutineExited());

    from_coro = coroutine.Iterate(nullptr);
    EXPECT_FALSE(coroutine.IsCoroutineExited());
    EXPECT_TRUE(from_coro);
    EXPECT_EQ(PeekPayload<TestPayload>(from_coro).value, 0);
    EXPECT_EQ(coro_stage, 0);

    for (int i = 1; i < n; i++) {
        from_coro = coroutine.Iterate(MakePayload<TestPayload>(i * 100));
        EXPECT_FALSE(coroutine.IsCoroutineExited());
        EXPECT_TRUE(from_coro);
        EXPECT_EQ(PeekPayload<TestPayload>(from_coro).value, i * 10);
        EXPECT_EQ(coro_stage, i);
    }

    from_coro = coroutine.Iterate(MakePayload<TestPayload>(n * 100));
    EXPECT_TRUE(coroutine.IsCoroutineExited());
    EXPECT_FALSE(from_coro);
    EXPECT_EQ(coro_stage, n);

    EXPECT_EQ(DestructorCheck::undestructed_count, 0);
}

TEST(CoroTestThread, Cancelling) {
    string coro_stage = "init";
    auto cl = [&](InteriorInterface *ii) {
        DestructorCheck dc;
        std::unique_ptr<Payload> from_main;
        coro_stage = "point 1";
        from_main = ii->Yield(MakePayload<TestPayload>(10));
        EXPECT_TRUE(from_main);
        EXPECT_EQ(PeekPayload<TestPayload>(from_main).value, 100);
        coro_stage = "point 2";
        from_main = ii->Yield(MakePayload<TestPayload>(20));
        EXPECT_TRUE(from_main);
        EXPECT_EQ(PeekPayload<TestPayload>(from_main).value, 200);
        coro_stage = "point 3";
    };
    CoroOnThread coroutine(std::bind(cl, &coroutine), "coroutine");

    std::unique_ptr<Payload> from_coro;

    EXPECT_FALSE(coroutine.IsCoroutineExited());

    from_coro = coroutine.Iterate(nullptr);
    EXPECT_FALSE(coroutine.IsCoroutineExited());
    EXPECT_TRUE(from_coro);
    EXPECT_EQ(PeekPayload<TestPayload>(from_coro).value, 10);
    EXPECT_EQ(coro_stage, "point 1");

    coroutine.Cancel();
    EXPECT_TRUE(coroutine.IsCoroutineExited());

    EXPECT_EQ(DestructorCheck::undestructed_count, 0);
}

TEST(CoroTestThread, ImmediateCancelling) {
    string coro_stage = "init";
    auto cl = [&](InteriorInterface *ii) {
        DestructorCheck dc;
        std::unique_ptr<Payload> from_main;
        coro_stage = "point 1";
        from_main = ii->Yield(MakePayload<TestPayload>(10));
        EXPECT_TRUE(from_main);
        EXPECT_EQ(PeekPayload<TestPayload>(from_main).value, 100);
        coro_stage = "point 2";
    };
    CoroOnThread coroutine(std::bind(cl, &coroutine), "coroutine");

    std::unique_ptr<Payload> from_coro;

    EXPECT_FALSE(coroutine.IsCoroutineExited());
    coroutine.Cancel();
    EXPECT_TRUE(coroutine.IsCoroutineExited());

    EXPECT_EQ(DestructorCheck::undestructed_count, 0);
}
