# Mocking Reference

This page lists the facilities provided by GoogleTest for creating and working
with mock objects. To use them, include the header
`gmock/gmock.h`.

## Macros {#macros}

GoogleTest defines the following macros for working with mocks.

### MOCK_METHOD {#MOCK_METHOD}

`MOCK_METHOD(`*`return_type`*`,`*`method_name`*`, (`*`args...`*`));` \
`MOCK_METHOD(`*`return_type`*`,`*`method_name`*`, (`*`args...`*`),
(`*`specs...`*`));`

Defines a mock method *`method_name`* with arguments `(`*`args...`*`)` and
return type *`return_type`* within a mock class.

The parameters of `MOCK_METHOD` mirror the method declaration. The optional
fourth parameter *`specs...`* is a comma-separated list of qualifiers. The
following qualifiers are accepted:

| Qualifier                  | Meaning                                      |
| -------------------------- | -------------------------------------------- |
| `const`                    | Makes the mocked method a `const` method. Required if overriding a `const` method. |
| `override`                 | Marks the method with `override`. Recommended if overriding a `virtual` method. |
| `noexcept`                 | Marks the method with `noexcept`. Required if overriding a `noexcept` method. |
| `Calltype(`*`calltype`*`)` | Sets the call type for the method, for example `Calltype(STDMETHODCALLTYPE)`. Useful on Windows. |
| `ref(`*`qualifier`*`)`     | Marks the method with the given reference qualifier, for example `ref(&)` or `ref(&&)`. Required if overriding a method that has a reference qualifier. |

Note that commas in arguments prevent `MOCK_METHOD` from parsing the arguments
correctly if they are not appropriately surrounded by parentheses. See the
following example:

```cpp
class MyMock {
 public:
  // The following 2 lines will not compile due to commas in the arguments:
  MOCK_METHOD(std::pair<bool, int>, GetPair, ());              // Error!
  MOCK_METHOD(bool, CheckMap, (std::map<int, double>, bool));  // Error!

  // One solution - wrap arguments that contain commas in parentheses:
  MOCK_METHOD((std::pair<bool, int>), GetPair, ());
  MOCK_METHOD(bool, CheckMap, ((std::map<int, double>), bool));

  // Another solution - use type aliases:
  using BoolAndInt = std::pair<bool, int>;
  MOCK_METHOD(BoolAndInt, GetPair, ());
  using MapIntDouble = std::map<int, double>;
  MOCK_METHOD(bool, CheckMap, (MapIntDouble, bool));
};
```

`MOCK_METHOD` must be used in the `public:` section of a mock class definition,
regardless of whether the method being mocked is `public`, `protected`, or
`private` in the base class.

### EXPECT_CALL {#EXPECT_CALL}

`EXPECT_CALL(`*`mock_object`*`,`*`method_name`*`(`*`matchers...`*`))`

Creates an [expectation](../gmock_for_dummies.md#setting-expectations) that the
method *`method_name`* of the object *`mock_object`* is called with arguments
that match the given matchers *`matchers...`*. `EXPECT_CALL` must precede any
code that exercises the mock object.

The parameter *`matchers...`* is a comma-separated list of
[matchers](../gmock_for_dummies.md#matchers-what-arguments-do-we-expect) that
correspond to each argument of the method *`method_name`*. The expectation will
apply only to calls of *`method_name`* whose arguments match all of the
matchers. If `(`*`matchers...`*`)` is omitted, the expectation behaves as if
each argument's matcher were a [wildcard matcher (`_`)](matchers.md#wildcard).
See the [Matchers Reference](matchers.md) for a list of all built-in matchers.

The following chainable clauses can be used to modify the expectation, and they
must be used in the following order:

```cpp
EXPECT_CALL(mock_object, method_name(matchers...))
    .With(multi_argument_matcher)  // Can be used at most once
    .Times(cardinality)            // Can be used at most once
    .InSequence(sequences...)      // Can be used any number of times
    .After(expectations...)        // Can be used any number of times
    .WillOnce(action)              // Can be used any number of times
    .WillRepeatedly(action)        // Can be used at most once
    .RetiresOnSaturation();        // Can be used at most once
```

See details for each modifier clause below.

#### With {#EXPECT_CALL.With}

`.With(`*`multi_argument_matcher`*`)`

Restricts the expectation to apply only to mock function calls whose arguments
as a whole match the multi-argument matcher *`multi_argument_matcher`*.

GoogleTest passes all of the arguments as one tuple into the matcher. The
parameter *`multi_argument_matcher`* must thus be a matcher of type
`Matcher<std::tuple<A1, ..., An>>`, where `A1, ..., An` are the types of the
function arguments.

For example, the following code sets the expectation that
`my_mock.SetPosition()` is called with any two arguments, the first argument
being less than the second:

```cpp
using ::testing::_;
using ::testing::Lt;
...
EXPECT_CALL(my_mock, SetPosition(_, _))
    .With(Lt());
```

GoogleTest provides some built-in matchers for 2-tuples, including the `Lt()`
matcher above. See [Multi-argument Matchers](matchers.md#MultiArgMatchers).

The `With` clause can be used at most once on an expectation and must be the
first clause.

#### Times {#EXPECT_CALL.Times}

`.Times(`*`cardinality`*`)`

Specifies how many times the mock function call is expected.

The parameter *`cardinality`* represents the number of expected calls and can be
one of the following, all defined in the `::testing` namespace:

| Cardinality         | Meaning                                             |
| ------------------- | --------------------------------------------------- |
| `AnyNumber()`       | The function can be called any number of times.     |
| `AtLeast(n)`        | The function call is expected at least *n* times.   |
| `AtMost(n)`         | The function call is expected at most *n* times.    |
| `Between(m, n)`     | The function call is expected between *m* and *n* times, inclusive. |
| `Exactly(n)` or `n` | The function call is expected exactly *n* times. If *n* is 0, the call should never happen. |

If the `Times` clause is omitted, GoogleTest infers the cardinality as follows:

*   If neither [`WillOnce`](#EXPECT_CALL.WillOnce) nor
    [`WillRepeatedly`](#EXPECT_CALL.WillRepeatedly) are specified, the inferred
    cardinality is `Times(1)`.
*   If there are *n* `WillOnce` clauses and no `WillRepeatedly` clause, where
    *n* >= 1, the inferred cardinality is `Times(n)`.
*   If there are *n* `WillOnce` clauses and one `WillRepeatedly` clause, where
    *n* >= 0, the inferred cardinality is `Times(AtLeast(n))`.

The `Times` clause can be used at most once on an expectation.

#### InSequence {#EXPECT_CALL.InSequence}

`.InSequence(`*`sequences...`*`)`

Specifies that the mock function call is expected in a certain sequence.

The parameter *`sequences...`* is any number of [`Sequence`](#Sequence) objects.
Expected calls assigned to the same sequence are expected to occur in the order
the expectations are declared.

For example, the following code sets the expectation that the `Reset()` method
of `my_mock` is called before both `GetSize()` and `Describe()`, and `GetSize()`
and `Describe()` can occur in any order relative to each other:

```cpp
using ::testing::Sequence;
Sequence s1, s2;
...
EXPECT_CALL(my_mock, Reset())
    .InSequence(s1, s2);
EXPECT_CALL(my_mock, GetSize())
    .InSequence(s1);
EXPECT_CALL(my_mock, Describe())
    .InSequence(s2);
```

The `InSequence` clause can be used any number of times on an expectation.

See also the [`InSequence` class](#InSequence).

#### After {#EXPECT_CALL.After}

`.After(`*`expectations...`*`)`

Specifies that the mock function call is expected to occur after one or more
other calls.

The parameter *`expectations...`* can be up to five
[`Expectation`](#Expectation) or [`ExpectationSet`](#ExpectationSet) objects.
The mock function call is expected to occur after all of the given expectations.

For example, the following code sets the expectation that the `Describe()`
method of `my_mock` is called only after both `InitX()` and `InitY()` have been
called.

```cpp
using ::testing::Expectation;
...
Expectation init_x = EXPECT_CALL(my_mock, InitX());
Expectation init_y = EXPECT_CALL(my_mock, InitY());
EXPECT_CALL(my_mock, Describe())
    .After(init_x, init_y);
```

The `ExpectationSet` object is helpful when the number of prerequisites for an
expectation is large or variable, for example:

```cpp
using ::testing::ExpectationSet;
...
ExpectationSet all_inits;
// Collect all expectations of InitElement() calls
for (int i = 0; i < element_count; i++) {
  all_inits += EXPECT_CALL(my_mock, InitElement(i));
}
EXPECT_CALL(my_mock, Describe())
    .After(all_inits);  // Expect Describe() call after all InitElement() calls
```

The `After` clause can be used any number of times on an expectation.

#### WillOnce {#EXPECT_CALL.WillOnce}

`.WillOnce(`*`action`*`)`

Specifies the mock function's actual behavior when invoked, for a single
matching function call.

The parameter *`action`* represents the
[action](../gmock_for_dummies.md#actions-what-should-it-do) that the function
call will perform. See the [Actions Reference](actions.md) for a list of
built-in actions.

The use of `WillOnce` implicitly sets a cardinality on the expectation when
`Times` is not specified. See [`Times`](#EXPECT_CALL.Times).

Each matching function call will perform the next action in the order declared.
For example, the following code specifies that `my_mock.GetNumber()` is expected
to be called exactly 3 times and will return `1`, `2`, and `3` respectively on
the first, second, and third calls:

```cpp
using ::testing::Return;
...
EXPECT_CALL(my_mock, GetNumber())
    .WillOnce(Return(1))
    .WillOnce(Return(2))
    .WillOnce(Return(3));
```

The `WillOnce` clause can be used any number of times on an expectation. Unlike
`WillRepeatedly`, the action fed to each `WillOnce` call will be called at most
once, so may be a move-only type and/or have an `&&`-qualified call operator.

#### WillRepeatedly {#EXPECT_CALL.WillRepeatedly}

`.WillRepeatedly(`*`action`*`)`

Specifies the mock function's actual behavior when invoked, for all subsequent
matching function calls. Takes effect after the actions specified in the
[`WillOnce`](#EXPECT_CALL.WillOnce) clauses, if any, have been performed.

The parameter *`action`* represents the
[action](../gmock_for_dummies.md#actions-what-should-it-do) that the function
call will perform. See the [Actions Reference](actions.md) for a list of
built-in actions.

The use of `WillRepeatedly` implicitly sets a cardinality on the expectation
when `Times` is not specified. See [`Times`](#EXPECT_CALL.Times).

If any `WillOnce` clauses have been specified, matching function calls will
perform those actions before the action specified by `WillRepeatedly`. See the
following example:

```cpp
using ::testing::Return;
...
EXPECT_CALL(my_mock, GetName())
    .WillRepeatedly(Return("John Doe"));  // Return "John Doe" on all calls

EXPECT_CALL(my_mock, GetNumber())
    .WillOnce(Return(42))        // Return 42 on the first call
    .WillRepeatedly(Return(7));  // Return 7 on all subsequent calls
```

The `WillRepeatedly` clause can be used at most once on an expectation.

#### RetiresOnSaturation {#EXPECT_CALL.RetiresOnSaturation}

`.RetiresOnSaturation()`

Indicates that the expectation will no longer be active after the expected
number of matching function calls has been reached.

The `RetiresOnSaturation` clause is only meaningful for expectations with an
upper-bounded cardinality. The expectation will *retire* (no longer match any
function calls) after it has been *saturated* (the upper bound has been
reached). See the following example:

```cpp
using ::testing::_;
using ::testing::AnyNumber;
...
EXPECT_CALL(my_mock, SetNumber(_))  // Expectation 1
    .Times(AnyNumber());
EXPECT_CALL(my_mock, SetNumber(7))  // Expectation 2
    .Times(2)
    .RetiresOnSaturation();
```

In the above example, the first two calls to `my_mock.SetNumber(7)` match
expectation 2, which then becomes inactive and no longer matches any calls. A
third call to `my_mock.SetNumber(7)` would then match expectation 1. Without
`RetiresOnSaturation()` on expectation 2, a third call to `my_mock.SetNumber(7)`
would match expectation 2 again, producing a failure since the limit of 2 calls
was exceeded.

The `RetiresOnSaturation` clause can be used at most once on an expectation and
must be the last clause.

### ON_CALL {#ON_CALL}

`ON_CALL(`*`mock_object`*`,`*`method_name`*`(`*`matchers...`*`))`

Defines what happens when the method *`method_name`* of the object
*`mock_object`* is called with arguments that match the given matchers
*`matchers...`*. Requires a modifier clause to specify the method's behavior.
*Does not* set any expectations that the method will be called.

The parameter *`matchers...`* is a comma-separated list of
[matchers](../gmock_for_dummies.md#matchers-what-arguments-do-we-expect) that
correspond to each argument of the method *`method_name`*. The `ON_CALL`
specification will apply only to calls of *`method_name`* whose arguments match
all of the matchers. If `(`*`matchers...`*`)` is omitted, the behavior is as if
each argument's matcher were a [wildcard matcher (`_`)](matchers.md#wildcard).
See the [Matchers Reference](matchers.md) for a list of all built-in matchers.

The following chainable clauses can be used to set the method's behavior, and
they must be used in the following order:

```cpp
ON_CALL(mock_object, method_name(matchers...))
    .With(multi_argument_matcher)  // Can be used at most once
    .WillByDefault(action);        // Required
```

See details for each modifier clause below.

#### With {#ON_CALL.With}

`.With(`*`multi_argument_matcher`*`)`

Restricts the specification to only mock function calls whose arguments as a
whole match the multi-argument matcher *`multi_argument_matcher`*.

GoogleTest passes all of the arguments as one tuple into the matcher. The
parameter *`multi_argument_matcher`* must thus be a matcher of type
`Matcher<std::tuple<A1, ..., An>>`, where `A1, ..., An` are the types of the
function arguments.

For example, the following code sets the default behavior when
`my_mock.SetPosition()` is called with any two arguments, the first argument
being less than the second:

```cpp
using ::testing::_;
using ::testing::Lt;
using ::testing::Return;
...
ON_CALL(my_mock, SetPosition(_, _))
    .With(Lt())
    .WillByDefault(Return(true));
```

GoogleTest provides some built-in matchers for 2-tuples, including the `Lt()`
matcher above. See [Multi-argument Matchers](matchers.md#MultiArgMatchers).

The `With` clause can be used at most once with each `ON_CALL` statement.

#### WillByDefault {#ON_CALL.WillByDefault}

`.WillByDefault(`*`action`*`)`

Specifies the default behavior of a matching mock function call.

The parameter *`action`* represents the
[action](../gmock_for_dummies.md#actions-what-should-it-do) that the function
call will perform. See the [Actions Reference](actions.md) for a list of
built-in actions.

For example, the following code specifies that by default, a call to
`my_mock.Greet()` will return `"hello"`:

```cpp
using ::testing::Return;
...
ON_CALL(my_mock, Greet())
    .WillByDefault(Return("hello"));
```

The action specified by `WillByDefault` is superseded by the actions specified
on a matching `EXPECT_CALL` statement, if any. See the
[`WillOnce`](#EXPECT_CALL.WillOnce) and
[`WillRepeatedly`](#EXPECT_CALL.WillRepeatedly) clauses of `EXPECT_CALL`.

The `WillByDefault` clause must be used exactly once with each `ON_CALL`
statement.

## Classes {#classes}

GoogleTest defines the following classes for working with mocks.

### DefaultValue {#DefaultValue}

`::testing::DefaultValue<T>`

Allows a user to specify the default value for a type `T` that is both copyable
and publicly destructible (i.e. anything that can be used as a function return
type). For mock functions with a return type of `T`, this default value is
returned from function calls that do not specify an action.

Provides the static methods `Set()`, `SetFactory()`, and `Clear()` to manage the
default value:

```cpp
// Sets the default value to be returned. T must be copy constructible.
DefaultValue<T>::Set(value);

// Sets a factory. Will be invoked on demand. T must be move constructible.
T MakeT();
DefaultValue<T>::SetFactory(&MakeT);

// Unsets the default value.
DefaultValue<T>::Clear();
```

### NiceMock {#NiceMock}

`::testing::NiceMock<T>`

Represents a mock object that suppresses warnings on
[uninteresting calls](../gmock_cook_book.md#uninteresting-vs-unexpected). The
template parameter `T` is any mock class, except for another `NiceMock`,
`NaggyMock`, or `StrictMock`.

Usage of `NiceMock<T>` is analogous to usage of `T`. `NiceMock<T>` is a subclass
of `T`, so it can be used wherever an object of type `T` is accepted. In
addition, `NiceMock<T>` can be constructed with any arguments that a constructor
of `T` accepts.

For example, the following code suppresses warnings on the mock `my_mock` of
type `MockClass` if a method other than `DoSomething()` is called:

```cpp
using ::testing::NiceMock;
...
NiceMock<MockClass> my_mock("some", "args");
EXPECT_CALL(my_mock, DoSomething());
... code that uses my_mock ...
```

`NiceMock<T>` only works for mock methods defined using the `MOCK_METHOD` macro
directly in the definition of class `T`. If a mock method is defined in a base
class of `T`, a warning might still be generated.

`NiceMock<T>` might not work correctly if the destructor of `T` is not virtual.

### NaggyMock {#NaggyMock}

`::testing::NaggyMock<T>`

Represents a mock object that generates warnings on
[uninteresting calls](../gmock_cook_book.md#uninteresting-vs-unexpected). The
template parameter `T` is any mock class, except for another `NiceMock`,
`NaggyMock`, or `StrictMock`.

Usage of `NaggyMock<T>` is analogous to usage of `T`. `NaggyMock<T>` is a
subclass of `T`, so it can be used wherever an object of type `T` is accepted.
In addition, `NaggyMock<T>` can be constructed with any arguments that a
constructor of `T` accepts.

For example, the following code generates warnings on the mock `my_mock` of type
`MockClass` if a method other than `DoSomething()` is called:

```cpp
using ::testing::NaggyMock;
...
NaggyMock<MockClass> my_mock("some", "args");
EXPECT_CALL(my_mock, DoSomething());
... code that uses my_mock ...
```

Mock objects of type `T` by default behave the same way as `NaggyMock<T>`.

### StrictMock {#StrictMock}

`::testing::StrictMock<T>`

Represents a mock object that generates test failures on
[uninteresting calls](../gmock_cook_book.md#uninteresting-vs-unexpected). The
template parameter `T` is any mock class, except for another `NiceMock`,
`NaggyMock`, or `StrictMock`.

Usage of `StrictMock<T>` is analogous to usage of `T`. `StrictMock<T>` is a
subclass of `T`, so it can be used wherever an object of type `T` is accepted.
In addition, `StrictMock<T>` can be constructed with any arguments that a
constructor of `T` accepts.

For example, the following code generates a test failure on the mock `my_mock`
of type `MockClass` if a method other than `DoSomething()` is called:

```cpp
using ::testing::StrictMock;
...
StrictMock<MockClass> my_mock("some", "args");
EXPECT_CALL(my_mock, DoSomething());
... code that uses my_mock ...
```

`StrictMock<T>` only works for mock methods defined using the `MOCK_METHOD`
macro directly in the definition of class `T`. If a mock method is defined in a
base class of `T`, a failure might not be generated.

`StrictMock<T>` might not work correctly if the destructor of `T` is not
virtual.

### Sequence {#Sequence}

`::testing::Sequence`

Represents a chronological sequence of expectations. See the
[`InSequence`](#EXPECT_CALL.InSequence) clause of `EXPECT_CALL` for usage.

### InSequence {#InSequence}

`::testing::InSequence`

An object of this type causes all expectations encountered in its scope to be
put in an anonymous sequence.

This allows more convenient expression of multiple expectations in a single
sequence:

```cpp
using ::testing::InSequence;
{
  InSequence seq;

  // The following are expected to occur in the order declared.
  EXPECT_CALL(...);
  EXPECT_CALL(...);
  ...
  EXPECT_CALL(...);
}
```

The name of the `InSequence` object does not matter.

### Expectation {#Expectation}

`::testing::Expectation`

Represents a mock function call expectation as created by
[`EXPECT_CALL`](#EXPECT_CALL):

```cpp
using ::testing::Expectation;
Expectation my_expectation = EXPECT_CALL(...);
```

Useful for specifying sequences of expectations; see the
[`After`](#EXPECT_CALL.After) clause of `EXPECT_CALL`.

### ExpectationSet {#ExpectationSet}

`::testing::ExpectationSet`

Represents a set of mock function call expectations.

Use the `+=` operator to add [`Expectation`](#Expectation) objects to the set:

```cpp
using ::testing::ExpectationSet;
ExpectationSet my_expectations;
my_expectations += EXPECT_CALL(...);
```

Useful for specifying sequences of expectations; see the
[`After`](#EXPECT_CALL.After) clause of `EXPECT_CALL`.
