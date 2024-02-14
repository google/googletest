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
    virtual int Mock1() const = 0;
    virtual int Mock2() const = 0;
    virtual int Mock4() const = 0;
    virtual int Mock5(int x) const = 0;
};

class MockClass : public ClassToMock {
   public:
    MOCK_METHOD(int, Mock1, (), (const, override));
    MOCK_METHOD(int, Mock2, (), (const, override));
    MOCK_METHOD(int, Mock4, (), (const, override));
    MOCK_METHOD(int, Mock5, (int x), (const, override));
};

using ::testing::Return;

//////////////////////////////////////////////
// The actual tests

TEST(CardinalityTest, ExitAfterMethod1_OK) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE(ExitAfterMethod1) {
        auto cs = WAIT_FOR_CALL(mock_object, Mock1);
        cs.RETURN(10);
    };

    // absorb mock calls not accepted by coroutine
    EXPECT_CALL(mock_object, Mock2).WillOnce(Return(-1));
    coro.WATCH_CALL(mock_object, Mock1);
    coro.WATCH_CALL(mock_object, Mock2);

    // This is the body of the test case
    EXPECT_EQ(mock_object.Mock2(), -1);
    EXPECT_EQ(mock_object.Mock1(), 10);
}

TEST(CardinalityTest, ExitAfterMethod1_OK_Wild) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE(ExitAfterMethod1) {
        auto cs = WAIT_FOR_CALL(mock_object, Mock1);
        cs.RETURN(10);
    };

    // absorb mock calls not accepted by coroutine
    EXPECT_CALL(mock_object, Mock2).WillOnce(Return(-1));
    coro.WATCH_CALL();

    // This is the body of the test case
    EXPECT_EQ(mock_object.Mock2(), -1);
    EXPECT_EQ(mock_object.Mock1(), 10);
}

TEST(CardinalityTest, ExitAfterMethod1_Oversaturate) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE(ExitAfterMethod1) {
        auto cs = WAIT_FOR_CALL(mock_object, Mock1);
        cs.RETURN(10);
    };

    // absorb mock calls not accepted by coroutine
    EXPECT_CALL(mock_object, Mock2).WillOnce(Return(-1));
    coro.WATCH_CALL(mock_object, Mock1);
    coro.WATCH_CALL(mock_object, Mock2);
    coro.WATCH_CALL(mock_object, Mock4);

    // This is the body of the test case
    mock_object.Mock2();
    mock_object.Mock1();

    // We expect a TEST FAILURE while running the MUT, with a message about
    // oversaturation
    EXPECT_NONFATAL_FAILURE(mock_object.Mock4(), "Actual: called once - over-saturated and active");
}

TEST(CardinalityTest, ExitAfterMethod1_Oversaturate_Wild) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE(ExitAfterMethod1) {
        auto cs = WAIT_FOR_CALL(mock_object, Mock1);
        cs.RETURN(10);
    };

    // absorb mock calls not accepted by coroutine
    EXPECT_CALL(mock_object, Mock2).WillOnce(Return(-1));
    coro.WATCH_CALL(mock_object);

    // This is the body of the test case
    mock_object.Mock2();
    mock_object.Mock1();

    // We expect a TEST FAILURE while running the MUT, with a message about
    // oversaturation
    EXPECT_NONFATAL_FAILURE(mock_object.Mock4(), "Actual: called twice - over-saturated and active");
    // Note different message compared to ExitAfterMethod1_Oversaturate,
    // this is because the call count used in the message is kept by the
    // individual watchers (=expectations), not the coroutine.
}

TEST(CardinalityTest, ExitAfterMethod1_Simple) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE(ExitAfterMethod1) {
        auto cs = WAIT_FOR_CALL(mock_object, Mock1);
        cs.RETURN(10);
    };

    coro.WATCH_CALL(mock_object, Mock1);

    // This is the body of the test case
    EXPECT_EQ(mock_object.Mock1(), 10);
}

TEST(CardinalityTest, ExitAfterMethod1_Simple_Wild) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE(ExitAfterMethod1) {
        auto cs = WAIT_FOR_CALL(mock_object, Mock1);
        cs.RETURN(10);
    };

    coro.WATCH_CALL();

    // This is the body of the test case
    EXPECT_EQ(mock_object.Mock1(), 10);
}

// Note: this one does not use coroutines at all and only
// serves as an example of unsatisfaction in gmock.
TEST(CardinalityTest, NoCoroutine_Unsatisfy) {
    auto mock_object_ptr = make_unique<StrictMock<MockClass>>();

    EXPECT_CALL(*mock_object_ptr, Mock1).Times(3);

    // This is the body of the test case
    mock_object_ptr->Mock1();

    // The mock object should be unsatisfied and therefore fail in its destructor
    EXPECT_NONFATAL_FAILURE(mock_object_ptr.reset(), "Actual: called once - unsatisfied and active");
}

TEST(CardinalityTest, SatisfyAfterMethod1_Unsatisfy) {
    auto mock_object_ptr = make_unique<StrictMock<MockClass>>();

    auto coro_ptr = NEW_COROUTINE(SatisfyAfterMethod1) {
        WAIT_FOR_CALL(*mock_object_ptr, Mock1).RETURN(10);
        // Mock calls after this are not required for the test to pass
        SATISFY();
        WAIT_FOR_CALL(*mock_object_ptr, Mock1).RETURN(20);
    };

    // absorb mock calls
    EXPECT_CALL(*mock_object_ptr, Mock2).WillOnce(Return(-1));

    coro_ptr->WATCH_CALL(*mock_object_ptr, Mock1);

    // This is the body of the test case
    mock_object_ptr->Mock2();

    delete coro_ptr;  // Coro may not have exited so we must delete it before the
                      // mock object

    // The mock object should be unsatisfied and therefore fail in its destructor
    EXPECT_NONFATAL_FAILURE(mock_object_ptr.reset(), "Actual: never called - unsatisfied and active");
}

TEST(CardinalityTest, SatisfyAfterMethod1_Unsatisfy_Wild) {
    auto mock_object_ptr = make_unique<StrictMock<MockClass>>();

    auto coro_ptr = NEW_COROUTINE(SatisfyAfterMethod1) {
        WAIT_FOR_CALL(*mock_object_ptr, Mock1).RETURN(10);
        // Mock calls after this are not required for the test to pass
        SATISFY();
        WAIT_FOR_CALL(*mock_object_ptr, Mock1).RETURN(20);
    };

    // absorb mock calls
    EXPECT_CALL(*mock_object_ptr, Mock2).WillOnce(Return(-1));

    coro_ptr->WATCH_CALL(*mock_object_ptr);

    // This is the body of the test case
    mock_object_ptr->Mock2();

    // The mock object should be unsatisfied and therefore fail in its destructor
    EXPECT_NONFATAL_FAILURE(delete coro_ptr, "Actual: never called - unsatisfied and active");
    mock_object_ptr.reset();
}

TEST(CardinalityTest, SatisfyImmediate_OK) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE(SatisfyImmediate) {
        // Mock calls after this are not required for the test to pass
        SATISFY();
        auto cs = WAIT_FOR_CALL(mock_object, Mock1);
        cs.RETURN(10);
    };

    // absorb mock calls
    EXPECT_CALL(mock_object, Mock2).WillOnce(Return(-1));

    coro.WATCH_CALL(mock_object, Mock1);

    // This is the body of the test case
    mock_object.Mock2();
}

TEST(CardinalityTest, SatisfyImmediate_OK_Wild) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE(SatisfyImmediate) {
        // Mock calls after this are not required for the test to pass
        SATISFY();
        auto cs = WAIT_FOR_CALL(mock_object, Mock1);
        cs.RETURN(10);
    };

    // absorb mock calls
    EXPECT_CALL(mock_object, Mock2).WillOnce(Return(-1));

    coro.WATCH_CALL();

    // This is the body of the test case
    mock_object.Mock2();
}

TEST(CardinalityTest, SatisfyAfterMethod1_OK) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE(SatisfyAfterMethod1) {
        WAIT_FOR_CALL(mock_object, Mock1).RETURN(10);
        // Mock calls after this are not required for the test to pass
        SATISFY();
        WAIT_FOR_CALL(mock_object, Mock1).RETURN(20);
    };

    coro.WATCH_CALL(mock_object, Mock1);

    // This is the body of the test case
    mock_object.Mock1();
}

TEST(CardinalityTest, SatisfyAfterMethod1_OK_Wild) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE(SatisfyAfterMethod1) {
        WAIT_FOR_CALL(mock_object, Mock1).RETURN(10);
        // Mock calls after this are not required for the test to pass
        SATISFY();
        WAIT_FOR_CALL(mock_object, Mock1).RETURN(20);
    };

    coro.WATCH_CALL(mock_object);

    // This is the body of the test case
    mock_object.Mock1();
}

TEST(CardinalityTest, RetireAfterMethod1_OK) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE(RetireAfterMethod1) {
        WAIT_FOR_CALL(mock_object, Mock1).RETURN(10);
        // Retire will mean the mock will drop all further calls
        RETIRE();
    };

    EXPECT_CALL(mock_object, Mock4).WillOnce(Return(33));
    EXPECT_CALL(mock_object, Mock1).WillOnce(Return(-1));
    coro.WATCH_CALL(mock_object, Mock1);

    // This is the body of the test case
    EXPECT_EQ(mock_object.Mock1(), 10);
    EXPECT_EQ(mock_object.Mock1(), -1);
    EXPECT_EQ(mock_object.Mock4(), 33);
}

TEST(CardinalityTest, RetireAfterMethod1_OK_Wild) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE(RetireAfterMethod1) {
        WAIT_FOR_CALL(mock_object, Mock1).RETURN(10);
        // Retire will mean the mock will drop all further calls
        RETIRE();
    };

    EXPECT_CALL(mock_object, Mock4).WillOnce(Return(33));
    EXPECT_CALL(mock_object, Mock1).WillOnce(Return(-1));
    coro.WATCH_CALL();

    // This is the body of the test case
    EXPECT_EQ(mock_object.Mock1(), 10);
    EXPECT_EQ(mock_object.Mock1(), -1);
    EXPECT_EQ(mock_object.Mock4(), 33);
}

TEST(CardinalityTest, RetireAfterMethod1_Unsatisfy) {
    auto mock_object_ptr = make_unique<StrictMock<MockClass>>();

    auto coro_ptr = NEW_COROUTINE(RetireAfterMethod1) {
        WAIT_FOR_CALL(*mock_object_ptr, Mock1).RETURN(10);
        // Retire will mean the mock will drop all further calls
        RETIRE();
    };

    EXPECT_CALL(*mock_object_ptr, Mock4).WillOnce(Return(33));
    coro_ptr->WATCH_CALL(*mock_object_ptr, Mock1);

    // This is the body of the test case
    EXPECT_EQ(mock_object_ptr->Mock4(), 33);

    delete coro_ptr;  // Coro may not have exited so we must delete it before the
                      // mock object

    // The mock object should be unsatisfied and therefore fail in its destructor
    EXPECT_NONFATAL_FAILURE(mock_object_ptr.reset(), "Actual: never called - unsatisfied and active");
}

TEST(CardinalityTest, RetireAfterMethod1_Unsatisfy_Wild) {
    auto mock_object_ptr = make_unique<StrictMock<MockClass>>();

    auto coro_ptr = NEW_COROUTINE(RetireAfterMethod1) {
        WAIT_FOR_CALL(*mock_object_ptr, Mock1).RETURN(10);
        // Retire will mean the mock will drop all further calls
        RETIRE();
    };

    EXPECT_CALL(*mock_object_ptr, Mock4).WillOnce(Return(33));
    coro_ptr->WATCH_CALL();

    // This is the body of the test case
    EXPECT_EQ(mock_object_ptr->Mock4(), 33);

    // The mock object should be unsatisfied and therefore fail in its destructor
    EXPECT_NONFATAL_FAILURE(delete coro_ptr;, "Actual: never called - unsatisfied and active");
    mock_object_ptr.reset();
}

TEST(CardinalityTest, NoCoroutine_Unexpected) {
    StrictMock<MockClass> mock_object;

    EXPECT_CALL(mock_object, Mock5(99));

    // This is the body of the test case
    mock_object.Mock5(99);
    EXPECT_NONFATAL_FAILURE(mock_object.Mock5(55), "Unexpected mock function call - returning default value.");
}

TEST(CardinalityTest, Coroutine_Unexpected) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE(Unexpected) {
        SATISFY();
        while (1) {
            auto e = NEXT_EVENT();
            if (auto e2 = e.IS_CALL(mock_object, Mock5(99)))
                e2.RETURN(0);
            else
                e.DROP();
        }
    };
    coro.WATCH_CALL();

    // This is the body of the test case
    mock_object.Mock5(99);
    EXPECT_NONFATAL_FAILURE(mock_object.Mock5(55), "Unexpected mock function call - returning default value.");
}

TEST(CardinalityTest, ExitAfterMethod1_OversaturateOnSatisfy) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE(ExitAfterMethod1) {
        auto cs = WAIT_FOR_CALL(mock_object, Mock1);
        cs.RETURN(10);
        SATISFY();
    };

    // absorb mock calls not accepted by coroutine
    EXPECT_CALL(mock_object, Mock2).WillOnce(Return(-1));
    coro.WATCH_CALL(mock_object, Mock1);
    coro.WATCH_CALL(mock_object, Mock2);
    coro.WATCH_CALL(mock_object, Mock4);

    // This is the body of the test case
    mock_object.Mock2();
    mock_object.Mock1();

    // We expect a TEST FAILURE while running the MUT, with a message about
    // oversaturation even though the interior indicated satisfaction.
    EXPECT_NONFATAL_FAILURE(mock_object.Mock4(), "Actual: called once - over-saturated and active");
}

TEST(CardinalityTest, UnsatisfyRetired1) {
    auto mock_object_ptr = make_unique<StrictMock<MockClass>>();

    auto coro_ptr = NEW_COROUTINE(SatisfyAfterMethod1) {
        RETIRE();
        WAIT_FOR_CALL(*mock_object_ptr, Mock1).RETURN(10);
    };

    // absorb mock calls
    EXPECT_CALL(*mock_object_ptr, Mock2).WillOnce(Return(-1));

    coro_ptr->WATCH_CALL(*mock_object_ptr, Mock1);

    // This is the body of the test case
    mock_object_ptr->Mock2();

    delete coro_ptr;  // Coro may not have exited so we must delete it before the
                      // mock object

    // The mock object should be unsatisfied and therefore fail in its destructor
    EXPECT_NONFATAL_FAILURE(mock_object_ptr.reset(), "Actual: never called - unsatisfied and retired");
}

TEST(CardinalityTest, UnsatisfyRetired2) {
    auto mock_object_ptr = make_unique<StrictMock<MockClass>>();

    auto coro_ptr = NEW_COROUTINE(SatisfyAfterMethod1) {
        WAIT_FOR_CALL(*mock_object_ptr, Mock1).RETURN(10);
        RETIRE();
        WAIT_FOR_CALL(*mock_object_ptr, Mock1).RETURN(20);  // Ensure coro is unsatisfied
    };

    coro_ptr->WATCH_CALL(*mock_object_ptr, Mock1);

    // This is the body of the test case
    mock_object_ptr->Mock1();

    delete coro_ptr;  // Coro may not have exited so we must delete it before the
                      // mock object

    // The mock object should be unsatisfied and therefore fail in its destructor
    EXPECT_NONFATAL_FAILURE(mock_object_ptr.reset(), "Actual: called once - unsatisfied and retired");
}

TEST(CardinalityTest, UnsatisfyRetired3) {
    auto mock_object_ptr = make_unique<StrictMock<MockClass>>();

    auto coro_ptr = NEW_COROUTINE(SatisfyAfterMethod1) {
        WAIT_FOR_CALL(*mock_object_ptr, Mock1).RETURN(10);
        RETIRE();
        WAIT_FOR_CALL(*mock_object_ptr, Mock1).RETURN(10);  // Ensure RETIRE() runs (see UnsatisfyRetired2)
        WAIT_FOR_CALL(*mock_object_ptr, Mock1).RETURN(20);  // Ensure coro is unsatisfied
    };

    coro_ptr->WATCH_CALL(*mock_object_ptr, Mock1);

    // This is the body of the test case
    mock_object_ptr->Mock1();
    EXPECT_NONFATAL_FAILURE(mock_object_ptr->Mock1(), "Actual: it is retired");

    delete coro_ptr;  // Coro may not have exited so we must delete it before the
                      // mock object

    // The mock object should be unsatisfied and therefore fail in its destructor
    EXPECT_NONFATAL_FAILURE(mock_object_ptr.reset(), "Actual: called once - unsatisfied and retired");
}

TEST(CardinalityTest, SatisfyDestruct) {
    auto mock_object_ptr = make_unique<StrictMock<MockClass>>();

    auto coro_ptr = NEW_COROUTINE(SatisfyAfterMethod1) {
        WAIT_FOR_CALL(*mock_object_ptr, Mock1).RETURN(10);
        SATISFY();
        WAIT_FOR_CALL(*mock_object_ptr, Mock1).RETURN(20);  // Ensure coro is unsatisfied
    };

    coro_ptr->WATCH_CALL(*mock_object_ptr, Mock1);

    // This is the body of the test case
    EXPECT_EQ(mock_object_ptr->Mock1(), 10);

    delete coro_ptr;  // Coro may not have exited so we must delete it before the
                      // mock object

    // The mock object should be unsatisfied and therefore fail in its destructor
    mock_object_ptr.reset();
}

TEST(CardinalityTest, SatisfyDestruct2) {
    auto mock_object_ptr = make_unique<StrictMock<MockClass>>();

    auto coro_ptr = NEW_COROUTINE(SatisfyAfterMethod1) {
        WAIT_FOR_CALL(*mock_object_ptr, Mock1).RETURN(10);
        SATISFY();
    };

    coro_ptr->WATCH_CALL(*mock_object_ptr, Mock1);

    // This is the body of the test case
    EXPECT_EQ(mock_object_ptr->Mock1(), 10);

    delete coro_ptr;  // Coro may not have exited so we must delete it before the
                      // mock object

    // The mock object should be unsatisfied and therefore fail in its destructor
    mock_object_ptr.reset();
}
