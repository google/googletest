#include <string>

#include "cotest/cotest.h"
#include "gtest/gtest-spi.h"

using namespace std;
using namespace testing;
using ::testing::StrictMock;

////////////////////////////////////////////
// Code under test

class MutexInterface {
   public:
    virtual ~MutexInterface() {}
    virtual void lock() = 0;
    virtual void unlock() = 0;
};

class ExampleClass {
   public:
    ExampleClass(MutexInterface *mutex_) : mutex(mutex_) {}

    /* The problem with this class:
     * We know that Example1() is always called before Example2(), so
     * we only need to test with that scenario. The implementation anticipates
     * the "medium" difficulty case, in which the methods overlap and
     * Example2() has started to run and then been blocked on the
     * mutex by Example1(), by placing the var_x increment
     * in Example2() at the end, apparently forcing the correct
     * sequence of events. But this is wrong, and the "hard" test case
     * discovers the problem.
     */
    int Example1(int a) {
        var_x += a;  // unsafe: left outside of mutex
        mutex->lock();
        var_y += a;
        mutex->unlock();
        return var_x - var_y;
    }

    int Example2(int a) {
        // Note: if the var_x += a; is moved to here, the medium case fails.
        mutex->lock();
        var_y += a;
        mutex->unlock();
        var_x += a;  // unsafe: left outside of mutex
        return var_x - var_y;
    }

   private:
    int var_x = 0;
    int var_y = 0;
    MutexInterface *const mutex;
};

////////////////////////////////////////////
// Mocking assets

class MockMutex : public MutexInterface {
   public:
    MOCK_METHOD(void, lock, (), (override));
    MOCK_METHOD(void, unlock, (), (override));
};

//////////////////////////////////////////////
// The actual tests

COTEST(MutexScenarioTest, Simple) {
    StrictMock<MockMutex> mock_mutex;
    ExampleClass example(&mock_mutex);
    WATCH_CALL();

    auto d = LAUNCH(example.Example1(22));
    WAIT_FOR_CALL_FROM(mock_mutex, lock, d).RETURN();
    WAIT_FOR_CALL_FROM(mock_mutex, unlock, d).RETURN();
    EXPECT_EQ(WAIT_FOR_RESULT()(d), 0);
}

// The difficulty levels of the tests can be understood as increasing
// levels of eagerness of the imaginary thread that runs Example2():
// - Easy: Doesn't even schedule until Example1() has finished
// - Medium: Preempts Example1() and then blocks on the mutex
// - Hard: Preempts Example1() and causes Example1() to block on the mutex

COTEST(MutexScenarioTest, Easy) {
    StrictMock<MockMutex> mock_mutex;
    ExampleClass example(&mock_mutex);
    WATCH_CALL();

    // Easy case, there is no conflict, Example1() returns before Example2()
    // starts

    auto l1 = LAUNCH(example.Example1(11));
    WAIT_FOR_CALL_FROM(mock_mutex, lock, l1).RETURN();
    WAIT_FOR_CALL_FROM(mock_mutex, unlock, l1).RETURN();
    EXPECT_EQ(WAIT_FOR_RESULT()(l1), 0);

    // Example1() has finished, run Example2()
    auto l2 = LAUNCH(example.Example1(22));
    WAIT_FOR_CALL_FROM(mock_mutex, lock, l2).RETURN();
    WAIT_FOR_CALL_FROM(mock_mutex, unlock, l2).RETURN();
    EXPECT_EQ(WAIT_FOR_RESULT()(l2), 0);
}

COTEST(MutexScenarioTest, Medium) {  // NOTE: MediumFixedSeq2 is better example
    StrictMock<MockMutex> mock_mutex;
    ExampleClass example(&mock_mutex);
    WATCH_CALL();

    // Medium case, Example1() and Example2() overlap, but the lock/unlock
    // sequences don't

    auto l1 = LAUNCH(example.Example1(11));
    auto l1_lock_call = WAIT_FOR_CALL_FROM(mock_mutex, lock, l1);
    auto l2 = LAUNCH(example.Example2(22));
    auto l2_lock_call = WAIT_FOR_CALL_FROM(mock_mutex, lock, l2);

    l1_lock_call.RETURN();  // Example1 gets the lock
    WAIT_FOR_CALL_FROM(mock_mutex, unlock, l1).RETURN();
    EXPECT_EQ(WAIT_FOR_RESULT()(l1), 0);

    // Example1() has unlocked while Example2() is still awaiting the mutex, which
    // we now unblock
    l2_lock_call.RETURN();
    WAIT_FOR_CALL_FROM(mock_mutex, unlock, l2).RETURN();
    EXPECT_EQ(WAIT_FOR_RESULT()(l2), 0);
}

COTEST(MutexScenarioTest, Hard) {
    StrictMock<MockMutex> mock_mutex;
    ExampleClass example(&mock_mutex);
    WATCH_CALL();

    // Hard case, Example1() starts first but Example2 gets lock first
    auto l1 = LAUNCH(example.Example1(11));
    auto l1_lock_call = WAIT_FOR_CALL_FROM(mock_mutex, lock, l1);
    auto l2 = LAUNCH(example.Example2(22));
    auto l2_lock_call = WAIT_FOR_CALL_FROM(mock_mutex, lock, l2);

    // Permit Example2 to: take the lock, unlock and return
    l2_lock_call.RETURN();  // Example2 gets the lock
    WAIT_FOR_CALL_FROM(mock_mutex, unlock, l2).RETURN();
    EXPECT_EQ(WAIT_FOR_RESULT()(l2), 11);  // Would be 0 if not for the bug

    // Example2() has finished while Example1() is still awaiting the mutex, which
    // we now unblock
    l1_lock_call.RETURN();
    WAIT_FOR_CALL_FROM(mock_mutex, unlock, l1).RETURN();
    EXPECT_EQ(WAIT_FOR_RESULT()(l1), 0);
}
