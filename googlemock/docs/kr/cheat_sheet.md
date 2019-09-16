## gMock Cheat Sheet

### Mock Class 정의하기

#### 일반 Class Mocking하기

아래에 `Foo`라는 interface가 있습니다. (소멸자 `~Foo()`는 반드시 `virtual`로 선언해야 합니다.)
```cpp
class Foo {
  ...
  virtual ~Foo();
  virtual int GetSize() const = 0;
  virtual string Describe(const char* name) = 0;
  virtual string Describe(int type) = 0;
  virtual bool Process(Bar elem, int count) = 0;
};
```
`Foo` interface는 아래와 같이 mocking 할 수 있습니다.

```cpp
#include "gmock/gmock.h"

class MockFoo : public Foo {
  ...
  MOCK_METHOD(int, GetSize, (), (const, override));
  MOCK_METHOD(string, Describe, (const char* name), (override));
  MOCK_METHOD(string, Describe, (int type), (override));
  MOCK_METHOD(bool, Process, (Bar elem, int count), (override));
};
```

이 때, uninteresting call을 무시하려면 "nice" mock object로 생성하면 됩니다. 반대로 모든 uninteresting call을 failure로 처리하려면 "strict" mock object로 생성하면 됩니다.

```cpp
using ::testing::NiceMock;
using ::testing::NaggyMock;
using ::testing::StrictMock;

NiceMock<MockFoo> nice_foo;      // The type is a subclass of MockFoo.
NaggyMock<MockFoo> naggy_foo;    // The type is a subclass of MockFoo.
StrictMock<MockFoo> strict_foo;  // The type is a subclass of MockFoo.
```

#### Class Template Mocking하기

아래에 `StackInterface`라는 interface가 있습니다. (소멸자 `~StackInterface()`는 반드시 `virtual`로 선언해야 합니다.)
```cpp
template <typename Elem>
class StackInterface {
  ...
  virtual ~StackInterface();
  virtual int GetSize() const = 0;
  virtual void Push(const Elem& x) = 0;
};
```
`StackInterface`는 아래와 같이 mocking할 수 있습니다.

```cpp
template <typename Elem>
class MockStack : public StackInterface<Elem> {
  ...
  MOCK_METHOD(int, GetSize, (), (const, override));
  MOCK_METHOD(void, Push, (const Elem& x), (override));
};
```

#### Mock Function의 호출방식 명세하기

만약 mocking 대상이 기본적인 호출방식을 사용하지 않는다면, 이를 gMock에 알려줘야 합니다. 예를 들어 Windows의 `STDMETHODCALLTYPE`같은 경우에는 `ULONG STDMETHODCALLTYPE AddRef()`과 같은 형태로 function이 선언됩니다. 사용자는 gMock이 이러한 내용을 알 수 있도록 `MOCK_METHOD`의 4번째 parameter를 통해 관련내용을 전달할 수 있습니다.

```cpp
  MOCK_METHOD(bool, Foo, (int n), (Calltype(STDMETHODCALLTYPE)));
  MOCK_METHOD(int, Bar, (double x, double y),
              (const, Calltype(STDMETHODCALLTYPE)));
```
예제로 사용한 `STDMETHODCALLTYPE`은 Windows의 `<objbase.h>`파일에 정의되어 있습니다.

### Mock 사용하기

이제 mock을 사용할 차례입니다. Mock을 사용하는 일반적인 순서는 아래와 같습니다.

1. `using`을 통해서 사용하려는 gMock 기능을 추가하세요. 예외적인 경우를 제외하면 거의 모든 기능은 `testing` namespace에 정의되어 있습니다.
2. Mock object를 생성합니다.
3. (선택사항) Mock object의 default action을 설정합니다.
4. Mock object에 대한 expectation을 지정합니다. (어떤 방식으로 호출 될 것인지, 무슨일을 할 것인지 등)
5. Mock object를 사용하는 실제 코드를 수행합니다. 필요한 경우 googletest assertion을 함께 사용합니다.
6. Mock object가 소멸될 때, 설정된 모든 expectation들이 만족되었는지 확인합니다.

위의 6단계 순서를 아래 예제코드에서 확인할 수 있습니다.
```cpp
using ::testing::Return;                          // #1

TEST(BarTest, DoesThis) {
  MockFoo foo;                                    // #2

  ON_CALL(foo, GetSize())                         // #3
      .WillByDefault(Return(1));
  // ... other default actions ...

  EXPECT_CALL(foo, Describe(5))                   // #4
      .Times(3)
      .WillRepeatedly(Return("Category 5"));
  // ... other expectations ...

  EXPECT_EQ("good", MyProductionFunction(&foo));  // #5
}                                                 // #6
```

### Default Action 설정하기

gMock은 `void`, `bool`, 숫자형, 포인터 등을 return type으로 하는 function에 대해서는 **built-in default action**을 기본적으로 제공하고 있습니다.

다음으로 임의의 타입 `T`에 대한 default action을 전역적으로 변경하려면 아래 기능을 사용하면 됩니다.

```cpp
using ::testing::DefaultValue;

// Sets the default value to be returned. T must be CopyConstructible.
DefaultValue<T>::Set(value);
// Sets a factory. Will be invoked on demand. T must be MoveConstructible.
//  T MakeT();
DefaultValue<T>::SetFactory(&MakeT);
// ... use the mocks ...
// Resets the default value.
DefaultValue<T>::Clear();
```

위에서 설정한 default action을 사용하는 코드는 아래와 같습니다.

```c++
  // Sets the default action for return type std::unique_ptr<Buzz> to
  // creating a new Buzz every time.
  DefaultValue<std::unique_ptr<Buzz>>::SetFactory(
      [] { return MakeUnique<Buzz>(AccessLevel::kInternal); });

  // When this fires, the default action of MakeBuzz() will run, which
  // will return a new Buzz object.
  EXPECT_CALL(mock_buzzer_, MakeBuzz("hello")).Times(AnyNumber());

  auto buzz1 = mock_buzzer_.MakeBuzz("hello");
  auto buzz2 = mock_buzzer_.MakeBuzz("hello");
  EXPECT_NE(nullptr, buzz1);
  EXPECT_NE(nullptr, buzz2);
  EXPECT_NE(buzz1, buzz2);

  // Resets the default action for return type std::unique_ptr<Buzz>,
  // to avoid interfere with other tests.
  DefaultValue<std::unique_ptr<Buzz>>::Clear();
```

특정 method의 default action만 변경하려면 `ON_CALL()`을 사용하면 됩니다. `ON_CALL()`은`EXPECT_CALL()`과 사용방법이 비슷하며 default behavior를 변경하기 위한 목적으로 사용합니다. 더 자세한 정보는 [여기](cook_book.md#default-action을-상황에-따라-변화시키기)를 참조하세요.

```cpp
ON_CALL(mock-object, method(matchers))
    .With(multi-argument-matcher)   ?
    .WillByDefault(action);
```

### Expectation 설정하기

Mock method에 **expectation**을 설정할때는 `EXPECT_CALL()`을 사용합니다. Expectation이란 해당 mock method가 어떤 방식으로 호출될 것인지 그리고 무슨일을 할 것인지 등을 지정하는 것입니다.

```cpp
EXPECT_CALL(mock-object, method (matchers)?)
     .With(multi-argument-matcher)  ?
     .Times(cardinality)            ?
     .InSequence(sequences)         *
     .After(expectations)           *
     .WillOnce(action)              *
     .WillRepeatedly(action)        ?
     .RetiresOnSaturation();        ?
```

만약 `(matchers)` 부분이 없다면 어떤 argument가 와도 괜찮다는 의미의 `(_,_,_,_)`와 동일하게 동작합니다.

위에서 `Times()`를 설정하지 않은 경우에는 아래와 같이 호출횟수가 결정됩니다.

- `WillOnce()`와 `WillRepeatedly()` 둘 다 없는 경우, `Times(1)`로 간주됩니다.
- `WillOnce()`가 `n`개 있고 `WillRepeatedly()`가 없다면 `Times(n)`으로 간주됩니다. (`n` >=1 )
- `WillOnce()`가 `n`개 있고 `WillRepeatedly()`가 있다면 `Times(AtLeast(n))`으로 간주됩니다. (`n` >=0)

`EXPECT_CALL()`을 사용하지 않은 mock method는 *몇 번 호출되더라도 괜찮습니다.* 호출될 때마다 default action이 수행될 것입니다.

### Matchers

하나의 **Matcher**는 *하나의* argument를 비교하기 위한 목적으로 사용합니다. 주로 `ON_CALL()`이나 `EXPECT_CALL()`과 함께 사용하며, `EXPECT_THAT()`을 통해 값을 직접 검증하는 것도 가능합니다.

| Matcher | Description |
|:--------|:------------|
| `EXPECT_THAT(actual_value, matcher)` | `matcher`를 통해 `actual_value`를 검증합니다. |
| `ASSERT_THAT(actual_value, matcher)` | **fatal** failure를 발생시킨다는 점 외에는 `EXPECT_THAT(actual_value, matcher)`과 동일합니다. |

아래부터는 built-in matcher를 여러가지로 분류한 내용들이 계속됩니다. 자주 등장하는 용어인 `argument`는 mock function으로 전달되는 argument를 의미함을 기억하세요.

#### Wildcard
| Matcher | Description |
|:--------|:------------|
|`_`|타입만 문제없다면 `argument`로 어느 값이 전달돼도 괜찮습니다.|
|`A<type>()` or `An<type>()`|기본적으로 `_`와 동일하지만, `type`을 통해 `argument`의 타입을 명시적으로 지정할 수 있습니다.     |

#### Generic Comparison

| Matcher              | Description       |
|:---------------------|:------------------|
|`Eq(value)` or `value`|`argument == value`|
|`Ge(value)`           |`argument >= value`|
|`Gt(value)`           |`argument > value` |
|`Le(value)`           |`argument <= value`|
|`Lt(value)`           |`argument < value` |
|`Ne(value)`           |`argument != value`|
|`IsNull()`            |`argument`가 `NULL` 포인터이기를 기대합니다. (raw, smart 둘다가능)|
|`NotNull()`           |`argument`가 `NULL` 포인터가 아니기를 기대합니다. (raw, smart 둘다가능)|
|`Optional(m)`         |`argument`로 전달되는 `optional<>`을 matcher `m`을 통해 비교합니다.|
|`VariantWith<T>(m)`   |`argument`로 전달되는 `variant<>`를 matcher `m`을 통해 비교합니다.|
|`Ref(variable)`       |`argument`가 `variable`의 참조형태이길 기대합니다.|
|`TypedEq<type>(value)`|`argument`가 `type`이라는 타입을 가지며 그 값도 `value`와 같기를 기대합니다. 기본적으로 `Eq(value)`와 동일한 기능을 하지만, 타입지정이 가능하기 때문에 mock function이 overloaded 되었을 때 유용하게 사용할 수 있습니다.|

대부분의 matcher들은 자신이 실행되는 시점에 `value`에 대한 *복사본을 생성하고 내부적으로 보관합니다.* (`Ref()`는 제외) 이렇게 복사본을 만드는 이유는 `value`가 추후에 수정되거나 소멸될 것에 대비하기 위함입니다. 그러다가 실제로 mock function이 호출되면 위에서 만들어 놓은 복사본과 전달된 argument를 서로 비교하게 됩니다. 이런 안전장치가 있기 때문에 `EXPECT_CALL`이 수행된 이후에는 `value`가 수정되더라도 영향을 받지 않게 됩니다. 한가지 유의할 점은 copy constructor가 없는 `value`는 (당연하게도) compile error가 발생한다는 것입니다. 이 문제를 해결하기 위해서 `value`를 `ByRef()`로 감싸서 참조형식으로 전달하는 것도 가능합니다. 예를 들면 `Eq(ByRef(non_copyable_value))`와 같이 구현할 수 있습니다. 다만, 이것은 복사본을 만드는 것이 아니기 때문에 주의해서 사용하기 바랍니다. 참조형식을 사용하게 되면 `value`의 값에 따라 matcher의 동작도 달라진다는 점을 기억하세요.

#### Floating-Point Matchers

| Matcher            | Description                                                                                              |
|:-------------------|:---------------------------------------------------------------------------------------------------------|
|`DoubleEq(a_double)`|`argument`는 `double` 타입이며, 값은 `a_double`과 *거의 같습니다.* 두 개의 NaN을 비교할 때는 같지 않다고 판단합니다.           |
|`FloatEq(a_float)`  |`argument`는 `float` 타입이며, 값은 `a_float`와 *거의 같습니다.* 두 개의 NaN을 비교할 때는 같지 않다고 판단합니다.             |
|`NanSensitiveDoubleEq(a_double)`|`argument`는 `double` 타입이며, 값은 `a_double`과 *거의 같습니다.* 두 개의 NaN을 비교할 때는 같다고 판단합니다. |
|`NanSensitiveFloatEq(a_float)`|`argument`는 `float` 타입이며, 값은 `a_float`와 *거의 같습니다.* 두 개의 NaN을 비교할 때는 같다고 판단합니다.     |

위의 floating-point matcher들은 ULP-based 비교를 수행합니다. googletest의 floating-point assertion에서도 동일한 방법을 사용하고 있습니다. 이 때의 오차범위는 expected value의 절대값을 기반으로 자동 계산됩니다. 다음으로 `DoubleEq`, `FloatEq`를 사용해서 2개의 NaN를 비교한다면 IEEE 표준에 따라 `false`를 반환하도록 되어 있습니다. 만약 NaN 2개를 비교했을 때 `true`가 반환되기를 원한다면 `NanSensitive*`로 시작하는 버전을 사용하시기 바랍니다. 

| Matcher | Description |
|:--------|:------------|
|`DoubleNear(a_double, max_abs_error)`|`argument`는 `double` 타입이며, 값은 `a_double`과 *거의 같습니다.* (단, absolute error <= `max_abs_error`) 두 개의 NaN을 비교할 때는 같지 않다고 판단합니다.|
|`FloatNear(a_float, max_abs_error)`|`argument`는 `float` 타입이며, 값은 `a_float`와 *거의 같습니다.* (단, absolute error <= `max_abs_error`) 두 개의 NaN을 비교할 때는 같지 않다고 판단합니다.|
|`NanSensitiveDoubleNear(a_double, max_abs_error)`|`argument`는 `double` 타입이며, 값은 `a_double`과 *거의 같습니다.* (단, absolute error <= `max_abs_error`) 두 개의 NaN을 비교할 때는 같다고 판단합니다.|
|`NanSensitiveFloatNear(a_float, max_abs_error)`|`argument`는 `float` 타입이며, 값은 `a_float`와 *거의 같습니다.* (단, absolute error <= `max_abs_error`) 두 개의 NaN을 비교할 때는 같다고 판단합니다.|

#### String Matchers

String matcher로 전달가능한 `argument`는 C string이나 C++ string object 입니다.

| Matcher                 | Description                                                  |
| :---------------------- | :----------------------------------------------------------- |
| `ContainsRegex(string)` | `argument`가 주어진 정규식 `string`을 포함하는지 확인합니다. |
| `EndsWith(suffix)`      | `argument`가 `suffix`로 끝나는지 확인합니다.                 |
| `HasSubstr(string)`     | `argument`가 `string`을 포함하는지 확인합니다.               |
| `MatchesRegex(string)`  | `argument`가  주어진 정규식 `string`과 일치하는지 확인합니다. |
| `StartsWith(prefix)`    | `argument`가 `prefix`로 시작하는지 확인합니다.               |
| `StrCaseEq(string)`     | `argument`가 `string`과 같은지 확인합니다. 대소문자는 무시합니다. |
| `StrCaseNe(string)`     | `argument`가 `string`과 다른지 확인합니다. 대소문자는 무시합니다. |
| `StrEq(string)`         | `argument`가 `string`과 같은지 확인합니다.                   |
| `StrNe(string)`         | `argument`가 `string`과 다른지 확인합니다.                   |

`ContainsRegex()`, `MatchesRegex()`에 사용되는 정규식 문법은 [이곳](../../../googletest/docs/kr/advanced.md#정규식-문법)에 정의되어 있으며 `StrCaseEq()`, `StrCaseNe()`, `StrEq()`, `StrNe()`은 wide string 타입을 사용해도 잘 동작합니다.

#### Container Matchers

STL-style container들은 대부분 `==` 연산자를 제공하고 있습니다. 따라서 `Eq(expected_container)` 또는 `foo(expected_container)`와 같이 구현하면 별다른 조치를 취하지 않더라도 잘 동작할 것입니다. 혹시나 보다 다양한 방법으로 container를 비교하고 싶은 사용자는 아래 표를 확인하기 바랍니다. 이렇게 추가적으로 제공되는 matcher를 통해서 사용자가 처한 상황에 맞게 좀 더 유연한 방법을 선택할 수 있을 것입니다.


| Matcher | Description |
|:--------|:------------|
| `BeginEndDistanceIs(m)` | argument로 전달된 container의 `begin()`, `end()`간의 차이(*즉, container의 size*)를 matcher `m`통해 비교합니다. 예를 들어`BegintEndDistanceIs(2)` 또는 `BeginEndDistanceIs(Lt(2))`와 같이 사용하면 됩니다. 만약, 해당 container가 `size()`를 정의하고 있다면 `SizeIs(m)`를 사용해도 됩니다. 기능자체는 동일합니다. |
| `ContainerEq(container)` | 기본적인 동작은 `Eq(container)`와 유사합니다. 단, failure message에 어떤 element가 서로 다른지와 같은 추가정보를 출력해줍니다. |
| `Contains(e)` | argument로 전달된 container에 `e`를 만족하는 element가 있는지 확인합니다. 여기서 `e`는 값일 수도 있고 matcher일 수도 있습니다. |
| `Each(e)` | argument로 전달된 container의 *모든* element가 `e`를 만족해야 합니다. 여기서 `e`는 값일 수도 있고 matcher일 수도 있습니다. |
| `IsEmpty()` | argument로 전달된 container에 element가 하나도 없기를 기대합니다. (`container.empty()`) |
| `IsTrue()` | argument로 전달된 container가 `true`이기를 기대합니다. 단, container가 boolean 타입으로 사용가능한 경우만 해당됩니다. |
| `IsFalse()` | argument로 전달된 container가 `false`이기를 기대합니다. 단, container가 boolean 타입으로 사용가능한 경우만 해당됩니다. |
| `SizeIs(m)` | argument로 전달된 container의 size가 matcher `m`을 만족하는지 확인합니다. `SizeIs(2)` 또는 `SizeIs(Lt(2))`와 같이 사용할 수 있습니다. |
| `ElementsAre(e0, e1, ..., en)` | argument로 전달된 container의 element들이 `e0`, `...`, `en`을 각각 만족하는지 비교합니다. 여기서 `e`는 값일 수도 있고 matcher일 수도 있습니다. 허용되는 element 개수는 0~10개까지입니다. |
| `ElementsAreArray({ e0, e1, ..., en })`, `ElementsAreArray(a_container)`, `ElementsAreArray(begin, end)`, `ElementsAreArray(array)`, `ElementsAreArray(array, count)` | 기본적인 동작은 `ElementsAre()`과 유사합니다. 단, `e0`, `e1`과 같은 expected element를 initializer_list, STL-style container, C-style array와 같은 다양한 방법으로 전달할 수 있습니다. |
| `UnorderedElementsAre(e0, e1, ..., en)` | 기본적인 동작은 `ElementsAre()`과 유사합니다. 단, `argument`와 parameter의 element들을 순서대로 비교하지 않습니다. |
| `UnorderedElementsAreArray({ e0, e1, ..., en })`, `UnorderedElementsAreArray(a_container)`, `UnorderedElementsAreArray(begin, end)`, `UnorderedElementsAreArray(array)`, `UnorderedElementsAreArray(array, count)` | 기본적인 동작은 `UnorderedElementsAre()`과 유사합니다. 단, `e0`, `e1`과 같은 expected element를 initializer_list, STL-style container, C-style array와 같은 다양한 방법으로 전달할 수 있습니다. |
| `IsSubsetOf({e0, e1, ..., en})`, `IsSubsetOf(a_container)`, `IsSubsetOf(begin, end)`,  `IsSubsetOf(array)`, `IsSubsetOf(array, count)` | "argument container" ⊂ "expected container" |
|  `IsSupersetOf({e0, e1, ..., en})`, `IsSupersetOf(a_container)`, `IsSupersetOf(begin, end)`, `IsSupersetOf(array)`, `IsSupersetOf(array, count)` | "argument container" ⊃ "expected container" |
| `Pointwise(m, container)` | argument로 전달된 container의 element 개수와 `container`의 element 개수가 같아야 합니다. 이 때 2개의 container에 속한 모든 element들이 matcher `m`을 통해 비교됩니다. 이 때, 양측의 element는 2-tuples 형태로 matcher `m`에 전달됩니다. 예를 들어 `Pointwise(Le(), upper_bounds)`라는 코드가 있다면 이는 argument로 전달된 container의 element가 `upper_bounds`에 속한 element보다 작은지를 확인 할 것입니다. `Pointwise`와 관련해서는 표 아랫부분에서도 좀 더 설명하고 있습니다. |
| `UnorderedPointwise(m, container)`, `UnorderedPointwise(m, {e0, e1, ..., en})` | 기본적인 동작은 `Pointwise(m, container)`와 유사합니다. 단, element들을 순서대로 비교하지 않습니다. 순서에 관계없이 matcher `m`을 만족하면 됩니다. |
| `WhenSorted(m)` | argument로 전달된 container의 element들을 `<`operator 를 통해서 정렬하고(오름차순) 그 결과를 matcher `m`을 통해 비교합니다. 이 때, 정렬은 gMock에서 자동으로 해줍니다. 예를 들어 `WhenSorted(ElementsAre(1,2,3))`라는 코드는 argument로 `1, 2, 3` 혹은 `2, 3, 1`과 같은 값들이 전달되어야 만족하게 됩니다. |
| `WhenSortedBy(comparator, m)` | 기본적인 동작은 `WhenSorted(m)`와 유사합니다. 단, 정렬방식을 변경할 수 있습니다. 예를 들어 `WhenSortedBy(std::greater<int>(), ElementsAre(3,2,1))`과 같이 사용하면 내림차순으로 정렬한 결과를 비교하게 됩니다. |

몇 가지 추가적인 내용을 공유합니다.

- Matcher들은 아래와 같은 상황에서도 사용 가능합니다.
  1. 참조형식으로 전달되는 native array (예: `Foo(const int (&a)[5])`)
  2. pointer + array size 형태로 array를 사용할 때 (예: `Bar(const T* buffer, int len)` -- 자세한 사용방법은 [multi-argument matchers](#multi-argument-matchers)에서 참조하세요.)
- Multi-dimensional array에도 사용 가능합니다.  (예: container의 element 각각이 array인 경우)
- `Pointwise(m, ...)`에 사용하기 위한 `m`을 구현할 때는 `::testing::tuple<T, U>`라는 tuple을 전달받아서 `T`, `U`를 비교하는 형태가 되어야 합니다. 여기서 `T`와 `U`는 각각 actual container(argument)의 element type과 expected container(parameter)의 element type을 의미합니다. 예를 들어, `Foo`라는 타입이 `operator==` 없이 `Equals()`이라는 method만 제공한다면 해당 타입 2개를 비교하는 matcher는 아래처럼 구현하면 됩니다.

```cpp
using ::std::get;
MATCHER(FooEq, "") {
  return std::get<0>(arg).Equals(std::get<1>(arg));
}
...
EXPECT_THAT(actual_foos, Pointwise(FooEq(), expected_foos));
```

#### Member Matchers

| Matcher | Description |
|:--------|:------------|
|`Field(&class::field, m)`|`argument.field` 또는 `argument->field`를 matcher `m`을 통해 비교합니다. `argument`는 해당 *class*의 객체를 의미합니다.|
|`Key(e)`|`argument.first`는 `e`와 비교됩니다. `e`는 값 또는 matcher가 될 수 있습니다. 예를 들어 `Contatins(Key(Le(5)))`는 어떤 `map`에 `key <= 5`인 element가 있는지 확인합니다.|
|`Pair(m1, m2)`|`argument`는 `std::pair` 타입입니다. `first` 필드는 matcher `m1`을 통해 비교되며 `second` 필드는 matcher `m2`를 통해 비교됩니다.|
|`Property(&class::property, m)`|`argument.property()` 또는 `argument->property()`를 matcher `m`을 통해 비교합니다. `argument`는 해당 *class*의 object를 의미합니다.|

#### Matching the Result of a Function, Functor, or Callback

| Matcher          | Description                                                  |
| :--------------- | :----------------------------------------------------------- |
| `ResultOf(f, m)` | `f`는 function 혹은 functor이며 `f(argument)`의 결과를 matcher `m`과 비교합니다. |

#### Pointer Matchers

| Matcher                   | Description                                                  |
| :------------------------ | :----------------------------------------------------------- |
| `Pointee(m)`              | `argument`가 가리키는 값이 matcher `m`을 통해 비교됩니다. `argument`는 smart pointer 혹은 raw pointer 둘 다 가능합니다. |
| `WhenDynamicCastTo<T>(m)` | `argument`가 `dynamic_cast<T>()`를 통해 전달되었을 때, `m`을 사용해서 비교합니다. |

#### Multi-argument Matchers

기본적으로 하나의 matcher는 *하나의 값만 비교합니다.* 제목인 "multi-argument"라는 것도 사실은 *tuple*을 통해서 가능한 것이기 때문에 여전히 하나의 값을 비교한다고 주장해도 틀린말은 아닙니다. 어찌 됐든 아래 표에 있는 matcher들은 `(x, y)`형태의 tuple을 비교할 때 사용할 수 있습니다. 물론 사용자가 tuple을 사용할 수 있는 matcher를 직접 구현해도 됩니다.

| Matcher | Description |
|:--------|:------------|
|`Eq()`|`x == y`|
|`Ge()`|`x >= y`|
|`Gt()`|`x > y` |
|`Le()`|`x <= y`|
|`Lt()`|`x < y` |
|`Ne()`|`x != y`|

아래 표는 selector라고 불리는 기능들인데, tuple을 비교하는 matcher와 함께 사용하면 유용합니다.

| Matcher | Description |
|:--------|:------------|
|`AllArgs(m)`|사실 단순히 `m`이라고 구현하는 것과 동일합니다. 즉, `EXPECT_THAT(std::make_tuple(2,1), Gt());`과 `EXPECT_THAT(std::make_tuple(2,1), AllArgs(Gt());`은 동일한 의미입니다. `.With`와 함께 `.With(AllArgs(m))`와 같이 사용하는 것도 좋습니다.|
|`Args<N1, N2, ..., Nk>(m)`|전달된 tuple에서 몇 개만 선택하고 이들을 matcher `m`을 통해 비교합니다. argument는 0부터 시작합니다. 예를 들어 `EXPECT_THAT(std::make_tuple(1,2,3), Args<1, 2>(Eq())`이라는 코드는 값 `2`와 `3`이 같은지 비교합니다.|

#### Composite Matchers

여러개의 matcher를 묶어서 실행할 수도 있습니다.

| Matcher                  | Description                                                  |
| :----------------------- | :----------------------------------------------------------- |
| `AllOf(m1, m2, ..., mn)` | `argument`는 `m1`부터 `mn`까지 모든 matcher를 만족해야 합니다. |
| `AnyOf(m1, m2, ..., mn)` | `argument`는 `m1`부터 `mn`까지 중에서 하나의 matcher만 만족하면 됩니다. |
| `Not(m)`                 | `argument`는 matcher `m`을 만족하면 안됩니다.                |

#### Adapters for Matchers ###

| Matcher | Description |
|:--------|:------------|
|`MatcherCast<T>(m)`|Matcher `m`을 `Matcher<T>`로 변환합니다.|
|`SafeMatcherCast<T>(m)`| Matcher `m`을 `Matcher<T>`로 [safely casts](cook_book.md#casting-matchers) 변환합니다. |
|`Truly(predicate)`|`predicate`는 function이나 functor을 의미합니다. 이 때, `predicate(argument)`는 `boolean` compatible한 값을 반환해야 합니다.|

#### Using Matchers as Predicates

| Matcher | Description |
|:--------|:------------|
|`Matches(m)(value)`|`value`가 matcher `m`을 만족하면 `true`입니다. unary functor를 사용하듯이 단독으로 사용할 수 있습니다.|
|`ExplainMatchResult(m, value, result_listener)`|`value`가 matcher `m`을 만족하면 `true`입니다. 그 결과를 `result+listener`를 통해 출력합니다.|
|`Value(value, m)`|`value`가 matcher `m`을 만족하면 `true`입니다.|

#### Defining Matchers

| Matcher                                                      | Description                                                  |
| :----------------------------------------------------------- | :----------------------------------------------------------- |
| `MATCHER(IsEven, "") { return (arg % 2) == 0; }`             | argument가 짝수인지 확인하는 `IsEven()`이라는 matcher를 정의한 것입니다. |
| `MATCHER_P(IsDivisibleBy, n, "") { *result_listener << "where the remainder is " << (arg % n); return (arg % n) == 0; }` | argument가 `n`으로 나누어 떨어지는지 확인하는 `IsDivisibleBy(n)`이라는 matcher를 정의한 것입니다. |
| `MATCHER_P2(IsBetween, a, b, std::string(negation ? "isn't" : "is") + " between " + PrintToString(a) + " and " + PrintToString(b)) { return a <= arg && arg <= b; }` | argument가 a보다 크고, b보다 작은지 확인하는 `IsBetween(a, b)`이라는 matcher를 정의한 것입니다. |

**Notes:**

  1. `MATCHER*` macro는 function이나 class 내부에서는 사용하면 안 됩니다.
  1. Matcher의 body`{}`를 구현할때는 *purely functional*하게 구현해야 합니다. 어떤 side-effect도 가지면 안됩니다. 즉, 순수하게 matcher로 전달되는 argument와 parameter의 값을 읽고 비교해서 그 결과를 알려주는 동작만 수행해야 합니다. 또한, 프로그램의 다른 부분을 수정해서는 안 되며 argument, parameter를 제외한 외부의 다른 정보에 영향을 받아서도 안됩니다. 자신의 기능을 독립적으로 수행해야 합니다.
  1. `PrintToString(x)`를 사용하면 `x`가 어떤 타입이라도 string으로 변환시킬 수 있습니다.

#### Matcher를 Test Assertion처럼 사용하기

| Matcher | Description |
|:--------|:------------|
|`ASSERT_THAT(expression, m)`|`expression`의 값이 matcher `m`을 만족하지 않는다면 [fatal failure](../../../googletest/docs/kr/primer.md#basic-assertions)를 발생시킵니다.|
|`EXPECT_THAT(expression, m)`|`expression`의 값이 matcher `m`을 만족하지 않는다면 non-fatal failure를 발생시킵니다.|

### Actions

**Actions**은 mock function이 어떤 행동을 해야하는지를 명세합니다.

#### Returning a Value

| Action  | Description |
|:--------|:------------|
|`Return()`|Mock function은 `void` 타입을 반환하면서 종료합니다.|
|`Return(value)`|Mock function은 `value`를 반환하면서 종료합니다. 만약, mock function의 본래 return type과 `value`의 타입이 서로 다르다면, value의 타입을 선택하게 됩니다.|
|`ReturnArg<N>()`|Mock function으로 전달된 `N`번째(0부터 시작) argument를 반환합니다.|
|`ReturnNew<T>(a1, ..., ak)`|`new T(a1, ..., ak)`를 반환합니다. Action이 수행될때마다 매번 새로 생성됩니다.|
|`ReturnNull()`|Null pointer를 반환합니다.|
|`ReturnPointee(ptr)`|`ptr`이 가리키는 값을 반환합니다.|
|`ReturnRef(variable)`|변수 `variable`의 참조를 반환합니다.|
|`ReturnRefOfCopy(value)`|`value`를 복사하고 복사본의 참조를 반환합니다. 복사본의 life-time은 action의 life-time과 동일합니다.|

#### Side Effects

| Action  | Description |
|:--------|:------------|
|`Assign(&variable, value)`|`value`를 variable에 저장합니다.|
|`DeleteArg<N>()`| `N`번째(0부터 시작) argument를 delete합니다. 따라서 해당 argument는 pointer 형태입니다. |
|`SaveArg<N>(pointer)`| `N`번째(0부터 시작) argument를 `*pointer`에 저장합니다. |
|`SaveArgPointee<N>(pointer)`| `N`번째(0부터 시작) argument가 가리키는 값을 `*pointer`에 저장합니다. |
|`SetArgReferee<N>(value)` |	`N`번째(0부터 시작) argument가 참조하는 변수에 `value`를 저장합니다. |
|`SetArgPointee<N>(value)` |`N`번째(0부터 시작) argument가 가리키는 변수에 `value`를 저장합니다.|
|`SetArgumentPointee<N>(value)`|`SetArgPointee<N>(value)`과 동일하지만 deprecated 되었습니다. gMock v1.7.0 이후에 삭제될 예정입니다.|
|`SetArrayArgument<N>(first, last)`|[`first`, `last`)에 저장되어 있는 값을 `N`번째(0부터 시작) argument가 가리키는 array에 저장합니다. Array는 pointer 혹은 iterator일수도 있습니다. 이 때, action은 [`first`, `last`) 가 가리키는 값을 직접 소유하지는 않습니다.|
|`SetErrnoAndReturn(error, value)`|`error`에 `errno`를 저장하고, `value`를 반환합니다.|
|`Throw(exception)`|`expection`을 던집니다. 이 때, `exception`은 복사가능해야 합니다. gMock v1.1.0 이후부터 적용 되었습니다.|

#### Callable(Function, Functor, Lambda, Callback)을 Action처럼 사용하기

| Action  | Description |
|:--------|:------------|
|`Invoke(f)`|`f`를 호출합니다. mock function이 전달받은 argument를 `f`에 그대로 전달합니다.|
|`Invoke(object_pointer, &class::method)`|`class::method`를 호출합니다. mock function이 전달받은 argument를 `class:method`에 그대로 전달합니다.|
|`InvokeWithoutArgs(f)`|`f`를 호출합니다. 이 때 mock function이 전달받은 argument는 전달하지 않습니다.|
|`InvokeWithoutArgs(object_pointer, &class::method)`|`class::method`를 호출합니다. 이 때 mock function이 전달받은 argument는 전달하지 않습니다.|
|`InvokeArgument<N>(arg1, arg2, ..., argk)`|Mock function의 `N`번째 argument(0부터 시작)를 호출합니다. 이 때, k개의 argument도 함께 전달합니다.|

`Invoked*`를 통해서 호출된 callable의 반환값을 action 전체의 반환값으로도 사용할 수 있습니다. 즉, `Return*`과 동일한 역할을 수행하게 됩니다.

만약, `Invoked*`에 사용할 callabe을 구현해야 한다면 사용하지 않을 parameter는 `Unused`로 선언하는 것을 추천합니다.

```cpp
using ::testing::Invoke;
double Distance(Unused, double x, double y) { return sqrt(x*x + y*y); }
...
EXPECT_CALL(mock, Foo("Hi", _, _)).WillOnce(Invoke(Distance));
```

`Invoke(callback)`와 `InvokeWithoutArgs(callback)`과 같은 action은 전달받은 `callback`에 대한 소유권을 갖게 됩니다. 따라서 해당 action과 `callback`의 life-time은 동일합니다. 그리고 `callback`의 타입은 base callback이어야만 합니다. 아래 예제와 같이 dervied callback을 사용하면 compile error가 발생합니다.

```c++
  BlockingClosure* done = new BlockingClosure;
  ... Invoke(done) ...;  // This won't compile!

  Closure* done2 = new BlockingClosure;
  ... Invoke(done2) ...;  // This works.
```

`InvokeArgument<N>(...)`을 사용할 때, 참조형식으로 argument를 전달하고 싶다면 `ByRef()`를 사용하면 됩니다. 

```cpp
using ::testing::ByRef;
using ::testing::InvokeArgument;
...
InvokeArgument<2>(5, string("Hi"), ByRef(foo))
```
위 코드는 mock function으로 전달된 2번째 argument를 호출합니다. 그러므로 해당 mock function의 2번째 argument가 기본적으로 callable일 것입니다. 또한, 해당 callable을 호출할 때 3개의 argument(숫자 `5`, 문자열 `"Hi"`, `foo`라는 변수의 참조)도 같이 전달하고 있습니다.

#### Default Action

| Action  | Description |
|:--------|:------------|
|`DoDefault()`|Default action을 수행합니다. Built-in default action 또는 `ON_CALL()`을 통해 사용자가 정의한 default action을 실행합니다.|

**Note:** 기술적인 문제로 인해 `DoDefault()`를 composite action에 사용할 수는 없습니다. 그렇게 되면 runtime error가 발생합니다. Composite action이란 여러개의 action을 조합하는 사용하는 것을 의미하며 바로 아래에서 계속 설명합니다.

#### Composite Actions

| Action                         | Description                                                  |
| :----------------------------- | :----------------------------------------------------------- |
| `DoAll(a1, a2, ..., an)`       | `a1`부터 `an`까지 모든 action을 수행합니다. 전체의 반환값은 `an`의 반환값을 사용합니다. 나머지 action들은 `void`를 반환해야만 합니다. |
| `IgnoreResult(a)`              | Action `a`를 수행하고 그 반환값은 무시합니다. 단, `a`의 return type은 `void`가 되면 안 됩니다. |
| `WithArg<N>(a)`                | Mock function의 `N`번째(0부터 시작) argument를 action `a`에 전달합니다. |
| `WithArgs<N1, N2, ..., Nk>(a)` | Mock function의 argument들 중에서 몇 개를 골라서 action `a`에 전달합니다. |
| `WithoutArgs(a)`               | Action `a`를 argument 없이 수행합니다.                       |

#### Defining Actions

| Action                                        | Description                                                  |
| :-------------------------------------------- | :----------------------------------------------------------- |
| `ACTION(Sum) { return arg0 + arg1; }`         | mock function의 첫번째, 두번째 argument의 합을 반환하는 `Sum()`이라는 action을 정의한 것입니다. |
| `ACTION_P(Plus, n) { return arg0 + n; }`      | mock function의 첫번째 argument에 `n`만큼 더한 값을 반환하는 `Plus(n)`이라는 action을 정의한 것입니다. |
| `ACTION_Pk(Foo, p1, ..., pk) { statements; }` | `statements` 코드를 수행하는 `Foo(p1, ... pk)`라는 parameterized action을 정의한 것입니다. |

`ACTION*` macro를 function이나 class 내부에서 사용하면 안 됩니다.

### Cardinalities

Cardinalities는 mock function의 호출횟수를 명세하는데 사용합니다. 주로 `Times()`와 함께 사용합니다.

| Cardinality | Description |
|:------------|:------------|
|`AnyNumber()`|호출되는 횟수에 제한이 없습니다.|
|`AtLeast(n)`|적어도 `n`회 이상 호출되어야 합니다.|
|`AtMost(n)`|많아도 `n`회를 넘겨서는 안됩니다.|
|`Between(m, n)`|`m`회 이상 `n`회 이하로 호출되기를 원합니다.|
|`Exactly(n) or n`|정확히 `n`회만큼 호출되기를 원합니다. 만약, `n`이 `0`이라면 호출되지 않기를 기대한다는 의미입니다.|

### Expectation Order

기본설정에서는 expectation들간의 *호출순서를 부여하지 않고 있습니다.* 사용자가 expectation들간에 호출순서를 지정하고 싶다면 2가지 방법을 사용할 수 있습니다. 이 때 2가지 방법을 따로 사용해도 되고 같이 사용해도 됩니다.

#### The After Clause

```cpp
using ::testing::Expectation;
...
Expectation init_x = EXPECT_CALL(foo, InitX());
Expectation init_y = EXPECT_CALL(foo, InitY());
EXPECT_CALL(foo, Bar())
     .After(init_x, init_y);
```
위 코드는 `InitX()`와 `InitY()`가 수행된 이후에 `Bar()`가 호출되기를 기대한다는 의미합니다.

아직 개발 중이어서 `Bar()` 이전에 수행되어야 할 function들을 확정하기거 어려운 상황이라면 `ExpectationSet`을 사용하면 됩니다.

```cpp
using ::testing::ExpectationSet;
...
ExpectationSet all_inits;
for (int i = 0; i < element_count; i++) {
  all_inits += EXPECT_CALL(foo, InitElement(i));
}
EXPECT_CALL(foo, Bar())
     .After(all_inits);
```
위 코드는 `Bar()`가 `all_inits`에 포함된 모든 expectation들이 수행된 이후에 호출되기를 기대한다는 의미입니다. 그러나 `all_inits `에 포함된 expectaion들간의 호출순서는 신경쓰지 않습니다. 오직 `Bar()`가 그 후에 호출되기만 하면 됩니다.

이렇게 구현하면 나중에 `all_inits`에 expectation이 추가 혹은 삭제되더라도 `Bar()`가 그 다음에 호출되어야 한다는 점은 변함이 없습니다.

#### Sequences

Expectation들이 많다면 **sequence**를 사용하는 것도 좋습니다. 각 expectation들이 별도의 이름을 가질 필요는 없으며 같은 이름의 sequence를 사용하기만 하면 됩니다. 그렇게 동일한 sequence에 포함된 expectation들은 순서대로 호출되어야 합니다.

```cpp
using ::testing::Return;
using ::testing::Sequence;
Sequence s1, s2;
...
EXPECT_CALL(foo, Reset())
    .InSequence(s1, s2)
    .WillOnce(Return(true));
EXPECT_CALL(foo, GetSize())
    .InSequence(s1)
    .WillOnce(Return(1));
EXPECT_CALL(foo, Describe(A<const char*>()))
    .InSequence(s2)
    .WillOnce(Return("dummy"));
```
위의 코드는 `Reset()`이 다른 2개의 function인 `GetSize()`, `Describe()`보다 먼저 호출되기를 기대한다는 의미입니다. 그 외의 다른 호출순서는 없습니다. 예를 들면 `GetSize()`, `Describe()`라는 2개의 function은 누가 먼저 호출되든 상관이 없습니다. 왜냐하면 2개의 function은 동일한 sequence로 연결되지 않았기 때문입니다.

Sequence를 좀 더 편리하게 사용하는 방법은 아래와 같습니다.
```cpp
using ::testing::InSequence;
{
  InSequence seq;

  EXPECT_CALL(...)...;
  EXPECT_CALL(...)...;
  ...
  EXPECT_CALL(...)...;
}
```
위의 코드는 `seq`라는 scope에 속한 모든 expectation들이 순서대로 호출되기를 기대한다는 의미입니다. 여기서 `seq`라는 이름 자체에는 큰 의미가 없습니다.

### Verifying and Resetting a Mock

gMock은 어떤 mock object가 소멸될 때, 해당 mock object에 설정된 expectation들의 수행결과를 확인해줍니다. 만약 좀 더 일찍 확인하고 싶다면 아래처럼 하면 됩니다.

```cpp
using ::testing::Mock;
...
// Verifies and removes the expectations on mock_obj;
// returns true if and only if successful.
Mock::VerifyAndClearExpectations(&mock_obj);
...
// Verifies and removes the expectations on mock_obj;
// also removes the default actions set by ON_CALL();
// returns true if and only if successful.
Mock::VerifyAndClear(&mock_obj);
```

gMock이 mock object에서 발생하는 memory leak에 대해서는 점검하지 않도록 설정하는 것도 가능합니다.

```cpp
Mock::AllowLeak(&mock_obj);
```

### Mock Classes

gMock은 편리하게 사용할 수 있는 mock class template도 제공합니다.
```cpp
class MockFunction<R(A1, ..., An)> {
 public:
  MOCK_METHOD(R, Call, (A1, ..., An));
};
```
이에 대한 예제코드는 [여기](cook_book.md#stdfunction-mocking하기)를 참조하세요.

### Flags

| Flag | Description |
|:--------|:------------|
| `--gmock_catch_leaked_mocks=0` | Mock object에서 발생하는 memory leak에 대해서는 failure로 취급하지 않습니다. |
| `--gmock_verbose=LEVEL` | gMock에서 출력하는 정보의 양을 조절합니다.(`info`, `warning`, or `error`) |
