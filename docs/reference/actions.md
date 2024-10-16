# Actions Reference

[**Actions**](../gmock_for_dummies.md#actions-what-should-it-do) specify what a
mock function should do when invoked.
This page lists the built-in actions provided by GoogleTest.
To use them, add #include <gmock/gmock.h>.
All actions are defined in the `::testing` namespace.

## Returning a Value

| Action                            | Description                                   |
| :-------------------------------- | :-------------------------------------------- |
| `Return()`                        | Return from a `void` mock function.           |
| `Return(value)`                   | Return `value`. If the type of `value` is     different to the mock function's return type, `value` is converted to the latter type <i>at the time the expectation is set</i>, not when the action is executed. |
| `ReturnArg<N>()`                  | Return the `N`-th (0-based) argument.         |
| `ReturnNew<T>(a1, ..., ak)`       | Return `new T(a1, ..., ak)`; a different      object is created each time. |
| `ReturnNull()`                    | Return a null pointer.                        |
| `ReturnPointee(ptr)`              | Return the value pointed to by `ptr`.         |
| `ReturnRef(variable)`             | Return a reference to `variable`.             |
| `ReturnRefOfCopy(value)`          | Return a reference to a copy of `value`; the  copy lives as long as the action. |
| `ReturnRoundRobin({a1, ..., ak})` | Each call will return the next `ai` in the list, starting at the beginning when the end of the list is reached. |

## Side Effects

| Action                             | Description                             |
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

## Using a Function, Functor, or Lambda as an Action

In the following, by "callable" we mean a free function, `std::function`,
functor, or lambda.

| Action                              | Description                            |
| :---------------------------------- | :------------------------------------- |
| `f` | Invoke `f` with the arguments passed to the mock function, where `f` is a callable. |
| `Invoke(f)` | Invoke `f` with the arguments passed to the mock function, where `f` can be a global/static function or a functor. |
| `Invoke(object_pointer, &class::method)` | Invoke the method on the object with the arguments passed to the mock function. |
| `InvokeWithoutArgs(f)` | Invoke `f`, which can be a global/static function or a functor. `f` must take no arguments. |
| `InvokeWithoutArgs(object_pointer, &class::method)` | Invoke the method on the object, which takes no arguments. |
| `InvokeArgument<N>(arg1, arg2, ..., argk)` | Invoke the mock function's `N`-th (0-based) argument, which must be a function or a functor, with the `k` arguments. |

The return value of the invoked function is used as the return value of the
action.

When defining a callable to be used with `Invoke*()`, you can declare any unused
parameters as `Unused`:

```cpp
using ::testing::Invoke;
double Distance(Unused, double x, double y) { return sqrt(x*x + y*y); }
...
EXPECT_CALL(mock, Foo("Hi", _, _)).WillOnce(Invoke(Distance));
```

`Invoke(callback)` and `InvokeWithoutArgs(callback)` take ownership of
`callback`, which must be permanent. The type of `callback` must be a base
callback type instead of a derived one, e.g.

```cpp
  BlockingClosure* done = new BlockingClosure;
  ... Invoke(done) ...;  // This won't compile!

  Closure* done2 = new BlockingClosure;
  ... Invoke(done2) ...;  // This works.
```

In `InvokeArgument<N>(...)`, if an argument needs to be passed by reference,
wrap it inside `std::ref()`. For example,

```cpp
using ::testing::InvokeArgument;
...
InvokeArgument<2>(5, string("Hi"), std::ref(foo))
```

calls the mock function's #2 argument, passing to it `5` and `string("Hi")` by
value, and `foo` by reference.

## Default Action

| Action        | Description                                            |
| :------------ | :----------------------------------------------------- |
| `DoDefault()` | Do the default action (specified by `ON_CALL()` or the built-in one). |

{: .callout .note}
**Note:** due to technical reasons, `DoDefault()` cannot be used inside a
composite action - trying to do so will result in a run-time error.

## Composite Actions

| Action                         | Description                                 |
| :----------------------------- | :------------------------------------------ |
| `DoAll(a1, a2, ..., an)`       | Do all actions `a1` to `an` and return the result of `an` in each invocation. The first `n - 1` sub-actions must return void and will receive a  readonly view of the arguments. |
| `IgnoreResult(a)`              | Perform action `a` and ignore its result. `a` must not return void. |
| `WithArg<N>(a)`                | Pass the `N`-th (0-based) argument of the mock function to action `a` and perform it. |
| `WithArgs<N1, N2, ..., Nk>(a)` | Pass the selected (0-based) arguments of the mock function to action `a` and perform it. |
| `WithoutArgs(a)`               | Perform action `a` without any arguments. |

## Defining Actions

| Macro                              | Description                             |
| :--------------------------------- | :-------------------------------------- |
| `ACTION(Sum) { return arg0 + arg1; }` | Defines an action `Sum()` to return the sum of the mock function's argument #0 and #1. |
| `ACTION_P(Plus, n) { return arg0 + n; }` | Defines an action `Plus(n)` to return the sum of the mock function's argument #0 and `n`. |
| `ACTION_Pk(Foo, p1, ..., pk) { statements; }` | Defines a parameterized action `Foo(p1, ..., pk)` to execute the given `statements`. |

The `ACTION*` macros cannot be used inside a function or class.
