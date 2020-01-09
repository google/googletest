## gMock for Dummies

### gMock이란 무엇인가?

프로토타입 혹은 테스트 프로그램을 구현할 때, 처음부터 관련된 모든 소프트웨어를 구현하고 실행하는 것은 사실상 불가능하다. 이러한 경우, 아직 구현하지 못한 소프트웨어(real object)들을 대체하기 위해서 그와 동일한 interface를 갖는 mock object를 만들어서 적용한다면 runtime에 마치 real object가 있는 것처럼 동작시킬 수 있다. 예를 들어서 "어떤 function을 호출할지, 그러한 function은 어떤 argument를 통해 몇 번 호출되어야 하는지, return value는 어떻게 할지, function이 여러개라면 그들간의 호출순서는 어떠한지" 등을 사용자가 자유롭게 지정할 수 있게 된다.

**Note**: Mock object라는 용어와 *fake object*라는 용어는 혼동하기 쉽다. 그러나 TDD(Test-Driven Development) 관점에서 보면 fake와 mock의 의미는 매우 다르다.

- **Fake** object는 동작에 필요한 실제 구현부가 있고 그 결과도 real object와 동일해야 한다. 다만, real object보다 간단하거나 빠르게 동작할 수 있도록 구현해야 한다. 대표적인 예로는 데이터베이스 혹은 파일시스템 처리를 메모리에서 수행하도록 구현하는 것이 있다.
- **Mock** object는 *expectation*을 통해 수행할 동작을 지정할 수 있다. 여기서 expectation이란 mock object(엄밀히는 mock object의 method)가 어떤 일을 하고 또 어떤 결과를 만들어내야 하는지에 대해서 기대하고 있는 바를 미리 지정하는 것이다.

지금 시점에서 위의 개념이 추상적이고 구별이 안될 수도 있지만 너무 걱정할 필요는 없다. 일단은 mock이 테스트 대상코드와 mock 간의 상호작용(interaction) 검증이 가능하도록 도와준다는 것만 기억하면 된다. 앞으로 mock을 사용하다 보면 fake와 mock의 차이를 좀 더 명확히 알게 될 것이다.

**gMock**이란 mock class를 만들고 사용하기 위한 function, macro 등을 제공하는 library 이다. (때때로 "framework" 이라고 부르기도 한다.) 이를 통해 Java의 jMock, EasyMock에서 제공하는 다양한 기능을 C++에서도 사용할 수 있게 된다.

gMock을 사용할 때는 기본적으로 아래 과정을 거치게 된다.

- 먼저, gMock에서 제공하는 macro를 이용해서 테스트대상(interface)을 mocking하고 해당 mock class에 필요한 내용을 추가한다.
- 다음으로 mock object를 만들어서 expectation, behavior를 지정한다. 이를 위한 매우 직관적인 문법의 기능들을 제공하고 있다.
- 이제 mock object를 사용하는 코드를 실행하면 gMock이 위에서 지정한 expectation중에 만족하지 않는 부분을 찾아내서 알려줄 것이다.

### 왜 gMock 인가?

상술한 것처럼 mock object는 소프트웨어 간에 존재하는 의존성을 제거해주기 때문에 개발자가 테스트 프로그램 또는 프로토타입 소프트웨어를 개발할 때 더 빠르고 신뢰성있게 구현할 수 있도록 도와준다. gMock을 사용하지 않고서 mock object를 직접 구현하는 것은 *상당히 어려운데* 그 이유는 아래와 같다.

- 먼저 mock도 소프트웨어이므로 누군가는 그것을 구현해야 한다. 그런데 그러한 작업이 힘들고 시간이 많이 소모되는 작업이었기 때문에 C++ 개발자들이 mock을 멀리하는 주된 원인이 되었다.
- Mock을 구현하는데 성공했다고 하더라도 동작이 완벽하지 않을 수 있다. 물론 많은 시간을 투자하면 쓸만한 mock을 만들수도 있지만 긴 시간이 필요한 작업은 효율성 측면에서 좋지 않다.
- Mock을 개발함에 있어서 뚜렷한 규칙이 없다면 새로운 mock을 만들때마다 이전에 고생했던 만큼의 노력이 필요하게 된다.

Java, Python 같은 언어에서는 이미 mock을 자동으로 생성가능한 효율적인 mock framework([jMock](http://www.jmock.org/), [EasyMock](http://www.easymock.org/), [Mox](https://code.google.com/archive/p/pymox/wikis/MoxDocumentation.wiki))을 제공하고 있다. 그 결과 mocking이 효율적인 기술이라는 것이 증명되었으며 여러 커뮤니티를 통해서 관련 기술이 발전될 수 있었다. 그러나 C++ 에서는 mock을 위한 좋은 도구가 없었기 때문에 점점 격차가 벌어졌다.

gMock은 이렇게 어려움을 겪고 있는 C++ 프로그래머들을 돕기 위해서 만들어졌다. jMock, EasyMock으로부터 많은 아이디어를 가져온 것은 맞지만 거기에 더해 C++의 특징을 잘 살려서 설계되었다. C++을 사용하면서 아래와 같은 문제들로 인해 어려움을 겪고 있다면 이제부터는 gMock이 좋은 친구가 되어줄 것이다.

- 설계가 문제없는지 프로토타입을 통해 확인하고 싶지만 프로토타입을 "빠르게" 만드는 것이 어려울 때
- 너무 많은 리소스(라이브러리, 데이터베이스 등)를 사용함으로 인해서 테스트의 속도가 느린 경우
- 네트워크와 같이 실패가능성이 있는 리소스를 사용함으로 인해서 테스트도 뜻하지 않게 실패하는 일이 발생할 떄
- 오류처리에 관한 테스트를 하고 싶지만, 해당 오류가 발생하는 빈도자체가 낮아서 테스트가 어려울 때 (예: file checksum 오류 등)
- 모듈간에 데이터를 주고받는 내부과정을 들여다보고 싶은데 잘 안되서 최종적인 수행결과만 확인하고 있는 경우
- 의존성 제거를 위해서 mock을 사용하고 싶지만, mock을 직접 구현하기가 싫은 경우

마지막으로 gMock을 아래와 같은 용도로 사용하기를 권장한다.

- *설계도구* : 설계에 문제가 없는지 빠르게 확인하고 문제가 있다면 개선할 수 있다.
- *테스트도구* : 외부와 연결되는 의존성을 제거하고 모듈간의 상호작용을 검증할 수 있다.

### 시작하기

gMock은 googletest와 함께 제공되는 번들소프트웨어이다.

### 예시: Mock Turtles

먼저 우리가 어떤 그래픽 관련 프로그램을 개발하고 있다고 가정해보자. 또한, 렌더링 API로는 [LOGO](https://en.wikipedia.org/wiki/Logo_(programming_language))라는 것을 사용하고 있다고 한다면 과연 이 프로그램은 어떻게 테스트하면 좋을까? 가장 쉬운 방법은 역시 해당 API를 사용해서 그림을 그린 후, 그 결과를 golden image와 비교하는 것이다.(golden image = "OK"라고 판단할 수 있는 비교대상) 그러나 이러한 테스트는 시간이 많이 걸리고 깨지기 쉬운 테스트 방법으로 여겨진다. 만약에 그래픽카드를 변경했더니 anti-aliasing 동작이 바뀌어 버렸다면 golden image를 교체해야만 한다. 왜냐하면 그려진 그림의 결과가 그래픽카드에 종속되어 있기 때문이다. 테스트가 많으면 많을수록 golden image를 변경하는 작업은 매우 힘들고 귀찮은 일이 될 것이다. 이제 mock을 사용하면 이러한 문제를 해결할 수 있다. 본격적으로 시작하기에 앞서 의존성 주입([Dependency Injection](https://en.wikipedia.org/wiki/Dependency_injection)) 패턴을 어느정도 알고 있다면 도움이 될 것이다. 물론 완벽히는 몰라도 괜찮다. 앞으로 나오는 내용들을 따라가다 보면 그 의미를 자연스럽게 알게 될 것이다. 그럼 `Turtle`이라는 interface를 정의하고 본격적으로 시작해보자.

```cpp
class Turtle {
  ...
	      virtual ~Turtle() {};
	      virtual void PenUp() = 0;
	      virtual void PenDown() = 0;
	      virtual void Forward(int distance) = 0;
	      virtual void Turn(int degrees) = 0;
	      virtual void GoTo(int x, int y) = 0;
	      virtual int GetX() const = 0;
	      virtual int GetY() const = 0;
	    };
```

(`Turtle`의 destructor는 virtual function으로 선언해야만 한다. 왜냐하면 C++에서 base class의 destructor가 `virtual`로 선언되어야만 derived class들의 destructor를 자동으로 호출해주기 때문이다. 그렇지 않으면 당연히 memory leak과 같은 문제가 발생할 수도 있다.)

`PenUp()`, `PenDown()`은 turtle(거북이)이 움직일 때에 흔적을 남길지 말지를 결정한다. `PenDown()`이 호출되면 곧 "펜"을 쓴다는 의미이므로 흔적을 남기게 된다. `Forward()`, `Turn()`, `GoTo()`는 turtle이 어느 방향으로 이동할지를 지정한다. 마지막으로 `GetX()`, `GetY()`는 turtle의 현재 위치를 알려준다.

이렇게 정의된 interface를 real class와 mock class로 나눠서 구현하게 된다. 이 때, real class를 구현하는 것은 실제로 많은 시간과 노력이 필요할 수도 있기 때문에 mock class를 먼저 구현함으로써 해당 interface의 동작을 미리 그리고 빠르게 검증해 볼 수 있게 된다. 어떤 argument를 가지고 어떤 function을 호출할 것인가, 또 function 간의 호출순서는 어떠한가 등을 검증할 수 있다. 이를 통해서 쉽고 빠르게 원하는 바를 확인할 수 있으며 그러면서도 여전히 강력하고 유지보수 측면에서도 더 좋은 코드를 구현할 수 있다. 또한, 위에서 언급한 그래픽카드 변경에 따른 golden image 교체작업도 할 필요가 없어진다. 왜냐하면 mock class를 사용한 검증에서는 어떤 코드의 수행결과나 바이너리를 확인하는 것이 아니며 그 수행과정 자체를 코드레벨에서 확인하기 때문이다. 마지막으로 역시 제일 중요한 것은 mock class를 사용하면 이러한 일련의 작업이 *매우 매우 빠르다는 것이다.*

### Mock Class 작성하기

물론 mock이 이미 구현되어 있다면 좋겠지만 그렇지 않은 경우에는 직접 만들어야만 한다. gMock을 사용하면 이러한 작업도 (대부분의 경우에) 매우 간단하게 진행할 수 있을 것이다.

#### 정의하는 방법

`Turtle` interface를 계속 사용한다. 기본적으로 mock class를 정의하기 위해서는 아래와 같은 단계를 거치게 된다.

- `Turtle` interface를 상속받는 `MockTurtle` class를 생성한다.
- `Turtle` interface에 정의한 *virtual* function의 이름과 해당 function이 몇 개의 argument를 가지고 있는지 확인한다. (상속 대신 템플릿을 이용하는 것도 가능하지만 상속보다는 약간 어렵다. 관련내용은 [여기](cook_book.md#nonvirtual-method-mocking-하기)에서 확인 가능하다.)
- Derived class의 `public:` 영역에 `MOCK_METHOD();` macro를 사용한다.
- 다음으로는 `Turtle` interface의 function signature에서 *function name*, *return type*, *argument*들을 복사해서 `MOCK_METHOD();`에 그대로 붙여 넣는다. (아래 예제코드 참조)
- 만약, const method를 mocking하고 있다면 4번째 parameter에 `(const)`를 추가한다.
- Virtual method를 overriding하고 있는 것이므로 `override` 키워드를 사용하기를 추천한다. 그렇게 되면 4번째 parameter는 `(override)` 혹은 `(const, override)`가 될 것이다.
- 위의 내용을 `Turtle` interface의 다른 모든 virtual function에 대해서도 수행한다. C++에서 interface(abstract class)를 상속받는 derived class는 interface의 pure virtual method를 전부 override해야만 object를 생성할 수 있기 때문이다.

위의 과정을 거치면 최종적으로 아래와 같은 모양이 된다.

```cpp
#include "gmock/gmock.h"  // Brings in gMock.

class MockTurtle : public Turtle {
 public:
  ...
  MOCK_METHOD(void, PenUp, (), (override));
  MOCK_METHOD(void, PenDown, (), (override));
  MOCK_METHOD(void, Forward, (int distance), (override));
  MOCK_METHOD(void, Turn, (int degrees), (override));
  MOCK_METHOD(void, GoTo, (int x, int y), (override));
  MOCK_METHOD(int, GetX, (), (const, override));
  MOCK_METHOD(int, GetY, (), (const, override));
};
```

위와 같이 했다면 다른 추가작업은 필요 없다. 나머지는 `MOCK_METHOD` macro가 알아서 해주게 된다. 이렇듯 간단하게 mock class와 mock function을 정의할 수 있으며 조금만 익숙해지면 매우 빠르게 mock class를 생성할 수 있을 것이다.

#### 어떤 파일에 저장해야 할까?

Mock class를 정의할 때, 해당 class를 실제로 어디에 저장해야 할지도 고민해 볼 부분이다. 예를 들어 `Foo`라는 interface가 있다면 이것을 사용하는 모든 개발자가 본인만의 공간에 본인만의 테스트파일(예: `*_test.cc`)을 만들고 `MockFoo` class를 구현할 수 있다. 물론, 이렇게 해도 동작상에 문제는 없지만 유지보수 측면에서는 어려움이 발생할 것이다. 왜냐하면 `Foo` interface가 고쳐졌다고 가정하면, 독자적으로 이것을 상속받아 사용하던 모든 테스트가 실패하거나 영향을 받기 때문이다.

따라서 되도록이면 interface와 mock class는 같은 패키지에 있는 것이 좋다. (이 경우에 production 빌드와 test 빌드를 확실히 구분해서 둘이 섞이지 않도록 주의해야 한다.) 즉, `Foo`가 포함된 패키지안에 `mock_foo.h`라는 파일을 만들고 `MockFoo`를 구현하면 된다. 그리고 모든 개발자가 `mock_foo.h`를 사용하게 한다. 이렇게 되면 `Foo`가 변경되더라도 1개의 `MockFoo`만 수정하면 되기 때문에 유지보수 측면에서 도움이 된다.

또 다른 방법은 `Foo` interface보다 더 상위에 `FooAdaptor`라는 새로운 interface를 만드는 것이다. 이러한 패턴은 외부로 노출되는 `FooAdaptor`는 쉽게 변하지 않도록 유지하면서도 내부에서는 자유롭게 변경작업이 진행될 수 있도록 도와준다. 물론 초반에 해야할 일이 많아지는 것은 사실이지만 코드의 가독성을 높이고 구현난이도를 낮춰주는 장점이 있기 때문에 장기적으로 볼 때는 좋은 방법이라고 할 수 있다.

### 테스트에서 Mock 이용하기

Mock class를 생성한 이후에는 아래와 같은 방법으로 진행한다.

1. gMock 관련 기능을 사용하기 위해서 `testing` namespace를 포함시킨다. (한 파일에 한 번씩만)
2. Mock object를 생성한다.
3. Mock object에 expectation을 지정한다. (어떤 method를 호출할지, 어떤 argument를 전달받을지, 어떤 동작을 할지 등)
4. Mock object를 사용하는 실제코드를 수행한다. 이 때 expectation으로 지정된 횟수 및 argument 정보와 다르게 호출되는 부분이 있다면 즉시 에러로 검출된다. (결과확인을 위해 googletest assertion을 추가로 사용하는 것도 좋다.)
5. Mock object가 소멸될 때, gMock은 설정한 *모든* expectation들이 만족되었는지 확인해준다.

4단계는 각각의 expectation이 호출될 때마다 잘못된 부분을 검출해서 알려주는 것이고 5단계는 해당 mock object에 지정했던 모든 expectation들을 다시 한 번 확인해주는 것이기 때문에 다른 항목이다. 출력되는 내용도 조금 다르며 디버깅에 필요한 정보는 일반적으로 4단계에서 더 많이 출력된다.

아래 예제코드를 보자.

```c++
#include "path/to/mock-turtle.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::AtLeast;                         // #1

TEST(PainterTest, CanDrawSomething) {
  MockTurtle turtle;                              // #2
  EXPECT_CALL(turtle, PenDown())                  // #3
      .Times(AtLeast(1));

  Painter painter(&turtle);                       // #4

  EXPECT_TRUE(painter.DrawCircle(0, 0, 10));      // #5
}
```

위 코드를 간단히 해석해보면 보면, `PenDown()`이 적어도(AtLeast) 한 번 이상 호출되도록 expectation이 설정되었다. 따라서 `painter` object의 method인 `DrawCircle()`이 내부에서 `PenDown()`을 호출하지 않는다면 아래와 같은 에러가 발생할 것이다.

```bash
path/to/my_test.cc:119: Failure
Actual function call count doesn't match this expectation:
Actually: never called;
Expected: called at least once.
Stack trace:
...
```

**Tip 1:** Emacs buffer에서 테스트 프로그램을 실행하는 사람은 line number에서 바로 실패한 부분(expectation)으로 이동할 수 있다.

**Tip 2:** Mock object가 소멸되지 않았다면 최종결과도 출력되지 않는다. 따라서 mock을 지역변수가 아닌 heap에 할당하는 경우에는 heap leak checker 등을 사용해서 누수되는게 없는지 확인해볼 필요가 있다. `gtest_main`이라는 library를 사용하면 heap leak checker 기능이 자동으로 포함된다.

**Important note:** 당연하겠지만 expectation은 mock function이 호출되기 **전에** 설정되어야 한다. 그렇지 않은 경우의 동작은 **미정의**이기 때문에 문제가 발생할 것이다. 그리고 `EXPECT_CALL()`을 여러개 사용할 때는 `EXPECT_CALL()`들의 사이에 mock function을 끼워넣으면 안 된다. `EXPECT_CALL()`들을 먼저 지정하고 mock function들을 호출해야 한다.

`EXPECT_CALL()`은 *미래에 일어날 함수호출을* 미리 지정함을 의미한다. 왜 gMock은 이렇게 동작할까? 왜냐하면 expectation은 말 그대로 기대하는 바를 알려주는 것이기 때문이다. 또한, 이렇게 미리 지정해야만 잘못된 동작으로 인해 실행환경(stack 등)이 깨지기 전에 문제를 잡아내고 사용자에게 알려줄 수 있다. Stack이 이미 깨진 상태에서 제공되는 정보는 디버깅에 도움이 되지 않을 것이다.

위 코드는 조금 단순한 예제이긴 했는데, 계속해서 gMock이 제공하는 보다 다양한 기능들을 확인해보자.

### Expectation 설정하기

"mock object를 잘 쓰고 있다"라고 말하려면 역시 *expectation을 제대로* 사용해야 한다. 이러한 expectation을 사용하는 것도 쉽지는 않다. 먼저 expectation을 너무 엄격하게 지정하면 테스트가 소소한 변화에도 영향을 받아서 실패하게 되며 반대로 너무 느슨하게 지정하면 잡아내야 할 bug를 검출하지 못 하게 된다. 누구나 테스트 프로그램이 bug를 정확히 잡아내서 목적한 바를 이루길 원하며 gMock은 그러한 목적달성을 위해 최적의 기능들을 제공하고 있다.

#### 일반적인 문법

gMock에서는 `EXPECT_CALL()` macro를 사용하여 mock method에 대한 expectation을 지정한다. 기본적인 사용법은 아래와 같다.

```cpp
EXPECT_CALL(mock_object, method(matchers))
    .Times(cardinality)
    .WillOnce(action)
    .WillRepeatedly(action);
```

`EXPECT_CALL()`은 두 가지 argument를 전달받는다. 첫 번째는 mock object이며 두 번째는 mock method(+ its argument)이다. 이 두 가지는 점(`.`)이 아니라 콤마(`,`)로 구분된다.

만약, 해당 mock method가 overloaded method가 아니라면 matcher 없이 바로 사용하는 것도 가능하다.

```c++
EXPECT_CALL(mock_object, non-overloaded-method)
    .Times(cardinality)
    .WillOnce(action)
    .WillRepeatedly(action);
```

`EXPECT_CALL()`을 matcher 없이 사용하게 되면 "called with any arguments"를 가능하게 해준다. 즉, argument의 타입이나 개수를 굳이 쓰지 않아도 된다. (overloaded method는 이름이 같은 method가 여러개 존재할 수 있기 때문에 matcher없이는 사용할 수 없다.)

`EXPECT_CALL()`에는 매우 다양한 clause를 붙일 수 있으며 각각에 대해서는 다음 섹션에서 계속 설명한다.

expectation을 지정하는 문법은 마치 영어를 읽는 것처럼 자연스럽게 읽을 수 있도록 설계되었다. 아래 예제를 보자.

```cpp
using ::testing::Return;
...
EXPECT_CALL(turtle, GetX())
    .Times(5)
    .WillOnce(Return(100))
    .WillOnce(Return(150))
    .WillRepeatedly(Return(200));
```

이것은 곧 "`turtle` object의 `GetX()`라는 함수는 총 5번 호출되는데, 첫 번째 호출에서는 100을 반환하고, 두 번째 호출에서는 150을 반환하고, 그 다음부터는 계속해서 200을 반환한다"라는 expectation을 의미한다. 어떤 사람들은 이러한 스타일을 Domain Specific Language(DSL)라고 부르기도 한다.

**Note:** `EXPECT_CALL`은 왜 macro 형태로 구현되었을까? 여기에는 두 가지 이유가 있다. 첫 번째로 검색하기가 편리하다. 원하는 `EXPECT_CALL`을 찾을 때, `gsearch`와 같은 도구와 연동해서 쉽게 찾아낼 수 있다. 더불어 사람의 눈으로 직접 확인할 때도 macro 형태의 코드가 더 알아보기 쉽다. 두 번째로 테스트가 실패했을 때, gMock이 해당 expectation의 위치를 알려주기가 용이하다. 그로 인해서 그 위치를 참고하는 사용자 입장에서도 더 편리해진다. 이런 부분들은 테스트가 실패해서 디버깅을 해야할 때 도움이 될 것이다.

#### Matchers: 전달되기를 바라는 argument 지정하기

먼저, gMock에서는 mock function으로 전달되는 argument의 기대값을 지정하는 것이 가능하다.

```c++
// Expects the turtle to move forward by 100 units.
EXPECT_CALL(turtle, Forward(100));
```

위 코드는 `Forward()`가 호출될 때, argument로 `100`이 전달되기를 기대한다는 의미이다. 물론 적법한 코드이지만 상황에 따라서 이처럼 구체적인 값까지는 필요하지 않을 수 있다. 앞에서도 언급한 것처럼 너무 엄격한 테스트는 깨지기 쉽고 때때로 테스트의 가독성을 떨어트리기도 하기 때문에 꼭 필요한 만큼만 지정하는 것이 중요하다. 테스트가 쉽게 깨지지 않으면서도 bug를 잘 찾아낼 수 있도록 계속해서 고민해야 한다. 다시 본론으로 돌아오면 `Forward()`로 전달되는 argument가 테스트의 목표에 큰 영향을 끼치지 않는다면 `_`라고 표시하는 것도 좋은 방법이다. 여기서 `_`는 어떤 값이라도 괜찮다는 것을 의미하는데 관련 예제가 아래에 있다.

```c++
using ::testing::_;
...
// Expects that the turtle jumps to somewhere on the x=50 line.
EXPECT_CALL(turtle, GoTo(50, _));
```

위 코드는 "`GoTo()`라는 mock function의 첫번째 argument에는 정확히 `50`이 전달되어야 하고, 두번째 argument에는 어떤 값이라도 올 수 있다"를 의미한다. 여기서 두번째 argument에 사용한 `_`이 바로 **matcher**의 한 종류이다. 이렇듯 matcher는 어떤 값이 전달되기를 기대하는지에 대해 좀 더 세밀한 조작이 가능하도록 도와주는 기능이며 `EXPECT_CALL()`과 함께 사용하면 매우 유용하다. (꼭, `EXPECT_CALL()`이 아니더라도 사용할 수는 있다.)

사실 `50`, `100`처럼 값으로 구현한 코드들도 내부적으로는 `Eq(50)`, `Eq(100)`과 같이 matcher를 사용하는 코드와 동일하게 동작한다. (`Eq()`는 값이 같은지 확인하는 matcher이다.) 한 가지 중요한 점은 `Eq()`를 사용하든 값을 사용하든 해당 타입은 `operator==`를 지원해야 한다는 점이다. 그래야만 gMock이 비교를 수행할 수 있으며 `operator==`를 지원하지 않는 타입에 `Eq()`를 사용하려면 해당 연산자를 직접 구현해서 제공해야 한다. 마지막으로 gMock은 `_`나 `Eq()` 외에도 다양한 built-in matcher들을 제공하고 있으며 이들을 확인하려면 [built-in matcher](cheat_sheet.md#matchers)를 참조하기 바란다. 더불어 새로운 matcher를 직접 구현하고자 한다면 [custom matcher](cook_book.md#새로운-matcher-구현하기)를 참조하면 된다.

아래 코드는 `Ge()`라는 또 다른 built-in matcher를 사용한 예제이며 "`turtle.Foward()`로 전달되는 argument가 100보다 크거나 같아야 한다"를 의미한다.

```cpp
using ::testing::Ge;
...
// Expects the turtle moves forward by at least 100.
EXPECT_CALL(turtle, Forward(Ge(100)));
```

위 코드도 마찬가지로 `Forward(_)`와 같이 구현하게 되면 어떤 argument가 전달돼도 괜찮음을 의미한다. `_`는 사실 좀 더 간결하게 구현하는 것도 가능한데 아래처럼 argument 부분을 아예 구현하지 않으면 `_`를 사용한 것과 동일한 의미로 동작하게 된다. 단, 이렇게 argument 부분을 생략하는 방법은 대상 mock function이 non-overloaded일 때만 사용할 수 있다.

```c++
// Expects the turtle to move forward.
EXPECT_CALL(turtle, Forward);
// Expects the turtle to jump somewhere.
EXPECT_CALL(turtle, GoTo);
```

#### Cardinalities: 몇 번 호출되어야 하는지 그 횟수를 지정하기

`EXPECT_CALL()`과 함께 사용되는 `Times()`라는 clause를 본 적이 있을 것이다. 이 `Times()`가 전달받는 argument를 바로 **cardinality**라고 부르며 이것은 곧 "*mock method가 n번 호출되기를 기대한다*"를 의미한다. 이러한 cardinality를 사용하면 동일한 expectation을 중복해서 구현하지 않고도 어떤 method가 여러번 호출되어야 한다는 것을 쉽게 표현할 수 있다. 또한, matcher의 `_`와 같이 cardinality도 "fuzzy"하게 지정할 수 있기 때문에 사용자가 테스트의 목적을 보다 쉽게 표현할 수 있을 것이다.

`Times(0)`은 좀 특이한 형태이다. 코드를 보고 예측할 수 있듯이 이것은 "mock method가 지정된 argument와 함께 호출되서는 안 된다"를 의미한다. 따라서 gMock은 `Times(0)`으로 지정된 mock method가 호출되면 테스트 실패로 판단하게 된다.

`AtLeast(n)`도 본 적이 있을 것이다. `AtLeast(n)`은 fuzzy cardinalities 중 하나이며 "최소한 `n`번 이상 호출되기를 기대한다"라는 의미로 사용된다. 이와 같이 gMock이 제공하는 다양한 built-in cardinalities는 확인하려면 [cheat_sheet](cheat_sheet.md#cardinalities)를 참조하기 바란다.

마지막으로 사용자가 `Times()`를 사용하지 않은 경우에는 **gMock이 cardiniality를 추론**하게 되는데, 이 때 적용되는 추론규칙은 아래와 같다.

- `WillOnce()`와 `WillRepeatedly()` 가 모두 없다면, cardinality → `Times(1)`
- `WillOnce()`가 `n`개(>=1) 있고 WillRepeatedly() 는 없다면, cardinality → `Times(n)`
- `WillOnce()`가 `n`개(>=0) 있고 WillRepeatedly() 가 있다면, cardinality → `Times(AtLeast(n))`

**Quick quiz**: 어떤 function이 2회 호출될 것으로 expectation 설정되었지만, 실제로는 4번 호출되었다면 어떤 일이 발생할까?

#### Actions: 수행해야 할 동작 지정하기

위에서 `MockTurtle` class를 생성할 때, 각각의 mock method를 실제로 구현하지는 않았다. 왜냐하면 `MOCK_METHOD` macro를 사용하면 gMock이 기본적인 구현을 해주기 떄문이다. 그러나 테스트의 목적을 명확히 표현하려면 mock method가 호출되었을 때 수행해야 할 동작을 직접 명세해주는 것이 더 좋다. 이 때, mock method가 수행해야 할 동작들을 action이라는 명칭으로 부르며 gMock은 이를 위한 다양한 기능을 제공한다.

먼저, mock function의 return type이 기본자료형 혹은 포인터라면 사용자가 별도의 action을 지정하지 않더라도 default action을 수행해 준다. 예를 들어 `void` 타입은 그냥 `return`, `bool` 타입은 `return false`, 다른 타입들은 `return 0`을 수행해 준다. 게다가 C++11 이후 버전부터는 mock function의 return type이 꼭 기본자료형이 아니더라도 default constructor를 가지고 있는 타입이라면 해당 default constructor를 자동으로 호출해 준다. 이처럼 gMock은 사용자가 아무런 action을 지정하지 않더라도 가능한 최대한의 default action을 알아서 수행한다.

그렇더라도 해당 mock method에 대한 default action이 없는 경우는 여전히 존재하며 default action만으로는 원하는 목적을 달성할 수 없는 경우도 많이 있다. 이런 경우에 사용자가 직접 action을 지정하는 것도 어렵지는 않으며 `WillOnce()` 혹은 `WillRepeatedly()`를 통해 action을 지정할 수 있다. 간단한 예제가 아래에 있다.

```cpp
using ::testing::Return;
...
EXPECT_CALL(turtle, GetX())
     .WillOnce(Return(100))
     .WillOnce(Return(200))
     .WillOnce(Return(300));
```

위 코드는 "`turtle.GetX()`가 3번 호출되기를 기대한다"는 의미이며 (`WillOnce()`가 `Times(1)`이라는 의미로 추론되기 때문에) 따라서 `GetX()`가 3번 호출되면 호출될 때마다 `100`, `200`, `300`을 순서대로 반환한다.

```cpp
using ::testing::Return;
...
EXPECT_CALL(turtle, GetY())
     .WillOnce(Return(100))
     .WillOnce(Return(200))
     .WillRepeatedly(Return(300));
```

위 코드는 "`turtle.GetY()`가 적어도 2번 호출되기를 기대한다"는 의미이다. (`WillOnce()`가 2번 사용됐기 때문에 최소한 2번은 호출되어야 한다.) 따라서 `GetY()`가 호출되면 처음에는 `100`을 반환하고 다음에는 `200`을 반환하며 세번째부터는 `300`을 반환한다.

물론, `Times()`를 통해서 호출횟수를 명시적으로 지정해도 되는데 그렇게 되면 gMock은 `WillOnce()`로부터 호출횟수를 추론하지는 않는다. `Time()`에 지정된 값이 `WillOnce()`보다 많으면 어떻게 될까? 그런 경우에는 지정된 횟수를 초과했다면 gMock이 제공하는 default action이 수행된다. 이렇게 default action이 사용되는게 싫다면 `WillRepeatedly()`를 사용해서 default action을 대체하는 것이 좋다.

`Return()`말고도 다양한 action이 존재한다. 예를 들면, `ReturnRef(variable)`을 사용해서 참조타입을 반환할 수도 있고 미리 구현해놓은 다른 함수를 연계적으로 호출할 수도 있다. 더 자세한 내용은 [여기](cheat_sheet.md#actions)를 확인하자.

**Important note**: `EXPECT_CALL()`는 action clause를 한 번 확인한 후에 그 내용을 저장해둔다. 그런 후에는 action이 수행 될 때마다 처음에 저장해 놓은 것을 계속 사용한다. 이러한 동작방식을 알고 있어야만 의도치 않은 실수를 방지할 수 있을 것이다. 아래 예제를 보자.

```cpp
using ::testing::Return;
...
int n = 100;
EXPECT_CALL(turtle, GetX())
    .Times(4)
    .WillRepeatedly(Return(n++));
```

위의 mock function이 100, 101, 102... 를 순서대로 반환할까? 그렇지 않다. 결과만 놓고 보면 항상 100을 반환하게 된다. 왜냐하면 `n++`이라는 코드가 수행되는 시점의 값인 `100`이 gMock 내부에 저장되었다가 실제로 호출되었을 때도 그대로 사용되기 때문이다. 다시 말해서 `GetX()`가 실제로 호출되는 시점의 `n` 값을 사용하는 것이 아니라 `EXPECT_CALL()` 코드가 수행되는 시점의 `n` 값을 저장해 놨다가 `GetX()`가 호출될 때에는 그걸 그대로 반환해 준다. 이와 유사하게 `EXPECT_CALL()`에 `Return(new Foo)` 라는 expectation을 지정해도 호출될 때마다 새로운 object를 반환해주는 것이 아니라 항상 같은 주소값(pointer 값)을 반환하게 된다. 만약, 사용자가 원했던 동작이 이렇게 고정된 값을 반환하는 것이 아니라 `n`에 저장된 값을 동적으로 읽어서 반환하고 싶었던 것이라면 [CookBook](cook_book.md#mock-method에서-live-value-반환하기)에서 이에 대한 해결방법을 확인할 수 있을 것이다.

이제 또 퀴즈를 풀어 보자. 아래코드는 무엇을 의미할까?

```cpp
using ::testing::Return;
...
EXPECT_CALL(turtle, GetY())
    .Times(4)
    .WillOnce(Return(100));
```

이것은 명확하게 `turtle.GetY()`가 4번 호출될 것임을 의미한다. 그러나 4번 모두 `100`을 반환한다고 생각하면 문제가 된다. `WillOnce()`는 맨 처음의 action에만 영향을 주기 떄문이다. 즉, `GetY()`에 대한 2번째 호출부터는 default action에 의해 `0`이 반환된다. 왜냐하면 `int` 타입을 반환하는 function의 default action은 `return 0`이기 때문이다.

#### 여러개의 Expectations 사용하기

지금까지는 mock method 한 개당 expectation 한 개만 지정한 예제들을 주로 다뤘는데 실제 개발환경에서는 다양한 expectation들을 복합적으로 사용하게 된다. 여기서는 mock method 한 개에 expectation 여러 개를 지정하는 방법에 대해 알아보자.

사용자가 여러개의 expectation을 지정하게 되면 gMock은 가장 밑에 있는 expectation부터 시작해서 조건을 만족하는 expectation이 있는지 차례로 확인한다. 즉, **reverse order**로 expectation을 탐색한다. 이렇게 탐색순서가 있다는 부분만 유의하면 여러개의 expectation을 적용하는 것도 개별 expectation을 사용하는 방법과 크게 다르지 않다. 중요한 것은 expectation 탐색순서를 잘 고려하지 않으면 의도한 대로 동작하지 않을 수도 있다는 점이다. 왜냐하면 expectation을 여러개 사용했다고 하더라도 어떤 expectation에 지정된 cardinality가 초과되면 upper-bound-violated failure가 발생한다는 사실은 여전히 동일하기 때문이다. 아래 예제를 보자.

```cpp
using ::testing::_;
...
EXPECT_CALL(turtle, Forward(_));  // #1
EXPECT_CALL(turtle, Forward(10))  // #2
    .Times(2);
```

위의 코드와 같이 `Forward()`라는 function에 2개의 expectation을 지정한 상태에서 `Foward(10)`이 3번 연속으로 호출된다면 upper-bound-violated failure가 발생하게 된다. 왜냐하면 #2번 expectation이 기대하고 있는 2회 호출을 초과하기 때문이다. 그런데 신기하게도 `Foward(20)`으로 3번 호출한다면 upper-bound-violated failure는 발생하지 않는다. 왜 그럴까? 그 이유는 바로 `Foward(20)`은 #1번 expectation을 사용하기 때문이다. 이러한 동작은 사용자의 세심한 주의가 필요한 부분으로서 관련된 해결방법을 이 문서의 하단에서 계속해서 설명한다.

**Note**: gMock은 왜 reverse order로 expectation을 탐색할까? 먼저, test fixture를 떠올려보자. 우리가 test fixture를 사용하는 목적은 test fixture 레벨에서 공통적으로 적용해야 할 내용들을 `SetUp()` 또는 constructor에 구현해서 중복된 코드를 없애기 위해서이다. 그런 후에 개별 test case에는 좀 더 특화된 내용을 구현하게 된다. 이 개념을 그대로 가져오면 `SetUp()` 또는 constructor에는 default expectation과 같은 일반적인 expectation을 먼저 구현하고 각각의 test case에는 특화된 expectation들을 구현하는 구조로 만들 수 있다. 다시 말하면 소스코드 상에서 나중에 수행되는 expectation일수록 더 특화된 expectation이라는 규칙이 만들어진다. 이제 이러한 규칙은 개별 test case 내부에서도 그대로 적용되어야 하기 때문에 어떤 method에 여러개의 expectation들이 존재한다면 아래쪽에 있는 expectation부터 탐색하도록 설계된 것이다.

**Tip:** 테스트코드 구현을 처음 시작하는 시점에는 expectation을 최대한 느슨하게 설정하는 것이 좋다. 쉽게 말해서 cardinality는 `Times(AnyNumber())`으로 하고 matcher는 `_`로 해놓고 시작하는 것이다. 그런 후에 개발을 진행하면서 점점 특화된 경우를 늘려가는 것이 유연성 측면에서 좋은 방법이다. 물론, "uninteresting call" 에 대해서는 굳이 이렇게 할 필요가 없겠지만, 관심대상이 되는 mock method에 대해서는 상당히 유용할 것이다. "uninteresting call"에 대해 궁금하다면 [Understanding Uninteresting vs Unexpected Calls](cook_book.md#uninteresting-vs-unexpected-를-구분하자)를 참조하자.

#### Ordered vs Unordered Calls

위에서 얘기한 것처럼 **특정(1개)** mock method에 여러개의 expectation이 지정되어 있는 상태에서 해당 mock method가 호출되면 reverse order로 expectation을 탐색하게 된다. 그런데 여기서 주의해야 할 점은 reverse order라는 것은 1개의 mock method에 지정된 expectation들을 탐색하는 순서일 뿐이며 우리는 아직 그들이 호출되어야 하는 순서를 지정한 적은 없다. 탐색순서와 호출순서가 서로 다른 것임에 주의하면서 이제부터는 mock method의 호출순서를 지정하는 방법에 대해 알아보자. 이 때는 1개의 mock method에 지정된 expectation들간의 호출순서뿐만 아니라 여러 mock method간의 호출순서를 지정하는 것도 포함한다.

gMock에서는 expectation들의 호출순서를 지정하는 방법도 아래처럼 간단하게 구현할 수 있다.

```c++
using ::testing::InSequence;
...
TEST(FooTest, DrawsLineSegment) {
  ...
  {
    InSequence seq;

    EXPECT_CALL(turtle, PenDown());
    EXPECT_CALL(turtle, Forward(100));
    EXPECT_CALL(turtle, PenUp());
  }
  Foo();
}
```

위 예제처럼 어떤 scope(block)를 만들고 scope 내부에 `InSequence` object를 생성하게 되면 해당 scope 내에 구현된 모든 expectation들이 *순서대로 호출되기를 기대한다*는 의미이다. 또한, 이 때의 호출순서는 reverse order가 아니며 위에서 아래 방향이 된다. 사용자가 몰라도 되는 부분이긴 하지만 gMock 내부적으로는 `InSequence` object의 constructor와 destructor를 통해서 이러한 호출순서를 검증하게 된다. (`InSequence` object의 이름이 무엇인지는 중요하지 않습니다.)

결과적으로 위 코드는 `Foo()`라는 function이 내부에서 `turtle` object의 mock method 3개(`PenDown()`, `Forward(100)`, `PenUp()`)를 순서대로 호출하기를 기대한다고 해석할 수 있다. 따라서 3개 function이 모두 호출되었다고 하더라도 `PenDown()` -> `Forward(100)` -> `PenUp()`이라는 순서를 지키지 않았다면 테스트는 실패한다.

만약, 관심대상인 function들을 단 하나의 흐름으로 묶는것이 아니라 여러 흐름으로 나눠서 호출순서를 지정하고 싶다면 어떻게 해야할까? gMock은 그러한 방법도 제공하고 있으며 관련된 내용은 [여기](cook_book.md#함수의-호출순서를-부분부분-지정하기)에서 확인하자.

#### 모든 Expectation은 연결되어 있습니다. (Sticky Expectations)

간단한 퀴즈를 풀어봅시다. `Turtle`이 원점에 도착한 횟수가 2번인지 확인하는 테스트를 구현하려면 어떻게 해야 할까? 즉, 테스트의 목적은 원점이동이 2회인지를 검증하는 것 외에 다른 것에는 관심이 없다고 가정해보자.

먼저 스스로 풀어보고 아래 코드와 비교해보자.

```cpp
using ::testing::_;
using ::testing::AnyNumber;
...
EXPECT_CALL(turtle, GoTo(_, _))  // #1
     .Times(AnyNumber());
EXPECT_CALL(turtle, GoTo(0, 0))  // #2
     .Times(2);
```

위 코드와 같이 expectation을 설정한 다음에 원점으로 이동하라는 명령인 `GoTo(0, 0)`를 3번 호출하면 어떻게 될까? 일단, gMock은 reverse order로 expectation을 탐색하기 때문에 #2번 expectation을 먼저 확인한다. 그런데 #2번에서는 `GoTo()`가 정확히 2번 호출될 것이라고 기대하고 있기 때문에 3번째로 호출되면 테스트는 실패하게 될 것이다. (여기서 실패하는 이유는 [Using Multiple Expectation](#여러개의-expectations-사용하기)을 참고)

이 예제는 사실 gMock의 **expectation들이 "sticky" 모드로 설정되어 있음**을 보여준다. "sticky" 모드는 호출횟수가 초과된 expectation들이 active 상태로 남아있는 모드를 의미한다. 이것은 매우 중요한 규칙인데 왜냐하면 다른 mocking framework들과 **구별되는** gMock의 특징이기 때문이다. 그럼 gMock에서는 왜 그렇게 했을까? 그 이유는 이러한 방식이 테스트를 구현하고 이해하는데 있어서 더 유리하다고 판단했기 때문이다.

이제 좀 더 복잡한 예제를 통해서 자세히 알아보자. 아래 예제의 결과를 어떻게 될까?

```cpp
using ::testing::Return;
...
for (int i = n; i > 0; i--) {
  EXPECT_CALL(turtle, GetX())
      .WillOnce(Return(10*i));
}
```

위 예제 코드는 `GetX()`를 호출하면 10, 20, 30, ... 과 같은 순서로 반환하기를 기대하고 구현한 코드이다. 코드가 조금 헷갈릴 수 있는데 반복문(`for`)의 초기값이 `n`이기 때문에 `EXPECT_CALL().WillOnce(Return(10*n))`, `EXPECT_CALL().WillOnce(Return(30))`, `EXPECT_CALL().WillOnce(Return(20))`, `EXPECT_CALL().WillOnce(Return(10))`과 같은 순서로 expectation이 설정된다.

이제 문제점을 분석해 보자. 과연 `GetX()`가 10, 20, 30, ... 을 순서대로 잘 반환할까? 그렇지 않다. 먼저 expectation들은 기본적으로 sticky 모드로 설정되어 있다. 따라서 `GetX()`가 처음 호출되면 `EXPECT_CALL().WillOnce(Return(10))`을 사용하여 원하는 대로 10을 반환하지만 두번째로 호출되었을 때는 원하던 값인 20을 반환하지 못 한다. 왜냐하면 `EXPECT_CALL().WillOnce(Return(10))`이 여전히 active상태이기 때문에 두 번째 호출시점에도 `EXPECT_CALL().WillOnce(Return(10))`을 사용하려고 시도하게 된다. 더군다나 gMock의 cardinality 추론규칙에 의하면 `WillOnce()`는 곧 `Times(1)`을 의미하기 때문에 해당 expectation은 upper bound violated failure를 발생시키게 된다.

그럼 `GetX()`가 10, 20, 30, ... 을 반환하게 하려면 어떻게 해야할까? 이를 위해서는 명시적으로 expectation이 sticky하지 않다고 알려줘야 한다. 즉, 지정한 호출횟수를 만족한 expectation은 retire시켜주는 것이다.

```cpp
using ::testing::Return;
...
for (int i = n; i > 0; i--) {
  EXPECT_CALL(turtle, GetX())
      .WillOnce(Return(10*i))
      .RetiresOnSaturation();
}
```

위 코드와 같이 `RetiresOnSaturation()`을 사용하면 expectation을 retire 처리할 수 있다.

`InSequence`를 사용하는 경우에는 상황이 좀 달라진다. 위에서 설명한 것처럼 `InSequence`에 포함된 expectation들은 위에서 아래 방향으로 호출순서가 지정된다. 따라서 `EXPECT_CALL()`을 지정하는 반복문의 초기값이 `1`부터 시작해야만 원하는 대로 10, 20, 30, ... 순서로 반환될 것이다.

```cpp
using ::testing::InSequence;
using ::testing::Return;
...
{
  InSequence s;

  for (int i = 1; i <= n; i++) {
    EXPECT_CALL(turtle, GetX())
	.WillOnce(Return(10*i))
	.RetiresOnSaturation();
  }
}
```

만약, `InSequence` 내부에 `RetiresOnSaturation()`이 없다면 될까? 이 때에는 바로 다음순서의 expectation으로 자동으로 넘어가기 때문에 문제가 없다. 즉, `InSequence`를 사용하면 기대되는 횟수가 만족된 expectation이 자동으로 retire되기 때문에 `RetiresOnSaturation()`를 사용하지 않아도 괜찮다. (물론, 사용해도 된다.)

#### Uninteresting Calls

Mock class에 여러개의 mock method가 있을 때, 사용자가 모든 mock method를 사용하지는 않을 수도 있다. 만약에 `GetX()`, `GetY()`라는 mock method가 얼마나 호출되든 관심이 없다면 어떻게 해야 할까?

이렇게 관심대상이 아닌 mock method에 대해서는 그냥 아무런 조치도 안 하면 된다. 물론 gMock이 아무런 조치도 안 한(expectation이 없는) mock method를 확인해서 warning을 출력하긴 하지만 테스트의 최종결과(성공이나 실패)에 영향을 주지는 않는다. 이렇게 관심대상이 아닌 mock method에 대해서 warning을 출력해주는 동작방식을 "naggy"라고 하는데 이러한 방식도 변경은 가능하다. 보다 자세한 내용은 [The Nice, the Strict, and the Naggy](cook_book.md#nice-모드-strict-모드-naggy-모드-naggy-잔소리가-심한)를 참조하자.
