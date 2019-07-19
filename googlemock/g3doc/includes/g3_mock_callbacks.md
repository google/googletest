#### Mock Callbacks

Callbacks (`"base/callback.h"`) are widely used in `google3` to package data and
logic together. Sometimes you want to test how your code invokes callbacks (with
what arguments, how many times, in which order, and etc). This is a job cut out
for mock callbacks.

`"testing/base/public/mock-callback.h"` defines a class template to mock
callbacks. Given arbitrary types `R`, `T1`, ..., and `Tn`, class
**`MockCallback<R(T1, ..., Tn)>`** mocks a callback that takes arguments of type
`T1`, ..., and `Tn`, and returns type `R`, which can be `void`. This class is
derived from its corresponding abstract callback classed defined in
`"base/callback.h"`, for example:

*   `MockCallback<void()>` inherits from `Closure`,
*   `MockCallback<void(int, double)>` inherits from `Callback2<int, double>`,
*   `MockCallback<int()>` derives from `ResultCallback<int>`, and
*   `MockCallback<string(bool)>` derives from `ResultCallback1<string, bool>`.

Compared with the various classes in `"base/callback.h"`, the mock classes share
the same name and only differ in the template arguments, so you will never have
trouble remembering which is called what.

Like a real callback, a mock callback can be either *single-use* or *permanent*.
A single-use mock callback will delete itself when invoked. A permanent mock
callback will not and thus can be invoked many times - you have to make sure it
is deleted somehow.

Since a mock object verifies all expectations on its mock methods in the
destructor, please link with `//base:heapcheck` (it is already linked
automatically if you link with `//testing/base/public:gunit_main`) to make sure
all mock callbacks

are properly deleted.

`MockCallback<R(T1, ..., Tn)>` has a mock method `OnRun()` with the signature:

```cpp
  R OnRun(T1, ..., Tn);
```

`OnRun()` will be called whenever the mock callback is invoked. Note that we
don't name it `Run()` to match the method in the base class, as doing so will
interfere with mocking single-use callbacks.

Finally, `"mock-callback.h"` is a header-only library, so just include it and
go. Here's a complete example on how you use it:

```cpp
#include "testing/base/public/mock-callback.h"

// 1. Import the necessary names from the testing name space.
using ::testing::_;
using ::testing::MockCallback;
using ::testing::NotNull;
using ::testing::NewMockCallback;
using ::testing::NewPermanentMockCallback;
using ::testing::SetArgPointee;

TEST(FooTest, DoesBar) {
  // 2. Create a single-use mock callback using NewMockCallback(), or
  //    a permanent mock callback using NewPermanentMockCallback().
  MockCallback<string(int n, bool show_sign)>* show_int = NewMockCallback();
  std::unique_ptr<MockCallback<void(int* count)> > get_count(
        NewPermanentMockCallback());

  // 3. Set expectations on the OnRun() method of the mock callbacks.
  EXPECT_CALL(*show_int, OnRun(5, true))
      .WillOnce(Return("+5"));
  EXPECT_CALL(*get_count, OnRun(NotNull()))
      .WillOnce(SetArgPointee<0>(1))
      .WillOnce(SetArgPointee<0>(2));

  // 4. Exercise code that uses the mock callbacks.  The single-use
  //     mock callback show_int will be verified and deleted when it's
  //     called.  Link with //base:heapcheck to make sure it is not
  //     leaked.
  Foo(5, show_int, get_count.get());
  // Foo()'s signature:
  //   void Foo(int n, ResultCallback2<string, int, bool>* show_int,
  //            Callback1<int*>* get_count);

  // 5. The permanent mock callback will be verified and deleted here,
  //    thanks to the std::unique_ptr.
}
```

Did you notice that you don't specify the types when calling `NewMockCallback()`
and `NewPermanentMockCallback()`? Apparently they can read your mind and know
the type of the mock callback you want. :-)

(Seriously, these functions figure out their return types from the
left-hand-side of the assignment or the initialization, with the help of some
template tricks. But you don't need to understand how they work in order to use
mock callbacks.)
