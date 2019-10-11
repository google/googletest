## Legacy gMock FAQ

### Mock object의 method를 하나 호출했는데 real object의 method가 호출되어 버렸습니다. 왜 그런건가요? ###

Class method를 mocking하기 위해서는 기본적으로 *virtual*로 선언해야 합니다. 그게 어려운 경우라면 [high-perf dependency injection technique](cook_book.md#non-virtual-method-mocking-하기)을 사용해도 됩니다.

### 가변함수(variadic function)도 mocking 할 수 있나요? ###

흔히 (`...`) 형태의 argument로 대변되는 variadic function을 gMock에서 직접 mocking할 수는 없습니다. 

1차적인 문제는 googletest는 variadic function에 몇 개의 argument가 사용될지 *알 수 없다는 것입니다.* 물론 argument type도 마찬가지로 알 수가 없습니다. 이것은 *base class를 직접 구현하는 사람만이 알고 있을 것입니다.*

이런 variadic function을 mocking하려면 mock object에게 몇 개의 argument가 어떤 타입으로 전달될지를 알려줘야만 합니다. 이를 위한 한 가지 방법은 overloaded function을 제공하는 것입니다.

사실(`...`) 형태의 argument는 C언어의 기능이며 C++에서 새로 생겨난 것은 아닙니다. 그렇기 때문에 C++의 기능들과 함께 사용할 때 100% 안전하지는 않습니다. 되도록이면 variadic function 사용은 지양하기를 권장합니다.

### MSVC에서 const parameter를 전달받는 mock method를 compile하면 C4301, C4373과 같은 warning이 발생합니다. 왜 그런건가요?

만약, Microsoft Visual C++ 2005 SP1을 통해 아래 코드를 compile한다면,

```c++
class Foo {
  ...
  virtual void Bar(const int i) = 0;
};

class MockFoo : public Foo {
  ...
  MOCK_METHOD(void, Bar, (const int i), (override));
};
```

아래와 같은 warning이 발생할 것입니다. 그러나 이것은 MSVC의 bug입니다. 동일한 코드가 gcc에서는 문제없이 잘 compile될 것입니다.

```bash
warning C4301: 'MockFoo::Bar': overriding virtual function only differs from 'Foo::Bar' by const/volatile qualifier
```

다음으로 Visual C++ 2008 SP1을 사용한다면 아래와 같은 warning이 발생할 것입니다.

```bash
warning C4373: 'MockFoo::Bar': virtual function overrides 'Foo::Bar', previous versions of the compiler did not override when parameters only differed by const/volatile qualifiers
```

C++의 value parameter에 `const`를 사용하면 `const`는 무시됩니다. 즉, compiler가 `Bar(int)`와 `Bar(const int)`를 동일하게 취급하기 때문에 처음에 구현했던 코드는 사실  같다고 봐야합니다.

```c++
class Foo {
  ...
  virtual void Bar(int i) = 0;  // int or const int?  Makes no difference.
};
```

이렇게 method parameter에 `const`를 사용하는 것이 의미가 없기 때문에 MSVC에서 warning을 보기 싫다면 `Foo`와 `MockFoo`에서 `const`를 아예 제거하는 것도 괜찮습니다.

단, 현재 얘기하고 있는 `const`는 *top-level* `const`만 해당한다는 것을 주의하기 바랍니다. (쉽게 생각해서 value type 변수만) 만약, method parameter가 pointer나 reference라면 `const`는 여전히 의미가 있습니다. 예를 들어 아래코드의 `Bar()` 2개는 서로 다른 의미를 가집니다.

```c++
void Bar(int* p);         // Neither p nor *p is const.
void Bar(const int* p);  // p is not const, but *p is.
```

### Expectation을 만족하지 못해서 테스트가 실패했는데, 뭐가 문제인지 잘 모르겠습니다. 어떻게 해야 할까요?

`--gmock_verbose=info`라는 flag와 함께 테스트를 수행하면, mock function의 trace까지 포함해서 최대한 많은 정보를 출력해 줍니다. 이러한 trace를 참고하면 expectation이 만족되지 않은 이유를 밝히는데 도움이 될 것입니다.

혹시, "The mock function has no default action set, and its return type has no default value set."이라는 message가 출력되었다면 [adding a default action](cheat_sheet.md#default-action-설정하기)을 적용해보기 바랍니다. 내부적인 이슈로 인해서 default action이 없는 상태에서 발생한 unexpected call에 대해서는 자세한 정보(actual argument와 expected argument 비교 등)를 출력하지 않고 있으며 위와 같은 message만 출력하고 있습니다.

### Program이 crash가 발생한 후에, `ScopedMockLog`가 너무 많은 내용을 출력합니다. 혹시 gMock의 bug가 아닌가요?

gMock과 `ScopedMockLog`는 정상적으로 동작한 것으로 보입니다.

테스트가 실패하면 failure signal handler는 최대한 많은 정보를 수집하려고 합니다. (stack trace, address map 등을 포함해서) 복잡한 환경일수록 정보도 많습니다. 예를 들어 여러 thread와 그들의 stack으로부터도 정보가 계속 수집됩니다. 그러다가 문제가 발생하는 시점에 `ScopeMockLog`이 관련 정보를 출력해 주는 것입니다.

물론, 이러한 error를 출력하지 않는 방법도 제공을 하고 있습니다. 아니면 아래 코드와 같이 관심대상이 아닌 내용을 구분하도록 구현해도 됩니다.

```c++
using ::testing::AnyNumber;
using ::testing::Not;
...
  // Ignores any log not done by us.
  EXPECT_CALL(log, Log(_, Not(EndsWith("/my_file.cc")), _))
      .Times(AnyNumber());
```

### Mock function이 호출되지 않기를 바란다면 어떻게 구현해야 하나요?

아래처럼 구현하면 됩니다.

```c++
using ::testing::_;
...
  EXPECT_CALL(foo, Bar(_))
      .Times(0);
```


### 테스트가 실패했는데 gMock이 어떤 expectation에 대해 같은 내용을 두 번 출력했습니다. 중복으로 출력된 건가요? ###

gMock은 failure를 발견했을 때, 사용자를 위해 다양한 디버깅 정보들을 출력해줍니다. 예를 들면 mock function으로 전달된 argument 나 expectation의 상태 등을 알려줍니다. 이것은 failure가 발견될 때마다 같은 방식으로 동작합니다.

만약, 2개의 expectation이 있고 기대하는 것들이 같다고 가정해봅시다. 그러한 2개의 expectation을 테스트하면서 기대하는 바를 충족하지 못한다면 당연히 2개의 failure가 발생하고 출력되는 정보도 동일하게 됩니다. 이런 경우는 *중복이 아니라 서로 다른 시점에 동일한 문제가 발견 된 것입니다.* 비록 그 내용이 같을지라도 필요한 정보를 출력했다고 봐야합니다.

### Real object를 사용하면 괜찮은데 Mock object를 사용하면 heap check failure가 발생합니다. 뭐가 잘못된 걸까요? ###

지금 mocking하고 있는 class(pure interface이면 좋습니다.)가 virtual destructor를 가지고 있나요?

C++에서 base class의 destructor는 언제나 virtual로 선언되어야 합니다. 그렇지 않으면 derived class의 destructor들이 호출되지 않기 때문에 문제가 됩니다. 아래 코드를 보겠습니다.

```cpp
class Base {
 public:
  // Not virtual, but should be.
  ~Base() { ... }
  ...
};

class Derived : public Base {
 public:
  ...
 private:
  std::string value_;
};

...
  Base* p = new Derived;
  ...
  delete p;  // Surprise! ~Base() will be called, but ~Derived() will not
                 // - value_ is leaked.
```

주석에 설명한 것처럼 위 코드에서는 `delete p`를 해도 `Derived` class의 destructor가 호출되지 않습니다. 해결방법은 `~Base()`를 `virtual ~Base()`로 변경하는 것입니다. 그러면 heap checker의 결과도 문제가 없을 것입니다.

### "newer expectations override older ones"라는 규칙이 이상해 보입니다. gMock은 왜 이런 정책을 선택한건가요? ###

아래 예제를 먼저 보겠습니다.

```cpp
using ::testing::Return;
...
  // foo.Bar() should be called twice, return 1 the first time, and return
  // 2 the second time.  However, I have to write the expectations in the
  // reverse order.  This sucks big time!!!
  EXPECT_CALL(foo, Bar())
      .WillOnce(Return(2))
      .RetiresOnSaturation();
  EXPECT_CALL(foo, Bar())
      .WillOnce(Return(1))
      .RetiresOnSaturation();
```

위의 코드를 구현하면서 (주석에 쓰여있는 것과 같은) 불평을 하고 있다면 과연 적절한 방법으로 구현한 것인지 고민해 볼 필요가 있습니다.

gMock의 기본적으로 expectation간에 *어떠한 호출순서도 부여하지 않습니다.* (탐색순서만 있습니다.) 이것은 gMock 그리고 jMock의 기본철학입니다. 이러한 규칙은 사용자가 테스트를 너무 과하게 지정하는 실수를 줄이기 위한 것입니다. 만약, expectation간에 호출순서를 부여하고 싶다면 사용자가 직접 명시적으로 표현해야 합니다.

호출순서를 부여하기 위해서는 expectation들을 sequence에 포함시키면 됩니다.

```cpp
using ::testing::Return;
...
  // foo.Bar() should be called twice, return 1 the first time, and return
  // 2 the second time.  Using a sequence, we can write the expectations
  // in their natural order.
  {
    InSequence s;
    EXPECT_CALL(foo, Bar())
        .WillOnce(Return(1))
        .RetiresOnSaturation();
    EXPECT_CALL(foo, Bar())
        .WillOnce(Return(2))
        .RetiresOnSaturation();
  }
```

아니면 expectation에 여러개의 action을 지정해도 됩니다. action의 호출순서는 위에서 아래입니다.

```cpp
using ::testing::Return;
...
  // foo.Bar() should be called twice, return 1 the first time, and return
  // 2 the second time.
  EXPECT_CALL(foo, Bar())
      .WillOnce(Return(1))
      .WillOnce(Return(2))
      .RetiresOnSaturation();
```

그럼 왜 gMock은 expectation들의 호출순서는 강제하지 않으면서 탐색할 때는 밑에서부터 탐색할까요? 왜냐하면 이렇게 해야 mock의 동작을 일반적인 내용부터 시작해서 상세한 동작으로 점진적으로 지정할 수 있기 때문입니다. 즉, mock의 set-up 시점에 일반적인 expectation을 지정하고, 각각의 test case에서는 좀 더 상세한 expectation을 지정하는 것입니다. 만약, gMock이 위에서부터 아래 방향으로 expectation을 검색하면 위와 같은 패턴을 적용할 수 없게 됩니다.

### ON_CALL을 통해 default action을 지정했음에도 불구하고 EXPECT_CALL을 설정하지 않은 function이 호출되면 warning을 출력합니다. 이런 경우에 warning을 출력하는게 맞나요? ###

간편함과 안전함 중에 선택하라고 할 때, gMock은 후자를 선택합니다. 그러한 이유로 여기서도 warning을 출력해주는게 더 좋다고 생각합니다.

사람들은 mock object의 constructor나 `SetUp()`에 `ON_CALL`을 구현합니다. 왜냐하면 어떤 mock method의 default behavior를 지정할 때는 여러 test case에 사용될 수 있고 잘 변하지 않는 일반적인 내용을 주로 지정하기 때문입니다. 그 다음에 각각의 test case를 구현할 때 해당 test case에 특화된 expectation을 지정할 것입니다. 즉, set-up 부분에서 지정하는 default behavior는 말 그대로 기본동작만 지정한 것이지 실제로 호출되길 바라는 expectation은 아닌 것입니다. 따라서 개별 test case에 `EXPECT_CALL`이 없는데도 mock method가 호출되었다면 문제가 있다고 판단하게 됩니다. 또 이러한 내용을 사용자에게 알려줘야 잠재적인 bug를 예방하는데 유리합니다. 다만, 실제로 bug일지 아닐지는 모르기 때문에 error가 아닌 warning을 통해 알려주는 것입니다.

예제를 통해 확인해보면, 아래처럼 `ON_CALL`을 통해서 default behavior를 변경했다고 해도 warning은 계속해서 출력됩니다.

```cpp
using ::testing::_;
...
  ON_CALL(foo, Bar(_))
      .WillByDefault(...);
```

발생한 호출이 문제가 없어서 warning이 불필요하다면 아래처럼 구현하기만 하면 됩니다.

```cpp
using ::testing::_;
...
  EXPECT_CALL(foo, Bar(_))
      .WillRepeatedly(...);
```

이제 gMock은 해당 method가 호출되어도 warning을 출력하지 않을 것입니다.

더불어 gMock이 출력하는 내용을 조절하는 것도 가능합니다. 디버깅을 해야하는데 너무 많은 정보가 출력되어 어려움을 겪고 있다면 `--gmock_verbose` flag의 레벨을 조절하시기 바랍니다.

### Action에서 mock function으로 전달되는 argument를 삭제(delete)하려면 어떻게 해야 하나요? ###

Mock function이 전달받는 pointer argument를 삭제하려고 하는 건가요? 그런 경우라면 `testing::DeleteArg()`를 사용해서 N번째(0부터 시작) argument를 삭제할 수 잇습니다.

```c++
using ::testing::_;
  ...
  MOCK_METHOD(void, Bar, (X* x, const Y& y));
  ...
  EXPECT_CALL(mock_foo_, Bar(_, _))
      .WillOnce(testing::DeleteArg<0>()));
```

### 새로운 action을 만들 수 있나요?

gMock에서 지원하지 않는 새로운 action을 구현하고 싶다면 [MakeAction()](cook_book.md#새로운-monomorphic-action-구현하기), [MakePolymorphicAction()](cook_book.md#새로운-polymorphic-action-구현하기)를 사용하면 됩니다. 또한 stub function을 구현하고 [Invoke()](cook_book.md#functionsfunctorslambda를-action으로-사용하기)를 사용해서 호출하는 것도 가능합니다.

```c++
using ::testing::_;
using ::testing::Invoke;
  ...
  MOCK_METHOD(void, Bar, (X* p));
  ...
  EXPECT_CALL(mock_foo_, Bar(_))
      .WillOnce(Invoke(MyAction(...)));
```

### static/global function도 mocking 가능한가요? ###

물론 가능합니다, 그 전에 약간의 확인 및 수정작업은 필요합니다.

먼저, static/global function에 대한 mocking이 필요하다는 것은 소스코드가 너무 tightly coupled (혹은 less flexible, less resuable, less testable)되어 있음을 의미하기도 합니다. 이런 경우에는 작은 interface를 하나 정의하고 해당 interface를 통해서 static/global function을 호출하도록 변경하는 것이 좋습니다. 이러한 구조가 되면 mocking을 하기에도 유리해집니다. 물론 초반에 해야할 일이 좀 더 생기겠지만, 나중에는 도움이 될 것입니다.

관련 내용들이 [여기](https://testing.googleblog.com/2008/06/defeat-static-cling.html)에 매우 잘 설명되어 있습니다.

### Mock object를 통해 여러가지 복잡한 일을 해보고 싶습니다. 그러나 gMock에서 action을 명세하는 작업이 너무 힘듭니다. ###

엄밀히 말해서 질문이라고 보기는 어렵지만, 최대한 답을 드려보겠습니다.

gMock이라는 도구를 통해 C++에서도 간단하게 mock을 사용할 수 있게 되었지만 여전히 gMock조차도 어려워하는 사람들이 많이 있습니다. 무엇이 문제일까요?

먼저, mock이 없이 테스트코드를 구현한다고 가정해 봅시다. 이 때에는 어떤 변수나 시스템의 상태(state)를 검증하게 됩니다. 여기서 상태는 어떤 동작이 완료된 후에 변화된 결과를 의미하며 이렇게 상태를 기반으로 검증하는 방법을 "state-based testing"이라고 부르기도 합니다.

반면에 mock을 사용하면 "interaction-based testing"이라는 방법을 사용하게 됩니다. 즉, 시스템의 상태를 검증하는 것이 아니라 상태를 변화시키기 위한 일련의 동작들이 원하는 방향으로 진행되고 있는지 확인하고 그 결과를 바로 알려줍니다. 그 과정에서 발생한 문제가 있다면 해당 상황에 대한 정보도 제공해줍니다. 많은 경우에 "interaction-based testing" 접근방법이 "state-based testing"보다 효과적이고 경제적으로 여겨집니다.

물론, mock이 아닌 다른 test double(fake 등) + "state-based testing"으로도 충분하고 그렇게 하는 것이 적절한 상황이라면 당연히 그렇게 구현해야 합니다. Mock을 적용하는 것도 노력이 필요한 일이기 때문에 꼭 필요한 곳에만 적용하기 바랍니다. 즉, 사용자의 환경에 mock을 적용하기가 어렵다면 그 상황에 더 알맞은 다른 도구를 검토해 볼 필요도 있다는 것입니다. 아니면 gMock을 사용하면서 어려웠던 부분을 저희와 함께 개선해보면 어떨까요?

### "Uninteresting function call encountered - default action taken.." 이라는 warning이 발생했습니다. 큰 문제인가요? ###

당연히 아닙니다, 단순히 정보를 전달하기 위한 목적입니다.

해당 warning은 어떤 mock function에 아무런 expectation을 지정하지 않았는데도 호출되었음을 의미합니다. 이러한 경우에 gMock은 expectation이 없기 때문에 몇 번 호출되든 상관없다고 판단합니다. "호출되면 안 된다"라고 지정한 것이 아니기 때문에 warning 외에 다른 문제는 없을 것입니다.

gMock은 warning을 통해서 잠재적인 문제에 대한 정보를 사용자에게 알려줍니다. 즉, warning이 발생했다면 관련 내용을 검토해 볼 필요가 있습니다. 예를 들어 원래는 "호출되면 안 된다"라고 하고 싶었는데 깜박했을 수도 있습니다. 즉, `EXPECT_CALL(foo, Bar()).Times(0)`이라는 구현을 빼먹은 경우입니다.

이제 질문과 같은 warning이 출력되면 이것이 실제로 문제가 될지 안될지를 판단하기를 바랍니다. 사용자가 실수를 했을지도 모른다는 가정으로 알려주는 정보인 것입니다. Function name, argument들도 출력해주기 때문에 상당한 도움이 될 것입니다.

### Action을 직접 정의하고 싶습니다. Invoke()를 사용하는게 좋은가요 아니면 action interface를 구현하는게 좋은가요? ###

어느 것을 사용해도 괜찮습니다. 상황에 맞는 것을 고르면 됩니다.

일반적으로 action이 특정타입에 대해서만 수행된다면 `Invoke()`를 사용해서 구현하는게 쉽습니다. 반대로 `Return(value)`와 같이 여러가지 타입에 두루두루 사용할 수 있는 action을 원한다면 `MakePolymorphicAction()`를 사용하는 것이 좋습니다. 후자의 경우에는 `ActionInteface`를 통해서 구조를 좀 더 간결하게 할 수도 있습니다. 관련 예제로 `include/gmock/gmck-actions.h` 파일의 `Return()` 구현부를 보면 도움이 될 것입니다.

### WillOnce()와 함께 SetArgPointee()를 사용하는데 "conflicting return type specified"라는 compile error가 발생했습니다. 무슨 뜻인가요? ###

Mock method가 호출되었지만 어떤 값을 반환해야 할지 모를 때에 위와 같은 에러가 발생합니다. `SetArgPointee()`는 argument에 값을 저장하는 동작만 수행하는 action입니다. 이런 문제가 발생하는 이유는 mock method에 대한 `Return()` 동작을 지정하지 않았기 때문입니다. `DoAll()`을 사용해서 `SetArgPointee()`와 `Return()`을 둘 다 수행할 수 있도록 연결해주시기 바랍니다.

[여기](cook_book.md#action-조합하기)의 예제를 참조하기 바랍니다.

### Microsoft Visual C++에서 out out memory라고 하며 compile에 실패합니다.  어떻게 해야하나요? ###

Visual C++ 을 사용할 때, `/clr`이라는 compiler flag를 확인해보기 바랍니다. `/clr`이 사용되면 mock class를 compile하기 위해 기존보다 5~6배 많은 메모리를 사용하게 됩니다. 해당 flag를 제거하고 다시 시도해보기 바랍니다.
