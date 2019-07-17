# A Study on Mockpp

<!--#include file="under-construction-banner.md"-->

**Author:** Zhanyong Wan (who/wan)

## What Mockpp Is

[Mockpp](http://mockpp.sourceforge.net/) is an open-source library for writing
mock classes in C++. Roughly speaking, it is ported from the
[EasyMock](http://www.easymock.org/) and [jMock](http://www.jmock.org/)
libraries for Java. This document studies the design and usability of this
library. The emphasis is on whether it would be a good fit for Google's
development environment.

## Why Mockpp Is Chosen for the Study

When I searched for "[C++ mock](http://www.google.com/search?q=c%2B%2B+mock)" on
Google, all relevant results pointed to mockpp. There may be lesser-known C++
mock tools, but if they are not on Google's radar, they are probably not worth
considering.

The fact that mockpp is the only widely-known solution can mean any of the
following:

1.  It has solved the problem so well that there is no room for other
    contenders;
2.  The C++ community hasn't recognized the importance of mock objects yet; or
3.  A good tool that facilitates mock object authoring in C++ is hard to
    develop.

From my limited experience with mockpp, I don't think mockpp has done a very
good job with the problem. I would attribute the current situation mostly to #2
and #3.

## Evaluation on Mockpp's Expressiveness

### Handling Parameters Passed by Reference

In C++, a programmer has the choice between passing an object by value and by
reference. In the former case, the object doesn't have an identify, so
constraints on its address shouldn't be allowed. In the latter, a constraint may
choose to work on the object's value or its address, or even both. For example,
given

```cpp
virtual void Foo(int x, const double& y);
```

the mock for `Foo()` should allow constraints like this:

```cpp
const double& z = ...;
foo_mocker.expects(once()).with(eq(5), and(same(z), gt(0.0)))...
// The first argument must be equal to 5; the second argument must
// refer to the same value as z refers to, and must be positive.
```

while rejecting this:

```cpp
const int& w = ...;
foo_mocker.expects(once()).with(same(w), ...)...
// This shouldn't compile as same() requires the parameter to be passed
// by reference.
```

This also means that constraints like `eq()` and `gt()` should work on both
value parameters and reference parameters.

Mockpp doesn't make this distinction between the two kinds of parameters. As a
result, this will compile:

```cpp
ChainableMockMethod<void, int, double> foo_mocker;
```

but the following will not:

```cpp
ChainableMockMethod<void, int, const double&> foo_mocker;
// Compiler error: forming reference to reference type 'const double&'.
```

This means in mockpp a constraint cannot check the address of a by-reference
parameter. I view this as a significant, if not fundamental, flow in the design.

**Note:** jMock doesn't have this problem, as in Java all objects are passed by
reference. The mistake of mockpp is to copy jMock's design without adapting it
to C++.

### Handling Const Parameters

In C++, a user can choose to mark a function parameter as `const`. This has no
effect on the linker, but would make sure that the function body does not change
its value. For example:

```cpp
virtual void Foo(const int x);
```

(Note that technically there is no need to mark a parameter as `const` in the
*declaration* of the method. You can leave out `const` in the declaration and
keep it in the definition. Still, some people may choose to use `const`
parameters for various reasons.)

If, however, the mock class writer tries to mock this method naively:

```cpp
ChainableMockMethod<void, const int> foo_mocker;
// The canonical form should be ChainableMockMethod<void, int>.
```

the compiler will *not* complain about this. Instead, you'll get a compiler
error when trying to set an expectation on this mock method:

```cpp
foo_mocker.expect(once()).with(eq(5))....
// Long error message: no matching function for call to ...
```

This can be very confusing. Ideally, we should either make
`ChainableMockMethod<void, const int>` fail to compile, or make it actually
usable.

## Evaluation on Mockpp's Usability

Mockpp provides *five* different ways for mocking a method. In many cases, a
user can choose any of them. This section inspects the usage of each of them,
and presents our impression on how they appeal to a user and how they can be
improved.

### Basic Mocks (Explicitly Checking Expections)

#### For Functions with Non-void Input/Output Types

To implement a mock for

```cpp
class MyClass {
  virtual U Foo(T x);
  ...
};
```

we need to:

1.  Have some data structures to hold the input and output of the function
    `Foo()`:

    ```cpp
      ExpectationList<T>  foo_x_;
      ReturnObjectList<U> foo_output_;
    ```

2.  Initialize them in the c'tor of the mock class:

    ```cpp
    MyClassMock() : MockObject("MyClassMock", NULL),
       foo_x_("MyClassMock/foo_x_", this),
       foo_output_("MyClassMock/foo_output_", this) {}
    ```

3.  Implement the mocked method:

    ```cpp
    virtual U Foo(T x) {
       foo_x_.addActual(x);
       return foo_output_.nextReturnObject();
    }
    ```

To use the mock class, we need to set the expectation and return values,
exercise the code, and then verify the expectations:

```cpp
MyClassMock mock;
mock.foo_x_.addExpected(15);
mock.foo_x_.addExpected(gt<T>(16));
mock.foo_output_.addObjectToReturn(23);
mock.foo_output_.addObjectToReturn(25);
...
mock.verify();
```

**Comments:**

*   `nextReturnObject()` is a misnomer, as in C++ not everything is an object.
    `NextReturnValue()` is a better name.

*   `addObjectToReturn()` has the same problem, and is inconsistent with
    `nextReturnObject()` (why `ObjectToReturn` vs `ReturnObject`?). I'd call it
    `AddReturnValue()`.

*   What if the function takes more than one parameters? It's inconvenient to
    have one object for each input parameter.

*   It's tedious to have separate objects to hold the input and output of
    `Foo()`. I would merge them into a single object `foo_` of type
    `FunctionIO1<T, U>`:

    ```
      FunctionIO1<T, U> foo_;

      virtual U Foo(T x) {
         return foo_.Invoke(x);
      }
    ```

*   It's bad to have separate lines of code to add expectations and return
    values. (Or is it so?) The reader needs to jump back and forth to see the
    connection. What if their numbers don't match? I prefer something like

    ```
    mock.foo_(15).Returns(23);
    mock.foo_(16).Returns(25);
    ```

    or

    ```
    mock.foo_.AddResponse(15, 23);  // Foo(15) returns 23.
    mock.foo_.AddResponse(16, 25);  // Foo(16) returns 25.
    ```

    I haven't decided which syntax is better yet. The latter probably is better
    at Google as the former seems too clever to the taste of the style guide.

#### For Functions with Void Input/Output Types

To implement a mock for

```cpp
class MyClass {
  virtual void Foo();
  ...
};
```

we need:

```cpp
  ExpectationCounter foo_counter_;

  MyClassMock() : MockObject("MyClassMock", NULL),
    foo_counter_("MyClassMock/foo_counter_", this) {}

  virtual void Foo() {
    foo_counter_.inc();
  }
```

To use the mock:

```cpp
  MyClassMock mock;
  mock.foo_coutner_.setExpected(2);
  ...
  mock.verify();
```

**Comments:**

*   `setExpected(2)` can be easily confused with `addExpected(2)`. Also, it's a
    bit undesirable that the user has to use a different template class when the
    input type is `void`. I'd do something like:

    ```cpp
      FunctionIO0<void> foo_;

      virtual void Foo() {
        foo_.Invoke();
      }

      MyClassMock mock;
      mock.foo_.ExpectCall();
      mock.foo_.ExpectCall();  // Expects 2 calls.
    ```

### Visitable Mocks Using Macros

To implement a mock for

```cpp
class MyClass {
  virtual U Foo(T x);
  ...
};
```

we need to:

```cpp
class MyClassMock : public VisitableMockObject {
  MyClassMock() : VisitableMockObject("MyClassMock", NULL),
    MOCKPP_CONSTRUCT_MEMBERS_FOR_VISITABLE_EXT1(Foo, ext) {}

  MOCKPP_VISITABLE_EXT1(MyClassMock, U, Foo, T, U1, ext, T1);
  // T1 is the type of the internal variable for parameter 1.
  // U1 is the type of the internal return value.
  // ext is the extension for the internal variable name.
};
```

To use it:

```cpp
  MyClassMock mock;
  MOCKPP_CONTROLLER_FOR_EXT(MyClassMock, Foo, ext) foo_controller(&mock);

  // Expects a call to Foo() where the argument is close to 5.
  mock.Foo(eq<T1>(5, 2));

  // Lets Foo() return 10 when the argument is close to 8.
  foo_controller.addResponseValue(10, eq<U1>(8, 3));

  mock.activate();
  ...
  mock.verify();
```

**Comments:** The syntax sucks **big time**. It quickly gets unmanageable when
the function has more parameters. We cannot realistically expect people to learn
and endure this.

### Visitable Mocks Using Templates

To define the mock:

```cpp
class MyClassMock : public VisitableMockObject {
  MyClassMock() : VisitableMockObject("MyClassMock", NULL),
    foo_("Foo", this) {}

  virtual U Foo(T x) {
    return foo_.forward(x);
  }

  void Foo(ConstraintHolder<T>& x) {
    foo_.forward(x);
  }

  VisitableMockMethod<U, T> foo_;
};
```

To use it:

```cpp
MyClassMock mock;
// Expects a call with an argument close to 2.
mock.Foo(eq<T>(2,1));
// Expects a call with an argument close to 7.
mock.Foo(eq<T>(7,1));
mock.foo_.addResponseValue(10, eq<T>(2, 2));
mock.foo_.addResponseValue(15, eq<T>(6, 2));
mock.activate();
...
mock.verify();
```

**Comments:**

*   It's tedious to have to define `Foo()` twice: once taking a `T` parameter,
    and once taking a `ConstraintHolder<T>`. It's not a problem for Java, which
    automatically generates the methods using reflection. For C++, however, this
    gets repetitive.

*   The way for setting expectations (`mock.Foo(eq<T>(2,1));`) is clever, but
    requires the function return type `U` to have a public default constructor,
    or the code won't compile. This is pretty limiting. (Does this work when the
    return type is a reference?)

    Another problem is that there may be a problem if the default constructor of
    `U` has undesirable side effects.

    Note that it's not a problem at all for Java, where objects are always
    passed by handles (and one can always return `null`). However, a literal
    port of the Java design is not appropriate for C++.

*   The way for specifying the response is strange. Why wouldn't the user just
    override `Foo()` and specify the logic in the subclass? I can understand why
    jMock and EasyMock don't do this, as they create mock *objects*, not mock
    *classes*, but C++ is different.

Q: How does mockpp overload the `VisitableMockMethod` template? Using default
parameters?

Q: How does one specify the ordering of invocations to **two** methods? E.g. we
want to verify that `Foo()` is called before `Bar()`.

### Chainable Mocks Using Macros

Like visitable mocks using macros, this approach involves many ugly macros. I
don't think we want to use this.

### Chainable Mocks Using Templates

To implement a mock:

```cpp
class MyClassMock : public ChainableMockObject {
  MyClassMock() : ChainableMockObject("MyClassMock", NULL),
    foo_("Foo", this) {}

  virtual U Foo(T x) {
    return foo_.forward(x);
  }

  ChainableMockMethod<U, T> foo_;
};
```

To use it:

```cpp
MyClassMock mock;
mock.foo_.expects(atLeast(3))       // Foo() should be called >= 3 times,
         .with(eq<T>(5, 1))         // with an argument close to 5,
         .after("Bar")              // after Bar() has been called,
         .will(returnValue<U>(1));  // and will return 1.
...
mock.verify();
```

**Comments:**

*   I'm still confused what order the chain of clauses should be in. Can I swap
    `with` and `after`? If I do that, does the meaning change? Java has very
    good IDEs that auto-complete stuff so this is less of a concern, but
    equally-good IDEs for C++ are not widely used at Google.

*   What's the rule for interpreting a long chain of clauses?

*   It's tedious to have to have a boilerplate argument `this` in the
    initialization of each mock method object. We could avoid this by making the
    container class handle the registration, i.e. instead of

    ```cpp
      MyClassMock() : ChainableMockObject("MyClassMock", NULL),
        foo_("Foo", this),
        bar_("Bar", this),
        baz_("Baz", this) {}
    ```

    we could write something like

    ```cpp
      MyClassMock() : ChainableMockObject("MyClassMock") {
        RegisterMocks("Foo", foo_,
                      "Bar", bar_,
                      "Baz", baz_,
                      NULL);
      }
    ```

*   If we let the d'tor of `ChainableMockMethod` call `verify()`, then normally
    the user won't need to call `mock.verify()` directly. jMock doesn't do this
    as in Java the programmer doesn't control the timing of the destruction of
    an object. However, this is C++ and we can take advantage of the
    deterministic construction/destruction of objects.

## Conclusions

While mockpp has its values, I don't think it's well-designed, and wouldn't
recommend to use it at Google. The main things I don't like about mockpp are:

*   **Complexity:** The entire code consists of **74,000+** lines of C++ code,
    or **37,000+** lines if we exclude tests and examples. This gives a hint to
    the amount of effort needed to learn and maintain the tool. It is a lot when
    we consider the number of potential users.
*   **Kitchen sink:** Mockpp provides too many ways to do the same thing, and
    therefore it's often unclear to a user which option should be taken. (It
    supports both visitable mocks from EasyMock *and* chainable mocks from
    jMock, which have vastly overlapping functionalities but completely
    different syntaxes; for each of the two, there are a macro version *and* a
    template-based version, which again provide similar functionalities with
    different syntaxes.)
*   **Ugly macros:** Mockpp uses many macros that are plain ugly and unreadable,
    e.g. `MOCKPP_VOID_CHAINABLE_EXT1(ChainMock, open, const string &, ext,
    string);` and `MOCKPP_CONSTRUCT_MEMBERS_FOR_VOID_VISITABLE_EXT1(calculate,
    ext)`. This is a sure-fire way to drive users away.
*   **Dependency on exceptions:** Mockpp uses C++ exceptions, which are disabled
    at Google. We need to figure out the best way to write mocks without using
    exceptions, which may end up being a different style.
*   **Too direct a port from Java:** I noticed quite a few things that I would
    do differently to accommodate for the difference between C++ and Java. I
    feel that the author of mockpp was focusing on providing a faithful port
    from EasyMock and jMock, without thinking about C++'s specifics and
    exploring other design options enough.

I'm not claiming to be able to do a better job in *all* of the above categories.
However, some of these problems are glaring and likely to seriously impede the
practice of using C++ mock objects for testing at Google. Before trying to adopt
mockpp, we should fully explore the design space to make sure we have a design
that suits the need of C++ programmers at Google best.
