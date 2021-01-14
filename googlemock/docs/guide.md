# googletest gMock Users Guide

go/gmockguide

Welcome to googletest: Google's C++ testing and mocking framework. gMock is a
mocking part of googletest.

*   [OSS Version](https://github.com/google/googletest)
*   [Google3](http://google3/third_party/googletest/)

*   If you are new to gMock, start with [*gMock for Dummies*](for_dummies.md) to
    learn the basic usage.

*   Read [gMock Cookbook](cook_book.md) to learn more advanced usage and useful
    tips.

*   For a quick reference, check out [gMock Cheat Sheet](cheat_sheet.md).

*   If you have questions, search [gMock FAQ](#GMockFaq) and the gmock-users@
    archive before sending them to gmock-users@.

<!-- GOOGLETEST_CM0035 DO NOT DELETE -->

<!--#include file="for_dummies.md"-->

#### Side Effects

<!-- mdformat off(github rendering does not support multiline tables) -->
| Matcher                            | Description                             |
| :--------------------------------- | :-------------------------------------- |
| `Assign(&variable, value)` | Assign `value` to variable. |
| `DeleteArg<N>()` | Delete the `N`-th (0-based) argument, which must be a pointer. |
| `SaveArg<N>(pointer)` | Save the `N`-th (0-based) argument to `*pointer`. |
| `SaveArgPointee<N>(pointer)` | Save the value pointed to by the `N`-th (0-based) argument to `*pointer`. |
| `SetArgReferee<N>(value)` | Assign `value` to the variable referenced by the `N`-th (0-based) argument. |
| `SetArgPointee<N>(value)` | Assign `value` to the variable pointed by the `N`-th (0-based) argument. |
| `SetArgumentPointee<N>(value)` | Same as `SetArgPointee<N>(value)`. Deprecated. Will be removed in v1.7.0. |
| `SetArrayArgument<N>(first, last)` | Copies the elements in source range [`first`, `last`) to the array pointed to by the `N`-th (0-based) argument, which can be either a pointer or an iterator. The action does not take ownership of the elements in the source range. |
| `SetErrnoAndReturn(error, value)` | Set `errno` to `error` and return `value`. |
| `Throw(exception)` | Throws the given exception, which can be any copyable value. Available since v1.1.0. |
<!-- mdformat on -->

*   When compiling with exceptions in google3, it's not enough to specify
    `-fexceptions` to copts in your cc_test target. That flag will not be
    inherited by gmock, and various headers will be compiled both with and
    without `-fexceptions` causing subtle bugs. Instead you must pass
    `--copt=-fexceptions` to the blaze command so the flag gets passed to all
    targets... but this is Google and we don't use exceptions so it shouldn't
    really be an issue.
