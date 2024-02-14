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
    virtual int MockA(int x) = 0;
    virtual int MockB(int x) = 0;
    virtual int MockC(int x) = 0;
    virtual int MockD(int x) = 0;
};

class ExampleClass {
   public:
    ExampleClass(ClassToMock *dep_) : dep(dep_) {}

    int A(int x) { return dep->MockA(x); }

    int B(int x) { return dep->MockB(x); }

    int C(int x) { return dep->MockC(x); }

    int D(int x) { return dep->MockD(x); }

   private:
    ClassToMock *const dep;
};

////////////////////////////////////////////
// Mocking assets

class MockClass : public ClassToMock {
   public:
    MOCK_METHOD(int, MockA, (int x), (override));
    MOCK_METHOD(int, MockB, (int x), (override));
    MOCK_METHOD(int, MockC, (int x), (override));
    MOCK_METHOD(int, MockD, (int x), (override));
};

//////////////////////////////////////////////
// The actual tests

/*
 * In this example, we demonstate interleaved coroutines that share
 * mock calls made by launch sessions.
 */

TEST(LaunchMultiCoroTest, DualInSeqDrop) {
    StrictMock<MockClass> mock_object;
    ExampleClass example(&mock_object);

    // Notice that in these tests, the observing coroutine (that
    // waits for mock calls) and the instigator (that launches calls)
    // comes last. This ensures that the observer is not destructed
    // before the instigator completes.
    auto observer_coro = COROUTINE(Observer) {
        WATCH_CALL(mock_object, MockA);  // lower prio

        // We see this because instigator_coro drops it
        auto e = NEXT_EVENT();
        EXPECT_TRUE(e.IS_CALL(mock_object, MockA(1)).RETURN(9));

        // We see this because higher prio
        e = NEXT_EVENT();
        EXPECT_TRUE(e.IS_CALL(mock_object, MockB(2)));
        e.DROP();
    };

    auto instigator_coro = COROUTINE(Instigator) {
        WATCH_CALL();
        observer_coro.WATCH_CALL(mock_object, MockB);  // higher prio

        auto da = LAUNCH(example.A(1));

        auto ea = NEXT_EVENT();
        EXPECT_TRUE(ea.IS_CALL(mock_object, MockA(1)).From(da));
        ea.DROP();  // required to avoid deadlocking on GMock's mutex

        auto e = NEXT_EVENT();
        EXPECT_TRUE(e.IS_RESULT(da));
        EXPECT_EQ(e(da), 9);

        auto db = LAUNCH(example.B(2));

        auto eb = NEXT_EVENT();
        EXPECT_TRUE(eb.IS_CALL(mock_object, MockB(2)).From(db).RETURN(2000));

        e = NEXT_EVENT();
        EXPECT_TRUE(e.IS_RESULT(db));
        EXPECT_EQ(e(db), 2000);
    };
}

/*
 * Here we attempt to reverse the two mock calls using delayed return
 * but we can only delay RETURN(), not DROP(), so we can't demonstrate
 * a sheared call sequence as seen by watcher.
 */
TEST(LaunchMultiCoroTest, DualInSeqAccept) {
    StrictMock<MockClass> mock_object;
    ExampleClass example(&mock_object);

    auto observer_coro = COROUTINE(Observer) {
        // We don't get this because instigator_coro cannot decide to drop
        // MockA based on argument passed to MockB
        // auto ea = NEXT_EVENT();
        // EXPECT_TRUE( ea.IS_CALL(mock_object, MockA(1)) );
        // ea.RETURN(9);

        // We see this because higher prio
        auto e = NEXT_EVENT();
        EXPECT_TRUE(e.IS_CALL(mock_object, MockB(2)));
        e.DROP();
    };

    auto instigator_coro = COROUTINE(Instigator) {
        observer_coro.WATCH_CALL(mock_object, MockA);  // lower prio
        WATCH_CALL();
        observer_coro.WATCH_CALL(mock_object, MockB);  // higher prio

        auto da = LAUNCH(example.A(1));

        auto ea = NEXT_EVENT().IS_CALL(mock_object, MockA(1));
        EXPECT_TRUE(ea.From(da));
        ea.ACCEPT();  // required to avoid deadlocking on GMock's mutex

        auto db = LAUNCH(example.B(2));

        auto eb = NEXT_EVENT().IS_CALL(mock_object, MockB(2));
        EXPECT_TRUE(eb.From(db));
        int x = eb.GetArg<0>();

        eb.RETURN(2000);
        auto e = NEXT_EVENT();
        EXPECT_TRUE(e.IS_RESULT(db));
        EXPECT_EQ(e(db), 2000);

        ea.RETURN(x);
        e = NEXT_EVENT();
        EXPECT_TRUE(e.IS_RESULT(da));
        EXPECT_EQ(e(da), 2);
    };
}

class ExampleClassAlternative {
   public:
    ExampleClassAlternative(ClassToMock *dep_) : dep(dep_) {}

    int A(int x) { return dep->MockA(x); }

    int B(int x) {
        int y = dep->MockB(x);
        y += dep->MockC(x);
        return y;
    }

    int C(int x) { return dep->MockC(x); }

    int D(int x) { return dep->MockD(x); }

   private:
    ClassToMock *const dep;
};

TEST(LaunchMultiCoroTest, FollowOn) {
    StrictMock<MockClass> mock_object;
    ExampleClassAlternative example(&mock_object);

    auto observer_coro = COROUTINE(Observer) {
        WATCH_CALL(mock_object, MockB);  // lower prio
        // We see this because higher prio
        auto e = NEXT_EVENT();
        auto e2 = e.IS_CALL(mock_object, MockB(2));
        EXPECT_TRUE(e2.RETURN(1000));
    };

    auto instigator_coro = COROUTINE(Instigator) {
        WATCH_CALL();

        auto db = LAUNCH(example.B(2));

        auto eb = NEXT_EVENT();
        EXPECT_TRUE(eb.IS_CALL(mock_object, MockB(2)).From(db));
        eb.DROP();

        eb = NEXT_EVENT();
        EXPECT_TRUE(eb.IS_CALL(mock_object, MockC(2)).From(db).RETURN(20));

        auto e = NEXT_EVENT();
        EXPECT_TRUE(e.IS_RESULT(db));
        EXPECT_EQ(e(db), 1020);
    };
}

TEST(LaunchMultiCoroTest, Arbitrary) {
    StrictMock<MockClass> mock_object;
    ExampleClass example(&mock_object);

    StrictMock<MockClass> mock_object2;
    ExampleClass example2(&mock_object2);

    ::testing::LaunchHandle<int> *pdb = nullptr;

    auto observer_coro = COROUTINE(Observer) {
        WATCH_CALL(mock_object, MockC);
        WATCH_CALL(mock_object2, MockA);
        WATCH_CALL(mock_object, MockB);

        auto ea = WAIT_FOR_CALL(mock_object2, MockA);  // due to Instigator's da
        ea.RETURN(567);

        auto eb = NEXT_EVENT().IS_CALL(mock_object, MockB(2));  // due to Instigator's db
        EXPECT_TRUE(eb.From(*pdb));
        eb.ACCEPT();

        auto dc = LAUNCH(example.C(7));
        auto e = NEXT_EVENT();
        auto cc = e.IS_CALL(mock_object, MockC);
        e.ACCEPT();

        eb.RETURN(123);
        // No need for NEXT_EVENT since we're returning

        cc.RETURN(99);

        e = NEXT_EVENT();
        EXPECT_TRUE(e.IS_RESULT(dc));
    };

    auto instigator_coro = COROUTINE(Instigator) {
        auto da = LAUNCH(example2.A(0));  // Observer will handle MockA()

        auto e = NEXT_EVENT();
        EXPECT_TRUE(e.IS_RESULT(da));
        EXPECT_EQ(e(da), 567);

        auto db = LAUNCH(example.B(2));  // Observer will handle MockB()
        pdb = &db;

        e = NEXT_EVENT();
        EXPECT_TRUE(e.IS_RESULT(db));
        EXPECT_EQ(e(db), 123);
    };
}

TEST(LaunchMultiCoroTest, ArbitraryEx) {
    StrictMock<MockClass> mock_object;
    ExampleClassAlternative example(&mock_object);

    StrictMock<MockClass> mock_object2;
    ExampleClass example2(&mock_object2);

    ::testing::LaunchHandle<int> *pdb = nullptr;

    auto observer_coro = COROUTINE(Observer) {
        auto ea = WAIT_FOR_CALL(mock_object2, MockA);  // due to Instigator's da
        ea.RETURN(567);

        auto eb = NEXT_EVENT().IS_CALL(mock_object, MockB(2));  // due to Instigator's db
        EXPECT_TRUE(eb.From(*pdb));
        eb.ACCEPT();

        auto dc = LAUNCH(example.C(7));
        auto e = NEXT_EVENT();
        auto cc = e.IS_CALL(mock_object, MockC).From(dc);
        e.ACCEPT();

        eb.RETURN(123);
        auto e3 = NEXT_EVENT();
        EXPECT_TRUE(e3.From(*pdb).ACCEPT());

        cc.RETURN(99);
        e = NEXT_EVENT();
        EXPECT_TRUE(e.IS_RESULT(dc));

        e3.IS_CALL(mock_object, MockC).RETURN(23);
    };

    observer_coro.WATCH_CALL(mock_object, MockC);
    observer_coro.WATCH_CALL(mock_object2, MockA);
    observer_coro.WATCH_CALL(mock_object, MockB);

    auto instigator_coro = COROUTINE(Instigator) {
        auto da = LAUNCH(example2.A(0));  // Observer will handle MockA()

        auto e = NEXT_EVENT();
        EXPECT_TRUE(e.IS_RESULT(da));
        EXPECT_EQ(e(da), 567);

        auto db = LAUNCH(example.B(2));  // Observer will handle MockB()
        pdb = &db;

        e = NEXT_EVENT();
        EXPECT_TRUE(e.IS_RESULT(db));
        EXPECT_EQ(e(db), 146);
    };
}

TEST(LaunchMultiCoroTest, ObserverQueue) {
    StrictMock<MockClass> mock_object;
    ExampleClass example(&mock_object);

    EXPECT_CALL(mock_object, MockB).WillOnce(Return(102));

    auto observer_coro = COROUTINE(Observer) {
        WATCH_CALL(mock_object);

        WAIT_FOR_CALL(mock_object, MockA(0)).RETURN(10);
        auto ea1 = WAIT_FOR_CALL(mock_object, MockA(1));
        auto dc = LAUNCH(example.C(3));
        auto ec = WAIT_FOR_CALL(mock_object, MockC(3));

        ea1.RETURN(11);

        // Note: this is a rare case in which we require more than one
        // consecutive mock return. The above return permits MockA() to
        // return, launch session da2 to return and coro Instigator to exit.
        // Since there are no launch sessions in main that might cause mock
        // calls, we reach the RAII destructors for the test coroutines.
        // These provide an extra iteration to the coro if it has not yet
        // exited. A NEXT_EVENT() here would now run, but would not find
        // any events waiting. It would request resumption of main hoping
        // for more mock calls but destructors would complete leaving
        // Observer unsatisfied (didn't exit or SATISFY()) and dc and ec
        // uncompleted.
        // auto e = NEXT_EVENT();

        ec.RETURN(13);

        EXPECT_EQ(WAIT_FOR_RESULT()(dc), 13);
    };

    auto instigator_coro = COROUTINE(Instigator) {
        auto da = LAUNCH(example.A(0));
        EXPECT_EQ(NEXT_EVENT().IS_RESULT()(da), 10);
        auto db = LAUNCH(example.B(2));
        EXPECT_EQ(NEXT_EVENT().IS_RESULT()(db), 102);
        auto da2 = LAUNCH(example.A(1));

        EXPECT_EQ(NEXT_EVENT().IS_RESULT()(da2), 11);
    };
}
