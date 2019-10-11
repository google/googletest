# Advanced googletest Topics

## 소개

이 문서를 읽기 전에 [googletest primer](primer.md)를 먼저 보기를 추천한다. googletest primer를 다 읽었다면 이제 다양한 심화기능을 공부 할 시간이다. 이 문서는 googletest의 다양한 assertion 사용법, 사용자정의 failure message 만들기, fatal failure 전파하기, test fixture를 빠르고 재사용하기 쉽게 만들기, test program실행시 flag 사용법과 같은 여러가지 주제를 포함하고 있다.

## Googletest가 지원하는 다양한 Assertion 소개 및 사용법

자주 사용되지는 않지만, 여전히 중요하고 또 필요한 여러가지 assertion들을 확인해보자.

### 성공과 실패 명시하기

`SUCCEED()`, `FAIL()`, `ADD_FAILURE()`라는 assertion들은 value나 expression의 판정하지는 않지만 성공과 실패를 명시적으로 나타내기 위해 사용한다. 다른 assertion macro들처럼 failure message를 변경하는 것도 가능하다.

```c++
SUCCEED();
```

위 코드는 테스트가 성공했음을 코드상에서 표시하기 위한 용도로 사용한다. 다만, 테스트가 여기까지 진행되었다면 성공이다라고 명세하는 것 뿐이며 해당 테스트에 **실패가 없다**를 의미하지는 않는다. 즉, `SUCCEED()`가 포함된 테스트를 실행하더라도 다른 assertion으로 인해 실패가 발생했다면 그 결과는 그대로 실패가 된다. 왜냐하면 `SUCCEDD()`가 테스트 결과에 실제로 영향을 주거나 하지는 않기 때문이다.

NOTE: `SUCCEED()`는 코드상에서 추가적인 정보를 전달하기 위해 사용한다. 그 외에 사용자가 확인할 수 있는 출력물을 제공하거나 하지는 않는다. 추후에 변경될 수는 있지만 현재로서는 그렇다.

```c++
FAIL();
ADD_FAILURE();
ADD_FAILURE_AT("file_path", line_number);
```

`FAIL()`은 fatal failure를 발생시키며 `ADD_FAILURE()`와 `ADD_FAILURE_AT()`는 non-fatal failure를 발생시킨다. 바로 위에서 설명한 `SUCCEED()`는 결과에 영향을 끼치지 않지만, `FAIL()`, `ADD_FAILURE*()`는 실제로 해당 테스트를 실패로 판정한다. 제어흐름 상에서 불필요한 부분을 표현하는데 유용하게 쓸 수 있다. 예를 들어 아래와 같은 코드에 사용할 수 있다.

```c++
switch(expression) {
  case 1:
     ... some checks ...
  case 2:
     ... some other checks ...
  default:
     FAIL() << "We shouldn't get here.";
}
```

NOTE: `FAIL()`은  return type이 `void`인 function에만 사용할 수 있다. 이와 관련한 자세한 내용은 [Assertion Placement section](advanced.md#assertion을-사용가능한-곳)를 참조할 수 있다.

### Exception Assertions

아래 assertion들은 주어진 코드(`statement`)가 exception를 던지는지 아닌지를 검증하는데 사용한다.

| Fatal assertion                            | Nonfatal assertion                         | Verifies                                          |
| ------------------------------------------ | ------------------------------------------ | ------------------------------------------------- |
| `ASSERT_THROW(statement, exception_type);` | `EXPECT_THROW(statement, exception_type);` | `statement` throws an exception of the given type |
| `ASSERT_ANY_THROW(statement);`             | `EXPECT_ANY_THROW(statement);`             | `statement` throws an exception of any type       |
| `ASSERT_NO_THROW(statement);`              | `EXPECT_NO_THROW(statement);`              | `statement` doesn't throw any exception           |

예제코드:

```c++
ASSERT_THROW(Foo(5), bar_exception);

EXPECT_NO_THROW({
  int n = 5;
  Bar(&n);
});
```

**Availability**: 기본적으로 빌드시에 exception이 활성화되어 있어야 동작한다.

### Predicate Assertion을 통해 더 유용한 Error Message 만들기

지금까지 확인한 것처럼 googletest는 다양한 assertion들을 제공하고 있다. 다만, 여전히 사용자가 원하는 모든 경우를 커버할 수는 없으며 또 그런 상황들을 위해서 너무 다양한 assertion을 제공하는 것도 좋은 방법은 아닌 것 같다. 그렇다보니 `EXPECT_TRUE()`와 같이 (단순하다고도 할 수 있는) assertion에 매우 복잡한 expression이 전달되는 상황이 발생하기도 한다. 물론 복잡한 expression을 사용한다고 해서 꼭 문제가 되는 것은 아니지만 사용자 입장에서의 불편함들을 초래한 것은 사실이다. 예를 들어 expression이 복잡하면 기본적으로 제공되는 failure message만으로는 부족하게 되고 사용자가 직접 failure message를 변경하게 될 확률이 높다. 이 때, 해당 expression이 어떤 side-effect(부수효과)를 포함하고 있다면 failrue message가 출력해주는 값들이 정확한지도 의심해야 되고 여러가지로 신경써야 할 것들이 많아지기 때문이다.

이에 googletest는 위와 같은 상황에서 유연하게 대처하기 위한 몇 가지 방법을 제공하고 있다.

#### 이미 정의해 놓은 Boolean Function 사용하기

사용자가 구현한 일반 C++ function(혹은 functor)을 assertion처럼 사용할 수 있다. (단, 해당 function의 return type이 `bool`이어야 한다. *predicate assertion*이라고 부르는 이 기능은 function을 *assertion*처럼 동작하게 만들고 원하는 failure message를 출력하도록 도와준다. 그럼 아래 표에서 관련 macro를 확인해보자.

| Fatal assertion                    | Nonfatal assertion                 | Verifies                    |
| ---------------------------------- | ---------------------------------- | --------------------------- |
| `ASSERT_PRED1(pred1, val1);`       | `EXPECT_PRED1(pred1, val1);`       | `pred1(val1)` is true       |
| `ASSERT_PRED2(pred2, val1, val2);` | `EXPECT_PRED2(pred2, val1, val2);` | `pred2(val1, val2)` is true |
| `...`                              | `...`                              | `...`                       |

첫 번째 argument인 `predn`은 `bool`을 반환하는 function(혹은 functor)를 의미한다. 이 때, 해당 function이 `ASSERT_PREDn` macro의 `n`과 동일한 개수의 argument를 전달받아야 하기때문에 `predn`이라고 표시한 것이다. 두 번째 및 그 이후에 전달되는 argument들(`val1`, `val2`, `...`)은 `predn`에 전달하기 위한 argument이다. 이렇게 정의된 predicate assertion도 다른 assertion과 동일하게 동작한다. 즉, `true`가 반환되면 성공을 의미하고 `false`가 반환되면 실패를 의미한다. 그리고 assertion이 실패할 때는 각 argument의 정보도 출력해 준다.

그럼 예제를 보자.

```c++
// Returns true if m and n have no common divisors except 1.
bool MutuallyPrime(int m, int n) { ... }

const int a = 3;
const int b = 4;
const int c = 10;
```

전달된 argument 2개(`m`, `n`)에 공약수가 1밖에 없으면 `true`를 반환하는 `MutuallyPrime()`이라는 function이 정의되어 있다. 이제 아래와 같이 구현하면 해당 function을 assertion처럼 사용할 수 있다.

```c++
  EXPECT_PRED2(MutuallyPrime, a, b);
```

`a`와 `b`는 1 외에는 공약수가 없으므로 위 assertion은 성공할 것이다.

반면에 아래에 있는 assertion에서는 `b`와 `c`가 1 외에도 공약수(2)를 가지므로 실패할 것이다.

```c++
  EXPECT_PRED2(MutuallyPrime, b, c);
```

 그리고 아래와 같은 failure message를 출력할 것이다.

```none
MutuallyPrime(b, c) is false, where
b is 4
c is 10
```

> NOTE:
>
> 1.  `ASSERT_PRED*` 또는 `EXPECT_PRED*`를 사용할 때, "no matching function to call"와 같은 compile error가 발생하면 [여기](faq.md#assert_pred를-사용할-때-no-matching-function-to-call-이라는-compile-error가-발생했습니다-어떻게-해야-하나요)를 확인해보자.

#### AssertionResult를 반환하는 function을 사용하기

`EXPECT_PRED*()` 계열은 유용하지만 argument 개수에 따라 macro가 달라지기 때문에 약간 불편한 부분도 있긴 하다. C++ 문법보다는 Lisp 문법에 가까워 보이기도 한다. 여기서는 또 다른 방법으로 `::testing::AssertionResult` class를 사용해 보자.

`AssertionResult`는 이름 그대로 assertion의 결과를 의미하며 사용법은 간단하다. googletest에서 제공하는 factory function을 사용하면 된다. Factory function 2개는 아래와 같다.

```c++
namespace testing {

// Returns an AssertionResult object to indicate that an assertion has
// succeeded.
AssertionResult AssertionSuccess();

// Returns an AssertionResult object to indicate that an assertion has
// failed.
AssertionResult AssertionFailure();

}
```

더불어 `AssertionResult` object에 `<<` 연산자를 구현하면 stream message를 변경할 수도 있다.

사용방법은 `bool MutuallyPrime()`를 predicate assertion으로 사용했던 것과 동일하다. 다만, function의 return type에 `bool`이 아니라 `AssertionResult`을 사용해야 한다. 먼저 아래와 같이 `n`이 짝수인지 아닌지 판정하는 `IsEven()`이라는 function이 있다고 가정해 보자.

```c++
bool IsEven(int n) {
  return (n % 2) == 0;
}
```

`IsEven()`이 `AssertionResult`를 반환할 수 있도록 변경하자.

```c++
::testing::AssertionResult IsEven(int n) {
  if ((n % 2) == 0)
     return ::testing::AssertionSuccess();
  else
     return ::testing::AssertionFailure() << n << " is odd";
}
```

모두 준비가 끝났다. 이제 기존 assertion과 함께 사용하기만 하면 된다.

예를 들어 `EXPECT_TRUE(IsEven(Fib(4)))`와 같이 사용하면 된다. 만약 실패하면 아래와 같은 failure message를 출력해 줄 것이다. "3 is odd"라는 추가 정보가 출력되었다. 이렇듯 간단하게 디버깅에 필요한 더 많은 정보를 제공할 수 있다.

```none
Value of: IsEven(Fib(4))
  Actual: false (3 is odd)
Expected: true
```

위의 failure message가 왜 좋을까? 아래의 기존 failure message와 비교해보면 "3 is odd"라는 디버깅 정보를 바로 확인할 수 있게 되었다. 테스트의 양이 많아질수록 이런 작은 부분들도 도움이 될 것이다.

```none
Value of: IsEven(Fib(4))
  Actual: false
Expected: true
```

`IsEven`을 `EXPECT_FALSE` 또는 `ASSERT_FALSE`와 같은 negative assertion에도 사용할 수 있도록 조금 더 개선해 보자.

```c++
::testing::AssertionResult IsEven(int n) {
  if ((n % 2) == 0)
     return ::testing::AssertionSuccess() << n << " is even";
  else
     return ::testing::AssertionFailure() << n << " is odd";
}
```

이제 `EXPECT_FALSE`와 함께 사용해도 추가적인 디버깅 정보를 제공할 수 있게 되었다. `IsEven()`의 최종코드는 `EXPECT_FALSE(IsEven(Fib(6)))`라는 assertion에 대해서도 꽤 괜찮은 디버깅 정보를 출력해 줄 것이다.

```none
  Value of: IsEven(Fib(6))
     Actual: true (8 is even)
  Expected: false
```

#### Predicate-Formatter 사용하기

지금까지 `(ASSERT|EXPECT)_PRED*` 와 `(ASSERT|EXPECT)_(TRUE|FALSE)`을 일반 function(또는 functor)과 함께 사용하는 방법에 대해 확인했다. 아직도 부족하다고 생각하는 사람도 있을 것이다. 예를 들어 argument가 `ostream`을 사용하지 못하는 타입이라면 위의 2가지 방법을 적용하기 어렵다. 그런 경우에는 *predicate-formatter assertions*을 사용해야 한다. *predicate-formatter assertions*을 사용하면 아예 모든 message를 사용자가 정의할 수 있게 된다.

| Fatal assertion                                  | Nonfatal assertion                               | Verifies                                 |
| ------------------------------------------------ | ------------------------------------------------ | ---------------------------------------- |
| `ASSERT_PRED_FORMAT1(pred_format1, val1);`       | `EXPECT_PRED_FORMAT1(pred_format1, val1);`       | `pred_format1(val1)` is successful       |
| `ASSERT_PRED_FORMAT2(pred_format2, val1, val2);` | `EXPECT_PRED_FORMAT2(pred_format2, val1, val2);` | `pred_format2(val1, val2)` is successful |
| `...`                                            | `...`                                            | `...`                                    |

`(ASSERT|EXPECT)_PRED_FORMAT*` 계열과 앞서 설명한 2가지 방법과의 다른 점은 predicate가 아니라 *predicate-formatter*(`pred_formatn`)를 사용한다는 점입니다. *predicate-formatter*란 아래와 같은 형식으로 정의된 function을 의미한다.

```c++
::testing::AssertionResult PredicateFormattern(const char* expr1,
					       const char* expr2,
					       ...
					       const char* exprn,
					       T1 val1,
					       T2 val2,
					       ...
					       Tn valn);
```

`PredicateFormattern()`로 전달되는 `valn` argument들은 실제로 assertion에 사용되는 값들이다. 그리고 `Tn`은 `valn`의 타입을 의미하는데 값, 참조 둘 다 가능하다. 예를 들어 `int` 타입이라면 그대로 `int` 혹은 `const Foo&` 등이 올 수 있다. 마지막으로 `exprn`은 `valn`을 전달하는 caller 쪽 소스코드가 그대로 저장된 문자열이다. 쉽게 말해서 caller쪽의 구현이 `Foo(variable)`라면 `expr1`은 `"variable"`이라는 문자열이 된다.

이제 예제를 보자. 예제코드의 `AssertMutuallyPrime()`은 `EXPECT_PRED2()`를 설명할 때 사용했던 `MutuallyPrime()`을 *predicate-formatter*로 변경한 코드이다.

```c++
// Returns the smallest prime common divisor of m and n,
// or 1 when m and n are mutually prime.
int SmallestPrimeCommonDivisor(int m, int n) { ... }

// A predicate-formatter for asserting that two integers are mutually prime.
::testing::AssertionResult AssertMutuallyPrime(const char* m_expr,
					       const char* n_expr,
					       int m,
					       int n) {
  if (MutuallyPrime(m, n)) return ::testing::AssertionSuccess();

  return ::testing::AssertionFailure() << m_expr << " and " << n_expr
      << " (" << m << " and " << n << ") are not mutually prime, "
      << "as they have a common divisor " << SmallestPrimeCommonDivisor(m, n);
}
```

이렇게 구현된 `AssertMutuallyPrime()`는 아래처럼 사용할 수 있다.

```c++
  EXPECT_PRED_FORMAT2(AssertMutuallyPrime, b, c);
```

위 코드에서 `b`, `c`의 값이 각각 `4`, `10` 이었다면 최종적으로 아래와 같은 failure message가 출력됨을 확인할 수 있다.

```none
b and c (4 and 10) are not mutually prime, as they have a common divisor 2.
```

이미 눈치챘을 수도 있지만 `(EXPECT|ASSERT)_PRED_FORMAT*`은 가장 기본적인 assertion 정의방법이며 대부분의 built-in assertion들도 이를 기반으로 만들어 졌다.

### Floating-Point 비교하기

반올림 이슈로 인해서 2개의 floating-point 값이 정확히 같다고 판정하는 것은 언제나 까다로운 문제이다. 기존에 사용하던 `ASSERT_EQ`로는 정확한 답을 얻을 수 없을 것이다. 또한, floating-point는 값의 범위가 큰 경우가 많기 때문에 고정 오차범위보다는 상대 오차범위를 사용하는 것이 정밀도 측면에서 더 좋은 선택이다. 고정 오차범위가 더 좋은 경우는 `0`과 같은지 비교할 때 뿐이다.

결론적으로 floating-point를 비교하기 위해서는 오차범위를 신중하게 선택해야 한다. googletest는 오차범위 단위로 ULPs를 기본적으로 사용하고 있으며 그 범위는 4 ULP's 이다. 만약, 직접 오차범위를 지정하기가 꺼려진다면 googletest의 기본설정을 사용하기를 추천한다. 물론, 기본설정 오차범위를 이용하는 macro와 사용자가 오차범위를 지정할 수 있는 macro를 둘 다 제공하므로 주어진 상황에 맞게 사용할 수 있다. (ULPs 및 floating-point 비교에 대한 자세한 설명은 [이곳](https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/) 또는 [이곳](https://bitbashing.io/comparing-floats.html)에서 확인할 수 있다.)

#### Floating-Point Macros

| Fatal assertion                 | Nonfatal assertion              | Verifies                                 |
| ------------------------------- | ------------------------------- | ---------------------------------------- |
| `ASSERT_FLOAT_EQ(val1, val2);`  | `EXPECT_FLOAT_EQ(val1, val2);`  | the two `float` values are almost equal  |
| `ASSERT_DOUBLE_EQ(val1, val2);` | `EXPECT_DOUBLE_EQ(val1, val2);` | the two `double` values are almost equal |

위 표에서 "almost equal"이란, googletest 기본설정에 따라 2개의 값이 오차범위 4 ULP's 내에서 같음을 의미한다.

만약, 오차범위를 직접 지정하고 싶다면 아래의 assertion을 사용하면 된다.

| Fatal assertion                       | Nonfatal assertion                    | Verifies                                                     |
| ------------------------------------- | ------------------------------------- | ------------------------------------------------------------ |
| `ASSERT_NEAR(val1, val2, abs_error);` | `EXPECT_NEAR(val1, val2, abs_error);` | the difference between `val1` and `val2` doesn't exceed the given absolute error |

#### Floating-Point Predicate-Format Functions

Floating-point 연산자들을 더 다양하게 만들어서 제공할 수도 있지만 macro가 너무 많아지는 것도 좋지 않다. 대신에 predicate-formatter assertion을 floating-piont에도 사용할 수 있도록 했다. 사용방법은 기존과 동일하며 첫번째 argument에 `FloatLE`, `DoubleLE`와 같은 floating-point 관련 function을 전달한다는 점만 다르다.

```c++
EXPECT_PRED_FORMAT2(::testing::FloatLE, val1, val2);
EXPECT_PRED_FORMAT2(::testing::DoubleLE, val1, val2);
```

위 예제는 `val1`이 `val2`보다 작거나 같은지(almost equal)를 검사하는 코드이다. `ASSERT_PRED_FORMAT2`을 사용할때도 동일하게 사용하면 된다.

### gMock Matchers를 이용한 Asserting

C++ mocking framework인 [gMock](../../../../googlemock)을 개발하면서 mock object로 전달되는 argument를 확인하기 위한 방법으로 matcher라는 것을 새로 도입했다. gMock *matcher*는 predicate와 그 원리가 같으면서도 이미 풍부한 built-in matcher들을 제공하고 있다.

이러한 matcher들을 `*_THAT` 계열 assertion macro와 함께 사용하기만 하면 된다.

| Fatal assertion                | Nonfatal assertion             | Verifies              |
| ------------------------------ | ------------------------------ | --------------------- |
| `ASSERT_THAT(value, matcher);` | `EXPECT_THAT(value, matcher);` | value matches matcher |

아래는 `StartsWith(prefix)`라는 built-in matcher를 통해서 `Foo()`의 반환값(문자열)이 `"Hello"`로 시작하는지 검사해주는 assertion을 구현한 코드이다.

```c++
using ::testing::StartsWith;
...
    // Verifies that Foo() returns a string starting with "Hello".
    EXPECT_THAT(Foo(), StartsWith("Hello"));
```

이렇듯 matcher를 사용하면 간단하게 새로운 assertion을 만들 수 있다. 좀 더 상세한 사용방법은 [여기](../../../../googlemock/docs/translations/ko_KR/cook_book.md#matcher를-googletest-assertion처럼-사용하기)에서 확인할 수 있다.

gMock은 `StartsWith()` 외에도 다양한 built-in matcher를 제공하고 있다. 이러한 built-in matcher를 알아보려면 [여기](../../../../googlemock/docs/translations/ko_KR/cheat_sheet.md#matchers)를 참조하라. 만약, built-in matcher 중에 원하는 기능이 없다면 matcher를 직접 구현하는 것도 가능하다. [여기](../../../../googlemock/docs/translations/ko_KR/cook_book.md#새로운-matcher-구현하기)에서 그 방법을 확인할 수 있다.

마지막으로 gMock 자체가 googletest와 함께 제공되는 번들 소프트웨어이므로 matcher 사용을 위한 추가적인 환경설정은 따로 필요하지 않다. 헤더파일만 포함(`#include "testing/base/public/gmock.h"`)하면 바로 사용할 수 있을 것이다.

### More String Assertions

([이전](#gmock-matchers를-이용한-asserting) 섹션을 먼저 읽어야 함)

바로 위에서도 확인했듯이 gMock은 문자열과 관련된 [string matchers](../../../../googlemock/docs/translations/ko_KR/cheat_sheet.md#string-matchers)를 풍부하게 제공하고 있다. 이렇게 제공되는 built-in matcher들을 `EXPECT_THAT()` 또는 `ASSERT_THAT()`과 함께 사용하기만 하면 된다. 이를 통해 sub-string, prefix, suffix, regular expression과 같은 다양한 방법의 string assertion을 수행할 수 있다. 사용방법은 아래 예제코드와 같다.

```c++
using ::testing::HasSubstr;
using ::testing::MatchesRegex;
...
  ASSERT_THAT(foo_string, HasSubstr("needle"));
  EXPECT_THAT(bar_string, MatchesRegex("\\w*\\d+"));
```

또한, 문자열이 HTML이나 XML을 포함할 때는 [XPath expression](http://www.w3.org/TR/xpath/#contents)를 통해서 DOM tree와의 비교도 수행할 수 있다.

```c++
// Currently still in //template/prototemplate/testing:xpath_matcher
#include "template/prototemplate/testing/xpath_matcher.h"
using prototemplate::testing::MatchesXPath;
EXPECT_THAT(html_string, MatchesXPath("//a[text()='click here']"));
```

### Windows HRESULT assertions

Windows 환경의 `HRESULT`를 위한 assertion도 제공하고 있다.

| Fatal assertion                        | Nonfatal assertion                     | Verifies                            |
| -------------------------------------- | -------------------------------------- | ----------------------------------- |
| `ASSERT_HRESULT_SUCCEEDED(expression)` | `EXPECT_HRESULT_SUCCEEDED(expression)` | `expression` is a success `HRESULT` |
| `ASSERT_HRESULT_FAILED(expression)`    | `EXPECT_HRESULT_FAILED(expression)`    | `expression` is a failure `HRESULT` |

위의 assertion을 사용하면 주어진 `expression`을 수행하고 그 결과인 `HRESULT`를 출력해 준다.

사용방법은 아래와 같다.

```c++
CComPtr<IShellDispatch2> shell;
ASSERT_HRESULT_SUCCEEDED(shell.CoCreateInstance(L"Shell.Application"));
CComVariant empty;
ASSERT_HRESULT_SUCCEEDED(shell->ShellExecute(CComBSTR(url), empty, empty, empty, empty));
```

### Type Assertions

타입 `T1`과 `T2`가 같은지 확인하기 위한 assertion은 아래와 같다.

```c++
::testing::StaticAssertTypeEq<T1, T2>();
```

Type assertion은 compiler에 의해서 수행되기 때문에 성공하면 아무런 문제없이 지나가지만 실패했을 때는 compile error가 발생한다. 그렇게 실패했을 때는 `type1 and type2 are not the same type`이라는 error message가 출력되며 이와 더불어 (compiler에 따라 다르긴 하지만) `T1`, `T2`가 실제로 무엇인지도 알려준다. 템플릿 코드를 구현할 때 유용할 것이다.

**Caveat**: 한가지 주의할 점은 위의 assertion을 function template이나 class template에서 사용하게 되면 해당 타입에 대한 template 코드가 실제로 만들어 질 때만 동작한다는 것이다. 왜냐하면 C++에서는 호출되지 않거나 사용되지 않는 template은 compile 대상에도 포함되지 않기 때문이다. 예를 들어 아래와 같은 class template이 있다고 가정해보자.

```c++
template <typename T> class Foo {
 public:
  void Bar() { ::testing::StaticAssertTypeEq<int, T>(); }
};
```

다음으로 위에서 정의한 class template인 `Foo`를 사용하는 코드를 살펴보자. 먼저 아래의 `Test1()`은 `StaticAssertTypeEq`로 전달되는 2개 타입(`int`, `bool`)이 서로 다름에도 compile error가 발생하지 않는다. 왜냐하면 `Bar()`를 호출하는 코드가 없기 때문에 compile 대상에서도 제외되기 때문이다.

```c++
void Test1() { Foo<bool> foo; }
```

반면에 아래의 `Test2()`는 `Foo<bool>::Bar()`를 호출하는 코드가 존재하기 때문에 assertion이 동작하게 된다. 그리고 그 결과는 타입 불일치로 인한 compile error일 것이다.

```c++
void Test2() { Foo<bool> foo; foo.Bar(); }
```

### Assertion을 사용가능한 곳

Assertion은 C++ function이기만 한다면 어디에든 사용할 수 있다. 꼭 test fixture class의 member function일 필요도 없다. 다만, 유일한 한 가지 제약은 fatal failure(`FAIL*`, `ASSERT_*`) 계열 assertion들은 return type이 `void`인 function에서만 사용할 수 있다는 것인데 그 이유는 googletest가 exception을 사용하지 않기 때문이다. Return type이 `void`가 아닌 function에서 fatal assertion을 사용하면 아래와 같은 compile error 중 하나가 발생하게 된다.

```bash
error: void value not ignored as it ought to be
```
```bash
cannot initialize return object of type 'bool' with an rvalue of type 'void'
```
```bash
error: no viable conversion from 'void' to 'string'.
```

만약 return type이 `void`가 아닌 function에서 fatal assertion을 사용해야 한다면 조금 수정이 필요하다. 먼저 해당 function의 return type을 `void`로 변경하고 원래 반환하려고 했던 값은 반환하는게 아니라 argument에 대입하도록 수정해야 한다. 예를 들어 `T2 Foo(T1 x)`라는 function을 `void Foo(T1 x, T2* result)`로 변경하면 된다. 그러면 return type이 `void`로 변경되었으니 `Foo()` 내부에서도 fatal assertion을 사용할 수 있게 된다. 주의할 점으로 이러한 변경은 `Foo()`의 caller와 `Foo()` 간의 관계를 변경하는 것이기 때문에 `Foo()`가 어떤 이유로 인해서든 `result`에 의미 있는 값을 대입하지 못하고 비정상 종료되더라도 `Foo()`를 사용하는 쪽에서는 문제가 없도록 구현해야 한다.

만약, 이처럼 return type을 변경할 수 없는 상황이라면 어쩔 수 없이 `ADD_FAILURE*` 또는 `EXPECT_*`와 같은 non-fatal failure를 사용해야 한다.

NOTE: class의 constructor나 destructor는 return type이 따로 없기 때문에 fatal assertion도 사용할 수 없다. 사용하더라도 compile error가 발생할 것이다. 이를 위한 첫 번째 대안으로 `abort`를 사용할 수 있는데 `abort`는 test program을 아예 종료하는 것이기 때문에 원하는 동작이 맞는지는 확인해봐야 한다. 두 번째 대안은 `SetUp`/ `TearDown`을 사용하는 것이며 이와 관련 내용은 [constructor/destructor vs. `SetUp`/`TearDown`](faq.md#test-fixture에서-constructordestructor-와-setupteardown중-어느것을-써야하나요)에서 자세하게 확인할 수 있다.

WARNING: constructor, desturctor에서 fatal assertion을 사용하는 또 다른 방법은 assertion을 수행 할 function을 별도로 만들고(private 영역에) constructor나 destructor가 해당 function을 호출하도록 하는 것이다. 여기서 주의할 점은 constructor나 destructor에서 발생한 fatal assertion은 진행 중인 테스트를 중단시키지는 못하고 자기자신만 중단한다는 것이다. 그렇게 되면 상황에 따라 object의 생성이나 소멸이 완료되지 못한 상태에서 테스트가 진행될 가능성이 있기 때문에 심각한 문제를 초래할 수도 있다. 이와 같은 이유로 constructor나 destructor에서 fatal assertion을 사용하려는 사용자는 그에 따른 문제가 없는지에 대해서 사전에 철저히 확인해야 하며 그러한 부분이 부담된다면 `SetUp/TearDown`을 사용하거나 아니면 `abort`를 사용하는 것이 더 편할 것이다.

## Googletest의 디버깅정보 출력방식 변경하기

`EXPECT_EQ`같은 assertion이 실패하면, googletest는 argument로 전달된 값을 비롯해서 디버깅에 필요한 정보를 출력해준다. 이렇게 디버깅 정보를 출력해주는 기능을 printer라고 하는데 이 printer는 사용자가 원하는 방향으로 확장도 가능하다.

Googletest는 기본적으로 C++ built-in 타입, STL container, `<<` 연산자를 정의한 타입에 대해서는 디버깅 정보를 출력해주지만 그 외의 타입들은 raw bytes 값만 출력해주기 때문에 디버깅이 어려울 수 있다.

이런 경우에는`<<` 연산자를 재정의해서 printer를 확장하는 것이 좋다. 아래 예제코드를 보자.

```c++
// Streams are allowed only for logging.  Don't include this for
// any other purpose.
#include <ostream>

namespace foo {

class Bar {  // We want googletest to be able to print instances of this.
...
  // Create a free inline friend function.
  friend std::ostream& operator<<(std::ostream& os, const Bar& bar) {
    return os << bar.DebugString();  // whatever needed to print bar to os
  }
};

// If you can't declare the function in the class it's important that the
// << operator is defined in the SAME namespace that defines Bar.  C++'s look-up
// rules rely on that.
std::ostream& operator<<(std::ostream& os, const Bar& bar) {
  return os << bar.DebugString();  // whatever needed to print bar to os
}

}  // namespace foo
```

다만, 경우에 따라서는 `<<` 연산자를 재정의하는 것에 대해 팀 동료들이 반대할 수도 있고 `<<` 연산자가 이미 구현되어 있어서 바꿀 수 없을 수도 있다. 그럴 때에는 `PrintTo()`와 같은 디버깅정보를 확인하기 위한 별도의 function을 정의하는 것도 좋은 방법이다. 아래 예제코드를 보자.

```c++
// Streams are allowed only for logging.  Don't include this for
// any other purpose.
#include <ostream>

namespace foo {

class Bar {
  ...
  friend void PrintTo(const Bar& bar, std::ostream* os) {
    *os << bar.DebugString();  // whatever needed to print bar to os
  }
};

// If you can't declare the function in the class it's important that PrintTo()
// is defined in the SAME namespace that defines Bar.  C++'s look-up rules rely
// on that.
void PrintTo(const Bar& bar, std::ostream* os) {
  *os << bar.DebugString();  // whatever needed to print bar to os
}

}  // namespace foo
```

만약, 사용자가 `<<`와 `PrintTo()`를 2개 모두 구현했다면 `PrintTo()`가 우선적으로 선택된다. 그 이유는 2개를 비교했을 때, `<<` 연산자 쪽이 이미 구현되어 다른 용도로 사용하고 있을 확률이 더 높기 때문이다.

`Bar` class에 `<<` 혹은 `PrintTo()`를 정의하는 것까지 완료했다면, 이제 확인하고 싶은 값을 출력하기만 하면 된다. 예를 들어 `x`의 값을 출력하고 싶다면 `::testing::PrintToString(x)`라고 구현하면 된다. 여기서 `::testing::PrintToString(x)`의 return type은 `std::string`이다. 사용방법은 아래 예제코드에서 확인할 수 있다.

```c++
vector<pair<Bar, int> > bar_ints = GetBarIntVector();

EXPECT_TRUE(IsCorrectBarIntVector(bar_ints))
    << "bar_ints = " << ::testing::PrintToString(bar_ints);
```

## Death Test

원하는 조건이 충족되지 않았을 때 스스로를 종료시키는 프로그램도 당연히 존재할 것이다. 예를 들어 프로그램이 동작하다가 치명적인 문제를 일으킬 수도 있는 비정상상태로 진입했음을 알게되면 문제가 더 악화되기 전에 스스로를 종료시키는 상황이 있을 것이다. (여기서 치명적인 문제라는 것은 memory corruption이나 security holes와 같은 문제를 의미한다.)

Googletest는 이렇게 스스로 종료하도록 구현된 프로그램들이 실제로 원하는 상황에서 원하는 방향으로 잘 종료되는지를 확인하기 위한 방법을 제공하고 있으며 이를 *death test*라고 부른다. 쉽게 말해서 *death test*란 프로그램이 원하는 방향으로 종료되었는지 확인하는 것이다.

단, exception은 death test에 포함되지 않는다. 왜냐하면 exception은 catch해서 적절한 조치를 취하도록 구현하는 것이 목적이기 때문에 프로그램을 아예 종료하는 death test와는 다른 관점에서 봐야하기 때문니다. 만약, exception에 대한 테스트가 필요하다면 [Exception Assertions](#exception-assertions)에서 관련 내용을 별도로 다루고 있다.

### Death Test 구현하기

Death test를 위해서 아래와 같은 macro를 제공하고 있다.

| Fatal assertion                                  | Nonfatal assertion                               | Verifies                                                     |
| ------------------------------------------------ | ------------------------------------------------ | ------------------------------------------------------------ |
| `ASSERT_DEATH(statement, matcher);`              | `EXPECT_DEATH(statement, matcher);`              | `statement` crashes with the given error                     |
| `ASSERT_DEATH_IF_SUPPORTED(statement, matcher);` | `EXPECT_DEATH_IF_SUPPORTED(statement, matcher);` | if death tests are supported, verifies that `statement` crashes with the given error; otherwise verifies nothing |
| `ASSERT_EXIT(statement, predicate, matcher);`    | `EXPECT_EXIT(statement, predicate, matcher);`    | `statement` exits with the given error, and its exit code matches `predicate` |

먼저 `statement`는 종료되기를 기대하는 대상 프로그램이나 코드를 의미한다. 당연히 여러줄의 코드도 가능하다. 다음으로 `predicate`는 `statement`의 exit status를 확인하는 function이나 function object이다. 마지막으로 `matcher`에는 `const std::string&`을 전달받을 수 있는 gMock matcher 또는 (Perl) regular expression을 사용할 수 있다. 이러한 `matcher`는 `statement`가 stderr로 출력하는 error message를 확인하는데 사용된다. `statement`의 stderr 출력이 `matcher`를 만족하지 못하면 death test는 실패한다. 만약에 `matcher`에 순수 문자열만 전달한다면 `ContainsRegex(str)`로 자동 변환된다. 그 이유는 googletest의 이전 버전에서는 regular expression만 전달받을 수 있었다가 gMock matcher도 사용할 수 있게 변경되었기 때문에 하위호환을 위해 그렇게 구현되었다.

Death test assertion도 `ASSERT`계열은 현재 실행중인 test function을 중단시키고 `EXPECT`계열은 계속해서 진행한다.

> NOTE: 위 표에서 사용된 "crash"라는 단어는 process가 종료될 때의 exit status가 `0`이 아님을 의미한다. 2가지 가능성이 있다. 해당 process가 `exit()` 또는 `_exit()`를 호출하면서 `0`이 아닌 값을 전달했거나 혹은 signal을 수신해서 종료된 경우이다.
>
> 반대로 `statement`의 exit status가 `0`이라면 이는 곧 crash가 아님을 의미하기 때문에 `EXPECT_DEATH`를 사용할 수가 없게 된다. 만약, 이처럼 crash가 아니거나 또는 exit status를 좀 더 세부적으로 조작하고 싶은 경우에는 `EXPECT_EXIT`를 사용해야 한다.

`predicate`는 return type이 `int` 또는 `bool`인 function만 사용 가능하며 그렇게 전달된 `predicate`가 `true`를 반환하면 death test도 성공하게 된다. 즉, `statement`가 원하는 exit code와 함께 종료된 것을 의미한다. 더불어 googletest는 일반적인 상황에서 사용가능한 `predicate`들을 기본적으로 제공하고 있다.

```c++
::testing::ExitedWithCode(exit_code)
```

위의 `predicate`는 `statement`의 exit code가 `exit_code`와 동일하다면  `true`이다.

```c++
::testing::KilledBySignal(signal_number)  // Not available on Windows.
```

위의 `predicate`는 `statement`가 signal을 수신했고, 해당 signal number가 `signal_number`과 동일하다면 `true`이다.

`*_DEATH` macro는 사실 `*_EXIT`를 사용해서 확장한 wrapper macro이며 `statment`의 exit code가 0인지 아닌지를 확인하기 위해 사용한다.

정리해보면 death test assertion은 아래 3가지를 확인해서 성공,실패를 결정한다.

1.  `statement`가 `abort` 또는 `exit` 되었는지 확인
2.  `ASSERT_EXIT` 또는 `EXPECT_EXIT`는 exit status가 `predicate`를 만족하는지 확인하고, `ASSERT_DEATH` 또는 `EXPECT_DEATH`는 exit code != 0 을 만족하는지 확인
3.  `statment`의 stderr출력이 `matcher`를 만족하는지 확인

한가지 주의할 점은 만약 `statement` 내부에 `ASSERT_*` 또는 `EXPECT_*`를 사용했다고 하더라도 death test assertion의 결과에는 영향을 미치지 않는 다는 것이다. 당연하게도 death test는 `statement`가 어떻게 종료되는지 확인하는 것이 목적이기 때문이다. `ASSERT_*`, `EXPECT_*`은 프로그램을 종료시키거나 하는 macro가 아니기 때문에 `statement`내부에서 발생하는 작업일 뿐이며 death test의 관심대상은 아니다.

이제는 death test를 구현할 수 있을 것이며 간단한 예시가 아래에 있다. 예제코드에서 `statement`들의 세부 구현은 생략헸다.

```c++
TEST(MyDeathTest, Foo) {
  // This death test uses a compound statement.
  ASSERT_DEATH({
    int n = 5;
    Foo(&n);
  }, "Error on line .* of Foo()");
}

TEST(MyDeathTest, NormalExit) {
  EXPECT_EXIT(NormalExit(), ::testing::ExitedWithCode(0), "Success");
}

TEST(MyDeathTest, KillMyself) {
  EXPECT_EXIT(KillMyself(), ::testing::KilledBySignal(SIGKILL),
	      "Sending myself unblockable signal");
}
```

위 코드는 아래 3가지를 검증한다.

*   `Foo(5)`를 호출하면 stderr로 `"Error on line .* of Foo()"`를 출력한 후에 "exit code != 0" 이 아닌 값으로 종료하는지 확인
*   `NormalExit()`를 호출하면 sdterr로 `"Success"`를 출력한 후에 "exit code == 0" 인 값으로 종료하는지 확인
*   `KillMyself()`를 호출하면 stderr로 `"Sending myself unblockable signal"`을 출력한 후에 `SIGKILL`시그널을 전달받아서 종료하는지 확인

`TEST()`, `TEST_F()`를 구현할 때 death test assertion과 다른 assertion 혹은 코드를 함께 사용하는 것도 물론 가능하다.

### Death Test 이름짓기

IMPORTANT: death test를 구현할 때는 **test suite**(not test case)을 `*DeathTest`라는 이름으로 작성하여 사용해야 한다. 그 이유는 [Death Tests And Threads](advanced.md#death-tests-그리고-threads)에 상세하게 설명되어 있다.

만약 사용자의 test fixture class가 normal test case와 death test case를 모두 포함한다면 `using` 혹은 `typedef`를 사용하여 alias를 만드는 것이 좋다. 아래 예제를 참고하자.

```c++
class FooTest : public ::testing::Test { ... };

using FooDeathTest = FooTest;

TEST_F(FooTest, DoesThis) {
  // normal test
}

TEST_F(FooDeathTest, DoesThat) {
  // death test
}
```

### 정규식 문법

POSIX system(e.g. Linux, Cygwin, Mac) 환경에서는 [POSIX extended regular expression](http://www.opengroup.org/onlinepubs/009695399/basedefs/xbd_chap09.html#tag_09_04) 문법을 사용한다. 관련 문법은 [Wikipedia entry](http://en.wikipedia.org/wiki/Regular_expression#POSIX_Extended_Regular_Expressions)서 확인할 수 있다.

Windows 환경에서는 googletest 자체적으로 간단한 문법을 구현하여 사용하고 있다. 따라서 기능이 약간 부족할 수도 있다. 예를 들어 (`"x|y"`), (`"(xy)"`), (`"[xy]"`), (`"{5,7}"`) 과 같은 문법은 지원하지 않고 있다. 아래 표에서 사용가능한 문법을 확인할 수 있다. (`A`와 `.`는 문자, `\\`은 escape sequnce, `x`, `y`는 정규식을 의미한다.)

| Expression | Meaning                                                      |
| ---------- | ------------------------------------------------------------ |
| `c`        | matches any literal character `c`                            |
| `\\d`      | matches any decimal digit                                    |
| `\\D`      | matches any character that's not a decimal digit             |
| `\\f`      | matches `\f`                                                 |
| `\\n`      | matches `\n`                                                 |
| `\\r`      | matches `\r`                                                 |
| `\\s`      | matches any ASCII whitespace, including `\n`                 |
| `\\S`      | matches any character that's not a whitespace                |
| `\\t`      | matches `\t`                                                 |
| `\\v`      | matches `\v`                                                 |
| `\\w`      | matches any letter, `_`, or decimal digit                    |
| `\\W`      | matches any character that `\\w` doesn't match               |
| `\\c`      | matches any literal character `c`, which must be a punctuation |
| `.`        | matches any single character except `\n`                     |
| `A?`       | matches 0 or 1 occurrences of `A`                            |
| `A*`       | matches 0 or many occurrences of `A`                         |
| `A+`       | matches 1 or many occurrences of `A`                         |
| `^`        | matches the beginning of a string (not that of each line)    |
| `$`        | matches the end of a string (not that of each line)          |
| `xy`       | matches `x` followed by `y`                                  |

사용자의 환경에서 어떤 정규식 표현이 가능한지 확인하기 위한 macro를 제공하고 있다. `#if` 를 사용해서 현재 시스템에 맞는 정규식을 구분해서 사용하자. 관련 macro는 아래와 아래와 같다.

`GTEST_USES_PCRE=1`, or `GTEST_USES_SIMPLE_RE=1` or `GTEST_USES_POSIX_RE=1`

### Death Test 의 동작방식

`ASSERT_EXIT()`는 내부적으로 새로운 process를 생성하고 death test assertion으로 전달된 `statement`를 수행하도록 되어 있다. 다만, 그 과정에서 사용자의 시스템이 POSIX인지 Windows인지에 따라 그 동작이 조금씩 달라진다. 또한, 환경변수 `::testing::GTEST_FLAG(death_test_style)`의 설정값에 따라서도 동작을 다르게 할 수 있다. 이 환경변수는 test program을 실행할 때 cmd line을 통해서 설정할 수 있다.(`--gtest_death_test_style`) 이렇게 설정된 환경변수를 death test style이라고 부르는데 현재는 2가지(`fast`와 `threadsafe`) style을 지원하고 있다. 아래는 사용자의 시스템 환경과 death test style에 따라 death test의 동작이 어떻게 다른지에 대한 설명이다.

*   POSIX 시스템에서는 child process를 만들기 위해 `fork()`(`clone()` on Linux)를 사용한다.
    *   만약, death test style이 `"fast"`라면 death test의 `statement`는 즉시 수행된다.
    *   만약, death test style이 `"threadsafe"`라면 test program 자체를 1개 더 실행시킨 후에 필요한 해당 death test를 실행한다.
*   Windows 시스템에서는 child process를 만들기 위해 `"CreateProcess()"` API를 사용한다. 그런 후에 해당 process를 실행시켜서 death test를 수행한다. 따라서 POSIX의 `"threadsafe"` 모드일 때와 유사하게 동작한다.

현재 death test style의 기본 설정값은 `"fast"`이다. Death test style에서 지원하지 않는(`fast`, `threadsafe`가 아닌) 다른 값을 지정하면 death test는 실패하게 된다. 마지막으로 death test의 성공조건 2가지를 다시 한 번 기억하고 넘어가자.

1.  child process의 exit status(code)는 `predicate`를 만족해야 한다.
2.  child process의 strerr 출력은 `matcher`와 동일해야 한다.

만약 death test의 `statement`가 문제없이 정상적으로 코드를 다 수행하고 종료되면 어떻게 될까? 일단 child process는 문제없이 잘 종료될 것이다. 즉, child process가 종료되는 건 보장된다. 단지 death test의 결과가 실패일 뿐이다.

### Death Test 그리고 Thread

Death test를 위해서는 test program의 child thread를 생성하는 작업이 꼭 필요하다. 따라서 death test는 thread safety가 보장되는 상황에서 제일 안정적으로 수행될 수 있다. 이를 위해서는 기본적으로 death test를 single-threaded 환경에서 실행하는 것이 제일 중요하다. 그러나 만약 애초에 single-threaded 환경이 불가능하면 어떻게 될까? 예를 들어서 main function이 실행되기 전에 생성되는 (정적으로 초기화하는) thread가 있다면 어떻게 해야 할까? 사실 그런 경우에 원래대로 돌아가기란 정말 쉽지 않다. Thread가 일단 하나라도 생성되었다면 parent process를 다시 처음 상태로 되돌리는 것은 매우 어려운 문제이다.

Googletest는 이런 상황을 대비해서 thread 관련 이슈를 사용자에게 최대한 전달하려고 노력한다.

1. Death test가 시작했을 때, 2개 이상의 thread가 실행중이라면 이에 대한 경고문을 출력한다.
2. *"DeathTest"*라는 이름을 가진 test suite을 다른 테스트들보다 먼저 실행해 준다.
3. Linux 환경에서는 `fork()` 대신 `clone()`을 사용해서 child process를 생성하도록 했다. 왜냐하면 multi-threaded 환경에서 `fork()`가 문제를 일으킬 확률이 더 크기 때문이다. (Cygwin이나 Mac에서는 `clone()`을 지원하지 않 때문에 fork()를 그대로 사용한다.)

위의 내용들과는 별개로 death test로 전달되는 `statement` 내부에서 child thread를 생성하는 것은 괜찮다. 아무런 문제가 되지 않는다.

### Death Test Style

Multi-threaded 환경에서 "threadsafe" 모드를 사용하면 테스트에 문제가 발생할 확률을 조금이나마 줄여준다. 테스트 실행 시간을 증가시킨다는 단점은 있지만 thread safety를 위해서는 좋은 선택이라고 할 수 있다.

만약, 사용자가 자동화된 테스트 환경을 사용 중이라서 cmd line flag를 전달할 수 없다면, 아래와 같이 소스코드에서 직접 설정하는 것도 가능하다.

```c++
testing::FLAGS_gtest_death_test_style="threadsafe"
```

위의 코드는 `main()`에 추가해도 되고 아니면 각각의 test case에 추가해도 된다. 단, 개별 test case에 설정한 값은 해당 test function에만 적용되며 function이 종료되면 원래 값(전역 값)으로 복구된다.

```c++
int main(int argc, char** argv) {
  InitGoogle(argv[0], &argc, &argv, true);
  ::testing::FLAGS_gtest_death_test_style = "fast";
  return RUN_ALL_TESTS();
}

TEST(MyDeathTest, TestOne) {
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  // This test is run in the "threadsafe" style:
  ASSERT_DEATH(ThisShouldDie(), "");
}

TEST(MyDeathTest, TestTwo) {
  // This test is run in the "fast" style:
  ASSERT_DEATH(ThisShouldDie(), "");
}
```

### Caveats

`ASSERT_EXIT()`로 전달되는 `statement`에는 어떤 C++ statement라도 구현할 수 있다. 다만, `statement`가 `return`으로 끝나거나 (`exit()`가 아니라) exception을 발생시키면 해당 death test는 실패로 간주된다. `ASSERT_TRUE()`와 같은 대다수의 googletest macro가 `return`으로 끝나는 것과 비교하면 확연히 구분되는 다른 점이다.

또한, `statement`는 child process(thread)에서 수행되기 때문에 메모리 관리에 있어서 주의가 필요하다. 알다시피 child process에서의 잘못된 동작을 parent process에서는 알 수 없기 때문이다. 만약 death test에서 메모리를 해제해버리면 parent process는 이를 알 수 없고, 따라서 heap checker가 이를 추적할 수 없기 때문에 테스트는 실패로 판정된다. 이를 위해서 아래와 같이 구현하기를 권장한다.

1.  death test statment(child process)에서 메모리를 해제하지 말라.
2.  child process에서 해제한 메모리를 parent process에서 다시 해제하지 말라.
3.  test program에서 heap checker를 사용하지 말라.

Death test 구현상의 특징으로 인해 소스코드 1줄에 여러개의 death test assertion을 사용할 수는 없다. 그렇게 구현하면 compile error가 발생할 것이다.

Death test를 "threadsafe"로 설정하는 것이 thread safety를 향상시키도록 도와주기는 하지만 deadlock과 같은 thread 사용에 따른 기본적인 이슈들은 여전히 사용자가 직접 꼼꼼하게 확인해야 한다. 이런 이슈는 `pthread_atfork(3)`핸들러가 등록되어 있다고 해도 여전히 발생할 수 있다.

## Sub-routines에서 Assertion을 사용하는 방법

### Assertions 추적하기

먼저, *sub-routine*은 기본적으로 C++의 일반적인 function을 뜻하며 test function 내부에서 호출하기 때문에 sub-routine이라고 표현하고 있다. 그럼 test function이 동일한 sub-routine을 여러번 호출한다고 가정해 보자. 그리고 테스트를 수행했더니 sub-routine에 구현된 assertion이 실패했다고 하자. 이런 상황에서는 어떤 호출에서 실패했는지 어떻게 구분할 수 있을까?

물론 단순하게 로그를 추가하거나 failure message를 변경해서 확인해도 되지만 그러한 방법은 테스트를 복잡하게 만드는 원인이 된다. 이런 상황에서는 `SCOPED_TRACE`와 `ScopedTrace`를 사용하는 것을 추천한다.

```c++
SCOPED_TRACE(message);
ScopedTrace trace("file_path", line_number, message);
```

먼저, `SCOPED_TRACE`는 현재 실행중인 file name, line number, failure message를 출력해 준다. 이 때, `SCOPED_TRACE`로 전달되는 argument인 `message`는 `std::ostream`으로 출력가능한 값이여야 하며 그 내용이 default failure message에 더해져서 출력되게 된다. 다음으로 `ScopedTrace`는 `SCOPE_TRACE`가 출력해주는 file name, line number와 같은 내용을 보다 명시적으로 전달하고 싶을 때 사용한다. 이러한 방법은 test helper를 구현할 때 유용하다. 마지막으로 scope을 벗어나면 적용했던 내용들도 해제될 것이다.

아래는 예제코드이다.

```c++
10: void Sub1(int n) {
11:   EXPECT_EQ(Bar(n), 1);
12:   EXPECT_EQ(Bar(n + 1), 2);
13: }
14:
15: TEST(FooTest, Bar) {
16:   {
17:     SCOPED_TRACE("A");  // This trace point will be included in
18:                         // every failure in this scope.
19:     Sub1(1);
20:   }
21:   // Now it won't.
22:   Sub1(9);
23: }
```

위 예제는 아래와 같은 failure message를 만들어 낸다. `SCOPED_TRACE`가 사용된 첫번째 실패에서 `path/to/foo_test.cc:17: A`라는 출력문이 추가된 것을 확인할 수 있다.

```none
path/to/foo_test.cc:11: Failure
Value of: Bar(n)
Expected: 1
  Actual: 2
   Trace:
path/to/foo_test.cc:17: A

path/to/foo_test.cc:12: Failure
Value of: Bar(n + 1)
Expected: 2
  Actual: 3
```

이러한 기능을 적용하지 않았다면 `Sub1()`이 동일한 failure message를 출력하기 때문에 어떤 호출에서 실패한 것인지 구별할 수가 없다. 물론 `Sub1()`에서 사용중인 assertion에 `n`도 같이 출력하도록 수정해도 되겠지만 assertion의 개수가 많아지면 해야할 일도 많아지기 때문에 불편할 것이다.

아래에 `SCOPED_TRACE`를 사용할 때의 유용한 팁을 공유한다.

1.  Sub-routine을 호출할 때마다 `SCOPE_TRACE`를 사용하는 것보다는 아예 sub-routine의 시작부분에 구현하는 것이 편리할 수 있다.
2.  반복문에서 sub-routine을 호출한다면 `message`에 iterator를 포함시켜서 어떤 호출에서 failure가 발생했는지 구분할 수 있도록 하자.
3.  추가적인 failure message 없이 file name, line number만 알아도 충분하다면 `message`에 빈 문자열(`""`)을 전달하면 된다.
4.  여러 개의 scope가 중첩(nested)되어도 괜찮다. 이런 경우에는 중첩된 scope의 failure message를 모두 출력해 준다. 순서는 안쪽에 있는 scope부터 출력됩니다.
5.  Emacs를 사용한다면 trace dump를 확인할 수 있다. 해당 line number에서 `return` 키를 누르면 바로 소스파일로 이동한다.

### Sub-routine에서 발생한 Fatal Failures를 test function(혹은 caller)에 알려주기

`ASSERT_*` 나 `FAIL*`가 실패했을 때, 수행중인 test case를 중단하는 것이 아니라 *현재 수행중인 function만* 종료된다는 것을 기억하자. 즉, sub-routine에서 fatal failure가 발생해도 해당 sub-routine만 종료될 뿐이지 상위 test function은 종료되지 않음을 의미한다. 아래 예제는 이로 인해 발생할 수 있는 문제(여기서는 segfault) 중 하나를 보여준다.

```c++
void Subroutine() {
  // Generates a fatal failure and aborts the current function.
  ASSERT_EQ(1, 2);

  // The following won't be executed.
  ...
}

TEST(FooTest, Bar) {
  Subroutine();  // The intended behavior is for the fatal failure
		 // in Subroutine() to abort the entire test.

  // The actual behavior: the function goes on after Subroutine() returns.
  int* p = NULL;
  *p = 3;  // Segfault!
}
```

위의 `Subroutine()` function에 사용된 assertion은 해당 test case(`TEST(FooTest, Bar)`가 종료되기를 바라고 구현한 것이다. 그러나 실제로는 `int *p = NULL` 까지가 모두 수행되며 이는 곧 segfault를 유발하게 된다. 이런 문제를 해결하기 위해서 googletest는 3가지 해결방법을 제공하고 있다. 첫 번째는 exception, 두 번째는 `ASSERT_NO_FATAL_FAILURE / EXPECT_NO_FATAL_FAILURE` 계열의 assertion, 세 번째는 `HasFatalFailure()` 이다. 이제 각각에 대해서 자세하게 알아보자.

#### Sub-routine의 Assertion을 Exception처럼 사용하기

아래와 같이 `ThrowListener`를 등록해 놓으면 fatal failure를 exception으로 변경해준다.

```c++
class ThrowListener : public testing::EmptyTestEventListener {
  void OnTestPartResult(const testing::TestPartResult& result) override {
    if (result.type() == testing::TestPartResult::kFatalFailure) {
      throw testing::AssertionException(result);
    }
  }
};
int main(int argc, char** argv) {
  ...
  testing::UnitTest::GetInstance()->listeners().Append(new ThrowListener);
  return RUN_ALL_TESTS();
}
```

주의 할 점은 다른 listener들이 모두 등록된 후에 `ThrowListener`를 등록해야 한다는 것이다. 그렇지 않으면 의도한대로 동작하지 않는다.

#### Sub-routine의 Asserting 알아내기

위에서 확인했듯이 test function(test case)이 sub-routine을 호출하는 구조에서 sub-routine에서 발생한 fatal failure를 test function에서는 알 수가 없고, 이로 인해서 sub-routine에서 fatal failure가 발생하더라도 test function 자체는 종료되지 않았다. 그러나 `ASSERT_*` 계열의 assertion은 해당 test function을 종료시키는 목적으로 사용하는 것이 맞기 때문에 뭔가 좀 어색했던 것도 사실이다.

역시나 많은 사람들이 이러한 문제제기를 했다. 이에 googletest는 sub-routine에서 발생한 fatal failure를 마치 exception처럼 상위 function으로 전달해주는 기능을 추가했다.

| Fatal assertion                       | Nonfatal assertion                    | Verifies                                                     |
| ------------------------------------- | ------------------------------------- | ------------------------------------------------------------ |
| `ASSERT_NO_FATAL_FAILURE(statement);` | `EXPECT_NO_FATAL_FAILURE(statement);` | `statement` doesn't generate any new fatal failures in the current thread. |

어떠한가? 다만, 위의 macro를 사용한다고 해도 현재 thread에서 발생하는 fatal failure에 대해서만 유효하다. 예를 들어 `statement`가 새로운 thread를 생성하고 그 thread에서 fatal failure가 발생하는 경우에는 확인할 수 없다.

아래는 예제코드이다.

```c++
ASSERT_NO_FATAL_FAILURE(Foo());

int i;
EXPECT_NO_FATAL_FAILURE({
  i = Bar();
});
```

참고사항으로 Windows 환경에서는 multiple threads에서의 assertion사용 자체를 (현재는) 지원하지 않고 있다.

#### 현재 테스트에서 발생한 Failures를 확인하기

`::testing::Test` class의 `HasFatalFailure()` function은 수행중인 test case에서 fatal failure가 발생했다면 `true`를 반환해 준다. 이를 통해 sub-routine의 fatal failure를 확인하고 원하는 조치를 할 수 있도록 도와준다.

```c++
class Test {
 public:
  ...
  static bool HasFatalFailure();
};
```

일반적인 사용법은 아래와 같다.

```c++
TEST(FooTest, Bar) {
  Subroutine();
  // Aborts if Subroutine() had a fatal failure.
  if (HasFatalFailure()) return;

  // The following won't be executed.
  ...
}
```

만약 `TEST()`, `TEST_F()`, test fixture 외의 장소에서 `HasFatalFailure()`을 사용하려면 아래처럼 `::testing::Test::` prefix를 사용해야 한다.

```c++
if (::testing::Test::HasFatalFailure()) return;
```

동일한 방법으로 `HasNonfatalFailure()`는 non-fatal failure가 발생한 경우에 `true`를 반환해주며 `HasFailure()`은 fatal, non-fatal 관계 없이 failure가 발생한 경우를 확인할 수 있도록 도와준다.

## 추가정보 기록하기

이 문서의 앞 부분에서 message 혹은 failure message를 변경하는 방법에 대해서 배운 적이 있다. 이에 더해서 조금 다른 방법으로 추가정보를 기록하는 것도 가능하다. 예를 들면 test program을 실행할 때 `--gtest_output="xml"`과 같은 cmd line flag를 전달해서 [XML output](#xml-report-출력하기)으로 출력하는 것도 가능하다. 이런 경우를 위해서 제공되는 function은 `RecordProperty("key", value)`이다. `value`에는 `string` 이나 `int` 타입을 사용할 수 있다. `key`는 말 그대로 `value`를 구분하는 key(구분자)로 사용된다. 관련예제를 아래에서 확인하자.

```c++
TEST_F(WidgetUsageTest, MinAndMaxWidgets) {
  RecordProperty("MaximumWidgets", ComputeMaxUsage());
  RecordProperty("MinimumWidgets", ComputeMinUsage());
}
```

위의 테스트코드가 실행되면 XML파일에 아래와 같은 내용이 추가된다.

```xml
  ...
    <testcase name="MinAndMaxWidgets" status="run" time="0.006" classname="WidgetUsageTest" MaximumWidgets="12" MinimumWidgets="9" />
  ...
```

> NOTE:
>
> *   `RecordProperty()`는 `Test`라는 class의 static member function이다. 따라서 `TEST`, `TEST_F`, 혹은 test fixture class에서 사용하는 것이 아니라면 `::testing:Test::`라는 prefix를 붙여줘야만 한다.
> *   `key`는 XML attribute로서 유효한 이름으로 지정해야 한다. 또한, googletest에서 이미 사용중인 attribute도 사용해서는 안 된다. (이미 사용중인 attribute : `name`, `status`, `time`, `classname`, `type_param`, `value_param`)
> *   `RecordProperty()`를 test case가 아니라 test suite에서 사용한다면 (즉, `SetUpTestSuite()`이 호출되고 `TearDownTestSuite()`호출되기까지의 구간) XML 결과물에도 test suite의 정보로 기록된다. 같은 맥락에서 test suite의 바깥에서 사용한다면 XML 결과물에도 top-level element 정보로 기록된다.

## Test Suite의 자원을 Test Case간에 공유하기

Googletest는 개별 test case를 실행할 때마다 test fixure object를 새로 생성한다. 그 이유는 각 test case를 독립된 환경에서 수행하면 안정성을 확보할 수 있고 동시에 문제가 발생했을 때 디버깅에도 도움이 되기 때문이다. 이러한 방법을 one-copy-per-test model이라고도 한다. 이 model의 한 가지 단점은 자원이 많은 경우에는 이들을 매번 set-up 하는게 부담이 될 수도 있다는 것이다.

만약 test case들이 자원을 변경하지 않는다면 여러 test case들이 동일한 자원을 공유하게 해도 문제가 없다. 이런 이유로 per-test set-up/tear-down에 더해 per-test-suite set-up/tear-down 기능도 제공하고 있으며 사용법은 아래와 같다.

1.  `FooTest`라는 test fixture class가 있다고 가정하자. 공유해야 하는 자원(변수)을 `static`으로 선언한다.
2.  해당 `static` 변수를 초기화한다. (C++에서 `static` 멤버변수는 class 바깥에서도 초기화해야 한다.)
3.  이제 test fixture class에 `static void SetUpTestSuite()` 과 `static void TearDownTestSuite()`을 구현한다. 두 함수 내부에는 공유자원을 위한 초기화 작업, 정리 작업을 구현하면 된다. (`SetupTestSuite`이 아니라 대문자 *`Up`*인 점을 유의하자.)

이제 준비는 끝났다. googletest는 해당 test fixture(`FooTest`)의 첫번째 test case을 수행하기 전에 `SetUpTestSuite()`을 호출한다. 그리고 마지막 test case를 수행한 후에는 `TearDownTestSuite()`을 호출할 것이다. 이와 같은 방법을 통해 `FooTest`에 포함된 test case끼리 자원을 공유할 수 있게 된다.

단, 현재 test case가 수행되는 순서는 정하지 않았다. 따라서 각각의 test case는 서로 의존성이 없어야 하며 공유자원을 변경해서도 안 된다. 만약 공유자원을 꼭 변경해야 한다면 해당 test case가 끝나기 전에 원래대로 복구시켜야 한다.

아래는 지금까지 설명한 per-test-suite에 대한 set-up/tear-down을 구현한 예제이다.

```c++
class FooTest : public ::testing::Test {
 protected:
  // Per-test-suite set-up.
  // Called before the first test in this test suite.
  // Can be omitted if not needed.
  static void SetUpTestSuite() {
    shared_resource_ = new ...;
  }

  // Per-test-suite tear-down.
  // Called after the last test in this test suite.
  // Can be omitted if not needed.
  static void TearDownTestSuite() {
    delete shared_resource_;
    shared_resource_ = NULL;
  }

  // You can define per-test set-up logic as usual.
  virtual void SetUp() { ... }

  // You can define per-test tear-down logic as usual.
  virtual void TearDown() { ... }

  // Some expensive resource shared by all tests.
  static T* shared_resource_;
};

T* FooTest::shared_resource_ = NULL;

TEST_F(FooTest, Test1) {
  ... you can refer to shared_resource_ here ...
}

TEST_F(FooTest, Test2) {
  ... you can refer to shared_resource_ here ...
}
```

NOTE: 위 코드는 `SetUpTestSuite()`을 protected로 선언했지만 public으로 선언해야 하는 경우도 있다. 예를 들면 `TEST_P`와 같은 macro를 사용하기 위해서는 public으로 선언하게 된다.

## Global Set-Up, Tear-Down

개별 test case 혹은 test suite에서 set-up 및 tear-down을 설정하는 방법에 대해서 알아보았다. googletest는 test program 레벨에서도 이러한 설정이 가능하도록 지원하고 있다. 즉, 전역적인 set-up, tear-down 구현이 가능하다.

먼저, `::testing::Environment`라는 class를 상속받아서 test envrionment를 정의해야 한다. Test environment는 전역적인 set-up, tear-down 설정을 도와준다.

```c++
class Environment : public ::testing::Environment {
 public:
  virtual ~Environment() {}

  // Override this to define how to set up the environment.
  void SetUp() override {}

  // Override this to define how to tear down the environment.
  void TearDown() override {}
};
```

다음으로 위에서 상속받은 class의 object를 생성하고 `::testing:AddGlobalTestEnvironment()`을 통해 등록한다.

```c++
Environment* AddGlobalTestEnvironment(Environment* env);
```

이제 `RUN_ALL_TESTS()`를 호출하면 등록된 envrinonment object의 `SetUp()`이 호출 될 것이다. `SetUp`이 호출되는 동안 fatal failure가 발생하지 않고 `GTEST_SKIP()`도 호출되지 않았다면 계속해서 모든 테스트를 수행하게 된다. 그리고 `RUN_ALL_TESTS()`가 호출되면 `TearDown()`도 무조건 같이 호출해 준다. (테스트의 수행여부와 관계없이 호출된다.)

Environment object를 여러개 등록하는 것도 허용된다. 그런 경우에 `SetUp()`은 등록된 순서대로 호출되며 `TearDown()`은 그 반대 순서로 호출된다.

Environment object가 등록되었다면 그 소유권은 googletest로 옮겨지므로 사용자가 **삭제할 수 없다.**

`ADDGlobalTestEnvironment()`는 `RUN_ALL_TESTS()`보다 먼저 호출해야 한다. `RUN_ALL_TESTS()`는 일반적으로 `main()`에 구현하기 때문에 만약 사용자가 `gtest_main` 라이브러리를 `main()` 대신 사용하고 있다면 이러한 수정이 어려울 것이다. 이렇게 `gtest_main` 라이브러리를 사용하는 사용자는 전역 변수를 사용하면 동일한 기능을 사용할 수 있다.

```c++
::testing::Environment* const foo_env =
    ::testing::AddGlobalTestEnvironment(new FooEnvironment);
```

위와 같은 방법이 있긴 하지만 `AddGlobalTestEnvironment()`를 사용하려는 사용자는 `main()`을 직접 구현하기를 권장한다. 1차적인 이유는 전역변수가 소스코드 가독성을 떨어트리기 때문이며 이에 더해 environment를 여러개 사용하면서 각각의 전역변수를 여러 파일에 분산시켜 놓는다면 문제가 발생할 수 있기 때문이다. 알다시피 C++ compiler의 전역변수 초기화 순서는 예측할 수 없으며 그로 인해 environment 간에 의존성이 필요한 경우에 목적하는 바를 달성하기 어려워질 수도 있다.

## Value-Parameterized Tests

*Value-parameterized tests*를 적용하면 하나의 테스트 코드에 데이터(parameter)만 변경해가면서 다양한 테스트를 수행할 수 있다. 특히 아래와 같은 경우에 유용하다.

- cmd line flag에 따라 동작이 달라져야 하기 때문에 각각 flag를 모두 검증해야 할 때
- interface(abstract class)를 구현한 implementation class가 여러개 있을 때
- 그 외 다양한 입력들에 대해서도 문제없이 잘 동작하는지 확인하고 싶을 때 (data-driven testing이라고도 불리우는 이 방법은 꼭 필요하지 않은데도 사용하는 경우가 자주 발생하기 때문에 남용하지 않도록 유의하자.)

### Value-Parameterized Tests를 구현하는 방법

Value-parameterized tests를 구현하려면 먼저 fixture class를 만들어야 하며 이를 위해서는 `::testting::Test`과 `::testing::WithParamInterface<T>`를 함께 상속받아야 한다. 특히 `::testing::WithParamInterface<T>`는 abstract class이며 `T`는 parameter로 전달되는 값의 타입을 의미한다. 좀 복잡하게 느껴질지도 모르겠다. 사실은 위의 2가지를 상속받아서 구현된 `::testing::TestWithParam<T>`를 이미 제공하고 있다. Template parameter `T`는 복사가 가능한 타입이라면 어떤 타입이라도 괜찮다. 다만, raw pointer를 사용한다면 가리키는 대상의 관리도 사용자가 직접 해야하는 부분은 주의해야 한다.

NOTE: 만약 value-parameterized tests의 test fixture class에 `SetUpTestSuite()` 또는 `TearDownTestSuite()`을 사용하려면 **protected**가 아닌 **public** 영역에 선언해야 한다. 그래야만 `TEST_P`를 사용할 수 있다.

```c++
class FooTest :
    public testing::TestWithParam<const char*> {
  // You can implement all the usual fixture class members here.
  // To access the test parameter, call GetParam() from class
  // TestWithParam<T>.
};

// Or, when you want to add parameters to a pre-existing fixture class:
class BaseTest : public testing::Test {
  ...
};
class BarTest : public BaseTest,
		public testing::WithParamInterface<const char*> {
  ...
};
```

위의 코드에서 `FooTest`는 `TestWithParam<T>`만 상속받아서 손쉽게 value-parameterized test를 위한 test fixture class를 구현했다. 다만, 이렇게 test fixture를 새로 만드는 것이 아니라 이미 사용중인 test fixture를 value-parameterized test로 확장하고 싶다면 `BaseTest`, `BarTest`처럼 구현하면 된다. 여기까지 해서 test fixture가 준비되었다면 `TEST_P`를 사용해서 test function을 정의하면 된다. 여기서 `TEST_P`의 `_P`는 "parameterized" 또는 "pattern"을 의미한다. 의미만 통한다면 어느 쪽으로 생각하든 괜찮다.

```c++
TEST_P(FooTest, DoesBlah) {
  // Inside a test, access the test parameter with the GetParam() method
  // of the TestWithParam<T> class:
  EXPECT_TRUE(foo.Blah(GetParam()));
  ...
}

TEST_P(FooTest, HasBlahBlah) {
  ...
}
```

이제 거의 다 왔다. 테스트를 위한 데이터(parameter)를 정의해보자. 이를 위해서 `INSTANTIATE_TEST_SUITE_P`를 사용한다. googletest는 `INSTANTIATE_TEST_SUITE_P`와 함께 사용하여 parameter 생성을 도와주는 여러가지 function도 함께 제공한다. 이러한 function을 *parameter generator*라고 부르며 아래 표에서 관련 내용을 확인하자. (이들은 모두 `testing` namespace에 정의되어 있다.)

| Parameter Generator                             | Behavior                                                     |
| ----------------------------------------------- | ------------------------------------------------------------ |
| `Range(begin, end [, step])`                    | Yields values `{begin, begin+step, begin+step+step, ...}`. The values do not include `end`. `step` defaults to 1. |
| `Values(v1, v2, ..., vN)`                       | Yields values `{v1, v2, ..., vN}`.                           |
| `ValuesIn(container)` and `ValuesIn(begin,end)` | Yields values from a C-style array, an STL-style container, or an iterator range  `[begin, end)`. |
| `Bool()`                                        | Yields sequence `{false, true}`.                             |
| `Combine(g1, g2, ..., gN)`                      | Yields all combinations (Cartesian product) as std\:\:tuples of the values generated by the `N` generators. |

보다 자세한 내용은 해당 function의 주석을 확인하자.

아래 예제는 test suite(`FooTest`)을 위한 value-parameter 3개(`"meeny"`, `"miny"`, `"moe"`)를 초기화하고 있다. 이제 총 6개의 서로 다른 test function이 자동으로 생성될 것이다. (`TEST_P` 개수 x parameter 개수)

```c++
INSTANTIATE_TEST_SUITE_P(InstantiationName,
			 FooTest,
			 testing::Values("meeny", "miny", "moe"));
```

NOTE: 위 코드는 전역범위나 특정 namespace에 포함되어야 한다. Function 내부에서는 사용할 수 없다.

NOTE: 이전까지 아무리 잘 구현했어도 `INSTANTIATE_TEST_SUITE_P`를 사용하지 않으면 말짱 도루묵이다. 테스트가 실제로 수행되지도 않고 성공한 것처럼 보이기도 하니 주의해야 한다.

동일한 test suite에 대해서 `INSTANTIATE_TEST_SUITE_P`를 여러번 초기화하는 것도 가능하기 때문에 이들을 구분 할 방법이 필요한데 이 때 사용되는 값이 `INSTANTIATE_TEST_SUITE_P`의 첫번째 argument이다. 따라서 첫 번째 파라미터는 중복되면 안 된다. 아래는 지금까지 만들어 본 6개의 test function의 이름과 각각에 사용할 value parameter를 보여준다.

*   `InstantiationName/FooTest.DoesBlah/0` for `"meeny"`
*   `InstantiationName/FooTest.DoesBlah/1` for `"miny"`
*   `InstantiationName/FooTest.DoesBlah/2` for `"moe"`
*   `InstantiationName/FooTest.HasBlahBlah/0` for `"meeny"`
*   `InstantiationName/FooTest.HasBlahBlah/1` for `"miny"`
*   `InstantiationName/FooTest.HasBlahBlah/2` for `"moe"`

위의 test function들에 대해서도 [`--gtest_filter`](#전체-test-중에서-일부만-수행하기)를 사용해서 원하는 테스트만 수행할 수 있다.

아래 예제는 test suite(`FooTest`)을 테스트하기 위한 데이터(parameter)를 추가하고 있다. 이 때 기존에 생성된 test function들과의 구분을 위해 `INSTANTIATE_TEST_SUITE_P`의 첫 번째 파라미터를 다르게 했음을 확인할 수 있다.

```c++
const char* pets[] = {"cat", "dog"};
INSTANTIATE_TEST_SUITE_P(AnotherInstantiationName, FooTest,
			 testing::ValuesIn(pets));
```

위의 코드를 통해 생성된 4개의 test function은 아래와 같다.

*   `AnotherInstantiationName/FooTest.DoesBlah/0` for `"cat"`
*   `AnotherInstantiationName/FooTest.DoesBlah/1` for `"dog"`
*   `AnotherInstantiationName/FooTest.HasBlahBlah/0` for `"cat"`
*   `AnotherInstantiationName/FooTest.HasBlahBlah/1` for `"dog"`

지금까지 본 것처럼 `INSTANTIATE_TEST_SUITE_P`는 `TEST_P` macro를 통해 정의한 *모든* test case에 대해 각 parameter마다 별도의 test function들을 생성해준다. 즉 `TEST_P` x parameter 개수 만큼의 test function이 생성된다. 이 때, `INSTANTIATE_TEST_SUITE_P`와 `TEST_P`의 구현순서는 중요하지 않다.

좀 더 자세한 예제가 필요하다면 [sample7_unittest.cc](../../../samples/sample7_unittest.cc), [sample8_unittest.cc](../../../samples/sample8_unittest.cc)에서 확인 가능하다.

### Value-Parameterized Abstract Tests 생성하기

위에서 하나의 test suite(`FooTest`)을 여러 데이터(parameter)를 통해 검증하는 방법에 대해 알아봤다. 이러한 value-parameterized tests를 라이브러리에 포함시켜서 다른 사람들이 사용할 수 있도록 공유하는 것도 좋은 방법이다. 또 이러한 방법론을 *abstract test*라고 부른다. 어떤 interface를 설계하면서 그 interface를 테스트하기 위한 코드도 제공한다면 interface 사용자들에게 도움이 될 것이다. Interface를 구현하게 될 다양한 사용자를 아우르려면 아무래도 일반적인 관점에서 구현되어야 하기 때문에 standard suite of abstract tests를 제공해서 interface의 implementation class들을 검증할 수 있도록 해야 한다. Standard suite of abstract tests를 구현하는 방법의 하나로는 factory function(test parameter을 전달받으면 test case를 생성해주는)을 제공하는 것도 좋다. 결론적으로 상위 interface의 test suite(abstract tests)을 만들어서 하나의 test suite만 가지고도 해당 interface를 구현한 implementation class들을 검증하는데 공통적으로 사용하려는 것이다. 이렇게 설계가 된다면 특정 implementation class를 위한 test suite을 만들 때 abstract test를 상속받기만 하면 쉽게 만들 수 있다.

Abstract test를 정의하기 위해서는 아래와 같이 코드를 구성하면 된다.

1.  Abstract test를 *선언한다.* 예를 들어 `FooTest`라는 test fixture class를 `foo_param_test.h`라는 헤더파일에 저장한다.
1.  Abstract test를 *구현한다.* 이 때는 `foo_param_test.cc`와 같은 소스파일을 만들고 헤더파일(`foo_param_test.h`)을 포함시킨다. 내부에는 `TEST_P`를 사용해서 test case를 구현한다.

이렇게 abstract test의 구현이 완료되면 라이브러리로 배포한다. 그럼 사용자는 abstract test 라이브러리를 링크하고 코드에는 `#include foo_param_test.h` 를 포함시킨다. 그러면 끝이다. 이제 `INSTANTIATE_TEST_SUITE_P()`를 사용해서 원하는 데이터를 가지고 test function을 생성할 수 있다. 이를 통해 abstract test suite 라이브러리만 포함하면 자신이 필요한 value parameter를 가지고 test case를 생성할 수 있게 된다. 위에서도 확인한 내용이지만 `INSTANTIATE_TEST_SUITE_P()`는 첫번째 argument만 다르다면 동일한 test suite에다가 여러번 사용하는 것도 가능하다. (소스파일이 서로 다르면 더욱 좋다.)

### Value-Parameterized Test Parameters 이름짓기

지금까지는 `INSTANTIATE_TEST_SUITE_P()`에 3개의 argument만 사용했지만 필요하다면 4번째 argument도 사용할 수 있다. 4번째 argument에는 function(또는 functor)을 전달할 수 있으며 그 목적은 test function의 이름을 만들 때 parameter를 사용하기 위함이다. (3개의 argument만 사용하면 0,1,2와 같은 숫자를 통해 test function의 이름이 정해진다.) 전달되는 function(또는 functor)은 `testing::TestParamInfo<class ParamType>` 타입의 값을 전달받아서 `std::string` 타입을 반환하는 function이어야 한다.

Googletest는 이러한 4번째 argument를 쉽게 사용할 수 있도록 `test::PrintToString(GetParam())`의 값을 반환하는`testing::PrintToStringParamName`이라는 built-in test suffix generator를 제공하고 있다. 이것은 `std:string`이나 C string에는 사용할 수 없다.

NOTE: 모든 test case(혹은 test function)의 이름은 유일해야 하고 ASCII 문자나 숫자만 포함할 수 있다. 당연하지만 값이 없어도 안 된다. 마지막으로 [should not contain underscores](faq.md#test-case-또는-test-suite의-이름을-정할-때-밑줄을-사용하면-안되는-이유가-뭔가요)에 명시된 것처럼 밑줄(`_`)은 사용할 수는 있지만 최대한 지양해야 한다.

아래는 4번째 argument에 built-in test suffix generator를 사용한 예제이다.

```c++
class MyTestSuite : public testing::TestWithParam<int> {};

TEST_P(MyTestSuite, MyTest)
{
  std::cout << "Example Test Param: " << GetParam() << std::endl;
}

INSTANTIATE_TEST_SUITE_P(MyGroup, MyTestSuite, testing::Range(0, 10),
			 testing::PrintToStringParamName());
```

이에 반해서 아래 코드처럼 custom function(functor)을 직접 구현해서 사용하면 test parameter name을 보다 세밀하게 조작할 수도 있다. 물론, built-in test suffix generator가 나쁜 것은 아니지만 사용자가 입력한 parameter를 `std::string`으로 형변환하기 때문에 사용성이 조금 떨어지는 경우가 발생하기도 한다. 아래 예제는 parameter, enumeration type, string을 가지고 test parameter name을 사용자가 직접 생성하는 코드를 보여준다. (코드를 간결하게 하기 위해 labmda를 사용했다.)

```c++
enum class MyType { MY_FOO = 0, MY_BAR = 1 };

class MyTestSuite : public testing::TestWithParam<std::tuple<MyType, string>> {
};

INSTANTIATE_TEST_SUITE_P(
    MyGroup, MyTestSuite,
    testing::Combine(
	testing::Values(MyType::VALUE_0, MyType::VALUE_1),
	testing::ValuesIn("", "")),
    [](const testing::TestParamInfo<MyTestSuite::ParamType>& info) {
      string name = absl::StrCat(
	  std::get<0>(info.param) == MY_FOO ? "Foo" : "Bar", "_",
	  std::get<1>(info.param));
      absl::c_replace_if(name, [](char c) { return !std::isalnum(c); }, '_');
      return name;
    });
```

## Typed Tests

동일한 요구사항을 수행하기 위한 interface가 있고 이를 위한 implementation class가 여러개 있다고 가정해 보자. 이 때, implementation class는 하나의 타입으로도 볼 수 있기 때문에 위의 상황은 결국 같은 동작을 여러가지 타입에 대해서 수행한다는 의미와 동일하다. 특히, 이 경우에는 상위 interface가 동일하기 때문에 해당 interface에 대한 테스트가 다양한 타입(implementation class)을 커버할 수 있다면 좋을 것이다. 이러한 테스트 방법을 *typed tests*라고 합니다. 바로 위에서 설명한 value-parameterized tests와  조금 헷갈릴 수 있는데 value-parameterized tests가 하나의 테스트를 가지고 여러가지 값(parameter)을 검증했다면, typed tests는 하나의 테스트를 가지고 여러가지 타입(type)을 검증하는 것이다.

물론 `TEST` 또는 `TEST_F`를 템플릿으로 만들거나 여러가지 타입에 대해서 모두 동작하도록 여러개를 구현해도 된다. 단, 그렇게 되면 `m`개의 test case를 `n`개의 타입에 대해서 수행하려 할 때 `m*n`개의 `TEST`를 구현하게 될 수도 있다. 이런 작업은 힘들고 그 결과에 대한 유지보수성도 떨어진다.

*Typed tests*는 이러한 내용을 보다 쉽게 구현할 수 있도록 도와준다. 검증할 대상타입이 무엇인지 미리 알아야 한다는 단점은 있지만 구현을 비롯한 다른 측면에서는 더 좋다.

이를 위해서는 먼저 test fixture class를 구현한다. 이 때, `::testing::Test`를 상속받는 부분은 일반적인 test fixture와 동일하지만 template class으로 선언해야 한다.

```c++
template <typename T>
class FooTest : public ::testing::Test {
 public:
  ...
  typedef std::list<T> List;
  static T shared_;
  T value_;
};
```

이제 위에서 정의한 test fixture class에 검증하고자 하는 타입들을 연결한다. 구현방법은 아래와 같다.

```c++
using MyTypes = ::testing::Types<char, int, unsigned int>;
TYPED_TEST_SUITE(FooTest, MyTypes);
```

`TYPED_TEST_SUITE`의 2번째 argument에는 여러가지 타입을 묶어서 전달한다. 이 때 주의해야 할 점은 `using`이나 `typedef`를 통해서 alias를 만들고 그 alias를 전달해야 한다는 것이다. 그렇지 않으면 compiler가 comma(`,`)를 제대로 해석하지 못한다.

이제 test case를 정의할 시간이다. Typed tests에서는 `TEST_F()` 대신에 `TYPED_TEST()`를 사용해서 test case를 구현한다. 관련 예제코드는 아래와 같다.

```c++
TYPED_TEST(FooTest, DoesBlah) {
  // Inside a test, refer to the special name TypeParam to get the type
  // parameter.  Since we are inside a derived class template, C++ requires
  // us to visit the members of FooTest via 'this'.
  TypeParam n = this->value_;

  // To visit static members of the fixture, add the 'TestFixture::'
  // prefix.
  n += TestFixture::shared_;

  // To refer to typedefs in the fixture, add the 'typename TestFixture::'
  // prefix.  The 'typename' is required to satisfy the compiler.
  typename TestFixture::List values;

  values.push_back(n);
  ...
}

TYPED_TEST(FooTest, HasPropertyA) { ... }
```

이상의 내용이 도움되었기를 바라며 더 자세한 예제코드가 필요한 사람은 [sample6_unittest.cc](../../../samples/sample6_unittest.cc)에서도 확인할 수 있다.

## Type-Parameterized Tests

*Type-parameterized tests*는 typed tests와 유사하지만 필요한 타입을 미리 지정할 필요가 없다는 점이 다르다. 테스트만 미리 구현해 놓은 상태에서 타입은 나중에 지정해도 괜찮다. 또, 원한다면 여러번 지정하는 것도 가능하다.

Typed tests에서도 이야기했지만, interface를 설계하고 있다면 해당 interface를 상속받게 될 다양한 타입(implementation class)을 검증하는 방법도 준비하는 것이 좋다. 왜냐하면 각 타입에 대한 테스트를 중복해서 구현하지 않아도 되기 때문이다. 그럼 이제부터 type-parameterized tests를 사용하는 방법에 대해 알아보자.

먼저 typed tests에서 했던 것처럼 test fixture class를 template class으로 정의한다.

```c++
template <typename T>
class FooTest : public ::testing::Test {
  ...
};
```

다음으로 위에서 만든 template class가 type-parameterized test임을 googletest에 알려준다.

```c++
TYPED_TEST_SUITE_P(FooTest);
```

이제 test case를 구현하자. 일반적인 과정은 기존과 동일하다. 단, 이제까지 써왔던 macro(`TEST`, `TEST_F`, `TYPED_TEST`)가 아니라 `TYPED_TEST_P()`라는 또 다른 macro를 사용해야 한다.

```c++
TYPED_TEST_P(FooTest, DoesBlah) {
  // Inside a test, refer to TypeParam to get the type parameter.
  TypeParam n = 0;
  ...
}

TYPED_TEST_P(FooTest, HasPropertyA) { ... }
```

다음으로 위에서 구현한 test suite과 test case들을 `REGISTER_TYPED_TEST_SUITE_P`를 통해서 연결한다. Macro의 첫 번째 argument는 test suite 이름이고 그 다음부터는 각 test case들의 이름을 하나하나 적어준다. 관련 코드는 아래와 같다.

```c++
REGISTER_TYPED_TEST_SUITE_P(FooTest,
			    DoesBlah, HasPropertyA);
```

여기까지 했으면 모든 사용준비가 끝났다. 위의 내용들을 헤더파일에 넣어서 제공한다면 재사용성 측면에서도 좋을 것이다. 사용할 때는 아래처럼 테스트하려는 타입을 초기화하기만 하면 된다.

```c++
typedef ::testing::Types<char, int, unsigned int> MyTypes;
INSTANTIATE_TYPED_TEST_SUITE_P(My, FooTest, MyTypes);
```

타입 초기화를 위한 macro는 `INSTANTIATE_TYPED_TEST_SUITE_P`이다. 여기서 macro의 첫 번째 argument가 test suite 이름의 prefix로 사용되기 때문에 중복해서 사용하면 안 된다.

마지막으로 타입이 1개 뿐이라면 굳이 alias를 만들지 않고 해당 타입이름을 바로 사용해도 된다.

```c++
INSTANTIATE_TYPED_TEST_SUITE_P(My, FooTest, int);
```

모두 끝났다. 전체 예제코드가 필요하다면 [sample6_unittest.cc](../../../samples/sample6_unittest.cc)를 참조하자.

## Private Code 테스트하기

만약, 어떤 interface를 구현하는 것이 아니라 해당 interface의 내부구현만을 변경하고 있다면 interface에 대한 테스트는 그러한 변경작업에 관계 없이 잘 동작해야 한다. 이와 같이 어떤 software에 대해서 외부사용자(client) 입장에서 테스트하는 접근방법을 **black-box testing**이라고도 하는데 여기서 외부사용자 입장이라는 것은 interface 혹은 class의 public method에 대한 테스트를 의미한다.

**Black-box testing** 관점에서 볼 때, public method가 아니라 내부 구현에 대한 테스트가 필요하다고 느꼈다면 **사실은 테스트에 앞서 class 설계에 문제가 없는지를 먼저 검토해야 한다.** 예를 들어 대상 class가 너무 많은 일을 하는건 아닌지 점검하고 문제가 있다면 기존 class를 분리하는 등의 설계변경이 필요할 수도 있다. (구현을 담당하는 implemetation class를 새로 추가하는 것도 해결방법 중에 하나이다.)

이유야 어찌 됐건, 외부로 노출되는 interface(public method)가 아니라 내부 구현에 대한 테스트를 하고 싶다고 가정해보면 아마도 아래와 같은 function들이 그러한 테스트의 대상이 될 것이다.

*   Static function(class의 static method를 의미하는 것이 아니라 c-style global function을 의미합니다.) 또는 unnamed namespace에 정의된 function
*   Class의 private method 또는 protected method

이들을 테스트하기 위해서는 아래와 같은 방법을 사용할 수 있다.

*   기본적으로 static function과 unnamed namespace function은 어디서든 사용이 가능하다. 따라서 이들을 테스트하려면 해당 function이 구현된 `.cc` 파일만 `#include` 하면 된다. 다만, `.cc` 파일을 include하는 것이 좋은 구현이라고 볼 수는 없기 때문에 (꼭 필요한 경우에) 테스트코드에서만 사용해야 한다.

    더 좋은 방법은 이들을 특정한 namespace에 모아서 `private`하게 사용하는 것이다. 예를 들어 진행중인 프로젝트에서 일반적으로 사용하는 namespace가 `foo`라면 `foo::internal`이라는 namespace를 새로 만들고 거기에 `private`한 function들을 모두 모아서 관리하는 것이다. 그런 다음에는 `-internal.h`와 같은 헤더파일을 포함해서 사용하면 된다. 다만, 이 과정에서 주의할 점은 `foo::internal`과 `-internal.h`는 해당 프로젝트의 개발자들만 접근할 수 있도록 제한해야 한다는 점이다. 이러한 방법을 이용하면 외부에 구현을 노출시키지 않고도 테스트가 가능해질 것이다.

*   Class의 private method는 해당 class 혹은 friend 에서만 호출이 가능하다. 따라서 테스트대상 class에서 test fixture class나 혹은 method를 friend로 선언하는 작업이 필요하다. 그런데 아직도 문제가 있다. 왜냐하면 googletest의 test function들은 사실 내부적으로는 test fixture class를 상속받고 있기 때문에 test function에서 테스트대상의 private 영역에 바로 접근할 수가 없다. C++에서 base class끼리 friend라고 해도 그 관계가 derived class로 전달되지는 않기 때문이다. (아버지의 친구가 아들의 친구가 아니듯이) 이 문제를 해결하기 위해서 test fixture class에 테스트대상의 private 영역에 대한 accessor function도 구현해야 한다. 그래야만 test function에서 해당 accessor를 통해 테스트대상의 private 영역에 접근할 수 있게 된다.

    위에서 설명했듯이 여기서도 전체적인 리팩토링을 하는 것이 하나의 방법이 될 수 있다. 방법은 테스트대상의 private function들을 implementation class로 분리하는 것이다. 그 다음에 `*-internal.h`와 같이 개발자만 사용할 수 있는 헤더파일을 만들고 사용하면 된다. 이 헤더파일을 외부에서는 사용할 수 없도록 제한하면 마치 private function인 것처럼 사용할 수 있다. 이러한 방법은 상당히 널리 사용되는 방법으로서 소위 [Pimpl](https://www.gamedev.net/articles/programming/general-and-gameplay-programming/the-c-pimpl-r1794/)(Private Implementation) idiom이라고도 부른다.

    다시 friend 사용법으로 돌아와서 googletest는 friend 관계를 간단히 사용하기 위한 기능도 제공하고 있다. 테스트대상 class에서 아래와 같은 방법을 사용하면 별도의 accessor function을 구현할 필요가 없다는 장점이 있다. 단, 필요한 test function 별로 각각 지정해야 한다.

    ```c++
	FRIEND_TEST(TestSuiteName, TestName);
    ```

    아래는 예제코드이다.

    ```c++
    // foo.h
    class Foo {
      ...
     private:
      FRIEND_TEST(FooTest, BarReturnsZeroOnNull);

      int Bar(void* x);
    };

    // foo_test.cc
    ...
    TEST(FooTest, BarReturnsZeroOnNull) {
      Foo foo;
      EXPECT_EQ(foo.Bar(NULL), 0);  // Uses Foo's private member Bar().
    }
    ```

    `FRIEND_TEST`를 사용할 때의 주의사항으로 테스트대상 class가 특정한 namespace에 선언되어 있는 경우에는 test fixture와 test function도 같은 namespace에 구현해야 한다는 것이다. 그래야만 friend 관계가 정상적으로 맺어진다. 아래 예제를 보자.

    ```c++
	namespace my_namespace {

	class Foo {
	  friend class FooTest;
	  FRIEND_TEST(FooTest, Bar);
	  FRIEND_TEST(FooTest, Baz);
	  ... definition of the class Foo ...
	};

	}  // namespace my_namespace
    ```

    위 코드의 `my_namespace::Foo` class를 테스트하기 위한 테스트코드도 같은 namespace에 구현해야 한다. 예제코드는 아래와 같다.

    ```c++
    namespace my_namespace {

    class FooTest : public ::testing::Test {
     protected:
      ...
    };

    TEST_F(FooTest, Bar) { ... }
    TEST_F(FooTest, Baz) { ... }

    }  // namespace my_namespace
    ```

## "Catching" Failures

만약, Googletest를 기반으로 하는 새로운 테스트 도구를 만들고 있다면 그러한 테스트 도구가 잘 동작하는지도 확인이 필요하다. 역설적이만 그런 경우에도 googletest를 사용할 수 있다.

테스트 도구의 목적은 failure를 잘 찾아내는 것에 있다. 따라서 그러한 테스트 도구를 검증하려면 원하는 상황에서 failure가 발생하는지 테스트하면 된다. 그런데 테스트 도구가 failure를 알리기 위한 방법으로 exception을 던지도록 구현되어 있다면 어떻게 해야 할까? (알다시피 googletest는 기본적으로 exception를 사용하지 않고 있다.)

이를 위해서는 `gunit-spi.h`라는 파일을 사용해야 한다. 이 파일에는 exception 확인이 가능한 macro들을 제공하고 있다. 아래 내용을 보자.

```c++
  EXPECT_FATAL_FAILURE(statement, substring);
```

위 macro는 `statement`가 fatal failure를 발생시키는지 확인한다. 또한, 해당 fatal failure의 failure message가 `substring`를 포함하는지도 확인한다.

```c++
  EXPECT_NONFATAL_FAILURE(statement, substring);
```

위 macro도 동작은 동일하며 대신 non-fatal failure를 확인하는데 사용한다.

다만, 위의 2가지 macro는 current thread에서 발생한 failure들만 확인할 수 있다. 따라서 current thread가 아닌 thread에서 failure가 발생하면 검증이 불가능하다. 그럼 `statement`에서 생성한 thread에서 failure가 발생했다면 어떻게 확인해야 할까? 그런 경우에는 아래 macro를 사용하기 바란다.

```c++
  EXPECT_FATAL_FAILURE_ON_ALL_THREADS(statement, substring);
  EXPECT_NONFATAL_FAILURE_ON_ALL_THREADS(statement, substring);
```

NOTE: Windows 환경에서는 위와 같이 multiple thread에서 failure를 확인하는 방법을 사용할 수 없다.

마지막으로 위의 macro들을 사용함에 있어서 아래의 제약사항들은 주의가 필요하다.

1.  위의 macro들은 failure message를 출력할 수 없다.

1.  fatal failure(`EXPECT_FATAL_FAILURE*`) macro의 `statement`에서 `this`의 non-static local variable 및 non-staic method를 참조할 수는 없다.

1.  fatal failure(`EXPECT_FATAL_FAILURE*`) macro의 `statement`는 값을 반환하면 안 된다.

## 개별 Test를 런타임에 등록하기

Googletest에서 test case를 정의할 때 `TEST` 계열 macro를 사용하는 것이 일반적이다. 실제로도 대다수의 개발자들이 `TEST` 계열 macro를 사용하고 있으며 이들은 내부적으로 test case들을 compile time에 등록하게 된다. 그럼 혹시 test case를 runtime에 등록하고 싶은 사용자도 있지 않을까? 예를 들어 별도의 설정파일(.config 등)에서 test case의 정보를 관리하다가 test program이 실행된 후에 설정파일을 읽어서 test case를 등록하고 싶은 경우가 있을 것이다. googletest는 그러한 사용자를 위해서 `::testing::RegisterTest`를 제공하고 있다.

다만, 이러한 기능을 제공하고 있기는 하지만 `::testing::RegisterTest`는 복잡도가 상당히 높기 때문에 꼭 필요한 경우에만 사용하는 것이 좋다. 되도록이면 `TEST`를 통해서 정적으로 등록하는 것을 권장한다.

그럼 googletest에서 제공하는 `::testing::RegisterTest`의 정의는 아래와 같다.

```c++
template <typename Factory>
TestInfo* RegisterTest(const char* test_suite_name, const char* test_name,
		       const char* type_param, const char* value_param,
		       const char* file, int line, Factory factory);
```

마지막 argument인 `factory`는 test case(function)를 생성할 수 있는 factory function를 의미한다. 이 때는 꼭 function pointer가 아니더라도 factory 역할을 할 수 있는 callable object라면 어느 것이라도 괜찮다. 단, 어떤 걸 사용하든 return type은 `Fixture`타입이 되어야 하며 여기서 `Fixture`는 test suite class를 의미한다. 정리하면 첫 번째 argument이자 구분자인 `test_suite_name`에 속한 모든 test case들은 동일한 return type(test fixture class)을 가져야 한다. 이제 이러한 제약사항들이 모두 만족되면 runtime에 test case를 등록할 수 있을 것이다.

Googletest는 `factory`로부터 test fixture class를 추론할 수 있기 때문에 test suite class의 function(`SetUpTestSuite`, `TearDownTestSuite `등)들도 호출할 수 있게 된다.

위의 모든 내용들이 `RUN_ALL_TEST()`보다 앞서 수행되어야 하며 그렇지 않다면 미정의 동작이므로 문제가 된다. 아래 예제를 통해 관련내용을 확인해보자.

```c++
class MyFixture : public ::testing::Test {
 public:
  // All of these optional, just like in regular macro usage.
  static void SetUpTestSuite() { ... }
  static void TearDownTestSuite() { ... }
  void SetUp() override { ... }
  void TearDown() override { ... }
};

class MyTest : public MyFixture {
 public:
  explicit MyTest(int data) : data_(data) {}
  void TestBody() override { ... }

 private:
  int data_;
};

void RegisterMyTests(const std::vector<int>& values) {
  for (int v : values) {
    ::testing::RegisterTest(
	"MyFixture", ("Test" + std::to_string(v)).c_str(), nullptr,
	std::to_string(v).c_str(),
	__FILE__, __LINE__,
	// Important to use the fixture type as the return type here.
	[=]() -> MyFixture* { return new MyTest(v); });
  }
}
...
int main(int argc, char** argv) {
  std::vector<int> values_to_test = LoadValuesFromConfig();
  RegisterMyTests(values_to_test);
  ...
  return RUN_ALL_TESTS();
}
```

## 현재 수행중인 Test의 이름 확인하기

테스트코드를 구현하다 보면 현재 실행중인 test case의 이름을 확인하고 싶은 경우도 있을 것이다. 예를 들어 test case의 이름에 따라 golden file을 바꿔가며 테스트를 수행할 수도 있다. (golden file: 테스트의 결과를 저장하거나 비교하기 위해 생성하는 파일) Googletest의 `::testing::TestInfo` class는 이러한 정보를 가지고 있으며 사용자도 이것을 사용할 수 있다.

```c++
namespace testing {

class TestInfo {
 public:
  // Returns the test suite name and the test name, respectively.
  //
  // Do NOT delete or free the return value - it's managed by the
  // TestInfo class.
  const char* test_suite_name() const;
  const char* name() const;
};

}
```

`TestInfo` class를 사용하고 싶다면 `UnitTest`(singleton object)의 member function인 `current_test_info()`를 사용하면 된다.

```c++
  // Gets information about the currently running test.
  // Do NOT delete the returned object - it's managed by the UnitTest class.
  const ::testing::TestInfo* const test_info =
    ::testing::UnitTest::GetInstance()->current_test_info();

  printf("We are in test %s of test suite %s.\n",
	 test_info->name(),
	 test_info->test_suite_name());
```

`current_test_info()`가 호출되었는데 실행 중인 test case가 없다면 null pointer를 반환하게 된다. 예를 들면 `TestSuiteSetUp()`, `TestSuiteTearDown()`과 같은 function들은 test case를 실행 중인 상황이 아니기 때문에 null pointer가 반환된다.

## Test Events를 통해 googletest 확장하기

Googletest는 test program의 진척상황이나 test failure를 알려주기 위한 **event listener API**라는 기능을 제공하고 있다. 이 때 발생하는 event들은 test program, test suite, test case 각각의 시작시점과 종료시점에 수신해 볼 수 있다. 이 기능이 사용가능한 분야로는 console이나 XML로 출력되는 output을 변경하고 싶은 경우나 아니면 resource leak checker를 위한 checkpoint가 필요한 경우 등이 있다.

### Event Listeners 정의하기

Event listener를 정의하려면 `testing::TestEventListener` 또는 `testing::EmptyTestEventListener`를 상속받는 class를 하나 만들어야 한다. 전자인 `testing::TestEventListener`는 interface(abstract class)이기 때문에 포함된 *pure virtual function을 모두 override 해야 한다.* 다음으로 후자인 `testing::EmptyTestEventListener`는 member function의 기본적인 구현(사실, 비어 있음)을 제공하기 때문에 필요한 것만 override해서 구현하면 된다.

둘 중 하나를 상속하기로 결정했다면 이제 event를 처리하기 위한 동작을 구현하면 된다. 기본적으로 event가 발생하면 handler function들이 자동으로 호출되는데 이 때 handler function에는 아래와 같은 타입의 argument들이 전달된다.

- UnitTest는 test program 전체의 상태를 가지고 있다.
- TestSuite는 이름 그대로 test suite의 정보를 가지고 있다. Test suite은 1개 이상의 test case를 가지고 있다.
- TestInfo는 현재 test case의 정보를 가지고 있다.
- TestPartResult는 test assertion의 결과를 가지고 있다.

사용자는 event handler function에서 이러한 argument를 사용할 수 있다. 아래는 이에 대한 예제 코드이다.

```c++
  class MinimalistPrinter : public ::testing::EmptyTestEventListener {
    // Called before a test starts.
    virtual void OnTestStart(const ::testing::TestInfo& test_info) {
      printf("*** Test %s.%s starting.\n",
	     test_info.test_suite_name(), test_info.name());
    }

    // Called after a failed assertion or a SUCCESS().
    virtual void OnTestPartResult(const ::testing::TestPartResult& test_part_result) {
      printf("%s in %s:%d\n%s\n",
	     test_part_result.failed() ? "*** Failure" : "Success",
	     test_part_result.file_name(),
	     test_part_result.line_number(),
	     test_part_result.summary());
    }

    // Called after a test ends.
    virtual void OnTestEnd(const ::testing::TestInfo& test_info) {
      printf("*** Test %s.%s ending.\n",
	     test_info.test_suite_name(), test_info.name());
    }
  };
```

### Event Listeners 사용하기

위에서 event listener를 정의하는 방법에 대해서 배웠다. 다음으로 정의된 event listener를 실제로 사용하기 위해서는 해당 event listener의 instance를 생성하고 이를 googletest event listener list에 추가하는 작업이 필요하다. `main()` function을 구현하면서 마지막 부분에 `return RUN_ALL_TESTS()`를 필수적으로 호출해야 한다고 배웠는데 그보다 앞 부분에 event listener를 추가해주면 된다. 이 때 사용하는 class를 `TestEventListeners`라고 부른다. 자세히 보면 class 이름에 `s`가 붙어 있는데 이는 여러개의 event listener를 등록하는 것도 가능함을 의미한다. 그럼 아래의 예제코드를 보자.

```c++
int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  // Gets hold of the event listener list.
  ::testing::TestEventListeners& listeners =
	::testing::UnitTest::GetInstance()->listeners();
  // Adds a listener to the end.  googletest takes the ownership.
  listeners.Append(new MinimalistPrinter);
  return RUN_ALL_TESTS();
}
```

아마도 위 코드를 구현한 사람은 `MinimalistPrinter`라는 event listener를 통해 console이나 XML 등으로 출력되는 테스트 결과를 최소화려고 했을 것이다. 따라서 원하는 목적을 달성하려면 기존에 동작하고 있던 default test result printer는 해제해줘야 한다. 이를 위한 코드는 아래와 같다.

```c++
  ...
  delete listeners.Release(listeners.default_result_printer());
  listeners.Append(new MinimalistPrinter);
  return RUN_ALL_TESTS();
```

이제 모두 끝났다. 의도한 대로 결과가 출력되는지 확인해보자. 보다 상세한 내용은 [sample9_unittest.cc](../../../samples/sample9_unittest.cc)에서도 확인할 수 있다.

앞에서 얘기한 것처럼 1개 이상의 listener를 사용하는 것도 가능하다. 따라서 동일한 event에 대해서 listener의 호출순서가 어떻게 되는지는 알아두는게 좋다. 주의할 점은 handler function의 종류에 따라서 순서가 다르다는 점이다. 먼저 `On*Start()` 또는 `OnTestPartResult()`라는 event가 발생하면 list에 등록된 listener들을 순서대로 호출해준다. 즉, 새로 등록한 listener일수록 나중에 호출된다. 이와 반대로 `On*End()`와 같은 event는 등록순서의 *역방향*으로 listener handler를 호출해준다. 따라서 사용자가 결과물을 보게 되면 먼저 등록된 listener handler의 출력물이 늦게 등록된 listener handler의 출력물을 감싸는 (프레임) 형태가 될 것이다.

### Listeners에서 failure 발생시키기

Event를 처리할 때에도 `EXPECT_*()`, `ASSERT_*()`, `FAIL()`와 같은 assertion을 사용할 수 있다. 단, 아래 내용들은 주의가 필요하다.

1.  `OnTestPartResult()` 안에서는 어떤 failure라도 발생시키면 안 된다. 만약 `OnTestPartResult()`에서 failure가 발생하면 무한히 재귀호출할 것이다.
1.  `OnTestPartResult()`를 처리하는 listener도 failure를 발생시키면 안 된다.

만약 failure가 발생가능한 listener를 구현했다면 등록할때는 `OnTestPartResult()`를 먼저 등록한 후에 failure 발생가능한 listener를 나중에 등록해야 한다. 그렇게 해야만 failure 정보가 정상적으로 기록된다.

Failure-raising listener에 대한 예제코드는 [sapmle10_unittest.cc](../../../samples/sample10_unittest.cc)에서 확인할 수 있다.

## Test program을 실행하는 다양한 방법

Googletest를 사용한 test program도 일반적인 실행파일과 다르지 않다. 빌드된 후에는 자유롭게 실행할 수 있고 실행시에 cmd line을 통해 flag를 전달할 수도 있다. 그리고 test program에서 cmd line flag를 적용하기 위해서는 `RUN_ALL_TESTS()`를 호출하기 전에 `::testing::InitGoogleTest()`를 먼저 호출하면 된다.

지원하는 flag 목록을 확인하고 싶다면 test program을 실행할 때, `--help`를 입력하면 된다. `--help`외에 `-h`, `-?`, `/?` 등도 동일한 목적으로 사용할 수 있다.

만약 동일한 옵션에 대해서 2가지 방법(코드에서 직접 environment variable 변경, cmd line flag 전달)을 모두 사용했다면 cmd line flag가 우선적으로 적용된다.

### 필요한 Test 선택하기

#### Test 목록보기

test program을 시작하기 전에 해당 프로그램에서 사용가능한 테스트 목록을 확인할 수 있다. 더불어 이렇게 목록을 확인한 후에는 filter 기능을 사용해서 필요한 테스트만 선택해서 실행하는 것도 가능하다. (밑에서 설명) 이를 위해서는 `--gtest_list_tests`라는 flag를 사용한다.

```none
TestSuite1.
  TestName1
  TestName2
TestSuite2.
  TestName
```

`--gtest_list_tests` flag를 사용하면 테스트는 수행되지 않으며 목록만 출력해준다. 더불어 이 옵션은 environment variable로는 설정할 수 없는 옵션이다.

#### 전체 Test 중에서 일부만 수행하기

기본적으로 googletest는 test program에 정의된 모든 테스트를 수행한다. 그러나 `GTEST_FILTER`라는 environment variable을 사용하면 디버깅 혹은 빠른 확인을 위해 테스트 중 일부만 수행할 수도 있다. 동일한 옵션을 `--gtest_filter`라는 cmd line flag를 통해서도 지정할 수 있다. 그러면 googletest는 테스트의 이름이 filter와 매칭되는 것들만 수행한다. (포맷 : `TestsuiteName.TestName`)

Filter를 사용할 때, *positive pattern*(매칭되는 테스트 수행함)은 바로 이름을 적으면 되고 *negative pattern*(매칭되는 테스트 수행안함)는 이름 앞에 '`-`' 를 적어야 한다. Positive, negative 각각에 대해서도 여러개의 pattern을 지정할 수 있는데 그런 경우에는 seperator '`:`'를 사용하면 된다. 순서로 보면 *positive pattern*을 (1개 혹은 여러개) 적은 후에, *negative pattern*을 (1개 혹은 여러개) 적는다. 결과적으로 positive pattern과 매칭되면서 negative pattern과는 매칭되지 않는 테스트만 수행될 것이다.

Pattern을 만들때는 `'*'`, `'?'`와 같은 wildcard도 사용할 수 있다. 이를 이용하면 아래처럼 간단한 표현이 가능하다. 또한, 아래와 같은 경우에는 '`*`'를 생략하는 것도 가능하다.

 `'*-NegativePatterns'` → `'-NegativePatterns'` (둘 다 가능)

예제를 통해 다양한 사용법을 확인해보자.

*   `./foo_test` : 모든 테스트
*   `./foo_test --gtest_filter=*` : 모든 테스트
*   `./foo_test --gtest_filter=FooTest.*` : "`FooTest`" 라는 Testsuite에 포함된 모든 테스트
*   `./foo_test --gtest_filter=*Null*:*Constructor*` : 전체 이름에 "`Null`"혹은 "`Constructor`"가 포함된 모든 테스트
*   `./foo_test --gtest_filter=-*DeathTest.*` : "`DeathTest`"가 아닌 모든 테스트
*   `./foo_test --gtest_filter=FooTest.*-FooTest.Bar`  : "`FooTest`" 에서 "`FooTest.Bar`"를 제외한 모든 테스트
*   `./foo_test --gtest_filter=FooTest.*:BarTest.*-FooTest.Bar:BarTest.Foo` : "`FooTest`"에서 "`FooTest.Bar`"를 제외한 모든 테스트와 "`BarTest`"에서 "`BarTest.Foo`"를 제외한 모든 테스트

#### Test를 임시로 비활성화하기

어떤 문제로 인해 전체 테스트를 진행할 수 없다면, 문제를 일으키는 test case를 `DISABLED_` prefix를 통해 해당 비활성화 할 수 있다. 이 방법은 `#if 0` 으로 막는 것보다 유용하다. 왜냐하면 단지 실행만 안될 뿐이지 컴파일 대상에는 포함되기 때문에 계속해서 유지보수할 수 있기 때문이다.

물론, test case뿐만 아니라 test suite을 비활성화 하는 것도 가능하다. 방법은 똑같이 "`DISALBED_`" prefix만 붙여주면 된다.

이에 대한 예제코드는 아래와 같다.

```c++
// Tests that Foo does Abc.
TEST(FooTest, DISABLED_DoesAbc) { ... }

class DISABLED_BarTest : public ::testing::Test { ... };

// Tests that Bar does Xyz.
TEST_F(DISABLED_BarTest, DoesXyz) { ... }
```

NOTE: 이 기능은 임시로 비활성화 할 때만 사용하는 것이 좋다. 문제가 있는 테스트는 언젠가 고쳐야 하며 이 시점은 빠를 수록 좋기 때문이다. 더불어 googletest는 이렇게 비활성화된 테스트를 발견했을 때, warning message를 출력해준다.

TIP: `gsearch` 혹은 `grep`을 사용하면 비활성화된 테스트의 개수를 쉽게 파악할 수 있다. 이런 결과는 프로젝트의 테스트 수준을 측정하는 지표로도 사용할 수 있을 것이다.

#### 비활성화된 Test를 활성화하기

비활성화되어 있는 테스트를 실행하는 것도 가능하다. 이 때에는 `--gtest_also_run_disabled_test`라는 flag를 전달하거나`GTEST_ALSO_RUN_DISABLED_TESTS`라는 environment variable을 `0`이 아닌 값으로 설정하면 된다. 더불어 `--gtest_filter`를 사용하면 비활성화 테스트 중에서도 원하는 것만을 골라서 수행할 수 있다.

### Test를 반복 수행시키기

가끔은 테스트의 결과가 왔다갔다 할 때가 있다. 재현빈도가 낮은 이슈들이 여기에 속하는데 이러한 문제들은 반복적인 수행을 통해서 재현빈도와 문제점을 확인해봐야 한다.

이런 경우에는 `--gtest_repeat` flag를 사용하면 문제되는 테스트를 반복적으로 수행해 볼 수 있으며 디버깅에도 도움이 될 것이다. 아래처럼 사용할 수 있다.

```none
$ foo_test --gtest_repeat=1000
Repeat foo_test 1000 times and don't stop at failures.

$ foo_test --gtest_repeat=-1
A negative count means repeating forever.

$ foo_test --gtest_repeat=1000 --gtest_break_on_failure
Repeat foo_test 1000 times, stopping at the first failure.  This
is especially useful when running under a debugger: when the test
fails, it will drop into the debugger and you can then inspect
variables and stacks.

$ foo_test --gtest_repeat=1000 --gtest_filter=FooBar.*
Repeat the tests whose name matches the filter 1000 times.
```

만약 test program이 [global set-up/tear-down](#global-set-up-tear-down)을 포함하고 있다면 그것도 역시 반복적으로 수행 될 것이다. 왜냐하면 문제가 전역코드에 있을 수도 있기 때문에 이 부분도 필요하다. 마지막으로 `GTEST_REPEAT` environment variable를 사용해서 반복횟수를 지정하는 것도 가능하다.

### Test 수행 순서를 섞기

`--gtest_shuffle` flag  사용하거나 `GTEST_SHUFFLE` environment variable을 `1`로 설정하면 테스트들을 random하게 수행할 수 있다. 이러한 수행의 장점은 테스트 간의 의존성으로 인해 발생하는 문제를 확인할 수 있다는 것이다.

Googletest는 random seed로 현재시간(time)을 사용하기 때문에 매 순간마다 random한 순서로 수행이 가능하다. 더불어 콘솔창에 random seed 값을 출력해주기 때문에 특정한 순서에서 발생하는 문제인지를 확인할 수 있다. 이러한 random seed를 변경하려면 `--gtest_random_seed=SEED` flag를 사용하거나 `GTEST_RANDOM_SEED` environment variable을 변경하면 된다. 설정값의 범위는 0~99999 이다. 만약, random seed = 0 이라면 googletest는 기본설정(현재시간)을 사용하게 된다.

`--gtest_shuffle`과 `--gtest_repeat=N`을 함께 사용하면 반복할 때마다 새로운 random seed를 사용한다.

### Test Output 이모저모

#### Terminal Output 색상 변경하기

Googletest는 terminal output의 색상을 통해 중요한 내용을 강조할 수 있도록 도와준다.

```bash
...
[----------] 1 test from FooTest
[ RUN      ] FooTest.DoesAbc
[       OK ] FooTest.DoesAbc
[----------] 2 tests from BarTest
[ RUN      ] BarTest.HasXyzProperty
[       OK ] BarTest.HasXyzProperty
[ RUN      ] BarTest.ReturnsTrueOnSuccess
...
some error messages
...
[   FAILED ] BarTest.ReturnsTrueOnSuccess
...
[==========] 30 tests from 14 test suites ran.
[   PASSED ] 28 tests.
[   FAILED ] 2 tests, listed below:
[   FAILED ] BarTest.ReturnsTrueOnSuccess
[   FAILED ] AnotherTest.DoesXyz 2 FAILED TESTS
```

`GTEST_COLOR` environment variable 또는 `--gtest_color` flag에 `yes`, `no`, `auto` 값을 지정하면 된다. 기본적으로는 `auto`로 설정되어 있다. `auto`인 경우에 googletest의 출력은 terminal 설정에 따라 달라진다. Windows가 아닌 경우에는 `xterm` 혹은 `xterm-color`에 의해 설정되는 `TERM`이라는 environment variable을 이용한다.

#### Test 수행시간 출력하지 않기

Googletest는 기본적으로 각 테스트의 수행시간을 출력하고 있다. 이러한 기능을 비활성화 하려면 `--gtest_print_time=0` flag를 사용하거나 `GTEST_PRINT_TIME` environment variable을 `0`으로 설정하면 된다.

#### UTF-8 형식 사용하지 않기

테스트가 실패했을 때, googletest는 expected value와 actual value의 `string`값을 hex-encoded로 출력해 주거나 UTF-8로 출력해 준다. 만약, 사용자 환경에서 UTF-8을 사용할 수 없다면 이를 비활성화해야 한다. 이를 위해서는 `--gtest_print_utf8` flag를 `0`으로 설정하거나 `GTEST_PRINT_UTF8` environment variable을 `0`으로 설정한다.

#### XML Report 출력하기

Googletest는 테스트결과를 XML 형식으로 출력하는 방법도 제공한다. XML Report는 각 테스트의 수행시간도 포함하기 때문에 느리게 수행되는 테스트를 확인하는데에 도움이 될 것이다. 또한, XML Report를 이용하면 dashboard를 구축하는데도 도움이 될 것이다.

XML Report를 생성하려면 `--gtest_output` flag 혹은 `GTEST_OUTPUT` environment variable에 `"xml:path_to_output_file"`을 대입하면 된다. 경로지정 없이 그냥 `"xml"`만 대입하면 현재 경로에 `test_detail.xml` 파일을 생성해 준다.

경로를 지정할 때는 Linux에서는 `"xml:output/directory/"`와 같이 지정하고 Windows에서는 `"xml:output\directory\"`과 같이 지정하면 된다. 그러면 해당 경로에 XML 파일이 저장되며 파일의 이름은 test program 이름을 사용하게 된다. 예를 들어서 test program이 `foo_test` 혹은 `foo_test.exe` 라면 XML 파일은 `foo_test.xml`이 된다. 만약 동일한 파일이 이미 존재한다면 `foo_test_1.xml`과 같은 이름으로 자동으로 변경하기 때문에 덮어쓰기는 걱정하지 않아도 된다.

Googletest의 XML Rerpot 출력방식은 `junitreport` Ant task를 모티브로 한다. 다만, 대상 언어가 C++, Java로 서로 다르기 때문에 약간의 차이는 있을 수 있다. 그럼 googletest의 XML Report 예시는 아래와 같다.

```xml
<testsuites name="AllTests" ...>
  <testsuite name="test_case_name" ...>
    <testcase    name="test_name" ...>
      <failure message="..."/>
      <failure message="..."/>
      <failure message="..."/>
    </testcase>
  </testsuite>
</testsuites>
```

*   `<testsuites>`은 전체 test program을 의미한다. (testsuites에 's'가 있음을 유의하자.)
*   `<testsuite>`은 각각의 test suite을 의미한다.
*   `<testcase>`는 `TEST()` 혹은 `TEST_F()`로 정의한 test case(test function)을 의미한다.

아래와 같은 test program이 있다고 가정해보면

```c++
TEST(MathTest, Addition) { ... }
TEST(MathTest, Subtraction) { ... }
TEST(LogicTest, NonContradiction) { ... }
```

XML Report는 아래와 같이 출력된다.

```xml
<?xml version="1.0" encoding="UTF-8"?>
<testsuites tests="3" failures="1" errors="0" time="0.035" timestamp="2011-10-31T18:52:42" name="AllTests">
  <testsuite name="MathTest" tests="2" failures="1" errors="0" time="0.015">
    <testcase name="Addition" status="run" time="0.007" classname="">
      <failure message="Value of: add(1, 1)&#x0A;  Actual: 3&#x0A;Expected: 2" type="">...</failure>
      <failure message="Value of: add(1, -1)&#x0A;  Actual: 1&#x0A;Expected: 0" type="">...</failure>
    </testcase>
    <testcase name="Subtraction" status="run" time="0.005" classname="">
    </testcase>
  </testsuite>
  <testsuite name="LogicTest" tests="1" failures="0" errors="0" time="0.005">
    <testcase name="NonContradiction" status="run" time="0.005" classname="">
    </testcase>
  </testsuite>
</testsuites>
```

몇 가지 기억해야 할 점은 아래와 같다.

*   `<testsuites>`과 `<testsuite>`이 가지고 있는 `tests`라는 attribute은 해당 test suite에 몇 개의 test function이 있는지를 의미한다. 반면에 `failure` attribute은 그 중에서 몇 개의 test function이 실패했는지를 보여준다.

*   `time`은 test suites, test suite, test case들이 모두 가지고 있는 attribute이며 각각의 수행시간이 얼마인지를 알려준다.

*   `timestamp`는 test가 시작된 시각을 알려주는 attribute이다.

*   `<failure>` element는 실패한 각각의 googletest assertion을 의미한다. (`failures` attribute와는 다르다.)

#### JSON Report 출력하기

Googletest는 테스트결과를 JSON 형식으로 출력하는 방법도 제공한다. JSON Report를 출력하기 위해서는 `GTEST_OUTPUT` environment variable을 설정하거나 `--gtest_output` flag를 사용하면 된다. 사용방법은 XML Report와 유사하다. `GTEST_OUTPUT`이나 `--gtest_output` flag에 `"json:path_to_output_file"`을 대입하면 JSON Report가 저장될 위치를 지정할 수도 있다. 간단하게 `"json"`만 지정하면 현재 위치에 `test_detail.json` 파일을 생성한다.

그럼 googletest의 JSON Report 예시는 아래와 같다.

```json
{
  "$schema": "http://json-schema.org/schema#",
  "type": "object",
  "definitions": {
    "TestCase": {
      "type": "object",
      "properties": {
	"name": { "type": "string" },
	"tests": { "type": "integer" },
	"failures": { "type": "integer" },
	"disabled": { "type": "integer" },
	"time": { "type": "string" },
	"testsuite": {
	  "type": "array",
	  "items": {
	    "$ref": "#/definitions/TestInfo"
	  }
	}
      }
    },
    "TestInfo": {
      "type": "object",
      "properties": {
	"name": { "type": "string" },
	"status": {
	  "type": "string",
	  "enum": ["RUN", "NOTRUN"]
	},
	"time": { "type": "string" },
	"classname": { "type": "string" },
	"failures": {
	  "type": "array",
	  "items": {
	    "$ref": "#/definitions/Failure"
	  }
	}
      }
    },
    "Failure": {
      "type": "object",
      "properties": {
	"failures": { "type": "string" },
	"type": { "type": "string" }
      }
    }
  },
  "properties": {
    "tests": { "type": "integer" },
    "failures": { "type": "integer" },
    "disabled": { "type": "integer" },
    "errors": { "type": "integer" },
    "timestamp": {
      "type": "string",
      "format": "date-time"
    },
    "time": { "type": "string" },
    "name": { "type": "string" },
    "testsuites": {
      "type": "array",
      "items": {
	"$ref": "#/definitions/TestCase"
      }
    }
  }
}
```

이러한 JSON Report의 출력형식은 [JSON encoding](https://developers.google.com/protocol-buffers/docs/proto3#json)을 사용한 Proto3을 따르고 있다. Proto3은 아래와 같다.

```proto
syntax = "proto3";

package googletest;

import "google/protobuf/timestamp.proto";
import "google/protobuf/duration.proto";

message UnitTest {
  int32 tests = 1;
  int32 failures = 2;
  int32 disabled = 3;
  int32 errors = 4;
  google.protobuf.Timestamp timestamp = 5;
  google.protobuf.Duration time = 6;
  string name = 7;
  repeated TestCase testsuites = 8;
}

message TestCase {
  string name = 1;
  int32 tests = 2;
  int32 failures = 3;
  int32 disabled = 4;
  int32 errors = 5;
  google.protobuf.Duration time = 6;
  repeated TestInfo testsuite = 7;
}

message TestInfo {
  string name = 1;
  enum Status {
    RUN = 0;
    NOTRUN = 1;
  }
  Status status = 2;
  google.protobuf.Duration time = 3;
  string classname = 4;
  message Failure {
    string failures = 1;
    string type = 2;
  }
  repeated Failure failures = 5;
}
```

예를 들어 아래와 같은 test program이 있다면

```c++
TEST(MathTest, Addition) { ... }
TEST(MathTest, Subtraction) { ... }
TEST(LogicTest, NonContradiction) { ... }
```

JSON Report는 아래와 같이 출력된다.

```json
{
  "tests": 3,
  "failures": 1,
  "errors": 0,
  "time": "0.035s",
  "timestamp": "2011-10-31T18:52:42Z",
  "name": "AllTests",
  "testsuites": [
    {
      "name": "MathTest",
      "tests": 2,
      "failures": 1,
      "errors": 0,
      "time": "0.015s",
      "testsuite": [
	{
	  "name": "Addition",
	  "status": "RUN",
	  "time": "0.007s",
	  "classname": "",
	  "failures": [
	    {
	      "message": "Value of: add(1, 1)\n  Actual: 3\nExpected: 2",
	      "type": ""
	    },
	    {
	      "message": "Value of: add(1, -1)\n  Actual: 1\nExpected: 0",
	      "type": ""
	    }
	  ]
	},
	{
	  "name": "Subtraction",
	  "status": "RUN",
	  "time": "0.005s",
	  "classname": ""
	}
      ]
    },
    {
      "name": "LogicTest",
      "tests": 1,
      "failures": 0,
      "errors": 0,
      "time": "0.005s",
      "testsuite": [
	{
	  "name": "NonContradiction",
	  "status": "RUN",
	  "time": "0.005s",
	  "classname": ""
	}
      ]
    }
  ]
}
```

IMPORTANT: JSON Report 형식은 변경될 수도 있다.

### Failure가 발생했을 때 가능한 조작법

#### Assertion Failures를 Break-Points처럼 사용하기

디버거에서 test program을 동작시킬 때는 assertion 발생시점에 바로 interactive mode로 전환되는 것이 디버깅에 도움이 될 것이다. googletest는 그런 사용자를 위해 *break-on-failure* mode를 제공한다.

`GTEST_BREAK_ON_FAILURE` environment variable을 `0`이 아닌 값으로 설정하거나 `--gtest_break_on_failure` flag를 사용하면 이 기능을 활성화할 수 있다.

#### Exception Catching 비활성화하기

기본설정의 googletest는 테스트가 C++ exception를 던진 경우에 exception을 catch하고 해당 테스트를 실패로 판정한다. 그리고는 다음 테스트를 계속해서 진행한다. 이런 동작방식이 기본설정인 이유는 한 번의 실행으로도 최대한 많은 테스트를 수행하기 위함이다. 더불어 Windows와 같은 환경에서 발생할 수 있는 문제도 예방하게 된다. 예를 들면 uncaught exception으로 인해 발생한 pop-up이 자동화 테스트를 멈추게 하는데 googletest가 exception을 catch하고 실패로 처리함으로써 자동으로 넘어갈 수 있게 된다.

만약, 사용자가 googletest의 exception 처리를 비활성화하고 싶다면 그것도 가능하다. 그러한 경우의 예로 디버거의 exception catch 기능을 사용하고 싶은 경우가 있을 것이다. 이를 통해 exception 발생시점의 call stack 확인 등이 가능하기 때문이다. 그런 경우에는 `GTEST_CATCH_EXCEPTIONS` environment variable을 `0`으로 설정하거나 `--gtest_catch_exceptions=0` flag를 사용하면 된다.
