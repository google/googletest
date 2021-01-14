# gMock - a Framework for Writing and Using C++ Mock Classes

<!--#include file="under-construction-banner.md"-->

**Status:** Draft \
**Tiny URL:** http://go/gmockdesign \
**Author:** Zhanyong Wan (who/wan)

<!-- GOOGLETEST_CM0035 DO NOT DELETE -->

(To participate in discussions on gMock, please subscribe to
[opensource-gmock](https://groups.google.com/a/google.com/group/opensource-gmock/subscribe).
Past discussions can be viewed
[here](https://groups.google.com/a/google.com/group/opensource-gmock/topics) and
[here](https://mailman.corp.google.com/pipermail/c-mock-dev/).)

(The slides for my gMock preview talk can be found here:
[ppt](http://wiki.corp.google.com/twiki/pub/Main/WanTalks/0706-beijing-gmock-preview.ppt).)

## Objective

[Mock objects](http://en.wikipedia.org/wiki/Mock_object) are simulated objects
that mimic real objects in controlled ways. They are useful for driving the
design of a system, and for testing some other object when it's difficult to use
real objects in a test.

While the idea of mocks applies to all objected-oriented languages, writing them
in C++ has many practical difficulties, due to the lack of support for
reflection in the language, the complexity and irregularity of C++, and the lack
of adequate tools. As an unfortunate result, C++ programmers often avoid writing
mocks, resulting in big, monolithic classes in production, and slow, brittle,
and difficult-to-maintain tests.

We believe that a good framework can make it much more pleasant to write and use
mocks in C++. Such a tool would help people write more
[small](https://wiki.corp.google.com/twiki/bin/view/Main/GoogleTestDefinitions)
tests that are quick, robust, and precise. Perhaps more importantly,
incorporating mocks early and often in the design process helps people discover
the role interfaces in the system and thus often leads to
[better designs](http://www.jmock.org/oopsla2004.pdf).

We plan to develop *gMock* as a generic framework for creating and using mock
classes in C++. We would encourage people to *use gMock as a design tool as much
as a testing tool*.

### Goals of gMock

*   **Supporting all interfaces:** A user should be able to use gMock to create
    a mock class for any C++ interface (i.e. a class whose methods are virtual).
    In particular, interface templates should be supported, and there should be
    no restriction on the types of the parameters - `const` parameters, pointer
    parameters, reference parameters, reference-to-`const` parameters, and etc
    should all be allowed.

    gMock can also be used to mock a "loose" interface (i.e. the set of
    operations a template class or template function expects its type argument
    to support). This is useful for testing code that uses the
    ["high-performance dependency injection"](https://engdoc.corp.google.com/eng/doc/tott/episodes/33.md)
    technique.

*   **Precise specification of the intention:** gMock should enable a user to
    precisely specify the intended behavior of a mock object, and how its
    methods are expected to be called (in what order, with what arguments, etc).
    In particular, it should not force the user to over-specify the problem
    (which results in brittle tests that break when unrelated changes to the
    code are made), or to under-specify the problem (which results in tests that
    continue to pass when they shouldn't).

*   **Intuitive and declarative syntax:** A declarative syntax fosters thinking
    at the right abstraction level, and makes the code readable and less
    error-prone. Therefore gMock should provide intuitive and declarative syntax
    for:

    1.  creating a mock class, and
    2.  controlling the behavior of a mock object. When the two goals conflict,
        the latter takes precedence, as it usually needs to be done many more
        times than the former.

*   **Extensible:** No framework can be expected to cover all users' needs.
    Therefore, gMock shouldn't tie the users to whatever it provides. Instead, a
    user should be able to easily extend the framework to accomplish more
    advanced tasks.

*   **Helpful error messages:** Bad error messages are a sure-fire way to
    frustrate the users and drive them away. Therefore, gMock should generate
    clear and sensible messages

    1.  when the code fails to compile - this can be hard as lots of templates
        have to be used in the implementation, but we should try our best; and
    2.  when a user-supplied expectation fails. This also applies to
        user-defined extensions, given that the user has done a good job
        implementing the extensions.

*   **Easy to learn:** We want gMock to make people's life easier, not harder.
    It defeats our purpose if the framework is complex and difficult to learn.

*   **Easily automatable:** The design of gMock should make the process of
    creating a mock class from an interface fairly mechanical, and thus doable
    by the automated
    [mock class generator](https://wiki.corp.google.com/twiki/bin/view/Main/MockClassGeneratorDev).

*   **Working in Google's environment:** While we may be interested in open
    sourcing gMock later, our primary goal is to serve Google. Therefore gMock
    must work well in our environment. In particular, it must not use
    exceptions, and should work well with
    [gUnit](https://wiki.corp.google.com/twiki/bin/view/Main/GUnitGuide).

### Non-goals

*   **Mocking non-virtual methods:** gMock is a source-level tool that works
    with standard compilers and linkers. It doesn't attempt to swap the object
    code of a mock class and that of a real class on-the-fly. Therefore, only
    virtual methods and template arguments can be mocked by gMock.
*   **Supporting arbitrary number of parameters:** Due to limitations of the C++
    language, there will be a practical limit on the number of parameters a mock
    function can have. Support for more parameters can be added as needed.
*   **Supporting non-Linux platforms:** The initial implementation may not run
    on Windows or Mac OS. We have limited resources and need to make sure that
    Linux users are served first. However, we'll try to avoid doing things that
    will make porting gMock to non-Linux platforms difficult.
*   **Special support for particular projects:** gMock is a generic framework
    that makes mocking easy for *all* Google C++ projects. It should not contain
    logic that's useful only to a small number of projects.

## Background

### Terminology

Different people often use "mock" to mean different things. This document
borrows the terminology popularized by
<a href="http://www.martinfowler.com/articles/mocksArentStubs.html">Martin
Fowler</a>:

*   **Dummy** objects are passed around but never actually used. Usually they
    are just used to fill parameter lists.
*   **Fake** objects actually have working implementations, but usually take
    some shortcut (perhaps to make the operations less expensive), which makes
    them not suitable for production.
*   **Stubs** provide canned answers to calls made during the test, usually not
    responding at all to anything outside what's programmed in for the test.
*   **Mocks** are objects pre-programmed with expectations which form a
    specification of the calls they are expected to receive.

### Fakes vs Mocks

Many people are not clear about the difference between a fake and a mock, and
use the terms interchangeably. However, to understand why we need gMock and what
it will deliver, it's crucial to distinguish the two.

Compared with a fake, a mock object is "dumb" and usually doesn't have a working
implementation. However, it allows the user to control its behavior and set
expectations on the calls it will receive. For example, you can tell a mock
object that its `Foo()` method will be called twice with an argument that's
greater than 10, and it should return 12 and 14 respectively.

It may seem that mocks are not very useful compared to fakes, but the Java
community has shown this perception to be wrong. The ability to control a mock
object's behavior and specify the expected interaction between it and the code
under test makes it far more flexible and useful than a fake in designing and
testing a software system.

While fake classes have to be crafted with domain knowledge, mock classes can
actually be created mechanically - with suitable support from a framework. In
more dynamic languages like Java, C#, and Python, there are tools that create
mock objects on the fly without any user intervention. In C++, this cannot be
done within the language itself. However, a framework can make the task much
easier for a user, and the
[mock class generator](https://wiki.corp.google.com/twiki/bin/view/Main/MockClassGeneratorDev)
will make the process automated to a large extent.

### C++ Mocking at Google

To our knowledge, no tool or library is used at Google to facilitate the
creation of mock classes. As a result, people have been writing mock classes
manually. Such classes are typically tedious to create, and lack functionalities
for effective mocking. As a result, people are often frustrated and decided to
avoid mock classes.

As a rough estimate, as of 3/15/2007, the number of existing C++ mock classes in
our source tree is:

```shell
$ gsearch -f="\.(h|cc|cpp)$" -a -c "^\s*class\s+(Mock\w*|\w+Mock)\s*[:{]"
748
```

while the number of all C++ classes is:

```shell
$ gsearch -f="\.(h|cc|cpp)$" -a -c "^\s*class\s+\w+\s*[:{]"
188488
```

Roughly 1 out of every 250 C++ classes has a corresponding mock class. Clearly
this is not enough.

### Situation outside Google

The situation of using C++ mocks outside of Google is not a lot brighter either.
Although there is an open-source framework
([mockpp](http://mockpp.sourceforge.net/)) for writing mock classes, it is
overly complex and has limited functionalities. As a result, it doesn't have a
large following.

### Existing Mock Frameworks

A good mock framework takes years of hard work and actual use in the field to
mature. Therefore, it pays hugely to learn from existing mock frameworks: what
they can and cannot do, why they are the way they are, how they have evolved,
what lessons their creators have learned, and what they intend to do next, etc.

We studied some well-known mock frameworks for Java
([Mock Objects](http://www.mockobjects.com),
[EasyMock](http://www.easymock.org), [jMock 1](http://www.jmock.org), and
[jMock 2](http://cvs.jmock.codehaus.org/browse/~raw,r=1.3/jmock/website/content/cheat-sheet.html))
and for C++ ([mockpp](http://mockpp.sourceforge.net/)). Our conclusion is:

*   Mock Objects is the most primitive of the four. It provides some basic
    constructs for a user to set expected arguments and return values, but not
    much beyond that.
*   EasyMock makes the simple case very easy, but isn't flexible enough to
    handle more advanced usage well. Often the users are forced to either
    over-specify or under-specify their intention, resulting in brittle or
    imprecise tests.
*   jMock 1 and 2 share the same design philosophy, but have incompatible
    syntaxes. They allow a user to precisely specify the intention of the test
    in most cases, and can be easily extended by the user to handle more complex
    situations.
*   mockpp is a mixed bag of constructs from the above three. It doesn't have a
    coherent design philosophy, and doesn't address C++'s specific requirements
    well. It is more complex, redundant, and difficult to learn and use than we
    would like.

### Our Plan

We believe that jMock is the most interesting and promising of the four. Its
creators have been aggressively experimenting with new ideas and designs, and
have produced many iterations before the current form. They have also documented
their experience and lessons in developing jMock in
[two](http://www.jmock.org/oopsla2004.pdf)
[papers](http://mockobjects.com/files/evolving_an_edsl.ooplsa2006.pdf), which
contain many valuable insights.

Therefore, the design of gMock is heavily influenced by jMock. Many constructs
will be directly ported from jMock. Meanwhile, we'll revisit various design
decisions in C++'s context to make sure that we take advantages of C++ strengths
and avoid its weaknesses. We will also address some challenges that are unique
to C++.

## Overview

### Why a Framework

Mock objects serve two distinct purposes in designing and testing a software
system:

1.  They implement the same interfaces as the real classes and provide canned
    responses, allowing code that uses them to compile and run; and
2.  They can verify that the actual interaction between them and the code under
    test matches what the user expects (for example, the right functions are
    called, in the right order, with the right arguments, etc).

Without a framework, a user could manually implement mock functions to return
answers that are either pre-defined or computed using simplified logic. To
verify that the interaction that actually happens matches the expectation, a
user would typically let the mock functions record the interaction in some way,
and inspect the record in the end. This poor man's approach leaves several
things to be desired:

1.  Writing a mock class manually is not easy, and often viewed as a burden to
    be avoided.
2.  Different tests use a mock class in different ways. Therefore, it is often
    impractical to provide a working fake implementation that is useful for all
    tests.
3.  Describing what the interaction should be by inspecting what really has
    happened is round-about and unnatural. It obscure the intention of the test
    author, and results in tests that are hard to read and understand.
4.  It is often too late to check how the interaction went after it has
    finished. Much better is to report a failure at the exact moment an
    expectation is violated. This gives the user a chance to check the context
    of the failure (the stack trace, the variables, etc) before important
    information is lost.

The purpose of gMock is to address the above problems. In particular, it will:

1.  make the task of writing a mock class much easier by hiding the low-level
    mechanism from the user;
1.  let the user of a mock class, rather than its creator, specify the intended
    responses;
1.  let the user specify the intended interaction in a clear and direct syntax;
    and
1.  catch violations to the specification as soon as they arise.

### gMock's Expressiveness

The Java community's years of experience using mocks shows that a mock framework
should enable a user to directly specify the following properties of the
interaction between a mock object and its surrounding code:

*   How many times will a function be called?
*   What arguments will be used?
*   In what order will the calls be made?
*   What should the functions return?
*   What side effects (if any) should the calls incur?

Also, it's important to be able to loosen the constraints when necessary to
prevent brittle tests. For example,

*   If the test doesn't care about how many times a function will be called, the
    test writer should be able to make that clear;
*   If the exact value of an argument doesn't matter, the user should be able to
    say so;
*   If only a subset of the calls need to happen in a strict order, the user
    should be allowed to specify a partial order.

### Architecture of gMock

gMock is a C++ library that will be linked into a user's test code. It consists
of the following components (the syntax used in the code samples is
*tentative*):

1.  **Function mockers:** A family of template classes will be provided for the
    user to mock functions with different arities. For example, a field of type

    ```
    FunctionMocker<int(bool, const string&)>
    ```

    will be used to mock a function with signature

    ```
      virtual int Foo(bool, const string&);
    ```

1.  **Specification builder:** This provides a syntax for the user to specify
    the expected arguments and responses of a mock function. For example, to say
    that `Foo()` will be called exactly twice with arguments `true` and a string
    that contains `"hello"`, and will return 10 and 12 respectively, the user
    can write:

    ```
    EXPECT_CALL(mock_object, Foo(Eq(true), HasSubstring("hello"))
      .Times(2)
      .WillOnce(Return(10))
      .WillOnce(Return(12))
    ```

1.  **Cardinalities, matchers, and actions:** A collection of pre-defined
    *cardinalities* (e.g. `2`), argument *matchers* (e.g. `Eq()` and
    `HasSubstring()`), and stub *actions* (e.g. `Return()`) will enable the user
    to precisely specify the intended interaction in most cases. When this set
    is inadequate, the user can easily define new cardinalities, matchers, and
    actions.

1.  **Specification interpreter:** An underlying interpreter will verify that
    the actual calls made to the mock object conform to the user's expectations.

gMock helps a user in two kinds of activities: *writing* mock classes and
*using* them in tests. When writing a mock class, a user employs the function
mockers (#1); when using a mock class, the user relies on #2 and #3 to specify
the expected interaction between the mock object and the code under test. As the
test runs and the mock functions are invoked, the specification interpreter (#4)
verifies that the actual interaction matches the expectation, and fails the test
when the two don't match.

## Detailed Design

### Implementing a Mock

This section explains how a user would implement a mock class using gMock. The
final syntax may be slightly different to what's presented here, but the overall
idea should remain the same.

The goal of the design is to allow mocking functions that take 0 or more
arguments, functions that are overloaded on the number/types of parameters,
const methods, and methods that are overloaded on the const-ness of this object.

#### Using macros

The easiest way to define a mock class is to use the `MOCK_METHOD` macro.
Specifically, to mock an interface

```cpp
class FooInterface {
  ...
  virtual R Method(A1 a1, A2 a2, ..., Am am) = 0;
  virtual S ConstMethod(B1 b1, B2 b2, ..., Bn bn) = 0;
};
```

one would simply write

```cpp
class MockFoo : public FooInterface {
  ...
  MOCK_METHOD(R, Method, (A1 a1, A2 a2, ..., Am am), (override));
  MOCK_METHOD(S, ConstMethod, (B1 b1, B2 b2, ..., Bn bn), (const, override));
};
```

#### Using no macro

The user can also choose to implement a mock class without using the macros.

For each function to be mocked that is not overloaded, the user should define a
**function mocker** member variable and implement the function by forwarding the
call to the function mocker, which knows how to respond to the given arguments.

A user specifies the mock function's default behavior and expectations on it by
calling the *mock spec function* in an `ON_CALL()` or `EXPECT_CALL()` statement.

Now let's see the concrete syntax. To mock a function that takes no argument:

```cpp
class AbcInterface {
  ...
  virtual R Foo() = 0;
};
```

a user would write:

```cpp
class MockAbc : public AbcInterface {
  ...
  // Mock Foo().  Implements AbcInterface::Foo().
  virtual R Foo() { return gmock_Foo.Invoke(); }

  FunctionMocker<R()> gmock_Foo;
};
```

To mock a function that takes some arguments:

```cpp
  virtual R Bar(A1 a1, A2 a2);
```

a user would write:

```cpp
  virtual R Bar(A1 a1, A2 a2) { return gmock_Bar.Invoke(a1, a2); }

  FunctionMocker<R(A1, A2)> gmock_Bar;
```

To mock a `const` method:

```cpp
  virtual R Baz(A1 a1) const;
```

a user would write:

```cpp
  virtual R Baz(A1 a1) const { return gmock_Baz.Invoke(a1); }

  mutable FunctionMocker<R(A1)> gmock_Baz;
```

Mocking overloaded functions is a little bit more involved. For each overloaded
version, the user needs to define an overloaded mock controller function, e.g.

```cpp
  virtual R Bar(A a) { return gmock_Bar_1.Invoke(a); }
  MockSpec<R(A)>& gmock_Bar(Matcher<A> a) {
     return gmock_Bar_1.With(a);
  }

  virtual R Bar(B b, C c) { return gmock_Bar_2.Invoke(b, c); }
  MockSpec<R(B, C)>& gmock_Bar(Matcher<B> b, Matcher<C> c) {
     return gmock_Bar_2.With(b, c);
  }
 private:
  FunctionMocker<R(A)> gmock_Bar_1;
  FunctionMocker<R(B, C)> gmock_Bar_2;
```

If a method is overloaded on the const-ness of this object, the user can
distinguish between the two overloaded versions by using a const- vs non-const-
reference to the mock object. The `Const()` function provided by gMock can be
used to get a const reference to an object conveniently:

```cpp
template <typename T>
inline const T& Const(const T& x) { return x; }
```

### Syntax for Setting Default Actions and Expectations

For each mock function, there are two interesting properties for a user to
specify:

1.  the **default action**: what the function should do by default when invoked,
    and
2.  the **expectations**: how the function will be called in a particular test.

While the default actions of a mock class usually don't change from test to
test, a user typically sets different expectations in different tests.

The following syntax is proposed for setting the default action of and the
expectations on a mock function:

```cpp
ON_CALL(mock-object, method(argument-matchers))
  .With(multi-argument-matcher) ?
  .WillByDefault(action);
```

The `ON_CALL()` statement defines what a mock function should do when its
arguments match the given matchers (unless the user overrides the behavior in
`EXPECT_CALL()`). The `With()` clause is optional. The `WillByDefault()` clause
must appear exactly once.

```cpp
EXPECT_CALL(mock-object, method(argument-matchers))
  .With(multi-argument-matcher) ?
  .Times(cardinality) ?
  .InSequence(sequences) *
  .WillOnce(action) *
  .WillRepeatedly(action) ?
  .RetiresOnSaturation(); ?
```

The `EXPECT_CALL()` statement says that the mock function should be called the
given number of times (`cardinality`), in the order determined by the
`sequences`, and with arguments that satisfy the given `matchers`. When it is
called, it will perform the given `action`. In this statement, all clauses are
optional and you can repeat `WillOnce()` any number of times. When no action is
specified, the default action defined by `ON_CALL()` will be taken.

For non-overloaded methods, '(argument-matchers)' may be omitted:

```cpp
ON_CALL(mock-object, method)
  .With(multi-argument-matcher) ?
  .WillByDefault(action);

EXPECT_CALL(mock-object, method)
  .With(multi-argument-matcher) ?
  …cardinality and actions…
```

This allows test writers to omit the parameter list and match any call to the
method. Doing so eases the burden on test maintainers when refactoring method
signatures. The 'With()' clause is still optional when the parameter list is
omitted.

We make `ON_CALL()` and `EXPECT_CALL()` macros such that we can tell the mock
object the file name and line number of a rule, which can be used to produce
better error messages at run time. When running a test inside Emacs and an
expectation is violated, the user can jump to the expectation by hitting
`<return>` on the message.

#### Argument Matchers

An `argument-matcher` can be any of the following:

```cpp
Void(), Eq(value), Ge(value), Gt(value), Le(value), Lt(value), Ne(value),
HasSubstring(string), SubstringOf(string),
Same(value), Anything(), Any<type>(), Not(argument-matcher), AnyOf(argument-matchers), AllOf(argument-matchers)
```

In addition, a user can define custom matchers by implementing the
`MatcherImplInterface<type>` interface (TBD).

#### Multi-argument Matchers

Matchers in the previous section match one argument at a time. Sometimes it's
necessary to check all arguments together. This is when multi-argument matchers
are needed:

```cpp
Eq(), Ge(), Gt(), Le(), Lt(), Ne(),
HasSubstring(), SubstringOf(),
Same(), AnyThings(), Not(multi-argument-matcher), AnyOf(multi-argument-matchers), AllOf(multi-argument-matchers)
```

When there are multiple `WithArguments()` clauses in a rule, all of them have to
be satisfied for the rule to match a call.

A user can define new multi-argument matchers by implementing the
`MatcherImplInterface<std::tuple<type1, ..., type_n> >` interface (TBD).

#### Actions

```cpp
Return(), Return(value), DoDefault(), Fail(string),
SetArgPointee<N>(value), DoAll(actions), ...
```

The version of `Return()` that takes no argument is for mocking `void`-returning
functions. The clauses are all statically typed, so a user won't be able to
mistakenly use `Return()` when the mocked function has a non-void return type,
or to use `Return(value)` when the function returns `void`.

On consecutive calls that match a given expectation, actions specified in
multiple `WillOnce()` clauses in the expectation will be used in the order they
are presented. After all `WillOnce()` clauses have been exhausted, the action
specified by `WillRepeatedly()` will always be used. If there is no
`WillRepeatedly()`, the default action defined by `ON_CALL()` will be taken.

When side effects need to be mocked (e.g. changing a field or a global variable,
calling a function of a class-typed argument, and so on), users can define a
custom action by implementing the `ActionImplInterface<return-type(type1, ...,
type-n)>` interface (TBD).

#### Cardinalities

A cardinality tells how many times a function is expected to be called. The
number doesn't have to be always exact, as we don't want to over-specify the
behavior and result in brittle tests.

```cpp
integer, AtLeast(n), AtMost(n), Between(m, n), AnyNumber()
```

This set can be extended by the user implementing the `CardinalityImplInterface`
interface (TBD).

If no cardinality is specified in an `EXPECT_CALL()` statement, gMock will infer
it this way:

*   If there are n `WillOnce()` clauses but no `WillRepeatedly()`, the
    cardinality is n;
*   If there are n `WillOnce()` clauses and a `WillRepeatedly()`, the
    cardinality is `AtLeast(n)`.

#### Sequences

Often we want to specify the order in which mock functions are called. However,
we may not want to specify a total order as that may lead to flaky tests that
will be broken by unrelated changes. For this reason, gMock allows the user to
specify a partial order on the calls by organizing them into *sequences*.

Basically, a sequence is a chain of expectations that have to happen in the
order they are defined. Sequences are identified by their names. For example,
the following defines a sequence named `"a"`, which contains two expectations
where the first has to happen before the second:

```cpp
  Sequence a;

  EXPECT_CALL(mock_foo, Func1(Anything()))
     .Times(1)
     .InSequence(a);

  EXPECT_CALL(mock_bar, Func2(Eq(2)))
     .Times(3)
     .InSequence(a);
```

Note that expectations in the same sequence don't have to be on the same object
or same function, as the above example shows.

An expectation can belong to any number of sequences, in which case all order
constraints have to be honored. For convenience, we allow `InSequence()` to take
multiple sequences. In the following example, the first expectation must be
matched before the second and the third, but we don't care about the relative
order of the latter two:

```cpp
  Sequence a, b;

  EXPECT_CALL(mock_foo, Func1(Anything()))
     .Times(1)
     .InSequence(a, b);

  EXPECT_CALL(mock_bar, Func2(Eq(2)))
     .Times(AnyNumber())
     .InSequence(a);

  EXPECT_CALL(mock_bar, Func2(Eq(5)))
     .Times(AnyNumber())
     .InSequence(b);
```

For convenience, we allow an expectation to contain multiple `InSequence()`
clauses, in which case their arguments will be joined. For example, another way
to write the first expectation in the above example is:

```cpp
  EXPECT_CALL(mock_foo, Func1(Anything()))
     .Times(1)
     .InSequence(a)
     .InSequence(b);
```

A common scenario is that the user wants all expectations to match in the strict
order they are defined. Instead of letting the user put `InSequence()` in every
expectation, we provide the following short-hand:

```cpp
  {
     InSequence s;

     EXPECT_CALL(...)...;
     EXPECT_CALL(...)...;
     ...
  }
```

In the above snippet, when the variable `s` is constructed, gMock will generate
a unique new sequence and automatically put each `EXPECT_CALL()` in the scope of
`s` into this sequence. The result is that this group of expectations must match
in the strict order.

The user can also use an existing sequence like this:

```cpp
  Sequence a;
  ...
  {
     InSequence s(a);

     EXPECT_CALL(...)...;
     EXPECT_CALL(...)...;
     ...
  }
```

This can be useful if an existing sequence needs to be extended.

#### Examples

```cpp
EXPECT_CALL(mock_goat, Eat(Eq(5), Anything()))
  .WillOnce(Return(false));
```

The mock goat will be told to `Eat()` 5 of something exactly once; the method
should return `false`.

```cpp
EXPECT_CALL(mock_goat, Drink(HasSubstring("milk")))
  .Times(1);
```

The mock goat will be told to `Drink()` something that contains milk once; the
method should perform its default action when invoked.

```cpp
EXPECT_CALL(mock_elephant, Eat(Same(mock_goat)))
  .Times(0);
```

The mock elephant should never be told to `Eat()` the poor mock goat, which
would be a terrible thing.

```cpp
Sequence a;

EXPECT_CALL(mock_elephant, Eat(Anything()))
  .InSequence(a)
  .WillOnce(Return(true));

EXPECT_CALL(mock_elephant, Walk(Ge(5)))
  .Times(AtLeast(1))
  .InSequence(a)
  .WillOnce(Return(2));
```

The mock elephant will be told to `Eat()` something; after that it will be told
to `Walk()` >= 5 meters at least once; the `Walk()` method should return 2 the
first time, and should do the default action in future calls.

#### Syntax Checking

We will use a combination of compile-time and run-time checks to catch syntax
errors. In particular, the spelling and types of the individual clauses will
(obviously) be done by the C++ compiler, while we'll enforce the order and
counts of the clauses via run-time checks.

Please note that technically it is possible to do the latter checks at compile
time too, and that is the approach of jMock and Mockpp. For the designer of an
embedded domain-specific language (EDSL), it is appealing to leverage the
compiler of the hosting language (C++ in this case) to parse code in the EDSL
and catch errors in it as much as possible. It is also an interesting exercise
in pushing the envelope of EDSL implementation techniques.

However, while we initially wanted to go with jMock's approach, we now think
it's better to defer such checks to run time. The reasons are:

1.  Doing the checks at run time *significantly* reduces the number of template
    classes and simplifies the implementation. This is not only a benefit for
    the author and maintainer of gMock, but also makes it much easier for a user
    to learn gMock. New and existing users will have to read the gMock header
    files from time to time, so it's important to keep the public interface
    small. As an example of what happens when the API is not kept small, try to
    read the header files of Mockpp - you will find a plethora of template
    classes that reference each other, and it's very difficult to tell what
    different roles they play.
1.  The jMock approach enables the IDE to automatically suggest the next clause
    when a user is writing an expectation statement and thus makes it trivial to
    write syntactically correct expectations. Unfortunately, such benefit is
    merely theoretic for most C++ users, as C++ is such a complex language that
    most IDEs do a poor job at understanding the user's source code and
    suggesting auto-completion.
1.  C++ templates generate horrible, horrible compiler errors that often baffle
    even experienced programmers. By enforcing the syntax at compile time, we
    subject gMock's users to the mercy of the C++ compiler, which will generate
    lengthy and cryptic errors when a user makes a small mistake in the syntax.
    It would be much better for us to generate the errors at run time, as we can
    control the messages and choose plain and clear language that guides the
    user to fix the problem.
1.  The default action and expectation statements in gMock are *declarative*,
    and typically each of them will be executed once during the test (not to be
    confused with a rule *matching* an invocation multiple times). Therefore
    there should be little concern that a syntax error is not caught because the
    statement is never actually executed.

### Formal Semantics

The previous section presented the syntax and informally explained the meanings
of various clauses. To avoid ambiguities and make sure we all have the same
understanding on the meaning of a complete test using mock objects, we need to
define the semantics of gMock more strictly.

For an expectation rule to match an actual invocation, three types of
constraints have to be satisfied at the same time:

1.  the order constraints (does the call occur in the right order?),
2.  the cardinality constraints (can the rule accept more invocations?), and
3.  the argument constraints (do all arguments satisfy their matchers?).

As the designer of gMock, we need to decide in which order these constraints
should be applied and how to resolve ambiguities. Our goal is to choose a
semantics that is easy to understand and allows the user to easily express
properties useful for writing tests.

Given that gMock borrows heavily from jMock, naturally one would try to adopt
jMock's semantics. I couldn't find a documentation on that unfortunately. The
following semantics is based on my reverse-engineering jMock and what I think is
reasonable. It differs from the jMock semantics in several important regards.
The exact differences and the rationale behind our decision can be found on the
c-mock-dev [archive](https://g.corp.google.com/group/c-mock-dev-archive) and are
not repeated here.

The proposed semantics can be summarized by two simple principles:

1.  **The orders are sacred**: under no circumstance can an expectation in a
    sequence to match before all expectations that appear earlier in the same
    sequence have been satisfied; and
2.  **Earlier rules take precedence:** when multiple rules can match an
    invocation without violating the order constraints, the one defined the
    earliest wins.

To define the semantics formally, we will use the following terminology:

*   An `ON_CALL()` statement defines a **default action**.
*   An `EXPECT_CALL()` statement defines an **expectation**.
*   An expectation is **active** iff it still can be used to match invocations.
    Otherwise it is **retired**. Initially, all expectations are active.
*   An expectation X is an **immediate pre-requisite** of another expectation Y
    iff there exists a sequence S where X and Y are both in S, X is defined
    before Y, and there is no expectation in S between X and Y.
*   An expectation X is a **pre-requisite** of another expectation Y iff there
    exists a list X[0] = X, X[1], ..., X[n] = Y, where X[i] is an immediate
    pre-requisite of X[i+1] for all i.
*   An expectation (or its cardinality constraint) is said to be **satisfied**
    iff it has reached its minimum number of allowed invocations.
*   An expectation (or its cardinality constraint) is said to be **saturated**
    iff it has reached its maximum number of allowed invocations. A saturated
    expectation by definition must be satisfied, but not vice versa.

After the user has set the default action and the expectations, when a mock
function is called, the following algorithm (in pseudo code) will be used to
find the matching expectation and the matching action:

```cpp
void TryToDoDefault(FunctionMocker& m, const Arguments& args) {
  if (m has a default action for arguments args) {
     perform the default action;
  } else {
     raise error("No action is specified.");
  }
}

void OnInvocation(FunctionMocker& m, const Arguments& args) {
  for_each (active expectation e on function m in the order
                the expectations are defined) {
     if (all pre-requisites of e are satisfied &&
          args match e's argument matchers) {
        // We found a match!

        if (e.is_saturated)
          raise error("Invocation upper bound exceeded.");

        e.invocation_count++;
        retire all prerequisites of e;

        if (e.retires_on_saturation && e.is_saturated)
          e.is_active = false;

        if (e has more action left) {
          a = e.get_next_action();
          perform a;
        } else {
          TryToDoDefault(m, args);
        }
        return;
     }
  }

  TryToDoDefault(m, args);
}
```

To find the default action for the given arguments, we look through all
`ON_CALL()` rules for the mock function, and pick the first one where all
argument matchers are satisfied, if any.

Since C++ exceptions are disabled in `google3`, **we will abort the current
process when gMock raises an error**. We cannot just return from the current
function like what gUnit does, as the mock functions will be called from the
production code under test, and we don't have the luxury to change the
production code at each call site to propagate the error. This is unfortunate,
but I don't see a better solution without enabling exceptions.

The real implementation will be more sophisticated in order to get a decent
performance (e.g. we'll memoize and use other tricks), but the end result must
match the above reference implementation.

**Note:** If you carefully inspect the algorithm, you should convince yourself
that an expectation whose cardinality is `0` has no effect whatsoever, as it is
always satisfied and saturated. This means that you can write such an
expectation, but it won't affect your test in any way. Indeed, this is jMock's
behavior, and jMock's documentation suggests to use `Never()` (jMock's
equivalent of the `0` cardinality) for documentation purpose only.

This bothers me as it contradicts with what one would naturally expect. When I
see

```cpp
  EXPECT_CALL(mock_foo, Bar(Eq(5)))
     .Times(0);
```

I would think that it will be an error if `mock_foo.Bar(5)` is ever called, and
gMock will catch this error at run time. However, jMock tells me that it will
not try to enforce this, and I should treat the statement as if it doesn't
exist.

I propose to give `Times(0)` a semantics that I think is more intuitive. Namely,
we should treat

```cpp
  EXPECT_CALL(mock-object, method(argument-matchers))
     .WithArguments(multi-argument-matcher)
     .Times(0)
     .InSequence(sequences);
```

as if it were

```cpp
  EXPECT_CALL(mock-object, method(argument-matchers))
     .WithArguments(multi-argument-matcher)
     .Times(AnyNumber())
     .InSequence(sequences)
     .WillOnce(Fail("Unexpected call."));
```

I don't like making this a special case, but the other choice seems worse.

**Note:** If a call doesn't match any explicitly written `EXPECT_CALL()`
statement, gMock will perform the default action (as long as it exists) instead
of raising an "unexpected call" error. If you want to assert that a function
should never be called, you must explicitly write an `EXPECT_CALL()` with a `0`
cardinality. This design is picked to allow modular tests:

An interface may contain many methods. Typically, each test will be interested
in only a small number of them, as we favor small and focused tests. Such a test
shouldn't start to fail when the code under test is modified to call functions
not interesting to the test. If no `EXPECT_CALL()` were to mean "no call is
allowed", we would have to say

```cpp
  EXPECT_CALL(mock_foo, UninterestingMethod(Anything()))
     .Times(AnyNumber());
```

for **every** method we do **not** care about. It can be very tedious. Worse,
when we add methods to the interface or remove methods from it, we have to
update every existing test. Clearly this isn't modular and won't scale.

#### Examples

If you are not interested in whether a function will be called or not, you just
don't say anything about it. If the function is called, its default action will
be performed.

If you want to make sure that a function is never called, say it explicitly:

```cpp
  EXPECT_CALL(mock_foo, Bar).Times(0);
  // or:
  EXPECT_CALL(mock_foo, Bar(Anything())).Times(0);
```

If you expect certain calls to a function and want to ignore the rest, just
specify the calls you are explicitly expecting:

```cpp
  EXPECT_CALL(mock_foo, Bar(Eq(1)))
     .WillOnce(Return(2));
  EXPECT_CALL(mock_foo, Bar(Eq(2)))
     .Times(AtMost(5))
     .WillRepeatedly(Return(3));
```

If you expect certain calls to a function and don't want to allow any other
calls to it, just add a `Times(0)` expectation **after** the normal
expectations:

```cpp
  EXPECT_CALL(mock_foo, Bar(Eq(1)))
     .WillOnce(Return(2));
  EXPECT_CALL(mock_foo, Bar(Eq(2)))
     .Times(AtMost(5))
     .WillRepeatedly(Return(3));

  // Any call to mock_foo.Bar() that doesn't match the above rules
  // will be an error.
  EXPECT_CALL(mock_foo, Bar(Anything()))
     .Times(0);
```

Here's one complete example:

```cpp
  ON_CALL(mock_foo, Bar(Anything()))
     .WillByDefault(Return(1));

  Sequence x;

  EXPECT_CALL(mock_foo, Bar(Ne('a')))     // Expectation #1
     .InSequence(x)
     .WillOnce(Return(2))
     .WillRepeatedly(Return(3));

  EXPECT_CALL(mock_foo, Bar(Anything()))  // Expectation #2
     .Times(AnyNumber())
     .InSequence(x);

  mock_foo.Bar('b');  // Matches expectation #1; returns 2.
  mock_foo.Bar('c');  // Matches expectation #1; returns 3.
  mock_foo.Bar('b');  // Matches expectation #1; returns 3.
  mock_foo.Bar('a');  // Matches expectation #2; returns 1.

  // Now that expectation #2 has been used, expectation #1 becomes
  // inactive (remember that the order is sacred), even though it's not
  // yet saturated.

  mock_foo.Bar('b');  // Matches expectation #2, returns 1.
```

Another one:

```cpp
  Sequence a, b;

  EXPECT_CALL(mock_foo, Func1(Void()))      // #1
     .Times(1)
     .InSequence(a);

  EXPECT_CALL(mock_bar, Func2(Anything())   // #2
     .Times(AtLeast(1))
     .InSequence(   b);

  EXPECT_CALL(mock_foo, Func3(Eq(0)))  // #3
     .Times(AtMost(2))
     .InSequence(a, b);

  EXPECT_CALL(mock_foo, Func3(Anything()))      // #4
     .InSequence(a);

  // The order constraints can be illustrated as
  //
  //    #1 < #3 < #4
  //    #2 < #3

  mock_foo.Func1();  // Matches #1
  // Now #1 is saturated but not retired.
  // If Func1() is called again here, it will be an upper-bound error.

  // It would be an error to call mock_foo.Func3(0) here, as #2 is its
  // pre-requisite and hasn't been satisfied.

  mock_bar.Func2(1);  // Matches #2, which is now satisfied.

  mock_foo.Func3(1);
  // Matches #4.  This causes all of #4's remaining pre-requisites (#2
  // and #3) to become inactive.  Note that #3 is trivially satisfied
  // as that AtMost(2) doesn't require it to match any invocation.
```

Yet another one:

```cpp
EXPECT_CALL(mock_foo, Func(Eq(1)))       // #1
  .WillOnce(Return(2))
  .RetiresOnSaturation();

EXPECT_CALL(mock_foo, Func(Anything()))  // #2
  .WillOnce(Return(3));

mock_foo.Func(1);  // Matches #1.
// Now #1 is satisfied, saturated, and retired.

mock_foo.Func(1);  // Matches #2.
// Since #1 is retired now, it doesn't participate in matching function
// calls.  Otherwise this would cause an upper-bound-exceeded failure.
```

### Verifying that All Expectations Are Satisfied

During a test, gMock will verify that each invocation to a mock function matches
one of the expectation rules. However, at the end of a test, we will want to
verify that *all expectations for the mock function have been satisfied*. This
is done by the destructor of the `FunctionMocker<...>` class:

### Life of a Mock

Now let's put the pieces together and see the complete process of using mock
objects in a test. Typically, the user should do it in the following order:

*   **C:** *Constructs* the mock objects;
*   **B:** Set their default **behaviors** using `ON_CALL()`;
*   **E:** Set **expectations** on them using `EXPECT_CALL()`;
*   **I:** Exercise the production code, which will **invoke** methods of the
    mock objects;
*   **D:** *Destructs* the mock objects, which will cause gMock to verify that
    all expectations are satisfied.

Usually, the user can do step 1 and 2 during the set-up phase of the test, step
3 and 4 in the test function body, and step 5 in the tear-down phase.

If the user performs a step out of sequence (e.g. an `EXPECT_CALL()` is
encountered after the mock function is already been called by the test and
before `Verify()` is called), the behavior is **undefined**. gMock will try to
print a friendly error message, but doesn't guarantee to catch all possible
violations, and the initial version may not implement this error check at all.

Valid sequences of using a mock object can be described using the regular
expression

```none
CB*E*I*D
```

### Typing of Argument Matchers

Argument matchers in gMock are statically typed. If we don't provide automatic
conversion between matchers of "compatible" types, the user experience will be
rather unpleasant. Covariance and contravariance are two common schemes for
imposing a sub-typing relation between types. Our observation is that neither
works for matchers in general, and gMock must leave the decision to individual
matcher authors.

#### Background: How Argument Matchers Work

In gMock, argument matchers are used to determine if the actual arguments in a
function invocation match the test writer's expectation. Conceptually, a matcher
for arguments of type `T` implements the following interface:

```cpp
class Matcher<T> {
  virtual bool Matches(T x) = 0;
};
```

For a method with argument type `T`:

```cpp
virtual void Func(T x);
```

its mock will be declared as something like (the actual declaration will be more
complicated but the idea remains the same):

```cpp
void Func(Matcher<T>* x);
```

When the mock `Func()` is invoked with an argument value `v`, which has type
`T`, `Matches(v)` will be called to determine if it's a match.

#### Need for Sub-typing

A straightforward way to mock a method with parameter types `T1`, `T2`, ..., and
`Tn` is to use a list of matchers of type `Matcher<T1>`, `Matcher<T2>`, ..., and
`Matcher<Tn>`. However, this simplistic approach has a usability problem.
Suppose we have a series of functions and their mocks:

```cpp
void Func1(char a);
void Func1(Matcher<char>* a);

void Func2(const char a);
void Func2(Matcher<const char>* a);

void Func3(char& a);
void Func3(Matcher<char&>* a);

void Func4(const char& a);
void Func4(Matcher<const char&>* a);

void Func5(char* a);
void Func5(Matcher<char*>* a);

void Func6(const char* a);
void Func6(Matcher<const char*>* a);
```

(note that `Func2()` has a `const` parameter. Since argument matchers are not
allowed to modify the arguments in any case, technically we could use a
`Matcher<char>` in the mock of `Func2()`. However, we want to make sure that a
user can define the mock using a `Matcher<const char>` too, as this makes the
task of the mock class generator easier.) and some simple, pre-defined matcher
factories:

```cpp
// Matches if the argument equals the given value x.
Matcher<T>* Eq(T x);

// Matches if the argument has the same identify as the
// given object x.
Matcher<T&>* Same(T& x);
```

then a user might be surprised when trying to use these mocks:

```cpp
  Func1('a');       // Invokes the real method.  This works fine.
  Func1(Eq('a'));  // Invokes the mock method.  This works fine too.

  Func2('a');       // Invokes the real method.  This works fine.
  Func2(Eq('a'));  // Compiler ERROR - surprise!!!  Why can't I say that
                         // the argument, which is a const char, should be equal
                         // to 'a'?

  char a = 'a';
  Func3(a);       // Invokes the real method.  This works fine.
  Func3(Same(a));  // Fine.  The argument should reference the variable a.
  Func3(Eq('a'));  // Compiler ERROR - surprise!!!  Why can't I say that
                         // the argument, which is a char&, should have a value
                         // 'a', which is a char?

  const char b = 'b';
  Func4(b);       // Fine.
  Func4(Same(b));  // Fine.  The argument should reference the variable b.
  Func4(Eq(b));  // Compiler ERROR - surprise!!!  Why can't I say that
                         // the argument, which is a const char&, should have
                         // a value equal to b, which is a const char?
  Func4(Same(a));  // Compiler ERROR - surprise!!!  Why can't I say that
                         // the argument, which is a const char&, should reference
                         // a, which is a char?

  char* p = NULL;
  Func5(p);       // Fine.
  Func5(Eq(p));  // Fine.

  Func6("a");       // Fine.
  Func6(Eq("a"));  // Fine.
  Func6(Eq(p));  // Compiler ERROR - surprise!!!  Why can't I say that
                         // the argument, which is a const char*, should point
                         // to where p, which is a char*, points to?
```

(In Java, this issue isn't nearly as severe, as Java has neither reference nor
`const`. Lucky them.)

The compiler errors can be worked around using explicit template instantiating
in most cases, but require more dirty hacks in some others:

```cpp
  // The following works:
  Func2(Eq<const char>('a'));
  Func4(Eq<const char&>(b));
  Func4(Same<const char>(a));
  Func6(Eq<const char*>(p));

  // but this doesn't:
  Func3(Eq<char&>('a'));  // Compiler ERROR!

  // You have to use a temporary variable, and pray that it's not
  // accidentally changed before the actual invocation occurs.
  // No, you cannot make the temporary variable const - that won't
  // compile.
  char temp = 'a';
  Func3(Eq<char&>(temp));
```

Having to use these tricks all the time is a big bummer and makes the tests
harder to read. The author of Mockpp either chose to ignore this problem, or
wasn't aware of it, but I don't think that's a good solution.

To give the user a satisfying experience, I believe we should provide automatic
conversions between matchers of "compatible" types when it makes sense (i.e. we
should establish a sub-typing relation between matcher types). The challenges
are:

1.  Deciding when "it makes sense",
1.  Making sure the conversions are neither too restricted nor too liberal,
1.  Making it possible for the user to understand the compiler errors when
    automatic conversions cannot be done,
1.  Making the rules easy to learn and remember, and
1.  Implementing it.

#### Covariant or Contravariant?

We already know that making the matchers **invariant** (i.e. no auto-conversion
between matcher types) doesn't work, but what about **covariant** (`Matcher<A>`
can be used as `Matcher<B>` as long as `A` can be used as `B`) or
**contravariant** (`Matcher<A>` can be used as `Matcher<B>` as long as `B` can
be used as `A`)? Would one of them work?

It's easy to see that covariance doesn't work in general, as it requires a
matcher expecting a sub-type value to inspect a super-type value. What if the
matcher needs to look at a field that's only present in the sub-type?

On the surface level, it seems that contravariance is what we need: if type `B`
can be implicitly converted to type `A`, then given an argument of type `B`, we
can convert it to type `A` and then ask a `Matcher<A>` whether the converted
value matches. This means that we can use a `Matcher<A>` in place of a
`Matcher<B>`.

For example, given a class hierarchy and some (real and mock) functions that use
the classes:

```cpp
class Father { ... };

class Son : public Father {
 public:
  ...
  // New method not present in Father:
  virtual bool PropertyFoo() const;
};

class Grandson : public Son { ... };

void InviteFather(Father* obj);
void InviteFather(Matcher<Father*>* m);

void InviteGrandson(Grandson* obj);
void InviteGrandson(Matcher<Grandson*>* m);
```

we can expect to write the following:

```cpp
// Matches if the argument's PropertyFoo() method returns true.
Matcher<Son*>* HasFoo() { ... }

// The compiler should reject this as a Father object doesn't have
// the PropertyFoo() method:
//
//  InviteFather(HasFoo());

// This should work, as a Grandson object is also a Son object and
// has the PropertyFoo() method.
InviteGrandson(HasFoo());
```

In the latter case, the actual argument (of type `Grandson*`) will be implicitly
converted to a `Son*` and then passed to the matcher.

However, things are not always so simple. Take the example of the equality
matcher, `Func5()`, and `Func6()` we saw earlier:

```cpp
// Matches if the argument equals the given value x.
Matcher<T>* Eq(T x);

void Func5(char* a);
void Func5(Matcher<char*>* a);

void Func6(const char* a);
void Func6(Matcher<const char*>* a);
```

By making matchers contravariant, we have

```cpp
  // Remember that a char* can be implicitly converted to a const char*.

  Func5(Eq("a"));  // Compiles, but we DON'T want it to!!!  The user is
                         // trying to say that the actual argument (a char*)
                         // can be "a", and we should catch this error.

  char* p = ...;
  Func6(Eq(p));  // Still a compiler ERROR, as a const char* cannot be
                         // implicitly converted to a char*, which Eq(p) expects.
```

Clearly this isn't what we want. In fact, we want `Eq(value)` to be covariant:

```cpp
  char* p = ...;

  Func5(p);
  Func5(Eq(p));
  // Func5("a");        // The compiler will reject this,
  // Func5(Eq("a"));  // and thus should reject this too.

  Func6("a");
  Func6(Eq("a"));
  Func6(p);           // The compiler accepts this,
  Func6(Eq(p));      // and thus should accept this too.
```

In another example:

```cpp
void InviteSon(Son* obj);
void InviteSon(Matcher<Son*> m);

Father* father = ...;
Grandson* grandson = ...;

InviteSon(grandson);         // The compiler accepts this,
InviteSon(Eq(grandson));    // and thus should accept this too.

// InviteSon(father);       // The compiler will reject this,
// InviteSon(Eq(father));  // and thus should reject this too.
```

So, what was the problem? The key insight here is that *one size doesn't fit
all*. While some matchers should be contravariant (like `HasFoo()`), some should
be covariant (like `Eq(value)` and `Same(object)`). *The decision has to be left
to the individual matcher authors. gMock should not impose a global policy on
all the matchers.*

#### Implementing Automatic Type Conversion

In C++, you have several options if you want to make one class `A` act like
another class `B`:

1.  Derive `A` from `B`;
1.  In class `B`, define a public single-parameter constructor `B::B(const A&)`
    (don't make it `explicit`);
1.  In class `A`, define a type-conversion operator for type `B`.

Each of these approaches has its limitations:

*   #1 is most straightforward and requires the least amount of work. However,
    it means that an `A` object will contain all the members of `B`. It may not
    work for you if you want to be able to auto-convert `A` to multiple classes,
    and it certainly won't work if you want to convert `A` to an infinite number
    of other classes.
*   #2 requires you to be able to edit the implementation of `B`. This is not
    always possible, for example, when `B` is a class defined in a standard
    library and you are a user of that library. It's a closed approach, meaning
    that only the owner of `B` can decide which classes can be converted to `B`.
*   #3 requires more work to implement typically, but doesn't have the problems
    of #1 and #2. In particular, you can define a template type-conversion
    operator to convert `A` to an infinite number of other classes of your
    choice.

Also, remember that the compiler will only automatically insert one level of
type conversion on your behalf. For example, if `A` can be implicitly converted
to `B`, which can be implicitly converted to `C`, and you have a function
expecting a `C`, you cannot give the function an `A` without explicit casting,
unless `A` can be implicitly converted to `C` too.

gMock defines the `Matcher<T>` interface, which a user cannot modify. When
defining a polymorphic matcher (e.g. `Eq(value)`), a user needs to make it
behave like a (potentially infinite) number of matchers of different types. This
means that the last implementation technique should be used.

### Comparison with Mockpp and jMock

See GMockVsMockppAndJMock.

## Project

This design doc describes the project "[gMock](http://p?p=6164) - a framework
for writing C++ mock classes" in PDB.

## Code Location

This is a new project, so no existing file is expected to be touched. The
implementation is added in directory `//depot/google3/testing/base`.

## Group Members

ZhanyongWan : spending 60% of his time on this.

## Caveats

We considered existing solutions, but don't think they would work well enough
for Google. For details, please refer to MockppStudy.

TODO:

*   Explain why we pick the EDSL-style syntax.

## Documentation Plan

In additional to this design doc, a user's guide, an FAQ, and a codelab will
also be written.

## Testing Plan

gMock will be thoroughly tested on Linux using gUnit.

## Work Estimates

*   **Inspecting existing solutions:** The goal is to understand how other
    people have approached this problem, what they did well, and what did
    poorly. In addition to studying solutions for C++ (mockpp), we will also
    study solutions for Java (jMock and EasyMock). **3 weeks.**
*   **Initial design and prototyping:** Come up with a design tailored to C++'s
    specifics and Google's unique requirements, taking into account lessons
    learned from other solutions. **3 weeks.**
*   **Soliciting feedback on the design:** Listen to `testing-technology`,
    `testing-grouplet`, `c-users`, `c-mock-dev`, and `gunit-users`; revise the
    design based on the feedback. **3 weeks.**
*   **Initial implementation:** **6 weeks.**
*   **Documentation:** Write the user's guide and a codelab. **3 weeks.**
*   **Company-wide roll-out:** Implement a process for promoting and tracking
    adoption. **1 week.**
*   **Customer support, incremental improvements, and maintenance:** **On-going
    effort.**

## Potential Patents

We'll know whether there will be patentable inventions when we have a more
concrete design and prototype. At that time, we should talk to Google's
[patent counsel](mailto:patents@google.com).

## Things that Don't Apply

### Security Considerations

This is an internal library for writing (unit) tests, and will not be used in
production. Therefore there is no security concern.

### Privacy Considerations

gMock will not touch any user data. Therefore there is no concern about user
privacy.

### Standards

There is no existing standard in creating C++ mocks.

### Logging Plan

This is an internal library and will not handle any user request. Therefore
there is no plan for logging.

### Monitoring Plan

This is an internal library and will not run on our production servers.
Therefore no monitoring is required.

### Internationalization Plan

gMock is not visible to external users, so there is no plan to internationalize
it.

### Billing Plan and Tax Plan

gMock is an internal library and doesn't involve money, so there is no billing
plan or tax plan.

### Launch Plans

gMock will not launch externally as a Google property. However, we may later
decide to open source it.

## References

*   [jMock 1 cheat sheet](https://wiki.corp.google.com/twiki/bin/view/Main/JMockOneCheatSheet) -
    I compiled this from the downloaded jMock 1 source code.
*   [jMock 1 JavaDoc](http://www.corp.google.com/~wan/jmock1/javadoc/) - I built
    this locally. The one on http://jmock.org can be slow and may change.
*   [jMock 2 cheat sheet](http://www.jmock.org/cheat-sheet.html) - as found on
    http://jmock.org.
*   [jMock 2 cheat sheet](https://wiki.corp.google.com/twiki/bin/view/Main/JMockTwoCheatSheet) -
    I compiled this from the jMock 2 source code in CVS as of 3/21.
*   [jMock 2 JavaDoc](http://www.corp.google.com/~wan/jmock2/javadoc) - I
    generated this locally from the jMock 2 source code in CVS as of 3/21. No
    official jMock 2 JavaDoc is available yet, as the library hasn't been
    released.
*   [mockpp cheat sheet](https://wiki.corp.google.com/twiki/bin/view/Main/MockppCheatSheet) -
    I compiled this from the mockpp source code.
*   [mockpp API docs](http://mockpp.sourceforge.net/api-doc/index.html) -
    external.

## Acknowledgments

This design doc contains many ideas from PiotrKaminski. We'd also like to thank
the following people for contributing ideas to this project:

JoeWalnes, CraigSilverstein, JeffreyYasskin, KeithRay, MikeBland.
