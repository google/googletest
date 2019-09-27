## 왜 googletest 인가?

*googletest*를 통해서 C++에서도 진보된 테스트코드를 구현할 수 있게 되었습니다.

googletest는 Google Testing Technology Team 주도로 개발이 시작되었습니다. Linux, Windows, Mac 등 *플랫폼 제한없이* 사용할 수 있으며 단위테스트 뿐만 아니라 다른 종류의 테스트에도 적용이 가능합니다.

시작하기에 앞서, 좋은 테스트란 무엇이고 googletest가 이에 기여할 수 있는 부분을 알아보겠습니다.

1. 테스트는 *독립적이고 반복적으로* 수행될 수 있어야 합니다. 테스트들이 서로 영향을 받는다면 디버깅이 어려워질 것입니다. googletest는 각 테스트를 서로 다른 객체object에서 실행함으로써 테스트들을 격리시킵니다. 한 테스트가 실패했을 때 googletest로 그것만 따로 실행해서 디버깅을 빨리 할 수 있습니다.
2. 테스트는 *잘 정리되어 있어야* 하고 테스트한 코드의 구조도 반영해야 합니다. googletest는 관련있는 테스트를 테스트 스위트test suite로 묶어 데이터와 서브루틴subroutine을 공유할 수 있게 합니다. 이 보편적인 형태는 알아보기도 쉽고 테스트 유지보수도 쉽게 만듭니다. 이런 일관성은 사람들이 프로젝트를 바꿔 새로운 코드에서 일을 시작할 때 특히 도움이 됩니다.
3. 테스트는 *이식성*과 *재사용성*이 있어야 합니다. Google에는 플랫폼 중립적인 코드가 아주 많기 때문에 테스트코드 또한 플랫폼 중립적이어야 합니다. googletest는 여러 운영체제에서 여러 컴파일러와 예외exception가 활성화되었든 아니든 동작하므로 googletest는 다양한 구성configuration으로 동작할 수 있습니다.
4. 테스트가 실패했을 때 테스트 문제에 대해 가능한 많은 *정보를 제공해야 합니다.* googletest는 첫번째 테스트 실패에서 중단되지 않습니다. 해당 테스트만 먼추고 다음으로 진행합니다. non-fatal failure를 리포트 하고 계속 진행하게 테스트를 설정할 수도 있습니다. 이러한 설계의 장점은 한 번의 run-edit-compile cyclle로도 여러 개의 bug를 찾아내고 개선할 수 있다는 것입니다.
5. 테스팅 프레임워크는 테스트를 구현하는 사람이 실제 구현에만 집중할 수 있도록 도와줘야 합니다. googletest는 정의된 모든 Test를 자동으로 찾아서 실행해주기 때문에 사용자가 하나하나 찾아서 실행할 필요가 없습니다.
6. 테스트는 *빨라야 합니다.* googletest를 쓰면 테스트 간의 공유 자원을 재사용 할 수 있고, 테스트 간 상호 의존 없이 설정/해체을 한 번만 부담하면 됩니다.

googletest가 xUnit architecture를 기반으로 설계되었기 때문에 JUnit, PyUnit과 같은 xUnit계열 테스트 프레임워크를 사용해본 경험이 있다면 빠르게 적응할 수 있습니다. 물론, 그러한 경험이 없더라도 기초적인 것을 배우고 구현하는 데에는 10분이면 충분합니다. 그러니 같이 시작해 봅시다!

## 명명법 주의

*Note:* *테스트Test*, *테스트 케이스Test Case*, *테스트 스위트Test Suite*같은 용어의 정의 차이로 인해 혼란이 있을 수 있습니다.

역사적으로 googletest는 연관된 테스트들의 묶음이라는 뜻으로 *테스트 케이스*를 사용하기 시작했습니다. 반면에 [ISTQB](http://www.istqb.org/)를 비롯한 표준기관에서는 [*테스트 스위트*][istqb test suite]를 이런 의미로 사용합니다.

googletest에서 사용되는 관련 용어 *테스트*는 ISTQB 등의 용어로는 [*테스트 케이스*][istqb test case]에 해당됩니다.

*테스트*라는 용어는 일반적으로 충분히 넓은 뜻으로 받아 들여져 ISTQB의 *테스트 케이스* 정의까지 포함하므로 큰 문제는 아닙니다. 그러나 구글 테스트Google Test에서 사용된 *테스트 케이스*라는 용어는 모순된 의미를 가져 혼란을 야기합니다.

googletest는 *Test Case*라는 용어를 *Test Suite*로 바꾸기 시작했습니다. 선호하는 API는 *TestSuite*입니다. 이전의 TestCase API는 서서히 사용이 중단되고 리팩토링되고 있습니다.

그러니 용어들의 다른 의미에 주의해 주십시오.

의미 | googletest 용어 | [ISTQB](http://www.istqb.org/) 용어
------- | --------------- | -------------------------------------
특정 입력으로 프로그램을 실행하고 그 결과를 검증 | [TEST()](#간단한-테스트) | [Test Case][istqb test case]

[istqb test case]: http://glossary.istqb.org/en/search/test%20case
[istqb test suite]: http://glossary.istqb.org/en/search/test%20suite

## 기본 개념

googletest를 사용한다고 하면 *assertion*을 쓰는 것부터 시작하게 됩니다. Assertion이란 어떤 조건이 참인지 거짓인지 검사하는 것을 의미하며 이러한 assertion의 결과는 *success*, *non-fatal failure*, *fatal failure* 중에 하나가 됩니다. 만약, Test를 진행하는 중에 fatal failure가 발생하면 해당 Test가 중단되지만, 반대로 success나 non-fatal failure가 발생하면 중단하지 않고 계속 진행합니다.

각 Test에는 이러한 assertion을 사용해서 테스트 대상코드가 잘 동작하는지 구현하게 되며 assertion의 수행결과에 따라 Test의 *성공이나 실패도 결정됩니다.*

Test Suite은 하나 이상의 Test를 포함합니다. Test들은 테스트 대상코드의 구조에 맞춰 여러개의 Test Suite으로 그룹화 될 수 있습니다. 또한, Test Suite에 속한 Test 간에 공유해야 할 자원이 있는 경우에는 *test fixture*를 사용하면 자원을 편리하게 공유할 수 있습니다.

Test Program은 더 큰 개념으로서 여러개의 Test Suite을 포함합니다.

이제 가장 하위에서 assertion을 직접적으로 사용하는 Test로부터 시작해서 나아가 Test Suite, Test Program을 어떻게 구현해야 하는지 설명하겠습니다.

## Assertions

googletest의 assertion은 function처럼 보이기는 하지만 macro입니다. 사용자는 이러한 assertion을 통해 class나 function을 테스트하게 됩니다. googletest는 assertion이 실패하면 해당 소스파일과 실패한 위치, 그리고 오류메시지(실패한 이유)와 같은 정보를 출력해줍니다. 그리고 이렇게 출력되는 정보를 사용자가 자유롭게 변경할 수도 있습니다.

어떤 기능을 테스트할 때, 여러개의 assertion을 함께 사용하는 것도 가능합니다. Assertion을 상위 레벨에서 분류해보면 `ASSERT_*`계열과 `EXPECT_*`계열이 있습니다. 먼저, `ASSERT_*` 계열은 테스트가 실패하면 fatal failure를 발생시키며 현재 동작중인 function을 중단시킵니다. 다음으로 `EXPECT_*` 계열은 non-fatal failure를 발생시키며 현재 function을 중단시키지 않고 계속 진행합니다. 일반적으로는`EXPECT_*` 계열이 선호되는데 왜냐하면 어떤 Test에서 여러개의 failure가 발생한다고 가정하면 한 번의 실행만으로도 모든 failure를 검출해주기 때문입니다. 물론 failure가 발생했을 때, 계속해서 Test를 진행하는 것이 의미가 없다거나 또는 위험한 상황이라면 `ASSERT_*` 를 사용하는 것이 맞습니다.

`ASSERT_*`는 실행중인 Test가 바로 종료되기 때문에, 메모리 해제와 같은 정리작업을 수행하지 않았을 수도 있습니다. 따라서 memory leak과 같은 문제를 야기시키기도 합니다. 사실 테스트코드에서 발생하는 memory leak은 문제가 될지 안될지 판단하기가 어렵습니다. 따라서 googletest의 heap checker를 활성화해서 assertion 오류메시지에 memory leak 관련내용을 포함시키는 것도 도움이 됩니다.

Assertion을 사용함과 동시에 사용자정의 오류메시지를 추가하려면 아래처럼 `<<` 연산자를 사용하면 됩니다.

```c++
ASSERT_EQ(x.size(), y.size()) << "Vectors x and y are of unequal length";

for (int i = 0; i < x.size(); ++i) {
  EXPECT_EQ(x[i], y[i]) << "Vectors x and y differ at index " << i;
}
```

Assertion에서 `ostream`을 통해 출력가능한 오류메시지는 C언어 문자열 또는 C++의 `string`입니다. 혹시나 `std::wstring` 혹은 윈도우의 `UNICODE` 타입인 `wchar_t*`, `TCHAR*` 과 같은 wide string을 사용하면 UTF-8로 변환되어 출력됩니다.

### Basic Assertions

아래 assertion들은 true/false를 확인하는데 사용합니다.

Fatal assertion | Nonfatal assertion | Verifies
--------------- | ------------------ | --------
`ASSERT_TRUE(condition);` | `EXPECT_TRUE(condition);` | `condition` is true
`ASSERT_FALSE(condition);` | `EXPECT_FALSE(condition);` | `condition` is false

다시한번 기억할 것은 fail이 발생하면 `ASSERT_*`는 fatal failure을 발생시킴과 동시에 현재 function을 중단시키지만 `EXPECT_*`는 non-fatal failure이기 때문에 중단시키지는 않고 계속 진행한다는 점입니다. 더 중요한 부분은 fatal failure든 non-fatal failure든 해당 Test의 최종결과는 fail을 의미한다는 것입니다.

**Availability**: Linux, Windows, Mac.

### Binary Comparison

이 섹션에서는 2개의 값을 비교하는 assertion을 설명합니다.

Fatal assertion | Nonfatal assertion | Verifies
--------------- | ------------------ | -------
`ASSERT_EQ(val1, val2);` | `EXPECT_EQ(val1, val2);` | `val1 == val2`
`ASSERT_NE(val1, val2);` | `EXPECT_NE(val1, val2);` | `val1 != val2`
`ASSERT_LT(val1, val2);` | `EXPECT_LT(val1, val2);` | `val1 < val2`
`ASSERT_LE(val1, val2);` | `EXPECT_LE(val1, val2);` | `val1 <= val2`
`ASSERT_GT(val1, val2);` | `EXPECT_GT(val1, val2);` | `val1 > val2`
`ASSERT_GE(val1, val2);` | `EXPECT_GE(val1, val2);` | `val1 >= val2`

위의 assertion을 사용하려면 `val1`, `val2`와 같은 argument들이 assertion 종류에 맞는 비교연산자(`==`, `<` 등)를 제공해야 합니다. `int`, `double`과 같은 산술타입들은 기본적으로 제공되겠지만 사용자 정의 타입이라면 그에 맞는 비교연산자를 구현해서 제공해야 합니다. 그렇지 않으면 당연하게도 compile error가 발생합니다. 이전에는 argument에 대한 정보를 출력하기 위해 `<<` 연산자도 필요했지만 현재는 그 부분은 꼭 제공하지 않아도 괜찮습니다. 만약 argument가 `<<`연산자를 지원한다면 assertion이 실패했을 때 정의된 연산자를 통해 관련정보를 출력하게 됩니다. 반대로 지원하지 않는다면 googletest가 출력할 수 있는 방법을 최대한 찾게 됩니다. 이러한 출력정보에 관련한 더 자세한 내용은 [여기](../../../googlemock/docs/kr/cook_book.md#gmock이-사용자타입-정보도-출력-가능하게-만들기)을 참고하세요.

이처럼 assertion을 사용자정의 타입과 함께 사용하는 것이 가능하지만, 비교연산자도 함께 제공해야만 동작한다는 점을 기억해주시기 합니다. 물론, 사용자정의 타입에 비교연산자를 제공하는 것은 Google [C++ Style Guide](https://google.github.io/styleguide/cppguide.html#Operator_Overloading)에서 권장하는 내용이 아니기 때문에 비교연산자가 아닌 별도의 function을 통해 비교를 수행한 후에 그 결과인 `bool`값만 `ASSERT_TRUE()` 혹은 `EXPECT_TRUE()`를 통해 검사하는 것을 더 추천합니다.

물론 사용자 입장에서는 `ASSERT_TRUE(actual == expected)`보다 `ASSERT_EQ(actual, expected)`가 더 유용할 것입니다. 왜냐하면 후자를 사용하면 assertion에 `actual(실제값)`, `expected(기대값)` 을 모두 전달하기 때문에 실패시에 위의 값들을 출력해주기 때문입니다. 즉, 왜 실패했는지 더 빠르게 확인할 수 있게 됩니다.

Argument가 변수형태라면 assertion코드가 수행되는 시점에 저장되어 있는 값을 사용합니다. 따라서 그 이후에는 argument를 수정해도 괜찮습니다. 또한, 일반적인 C/C++ function과 마찬가지로 argument를 판정하는 순서는 따로 정해져 있지 않습니다. 예를 들어 `ASSERT_EQ(val1, val2)`라고 구현했을 때 `val1`, `val2` 중에서 어느 것이 먼저 수행될지는 알 수 없습니다. 이러한 순서는 컴파일러마다 다를 수 있기 때문에 assertion을 구현할 때는 결과값이 그러한 순서에 따라 영향을 받지 않도록 주의하기 바랍니다.

포인터에 대해서 `ASSERT_EQ()`를 사용하게 되면 의도한 것과 다르게 동작할 수도 있습니다. 왜냐하면 포인터가 가리키는 대상 전체가 아니라 가키리는 곳의 주소값만 비교하기 떄문입니다. 예를 들어, C언어 문자열(e.g. `const char*`) 2개를 비교한다고 가정하면 일반적으로 문자열 전체가 같은지를 비교하려는 목적일 것입니다. 그러나 여기서는 문자열을 비교하는 것이 아니라 memory location만 검사하게 됩니다. 만약, C언어 문자열의 내용을 비교하고 싶다면 `ASSERT_STREQ()`를 사용해야 합니다. 나중에 자세하게 설명하겠지만 C 문자열이 `NULL`이라는 것을 판정하려면 `ASSERT_STREQ(c_string, NULL)`를 사용하면 됩니다. C++11이 지원되는 경우에는 `ASSERT_EQ(c_string, nullptr)` 도 가능합니다. 마지막으로 2개의 string object를 비교한다면 `ASSERT_EQ`를 사용해야 합니다.

포인터가 아무것도 가리키지 않는지 확인하고자 할 때는 `*_EQ(ptr, NULL)`, `*_NE(ptr, NULL)`가 아니라 `*_EQ(ptr, nullptr)`, `*_NE(ptr, nullptr)`와 같이 구현해야 합니다. `nullptr`을 사용해야 타입관련 문제가 발생하지 않습니다. 이와 관련한 자세한 내용은 [FAQ](faq.md#왜-expect_eqnull-ptr-assert_eqnull-ptr만-지원하고-expect_nenull-ptr-assert_nenull-ptr은-지원하지-않나요)를 참조하십시오.

만약 부동소수점 타입을 확인하고자 할 때는 반올림관련 문제가 있기 때문에 별도의 assertion을 사용해야 합니다. 자세한 내용은 [Advanced googletest Topics](advanced.md#floating-point-비교하기)를 참조하세요.

이 섹션의 macro들은 narrow string(string), wide string(wstring) 양쪽 모두에 대해서 잘 동작합니다.

**Availability**: Linux, Windows, Mac.

**Historical note**: 2016년 2월 이전에는 `*_EQ` 계열 assertion을 `ASSERT_EQ(expected, actual)`과 같은 형태로 사용하는 규약이 있었기 때문에 그러한 소스가 많이 남아있습니다. 그러나 현재는 `expected`, `actual` 의 순서를 어떻게 하더라도 괜찮습니다.

### String Comparison

여기서는 C string을 비교하기 위한 assertion을 소개합니다. (만약, `string` object를 비교하고자 한다면 위에서 다룬 `EXPECT_EQ`, `EXPECT_NE`를 사용하면 됩니다.)

| Fatal assertion                 | Nonfatal assertion              | Verifies                                                 |
| ------------------------------- | ------------------------------- | -------------------------------------------------------- |
| `ASSERT_STREQ(str1, str2);`     | `EXPECT_STREQ(str1, str2);`     | the two C strings have the same content                  |
| `ASSERT_STRNE(str1, str2);`     | `EXPECT_STRNE(str1, str2);`     | the two C strings have different contents                |
| `ASSERT_STRCASEEQ(str1, str2);` | `EXPECT_STRCASEEQ(str1, str2);` | the two C strings have the same content, ignoring case   |
| `ASSERT_STRCASENE(str1, str2);` | `EXPECT_STRCASENE(str1, str2);` | the two C strings have different contents, ignoring case |

위의 assertion 중에서 중간에 "CASE"가 포함된 것은 대소문자는 무시하고 비교함을 의미합니다. 또한, `NULL`포인터와 empty string 문자열은 서로 *다른 것으로 간주됨을* 기억하세요.

`*STREQ*` 및 `*STRNE*` 계열도 wide C string(`wchar_t*`)을 허용합니다. 단, 실패한 경우에 출력되는 정보는 UTF-8 narrow 문자열로 출력됩니다.

**Availability**: Linux, Windows, Mac.

**See also**: 기본적인 문자열 비교 외에 추가로 제공되는 기능(substring, prefix, suffix, regular expression 비교하기 등)이 궁금하다면  [여기](../../../googlemock/docs/kr/cheat_sheet.md#string-matchers)를 참조하세요.

## 간단한 테스트

개별 Test(Test Case)를 생성할 때에는 아래와 같은 순서로 진행합니다.

* `TEST()` macro를 사용하여 Test의 이름을 정하고 구현합니다. `TEST()` macro는 return type이 `void`인 C++ function을 생성해줍니다.
* `TEST()` macro의 body에는 테스트에 필요한 C++ 소스코드를 구현하면 됩니다. 이 때 바로 assertion을 사용하게 되며 사용자가 확인하고 싶은 내용들을 직접 구현하면 됩니다.
* 앞에서도 얘기했듯이 어떤 Test의 수행결과는 assertion에 의해 결정됩니다. Failure의 종류에 관계없이(fatal, non-fatal) assertion 중에서 하나라도 실패하면 해당 Test의 최종결과는 실패입니다. 또는 소스코드 실행중에 crash가 발생해도 그 결과는 fail입니다. 이러한 문제들이 발생하지 않고 정상적으로 종료되었다면 당연하게도 success입니다.

```c++
TEST(TestSuiteName, TestName) {
  ... test body ...
}
```

`TEST()`로 전달되는 2개의 argument 중에서 *첫 번째는* 상위 Test Suite의 이름이고 *두 번째는* 개별 Test(Test Case)의 이름입니다. 두 이름 모두 C++ 언어에서도 유효한 식별자여야 하며 밑줄(`_`)을 포함하면 안됩니다. 유효한 이름들이 전달되었다면 `TestSuiteName`과 `TestName`을 합쳐서 Test Case의 *full name*이 만들어집니다. 따라서 `TestSuiteName`이 서로 다르다면 `TestName`은 같아도 됩니다. 더불어 `TEST()`를 사용하면 googletest가 `TestSuiteName`을 통해 Test Suite(Test Fixture)를 직접 생성해 줍니다. 반면에 `TEST_F()` 등을 사용하면 사용자가 직접 구현한 Test Suite(Test Fixture)를 적용하는 것도 가능합니다. 이 부분은 바로 다음 섹션에서 설명합니다.

그럼 `Factorial()`이라는 function을 테스트하는 간단한 예제를 보겠습니다.

```c++
int Factorial(int n);  // Returns the factorial of n
```

아래 소스코드는 먼저 `Factorial`을 테스트하기 위한 2개의 Test가 있고, 이들이 모두 `FactorialTest`이라는 Test Suite에 포함되어 있음을 보여줍니다.

```c++
// Tests factorial of 0.
TEST(FactorialTest, HandlesZeroInput) {
  EXPECT_EQ(Factorial(0), 1);
}

// Tests factorial of positive numbers.
TEST(FactorialTest, HandlesPositiveInput) {
  EXPECT_EQ(Factorial(1), 1);
  EXPECT_EQ(Factorial(2), 2);
  EXPECT_EQ(Factorial(3), 6);
  EXPECT_EQ(Factorial(8), 40320);
}
```

googletest가 Test Suite 별로 테스트결과를 취합해 주기 때문에 Test Suite에는 논리적으로 관련이 있는 Test들을 모아두는 것이 좋습니다. 위 소스코드도 마찬가지로 `FactorialTest` 라는 Test Suite은 `Factorial()`을 테스트하기 위한 Input을 종류별로 구분해서 여러개의 Test로 나누어 놓았습니다. 좀 더 자세히 보면 `HandlesZeroInput`은 argument로 0을 전달하는 Test이고, `HandlesPositiveInput`는 argument로 0보다 큰 양수들을 전달합니다. 크게 보면 2개의 Test 모두 숫자를 전달해서 `Factorial()`을 검증한다는 공통적인 주제로 묶을 수 있는 것입니다.

Test Suite과 Test의 이름을 만들때는 관련 규범인 [naming functions and classes](https://google.github.io/styleguide/cppguide.html#Function_Names)를 준수해야 합니다.

**Availability**: Linux, Windows, Mac.

## Test Fixtures: 동일한 데이터를 여러개의 테스트에 사용하기

유사한 데이터를 사용하는 2개 이상의 Test를 구현할 때에는 *test fixture*를 적용하는 것이 좋습니다. 이를 통해 여러개의 Test가 동일한 환경을 사용할 수 있도록 쉽게 구현할 수 있습니다.

Fixture를 생성하는 방법

1. `::testing::Test` class를 상속받아서 test fixture class를 하나 만듭니다. 이제 `protected:` 영역에 필요한 내용을 구현할 것입니다.
2. test fixture class 내부에 여러 Test들이 공유하게 될 데이터(변수)를 선언합니다.
3. 필요하다면 `SetUp()`이나 default constructor를 정의함으로서 test fixture에 속한 각각의 Test들이 시작될때 공통적으로 수행할 일들(공유자원 자원할당 등)을 구현할 수 있습니다. 이 때, `SetUp()`을 **`Setup()`** 으로 구현하는 대소문자 실수를 하는 경우가 많기 때문에 주의하기 바랍니다. C++11의 `override` 키워드를 사용한다면 실수를 줄일 수 있을 것입니다.
4. 필요하다면 `TearDown()`이나 destructor를 정의함으로서 개별 Test가 끝날때마다 공통적으로 수행할 일들(공유자원 자원해제 등)을 구현한 수 있습니다. `SetUp()/TearDown()`과 constructor/destructor를 어떻게 구분해서 사용해야 하는지 궁금하다면 [FAQ](faq.md#test-fixture에서-constructordestructor-와-setupteardown중-어느것을-써야하나요)을 참조하기 바랍니다.
5. 필요하다면 각 Test가 공통적으로 수행하게 될 함수(subroutine)를 정의하는 것도 좋습니다.

Test fixture를 사용하기 위해서는 개별 Test를 구현할 때, `TEST()` 대신에 `TEST_F()`를 사용해야 합니다. 이러한 `TEST_F()`의 첫 번째 argument로는 위에서  `::testing::Test`를 상속받아 생성한 test fixture class의 이름을 전달합니다.

```c++
TEST_F(TestFixtureName, TestName) {
  ... test body ...
}
```

`TEST()`와 마찬가지로 `TEST_F()`의 첫 번째 argument가 Test Suite의 이름인 것은 맞지만, 이름을 새로 만드는 것은 아니고 test fixture class 이름을 그대로 사용해야 합니다. `TEST_F()`에서 `_F`는 **f**ixture를 뜻합니다.

`TEST()`, `TEST_F()`를 통합하면 좋겠지만 안타깝게도 C++에서 단일 macro를 가지고 2가지 타입을 동시에 조작하는 것은 불가능합니다. Macro를 상호간에 잘못 사용하면 compile error가 발생하니 유의하시기 바랍니다.

또한, `TEST_F()`를 사용하기 전에 fixture 클래스를 먼저 정의해야 하는 것은 당연합니다. 그렇지 않으면  "`virtual outside class declaration`"과 같은 compile error가 발생합니다.

googletest는 `TEST_F()`로 정의된 각 Test가 실행될 때마다 해당 test fixture class의 object를 새롭게 생성합니다. 그런 후에 `SetUp()`을 통해 실행을 위한 준비작업을 하고 Test를 실행합니다. 이제 실행이 끝나면 `TearDown()`을 통해 정리작업을 하며 마지막으로 test fixture object를 삭제합니다. 이렇게 여러 Test들이 같은 Test Suite에 에 속해있다고 하더라도 별도의 test fixture object를 통해 독립적으로 수행됩니다. googletest는 새로운 test fixture object를 생성하기 전에 항상 이전에 사용하던 object를 삭제하기 때문에 이전 Test에서 fixture를 변경시켰다고 해서 다음 fixture에 영향을 주지 않습니다.

그럼 이제 `Queue`라는 class를 위한 Test를 한 번 구현해 보겠습니다. 먼저 `Queue` class는 아래와 같습니다.

```c++
template <typename E>  // E is the element type.
class Queue {
 public:
  Queue();
  void Enqueue(const E& element);
  E* Dequeue();  // Returns NULL if the queue is empty.
  size_t size() const;
  ...
};
```

이제 test fixture class를 정의합니다. 관례적으로 `Foo`라는 class 또는 function을 테스트할때는 test fixture class의 이름도 `FooTest`라고 하는 것이 일반적입니다.

```c++
class QueueTest : public ::testing::Test {
 protected:
  void SetUp() override {
     q1_.Enqueue(1);
     q2_.Enqueue(2);
     q2_.Enqueue(3);
  }

  // void TearDown() override {}

  Queue<int> q0_;
  Queue<int> q1_;
  Queue<int> q2_;
};
```

위 테스트에서는 각각의 Test를 수행한 후에 별도의 정리작업이 필요하지 않기 때문에 `TearDown()`은 굳이 정의하지 않았습니다. 기본적인 것들은 destructor를 통해 수행될 것이기 때문입니다.

이제 `TEST_F()`를 통해 개별 Test를 구현합니다.

```c++
TEST_F(QueueTest, IsEmptyInitially) {
  EXPECT_EQ(q0_.size(), 0);
}

TEST_F(QueueTest, DequeueWorks) {
  int* n = q0_.Dequeue();
  EXPECT_EQ(n, nullptr);

  n = q1_.Dequeue();
  ASSERT_NE(n, nullptr);
  EXPECT_EQ(*n, 1);
  EXPECT_EQ(q1_.size(), 0);
  delete n;

  n = q2_.Dequeue();
  ASSERT_NE(n, nullptr);
  EXPECT_EQ(*n, 2);
  EXPECT_EQ(q2_.size(), 1);
  delete n;
}
```

위 코드를 보면 `ASSERT_*`와 `EXPECT_*`를 모두 사용하고 있습니다. `EXPECT_*`는 failure가 발생해도 계속 진행하기 때문에 Test를 한 번 실행하면서 최대한 많은 문제를 찾아내고 싶을 때 주로 사용합니다. 반면에 failure가 발생했을 때에 Test를 계속 진행하는 것이 아무런 의미가 없는 경우에는 `ASSERT_*`를 사용해서 바로 종료시키는 것이 좋습니다. 예를 들어, `DequeueWorks`의 `ASSERT_NE(n, nullptr)` 부분을 보면 `n`이 `nullptr`이라면 그 다음에 나오는 `EXPECT_EQ(*n, 1);`에서 segfault가 발생할 것이 자명하기 때문에 테스트를 진행하는 것은 의미가 없어집니다. 바로 이런 경우에는 Test를 중단시키는 것이 논리적으로 맞습니다.

정리해보면 위의 test program을 실행하면 아래와 같은 일들이 일어나게 됩니다.

1. googletest가 `QueueTest` object(이하 `t1`)를 새로 하나 생성합니다.
2. `t1.Setup()`에서 공유데이터를 초기화합니다.
3. 첫 번째 Test인 `IsEmptyInitially`가 `t1`을 통해 수행됩니다.
4. `IsEmptyInitially`가 종료되면 `t1.TearDown()`이 호출되어 필요한 정리작업을 수행합니다.
5. `t1`이 소멸됩니다.
6. 1~5단계를 반복합니다. 여기서는 다음 Test인 `DequeueWorks`에 대해서 동일한 작업을 진행합니다.

**Availability**: Linux, Windows, Mac.

## 테스트 실행하기

`TEST()` 및 `TEST_F()`를 통해 정의된 Test들은 googletest에 등록되어 내부적으로 관리하게 됩니다. 그렇게 등록된 Test들은 googletest가 자동으로 실행해 주기 때문에 다른 C++언어의 테스트 프레임워크와 달리 각각의 Test를 별도로 실행할 필요가 없습니다.

Test program을 구현한 후 `main()` function에서 `RUN_ALL_TESTS()`만 호출하면 등록된 모든 Test를 실행해줍니다. ` RUN_ALL_TESTS()`는 `TEST()` 및 `TEST_F()`를 사용한 모든 Test를 수행하고 성공하면 `0`을 반환하고 실패하면 `1` 또는 다른 값을 반환합니다.

`RUN_ALL_TESTS()`을 수행하면 아래와 같은 동작들이 일어나게 됩니다.

* 모든 googletest flag의 상태를 저장합니다.
* 제일 먼저 수행해야 할 Test를 위해서 test fixture object를 생성합니다.
* `SetUp()`을 통해 위에서 생성한 test fixture를 초기화합니다.
* Test를 수행합니다.
* `TearDown()`을 통해 text fixture를 정리합니다.
* test fixture object를 삭제합니다.
* 모든 googletest flags의 상태를 복구합니다.
* 위의 단계들을 모든 Test에 대해 반복합니다.

주의할 점은 어떤 Test를 진행하다가 fatal failure가 발생했다면 그 다음 단계들은 수행하지 않고 다음 Test로 넘어간다는 점입니다.

> IMPORTANT: `RUN_ALL_TESTS()`는 `main()`의 맨 끝에서 호출되고 또 반환되어야만 합니다. 그렇지 않으면 compile error가 발생합니다. googletest가 이렇게 설계된 이유는 자동화된 testing service 환경에서 test program의 성공여부를 stdout/stderr이 아닌 exit code를 통해서 자동으로 확인할 수 있도록 하기 위함입니다. 이를 위해서 `main()`은 반드시 `RUN_ALL_TESTS()`를 반환해야 합니다.
>
> 또한, `RUN_ALL_TESTS()`는 딱 한 번만 호출되어야 합니다. 이 function을 두 번 이상 호출하게 되면 몇몇 advanced googletest features(예: thread-safe [death tests](http://collab.lge.com/main/display/SEETVTEST/Google+Test%3A+Advanced#DeathTests))와의 출동이 발생하기 때문에 지원하지 않고 있습니다.

**Availability**: Linux, Windows, Mac.

## main() function 작성하기

마지막으로 test program의 `main()` function을 작성하는 방법입니다.

사용자가 구현을 시작할 때, 아래 코드를 복사해서 사용하는 것도 좋은 방법입니다.

```c++
#include "this/package/foo.h"
#include "gtest/gtest.h"

namespace {

// The fixture for testing class Foo.
class FooTest : public ::testing::Test {
 protected:
  // You can remove any or all of the following functions if its body
  // is empty.

  FooTest() {
     // You can do set-up work for each test here.
  }

  ~FooTest() override {
     // You can do clean-up work that doesn't throw exceptions here.
  }

  // If the constructor and destructor are not enough for setting up
  // and cleaning up each test, you can define the following methods:

  void SetUp() override {
     // Code here will be called immediately after the constructor (right
     // before each test).
  }

  void TearDown() override {
     // Code here will be called immediately after each test (right
     // before the destructor).
  }

  // Objects declared here can be used by all tests in the test suite for Foo.
};

// Tests that the Foo::Bar() method does Abc.
TEST_F(FooTest, MethodBarDoesAbc) {
  const std::string input_filepath = "this/package/testdata/myinputfile.dat";
  const std::string output_filepath = "this/package/testdata/myoutputfile.dat";
  Foo f;
  EXPECT_EQ(f.Bar(input_filepath, output_filepath), 0);
}

// Tests that Foo does Xyz.
TEST_F(FooTest, DoesXyz) {
  // Exercises the Xyz feature of Foo.
}

}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
```

`::testing::InitGoogleTest()`는 command line으로부터 전달된 googletest flag들을 파싱하고 초기화합니다. 사용자는 flag를 통해서도 test program의 동작을 제어할 수 있습니다. Flag와 관련한 상세한 내용들은 [AdvancedGuide](advanced.md#test-program을-실행하는-다양한-방법)에서 더 자세히 다루고 있습니다. 그리고 `::testing::InitGoogleTest()`는 `RUN_ALL_TESTS()`보다 먼저 호출되어야만 합니다. 그렇지 않으면 flag들이 초기화되지 않은 상태에서 Test가 수행되기 때문에 문제가 발생합니다.

윈도우 환경에서는 `InitGoogleTest()`가 wide string에도 잘 동작되기 때문에 `UNICODE`모드에서 컴파일 된 프로그램에서도 사용할 수 있습니다.

만약, 매번 동일한 `main()`코드를 구현하는 것이 귀찮다면 gtest\_main 이라는 library를 링크해서 사용하기를 바랍니다. 거기에는 `main()`을 포함한 기본적인 내용들이 포함되어 있습니다.

NOTE: 예전에 사용하던 `ParseGUnitFlags()`는 `InitGoogleTest()`으로 대체되었습니다.

## Known Limitations

* googletest는 thread-safe하게 설계되었습니다. 다만, 이러한 설계는 `pthreads` library를 사용할 수 있는 시스템에만 해당됩니다. 예를 들어 Windows 환경에서 2개의 thread를 생성하고 각각의 thread에서 googletest assertion을 동시에 사용한다면 문제가 발생할 수 있습니다. 물론 아주 예외적인 경우를 제외하고는 대부분 main thread에서 assertion을 수행하기 때문에 괜찮을 것입니다. 사용자의 환경에 맞는 동기화를 직접 구현하려 한다면 `gtest-port.h` 파일을 사용하시기 바랍니다.
