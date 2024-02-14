#include <string>

#include "cotest/cotest.h"
#include "gtest/gtest-spi.h"

using namespace std;
using namespace testing;
using coro_impl::PtrToString;
using ::testing::StrictMock;

//////////////////////////////////////////////
// Mocking assets

struct MyStruct {
    int i;
    char c;
};

class ClassToMock {
   public:
    virtual ~ClassToMock() {}
    virtual int MockMethod1() const = 0;
    virtual void MockMethod2() const = 0;
    virtual int &MockMethod3() const = 0;
    virtual int *MockMethod4() const = 0;
    virtual unique_ptr<int> MockMethod5() const = 0;
    virtual shared_ptr<int> MockMethod6() const = 0;
    virtual MyStruct MockMethod7() const = 0;

    virtual void MockMethod11(int a) const = 0;
    virtual void MockMethod12(int &a) const = 0;
    virtual void MockMethod13(int *a) const = 0;
    virtual void MockMethod14(unique_ptr<int> a) const = 0;
    virtual void MockMethod15(shared_ptr<int> a) const = 0;
    virtual void MockMethod16(MyStruct a) const = 0;

    virtual unique_ptr<int> MockMethod20(unique_ptr<int> a) const = 0;
};

class MockClass : public ClassToMock {
   public:
    MOCK_METHOD(int, MockMethod1, (), (const, override));
    MOCK_METHOD(void, MockMethod2, (), (const, override));
    MOCK_METHOD(int &, MockMethod3, (), (const, override));
    MOCK_METHOD(int *, MockMethod4, (), (const, override));
    MOCK_METHOD(unique_ptr<int>, MockMethod5, (), (const, override));
    MOCK_METHOD(shared_ptr<int>, MockMethod6, (), (const, override));
    MOCK_METHOD(MyStruct, MockMethod7, (), (const, override));

    MOCK_METHOD(void, MockMethod11, (int a), (const, override));
    MOCK_METHOD(void, MockMethod12, (int &a), (const, override));
    MOCK_METHOD(void, MockMethod13, (int *a), (const, override));
    MOCK_METHOD(void, MockMethod14, (unique_ptr<int> a), (const, override));
    MOCK_METHOD(void, MockMethod15, (shared_ptr<int> a), (const, override));
    MOCK_METHOD(void, MockMethod16, (MyStruct a), (const, override));

    MOCK_METHOD(unique_ptr<int>, MockMethod20, (unique_ptr<int> a), (const, override));
};

//////////////////////////////////////////////
// The actual tests

TEST(TypesTest, IntReturn) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE() { WAIT_FOR_CALL(mock_object, MockMethod1).RETURN(10); };

    coro.WATCH_CALL(mock_object, MockMethod1);

    EXPECT_EQ(mock_object.MockMethod1(), 10);
}

TEST(TypesTest, IntReturnWild) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE() { WAIT_FOR_CALL(mock_object, MockMethod1).RETURN(10); };

    coro.WATCH_CALL();

    EXPECT_EQ(mock_object.MockMethod1(), 10);
}

TEST(TypesTest, VoidReturn) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE() { WAIT_FOR_CALL().RETURN(); };

    coro.WATCH_CALL(mock_object, MockMethod2);

    mock_object.MockMethod2();
}

TEST(TypesTest, VoidReturnWild) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE() { WAIT_FOR_CALL().RETURN(); };

    coro.WATCH_CALL();

    mock_object.MockMethod2();
}

TEST(TypesTest, VoidReturnWildSignature) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE() { WAIT_FOR_CALL(mock_object, MockMethod2).RETURN(); };

    coro.WATCH_CALL();

    mock_object.MockMethod2();
}

TEST(TypesTest, IntRefReturnSig) {
    StrictMock<MockClass> mock_object;
    int i = 10;

    auto coro = COROUTINE() {
        std::clog << "address i=" << PtrToString(&i) << std::endl;
        WAIT_FOR_CALL(mock_object, MockMethod3).RETURN(i);
    };

    coro.WATCH_CALL(mock_object, MockMethod3);
    int &ri = mock_object.MockMethod3();
    std::clog << "address ri=" << PtrToString(&ri) << std::endl;

    EXPECT_EQ(ri, 10) << "returned ref has the right value";
    EXPECT_EQ(++ri, 11) << "returned ref increments successfully";
    EXPECT_EQ(i, 11) << "alias effect shows we didn't make a copy";
}

TEST(TypesTest, IntRefReturnGen) {
    StrictMock<MockClass> mock_object;
    int i = 10;

    auto coro = COROUTINE() {
        std::clog << "address i=" << PtrToString(&i) << std::endl;
        WAIT_FOR_CALL(mock_object, MockMethod3).RETURN(i);
    };

    coro.WATCH_CALL();
    int &ri = mock_object.MockMethod3();
    std::clog << "address ri=" << PtrToString(&ri) << std::endl;

    EXPECT_EQ(ri, 10) << "returned ref has the right value";
    EXPECT_EQ(++ri, 11) << "returned ref increments successfully";
    EXPECT_EQ(i, 11) << "alias effect shows we didn't make a copy";
}

TEST(TypesTest, IntPtrReturn) {
    StrictMock<MockClass> mock_object;
    int i = 10;

    auto coro = COROUTINE() { WAIT_FOR_CALL(mock_object, MockMethod4).RETURN(&i); };

    coro.WATCH_CALL(mock_object, MockMethod4);

    EXPECT_EQ(mock_object.MockMethod4(), &i);
}

TEST(TypesTest, IntPtrReturnWild) {
    StrictMock<MockClass> mock_object;
    int i = 10;

    auto coro = COROUTINE() { WAIT_FOR_CALL(mock_object, MockMethod4).RETURN(&i); };

    coro.WATCH_CALL();

    EXPECT_EQ(mock_object.MockMethod4(), &i);
}

TEST(TypesTest, IntUniquePtrReturn) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE() { WAIT_FOR_CALL(mock_object, MockMethod5).RETURN(make_unique<int>(6)); };

    coro.WATCH_CALL(mock_object, MockMethod5);

    EXPECT_EQ(*mock_object.MockMethod5(), 6);
}

TEST(TypesTest, IntUniquePtrReturnWild) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE() { WAIT_FOR_CALL(mock_object, MockMethod5).RETURN(make_unique<int>(77)); };

    coro.WATCH_CALL();

    EXPECT_EQ(*mock_object.MockMethod5(), 77);
}

TEST(TypesTest, IntUniquePtrReturnWildSignature) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE() { WAIT_FOR_CALL(mock_object, MockMethod5).RETURN(make_unique<int>(34)); };

    coro.WATCH_CALL();

    EXPECT_EQ(*mock_object.MockMethod5(), 34);
}

TEST(TypesTest, IntSharedPtrReturn) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE() { WAIT_FOR_CALL(mock_object, MockMethod6).RETURN(make_shared<int>(63)); };

    coro.WATCH_CALL(mock_object, MockMethod6);

    EXPECT_EQ(*mock_object.MockMethod6(), 63);
}

TEST(TypesTest, IntSharedPtrReturnWild) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE() { WAIT_FOR_CALL(mock_object, MockMethod6).RETURN(make_shared<int>(69)); };

    coro.WATCH_CALL();

    EXPECT_EQ(*mock_object.MockMethod6(), 69);
}

TEST(TypesTest, IntSharedPtrReturnWildSignature) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE() { WAIT_FOR_CALL(mock_object, MockMethod6).RETURN(make_shared<int>(3)); };

    coro.WATCH_CALL();

    EXPECT_EQ(*mock_object.MockMethod6(), 3);
}

TEST(TypesTest, StructReturn) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE() { WAIT_FOR_CALL(mock_object, MockMethod7).RETURN(MyStruct{34, 'b'}); };

    coro.WATCH_CALL(mock_object, MockMethod7);

    auto s = mock_object.MockMethod7();
    EXPECT_EQ(s.i, 34);
    EXPECT_EQ(s.c, 'b');
}

TEST(TypesTest, StructReturnWild) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE() { WAIT_FOR_CALL(mock_object, MockMethod7).RETURN(MyStruct{14, 'L'}); };

    coro.WATCH_CALL();

    auto s = mock_object.MockMethod7();
    EXPECT_EQ(s.i, 14);
    EXPECT_EQ(s.c, 'L');
}

TEST(TypesTest, IntArg) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE() {
        auto cg = WAIT_FOR_CALL();
        EXPECT_EQ(cg.IS_CALL(mock_object, MockMethod11).GetArg<0>(), 22);
        cg.RETURN();
    };

    coro.WATCH_CALL();

    mock_object.MockMethod11(22);
}

TEST(TypesTest, IntRefArg) {
    StrictMock<MockClass> mock_object;
    int i = 10;

    auto coro = COROUTINE() {
        auto cg = WAIT_FOR_CALL();
        EXPECT_EQ(cg.IS_CALL(mock_object, MockMethod12).GetArg<0>(), 10);
        cg.RETURN();
    };

    coro.WATCH_CALL();

    mock_object.MockMethod12(i);
}

TEST(TypesTest, IntPtrArg) {
    StrictMock<MockClass> mock_object;
    int i = 10;

    auto coro = COROUTINE() {
        auto cg = WAIT_FOR_CALL();
        EXPECT_EQ(cg.IS_CALL(mock_object, MockMethod13).GetArg<0>(), &i);
        cg.RETURN();
    };

    coro.WATCH_CALL();

    mock_object.MockMethod13(&i);
}

TEST(TypesTest, UniquePtrArg) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE() {
        auto cg = WAIT_FOR_CALL();
        EXPECT_EQ(*(cg.IS_CALL(mock_object, MockMethod14).GetArg<0>()), 9);
        cg.RETURN();
    };

    coro.WATCH_CALL();

    mock_object.MockMethod14(make_unique<int>(9));
}

TEST(TypesTest, SharedPtrArg) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE() {
        auto cg = WAIT_FOR_CALL();
        EXPECT_EQ(*(cg.IS_CALL(mock_object, MockMethod15).GetArg<0>()), 5);
        cg.RETURN();
    };

    coro.WATCH_CALL();

    mock_object.MockMethod15(make_shared<int>(5));
}

TEST(TypesTest, StructArg) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE() {
        auto cg = WAIT_FOR_CALL();
        EXPECT_EQ(cg.IS_CALL(mock_object, MockMethod16).GetArg<0>().i, 43);
        EXPECT_EQ(cg.IS_CALL(mock_object, MockMethod16).GetArg<0>().c, '$');
        cg.RETURN();
    };

    coro.WATCH_CALL();

    mock_object.MockMethod16(MyStruct{43, '$'});
}

/*
 * TODO I think it just needs a eg MoveArg<>(), obvs should be
 * documented that it squishes the arg. */
TEST(TypesTest, UniquePtrArgAndReturn) {
    StrictMock<MockClass> mock_object;

    auto coro = COROUTINE() {
        auto cg = WAIT_FOR_CALL();
        auto cs = cg.IS_CALL(mock_object, MockMethod20);

        // Does not build
        // unique_ptr<int> u = std::move( cs.GetArg<0>() );

        // Builds and passes but is a bit of a cheat
        unique_ptr<int> u = std::make_unique<int>(*(cs.GetArg<0>()));

        ++*u;
        cs.RETURN(std::move(u));
    };

    coro.WATCH_CALL();

    EXPECT_EQ(*mock_object.MockMethod20(make_unique<int>(9)), 10);
}
