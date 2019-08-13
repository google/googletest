# Googletest FAQ

## Test Case 또는 Test Suite의 이름을 정할 때, 밑줄을 사용하면 안되는 이유가 뭔가요?

먼저, C++ 자체적으로도 밑줄(`_`)은 특별한 의미를 갖습니다. 예를 들면, 표준 library 및 compiler 개발자가 사용할 수 있도록 아래 identifier들을 예약해 두었습니다.

1. `_`로 시작하고 다음 글자로 대문자가 오는 identifier
2. `_`을 2개 연속(`__`)으로 사용하는 identifier

사용자 코드에서 위와 같은 방법으로 identifier는 만드는 것은 *금지되어* 있습니다.

이러한 제약이 `TEST`, `TEST_F`에 어떤 영향을 주는지 확인해 보겠습니다.

사용자가 `TEST(TestSuiteName, TestName)`라고 구현하면 googletest는 `TestSuiteName_TestName_Test`라는 identifier를 생성해줍니다. 이 때, `TestSuiteName` 또는 `TestName`에 밑줄(`_`)이 포함되면 어떤 일이 발생할까요?

1. `TestSuiteName`이 `_Foo`와 같이 `_`로 시작하면서 바로 뒤에 대문자를 포함하면 `_Foo_TestName_Test`라는 identifier가 만들어 집니다. C++에서 금지된 예약어입니다.
2. `TestSuiteName`이 `Foo_`와 같이 `_`로 끝나게 되면 `Foo__TestName_Test`라는 이름이 만들어 집니다. C++에서 금지된 예약어입니다.
3. `TestName`이 `_Bar`와 같이 `_`로 시작하면 `TestSuiteName__Bar_Test`라는 이름이 만들어집니다. C++에서 금지된 예약어입니다.
4. `TestName`이 `Bar_`와 같이 `_`로 끝나면 `TestSuiteName_Bar__Test`라는 이름이 만들어 집니다. C++에서 금지된 예약어입니다.

위와 같은 이유로 `TestSuiteName`과 `TestName`은 `_`로 시작하거나 끝나면 안됩니다. 물론 `TestSuiteName`이 `_`로 시작하면서 바로 뒤에 소문자를 사용하면 괜찮기는 하지만 그런 예외를 생각하면서 구현하는 것이 오히려 어려울 것입니다. 간단하게 `_`는 사용하지 않는 것이 좋습니다.

어떤 사용자는 `TestSuiteName`, `TestName`의 중간에는 `_`를 써도 괜찮다고 생각할 수 있습니다. 그러나 그것도 위험합니다. 아래 예제를 보시기 바랍니다.

```c++
TEST(Time, Flies_Like_An_Arrow) { ... }
TEST(Time_Flies, Like_An_Arrow) { ... }
```

위의 코드는 동일한 이름(`Time_Files_Like_An_Arrow_Test)`을 가진 2개의 Test를 만들려고 하기 때문에 당연히 문제가 됩니다.

언급한 내용들을 유념하여 `TestSuiteName`, `TestName`에는 `_`를 사용하지 않기를 당부합니다. 복잡한 규칙을 기억하는 것보다는 아예 사용하지 않는 것이 편하겠지요. Googletest의 개선여지도 조금 남겨둘 수 있어서 여러모로 좋습니다.

물론 예외상황을 잘 찾아서 구현하면 지금 당장은 문제없이 동작할 수도 있습니다. 그러나 compiler나 googletest 버전이 변경되면 언젠가는 문제가 발생할지도 모릅니다. 따라서 `_`를 사용하지 않는다는 단순한 규칙을 기억하고 지켜주시기 바랍니다.

## 왜 `EXPECT_EQ(NULL, ptr)`, `ASSERT_EQ(NULL, ptr)`만 지원하고 `EXPECT_NE(NULL, ptr)`, `ASSERT_NE(NULL, ptr)`은 지원하지 않나요?

설명에 앞서 `EXPECT_NE(nullptr, ptr)`, `ASSERT_NE(nullptr, ptr)`와 같이 `NULL`을 `nullptr`로 변경하면 잘 동작할 것입니다. C++ style guide에서도 확인할 수 있듯이 `nullptr`과 `NULL`은 다릅니다. 간단한 예로 `NULL`을 사용하면 발생할 수 있는 타입관련 문제들이 `nullptr`를 사용하면 발생하지 않습니다. `nullptr`이 더 안전합니다.

그럼에도 `EXPECT_XX()`, `ASSERT_XX()`와 같은 macro에 `NULL`을 사용해야 한다면 몇 가지의 template meta programming trick이 필요합니다. 다만, 그러한 trick을 적용하기 위한 소스코드의 양이 상당히 많기 때문에 googletest에서는 꼭 필요한 곳에만 `NULL`을 사용할 수 있도록 했습니다.

`EXPECT_EQ()`는 첫 번째 argument에는 *expected value*, 두 번째 argument에 *actual value*를 전달받습니다. 이런 맥락에서 "어떤 포인터가 `NULL`인지 확인하고 싶다"는 것도 자연스러운 생각의 확장입니다. 또 실제로도 `EXPECT_EQ(NULL, some_expression)`과 같이 사용하게 해달라는 요청이 여러번 있었기 때문에 googletest는 `EXPECT_EQ(NULL, ptr) `을 지원하게 되었습니다.

반면에 `EXPECT_NE(NULL, ptr)`에 대한 요청은 그렇게 많지 않았습니다. 왜냐하면 assertion이 실패하면 `ptr`이 `NULL`이라는 것을 사용자가 바로 알게되기 때문에 `ptr`에 대한 추가적인 정보가 굳이 필요하지 않기 떄문입니다. 즉, `EXPECT_TRUE(ptr != NULL)`만으로도 부족함이 없었습니다.

또한, `EXPECT_NE(NULL, ptr)`를 지원한다면 `EXPECT_NE(ptr, NULL)`도 지원해야 합니다. 왜냐하면 `EXPECT_EQ`와 다르게 `EXEPCT_NE`에는 argument 순서에 대한 규약이 없기 때문입니다. 즉, *expected value*가 첫 번째 argument일 수도 있고, 두 번째 argument일 수도 있습니다. 이런 상황에서 `NULL`을 지원하려면 template meta programming trick 코드도 2배로 사용되게 됩니다. 결론적으로 유지보수 비용은 많이 증가하는 반면에 그러한 구현으로 얻는 이득이 크지 않다고 판단되어 지원하지 않게 되었습니다.

마지막으로 gMock의 matcher가 발전함에 따라 `EXPECT_THAT(value, matcher)`을 사용해보기를 추천합니다. 왜냐하면 `EXPECT_XX`와 같은 macro는 사용자가 원하는 방향으로 확장하기 어렵지만 matcher는 그러한 확장이 용이하기 때문입니다. 새로운 matcher를 만들수도 있고 여러 matcher들을 조합해서 사용하는 것도 가능합니다. 이런 이유로 `EXPECT_XX()`보다 `EXPECT_THAT()`과 matcher 사용을 좀 더 권장하고 있습니다.

## Interface가 1개 있고 이를 상속받고 구현한 concrete class는 여러개가 있습니다. Interface 레벨에서 각 concrete class가 잘 동작하는지 확인하려면 어떻게 해야 하나요?

동일한 interface에 대한 여러가지 구현을 테스트해야 한다면 typed test 혹은 value-parameterized test를 사용하면 됩니다. 두개 중에서 어떤 방법을 사용할지는 전적으로 사용자의 선택에 달려 있습니다. 아래에 상황에 따라 선택하는 방법을 간단히 가이드합니다.

- Typed test는 concrete type들이 동일한 방법으로 instance를 생성하는 경우에 좋습니다. 예를 들어 모든 concrete class들이 public default constructor를 사용한다거나 혹은 동작방식이 같은 factory function을 사용하는 경우입니다. 전자는 모든 concrete class가 `new TypeParam` 과  같은 방법으로 생성할 수 있는 경우를 의미합니다. 그렇게 되면 `TypeParam`만 바꾸는 방식으로 typted test가 가능해집니다. 후자는 `CreateInstance<TypeParam>()`과 같은 factory function을 사용하는 경우이며 여기서도 마찬가지로 `TypeParam`만 바꾸면 다양한 concrete type을 테스트할 수 있습니다.
- Value-parameterized test는 concrete type들이 서로 다른 방법으로 instance를 생성하는 경우에 유용합니다. 예를 들어 `Foo`와 `Boo`라는 concrete class가 있을 때, 각각의 instance 생성방식이 `new Foo`와 `new Bar(5)`처럼 다를 수 있습니다. Value-parameterized test에서는 이러한 차이를 극복하기 위해 factory function을 생성하고 해당 function의 function pointer를 test의 parameter로 넘겨주는 방식을 사용하게 됩니다.
- Typed test는 테스트가 실패하면, 해당 type을 출력해주도록 되어 있습니다. 그렇게 해야만 어떤 type이 잘못되었는지 빠르게 알 수 있기 때문입니다. 반면에 value-parameterized test는 테스트가 어떤 interation에서 멈췄는지만 알려주기 때문에 디버깅이 약간 더 어렵습니다. 이를 문제를 해결하기 위해서는 iteration name을 반환하는 function을 직접 정의하고 value-parameterized test를 초기화할 때 `INSTANTIATE_TEST_SUITE_P`의 3번째 parameter로 넘겨주면 됩니다.
- Typed test를 사용할 때, 어디까지나 interface관점에서 테스트하는 것이지 concrete type 자체를 테스트하는 것이 아님을 기억하기 바랍니다. 쉽게 말해서 `my_concrete_impl`를 테스트하는 것이 아니라 `implicit_cast<MyInterface*>(my_concrete_impl)`가 잘 동작하는지 테스트하는 것입니다. 이는 value-parameterized test를 사용할 때도 동일합니다.

더 혼란스럽게 만든건 아닌지 모르겠습니다. 아마도 위 방법들을 직접 사용해본다면 둘 간의 미묘한 차이를 이해하는데 도움이 될 것입니다. 한 번만 해보면 다음부터는 어떤 방법을 선택해야 할지 판단할 수 있게 될 것입니다.

## `ProtocolMessageEquals`를 사용했는데 Invalid proto descriptors와 관련된 run-time error가 발생합니다. 도와주세요!

**Note**: 시작하기 앞서 `ProtocolMessageEquals`와 `ProtocolMessageEquiv`는 *deprecated된* 기능입니다. `EqualsProto`를 사용하는 것이 더 좋습니다.

`ProtocolMessageEquals`와 `ProtocolMessageEquiv`가 최근에 변경되면서 invalid protocol buffer와 같은 문제에는 취약해 졌습니다. 그 결과 `foo.proto`라고 구현하면서 참조하고 있는 protocol message type 전체를 명시하지 않는다면 (예를 들어 `message<blah.Bar>`가 아니라 `message<Bar>`와 같이 사용하면) 아래와 같은 run-time error가 발생하게 됩니다.

```bash
... descriptor.cc:...] Invalid proto descriptor for file "path/to/foo.proto":
... descriptor.cc:...]  blah.MyMessage.my_field: ".Bar" is not defined.
```

만약, 위와 동일한 error message를 봤다면 `.proto` 파일이 잘못된 것이므로 앞서 설명한 것처럼 protocol message type 전체를 명시하도록 수정하면 잘 동작할 것입니다. `ProtocolMessageEquals`, `ProtocolMessageEquiv`을 새롭게 변경하다보니 나타난 문제입니다.

## Death test에서 어떤 상태를 변경하고 있습니다. 그러나 death test가 끝난 이후에는 변경한 상태가 유지되지 않는 것 같습니다. 왜 그런가요?

Death test는 child process를 생성하고 거기서 실행됩니다. 알다시피 child process에서 수정된 내용은 parent process에 영향을 주지 않습니다. 최악의 경우에 child process에서 crash가 발생한다고 하더라도 parent process(test program)는 종료되지 않습니다. 당연하게도 2개의 process가 서로 다른 실행환경을 가지기 때문에 대부분의 경우에 서로간에 영향을 주지 않습니다.

또한, parent process에서 method를 mocking한 후에 expectation을 설정했다고 하더라도 death test 내에서 해당 method를 호출한다면 parent process에서는 mock method가 호출되었는지 아닌지 알 수가 없습니다. 이런 상황에서는 해당 `EXPECT_CALL`도 death test 내부로 옮겨야 합니다.

## Opt mode에서 `EXPECT_EQ(htonl(blah), blah_blah)` 라고 사용하면 이상한 compile error가 발생합니다. 이거 googletest bug 아닌가요?

사실 bug는 `htonl()`에 있습니다.

`man htonl`을 통해 확인해보면 `htonl()`은 _function_입니다. 따라서 `htonl`을 function pointer로 사용하는 것은 적법합니다. 그러나 opt mode에서 `htonl()`은 macro로 정의됩니다. 이로 인해 `htonl`을 function pointer로 전달하는 `EXPECT_EQ(htonl(blah), blah_blah)`라는 코드에서 compile error가 발생하는 것입니다.

설상가상으로 `htonl()`의 구현에서 사용한 macro는 C++ 표준방법을 사용한 것이 아니라 `gcc`의 확장기능 중 하나를 사용했습니다. 어찌됐건 `htonl()`가 opt mode에서는 function이 아니라 macro이기 때문에 `Foo<sizeof(htonl(x))>()`와 같은 코드도 구현할 수가 없습니다. (여기서 `Foo`는 정수타입의 template argument를 전달 받는 template class입니다.)

`EXPECT_EQ(a, b)`는 내부적으로 `sizeof(... a ... )`를 사용합니다. 따라서 `a`가 `htonl()`을 포함한다면 opt mode에서 문제가 될 것입니다. 더군다나 C++표준 macro를 사용한 것도 아니기 때문에 플랫폼과 compiler에 따라 그 결과가 달라질 수 있습니다. `EXPECT_EQ`에서 `htonl()`의 bug를 바로 잡는 것은 어렵습니다.

`//util/endian/endian.h`에 나와있는 것처럼 `htonl()`에는 다른 문제들도 더 있습니다. 이를 위한 대안으로 `ghtonl()`을 제공하고 있습니다. `ghtonl()`은 `htonl()`과 동일하면서도 위와 같은 문제점이 없습니다. 따라서 `htonl()`을 사용하고 있다면 `ghtonl()`으로 변경하기를 권장합니다. 제품코드와 테스트코드 모두 마찬가지입니다.

`ghtonl()`, `ghtons()`를 사용하려면  `//util/endian`이 빌드될수 있도록 추가해야 합니다. 해당 library는 header file 1개만 포함하고 있기 때문에 binary 사이즈도 그렇게 증가하지는 않을 겁니다.

## Class 내부에 static const member variable을  정의했는데도 불구하고 "undefined references"라는 문제가 발생하고 있습니다. 왜 그럴까요?

만약, 아래와 같이 class 내부에 static data member를 정의했다면

```c++
// foo.h
class Foo {
  ...
  static const int kBar = 100;
};
```

Class 바깥에서도 정의를 해줘야 합니다.

```c++
const int Foo::kBar;  // No initializer here.
```

**C++**에서 static member variable을 사용할때는 항상 이렇게 해야합니다. 이러한 문법을 지키지 않으면서 googletest 기능을 사용하면 안 됩니다. Googletest 자체도 C++을 사용하기 때문입니다.

## 다른 test fixture로부터 상속받는 것이 가능한가요?

가능합니다.

설명에 앞서 googletest에서 test fixture와 test suite은 서로 동일한 이름을 사용함으로써 연결됩니다. 다시 말해서 `TEST_F()`의 첫 번째 argument(test suite)는 동일한 이름의 test fixture class가 어딘가에 정의되어 있어야 합니다. 이런식으로 test suite과 test fixture가 1:1 매칭되는 상황에서 여러개 test fixture에 공통적으로 적용해야 할 내용이 있다면 어떻게 해야 할까요? 이런 경우에는 상속을 사용하면 됩니다.

예를 들어 GUI library를 위한 여러가지 test suite들이 이미 구현되어 있다고 가정하겠습니다. Font를 위한 test suite, brush를 위한 test suite 등 다양한 test suite들이 이미 있을 건데요. 그러한 test suite들이 공통적으로 memory leak도 확인할 수 있도록 개선하려 합니다. 이를 위해서는 먼저 memory leak 검증을 수행하기 위한 test fixture를 하나 생성합니다. 그 다음에 기존에 존재하던 font test fixture와 brush test fixture가 memory leak test fixture를 상속받게 합니다. 그렇게 되면 font test fixture와 brush test fixture는 test suite과의 1:1 관계도 그대로 유지하면서 memory test fixture라는 공통적인 검증기능도 수행할 수 있게 됩니다.

간단한 예제코드는 아래와 같습니다.

```c++
// Defines a base test fixture.
class BaseTest : public ::testing::Test {
 protected:
  ...
};

// Derives a fixture FooTest from BaseTest.
class FooTest : public BaseTest {
 protected:
  void SetUp() override {
    BaseTest::SetUp();  // Sets up the base fixture first.
    ... additional set-up work ...
  }

  void TearDown() override {
    ... clean-up work for FooTest ...
    BaseTest::TearDown();  // Remember to tear down the base fixture
                           // after cleaning up FooTest!
  }

  ... functions and variables for FooTest ...
};

// Tests that use the fixture FooTest.
TEST_F(FooTest, Bar) { ... }
TEST_F(FooTest, Baz) { ... }

... additional fixtures derived from BaseTest ...
```

필요하다면, base test fixture를 상속받은 test fixture를 또 다시 상속하는 것도 가능합니다. 상속의 깊이에 특별한 제한은 없습니다.

보다 상세한 예제들은 [sample5_unittest.cc](../../samples/sample5_unittest.cc)에서 확인하기 바랍니다.

## "void value not ignored as it ought to be"라는 compile error가 발생합니다. 이게 무슨 뜻인가요?

아마도 `ASSERT_*()`에 return type이 `void`가 아닌 function을 사용했을 확률이 큽니다. `ASSERT_*()`는 `void`를 반환하는 function에만 사용할 수 있으며 그 이유는 googletest가 expception disabled이기 때문입니다. [여기](advanced.md#assertion을-사용가능한-곳)에서 보다 자세한 내용을 확인할 수 있습니다.

## Hang, seg-fault 등이 발생하면서 death test가 잘 동작하지 않습니다. 어떻게 해야할까요?

Googletest에서 death test는 기본적으로 child process에서 동작합니다. 그렇기 때문에 구현 시에도 세심한 주의가 필요합니다. Death test를 사용하려면 먼저 death test가 어떻게 동작하는지 정확히 이해해야 합니다. [이 부분](advanced.md#death-test-의-동작방식)을 꼭 읽어주세요.

Death test를 사용할 때 parent process에 여러개의 thread가 있는 경우에는 문제가 발생하기도 합니다. 혹시 `EXPECT_DEATH()`를 수행하기 전에 이미 여러개의 thread를 생성하고 있다면 이러한 부분을 한 번 제거해 보시기 바랍니다. 예를 들어 thread를 생성하는 real object를 mock이나 fake로 바꿔서 thread를 생성하지 않도록 해볼 수 있습니다.

간혹 `main()`이 시작하기도 전에 thread를 생성버리는 library도 있기 때문에 위와 같은 방법이 항상 통하는 것은 아닙니다. 다음 방법으로는 `EXPECT_DEATH()`에서 해야할 일을 최대한 줄여보시기 바랍니다. 기본적인 것만 해도 문제가 발생하는지 확인해보고 아니면 반대로 parent process에서 해야할 모든 일을 death test로 옮겨보십시오. 여전히 문제가 발생하는지 확인해보기 바랍니다. 또는 death test style(동작방식)을 `"threadsafe"`로 변경해보는 것도 좋은 방법입니다. `"threadsafe"` 모드를 사용하면 테스트의 속도는 조금 느려지지만 안전성을 증가시켜줍니다.

Thread-safe death test는 child process를 생성하기 전에 아예 test program(parent process)을 하나 더 실행시킵니다. 이런 방법으로 안전성을 증가시켜줍니다. 다만, 이를 위해서는 동일한 test program이 여러개 실행되어도 문제없도록 구현해야 합니다.

마지막으로 당연한 이야기이긴 하지만, race condition, dead lock이 없는지도 확인해보기 바랍니다. 여타의 개발과 동일하게 이 부분은 사용자 스스로가 주의해서 개발해야 합니다.

## Test fixture에서 constructor/destructor 와 `SetUp()`/`TearDown()`중 어느것을 써야하나요?

먼저 기억해야 할 것은 googletest는 test fixture를 재사용하지 않는 다는 점입니다. 즉, 각각의 test case를 실행할때마다 test fixture object를 매번 **새로** 생성합니다. 따라서 해당 object 생성되고 삭제될 때마다 constructor/destructor 및 `SetUp()`/`TearDown()`도 새롭게 호출됩니다.

그럼 constructor/destructor 와 `SetUp()`/`TearDown()` 중에서는 어느 것을 선택해야 할까요? 먼저 constructor/destructor를 사용할 때의 장점은 아래와 같습니다.

- Constructor에서 member variable을 초기화함으로서 해당 변수를 `const`로 선언할 수 있게 됩니다. 아시다시피 `const`는 해당값을 변경하는 등의 행위를 막아줍니다.
- Test fixture class를 상속받아서 또 다른 test fixture class를 만든다고 가정해봅시다. C++에서는 base class의 constructor와 derived class의 constructor가 순서대로 모두 호출되도록 보장됩니다. 또한, destructor도 방향은 거꾸로지만 모두 호출되도록 보장됩니다. 하지만 `SetUp()/TearDown()`은 이러한 것이 보장되지 않으므로 실수할 확률이 있습니다. 즉, derived class에서 base class의 `SetUp()`을 호출하지 않는 등의 문제가 발생할 수 있습니다.

다음으로 `SetUp()/TearDown()`을 사용하면 아래와 같은 장점이 있습니다.

*   C++의 constructor, destructor에서는 virtual function을 호출할 수 없습니다. 물론, 호출할 수는 있지만 그렇게 되면 derived class의 function을 동적으로 호출하는 것이 아니라 constructor/destructor가 구현된 base class의 function을 호출하게 됩니다. 이렇게 동작하는 이유는 base class의 constructor가 수행되는 시점에 derived class의 constructor는 아직 호출되지 않았을 것이고 이렇게 초기화되지 않은 derived class의 method를 호출하는 것은 매우 위험하기 때문입니다. 해당 virtual method가 초기화되지 않은 값들을 사용하기라도 한다면 큰 문제가 발생할 수 있습니다. 따라서 이렇게 overriding 관계에 있는 virtual function을 사용하려 `SetUp()/TearDown()`을 사용할 수 밖에 없습니다.
*   Constructor/destructor에는 `ASSERT_xx`계열 assertion을 사용할 수 없습니다. (`EXPECT_xx`는 아닙니다.) 만약, constructor에서 이런 목적을 구현하려면 `abort`를 사용해야 하는데 이것은 test program 자체를 종료시키기 때문에 `ASSERT_xx`보다 비효율적입니다. 따라서 test fixture 레벨에서 `ASSERT_xx`계열 assertion을 사용하고 싶다면 `SetUp()`을 사용하는 것이 더 좋습니다.
*   Test fixture를 정리하는 과정에서 exception을 던지고 싶을 수도 있습니다. 그러나 C++에서destructor가 exception을 던지는 것은 미정의 동작이므로 그렇게 구현하면 안됩니다. 대부분의 경우 test program을 바로 종료시킬 것입니다. 이런 경우에도 역시 `TearDown()`을 사용하면 좋습니다. 참고로 C++에서는 STL과 같은 standard library들도 예외를 발생시킬 수 있습니다. (컴파일러에 exception이 활성화 되어 있다면)
*   Googletest team은 exception이 활성화되어 있는 플랫폼(Windows, Mac OS, Linux client-side)에서는 assertion도 exception을 던질 수 있도록 하는 부분에 대해 고민하고 있습니다. 그렇게 되면 subroutine에서 발생한 failure를 caller쪽으로 전달할 때, 사용자가 직접 구현할 필요가 없어집니다. 추후에 추가될지 모르는 이러한 기능을 위해서 destructor에는 googletest assertion을 사용하면 안 됩니다.

## `ASSERT_PRED*`를 사용할 때, "no matching function to call" 이라는 compile error가 발생했습니다. 어떻게 해야 하나요?

만약 `ASSERT_PRED*`, `EXPECT_PRED*`에 전달한 predicate function이 overloaded 혹은 template function이라면 compiler가 어떤 것을 선택해야 하는지 모호해집니다. 그런 경우에는 `ASSERT_PRED_FORMAT*`이나 `EXPECT_PRED_FORMAT*` 을 사용해야 합니다.

네, 질문과 같은 compile error가 발생하면 `(ASSERT|EXPECT)_PRED_FORMAT*`으로 변경하는 것이 제일 좋습니다. 위의 compile error를 해결할 수 있을 뿐만 아니라 assertion이 실패했을 때의 failure message도 더 자세하게 출력해 주는 장점이 있기 때문입니다. 만약 꼭 `(ASSERT|EXPECT)_PRED*`를 사용해야 한다면 compiler에게 overloaded function, template function의 여러 버전 중에서 어떤 것을 사용할지 알려주면 됩니다.

아래에 예제가 있습니다.

```c++
bool IsPositive(int n) {
  return n > 0;
}

bool IsPositive(double x) {
  return x > 0;
}
```

위와 같은 overloaded function을 기존과 같이 사용하면 compile error가 발생합니다.

```c++
EXPECT_PRED1(IsPositive, 5);
```

이제 아래처럼 변경해서 모호함을 제거하기 바랍니다.

```c++
EXPECT_PRED1(static_cast<bool (*)(int)>(IsPositive), 5);
```

`static_cast` 연산자에 사용한 `bool (*)(int)`는 `bool IsPositive(int n)` 버전을 의미합니다.

다음은 template function을 사용할때의 예제입니다.

```c++
template <typename T>
bool IsNegative(T x) {
  return x < 0;
}
```

이 경우에도 어떤 type을 사용할 것인지 명확하게 지정해줘야 합니다.

```c++
ASSERT_PRED1(IsNegative<int>, -5);
```

만약, 하나 이상의 template argument가 1개 이상이라면 어떻게 해야 할까요?

```c++
ASSERT_PRED2(GreaterThan<int, int>, 5, 0);
```

위의 코드는 compile에 실패합니다. C++ 전처리기에서 `ASSERT_PRED2`에 4개의 argument를 준다고 판단하기 때문입니다. 이런 경우에는 아래처럼 괄호로 감싸줘야 compile에 성공합니다.

```c++
ASSERT_PRED2((GreaterThan<int, int>), 5, 0);
```

## `RUN_ALL_TESTS()`를 호출하면 "ignoring return value"라는 compile error가 발생합니다. 왜 그럴까요?

`RUN_ALL_TEST()`을 사용할 때는 항상 아래처럼 구현해야 합니다.

```c++
  return RUN_ALL_TESTS();
```

`return` 없이 구현하는 것은 상당히 **위험하고 잘못된** 코드입니다.

```c++
  RUN_ALL_TESTS();
```

Testing service는 `RUN_ALL_TESTS()`의 반환값을 통해서 테스트의 성공이나 실패를 판단하게 됩니다. 따라서 `return`이 없다면 실제로는 assertion failure가 발생했는데도 성공했다고 여겨질 수 있으며 매우 잘못된 정보를 전달하게 될 것입니다.

이러한 문제를 좀 더 확실하게 알려야 했기 때문에 `gcc`같은 경우 반환값이 없는 `RUN_ALL_TESTS()` 코드를 작성하면 compile error가 발생하도록 했습니다. 

물론 해결방법은 간단합니다. 질문과 같은 compile error가 발생했다면 `RUN_ALL_TESTS()`가 `main()` function의 `return`이 되도록 하면 됩니다.

만약, 이렇게 변경했더니 테스트코드가 실패하나요? 그랬다면 원래 존재하던 문제를 이번 기회에 찾았다고 봐야합니다. 이제 문제점을 대한 개선작업을 진행하기 바랍니다. `return RUN_ALL_TESTS()`으로 변경하는 것은 테스트코드의 동작에는 아무런 영향을 주지 않으며 오히려 정상적으로 동작하도록 해주기 때문입니다.

## Constructor 또는 destructor가 값을 반환할 수 없다는 compile error가 발생합니다. 어떻게 해야 하나요?

Googletest에서는 assertion과 함께 streaming message를 바로 출력하는 것이 가능합니다.

```c++
  ASSERT_EQ(1, Foo()) << "blah blah" << foo;
```

바로 이와 같은 기능을 지원하기 위해서 constructor/destructor에서는 `ASSERT*`, `FAIL*`를 사용할 수 없도록 했습니다. (`EXPECT*`, `ADD_FAILURE*`는 아닙니다) 이를 위한 해결방법은 constructor/destructor 대신에 private void method를 사용해서 원하는 내용을 구현하는 것입니다. 또는 `ASSERT_*()`를 `EXPECT_*()` 로 바꾸는 것도 방법입니다. 이와 관련한 자세한 내용은 [여기](advanced.md#assertion을-사용가능한-곳)를 확인하기 바랍니다.

## `SetUp()`이 호출되지 않습니다. 왜 그럴까요?

C++은 대문자/소문자가 중요합니다. 혹시 `Setup()`이라고 구현하진 않았나요?

이와 유사하게 `SetUpTestSuite()`도 실수로 인해 `SetupTestSuite()`처럼 구현하는 경우가 종종 있습니다.


## 동일한 test fixture logic을 사용해서 test suite을 여러개 만들고 싶습니다. Test fixture class를 새로 정의하는 것 외에는 방법이 없나요?

Test fixture class를 새로 정의하지 않아도 됩니다. 기존에 아래와 같이 구현했다면,

```c++
class FooTest : public BaseTest {};

TEST_F(FooTest, Abc) { ... }
TEST_F(FooTest, Def) { ... }

class BarTest : public BaseTest {};

TEST_F(BarTest, Abc) { ... }
TEST_F(BarTest, Def) { ... }
```

`typedef`를 사용하도록 변경하면 됩니다. Class를 새로 정의하지 않아도 되고 googletest의 규칙도 깨트리지 않습니다.

```c++
typedef BaseTest FooTest;

TEST_F(FooTest, Abc) { ... }
TEST_F(FooTest, Def) { ... }

typedef BaseTest BarTest;

TEST_F(BarTest, Abc) { ... }
TEST_F(BarTest, Def) { ... }
```

## Googletest의 출력물이 전체 LOG에 포함되어 알아보기가 힘듭니다. 어떻게 해야 하나요?

Googletest 출력물은 최대한 간결하게 제공하고 있습니다. 만약, 질문과 같은 상황이 발생했다면 사용자의 테스트가 자체적으로 너무 많은 LOG를 출력하는 것은 아닌지 확인해 볼 필요가 있습니다. 만약, 사용자의 LOG도 꼭 필요한 상황이라면 아래와 같은 방법을 통해 문제를 해결하기 바랍니다.

`LOG`는 대부분의 경우 stderr로 출력되기 때문에, googletest의 출력물은 stdout으로 출력되도록 설계했습니다. 따라서 test program을 실행할 때 redirection을 사용하면 LOG를 분리할 수 있습니다.

```shell
$ ./my_test > gtest_output.txt
```

## 왜 전역변수보다 test fixture를 선호해야 하나요?

이를 위한 몇가지 이유가 있습니다.

1. 테스트 코드에서 전역변수를 사용하고 싶을 때가 있습니다. 다만, 이러한 구현은 여러 test case가 전역변수를 공유할 때 문제를 발생시킬 수 있으며 디버깅도 어렵게 합니다. Test fixture class는 개별 test case를 실행할 때마다 자신의 object를 새롭게 만들기 때문에 test case간에 영향을 주지 않습니다.(변수나 function의 이름은 같겠지만) 따라서 각각의 test가 독립적으로 수행될 수 있게 됩니다.
2. 전역변수는 global namespace를 오염시킵니다.
3. Test fixture class는 상속에 의해 재사용될 수 있습니다. 전역변수로는 할 수 없는 일입니다. 여러개의 test suite을 대응하기 위해서 중복코드를 작성할 필요가 없습니다.

## `ASSERT_DEATH()` 의 statement argument에는 무엇이 올 수 있나요?

`ASSERT_DEATH(statement, regex)`와 같은 death test용 macro는 `statement`가 유효해야만 사용할 수 있습니다. 기본적으로 `statement`는 C++에서 유효한 코드이면 그대로 사용할 수 있습니다. 전역/지역변수를 참조하는 것도 가능하고 아래처럼 복잡한 내용들도 구현할 수 있습니다.

- 간단한 function 호출
- 복잡한 expression
- statement 조합

아래에 간단한 예제코드를 공유합니다. 좀 더 다양한 예제가 궁금하다면 gtest-death-test_test.cc 파일에서도 확인할 수 있습니다.

```c++
// A death test can be a simple function call.
TEST(MyDeathTest, FunctionCall) {
  ASSERT_DEATH(Xyz(5), "Xyz failed");
}

// Or a complex expression that references variables and functions.
TEST(MyDeathTest, ComplexExpression) {
  const bool c = Condition();
  ASSERT_DEATH((c ? Func1(0) : object2.Method("test")),
               "(Func1|Method) failed");
}

// Death assertions can be used any where in a function.  In
// particular, they can be inside a loop.
TEST(MyDeathTest, InsideLoop) {
  // Verifies that Foo(0), Foo(1), ..., and Foo(4) all die.
  for (int i = 0; i < 5; i++) {
    EXPECT_DEATH_M(Foo(i), "Foo has \\d+ errors",
                   ::testing::Message() << "where i is " << i);
  }
}

// A death assertion can contain a compound statement.
TEST(MyDeathTest, CompoundStatement) {
  // Verifies that at lease one of Bar(0), Bar(1), ..., and
  // Bar(4) dies.
  ASSERT_DEATH({
    for (int i = 0; i < 5; i++) {
      Bar(i);
    }
  },
  "Bar has \\d+ errors");
}
```

## `FooTest`라는 test fixture class가 있고 이를 사용하는 `TEST_F(FooTest, Bar)`라는 코드가 있습니다. 이 때, "no matching function for call to `FooTest::FooTest()`"라는 compile error가 발생하는데 왜 그럴까요?

Googletest는 test fixture class의 object를 생성해서 사용하기 때문에 test fixture class는 object 생성이 가능해야만 합니다. 즉, constructor를 가지고 제공해야 합니다. 기본적으로 compiler가 생성해주겠지만, 직접 정의해서 사용할 때는 몇가지를 유의해야 합니다.

- `FooTest`라는 test fixture class에 non-default constructor를 정의(`DISALLOW_EVIL_CONSTRUCTORS()` 를 사용해서)하고자 할 때에는 default constructor도 함께 정의해야 합니다. Default constructor에 구현할 내용이 없어도 빈 상태로라도 정의해야 합니다.
- `FooTest`라는 test fixture class가 const non-static data member를 가지고 있다면, default constructor를 생성한 후에 해당 default constructor의 initializer list에서 const member를 초기화해야만 합니다. `gcc`의 초기버전들은 이런 것을 강제화하지 않지만, `gcc 4`이후부터는 이렇게 해야합니다.

## 왜 `ASSERT_DEATH`는 이미 join된 thread에 대해서 문제가 있다고 하나요?

Linux pthread library를 사용할 떄, single thread에서 multiple threads로 한 번 넘어가면 원래 상태로는 절대로 돌아올 수 없습니다. 무슨 뜻이냐 하면 thread를 하나 만들면 이것을 관리하기 위한 manager thread도 기본적으로 함께 생겨나기 때문에 thread의 총 개수가 2개가 아니라 3개가 됩니다. 만약, 나중에 생겨난 thread가 main thread에 join되면 전체개수는 1 감소하지만 manager thread의 개수는 그대로 살아있습니다. 즉, join된 후에도 thread 개수는 2개가 유지됩니다. 결론적으로 multiple threads 환경으로 한 번 진입했다면 thread의 개수는 항상 2개 이상으로 유지되기 때문에 완벽하게 안전한 death test를 수행할 수는 없다는 의미입니다.

새로운 NPTL thread library를 사용하면 이러한 문제가 없습니다. NPTL은 manager thread를 만들지 않기 때문입니다. 그러나 테스트가 실행되는 모든 환경에서 NPTL thread library를 사용할 수 있음이 보장되지 않는 한 NPTL thread library에만 의존해서 구현하면 안됩니다.

## `ASSERT_DEATH`를 사용하기 위해 *DeathTest postfix를 사용할 때, 왜 개별 test case가 아닌 전체 test suite에 붙여야 하나요?

Googletest는 서로 다른 test suite에 포함된 test case들의 순서를 섞지 않습니다. 즉, 하나의 test suite을 마무리하고 다음 test suite으로 넘어갑니다. 그 이유는 test suite(test case가 아니라)을 위한 set-up/tear-down 작업도 존재하기 때문입니다. 이런 상황에서 test suite이 교차되면 효율적이지 않으며 문맥자체도 깔끔하게 유지되지 않게 됩니다.

테스트의 실행순서를 정할 때 test suite의 이름이 아니라 test case의 이름으로 정한다면 어떤 문제가 발생할까요? 아래 예제는 googletest가 `TEST_F(FooDeathTest, xxx)`가 아니라 `TEST_F(FooTest, xxxDeathTest)`와 같은 문법을 제공한다고 가정하고 작성한 코드입니다. (원래는 전자가 맞습니다.)

```c++
TEST_F(FooTest, AbcDeathTest) { ... }
TEST_F(FooTest, Uvw) { ... }

TEST_F(BarTest, DefDeathTest) { ... }
TEST_F(BarTest, Xyz) { ... }
```

위 코드는 test case이름으로 순서를 정할 수 있음을 가정한 것이며 사용자는 `FooTest.AbcDeathTest` -> `BarTest.DefDeatTest` -> `FooTest.Uvw` -> `BarTest.Xyz` 의 순서로 호출되기를 기대하고 있습니다.  그러나 googletest에서는 test suite끼리 교차되는 것을 금지하기 때문에 `BarTest.DefDeathTest`는 절대 `FooTest.Uvw`보다 먼저 수행될 수가 없습니다. 즉, test case이름으로 실행순서를 정하게 되면 test suite끼리 교차될 수 없다는 googletest의 기본원칙과 대립되는 상황이 발생할 수 있게 됩니다.

## Test suite 이름에 *DeathTest postfix를 붙이려고 합니다. 그런데 해당 test suite에 death test용이 아닌 test case도 포함되어 있다면 어떻게 해야 하나요?

그런 경우에는 test suite의 별칭을 만들어서 사용하면 됩니다. 즉, `FooTest`, `FooDeathTest`와 같이 사용하면 됩니다. 아래 예제를 확인하기 바랍니다.

```c++
class FooTest : public ::testing::Test { ... };

TEST_F(FooTest, Abc) { ... }
TEST_F(FooTest, Def) { ... }

using FooDeathTest = FooTest;

TEST_F(FooDeathTest, Uvw) { ... EXPECT_DEATH(...) ... }
TEST_F(FooDeathTest, Xyz) { ... ASSERT_DEATH(...) ... }
```

## Death test의 child process 관련 LOG는 실패했을 때만 출력됩니다. 성공했을 때도 LOG를 보려면 어떻게 해야 하나요? 

`EXPECT_DEATH()`의 내부 statement에서 발생하는 LOG를 출력하는 것은 parant process의 LOG를 이해하기 어렵게 만들기 때문에 googletest는 실패했을 때만 출력하는 정책을 선택했습니다.

만약 그래도 보고 싶다면 death test를 일부러 실패시켜야 합니다. 즉, `EXPECT_DEATH(statement, regex)`의 두 번째 argument인 `regex`를 살짝 바꿔서 일부러 실패하도록 하면됩니다. 물론, 추천할만한 방법은 아닙니다. 현재 진행중인 fork-and-exec-style death test의 개발을 완료한 후에 질문과 관련된 더 좋은 해결방방법도 찾아보겠습니다.

## Assertion을 사용하면 "no match for `operator<<`"라는 compile error가 발생합니다. 무슨 뜻인가요?

만약 사용자정의 타입(예: `FooType`)을 assertion에 사용했다면, `std::ostream& operator<<(std::ostream&, const FooType&)`도 제공해야만 합니다. 그래야 googletest에서 해당 타입의 정보를 출력해줄 수 있습니다.

추가적으로 `FooType`이 특정 namespace에 선언되어 있다면 `operator<<`도 *같은* namespace에 정의해야 합니다. 이와 관련한 자세한 정보는 https://abseil.io/tips/49를 참조하세요.

## Windows에서 memory leak message들을 안 보이게 하려면 어떻게 하면 될까요?

Googletest singleton은 정적으로 초기화되기 데는데 이 때 heap에도 memory allocation을 하게 됩니다. 이로 인해 테스트가 종료되는 시점에 Visual C++ memory leak detector는 해제되지 않은 memory가 있다고 보고하게 됩니다. 이러한 memory leak message가 보기 싫다면 `_CrtMemCheckpoint`와 `_CrtMemDumpAllObjectsSince`를 사용하면 됩니다. 이렇게 Windows 환경에서 heap check/debug routine을 조절하는 방법은 MSDN에서 상세히 확인할 수 있습니다.

## 현재 코드가 test 중임을 어떻게 알 수 있나요?

혹시 테스트가 진행 중인지 확인하는 목적이 제품코드에서 테스트인지 아닌지에 따라 분기하는 코드를 구현하기 위함인가요? 그렇다면 좋은 방법이 아닙니다. 제품코드에 테스트관련 의존성이 생기기 때문입니다. 사소한 실수로 인해서 제품코드에서 테스트코드가 수행되버릴 확률도 높아집니다. [Heisenbugs](https://en.wikipedia.org/wiki/Heisenbug)(100% 재현되는게 아니라 간헐적으로 재현되는 bug)라도 발생하면 많은 시간을 소모하게 될 것입니다. Googletest에서도 그런 방법을 제공하지 않습니다.

실행중에 테스트가 수행중인지 확인해서 분기하지 말고, 제품코드와 테스트코드에 동시에 사용할 수 있는 일반화된 코드를 개발하는 것이 중요합니다. 이를 위해서 추천하는 방법은 [Dependency Injection](https://en.wikipedia.org/wiki/Dependency_injection)을 사용하는 것입니다. 제품코드와 테스트코드의 의존성을 제거하고 구분이 필요하다면 [`testonly`](https://docs.bazel.build/versions/master/be/common-definitions.html#common.testonly)와 같은 빌드옵션을 통해서 구부하기 바랍니다.

정말 추천하지는 않지만 실행중에 분기하는 것 외에는 정말 방법이 없다면, 제품 실행파일과 테스트 실행파일의 이름을 `_test`와 같은 postfix를 사용해서 구분하기 바랍니다. 그런 다음에 `main()`으로 전달되는 `argv[0]` 변수를 확인하면 됩니다. `argv[0]`에는 실행파일 이름이 저장되어 있기 때문에 구분이 가능할 것입니다.

## 개별 test case를 비활성화 할 수 있을까요?

테스트가 깨졌는데 지금 바로 고칠 수 없는 경우가 발생할 수도 있습니다. 이런 경우에는 `DISABLED_` 라는 prefix를 `TEST()`의 두 번째 argument 앞에 붙여줍니다. 이렇게 하면 테스트 실행에서 제외됩니다. 이런 비활성화 방법은 `#if 0`을 사용해서 주석처리하는 것보다 효츌적입니다. 왜냐하면 `DISABLED_` 비활성화는 컴파일 대상에는 계속 포함되기 때문에 아예 버려지는 것이 아니며 다시 활성화되기 전까지 유지보수 될 수 있습니다.

만약, 이렇게 비활성화한 test case들도 실행해보고 싶다면 test program을 실행할 때, `--gtest_also_run_disabled_tests`라는 flag만 붙여주면 됩니다.

## 서로 다른 namespace라면 같은이름을 가진 `TEST(Foo, Bar)`를 정의해도 되나요?

가능합니다.

이런 경우에 적용되는 규칙은 **동일한 test suite 이름을 사용하는 모든 test case는 동일한 test fixture class를 사용해야 한다**는 것입니다. 말이 좀 어렵운데 아래 예제를 보겠습니다.

```c++
namespace foo {
TEST(CoolTest, DoSomething) {
  SUCCEED();
}
}  // namespace foo

namespace bar {
TEST(CoolTest, DoSomething) {
  SUCCEED();
}
}  // namespace bar
```

위 코드는 `CoolTest`라는 이름의 test suite가 있고, namespace별로 1개씩의 test case가 존재합니다. 먼저 2개의 test case는 각각의 full name이 namespace로 구분되기 때문에 문제가 없습니다. 다음으로 test suite이 동일하면서 test fixure class도 동일하기 때문에 또 문제가 없습니다. 이 때의 test fixture class는 `::testing::Test`를 의미하는데 왜냐하면 googletest에서 `TEST_F`가 아니라 `TEST`를 사용하면 test fixture class로서 `::testing::Test`를 공통적으로 사용하기 때문입니다.

반면에 아래 코드는 적법하지 않으며 runtime error가 발생합니다. 왜냐하면 `CoolTest`라는 동일한 test suite에 포함된 2개의 test case가 서로 다른 test fixture class를 사용하고 있기 때문입니다. 위에 있는 것은 `foo::CoolTest`를 사용하고 아래는 `bar::CoolTest`를 사용하게 됩니다.

```c++
namespace foo {
class CoolTest : public ::testing::Test {};  // Fixture foo::CoolTest
TEST_F(CoolTest, DoSomething) {
  SUCCEED();
}
}  // namespace foo

namespace bar {
class CoolTest : public ::testing::Test {};  // Fixture: bar::CoolTest
TEST_F(CoolTest, DoSomething) {
  SUCCEED();
}
}  // namespace bar
```
