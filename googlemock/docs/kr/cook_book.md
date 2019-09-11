## gMock Cookbook

이 문서에서는 gMock의 심화된 사용방법을 확인할 수 있습니다. 아직 gMock 사용경험이 없는 분은 [ForDummies](for_dummies.md) 문서를 먼저 보는 것이 좋습니다.

**Note:** gMock은 소스코드상에 `testing` 이라는 namespace에 구현되어 있습니다. 따라서 gMock에 구현된 `foo`라는 기능을 사용하고자 한다면 `using ::testing::Foo` 와 같이 namespace를 명시해서 사용해야 합니다. 이 문서에는 예제코드를 간단하게 작성하기 위해서 `using`을 사용하지 않은 코드도 종종 있긴 하지만 사용자가 구현할 때는 `using`을 꼭 사용해야 합니다.

### Mock Class 만들기 ###

사실 mock class를 정의하는 방법은 일반적인 C++ class를 정의하는 방법과 다르지 않습니다. 다만, mock class 내부에 mock method를 정의할 때는 `MOCK_METHOD`라는 macro를 사용해야 합니다. 이 macro를 사용해야만 mock method로서 사용할 수 있습니다. 물론, mock class 안에서 mock method가 아니라 일반 method를 정의할 때는 macro를 사용하지 않아도 됩니다. Macro는 3개 또는 4개의 parameter를 전달받을 수 있습니다. 아래 예제를 보시기 바랍니다.

```cpp
class MyMock {
 public:
  MOCK_METHOD(ReturnType, MethodName, (Args...));
  MOCK_METHOD(ReturnType, MethodName, (Args...), (Specs...));
};
```

처음 3개 parameter에는 mocking하려는 대상 method의 signature 정보만 그대로 써주면 됩니다. 마지막 4번째 parameter에는 대상 mock method의 특성을 적어줘야 합니다. 여기서 특성이라 함은 아래와 같은 정보를 의미합니다.

*   **`const`** - mock method를 `const`로 선언합니다. 대상 method가 `const` method 일 때 사용하면 됩니다.
*   **`override`** - mock method를 `override`로 선언합니다. 대상 method가 `virtual` method 일 때 사용하면 됩니다.
*   **`noexcept`** - mock method를 `noexcept`로 선언합니다. 대상 method가 `noexcept` method 일 때 사용하면 됩니다.
*   **`Calltype(...)`** - mock method의 call type을 지정합니다. Windows 환경에서 주로 사용합니다.(예: `STDMETHODCALLTYPE`)

#### Comma(`,`)를 문제없이 사용하는 방법

`MOCK_METHOD` macro를 사용할 때, comma의 사용은 주의가 필요합니다.

```c++
class MockFoo {
 public:
  MOCK_METHOD(std::pair<bool, int>, GetPair, ());  // Won't compile!
  MOCK_METHOD(bool, CheckMap, (std::map<int, double>, bool));  // Won't compile!
};
```

위 코드는 macro에 전달된 comma로 인해서 argument를 파싱할 때 모호함이 생기고 결국 compile error가 발생합니다. 다행히도 이에 대한 해결방법은 그렇게 어렵지 않습니다. 해결방법 2가지를 아래에서 확인하세요.

해결방법 1 - 괄호로 감싸기

```c++
class MockFoo {
 public:
  MOCK_METHOD((std::pair<bool, int>), GetPair, ());
  MOCK_METHOD(bool, CheckMap, ((std::map<int, double>), bool));
};
```

이 해결방법은 첫번째 argument, 세번째 argument를 명확히 표현하기 위해 괄호를 사용했습니다. 각각은 mocking 대상(`GetPair()`, `CheckMap()`)의 return type과 argument type을 의미합니다. 물론 C++에서 return type, argument type을 괄호로 감싸는 것은 적법하지 않지만, `MOCK_METHOD` macro는 파싱하고 나서 괄호를 제거하기 때문에 문제가 없습니다.

해결방법 2 - alias 사용하기

```c++
class MockFoo {
 public:
  using BoolAndInt = std::pair<bool, int>;
  MOCK_METHOD(BoolAndInt, GetPair, ());
  using MapIntDouble = std::map<int, double>;
  MOCK_METHOD(bool, CheckMap, (MapIntDouble, bool));
};
```

#### Private, Protected 영역에 정의된 Method를 Mocking하기 ####

Base class의 method가 `public`, `protected`, `private` 등 어느 영역에 정의되어 있는지에 관계없이 mock method를 정의할 때 사용하는 `MOCK_METHOD` macro는 항상 `public` 영역에서 사용해야 합니다. (C++은 base class의 virtual function이 어느 영역에 선언되었는지에 관계없이 derived class에서는 원하는 영역에 자유롭게 overriding 할 수 있습니다.) 이렇게 해야만 `ON_CALL()`, `EXPECT_CALL()`과 같은 매크로가 mock class 또는 mock function에 접근할 수 있습니다. 아래 예제를 확인하세요.

```cpp
class Foo {
 public:
  ...
  virtual bool Transform(Gadget* g) = 0;

 protected:
  virtual void Resume();

 private:
  virtual int GetTimeOut();
};

class MockFoo : public Foo {
 public:
  ...
  MOCK_METHOD(bool, Transform, (Gadget* g), (override));

  // The following must be in the public section, even though the
  // methods are protected or private in the base class.
  MOCK_METHOD(void, Resume, (), (override));
  MOCK_METHOD(int, GetTimeOut, (), (override));
};
```

#### Overloaded Method를 Mocking 하기 ####

Overloaded function을 mocking하는 것도 기존의 방법과 다르지 않습니다.

```cpp
class Foo {
  ...

  // Must be virtual as we'll inherit from Foo.
  virtual ~Foo();

  // Overloaded on the types and/or numbers of arguments.
  virtual int Add(Element x);
  virtual int Add(int times, Element x);

  // Overloaded on the const-ness of this object.
  virtual Bar& GetBar();
  virtual const Bar& GetBar() const;
};

class MockFoo : public Foo {
  ...
  MOCK_METHOD(int, Add, (Element x), (override));
  MOCK_METHOD(int, Add, (int times, Element x), (override));

  MOCK_METHOD(Bar&, GetBar, (), (override));
  MOCK_METHOD(const Bar&, GetBar, (), (const, override));
};
```

**Note:** 어떤 method에 overloaded method가 여러개 있다고 가정하겠습니다. 이 때, 전체가 아니라 일부 method만 mocking한다면 compiler가 warning message를 출력할 것입니다. Warning message의 목적은 base class의 몇몇 method가 숨겨져 있음을 알려주는 것입니다. 만약 이러한 warning message가 신경쓰인다면 `using`을 이용해서 dervied class에서 사용할 수 있게 하면 됩니다.

```cpp
class MockFoo : public Foo {
  ...
  using Foo::Add;
  MOCK_METHOD(int, Add, (Element x), (override));
  // We don't want to mock int Add(int times, Element x);
  ...
};
```

#### Class Template Mocking 하기 ####

Class template을 mocking하는 것도 기존의 방법과 다르지 않습니다.

```cpp
template <typename Elem>
class StackInterface {
  ...
  // Must be virtual as we'll inherit from StackInterface.
  virtual ~StackInterface();

  virtual int GetSize() const = 0;
  virtual void Push(const Elem& x) = 0;
};

template <typename Elem>
class MockStack : public StackInterface<Elem> {
  ...
  MOCK_METHOD(int, GetSize, (), (override));
  MOCK_METHOD(void, Push, (const Elem& x), (override));
};
```

#### Non-virtual Method Mocking 하기 ####

gMock에서는 non-virtual function도 간단하게 mocking 할 수 있습니다. 이러한 방법을 hi-perf dependency injection이라고도 부르는데요. 왜냐하면 vtable과 같이 virtual function에 필수적으로 수반되는 자원을 사용하지 않아도 되기 때문입니다.

Hi-perf dependency injection에서는 상속관계를 사용하지 않습니다. Dependency injection을 위한 방법으로 template을 이용하기 때문에 mock class가 interface 혹은 base class를 상속받을 필요가 없습니다. 대신 real class의 method 중에서 관심있는 method를 찾은 후에, mock class에 해당 method와 동일한 signature를 갖는 method를 정의하기만 하면 됩니다. 정의하는 방법은 *기존과 거의 유사하지만* `MOCK_METHOD` macro의 4번째 argument에 사용하던 `override` 키워드는 전달하지 않는다는 점만 다릅니다.

```cpp
// A simple packet stream class.  None of its members is virtual.
class ConcretePacketStream {
 public:
  void AppendPacket(Packet* new_packet);
  const Packet* GetPacket(size_t packet_number) const;
  size_t NumberOfPackets() const;
  ...
};

// A mock packet stream class.  It inherits from no other, but defines
// GetPacket() and NumberOfPackets().
class MockPacketStream {
 public:
  MOCK_METHOD(const Packet*, GetPacket, (size_t packet_number), (const));
  MOCK_METHOD(size_t, NumberOfPackets, (), (const));
  ...
};
```

코드에서 보이는 것처럼 `MockPacketStream`에는 `GetPacket()`과 `NumberOfPackets()`만 정의하고 `AppendPacket()`는 정의하지 않았습니다. 이처럼 사용하지 않는 method는 굳이 정의하지 않아도 됩니다.

이제 2개의 class를 상황에 따라 구분해서 사용할 수 있도록 구현해야 합니다. `ConcretePacketStream`은 제품코드에 사용하고 `MockPacketStream`은 테스트코드에 사용할 것입니다. 이 때, 2개의 class간에는 상속관계가 없기 때문에 다형성이 compile time에 결정될 수 있도록 구현해야 합니다.

몇 차례 언급한 것처럼 이 경우에는 template을 사용하게 됩니다. 즉, 2개의 class를 사용하는 곳에서 이들을 template type으로 취급하도록 구현하면 됩니다. 이를 통해 제품코드에 대해서는 `ConcretePacketStream` class가 사용되고 테스트코드에 대해서는 `MockPacketStream` class가 사용될 것입니다. 아래는 관련 예제입니다.

```cpp
template <class PacketStream>
void CreateConnection(PacketStream* stream) { ... }

template <class PacketStream>
class PacketReader {
 public:
  void ReadPackets(PacketStream* stream, size_t packet_num);
};
```

위 코드를 보면 template parameter의 이름을 `PacketStream`으로 해서 2개 class 모두를 일반적으로 가리킬 수 있도록 했습니다. 이제 사용하기만 하면됩니다. 즉, 제품코드에서는 `CreateConnection<ConcetePacketStream>()`, `PacketReader<ConcretePacketStream>`과 같이 사용하면 되고 테스트코드에서는 `CreateConnection<MockPacketStream>()`, `PacketReader<MockPacketStream>`과 같이 사용하면 됩니다. 아래는 이렇게 구현된 테스트코드의 예제입니다.

```cpp
  MockPacketStream mock_stream;
  EXPECT_CALL(mock_stream, ...)...;
  .. set more expectations on mock_stream ...
  PacketReader<MockPacketStream> reader(&mock_stream);
  ... exercise reader ...
```

#### Free Function Mocking 하기 ####

gMock에서는 free function을 mocking하는 것도 가능합니다. 여기서 free function이란 C-style function 또는 static method를 의미합니다. 잘 생각해보면 지금까지는 non-static class method만 다뤘음을 알 수 있습니다. gMock에서 free function을 mocking하기 위해서는 interface(abstract class)를 새로 만들어야 합니다.

쉽게 말해서 free function을 위한 wrapper class(interface)를 만들어야 합니다. 아래 코드를 보겠습니다. 먼저 mocking 대상은 `OpenFile()`이라는 free function입니다. 이를 mocking하기 위해서 `FileInterface`라는 interface를 새로 만들었으며 `Open()`이라는 pure abstract method도 선언했습니다. 다음으로 `FileInterface`를 상속받는 derived class(`File`)를 만들고 `Open()`이라는 method가 mocking대상인 `OpenFile()`을 호출하도록 overriding 했습니다.

```cpp
class FileInterface {
 public:
  ...
  virtual bool Open(const char* path, const char* mode) = 0;
};

class File : public FileInterface {
 public:
  ...
  virtual bool Open(const char* path, const char* mode) {
     return OpenFile(path, mode);
  }
};
```

다음으로 해야할 일은 `OpenFile()`이라는 free function을 직접 호출하던 기존의 코드들을 `FileInterface`의 `Open()`을 호출하도록 변경합니다. 그럼 끝입니다. 이렇게 free function들이 wrapper class를 갖게 되면 다음 순서는 non-static class method를 mocking하는 방법과 동일하게 진행하면 됩니다.

물론, 이와 같은 과정이 귀찮게 느껴질 수도 있습니다. 하지만 추상화 계층을 한 번만 잘 설계해두면 연관된 free function들을 같은 interface로 묶을 수 있습니다. 해야할 일이 거의 없을 뿐더러 소스코드는 깔끔해지고 mocking도 간단해집니다.

혹시나 virtual function을 호출하기 때문에 발생하는 성능저하가 우려된다면 [mocking non-virtual methods](cook_book.md#nonvirtual-method-mocking-하기)와 같이 사용하면 성능저하도 크게 문제가 되지 않습니다.

#### 이전방식의 macro인 `MOCK_METHODn` 간단소개

현재와 같은 `MOCK_METHOD` macro가 구현되기 전에는 `MOCK_METHODn` 계열의 macro를 사용했습니다. 이전방식 macro에서 `n`은 argument의 개수를 의미하며 이로 인해 argument 개수에 따라 여러개의 macro가 존재하게 되므로 약간의 불편함이 있었습니다. 물론, `MOCK_METHODn`계열도 여전히 지원하고는 있지만 사용자 환경에서도 점차 새로운 방식으로 변경하시길 추천합니다.

아래는 `MOCK_METHOD`와 `MOCK_METHODn`의 차이점을 보여줍니다.

- 기본구조가 `MOCK_METHOD(ReturnType, MethodName, (Args))`에서 `MOCK_METHODn(MethodName, ReturnType(Args))`으로 변경되었습니다.
- Argument의 개수를 의미하던 `n`이 제거되었습니다.
- const method를 위해서 존재하던 `MOCK_CONST_METHODn`가 제거되고 const 정보를 argument로 전달하도록 변경되었습니다.
- class template을 위해서 존재하던 `T`라는 suffix가 제거되었습니다.
- 함수 호출방식을 알려주는 `_WITH_CALLTYPE`이라는 suffix가 제거되고 역시 argument로 전달하도록 변경되었습니다.

아래 표에서 이전방식에서 사용하던 기능이 어떻게 대체되었는지 자세히 확인할 수 있습니다.

| **Purpose**                                         | Ver. | Macro                                                        |
| :-------------------------------------------------- | :--- | :----------------------------------------------------------- |
| **Simple**                                          | Old  | `MOCK_METHOD1(Foo, bool(int))`                               |
|                                                     | New  | `MOCK_METHOD(bool, Foo, (int))`                              |
| **Const Method**                                    | Old  | `MOCK_CONST_METHOD1(Foo, bool(int))`                         |
|                                                     | New  | `MOCK_METHOD(bool, Foo, (int), (const))`                     |
| **Method in a Class Template**                      | Old  | `MOCK_METHOD1_T(Foo, bool(int))`                             |
|                                                     | New  | `MOCK_METHOD(bool, Foo, (int))`                              |
| **Const Method in a Class Template**                | Old  | `MOCK_CONST_METHOD1_T(Foo, bool(int))`                       |
|                                                     | New  | `MOCK_METHOD(bool, Foo, (int), (const))`                     |
| **Method with Call Type**                           | Old  | `MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, Foo, bool(int))` |
|                                                     | New  | `MOCK_METHOD(bool, Foo, (int), (Calltype(STDMETHODCALLTYPE)))` |
| **Const Method with Call Type**                     | Old  | `MOCK_CONST_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, Foo, bool(int))` |
|                                                     | New  | `MOCK_METHOD(bool, Foo, (int), (const, Calltype(STDMETHODCALLTYPE)))` |
| **Method with Call Type in a Class Template**       | Old  | `MOCK_METHOD1_T_WITH_CALLTYPE(STDMETHODCALLTYPE, Foo, bool(int))` |
|                                                     | New  | `MOCK_METHOD(bool, Foo, (int), (Calltype(STDMETHODCALLTYPE)))` |
| **Const Method with Call Type in a Class Template** | Old  | `MOCK_CONST_METHOD1_T_WITH_CALLTYPE(STDMETHODCALLTYPE, Foo, bool(int))` |
|                                                     | New  | `MOCK_METHOD(bool, Foo, (int), (const, Calltype(STDMETHODCALLTYPE)))` |

#### Nice 모드, Strict 모드, Naggy 모드 (Naggy: 잔소리가 심한) ####

`EXPECT_CALL()` 없이 호출된 mock method가 있다면, gMock은 "uninteresting call"이라는 warning을 출력한 후에 default action을 수행합니다. 이 때, warning을 출력해주는 이유는 아래와 같습니다.

- 테스트코드가 이미 작성된 상태라고 해도 시간이 지남에 따라 mocking 대상 interface(혹은 base class)에 새로운 method가 추가될 수 있습니다. 또 이렇게 새로 추가된 method가 있다면 이를 위한 mock method를 추가하는 것도 자연스러운 과정입니다. 따라서 gMock에서는 새로 추가된 mock method가 있는데 `EXPECT_CALL()` 없이 호출되었으니 한 번 확인해보라는 의미로 warning을 출력합니다.
- 이렇게 예측하지 못했던 mock method가 호출되는 상황은 추후에 문제가 될 수도 있기 때문에 테스트 failure까지는 아니더라도 warning을 통해 알려주는 것입니다. 따라서 warning이 발생하면 해당 mock method가 호출되어야 할 상황인지 아닌지 확인해보기 바랍니다. 만약 호출되는 것이 맞다면 해당 mock method에 대해 `EXPECT_CALL()`만 추가하면 됩니다. 그럼 더 이상 warning도 발생하지 않을 것입니다.

"uninteresting call"이라는 warning을 출력해주는 것은 말 그대로 경고의 의미입니다. 다만, 이러한 warning조차도 아예 출력되지 않기를 바라는 사람도 있을 수 있고 또는 모든 warning을 error로 처리하고 싶은 사람도 있을 것입니다. gMock에서는 이러한 처리방법을 사용자가 선택할 수 있게 했습니다. 더불어 mock object 별로 이러한 설정을 다르게 할 수도 있습니다.

테스트코드에서 `MockFoo`라는 mock class를 사용하는 예제를 보겠습니다.

```cpp
TEST(...) {
  MockFoo mock_foo;
  EXPECT_CALL(mock_foo, DoThis());
  ... code that uses mock_foo ...
}
```

먼저, `MockFoo`에는 여러개의 mock method가 정의되어 있다고 가정하겠습니다. 이런 상황에서 위의 테스트코드를 실행했더니 `EXPECT_CALL()`을 명시적으로 사용한 `DoThis()` 외에 다른 mock method의 호출도 발생했습니다. 그럼 gMock은 warning을 출력할텐데 만약 이러한 warning을 보고 싶지 않다면 mock object를 생성할 때 `NiceMock<MockFoo>`으로 사용하면 됩니다. 즉, 아래와 같이 구현하면 더 이상 warning을 출력하지 않습니다.

```cpp
using ::testing::NiceMock;

TEST(...) {
  NiceMock<MockFoo> mock_foo;
  EXPECT_CALL(mock_foo, DoThis());
  ... code that uses mock_foo ...
}
```

gMock은 내부적으로 `NiceMock<MockFoo>`를 `MockFoo`의 derived class로 생성하기 때문에 `MockFoo`가 사용가능한 곳에는 `NiceMock<MockFoo>`도 사용할 수 있습니다.

또한, `NiceMock<MockFoo>`는 `MockFoo`의 constructor도 상속받기 때문에 `MockFoo`를 생성하던 방식도 그대로 사용할 수 있습니다. 

```cpp
using ::testing::NiceMock;

TEST(...) {
  NiceMock<MockFoo> mock_foo(5, "hi");  // Calls MockFoo(5, "hi").
  EXPECT_CALL(mock_foo, DoThis());
  ... code that uses mock_foo ...
}
```

다음으로 설명할 내용은 `StrictMock`인데 이름에서 유추되듯이 `StrictMock`은 uninteresting call이 발생하면 테스트 실패로 간주합니다. 비록 목적은 다르지만 사용방법은 `NiceMock`과 동일합니다.

```cpp
using ::testing::StrictMock;

TEST(...) {
  StrictMock<MockFoo> mock_foo;
  EXPECT_CALL(mock_foo, DoThis());
  ... code that uses mock_foo ...

  // The test will fail if a method of mock_foo other than DoThis()
  // is called.
}
```

NOTE: `NiceMock`과 `StrictMock`의 대상은 *uninteresting call*이지 *unexpected call*이 아니라는 것을 기억하세요. 이에 대한 자세한 내용은 [Understanding Uninteresting vs Unexpected Calls](cook_book.md#uninteresting-vs-unexpected-를-구분하자)를 참조하세요.

지금까지 설명한 기능들을 사용하기 위해서 지켜야 할 규칙이 몇 가지 있습니다. (주로 C++ 언어의 특성으로 인해 만들어진 것들입니다.)

1. `NiceMock<MockFoo>`, `StrictMock<MockFoo>`는 `MockFoo` 내부에 **직접** 정의된(`MOCK_METHOD` 사용) mock method에만 적용됩니다. 만약, `MockFoo`의 **base class**에 mock method가 정의되어 있다면 동작하지 않을 확률이 높습니다. Compiler에 따라 다를 수는 있지만 대부분의 경우에 동작하지 않습니다. 더불어 `NiceMock<StrictMock<MockFoo>>`와 같이 `NiceMock`, `StrictMock`을 nesting하는 것은 **지원하지 않습니다**.
2. `MockFoo`의 destructor가 virtual이 아니라면 `NiceMock<MockFoo>`, `StrictMock<MockFoo>`가 정상적으로 동작하지 않을 수 있습니다. 이 이슈는 내부적으로 확인 중에 있습니다.
3. `MockFoo`의 constructor나 destructor가 수행되는 동안에는 nice 모드, strict 모드가 적용되지 *않습니다*. 따라서 constructor나 destructor에서 `this`를 이용해 mock method를 호출하는 경우에는 의도한 것과 다르게 동작할 수 있습니다. 이러한 동작방식은 C++ 언어자체의 제약사항에 기반하며 constructor나 destructor에서 `this`를 이용해 virtual method를 호출하는 경우에는 해당 method를 non-virtual로 취급하게 됩니다. 다시 말해서 derived class의 constructor나 destructor가 호출되면 자연스럽게 base class의 constructor나 destructor도 호출될텐데 그런 경우에 base class의 constructor나 destructor에서 `this`를 사용했다면 이는 base class 자신을 가리킨다는 의미입니다. 이런 동작방식은 안정성을 보장하기 위한 C++언어 특징 중 하나입니다. 만약, 이렇게 동작하지 않는다면 base class의 constructor는 아직 초기화되지 않은 derived class의 정보를 사용하려고 시도할 것이며 마찬가지로 destructor도 이미 삭제된 derived class의 정보를 참조하게 되므로 심각한 문제를 초래할 수 있습니다.

마지막으로 naggy 모드, strict 모드는 테스트가 자주 실패하게 하고 유지보수를 어렵게 만들기도 하기 때문에 **주의**해서 사용해야 합니다. 예를 들어, 코드의 외부는 그대로 두고 내부적인 동작에 대해서만 refactoring하는 경우라면 테스트코드는 수정하지 않는 것이 이상적입니다. 그러나 naggy 모드로 설정되어 있다면 이러한 내부적인 수정에 대해서도 많은 warning들이 발생 할 것입니다. 게다가 strict 모드라면 아예 테스트가 실패하기 때문에 테스트코드의 유지보수 비용이 증가될 것입니다. 이를 위해 추천드리는 방법은 일반적인 상황에서는 nice 모드을 주로 사용하고 테스트코드를 개발할 때는 naggy 모드를 사용하는 것입니다. (다만, 현재 gMock은 naggy모드가 기본설정이긴 합니다.) 그리고 strict 모드는 필요한 경우에 한번씩 점검용으로 사용하기 바랍니다.

#### 기존코드에 영향을 주지 않고, interface를 단순하게 만들기 ####

아래처럼 argument의 개수가 아주 많은 method를 mocking한다면 테스트코드가 매우 복잡해 질 것입니다. 이런 경우에는 테스트의 목적을 달성하는 과정에서 크게 중요하지 않은 argument들은 걸러냄으로써 interface를 보다 단순하게 만들 수 있습니다.

```cpp
class LogSink {
 public:
  ...
  virtual void send(LogSeverity severity, const char* full_filename,
                    const char* base_filename, int line,
                    const struct tm* tm_time,
                    const char* message, size_t message_len) = 0;
};
```

위의 `send()`는 argument도 많고 게다가 `message`라는 argument는 문자열이기 때문에 문자일이 길어지면 길어질수록 소스코드를 해석하기가 어려워질 것입니다. 이러한 method에는 mock을 적용하는 것도 어렵습니다. 그렇다면 어떻게 해야할까요?

방법은 간단한 mock method를 하나 만들고 이것을 `send()`의 내부에서 호출하도록 구현하는 것입니다.

```cpp
class ScopedMockLog : public LogSink {
 public:
  ...
  virtual void send(LogSeverity severity, const char* full_filename,
                    const char* base_filename, int line, const tm* tm_time,
                    const char* message, size_t message_len) {
    // We are only interested in the log severity, full file name, and
    // log message.
    Log(severity, full_filename, std::string(message, message_len));
  }

  // Implements the mock method:
  //
  //   void Log(LogSeverity severity,
  //            const string& file_path,
  //            const string& message);
  MOCK_METHOD(void, Log,
              (LogSeverity severity, const string& file_path,
               const string& message));
};
```

위의 코드는 `send()`의 argument가 너무 많고 복잡하기 때문에 `Log()`라는 mock method를 새로 만들어서 `send()`로부터 관심대상이 되는 3개의 argument만 전달받도록 구현한 것입니다. 물론, 어디까지나 `Log()`가 전달받는 3개의 argument를 제외한 나머지는 관심대상이 아니기에 가능한 일입니다. 마지막으로 `Log()`에 expectation을 설정하면 됩니다. 이런 방법을 통해 `send()`를 mocking하는 것보다 간결하게 테스트코드를 구현할 수 있습니다.

동일한 방법을 overloaded function을 mocking할 때에도 적용할 수 있습니다.

```c++
class MockTurtleFactory : public TurtleFactory {
 public:
  Turtle* MakeTurtle(int length, int weight) override { return DoMakeTurtle(); }
  Turtle* MakeTurtle(int length, int weight, int speed) override { return DoMakeTurtle(); }

  // the above methods delegate to this one:
  MOCK_METHOD(Turtle*, DoMakeTurtle, ());
};
```

위의 코드의 모든 `MakeTurtle()`(overloaded method)들은 최종적으로는 `DoMakeTurtle()`을 호출하고 있습니다. 따라서 `DoMakeTurtle()`라는 method만 mocking하면 모든 overloaded method들이 동일하게 동작하도록 만들 수 있습니다.

이제 `DoMakeTurtle()`의 action만 아래와 같은 방법으로 지정하면 끝입니다. `MakeMockTurtle()`의 구현부는 생략되었지만 이름에서 알 수 있듯이 `Turtle` object를 생성해주는 action을 의미합니다.

```c++
ON_CALL(factory, DoMakeTurtle)
    .WillByDefault(MakeMockTurtle());
```

#### Concrete Classe Mocking 하기, 그리고 대안 ####

상위 interface가 없는 class를 바로 mocking 해야하는 상황도 발생할 수 있습니다. 이러한 class들을 `Concrete`라고 부르겠습니다. 이러한 `Concrete` class는 어떻게 mocking해야 할까요? 지금까지 공부한 내용을 기반으로 하면 먼저 `Concrete` class에 포함된 method를 virtual function으로 만들고 다음으로 `Concrete` class를 상속받는 mock class를 만들면 될 것 같습니다.

그러나 그렇게 하면 안됩니다.

왜냐하면 non-virtual function을 virtual function으로 만드는 것은 설계상의 큰 변화이며 class의 동작도 달라지기 때문입니다. Class invariants를 제어하기가 힘들어질 것입니다. 단순히 mocking을 위한 목적으로 그렇게 수정하는 것은 좋지 않습니다. Non-virtual function을 virtual function으로 바꾸고 derived class에서 override하는 구조로 변경할 때에는 타당한 이유가 있어야 합니다.

그럼 어떻게 할까요? `Concrete` class를 직접 mocking하는 것도 좋지 않습니다. 왜냐하면 제품코드와 테스트코드 간의 coupling이 증가되기 때문입니다. 예를 들어 `Concrete` class에 작은 수정이 발생할 때마다 mock class가 제대로 동작하는지 확인해야 하기 때문에 유지보수 비용이 상당히 증가할 것입니다.

사실 이러한 coupling 때문에 많은 개발자들이 interface를 선호합니다. 꼭 테스트코드를 위해서가 아니더라도 `Concrete` class를 사용하는 것은 coupling 문제를 야기하는 원인 중에 하나입니다. 이제 답을 드리면 `Concrete` class를 직접 사용하기보다는 interface를 하나 만들어서 `Concrete`의 adapter처럼 사용하기를 추천합니다. Interface는 항상 virtual function만 정의되어 있기 때문에 mocking 측면에서 유리하며 상술한 것처럼 꼭 테스트코드를 구현하기 위한 목적이 아니더라도 좋은 설계가 될 것입니다.

물론, interface를 사용하는 것이 몇 가지 overhead를 발생시키긴 합니다.

- Virtual function call 에 대한 비용이 추가됩니다. (그러나 대부분의 경우에는 미미합니다.)
- 개발자들이 공부해야할 내용(추상화 계층)이 추가됩니다.

그러나 그와 동시에 testability 관점에서는 많은 장점이 있습니다.

- 먼저, `Concrete` class의 API가 사용자가 원하는 방향과는 딱 맞지 않을수도 있습니다. 왜냐하면 `Concrete` class는 너무 구체화된 api들이 포함되어 있기 때문에 그것을 재사용하는 입장에서는 약간씩 수정해야 할 부분들이 생기기 때문입니다. 이런 경우에는 `Concrete` class를 위한 자신만의 interface를 만들어보면 어떨까요? 그렇게 되면 단순히 조금 변경하는 것을 넘어 상위수준의 기능을 추가할 수도 있고, 아예 이름을 변경할 수도 있을 것입니다. 계속해서 얘기하는 것처럼 interface를 사용하게 되면 코드의 가독성, 유지보수성, 생산성이 향상됩니다.
- `Concete` class의 내부구현을 수정할 때 coupling에 의한 영향이 줄어듭니다. Inteface만 그대로 유지한다면 기존의 코드들은 이러한 변화로부터 안전하게 보호될 것입니다.

위와 같은 방법이 어느정도의 중복된 코드를 만들기 때문에 부정적인 시각도 존재합니다. 물론, 이러한 걱정도 충분히 이해가 갑니다. 다만, 아래 2가지 이유를 읽어보면 걱정은 사라질 것입니다.

- 다양한 프로젝트에서 `Concrete` class를 가져다가 사용한다고 해봅시다. 각각의 프로젝트는 `Concrete` class를 사용하기 위해 기존 소스코드를 필연적으로 수정하게 됩니다. 이렇게 `Concrete` class를 사용하기 위해 원래 코드를 수정하는 것은 domain-specific한 interface를 정의하는 것과 사실상 다르지 않습니다. 오히려 수정량의 총합은 더 많을 수도 있습니다.
- 여러개의 프로젝트가 동일한 interface를 사용하고자 할 때는 언제든 공유할 수 있습니다. `Concrete` class를 공유하는 것과 하나도 다르지 않습니다. Interface나 adatptor를 만들어서 `Concrete` class 근처에 저장하고 여러 프로젝트들이 사용하도록 하기만 하면 됩니다.

어떤 방법이든 찬성과 반대의견을 잘 따져보는 것은 중요합니다. 다만, 여기서 설명한 interface를 사용하는 기술은 다양한 상황에서 적용가능하며 Java community에서 긴 시간에 걸쳐 효율성이 입증된 방법임을 말씀드립니다.

#### Fake Class에 위임하기 ####

어떤 interface를 위한 fake class를 구현해서 테스트를 비롯한 다양한 용도로 사용하는 개발자들이 많이 있습니다. 이렇게 이미 fake class를 잘 사용하고 있는 경우에 굳이 mock class를 새로 만들 필요가 있을까요? 사실 정답은 없으며 상황에 따라 사용자가 선택해야 합니다. 다만, gMock의 좋은 점은 mock class를 통해서 기존에 사용하던 fake class도 사용할 수 있다는 것입니다. 즉, mock class는 fake class를 포함할 수 있습니다.

```cpp
class Foo {
 public:
  virtual ~Foo() {}
  virtual char DoThis(int n) = 0;
  virtual void DoThat(const char* s, int* p) = 0;
};

class FakeFoo : public Foo {
 public:
  char DoThis(int n) override {
    return (n > 0) ? '+' :
           (n < 0) ? '-' : '0';
  }

  void DoThat(const char* s, int* p) override {
    *p = strlen(s);
  }
};
```

`Foo`라는 interface에 대해 mock class를 만들고 expectation을 설정하려 합니다. 이 때, 이미 사용하고 있던 `FakeFoo` class를 재사용하고 싶다면 어떻게 하면 될까요? 바로 mock function의 기본 동작으로 fake function을 지정하면 됩니다. 이를 통해 fake function과 동일한 내용을 mock class에 다시 구현하게 되는 중복작업를 피할 수 있습니다. 결과적으로 mock function의 동작은 이미 구현되어 있는 fake function을 그대로 사용하고 expectation만 지정하면 됩니다.

이렇게 gMock을 사용해 fake function을 default action으로 지정하고 싶을 경우, 아래 패턴을 사용하기 바랍니다.

```cpp
class MockFoo : public Foo {
 public:
  // Normal mock method definitions using gMock.
  MOCK_METHOD(char, DoThis, (int n), (override));
  MOCK_METHOD(void, DoThat, (const char* s, int* p), (override));

  // Delegates the default actions of the methods to a FakeFoo object.
  // This must be called *before* the custom ON_CALL() statements.
  void DelegateToFake() {
    ON_CALL(*this, DoThis).WillByDefault([this](int n) {
      return fake_.DoThis(n);
    });
    ON_CALL(*this, DoThat).WillByDefault([this](const char* s, int* p) {
      fake_.DoThat(s, p);
    });
  }

 private:
  FakeFoo fake_;  // Keeps an instance of the fake in the mock.
};
```

위의 코드를 보면 `MockFoo` class를 만드는 방법은 일반적인 mock class 생성방법과 다르지 않습니다. 다만, `DoThis()`, `DoThat()` 함수의 기본동작을 `FakeFoo` object로 위임하는 부분이 추가되었습니다.

`MockFoo` class를 실제로 사용하는 예제는 아래에 있습니다. `TEST()`의 시작 시점에 `foo.DelegateToFake()`를 호출함으로서 `FooFake`에 정의된 fake function들을 기본 action으로 사용하게 됩니다.

```cpp
using ::testing::_;

TEST(AbcTest, Xyz) {
  MockFoo foo;

  foo.DelegateToFake();  // Enables the fake for delegation.

  // Put your ON_CALL(foo, ...)s here, if any.

  // No action specified, meaning to use the default action.
  EXPECT_CALL(foo, DoThis(5));
  EXPECT_CALL(foo, DoThat(_, _));

  int n = 0;
  EXPECT_EQ('+', foo.DoThis(5));  // FakeFoo::DoThis() is invoked.
  foo.DoThat("Hi", &n);  // FakeFoo::DoThat() is invoked.
  EXPECT_EQ(2, n);
}
```

**팁 공유:**

  * 원한다면 얼마든지 mock function의 동작을 변경할 수 있습니다. 예제에서는 `TEST()`의 시작부분에서 `foo.DelegateToFake()`를 호출함으로써 fake function들을 default action으로 지정하긴 했지만, 굳이 fake function을 사용하지 않겠다면 `ON_CALL()` 또는 `EXPECT_CALL()`를 사용해서 자유롭게 변경하면 되는 것입니다.
  * `DelegateToFake()` 함수에서 굳이 모든 fake 함수를 지정할 필요는 없고 원하는 것만 선택적으로 지정해도 됩니다.
  * 여기서 논의된 방법들은 overloaded method에도 적용될 수 있습니다. 물론 compiler에 어떤 것을 사용할지는 알려줘야 합니다. 먼저, mock function에 대한 모호성 해결방법은 이 문서의 [Selecting Between Overloaded Functions](cook_book.md#여러개의-overloaded-function-중에서-선택하기) 부분을 참고하세요. 다음으로 fake function에 대한 모호성 해결방법은 `static_cast`를 사용하면 됩니다. 예를 들어 `Foo` class에 `char DoThis(int n)`, `bool DoThis(double x) const` 라는 2개의 overloaded function이 있다고 합시다. 이 상황에서 후자인 `bool DoThis(double x) const`를 invoke 대상으로 지정하고 싶다면 `Invoke(&fake_, static_cast<bool (FakeFoo::*)(double) const>(&FakeFoo::DoThis)`와 같이 사용하면 됩니다. `static_cast`를 사용해 fuction pointer type을 명확하게 지정한 것입니다.
  * 지금까지 mock, fake를 함께 사용하기 위한 방법을 설명했습니다. 그러나 사실 mock, fake를 함께 사용하는 경우가 발생했다면 해당 소프트웨어의 설계가 뭔가 잘못되었음을 의미하는 것이기도 합니다. 아마도 사용자가 interaction-based testing 방법론에 아직 익숙하지 않은 경우일 수 있습니다. 또는 대상 interface가 너무 많은 역할을 수행하고 있기 때문에 분리될 필요가 있을 수도 있습니다. 다시 말해서 **이 방법을 남용하지 않기를 바랍니다**. Mock, fake가 혼용되어야 한다면 설계를 먼저 리뷰해보는 것이 좋고, 이 방법은 어디까지나 refactoring을 위한 하나의 과정으로 생각하면 좋을 것 같습니다.

Mock, fake가 함께 나타날 때, bad sign을 찾아내는 예제를 하나 공유하겠습니다. 예를 들어 `System` class라는 low-level system operation을 수행하는 class가 하나 있다고 합시다. 이 class는 파일처리와 I/O라는 2가지 기능에 특화되었다고 하겠습니다. 이 때, `System` class의 I/O기능만 테스트하고자 한다면 파일처리는 문제 없이 기본적인 동작만 해주면 될 것입니다. 그런데 I/O기능을 테스트하기 위해서 `System` class를 mock class로 만들게 되면 파일처리 기능까지 mocking이 되어 버립니다. 이렇게 되면 개발자가 fake class를 제공하던지 해서 파일처리 기능과 동일한 역할을 구현해줘야 하는 어려움이 발생합니다.

위의 상황은 `System` class가 너무 많은 역할을 가지고 있음을 보여주는 예입니다. 이런 경우에는 `System` class의 역할을 분리해서 `FileOps`, `IOOps`라는 interface 2개로 분리하는 것도 좋은 방법입니다. 그렇게 되면 분리된 `IOOps` interface만 mocking하여 테스트할 수 있습니다.

#### Real Class에 위임하기 ####

테스트를 위해 testing double(mock, fake, stub, spy 등)을 사용할 떄의 문제는 동작이 real object와 달라질 수 있다는 것입니다. 물론 negative test를 위해서 일부러 다르게 만들기도 하지만, 때로는 실수로 인해 동작이 달라지기도 합니다. 후자의 경우라면 bug를 잡아내지 못하고 제품을 출시하는 것과 같은 문제가 발생하기도 합니다.

이런 경우를 위해 *delegating-to-real*이라고 불리는 기술을 사용할 수 있습니다. 이는 mock object가 real object와 동일하게 동작하게 만들어 주는 방법입니다. 사용법 자체는 바로 위에서 다룬 [delegating-to-fake](cook_book.md#fake-class에-위임하기)와 매우 유사합니다. 다른 점이라면 fake object 대신 real object로 변경된 것 뿐입니다. 아래 예제를 확인하세요.

```cpp
using ::testing::AtLeast;

class MockFoo : public Foo {
 public:
  MockFoo() {
    // By default, all calls are delegated to the real object.
    ON_CALL(*this, DoThis).WillByDefault([this](int n) {
      return real_.DoThis(n);
    });
    ON_CALL(*this, DoThat).WillByDefault([this](const char* s, int* p) {
      real_.DoThat(s, p);
    });
    ...
  }
  MOCK_METHOD(char, DoThis, ...);
  MOCK_METHOD(void, DoThat, ...);
  ...
 private:
  Foo real_;
};

...
  MockFoo mock;
  EXPECT_CALL(mock, DoThis())
      .Times(3);
  EXPECT_CALL(mock, DoThat("Hi"))
      .Times(AtLeast(1));
  ... use mock in test ...
```

이제 gMock을 통해 real object를 사용한 검증도 가능하게 되었습니다. 사실 real object를 사용하는 것은 독립적이고 빨라야 된다는 단위테스트의 특성과는 조금 거리가 있기도 합니다. 그러나 테스트코드를 통해서 real object와의 interaction도 확인할 수 있다는 점은 상황에 따라 매우 유용하게 사용할 수 있는 옵션이 될 것입니다.

#### Base Class에 위임하기 ####

이상적인 경우 interface(base class)는 pure virtual method만 가지는 것이 좋습니다. 다만, C++에서 interface의 method가 구현부를 가지는 것도 문법상 문제가 없습니다. 아래 예제를 보겠습니다.

```cpp
class Foo {
 public:
  virtual ~Foo();

  virtual void Pure(int n) = 0;
  virtual int Concrete(const char* str) { ... }
};

class MockFoo : public Foo {
 public:
  // Mocking a pure method.
  MOCK_METHOD(void, Pure, (int n), (override));
  // Mocking a concrete method.  Foo::Concrete() is shadowed.
  MOCK_METHOD(int, Concrete, (const char* str), (override));
};
```

사용자가 `MockFoo`라는 mock class를 만들기는 했지만 `Foo` interface에 있는 구현을 그대로 사용하고 싶은 method가 있을 수도 있습니다. 또는, 원래부터 특정 method만 mocking하려고 했고 나머지 method는 관심대상이 아니었을 수도 있습니다. 예제를 보면 `Foo::Concrete()`가 실제 구현부를 가지고 있습니다. 우리는 이 `Foo::Concrete()`가 구현부도 있고 관심대상도 아니라고 가정하겠습니다. 새롭게 action을 지정하지 않고 원래대로 동작하게 하고 싶다면 어떻게 해야 할까요? Interface에 있는 `Foo:Concrete()`의 구현부를 `MockFoo::Concrete`에다가 그대로 복사/붙여넣기 해야 할까요? 네, 그렇게하면 중복코드를 만들게 되므로 좋지 않습니다. 이런 경우에도 역시 위임하는 것이 좋습니다.

C++에서 dervied class가 base class에 접근하기 위한 방법은 `Foo::Concrete()`와 같이 범위지정 연산자( `::` )를 사용하는 것입니다. 아래 예제를 보겠습니다.

```cpp
class MockFoo : public Foo {
 public:
  // Mocking a pure method.
  MOCK_METHOD(void, Pure, (int n), (override));
  // Mocking a concrete method.  Foo::Concrete() is shadowed.
  MOCK_METHOD(int, Concrete, (const char* str), (override));

  // Use this to call Concrete() defined in Foo.
  int FooConcrete(const char* str) { return Foo::Concrete(str); }
};
```

이제 `EXPECT_CALL`을 사용해서 `Concrete()`에 대한 호출이 `FooConcrete()`를 호출하도록 지정하면 끝입니다.

```cpp
...
  EXPECT_CALL(foo, Concrete).WillOnce([&foo](const char* str) {
    return foo.FooConcrete(str);
  });
```

아니면 `ON_CALL()`을 사용해서 default action으로 지정하는 것도 괜찮습니다.

```cpp
...
  ON_CALL(foo, Concrete).WillByDefault([&foo](const char* str) {
    return foo.FooConcrete(str);
  });
```

그런데 왜 action을 지정할 때 `{ return foo.Concrete(str); }`과 같이 구현해서 base class의 method를 직접 호출하지 않고 한 단계를 거치도록 구현해야 할까요? 왜냐하면 base class의 `Concrete()`가 virtual function이기 때문에 이것을 직접 호출하면 derived class인 MockFoo의 `Concrete()`가 호출될 것이기 때문입니다. 게다가 derived class의 `MockFoo:Concrete`는 다시 base class의 `Foo::Concrete` 를 호출하고 있기 때문에 이러한 과정이 무한반복되는 문제가 발생할 것입니다.

### Matcher 사용하기 ###

#### 전달된 argument가 기대한 값과 일치하는지 확인하기 ####

Matcher를 통해서 mock method로 전달되는 argument가 정확히 어떤 값이기를 기대한다고 명세할 수 있습니다. 예를 들면, 아래에서 `DoThis()`로는 `5`가 전달되어야 하고, `DoThat()`은 `"Hello"`와 `bar`가 전달되기를 기대합니다.

```cpp
using ::testing::Return;
...
  EXPECT_CALL(foo, DoThis(5))
      .WillOnce(Return('a'));
  EXPECT_CALL(foo, DoThat("Hello", bar));
```

#### 간단한 Matcher 사용하기 ####

Matcher를 통해서 전달되는 argument가 특정한 범위에 포함되어야 한다고 명세하는 것도 가능합니다. 아래에서 `DoThis()`는 `5`보다 큰 값이 전달되기를 기대하고, `DoThat()`은 첫번째 argument로 `"Hello"`, 그리고 두번째 argument로는 `Null`이 아닌 값을 기대한다고 명세하고 있습니다.

```cpp
using ::testing::Ge;
using ::testing::NotNull;
using ::testing::Return;
...
  EXPECT_CALL(foo, DoThis(Ge(5)))  // The argument must be >= 5.
      .WillOnce(Return('a'));
  EXPECT_CALL(foo, DoThat("Hello", NotNull()));
      // The second argument must not be NULL.
```

또한, 어떤 값이라도 허용한다는 의미를 가지는 `_`는 자주 사용하는 matcher 중에 하나 입니다.

```cpp
EXPECT_CALL(foo, DoThat(_, NotNull()));
```

#### Matcher 조합하기 ####

`AllOf()`, `AnyOf()`, `Not()`, `AnyOfArray()`, `Not()` 을 이용하면 여러가지 matcher들을 조합할 수 있습니다. 각각의 역할은 이름과 동일합니다.

```cpp
using ::testing::AllOf;
using ::testing::Gt;
using ::testing::HasSubstr;
using ::testing::Ne;
using ::testing::Not;
...
  // The argument must be > 5 and != 10.
  EXPECT_CALL(foo, DoThis(AllOf(Gt(5),
                                Ne(10))));

  // The first argument must not contain sub-string "blah".
  EXPECT_CALL(foo, DoThat(Not(HasSubstr("blah")),
                          NULL));
```

#### Casting Matchers ####

gMock의 matcher는 정적으로 타입을 결정합니다. 즉, compile time에 타입을 검사해서 잘못된 부분을 알려준다는 의미입니다. 예를 들어 `Eq(5)`인 곳에 `string`을 전달하면 compile error가 발생하게 됩니다.

단, 이러한 타입검사가 완벽하게 철저하다고는 할 수 없습니다. 만약에 `long`을 위한 matcher에 `int`가 전달되면 어떻게 될까요? 네, 잘 동작합니다. 왜냐하면 `int`가 `Matcher<long>`을 만나면 `long`으로 먼저 변환된 후에 matcher에 전달되기 때문입니다. 즉, 암시적으로 타입변환이 일어납니다.

이러한 암시적 변환을 원하지 않는 경우에는, gMock의 `SafeMacherCast<T>(m)`를 사용할 수 있습니다. 이 기능은 `m`이라는 matcher를 받아서 `Matcher<T>` 타입으로 변환해 주는데 이 때 안전한 타입변환을 보장하기 위해서 gMock은 아래 내용들을 검사합니다.(matcher가 전달받은 타입을 `U`라고 가정합니다)

1. 타입 `T`가 타입 `U`로 암시적으로 변환 가능한지
2. `T`와 `U`가 모두 산술타입(`bool`, `int`, `float` 등)일 때, `T` 에서 `U`로 변하는 과정에 손실이 없는지
3. `U`가 참조타입일 때, `T`도 참조타입인지

위 조건들 중 하나라도 만족하지 않으면 compile error가 발생할 것입니다.

아래 예제를 확인하세요.

```cpp
using ::testing::SafeMatcherCast;

// A base class and a child class.
class Base { ... };
class Derived : public Base { ... };

class MockFoo : public Foo {
 public:
  MOCK_METHOD(void, DoThis, (Derived* derived), (override));
};

...
  MockFoo foo;
  // m is a Matcher<Base*> we got from somewhere.
  EXPECT_CALL(foo, DoThis(SafeMatcherCast<Derived*>(m)));
```

`SafeMatcherCast<T>(m)`의 타입검사가 너무 엄격해서 사용하기 어려운 경우에는 이와 유사한 `MatcherCast<T>(m)`을 사용할 수도 있습니다. 차이점이라면 `MatcherCast`는 `T`에서 `U`로의 `static_cast`가 가능한 경우라면 matcher의 타입변환도 허용해준다는 점입니다. 즉, 타입검사가 약간 더 느슨하다고 할 수 있습니다.

`MatcherCast`는 자동적으로 C++의 타입검사 기능(`static_cast` 등)를 사용하게 합니다. 다만, `static_cast`와 같은 C++ 타입검사는 변환과정에서 정보를 없애버리기도 하기 때문에 남용/오용하지 않도록 주의해야 합니다.

#### 여러개의 Overloaded Function 중에서 선택하기 ####

Overloaded function 중에서 하나를 선택하고 싶은 경우가 있을 것입니다. 이 때에는 모호성을 없애기 위해서 compiler에게 관련 내용을 알려줄 필요가 있습니다.

먼저, const-ness overloading에 대한 모호성 제거방법은 `Const()`를 사용하는 것입니다. gMock에서 `Const(x)`는 `x`의 `const` 참조를 반환하게 되어 있습니다. 사용법은 아래 예제를 참조하십시오.

```cpp
using ::testing::ReturnRef;

class MockFoo : public Foo {
  ...
  MOCK_METHOD(Bar&, GetBar, (), (override));
  MOCK_METHOD(const Bar&, GetBar, (), (const, override));
};

...
  MockFoo foo;
  Bar bar1, bar2;
  EXPECT_CALL(foo, GetBar())         // The non-const GetBar().
      .WillOnce(ReturnRef(bar1));
  EXPECT_CALL(Const(foo), GetBar())  // The const GetBar().
      .WillOnce(ReturnRef(bar2));
```

다음으로 argument의 개수는 같고 타입만 다를 때의 모호성 제거 방법입니다. 이를 위해서는 matcher에 정확한 타입을 기입하는 것이 필요합니다. `Matcher<type>()`을 사용해서 matcher를 감싸거나 `TypeEq<type>`, `An<Type>()`과 같이 타입을 미리 지정하는 matcher를 사용하면 됩니다.

```cpp
using ::testing::An;
using ::testing::Lt;
using ::testing::Matcher;
using ::testing::TypedEq;

class MockPrinter : public Printer {
 public:
  MOCK_METHOD(void, Print, (int n), (override));
  MOCK_METHOD(void, Print, (char c), (override));
};

TEST(PrinterTest, Print) {
  MockPrinter printer;

  EXPECT_CALL(printer, Print(An<int>()));            // void Print(int);
  EXPECT_CALL(printer, Print(Matcher<int>(Lt(5))));  // void Print(int);
  EXPECT_CALL(printer, Print(TypedEq<char>('a')));   // void Print(char);

  printer.Print(3);
  printer.Print(6);
  printer.Print('a');
}
```

#### Argument에 따라 다른 Action을 실행하기 ####

Mock method가 호출되면, 소스코드 상에서 제일 *밑에 있는* expectation부터 탐색합니다.(물론 해당 expectation은 active 상태여야 합니다.) 이러한 동작방식은 [여기](for_dummies.md#여러개의-expectations-사용하기) 에서 설명된 적이 있습니다. 이러한 규칙을 이용하면 argument로 전달되는 값에 따라 다른 action이 수행되도록 지정하는 것도 가능합니다.

```cpp
using ::testing::_;
using ::testing::Lt;
using ::testing::Return;
...
  // The default case.
  EXPECT_CALL(foo, DoThis(_))
      .WillRepeatedly(Return('b'));
  // The more specific case.
  EXPECT_CALL(foo, DoThis(Lt(5)))
      .WillRepeatedly(Return('a'));
```

위 코드의 `foo.DoThis()`는 전달된 argument가 `5` 미만이라면 `a`를 반환하고, 그 외의 경우에는 `b`를 반환할 것입니다.

#### Argument 간의 관계 비교하기 ####

지금까지는 argument 하나하나에 대한 matcher 사용법을 주로 다뤘습니다. 그럼 "첫번째 argument가 두번째 argument보다 작아야 된다."와 같이 argument 간의 관계를 비교하고 싶은 경우에는 어떻게 해야할까요? 이런 경우에는 `With()` 를 사용하면 원하는 조건을 비교할 수 있습니다.

```cpp
using ::testing::_;
using ::testing::Ne;
using ::testing::Lt;
...
  EXPECT_CALL(foo, InRange(Ne(0), _))
      .With(Lt());
```

위의 코드는 `InRange()`의 첫번째 argument가 `0`이 아니면서 두번째 argument보다 작아야함을 의미합니다.

`With(m)` 내부의 `m`은 `Matcher<::testing::tuple<A1, ..., An>>`타입이며 여기서 `A1`, ..., `An`은 function argument들을 의미합니다.

`With()` 내부에 `m` 대신에 `AllArgs(m)`이라고 구현하는 것도 가능합니다. 물론, 동작자체는 동일하지만 `.With(Lt())`보다 `.With(AllArgs(Lt()))`이 더 명시적으로 표현하는 방법입니다.

`Args<k1, ..., kn>(m)`을 사용하면 비교하고 싶은 argument만 골라서(tuple형태로) 비교할 수도 있습니다. 아래 예제를 보겠습니다.

```cpp
using ::testing::_;
using ::testing::AllOf;
using ::testing::Args;
using ::testing::Lt;
...
  EXPECT_CALL(foo, Blah)
      .With(AllOf(Args<0, 1>(Lt()), Args<1, 2>(Lt())));
```

위 코드는 `Blah()`가 argument 3개(`x`, `y`, `z`)를 전달받았을 때, `x < y < z`를 만족해야 함을 의미합니다.

`With`를 쉽게 사용하기 위해서 gMock의 일부 matcher들은 2-tuples 타입을 지원하고 있습니다. 위 예제의 `Lt()`도 그 중 하나입니다. 2-tuples를 지원하는 matcher의 전체목록은 [여기](cheat_sheet.md#multi-argument-matchers)를 참조하시기 바랍니다.

만약 `.With(Args<0, 1>(Truly(&MyPredicate)))`와 같이 직접 만든 predicate를 사용하고 싶은 경우에는 해당 predicate의 argument가 `::testing::tuple` 타입으로 선언되어 있어야만 합니다. 왜냐하면 gMock이 `Args<>`를 통해 선택된 argument들을 1개의 tuple 형태로 변환해서 predicate으로 전달하기 때문입니다.

#### Matcher를 Predicate처럼 사용하기 ####

이미 눈치챘겠지만 사실 matcher는 predicate의 확장판(출력문이 잘 정의된)이라고 볼 수 있습니다. 현재 C++ STL의 `<algorithm>`을 포함해서 많은 algorithm들이 predicate를 argument로 받을 수 있게 구현되어 있습니다. 따라서 predicate의 확장판인 matcher도 그러한 내용을 지원하는 것이 맞을 것입니다.

다행히도 matcher로 unary predicate functor를 대체할 수 있습니다. 아래 예제와 같이 `Matches()`를 이용해 matcher를 감싸주기만 하면 됩니다.

```cpp
#include <algorithm>
#include <vector>

using ::testing::Matches;
using ::testing::Ge;

vector<int> v;
...
// How many elements in v are >= 10?
const int count = count_if(v.begin(), v.end(), Matches(Ge(10)));
```

gMock에서는 matcher 여러개를 조합하는 것이 쉽게 가능합니다. 예를 들어 `x>=0, x<=100, x!=50`를 조합하는 방법도 아래처럼 간단합니다. 동일한 작업을 C++ STL의 `<functional>`을 통해 구현하려면 상당히 어려운 작업이 될 것입니다.

```cpp
using testing::AllOf;
using testing::Ge;
using testing::Le;
using testing::Matches;
using testing::Ne;
...
Matches(AllOf(Ge(0), Le(100), Ne(50)))
```

#### Matcher를 googletest Assertion처럼 사용하기 ####

Matcher는 기본적으로 predicate와 동일하고 상응하는 출력문도 잘 정의되어 있습니다. 따라서 matcher를 googletest assertion처럼 사용할 수 있다면 매우 유용할 것입니다. gMock의 `ASSERT_THAT`, `EXPECT_THAT`은 이러한 기능을 제공합니다.

```cpp
  ASSERT_THAT(value, matcher);  // Asserts that value matches matcher.
  EXPECT_THAT(value, matcher);  // The non-fatal version.
```

예를 들어, googletest에서 아래와 같이 사용할 수 있습니다.

```cpp
#include "gmock/gmock.h"

using ::testing::AllOf;
using ::testing::Ge;
using ::testing::Le;
using ::testing::MatchesRegex;
using ::testing::StartsWith;

...
  EXPECT_THAT(Foo(), StartsWith("Hello"));
  EXPECT_THAT(Bar(), MatchesRegex("Line \\d+"));
  ASSERT_THAT(Baz(), AllOf(Ge(5), Le(10)));
```

코드에서 예측가능한 것처럼 `Foo()`, `Bar()`, `Baz()`는 아래 내용들을 검증하게 됩니다.

- `Foo()`가 `"Hello"`로 시작하는 문자열을 반환하는지 검증합니다.
- `Bar()`가 정규식 `"Line \\d+"`과 매칭되는 값을 반환하는지 검증합니다.
- `Baz()`가 [5,10] 범위내의 값을 반환하는지 검증합니다.

이러한 macro의 좋은 점은 소스코드가 *말(영어)처럼 자연스럽게 읽혀진다*는 것입니다. 또한, 동일한 방법으로 failure message도 이해하기 쉽게 출력됩니다. 예를 들어 `Foo()`의 `EXPECT_THAT()`이 실패하면 아래와 같은 message가 출력됩니다.

```bash
Value of: Foo()
  Actual: "Hi, world!"
Expected: starts with "Hello"
```

**Credit:** `(ASSERT|EXPECT)_THAT`의 아이디어는 `assertThat()`을 `JUnit`에 추가했던 [Hamcrest](https://github.com/hamcrest/) 프로젝트에서 가져왔음을 밝힙니다.

#### Predicate를 Matcher처럼 사용하기 ####

gMock은 다양한 built-in matcher들을 제공하고 있습니다. 그럼에도 사용자가 필요로 하는 모든 경우를 만족하지는 못할 것입니다. 이처럼 새로운 matcher가 필요한 경우에는 unary predicate function 또는 functor를 matcher처럼 사용하는 것도 가능합니다. 이를 위해서는 `Truly()`로 predicate를 감싸기만 하면 됩니다.

```cpp
using ::testing::Truly;

int IsEven(int n) { return (n % 2) == 0 ? 1 : 0; }
...
  // Bar() must be called with an even number.
  EXPECT_CALL(foo, Bar(Truly(IsEven)));
```

또한, predicate function(functor)이 꼭 `bool` 타입을 반환하지 않아도 됩니다. 반환타입은 조건문 `if (condition)`에 사용할 수 있는 타입이기만 하면 됩니다.

#### 복사가 안되는 argument를 비교하기 ####

사용자가 `EXPECT_CALL(mock_obj, Foo(bar))`라고 구현했다면, gMock은 `Foo()`의 argument로 전달된 `bar`의 복사본을 만들고 내부적으로 저장해 둡니다. 그랬다가 추후에 `Foo()`가 실제로 호출되면 미리 저장해둔 `bar`의 복사본을 사용하는 것입니다. 이렇게 동작하는 이유는 `EXPECT_CALL()`이 호출되고 난 후에 `bar`의 값이 변경되는 문제를 방지하기 위한 것입니다. `Eq(bar)`, `Le(bar)`와 같은 다른 matcher를 사용하는 경우에도 동일한 방식이 적용됩니다.

그런데 여기서 `bar`의 복사가 불가능하다면 어떻게 해야 할까요?(예를 들어 copy constructor가 없다거나) 첫번째 해결방법은 자체적으로 matcher function을 구현하여 `Truly()`와 함께 사용하는 것입니다. 두번째 해결방법은 `EXPECT_CALL()`이 호출된 이후에는 `bar`가 수정되지 않을 것임을 gMock에게 알려주는 것입니다. 그렇게 되면 gMock은 `bar`의 복사본 대신에 참조(주소)를 저장하게 됩니다. 아래와 같이 하면 됩니다.

```cpp
using ::testing::ByRef;
using ::testing::Eq;
using ::testing::Lt;
...
  // Expects that Foo()'s argument == bar.
  EXPECT_CALL(mock_obj, Foo(Eq(ByRef(bar))));

  // Expects that Foo()'s argument < bar.
  EXPECT_CALL(mock_obj, Foo(Lt(ByRef(bar))));
```

Remember: 위와 같이 참조형태(`ByRef`)로 전달한 후에 `bar`를 수정하는 것은 미정의 동작이므로 위험합니다.

#### Object Member 비교하기 ####

Mock function의 argument로 object가 전달되면 어떻게 해야 할까요? Object로 전달되는 argument를 비교하기 위해 object 전체를 비교하는 것은 좋은 방법이 아닐 것입니다. 그럼 특정한 member variable만 비교하려면 어떻게 해야 할까요? 이런 경우에는 `Field()` 또는 `Property()`를 사용하세요. 사용방법은 아래와 같습니다.

```cpp
Field(&Foo::bar, m)
```

`Field()`는 `Foo` 객체의 `bar`라는 member variable이 matcher `m`을 만족하는지 비교합니다.

```cpp
Property(&Foo::baz, m)
```

`Property()`는 `Foo` 객체의 `baz()`라는 method의 반환값이 matcher `m`을 만족하는지 비교합니다.

아래는 예제입니다.

| Expression                                  | Description                                         |
| :------------------------------------------ | :-------------------------------------------------- |
| `Field(&Foo::number, Ge(3))`                | Matches `x` where `x.number >= 3`.                  |
| `Property(&Foo::name, StartsWith("John "))` | Matches `x` where `x.name()` starts with `"John "`. |

유의할 점은 `Property(&Foo::baz, ...)`에서 사용한 `baz()`는 argument가 없는 `const` function으로 선언되어야 한다는 점입니다.

더불어 `Field()`와 `Property()`는 object를 가리키는 포인터도 비교할 수 있습니다. 아래 예제를 보겠습니다.

```cpp
using ::testing::Field;
using ::testing::Ge;
...
Field(&Foo::number, Ge(3))
```

위 코드는 `p`라는 포인터가 전달되면 `p->number >= 3`이라는 비교를 수행하게 됩니다.(`p`가 `NULL`이라면 항상 실패하게 됩니다.)

만약, 1개 이상의 member variable을 비교하고 싶다면 어떻게 하면 될까요? 기존처럼 `AllOf()`를 사용하면 됩니다.

#### 포인터가 가리키는 값을 비교하기 ####

C++에서는 포인터를 argument로 사용하는 것도 당연히 가능합니다. 이를 위해서 gMock은 `IsNull()`, `NotNull()`과 같은 포인터용 matcher들을 다수 제공하고 있습니다. 그럼 포인터 자체를 비교하는 것이 아니라 포인터가 가리키는 값을 비교하려면 어떻게 하면 될까요? 그런 경우에는 `Pointee(m)` matcher를 사용하면 됩니다.

아래 예제의 `Pointee(m)`은 포인터가 가리키는 값을 비교하는데 사용합니다.

```cpp
using ::testing::Ge;
using ::testing::Pointee;
...
  EXPECT_CALL(foo, Bar(Pointee(Ge(3))));
```

위 코드의 의미는 `foo.Bar()`가 포인터를 argument로 전달받는데 그 포인터가 가리키는 값이 3보다 크거나 같기를 기대한다는 뜻입니다.

한 가지 좋은 점은 `Pointee()`는 `NULL` 포인터를 실패로 간주한다는 것입니다. 따라서 굳이 아래처럼 쓰지는 않아도 됩니다.

```cpp
using ::testing::AllOf;
using ::testing::NotNull;
using ::testing::Pointee;
...
  AllOf(NotNull(), Pointee(m))
```

이처럼 `NULL`을 자동으로 확인해주기 때문에 `NULL` 포인터가 테스트 프로그램을 비정상 종료시키는 문제는 염려하지 않아도 됩니다.

또한, `Pointee()`는 raw pointer뿐만 아니라 smart pointer(`linked_ptr`, `shared_ptr`, `scoped_ptr` 등)를 사용하는 경우에도 잘 동작합니다.

그럼 포인터를 가리키는 포인터를 어떨까요? 추측해 보십시오. 네, `Pointee()`는 여러번 중첩해서 사용할 수 있습니다. 예를 들어 `Pointee(Pointee(Lt(3)))`라는 코드의 의미는 포인터가 가리키는 포인터가 가리키는 값이 3보다 작거나 같은지 비교합니다.

#### Object Property 테스트하기 ####

Argument로 object가 전달되었을 때, 해당 object의 property를 검증하고 싶은 경우가 있을 것입니다. 그러나 그러한 경우를 위한 matcher는 아직 없으므로 필요한 경우에는 직접 정의해야 합니다. 여기서는 이처럼 matcher를 직접 구현해야 할 때 일반 function을 구현하는 것처럼 빠르게 구현하는 방법을 설명합니다.

일단, `Foo` class를 argument로 갖는 mock function이 있다고 가정합니다. 그리고 `Foo`는 `int bar()`와 `int baz()`라는 method를 가지고 있습니다. 이 때, `bar()`, `baz()`라는 method 2개의 반환값을 더한 값(`bar()` + `baz()`)이 기대한 바를 만족하는지 구현하고 싶습니다. 어떻게 하면 될까요? 아래 예제를 참조하세요.

```cpp
using ::testing::Matcher;
using ::testing::MatcherInterface;
using ::testing::MatchResultListener;

class BarPlusBazEqMatcher : public MatcherInterface<const Foo&> {
 public:
  explicit BarPlusBazEqMatcher(int expected_sum)
      : expected_sum_(expected_sum) {}

  bool MatchAndExplain(const Foo& foo,
                       MatchResultListener* /* listener */) const override {
    return (foo.bar() + foo.baz()) == expected_sum_;
  }

  void DescribeTo(::std::ostream* os) const override {
    *os << "bar() + baz() equals " << expected_sum_;
  }

  void DescribeNegationTo(::std::ostream* os) const override {
    *os << "bar() + baz() does not equal " << expected_sum_;
  }
 private:
  const int expected_sum_;
};

Matcher<const Foo&> BarPlusBazEq(int expected_sum) {
  return MakeMatcher(new BarPlusBazEqMatcher(expected_sum));
}

...
  EXPECT_CALL(..., DoThis(BarPlusBazEq(5)))...;
```

#### Container 비교하기 ####

STL container(list, vector, map 등)를 비교하려면 어떻게 해야 할까요? 일단, C++ STL containter 대부분은 `==` 연산자를 제공하기 때문에 간단하게 `Eq(expected_container)`를 사용해도 됩니다. 여기서 `expected_container`는 argument로 전달되기를 바라는 기대값 container입니다.

여기에 더해서 좀 더 유연하게 비교하고 싶을 수도 있습니다. 예를 들면 "첫번째 argument는 완전히 같아야 하고 두번째 argument는 어떤 값이든 괜찮다"와 같은 경우가 있을 수 있습니다. 이러한 비교를 구현할 때, container에 포함된 element 자체가 그렇게 많지 않다면 expected container를 선언하는 것 자체가 조금 귀찮은 일이 되기도 합니다.

gMock은 이런 경우를 위해서 `ElementsAre()` 및 `UnorderedElementsAre()`이라는 matcher를 제공하고 있습니다. 먼저 `ElementAre()`의 사용예제는 아래와 같습니다.

```cpp
using ::testing::_;
using ::testing::ElementsAre;
using ::testing::Gt;
...
  MOCK_METHOD(void, Foo, (const vector<int>& numbers), (override));
...
  EXPECT_CALL(mock, Foo(ElementsAre(1, Gt(0), _, 5)));
```

위의 matcher를 보면 `ElementsAre()`로 전달된 argument가 4개임을 볼 수 있습니다. 이것은 곧 argument로 전달된 container가 4개의 element를 가져야 함을 의미합니다. 다시 각각을 보면 container의 첫번째 element는 `1`과 같아야 하고, 두번째 element는 `0`보다 커야 하고, 세번째 element는 어떤 값이든 괜찮고, 네번째 element는 `5`이어야 함을 의미합니다.

다음으로 `UnorderedElementsAre()`를 사용한 코드는 아래와 같습니다.

```cpp
using ::testing::_;
using ::testing::Gt;
using ::testing::UnorderedElementsAre;
...

  MOCK_METHOD1(Foo, void(const vector<int>& numbers));
...

  EXPECT_CALL(mock, Foo(UnorderedElementsAre(1, Gt(0), _, 5)));
```

위의 matcher도 역시 container가 4개의 element를 가져야 함을 의미합니다. 다만 element들의 순서는 관계가 없고 조건을 만족하는 element 4개가 있는지만 확인합니다.

`ElementsAre()`과 `UnorderedElementsAre()`는 argument의 개수가 0~10개까지만 사용할 수 있도록 overloaded되어 있습니다. 따라서 container의 element가 10개 이상이라면 위와 같은 방법으로는 비교할 수 없습니다. 이 때에는 C-style 배열을 사용해서 expected container(elements들)를 지정하는 것이 좋습니다.

```cpp
using ::testing::ElementsAreArray;
...
  // ElementsAreArray accepts an array of element values.
  const int expected_vector1[] = {1, 5, 2, 4, ...};
  EXPECT_CALL(mock, Foo(ElementsAreArray(expected_vector1)));

  // Or, an array of element matchers.
  Matcher<int> expected_vector2[] = {1, Gt(2), _, 3, ...};
  EXPECT_CALL(mock, Foo(ElementsAreArray(expected_vector2)));
```

배열의 크기가 compile time에 정해지지 않은 상황이라면 배열크기를 같이 전달해야 합니다. 아래 예제에서 `ElementAreArray()`의 두번째 argument로 전달된 `count`가 바로 배열크기를 가리킵니다.

```cpp
using ::testing::ElementsAreArray;
...
  int* const expected_vector3 = new int[count];
  ... fill expected_vector3 with values ...
  EXPECT_CALL(mock, Foo(ElementsAreArray(expected_vector3, count)));
```

**Tips:**

  * `ElementsAre*()`은 STL iterator pattern(`const_iterator` 타입을 제공하고 `begin(), end()`를 지원하는 것)에 부합하는 container라면 *어떤 것*에도 적용할 수 있습니다. 이것은 곧 사용자가 만든 container타입도 STL iterator pattern에 부합한다면 사용할 수 있음을 의미합니다.
  * `ElementsAre*()`을 중첩해서 사용할 수도 있습니다 즉, 중첩된 container에 대해서도 동작합니다.
  * 만약, container가 참조형식이 아니라 포인터로 전달되는 경우라고 해도 `Pointee(ElementsAre*(...))`라고 써주기만 하면 됩니다.
  * `ElementAre*` 계열은 element 간의 *순서가 중요할 때* 사용해야 합니다. 따라서 `hash_map`과 같이 순서가 정해지지 않은 container에 대해서는 사용하면 안됩니다.

#### Matcher 공유하기 ####

gMock의 matcher는 내부적으로 ref-count 방식을 사용하는 포인터로 구현되어 있습니다. 이러한 구조를 통해서 matcher를 복사할 때는 포인터만 복사하면 되므로 매우 빠르게 동작합니다. 또한 ref-count 방식이므로 마지막으로 가리키는 포인터가 사라지면 해당 matcher object도 삭제됩니다.

자, 이제 복잡한 matcher를 하나 구현했다면 이것을 복사해서 재사용하면 됩니다. 코드를 복사할 필요 없이 matcher타입 변수에 matcher를 저장하고 필요할 때마다 가져다 쓰면 됩니다. 아래 예제를 참고하세요.

```cpp
using ::testing::AllOf;
using ::testing::Gt;
using ::testing::Le;
using ::testing::Matcher;
...
  Matcher<int> in_range = AllOf(Gt(5), Le(10));
  ... use in_range as a matcher in multiple EXPECT_CALLs ...
```

#### Matcher는 부수효과(side-effect)를 가지면 안됩니다.

WARNING: gMock은 언제 얼마나 많은 matcher가 실행될지 미리 알지 못합니다. 따라서 모든 matcher는 *순수하게 기능적인 동작*만 해야합니다. 즉, 프로그램 내의 다른 부분을 수정하면 안됩니다. 더불어 결과를 만들어낼 때 프로그램 내의 다른 부분으로부터 영향을 받아서도 안됩니다. 오직 matcher로 전달되는 parameter, argument, 및 내부변수만 가지고 결과를 만들어내야 합니다. Parameter나 argument가 참조 혹은 포인터 타입인 경우에는 수정하지 않도록 주의해야 합니다.

이 내용은 standard matcher이든 custom matcher이든 관계없이 모두 충족해야 하는 요구사항입니다. 같은 맥락에서 matcher는 내부적으로 mock function을 호출해서도 안됩니다. 왜냐하면 mock object와 gMock에 어떠한 수정도 가해서는 안되기 때문입니다.

### Expectation 설정하기 ##

#### ON_CALL, EXPECT_CALL을 구분하고 사용하는 방법 ####

gMock의 `ON_CALL()`은 아마도 자주 사용되는 기능은 아닐 것입니다.

알다시피 mock object의 행위를 지정하는 방법은 2가지가 있습니다. `ON_CALL()`, `EXPECT_CALL()`이 그것입니다. 그럼 2개의 다른점은 무엇일까요? 먼저 `ON_CALL()`의 경우를 보면 `ON_CALL()`은 mock method가 호출되었을 때 어떤 행위를 해야하는지를 지정하는 것은 가능하지만 *expectation*을 지정할 수는 없습니다. 다음으로 `EXPECT_CALL()`은 행위를 지정하는 것에 더해서 expectation을 지정하는 것도 가능합니다. 여기서 expectation을 지정한다는 것의 의미는 예를 들어서 *mock method로 어떤 argument가 전달되어야 하는지, 몇 번 호출되어야 하는지, mock method 간의 호출순서는 어떠한지*와 같은 정보를 테스트를 구현하는 사람이 명시할 수 있다는 의미입니다.

그럼 "`EXPECT_CALL()`이 제공하는 기능이 더 많으므로 `EXPECT_CALL()`이 `ON_CALL()`보다 좋다"라고 하면 될까요? 물론 아닙니다. 왜냐하면 `EXPECT_CALL()`은 테스트 대상의 행위에 어찌됐든 제약을 계속해서 추가하는 것입니다. 여기서 우리는 제약사항이 필요한 것보다 많은 상황이 부족한 상황보다 오히려 더 나쁜 것으로 봐야한다는 점이 중요합니다.

약간 직관에 반하는 이야기이기도 합니다. 왜 더 많이 검증하는 것이 더 적게 검증하는 것보다 나쁜 걸까요?

정답은 테스트가 *무엇을* 검증해야 하는가에 있습니다. **좋은 테스트는 코드의 상호작용을 검증해야 합니다.** 테스트가 너무 과도한 제약사항을 가지게 되면 구현을 자유롭게 할 수 없게 됩니다. 간단히 말해서 과도한 테스트로 인해서 인터페이스(모듈간의 상호작용)를 망가트리지 않음에도 불구하고 refactoring이나 optimization을 자유롭게 할 수 없다면 이게 더 큰 문제가 되는 것입니다. 극단적인 경우에는 어떤 수정사항이 테스트에서 실패하는 것을 제외하고는 아무런 문제가 없는 경우도 발생할 수 있을 것입니다. 게다가 테스트에 실패한 이유를 디버깅하고 해결하기 위해서 의미없는 시간을 허비하게 될 것입니다.

1개의 테스트로 많은 것을 검증하려고 하면 안됩니다. **1개 테스트로는 1개만 검증하는 것이 좋은 습관입니다.** 그렇게 해야만 bug가 발생해도 1~2개의 테스트에서만 문제가 될 것입니다. 그렇지 않고 여러개의 테스트가 한 번에 잘못되면 디버깅하기가 훨씬 어려워집니다. 또한, 테스트의 이름을 통해서 무엇을 검증하려 하는지 자세히 표현하는 것도 좋은 습관입니다. 그렇게 해야 log만 보고도 어떤 문제인지 예측할 수 있습니다.

이제부터는 `ON_CALL()`을 먼저 사용하고, 실제로 필요할 때만 `EXPECT_CALL()`을 사용하시기 바랍니다. 예를 들어 test fixture에 여러개의 `ON_CALL()`을 구현할 수도 있습니다. 그렇게 되면 모든 `TEST_F()`가 동일한 설정을 공유하도록 할 수 있습니다. 이렇게 `ON_CALL()`을 통해 기본적인 설정을 공유한 다음에 개별 `TEST_F()`에는 조심스럽게 `EXPECT_CALL()`을 적용하기 바랍니다. 이러한 전략은 각각의 `TEST_F()`에 많은 `EXPECT_CALL()`을 사용하는 것보다 훨씬 유연한 테스트로 만들어 줄 것입니다. 또한, 테스트의 목적도 명확하게 표현할 수 있기 때문에 가독성 및 유지보수성도 향상될 것입니다.

만약, `EXPECT_CALL()`을 사용하는 mock function들이 너무 많은 "Uninteresting mock function call"을 발생시켜서 문제가 된다면 `NiceMock`을 사용하기 바랍니다. 또는 문제가 되는 mock function에 대해 `EXPECT_CALL(....).Times(AnyNumber())`를 사용하는 것도 괜찮습니다. 단지 warning message를 없애기 위한 목적으로 너무 상세한 `EXPECT_CALL(....)`을 작성해서는 안 됩니다. 그렇게 하면 유지보수가 힘들어집니다.

#### Uninteresting Call 무시하기 ####

만약, 어떤 mock method가 호출되는것에 관심이 없다면 별다른 설정을 하지 않으면 됩니다. 그런 상황에서 해당 mock method가 호출되면 gMock은 test program을 계속 진행하기 위한 목적으로 default action을 자동적으로 수행합니다. 만약 이러한 default action이 원하는 방향과 다르다면 default action을 변경할 수도 있습니다. 이를 위해서는 `ON_CALL()`을 이용해도 되고 `DefaultValue<T>::Set()`을 overriding해도 됩니다.(뒤에서 다시 설명합니다.)

이렇게 `ON_CALL()`을 사용한 default action 설정과 대비하여 다시 한 번 기억해야 할 부분은 `EXPECT_CALL()`은 사용하는 순간부터 해당 mock method에 (더 엄격한) expectation을 설정하는 것이며 그러한 expectation이 만족하지 않으면 test failure를 발생한다는 것입니다.

#### Unexpected Call 허용하지 않기 ####

어떤 mock method가 호출되지 않기를 바라는 경우도 있을 것입니다. 이에 대해서는 아래처럼 구현할 수 있습니다.

```cpp
using ::testing::_;
...
  EXPECT_CALL(foo, Bar(_))
      .Times(0);
```

동일한 mock method에 대해서 어떤 방식의 호출은 허용하고, 나머지는 허용하지 않으려면 `EXPECT_CALL()`을  여러개 사용하면 됩니다.

```cpp
using ::testing::AnyNumber;
using ::testing::Gt;
...
  EXPECT_CALL(foo, Bar(5));
  EXPECT_CALL(foo, Bar(Gt(10)))
      .Times(AnyNumber());
```

만약 `foo.Bar()`가 호출되었는데 위의 코드에서 설정한 2개의 `EXPECT_CALL()` 중에서 어느 것도 만족하지 않았다면 해당 테스트는 실패하게 됩니다.

#### Uninteresting vs Unexpected 를 구분하자 ####

gMock에서 *uninteresting* 호출과 *unexpected*호출은 서로 다른 개념입니다. 매우 다릅니다.

먼저, `x.Y(...)`이 **uninteresting**이라는 의미는 `EXPECT_CALL(x, Y(....))`이 하나도 없음을 뜻합니다. 쉽게 말해서 테스트에서 해당 mock function이 어떻게 되든 신경쓰지 않겠다는 의미입니다.

다음으로 `x.Y(...)`이 **unexpected**라는 의미는 해당 함수에 대해 `EXPECT_CALL(x, Y(...))`을 사용하고 있지만, 실제로 `x.Y(...)`가 호출되었을 때, expectation을 만족하는 `EXPECT_CALL(x. Y(...))`을 찾지 못했음을 의미합니다. 즉, 해당 mock function이 기대한 대로 사용되지 않았기 때문에 해당 테스트의 결과는 실패가 됩니다.

**unexpected call이 발생한 테스트는 항상 실패로 판정됩니다.** 왜냐하면 이것은 곧 mock function에 대한 expectation과 실제 코드의 동작이 다름을 의미하기 때문입니다.

반대로 (기본모드naggy에서) **uninteresting call은 테스트 실패를 의미하지 않습니다.** 왜냐하면 테스트에서 해당 mock function의 동작을 정의한 적이 없기 때문입니다. gMock은 아무런 expectation도 설정되지 않은 mock function은 어떻게 수행되더라도 괜찮다고 판단합니다. 대신 언젠가 문제가 *될 수도 있음을* 알리기 위해 warning을 출력해 줍니다. 왜냐하면 예를 들어서 사용자가 테스트를 구현하다가 `EXPECT_CALL()`을 실수로 빠트렸을 수도 있기 때문입니다.

gMock에서 `NiceMock`과 `StrictMock`은 mock class를 "nice" 또는 "strict"로 만들어 줍니다. 이것이 uninteresting call과 unexpected call에 어떤 영향을 미칠까요?

**Nice mock**은 uninteresting call warning을 없애 줍니다. 기본모드에 비해 출력물에 차이는 있지만 테스트 결과 측면에서 달라지는 점은 없습니다. 즉, 기본모드에서 실패라면 nice mock에서도 실패인 것입니다.

**Strict mock**은 uninteresting call warning을 error로 간주합니다. 따라서 기본모드에서 성공하는 테스트라도 실패할 수 있게 됩니다. 즉, 테스트 결과가 달라질 수 있습니다.

아래 예제를 보겠습니다.

```cpp
TEST(...) {
  NiceMock<MockDomainRegistry> mock_registry;
  EXPECT_CALL(mock_registry, GetDomainOwner("google.com"))
          .WillRepeatedly(Return("Larry Page"));

  // Use mock_registry in code under test.
  ... &mock_registry ...
}
```

위의 `EXPECT_CALL()`은 `GetDomainOwner()`의 argument로 `"google.com"`을 전달받기를 기대합니다. 따라서 `GetDomainOnwer("yahoo.com")`라는 내용으로 호출되면 이것은 unexpected call이고 테스트는 실패합니다. 즉, `NiceMock`을 사용했다고 하더라도 unexpected call로 인한 실패는 동일하게 실패라는 것입니다.

그러면 `GetDomainOnwer()`가 어떤 argument를 전달받더라도 테스트가 실패하지 않게 하려면 어떻게 하면 될까요? 이런 경우에는 "catch all" 목적의 `EXPECT_CALL()`을 추가하는 것이 일반적입니다. `_` 와 `AnyNumber()`를 사용해서 구현합니다.

```cpp
  EXPECT_CALL(mock_registry, GetDomainOwner(_))
        .Times(AnyNumber());  // catches all other calls to this method.
  EXPECT_CALL(mock_registry, GetDomainOwner("google.com"))
        .WillRepeatedly(Return("Larry Page"));
```

`_`는 argument로 어떤 값이 전달되더라도 성공하는 wildcard matcher라는 것은 이미 배운 내용입니다. 이와 함께 두번째 `EXPECT_CALL()`에는 `GetDomainOwner("google.com")`도 지정하고 있습니다. 이것이 의미하는 바는 argument에 따라서 `GetDomainOnwer()` 함수의 행위를 다르게 하라는 것입니다.

한가지 주의할 점은 이렇게 동일한 mock function에 대해 여러개의 `EXPECT_CALL()`을 사용하면 소스코드 상에서 나중에 오는 것과 먼저 비교한다는 것입니다. 즉, 위의 코드에서는 `GetDomainOwner()`에 대한 호출이 발생하면 `GetDomainOwner("google.com")`와 먼저 비교해보고 이것을 만족하지 않으면 다음으로 `GetDomainOwner(_)`와의 비교를 수행하게 됩니다.

Uninteresting call, nice mock, strict mock에 대한 더 자세한 내용은 ["The Nice, the Strict, and the Naggy"](cook_book.md#nice-모드-strict-모드-naggy-모드-naggy-잔소리가-심한)를 참조하세요.

#### 함수의 호출순서 지정하기 ####

하나의 mock function에 대해 여러개의 `EXPECT_CALL()`을 사용했을 때, `EXPECT_CALL()`을 비교하는 순서가 있다고 바로 위에서 얘기했습니다. 그러나 이렇게 만족하는 `EXPECT_CALL()`을 탐색하는 과정에 순서가 있다고 해서 해당 mock function이 특정한 호출순서가 가진다고 말할 수는 없습니다. 예를 들어 어떤 mock function에 2개의 `EXPECT_CALL()`을 설정했다면 기대를 만족하는 `EXPECT_CALL()`은 첫번째 일수도 있고, 두번째 일수도 있습니다. 단지 두번째 것을 먼저 비교해보는 것 뿐이죠. 다시 말하면 비교순서는 정해져 있지만 호출순서는 아직 지정하지 않은 것입니다.

그러면 호출순서를 명확히 지정하고 싶다면 어떻게 해야할까요? 이를 위해서는 아래 예제와 같이 해당하는 `EXPECT_CALL()`들을 모아서 새로운 block에 넣고 상단에 `InSequence`라는 타입의 변수를 선언하면 됩니다.

```cpp
using ::testing::_;
using ::testing::InSequence;

  {
    InSequence s;

    EXPECT_CALL(foo, DoThis(5));
    EXPECT_CALL(bar, DoThat(_))
        .Times(2);
    EXPECT_CALL(foo, DoThis(6));
  }
```

위 코드는 `foo.DoThis(5)`가 제일 먼저 호출되고 그 다음에 `foo.DoThat(_)`, `foo.DoThis(6)`이 순서대로 호출되기를 기대하는 코드입니다. 만약 `foo.DoThis()`를 처음 호출할때 argument로 `6`이 전달되면 `DoThis(5)`를 만족하지 않기 때문에 테스트가 실패합니다. 즉, 현재 호출순서에 있는 `EXPECT_CALL()`이 기대를 만족하지 않는다고 해서 기대를 만족하는 다른 `EXPECT_CALL()`이 있는지 찾거나 하지 않습니다. 여기서 한가지 유의할 점은 기존에는 단순히 만족하는 `EXPECT_CALL()`을 탐색할 때는 소스코드상에서 하단에 있는 `EXPECT_CALL()`부터 먼저 비교했지만, `InSequence`를 사용하는 상황에서는 위에서부터 순서대로 진행한다는 점입니다.

#### 함수의 호출순서를 부분부분 지정하기 ####

`InSequence`는 function의 호출순서를 일렬로 지정합니다. 이에 더해서 여러 function에 다양한 순서를 지정하는 것도 가능합니다. 예를 들어 `A`가 `B`와 `C`보다 먼저 호출되기를 바라는 것과 동시에 `B`와 `C`간에는 호출순서를 지정하고 싶지 않을 수도 있습니다. 기존처럼 `InSequence`를 사용하는 것은 원하는 것에 비해 많은 제약을 지정하는 것이기 때문에 적합하지 않습니다.

gMock은 이렇게 부분적인 호출순서를 지원하기 위해서 DAG(directed acyclic graph)를 적용했습니다. 이를 사용하기 위한 몇 가지 방법이 있는데 먼저 `EXPECT_CALL()`에 `After()`(cheat_sheet.md#the-after-clause)를 붙여서 사용하는 것이 한가지 방법입니다.

또 다른 방법은 `InSequence()`를 사용하는 것입니다.(위에서 설명한 `InSequence` class와는 다릅니다.) 이 개념은 jMock 2에서 가져왔습니다. `After()`보다는 유연성이 떨어지긴 하지만 길고 연속된 함수호출을 지정할 때 편리합니다. 왜냐하면 하나의 호출흐름 안에 있는 `EXPECT_CALL()`들은 별도의 이름을 가질 필요가 없기 때문입니다. 아래에 계속해서 `InSequence()`의 동작방식을 설명하겠습니다.

`EXPECT_CALL()`을 graph 자료구조의 node라고 가정해봅시다. 그럼 node A에서 node B로 가는 edge를 추가함으로 DAG가 하나 만들어집니다. 이 때, 해당 DAG의 의미는 A가 B보다 먼저 호출되어야 함을 뜻합니다. 이렇게 DAG에서 직접적으로 연결되는 edge를 "sequence"라고 부릅니다. 여기서 하나의 sequence에는 또 다시 내부적으로 DAG를 구성할 수 있습니다. 이렇게 내부적으로 구성된 DAG는 자신이 가지고 있는 `EXPECT_CALL()`들의 호출순서 정보와 함께 바깥의 DAG를 만드는데 영향을 주는 `EXPECT_CALL()`도 알고 있어야 합니다.

이러한 부분적인 호출순서를 실제로 구현하려면 2가지만 알면 됩니다. 먼저 DAG의 edge(sequence)를 의미하는 `Sequence` object를 정의합니다. 다음으로 DAG의 node(`EXPECT_CALL()`)가 어느 `Sequence`에 속하는지 알려주는 것입니다. 

이제 같은 `Sequence`에 속하는 `EXPECT_CALL()`들은 소스코드상에서 위-아래로, 즉 작성된 순서대로 호출되어야 합니다.

```cpp
using ::testing::Sequence;
...
  Sequence s1, s2;

  EXPECT_CALL(foo, A())
      .InSequence(s1, s2);
  EXPECT_CALL(bar, B())
      .InSequence(s1);
  EXPECT_CALL(bar, C())
      .InSequence(s2);
  EXPECT_CALL(foo, D())
      .InSequence(s2);
```

위의 코드에 대해 DAG를 그려보면 아래와 같습니다. (`s1` 은 `A -> B` / `s2`는 `A -> C -> D`)

```bash
       +---> B
       |
  A ---|
       |
       +---> C ---> D
```

위 DAG는 A가 B,C보다 먼저 호출되어야 함을 의미하고, C가 D보다 먼저 호출되어야 함을 의미합니다. 그 외의 제약은 없습니다.

#### Expectation의 active/inactive 설정하기 ####

어떤 mock method가 호출되어 만족하는 expectation(`EXPECT_CALL()`)을 찾을 때, gMock은 active 상태인 expectation에 대해서만 이러한 작업을 수행합니다. Expectation들이 처음 생성되어 호출되기 전에는 기본적으로 active상태라고 보면 됩니다. 그러다가 자신보다 나중에 호출되어야 할 함수가 호출되는 시점에 inactive 상태가 됩니다. 이렇게 inactive 상태가 되는 것을 *retires*라고 부릅니다.

```cpp
using ::testing::_;
using ::testing::Sequence;
...
  Sequence s1, s2;

  EXPECT_CALL(log, Log(WARNING, _, "File too large."))      // #1
      .Times(AnyNumber())
      .InSequence(s1, s2);
  EXPECT_CALL(log, Log(WARNING, _, "Data set is empty."))   // #2
      .InSequence(s1);
  EXPECT_CALL(log, Log(WARNING, _, "User not found."))      // #3
      .InSequence(s2);
```

위의 코드는 `log.Log()`에 대해 #1, #2, #3 순서로 호출순서를 지정하고 있습니다. 따라서 `log.Log()`의 expectation #1번은 얼마든지 호출될 수 있지만, #2번 또는 #3번이 호출되는 순간 retire 됩니다. 왜냐하면 자신보다 나중에 호출되어야 할 expectation이 매칭되었기 때문입니다. 따라서 그 이후에 `log.Log()`에 "File too large."`라는 argument가 전달되면 테스트는 실패하게 될 것입니다.

아래와 같은 방법으로 `Seqeunce`없이 일반적인 방법으로 구현한 expectation들은 자동으로 retire되지 않는다는 점도 기억하기 바랍니다.

```cpp
using ::testing::_;
...
  EXPECT_CALL(log, Log(WARNING, _, _));                     // #1
  EXPECT_CALL(log, Log(WARNING, _, "File too large."));     // #2
```

위의 코드는 `"File too large."`라는 argument가 한 번만 전달되기를 바라면서 작성한 테스트코드입니다. `log.Log(WARNING, _, "File too large.")`라고 한 번 호출되었다면 문제가 없습니다. 그러나 두번째도 동일한 argument가 전달되면 테스트는 실패합니다. 왜냐하면 두번째 호출도 #2번 expectation을 사용하게 되는데 #2번 expectation은 1회만 호출되기를 기대하는 코드이기 때문입니다. 왜냐하면 cardinality가 생략되어 있으므로 이는 곧 `Times(1)`을 의미하기 때문입니다.

이러한 문제를 피하기 위해서는 expectation을 직접 retire시켜야 합니다. 즉, 원하는 cardinality가 충족(saturation)되었으니 retire시키라는 내용을 추가하면 됩니다.

```cpp
using ::testing::_;
...
  EXPECT_CALL(log, Log(WARNING, _, _));                     // #1
  EXPECT_CALL(log, Log(WARNING, _, "File too large."))      // #2
      .RetiresOnSaturation();
```

`RetiresOnSaturation()`을 사용해서 #2번 expectation이 1회 호출된 이후에는 retire되도록 구현했습니다. 이제 `Log(WARNING, _, "File too large.")`가 2회 호출되면 #2번 expectation을 건너뛰고 #1번 expectation으로 넘어갑니다. 왜냐하면 #2번은 retire되어 inactive상태로 변경되었고 gMock의 고려대상에서 제외되었기 때문입니다.

### Action 사용하기 ###

#### Mock Method에서 참조타입 반환하기 ####

만약 mock function이 참조타입을 반환해야 한다면 `Return()` 대신 `ReturnRef()`를 써야합니다.

```cpp
using ::testing::ReturnRef;

class MockFoo : public Foo {
 public:
  MOCK_METHOD(Bar&, GetBar, (), (override));
};
...
  MockFoo foo;
  Bar bar;
  EXPECT_CALL(foo, GetBar())
      .WillOnce(ReturnRef(bar));
...
```

#### Mock Method에서 Live Value 반환하기 ####

`Return(x)`는 해당 소스코드가 수행될 때 `x`의 복사본을 만들어서 미리 저장해둡니다. 따라서 `x`가 변수라고 해도 반환하는 값은 이미 고정되어 버립니다. 이렇게 미리 복사해 놓은 똑같은 값을 계속 반환하기를 원하지 않는다면 어떻게 해야 할까요? 이렇게 변수에 저장된 값을 동작으로 반영해서 반환하는 것을 *live value*라고 부릅니다. 즉, live value란 mock method가 호출되는 시점에 `x`라는 변수에 저장되어 있는 값을 반환값으로 사용한다는 의미입니다.

이전에 배웠던 것처럼 mock function이 참조타입을 반환하게 하는 `ReturnRef(x)`를 사용하면 되긴 하지만, `ReturnRef(x)`는 mock function의 return type이 실제로 참조일 때만 사용할 수 있습니다. 그럼 값을 반환하는 mock function에 live value를 사용하려면 어떻게 해야 할까요? 

`ByRef()`를 사용하면 될까요?

```cpp
using testing::ByRef;
using testing::Return;

class MockFoo : public Foo {
 public:
  MOCK_METHOD(int, GetValue, (), (override));
};
...
  int x = 0;
  MockFoo foo;
  EXPECT_CALL(foo, GetValue())
      .WillRepeatedly(Return(ByRef(x)));  // Wrong!
  x = 42;
  EXPECT_EQ(42, foo.GetValue());
```

아쉽지만, 위 코드는 원하는대로 동작하지 않습니다. 아래의 failure message와 함께 테스트는 실패할 것입니다.

```bash
Value of: foo.GetValue()
  Actual: 0
Expected: 42
```

그 이유는 `foo.GetValue()`의 반환값이 (mock method가 실제로 호출되는 시점의 `value`가 아니라) `Return(value)`라는 action이 수행되는 시점의 `value`로 정해지기 때문입니다. 이러한 결정은 `value`가 어떤 임시객체의 참조이거나 하는 위험한 상황을 대비하기 위한 것입니다. 해제된 임시객체를 가리키는 것은 언제나 위험합니다. 결과적으로 `ByRef(x)`는 `const int&`가 아니라 `int` 타입의 값으로 고정되어 버립니다. 그렇기 때문에 `Return(ByRef(x))`는 항상 `0`을 반환하게 되고 위의 테스트는 실패합니다.

위의 상황에서는 `ReturnPointee(pointer)`를 사용해야만 `foo.GetValue()`의 호출시점에 `pointer`에 저장된 값을 반환할 것입니다. 여기서 `pointer`에 저장된 값은 action 생성시점에 고정되는 것이 아니며 그 값이 달라질 수 있으므로 live value라고 부를 수 있습니다.

```cpp
using testing::ReturnPointee;
...
  int x = 0;
  MockFoo foo;
  EXPECT_CALL(foo, GetValue())
      .WillRepeatedly(ReturnPointee(&x));  // Note the & here.
  x = 42;
  EXPECT_EQ(42, foo.GetValue());  // This will succeed now.
```

#### Action 조합하기 ####

Mock function이 호출될 때, 1개 이상의 action을 수행하려면 어떻게 해야 할까요? `DoAll()`을 사용하면 여러개의 action을 모두 수행할 수 있습니다. 또한, 해당 mock function의 반환값으로는 마지막 action의 반환값을 사용한다는 점도 기억하기 바랍니다.

```cpp
using ::testing::_;
using ::testing::DoAll;

class MockFoo : public Foo {
 public:
  MOCK_METHOD(bool, Bar, (int n), (override));
};
...
  EXPECT_CALL(foo, Bar(_))
      .WillOnce(DoAll(action_1,
                      action_2,
                      ...
                      action_n));
```

#### 복잡한 argument 검증하기

Argument가 여러개이고 각각의 기대사항이 복잡한 mock method가 있다고 가정해 보겠습니다. 이런 경우에 각 argument의 기대사항이 다르다면(cardinality가 서로 다른경우 등) expectation을 지정하기가 쉽지 않습니다. 게다가 테스트가 실패한다면 어느 것이 잘못된 것인지 구분하기 어려울 것입니다.

```c++
  // Not ideal: this could fail because of a problem with arg1 or arg2, or maybe
  // just the method wasn't called.
  EXPECT_CALL(foo, SendValues(_, ElementsAre(1, 4, 4, 7), EqualsProto( ... )));
```

위 코드는 테스트가 실패했을 때, 어떤 부분에서 문제가 발생했는지 파악하기가 어렵습니다. 이럴 때에는 각각의 argument를 별도의 변수에 저장한 다음에 검증하면 도움이 됩니다.

```c++
  EXPECT_CALL(foo, SendValues)
      .WillOnce(DoAll(SaveArg<1>(&actual_array), SaveArg<2>(&actual_proto)));
  ... run the test
  EXPECT_THAT(actual_array, ElementsAre(1, 4, 4, 7));
  EXPECT_THAT(actual_proto, EqualsProto( ... ));
```

#### Mocking에서 Side Effects(부수효과) 사용하기 ####

Method가 반환값만으로 프로그램에 영향을 주는 것은 아닙니다. 예를 들어, method에서 global variable을 수정하면 반환값을 통하지 않더라도 프로그램의 상태를 변경할 수 있으며 문법적으로도 문제가 없습니다. 이렇게 반환값 외의 동작으로 프로그램에 영향을 주는 행위를 side-effect라고 합니다. side-effect라고 해서 꼭 문제되는 상황을 의미하는 것이 아님을 기억하기 바랍니다. 이러한 side-effect를 mocking에 사용하기 위한 가장 기본적인 방법은 `::testing::ActionInterface`를 사용자가 직접 정의하는 것입니다. 더불어 gMock은 side-effect와 관련된 기본적인 기능들도 역시 제공하고 있습니다. 

먼저, 포인터형식으로 전달된 argument의 값을 바꾸고 싶다면 built-in action인 `SetArgPointee()`를 사용하면 됩니다.

```cpp
using ::testing::_;
using ::testing::SetArgPointee;

class MockMutator : public Mutator {
 public:
  MOCK_METHOD(void, Mutate, (bool mutate, int* value), (override));
  ...
}
...
  MockMutator mutator;
  EXPECT_CALL(mutator, Mutate(true, _))
      .WillOnce(SetArgPointee<1>(5));
```

위와 같이 구현한 후에 `mutator.Mutate()`가 실제로 호출되면, 두번째 argument인 `value`가 가리키는 값이 `5`로 변경될 것입니다. 왜냐하면 `SetArgPointee<1>(5)`에서 `<1>`은 두번째 argument인 `value`를 의미하기 때문입니다. (`<0>`부터 시작이기 때문에 `<1>`이 두번째 argument입니다.)

`SetArgPointee()`는 전달해야 할 값(여기서는 `5`)의 복사본을 내부적으로 미리 만들어 둡니다. 위에서도 설명했듯이 그렇게 해야만 값이나 영역이 변경됨으로 인한 문제를 예방할 수 있기 때문입니다. 다만, 이를 위해서는 해당 타입이 copy constructor와 assignment operator를 지원해야 합니다.

Mock method에 `SetArgPointee()`를 사용함과 동시에 반환값도 지정하고 싶다면 `DoAll()`을 사용해서 `SetArgPointee()`와 `Return()`을 묶어 주기만 하면 됩니다. 예제코드는 아래와 같습니다.

```cpp
using ::testing::_;
using ::testing::Return;
using ::testing::SetArgPointee;

class MockMutator : public Mutator {
 public:
  ...
  MOCK_METHOD(bool, MutateInt, (int* value), (override));
}
...
  MockMutator mutator;
  EXPECT_CALL(mutator, MutateInt(_))
      .WillOnce(DoAll(SetArgPointee<0>(5),
                      Return(true)));
```

Argument가 배열인 경우에는 `SetArrayArgument<N>(first, last)`를 사용하시기 바랍니다. 이 action은 [first, last) 구간에 있는 값들은 전달받은 N번째 argument(배열)에 복사해줍니다.

```cpp
using ::testing::NotNull;
using ::testing::SetArrayArgument;

class MockArrayMutator : public ArrayMutator {
 public:
  MOCK_METHOD(void, Mutate, (int* values, int num_values), (override));
  ...
}
...
  MockArrayMutator mutator;
  int values[5] = {1, 2, 3, 4, 5};
  EXPECT_CALL(mutator, Mutate(NotNull(), 5))
      .WillOnce(SetArrayArgument<0>(values, values + 5));
```

`SetArrayArgument<N>(first, last)`는 iterator에도 문제없이 잘 동작합니다.

```cpp
using ::testing::_;
using ::testing::SetArrayArgument;

class MockRolodex : public Rolodex {
 public:
  MOCK_METHOD(void, GetNames, (std::back_insert_iterator<vector<string>>),
              (override));
  ...
}
...
  MockRolodex rolodex;
  vector<string> names;
  names.push_back("George");
  names.push_back("John");
  names.push_back("Thomas");
  EXPECT_CALL(rolodex, GetNames(_))
      .WillOnce(SetArrayArgument<0>(names.begin(), names.end()));
```

#### Mock Object의 동작을 프로그램에 상태에 따라 변화시키기 ####

Mock object의 동작을 프로그램의 상태에 따라 변화시키길 원한다면 `::testing::InSequence`를 사용하면 됩니다. 아래에 예제가 있습니다.

```cpp
using ::testing::InSequence;
using ::testing::Return;

...
  {
     InSequence seq;
     EXPECT_CALL(my_mock, IsDirty())
         .WillRepeatedly(Return(true));
     EXPECT_CALL(my_mock, Flush());
     EXPECT_CALL(my_mock, IsDirty())
         .WillRepeatedly(Return(false));
  }
  my_mock.FlushIfDirty();
```

위 코드에서 첫번째 `my_mock.IsDirty()`는 몇 번 호출되는지에 관계없이(`WillRepeatedly`) 계속해서 `true`를 반환합니다. 그러다가 `my_mock.Flush()`가 호출되면 첫번째 `my_mock.IsDirty()`는 자동으로 retire됩니다. 따라서 `my_mock.Flush()`가 호출된 후의 `my_mock.IsDirty()`는 계속해서 `false`를 반환할 것입니다. 즉, `my_mock.Flush()`가 호출됨으로써 프로그램 상태에 어떠한 변화가 생겼으며 그러한 상태변화에 따라 `my_mock.IsDirty()`의 동작을 달라지게 구현한 예제로 볼 수 있습니다.

좀 더 복잡한 구조를 가지는 프로그램에서는 변수를 사용해서 mock method 로 전달되는 값을 저장하고 다시 그것을 반환하도록 하는 방법도 유용할 것입니다.

```cpp
using ::testing::_;
using ::testing::SaveArg;
using ::testing::Return;

ACTION_P(ReturnPointee, p) { return *p; }
...
  int previous_value = 0;
  EXPECT_CALL(my_mock, GetPrevValue)
      .WillRepeatedly(ReturnPointee(&previous_value));
  EXPECT_CALL(my_mock, UpdateValue)
      .WillRepeatedly(SaveArg<0>(&previous_value));
  my_mock.DoSomethingToUpdateValue();
```

위의 코드에서 `my_mock.GetPrevValue()`는 `previous_value`의 최신값을 항상 반환합니다. 또한, 이러한 `previous_value`는 `UpdateValue()`에 의해서 변경됩니다.

#### Return Type의 Default Value 변경하기 ####

사용자가 어떤 mock method의 반환값을 별도로 지정하지 않은 경우에는 해당타입에 대한 C++ built-in default value가 반환됩니다. 아시다시피 이러한 값은 대부분의 경우에 `0`일 확률이 큽니다. 더불어 C++ 11 이상 버전에 대해서는 mock method의 return type이 default constructor를 가지고 있는 경우에는 `0`이 아니라 default constructor를 통해 생성된 값을 반환해줍니다. 이처럼 gMock은 사용자가 mock method의 반환값을 직접 지정하지 않더라도 default value를 최대한 찾아서 반환해줌으로써 테스트가 중단되지 않고 진행될 수 있도록 도와줍니다.

이제 여기서는 default value 자체를 변경하는 방법에 대해 공유하려 합니다. 이러한 기능이 필요한 이유는 default vaule가 사용자가 원하는 것과는 다를 수 있으며 gMock이 특정타입의 default value를 판단할 수 없는 경우에는 사용자가 직접 구현해서 제공해야하기 때문입니다. 아래 예제와 같이 `::testing ::DefaultValue`라는 template class를 사용하면 default value를 변경할 수 있습니다.

```cpp
using ::testing::DefaultValue;

class MockFoo : public Foo {
 public:
  MOCK_METHOD(Bar, CalculateBar, (), (override));
};


...
  Bar default_bar;
  // Sets the default return value for type Bar.
  DefaultValue<Bar>::Set(default_bar);

  MockFoo foo;

  // We don't need to specify an action here, as the default
  // return value works for us.
  EXPECT_CALL(foo, CalculateBar());

  foo.CalculateBar();  // This should return default_bar.

  // Unsets the default return value.
  DefaultValue<Bar>::Clear();
```

사실 default value를 직접 변경하는 것은 test program의 가독성을 저하시키기 때문에 주의해서 사용해야 합니다. 위 예제는 그러한 문제를 해결하기 위해서 `Set()`과 `Clear()`를 함께 사용한 코드입니다. 즉, default value를 변경하여 사용했다가 다시 원래대로 복구시키기 때문에 다른 곳에 끼치는 영향이 거의 없습니다.

#### Default Action을 상황에 따라 변화시키기 ####

위에서 특정타입에 대한 default value를 변경하는 방법에 대해 배웠습니다. 그러나 이렇게 반환값을 변경하는 것만으로는 부족할 수도 있습니다. 예를 들어 동일한 return type을 사용하는 2개의 mock method가 있을때 각각에 대해 다른 동작을 지정하고 싶을 수 있습니다. 이런 경우에는 `ON_CALL()`을 사용하면 mock method마다 다른 행위를 지정할 수 있습니다.

```cpp
using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Gt;
using ::testing::Return;
...
  ON_CALL(foo, Sign(_))
      .WillByDefault(Return(-1));
  ON_CALL(foo, Sign(0))
      .WillByDefault(Return(0));
  ON_CALL(foo, Sign(Gt(0)))
      .WillByDefault(Return(1));

  EXPECT_CALL(foo, Sign(_))
      .Times(AnyNumber());

  foo.Sign(5);   // This should return 1.
  foo.Sign(-9);  // This should return -1.
  foo.Sign(0);   // This should return 0.
```

동일한 mock method에 대해서 `ON_CALL()`을 여러개 사용하면 코드상에서 나중에 오는 것부터 먼저 탐색하게 됩니다. 즉, 밑에서 위 방향으로 하나씩 비교하면서 원하는 `ON_CALL()`이 있는지 찾게 됩니다. 이러한 비교순서는 mock object의 constructor나 test fixture에서 적용한 `ON_CALL()`의 우선순위가 개별 `TEST()`에서 지정한 `ON_CALL()`의 우선순위보다 낮아지도록 만듭니다. 쉽게 말하면 동일한 이름의 지역변수와 전역변수가 있을 때, 지역변수가 먼저 선택되는 것과 같습니다.

#### Functions/Functors/Lambda를 Action으로 사용하기 ####

테스트를 구현하다 보면 gMock에서 제공하는 built-in action만으로는 뭔가 부족한 상황이 발생할 수 있습니다. 이런 경우에는 기존에 사용하던 function, functor, `std::function`, lambda 등을 마치 action처럼 사용하는 것도 가능합니다. 이를 위한 구현방법은 아래 예제에 있습니다.

```cpp
using ::testing::_; using ::testing::Invoke;

class MockFoo : public Foo {
 public:
  MOCK_METHOD(int, Sum, (int x, int y), (override));
  MOCK_METHOD(bool, ComplexJob, (int x), (override));
};

int CalculateSum(int x, int y) { return x + y; }
int Sum3(int x, int y, int z) { return x + y + z; }

class Helper {
 public:
  bool ComplexJob(int x);
};

...
  MockFoo foo;
  Helper helper;
  EXPECT_CALL(foo, Sum(_, _))
      .WillOnce(&CalculateSum)
      .WillRepeatedly(Invoke(NewPermanentCallback(Sum3, 1)));
  EXPECT_CALL(foo, ComplexJob(_))
      .WillOnce(Invoke(&helper, &Helper::ComplexJob));
      .WillRepeatedly([](int x) { return x > 0; });

  foo.Sum(5, 6);         // Invokes CalculateSum(5, 6).
  foo.Sum(2, 3);         // Invokes Sum3(1, 2, 3).
  foo.ComplexJob(10);    // Invokes helper.ComplexJob(10).
  foo.ComplexJob(-1);    // Invokes the inline lambda.
```

여기서 한가지 유의할 점은 fucntion type을 어느정도 지켜줘야 한다는 것입니다.(Mock function의 signature와 완벽히 동일하지는 않더라도 *compatible*해야 합니다.) 즉, 양쪽의 argument와 return type이 암시적으로 형변환 가능해야 합니다. 여기서 타입을 좀 더 엄격하게 검사하지 않는 이유는 사용자가 이 기능을 유연하게 사용할 수 있도록 도와주기 위함입니다. 또한, gMock이 엄격한 타입검사를 하지 않기 때문에 사용자 스스로 확인하여 암시적인 형변환이 발생해도 안전하다고 판단되는 경우에만 사용해야 한다는 점도 기억하시기 바랍니다.

**Note: lambda를 action으로 사용할 때**

*   Callback의 소유권은 action에 있으며 따라서 action이 삭제될 때, callback도 삭제됩니다.
*   만약 사용하려는 callback이 `C`라는 base callback type을 상속받았다면 overloading 문제를 해결하기 위해 base callback type으로 casting 해줘야 합니다. 아래 예제코드를 확인하세요.

```c++
using ::testing::Invoke;
...
 ResultCallback<bool>* is_ok = ...;
 ...
 Invoke(is_ok) ...;  // This works.

 BlockingClosure* done = new BlockingClosure;
 ...
 Invoke(implicit_cast<Closure*>(done)) ...;  // The cast is necessary.
```

#### Argument 개수가 더 많은 Function/Functor/Lambda를 Action으로 사용하기

현재까지는 mock function에 `Invoke()`를 사용하기 위해서 argument의 개수가 동일한 function, functor, lambda만을 주로 다뤄왔습니다. 만약 function, functor, labmda 등의 argument가 mock function보다 많으면 어떻게 해야할까요? 이런 경우에는 `NewPermanentCallback`를 사용해서 부족한 argument를 미리(pre-bound) 지정할 수 있습니다. 아래 예제는 `SignOfSum()`의 첫번째 argument `x`에 `5`라는 값을 지정하는 방법을 보여줍니다.

```c++
using ::testing::Invoke;

class MockFoo : public Foo {
 public:
  MOCK_METHOD(char, DoThis, (int n), (override));
};

char SignOfSum(int x, int y) {
  const int sum = x + y;
  return (sum > 0) ? '+' : (sum < 0) ? '-' : '0';
}

TEST_F(FooTest, Test) {
  MockFoo foo;

  EXPECT_CALL(foo, DoThis(2))
      .WillOnce(Invoke(NewPermanentCallback(SignOfSum, 5)));
  EXPECT_EQ('+', foo.DoThis(2));  // Invokes SignOfSum(5, 2).
}
```

#### Agument 없는 Function/Functor/Lambda를 Action으로 사용하기 ####

`Invoke()`는 복잡한 action을 구현해야 할 때 매우 유용합니다. 특히, mock function으로 전달된 argument들을 연결된 function이나 functor로 그대로 전달해주기 때문에 mock function과 동일한 context를 사용할 수 있도록 도와줍니다.

그럼 function, functor와 mock function의 signature가 서로 다르고 compatible하지 않다면 어떻게 해야될까요? 현재까지 배운내용만 가지고 구현하려면 mock function과 signature가 compatible한 wrapper function을 새로 만들고 그것을 `Invoke()`를 통해 호출하면 될 것입니다. 그 다음에 wrapper function 내부에서 function, functor를 호출하도록 구현하면 됩니다.

그러나 위의 작업은 반복적이고 귀찮은 일입니다. gMock은 이러한 반복적인 작업을 대신 해줍니다. 이 때에는 `InvokeWithoutArgs()`를 사용하면 됩니다. 기본적인 동작은 `Invoke()`와 동일하지만 mock function의 argument를 전달하지 않는다는 부분만 다릅니다. 아래에 예제가 있습니다.

```cpp
using ::testing::_;
using ::testing::InvokeWithoutArgs;

class MockFoo : public Foo {
 public:
  MOCK_METHOD(bool, ComplexJob, (int n), (override));
};

bool Job1() { ... }
bool Job2(int n, char c) { ... }

...
  MockFoo foo;
  EXPECT_CALL(foo, ComplexJob(_))
      .WillOnce(InvokeWithoutArgs(Job1))
      .WillOnce(InvokeWithoutArgs(NewPermanentCallback(Job2, 5, 'a')));

  foo.ComplexJob(10);  // Invokes Job1().
  foo.ComplexJob(20);  // Invokes Job2(5, 'a').
```

#### Mock Function에 전달된 Callable Argument를 호출하기 ####

Mock function의 argument로 fuction pointer, functor가 전달되는 경우도 있을 것입니다. 즉, "callable" 타입이 argument로 전달되는 경우입니다. 아래 예제를 보시기 바랍니다.

```cpp
class MockFoo : public Foo {
 public:
  MOCK_METHOD(bool, DoThis, (int n, (ResultCallback1<bool, int>* callback)),
              (override));
};
```

위와 같이 전달된 callable argument를 호출하고 싶다면 어떻게 하면 될까요? 아래코드의 `WillOnce(...)`에는 어떤 action을 사용해야 할까요?

```cpp
using ::testing::_;
...
  MockFoo foo;
  EXPECT_CALL(foo, DoThis(_, _))
      .WillOnce(...);
      // Will execute callback->Run(5), where callback is the
      // second argument DoThis() receives.
```

NOTE: 이 내용자체는 C++에 lambda가 적용되기 전에 작성된 legacy documentation임을 미리 밝힙니다.

(Lambda가 없다고 가정해도) callable argument를 호출하기 위한 action을 직접 구현할 필요는 없습니다. gMock은 이를 위한 action도 이미 제공하고 있습니다.

```cpp
InvokeArgument<N>(arg_1, arg_2, ..., arg_m)
```

`InvokeArgument<N>(arg1, arg2, ..., arg_m)`라는 action은 mock function의 N번째 argument로 전달된 callable을 호출해줍니다. 게다가 callable에 argument를 전달하는 것도 가능합니다.

아래는 관련내용이 추가된 코드입니다.

```cpp
using ::testing::_;
using ::testing::InvokeArgument;
...
  EXPECT_CALL(foo, DoThis(_, _))
      .WillOnce(InvokeArgument<1>(5));
      // Will execute callback->Run(5), where callback is the
      // second argument DoThis() receives.
```

만약, callable에 참조형식 argument를 전달하고 싶다면 어떻게 해야할까요? 여기서도 `ByRef()`를 사용하면 됩니다.

```cpp
  ...
  MOCK_METHOD(bool, Bar,
              ((ResultCallback2<bool, int, const Helper&>* callback)),
              (override));
  ...
  using ::testing::_;
  using ::testing::ByRef;
  using ::testing::InvokeArgument;
  ...
  MockFoo foo;
  Helper helper;
  ...
  EXPECT_CALL(foo, Bar(_))
      .WillOnce(InvokeArgument<0>(5, ByRef(helper)));
      // ByRef(helper) guarantees that a reference to helper, not a copy of it,
      // will be passed to the callback.
```

Callable에 참조형식 argument를 전달할 때, `ByRef()`를 사용하지 않으면 어떻게 될까요? 그런 경우에는 `InvokeArgument()`가 해당 argument의 *복사본을 만들고 다시 그 복사본의 참조를 전달하게 됩니다.* 이러한 동작방식은 argument가 temporary value일 때 상당히 유용하며 gMock이 temporary value의 복사본을 만들어서 관리하고 있기 때문에 안전성도 보장됩니다. 아래는 이러한 temporary value가 사용된 예제입니다.

```cpp
  ...
  MOCK_METHOD(bool, DoThat, (bool (*f)(const double& x, const string& s)),
              (override));
  ...
  using ::testing::_;
  using ::testing::InvokeArgument;
  ...
  MockFoo foo;
  ...
  EXPECT_CALL(foo, DoThat(_))
      .WillOnce(InvokeArgument<0>(5.0, string("Hi")));
      // Will execute (*f)(5.0, string("Hi")), where f is the function pointer
      // DoThat() receives.  Note that the values 5.0 and string("Hi") are
      // temporary and dead once the EXPECT_CALL() statement finishes.  Yet
      // it's fine to perform this action later, since a copy of the values
      // are kept inside the InvokeArgument action.
```

#### Action의 반환값 무시하기 ####

모두 그런것은 아니지만 `Return`, `Invoke`와 같은 action은 무언가를 반환할 수 있습니다. 그러나 이러한 기능이 오히려 불편한 경우도 있습니다. 왜냐하면 mock function의 return type이 void일 때는 void가 아닌 다른 것을 반환하는 action은 사용할 수 없기 때문입니다. 또는 반환값이 있는 action을 `DoAll()`의 중간에 두고 싶을 수도 있습니다.(반환값이 있는 action은 원래 `DoAll()`의 마지막에 와야합니다.) Googletest는 이러한 상황에 대한 해결방법을 제공하고 있습니다. 이처럼 action의 반환값을 무시해야 하는 상황이 발생하면 `IgnoreResult()`을 사용할 수 있는데요. 아래 예제에서 확인하시기 바랍니다.

```cpp
using ::testing::_;
using ::testing::DoAll;
using ::testing::IgnoreResult;
using ::testing::Return;

int Process(const MyData& data);
string DoSomething();

class MockFoo : public Foo {
 public:
  MOCK_METHOD(void, Abc, (const MyData& data), (override));
  MOCK_METHOD(bool, Xyz, (), (override));
};

  ...
  MockFoo foo;
  EXPECT_CALL(foo, Abc(_))
      // .WillOnce(Invoke(Process));
      // The above line won't compile as Process() returns int but Abc() needs
      // to return void.
      .WillOnce(IgnoreResult(Process));
  EXPECT_CALL(foo, Xyz())
      .WillOnce(DoAll(IgnoreResult(DoSomething),
                      // Ignores the string DoSomething() returns.
                      Return(true)));
```

당연한 이야기지만 `IgnoreResult()`를 return type이 `void`인 action에는 사용하면 안 됩니다.

#### Action의 Argument 선택하기 ####

Argument가 7개인 mock function `Foo()`가 있고 여기에 직접 구현한 action을 연결하려 합니다. 그런데 action은 3개의 argument만 받도록 이미 구현되어 있다면 어떻게 해야할까요? 현재까지 파악된 방법으로는 아래처럼 구현할 수 밖에 없습니다.

```cpp
using ::testing::_;
using ::testing::Invoke;
...
  MOCK_METHOD(bool, Foo,
              (bool visible, const string& name, int x, int y,
               (const map<pair<int, int>>), double& weight, double min_weight,
               double max_wight));
...
bool IsVisibleInQuadrant1(bool visible, int x, int y) {
  return visible && x >= 0 && y >= 0;
}
...
  EXPECT_CALL(mock, Foo)
      .WillOnce(Invoke(IsVisibleInQuadrant1));  // Uh, won't compile. :-(
```

네, 위 코드는 당연히 컴파일이 안 될 것입니다. 이 문제를 해결하기 위해서는 먼저 `Foo()`와 동일한 signature를 갖는 "adaptor"를 하나 구현해서 `Invoke()`가 호출할 수 있도록 만들어줘야 합니다.

```cpp
using ::testing::_;
using ::testing::Invoke;
...
bool MyIsVisibleInQuadrant1(bool visible, const string& name, int x, int y,
                            const map<pair<int, int>, double>& weight,
                            double min_weight, double max_wight) {
  return IsVisibleInQuadrant1(visible, x, y);
}
...
  EXPECT_CALL(mock, Foo)
      .WillOnce(Invoke(MyIsVisibleInQuadrant1));  // Now it works.
```

이제 문제없이 동작할 것입니다. 다만, 중복코드가 좀 있어서 미관상 아름답지는 않습니다.

이런 경우에는 gMock에서 제공하는 *action adaptor*를 사용하면 훨씬 쉽고 간결한 구현이 가능합니다. 사용자가 직접 adaptor를 구현하지 않아도 됩니다.

```cpp
WithArgs<N1, N2, ..., Nk>(action)
```

위 코드와 같이 `WithArgs`는 괄호안에 있는 내부 `action`을 호출해줍니다.(`WithArgs`자체도 action이므로 구분하기 위해 내부 `action`이라고 했습니다.) 이를 통해 mock function으로 전달된 argument 중에서 필요한 것만 골라서 내부 `action`을 호출할 수 있습니다. 아래는 처음 코드에 `WithArgs`가 적용된 결과입니다.

```cpp
using ::testing::_;
using ::testing::Invoke;
using ::testing::WithArgs;
...
  EXPECT_CALL(mock, Foo)
      .WillOnce(WithArgs<0, 2, 3>(Invoke(IsVisibleInQuadrant1))); 
      // No need to define your own adaptor.
```

관련해서는 몇 가지 유사한 기능들도 아래와 같이 제공하고 있습니다.

- `WithoutArgs(action)`는 내부 `action`이 argument를 받지 않을 때 사용합니다.
- `WithArg<N>(action)`는 내부 `action`이 argument를 1개만 받을 때 사용합니다. (`Args`에서 `s`가 빠졌습니다)

이름에서도 유추할 수 있듯이 `InvokeWithoutArgs(...)`라는 action은 `WithoutArgs(Invoke(...))`와 동일한 동작을 하며 보기 좋게 만들어 제공하는 것 뿐입니다.

그럼 마지막으로 몇가지 사용팁을 소개합니다.

- `WithArgs`에서 사용하는 내부 action에는 `Invoke()`가 아닌 다른 action을 사용해도 괜찮습니다.
- 특정 argument를 반복해서 사용할 수도 있습니다. 예를 들어 `WithArgs<2, 3, 3, 5>(....)`와 같이 index=`3`인 argument를 여러번 전달해도 됩니다.
- argument들의 순서를 바꿔서 내부 action을 호출하는 것도 가능합니다. 예를 들어 `WithArgs<3, 2, 1>(...)`과 같이 전달해도 됩니다.
- 선택한 argument type과 내부 action의 argument type이 정확히 같지는 않아도 됩니다. 암시적인 형변환이 가능하다면 동작합니다. 예를 들어 mock function으로 전달된 argument가 `int` 타입이라면 이것을 action의 `double` 타입 argument에 전달해도 문제없이 잘 동작합니다.

#### 관심없는 Argument 무시하기 ####

[Action의 Argument를 선택하기](cook_book.md#action의-argument를-선택하기)에서 mock function이 action으로 argument를 전달하게 하는 방법을 배웠습니다. 이 때, `WithArgs<...>()`를 사용함에 있어서 한가지 단점은 테스트를 구현하는 사람이 조금 귀찮을 수 있다는 것입니다.

만약, `Invoke*()`와 함께 사용할 function(functor, lambda)을 지금 현재 구현하고 있다면 `WithArgs` 대신에 `Unused`를 사용하는 것도 좋은 옵션이 될 것입니다. `Unused`는 해당 argument는 사용하지 않을 것이라고 명시적으로 표현하는 것이기 때문에 굳이 `WithArgs`를 사용하지 않아도 mock function과 내부 action을 연결할 수 있습니다. 이것의 장점이라면 아무래도 소스코드가 깔끔해지고 argument 변경에 대해서도 쉽고 빠르게 대응할 수 있다는 것입니다. 물론 action의 재사용성도 좋아질 것입니다. 그럼 예제코드를 보겠습니다.

```cpp
 public:
  MOCK_METHOD(double, Foo, double(const string& label, double x, double y),
              (override));
  MOCK_METHOD(double, Bar, (int index, double x, double y), (override));
```

위 코드의 `Foo()`와 `Bar()`은 첫번째 argument만 다릅니다. 이 2개의 mock function에 동일한 동작을 하는 action을 연결하려면 어떻게 해야 할까요? 지금까지 배운 내용으로는 2개의 서로 다른 action을 정의해서 `Invoke`와 연결해줘야 합니다. 즉, 아래처럼 구현해야 했습니다.

```cpp
using ::testing::_;
using ::testing::Invoke;

double DistanceToOriginWithLabel(const string& label, double x, double y) {
  return sqrt(x*x + y*y);
}
double DistanceToOriginWithIndex(int index, double x, double y) {
  return sqrt(x*x + y*y);
}
...
  EXPECT_CALL(mock, Foo("abc", _, _))
      .WillOnce(Invoke(DistanceToOriginWithLabel));
  EXPECT_CALL(mock, Bar(5, _, _))
      .WillOnce(Invoke(DistanceToOriginWithIndex));
```

이 때, `Unused`를 사용하면 1개의 action만 구현해도 2개의 mock function에 대응할 수 있습니다. 관련 소스코드는 아래와 같습니다.

```cpp
using ::testing::_;
using ::testing::Invoke;
using ::testing::Unused;

double DistanceToOrigin(Unused, double x, double y) {
  return sqrt(x*x + y*y);
}
...
  EXPECT_CALL(mock, Foo("abc", _, _))
      .WillOnce(Invoke(DistanceToOrigin));
  EXPECT_CALL(mock, Bar(5, _, _))
      .WillOnce(Invoke(DistanceToOrigin));
```

#### Action 공유하기 ####

gMock에서 matcher는 내부적으로 ref-counted 포인터를 사용하기 때문에 여러 곳에서 공유하기가 편리하다는 내용을 위에서 설명했습니다. Action도 마찬가지입니다. 내부적으로 ref-counted 포인터를 사용하기 때문에 효율적으로 공유될 수 있습니다. 즉, 동일한 action object를 여러곳에서 참조하고 있다면 마지막 참조자가 없어질 때에 action object도 실제로 소멸될 것입니다.

따라서 복잡한 action을 재사용할 필요가 있다면 매번 새롭게 만들기보다 재사용 가능하도록 구현하는 것이 좋습니다. gMock에서는 action을 복사하는 것도 가능하므로 많은 도움이 될 것입니다. 물론 해당 action이 별도의 내부적인 상태를 갖지 않는 경우에만 자유롭게 재사용이 가능할 것입니다. 예를 들어 아래와 같은 `set_flag` action은 내부적으로는 어떠한 상태로 갖지 않으므로 반복해서 사용해도 문제가 없습니다.

```cpp
using ::testing::Action;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::SetArgPointee;
...
  Action<bool(int*)> set_flag = DoAll(SetArgPointee<0>(5),
                                      Return(true));
  ... use set_flag in .WillOnce() and .WillRepeatedly() .....
```

그러나 action이 스스로 상태를 정의하고 관리하는 경우에는 공유할 때 주의해야 합니다. 예를 들어서 `IncrementCounter(init)`이라는 action factory가 있다고 하겠습니다. 이 factory는 `init`이라는 argument로 전달된 값을 통해 내부변수를 초기화하고 그 변수에 다시 +1 하여 반환하는 action을 생성해줍니다. 아래의 2가지 예제코드를 통해 `IncrementCounter(init)`이라는 action의 공유여부에 따라 결과값이 달라지는 상황을 공유합니다.

```cpp
  EXPECT_CALL(foo, DoThis())
      .WillRepeatedly(IncrementCounter(0));
  EXPECT_CALL(foo, DoThat())
      .WillRepeatedly(IncrementCounter(0));
  foo.DoThis();  // Returns 1.
  foo.DoThis();  // Returns 2.
  foo.DoThat();  // Returns 1 - Blah() uses a different
                 // counter than Bar()'s.
```

먼저 위의 코드는 action을 공유하지 않는 코드입니다. 즉, `DoThis()`와 `DoThat()`은 서로 다른 `IncrementCounter()`를 갖게 됩니다. 2개 mock function에서 사용하는 action이 독립적인 상태를 유지하기 때문에 1,2,1 이 출력됩니다. 반면에 아래 코드는 `DoThis()`와 `DoThat()`이 `IncrementCounter()`를 공유합니다. 따라서 action의 내부변수도 공유하기 때문에 `0`으로 초기화한 후에 차례대로 1,2,3 이 출력될 것입니다. 이처럼 공유여부에 따라 action의 수행결과가 달라질 수 있기 때문에 사용하는 목적에 따라 주의해서 구현해야 합니다.

```cpp
  Action<int()> increment = IncrementCounter(0);

  EXPECT_CALL(foo, DoThis())
      .WillRepeatedly(increment);
  EXPECT_CALL(foo, DoThat())
      .WillRepeatedly(increment);
  foo.DoThis();  // Returns 1.
  foo.DoThis();  // Returns 2.
  foo.DoThat();  // Returns 3 - the counter is shared.
```

#### 비동기적인 동작을 검증하기

비동기적인 동작을 검증하는 것은 gMock을 사용하면서 자주 겪게되는 어려움 중에 하나입니다. EventQueue라는 class가 하나 있고 이를 위한 interface인 EventDispatcher도 있다고 가정해 보겠습니다. EventQueue class는 말 그대로 전달된 event에 대응하는 동작을 수행해주는 class입니다. 이 때, EventQueue class가 별도의 thread를 생성해서 코드를 수행한다면 이를 어떻게 검증할 수 있을까요? `sleep()`을 적절히 조절해서 event가 전달되기를 기다려야 할까요? 물론 가능한 방법이긴 하지만 `sleep()`을 사용하면 테스트의 동작이 non-deterministic하게 됩니다. 이런 상황에서 제일 좋은 방법은 gMock action과 notification object를 조합하는 것입니다. 이를 통해 비동기적인 동작을 동기적으로 변경할 수 있습니다. 아래 예제코드가 있습니다.

```c++
using ::testing::DoAll;
using ::testing::InvokeWithoutArgs;
using ::testing::Return;

class MockEventDispatcher : public EventDispatcher {
  MOCK_METHOD(bool, DispatchEvent, (int32), (override));
};

ACTION_P(Notify, notification) {
  notification->Notify();
}

TEST(EventQueueTest, EnqueueEventTest) {
  MockEventDispatcher mock_event_dispatcher;
  EventQueue event_queue(&mock_event_dispatcher);

  const int32 kEventId = 321;
  Notification done;
  EXPECT_CALL(mock_event_dispatcher, DispatchEvent(kEventId))
      .WillOnce(Notify(&done));

  event_queue.EnqueueEvent(kEventId);
  done.WaitForNotification();
}
```

위의 코드는 일반적인 gMock expectation의 구현방법과 크게 다르지 않지만 `Notification`이라는 object와 이를 사용하는 action이 하나 추가되었습니다. 내용은 `kEventId`가 `DispatchEvent()`로 전달되면 테스트가 성공하는 것입니다. 기존에는 `sleep()`을 통해서 얼마간 기다렸다가 `kEventId`가 전달되었는지 확인했다면 여기서는 `Notification::WaitForNotification()`을 호출해서 실제로 전달되는 시점을 확인할 수 있게 했습니다. 즉, 비동기적인 호출이 완료되기를 기다렸다가 안전하게 테스트를 종료할 수 있습니다.(`Notification` class는 `gtest-port.h`, `gtest-port.cc`에 구현되어 있습니다.)

Note: 이 예제의 단점도 있습니다. 만약 expectation이 만족하지 않으면 테스트가 영원히 끝나지 않기 때문입니다. 언젠가는 타임아웃이 발생하고 실패로 판정되긴 하겠지만 무작정 기다리는 것은 테스트 수행시간을 늘어나게 하며 디버깅도 어렵게 합니다. 이러한 문제를 해결하기 위해서 `WaitForNotificationWithTimeout(ms)`를 사용하면 얼마간의 시간동안만 기다렸다가 타임아웃을 발생시킬 수 있습니다.

### gMock의 다양한 사용법 ###

#### Method가 Move-Only Type을 사용할 때의 Mocking 방법 ####

C++11에서 *move-only-type*이 소개되었습니다. Move-only-type이란 이동은 가능하지만 복사는 불가능한 객체를 의미합니다. C++의 `std::unique_ptr<T>`가 대표적인 예입니다.

이러한 move-only-type을 반환하는 함수를 mocking하는 것은 사실 좀 어렵습니다. 그러나 역시 불가능한 것은 아닙니다. 다만, 이에 대한 해결방법이 2017년 4월에 gMock에 추가되었기 때문에 그보다 예전버전을 사용하고 있다면 [Legacy workarounds for move-only types](cook_book.md#legacy--move-only-type-해결방법)를 참조하기를 바랍니다.

먼저 가상의 프로젝트를 하나 진행하고 있다고 해봅시다. 이 프로젝트는 사람들이 "buzzes"라고 불리는 짧은 글을 작성하고 공유할 수 있게 해주는 프로젝트입니다. 이를 위해서 아래와 같은 타입들을 정의하고 사용하고 있습니다.

```cpp
enum class AccessLevel { kInternal, kPublic };

class Buzz {
 public:
  explicit Buzz(AccessLevel access) { ... }
  ...
};

class Buzzer {
 public:
  virtual ~Buzzer() {}
  virtual std::unique_ptr<Buzz> MakeBuzz(StringPiece text) = 0;
  virtual bool ShareBuzz(std::unique_ptr<Buzz> buzz, int64_t timestamp) = 0;
  ...
};
```

간단히 설명하면 `Buzz` object는 작성하고 있는 글을 의미합니다. 다음으로 `Buzzer`라는 interface를 상속받고 구체화한 class는 앞에서 말한 `Buzz` object를 사용하는 주체가 될 것입니다. 마지막으로 `Buzzer`의 method들은 return type 또는 argument type에 `unique_ptr<Buzz>`를 사용하고 있습니다. 네, 그럼 이제 `Buzzer`를 mocking 해보겠습니다.

먼저, `Buzzer` interface를 상속받는 mock class를 만들겠습니다. Move-only-type을 반환하는 method를 mocking할 때도 `MOCK_METHOD`를 사용하는 점은 동일합니다.

```cpp
class MockBuzzer : public Buzzer {
 public:
  MOCK_METHOD(std::unique_ptr<Buzz>, MakeBuzz, (StringPiece text), (override));
  MOCK_METHOD(bool, ShareBuzz, (std::unique_ptr<Buzz> buzz, int64_t timestamp),
              (override));
};
```

위와 같이 mock class를 정의했다면 이제 사용할 차례입니다. 위에서 정의한 `MockBuzzer` class의 object를 하나 생성합니다.

```c++
  MockBuzzer mock_buzzer_;
```

이제, mock method에 expectation을 설정할 차례인데요. 예를 들어 `unique_ptr<Buzz>`를 반환하는 `MakeBuzz()`에는 expectation을 어떻게 설정해야 할까요?

사실 지금까지 해왔던 것과 동일하게 `.WillOnce()`, `.WillRepeatedly()`를 사용하면 됩니다. 또한, action을 따로 연결하지 않는다면 default action이 수행되는 것도 같습니다. 여기서 `unique_ptr<>`를 반환하기 위한 default action은 default constructor를 사용해 object를 생성해주는 것이기 때문에 별도의 action을 연결하지 않는다면 `nullptr`을 가리키는 `unique_ptr` object가 반환될 것입니다. 

```cpp
  // Use the default action.
  EXPECT_CALL(mock_buzzer_, MakeBuzz("hello"));

  // Triggers the previous EXPECT_CALL.
  EXPECT_EQ(nullptr, mock_buzzer_.MakeBuzz("hello"))
```

이러한 default action을 변경하고 싶다면 [Setting Default Actions](cheat_sheet.md#default-action-설정하기)를 참조하세요.

`Return(ByMove(...))`를 사용하면 특정한 move-only-type 값을 반환하도록 지정할 수도 있습니다.

```cpp
  // When this fires, the unique_ptr<> specified by ByMove(...) will
  // be returned.
  EXPECT_CALL(mock_buzzer_, MakeBuzz("world"))
      .WillOnce(Return(ByMove(MakeUnique<Buzz>(AccessLevel::kInternal))));

  EXPECT_NE(nullptr, mock_buzzer_.MakeBuzz("world"));
```

Move-only-type 타입을 반환할 때, `ByMove()`를 사용하지 않는다면 당연하게도 compile error가 발생합니다.

여기서 문제입니다, `Return(ByMove(...))` action이 한 번 이상 수행될 수 있을까요? 예를 들면 `WillRepeatedly(Return(ByMove(...)));`와 같이 사용하는 것이 가능할까요? 결론부터 말하면 안됩니다. `Return(ByMove(...))`는 한 번만 호출될 수 있습니다. 왜냐하면 해당 action이 처음 한 번 수행되면 해당 값은 mock function을 호출한 caller쪽으로 이동하게 됩니다. 따라서 mock function과 action이 다시 호출되었을 때는 이동해야 할 값이 이미 없어진 상태이기 때문에 runtime error가 발생할 것입니다.

Lambda 혹은 callable object를 사용하면 위와 같은 문제를 해결할 수 있습니다. 즉, move-only-type에 대해서도 `WillRepeatedly`를 사용할 수 있습니다. 관련 예제코드가 아래에 있습니다.

```cpp
  EXPECT_CALL(mock_buzzer_, MakeBuzz("x"))
      .WillRepeatedly([](StringPiece text) {
        return MakeUnique<Buzz>(AccessLevel::kInternal);
      });

  EXPECT_NE(nullptr, mock_buzzer_.MakeBuzz("x"));
  EXPECT_NE(nullptr, mock_buzzer_.MakeBuzz("x"));
```

위 코드는 lambda를 사용함으로써 `Return(ByMove(...))`로는 불가능했던 일을 가능하게 했습니다. 이제 `mock_buzzer`가 호출될 때마다 새로운 `unique_ptr<Buzz>`가 생성되고 반환될 것입니다.

지금까지 move-only-type을 반환하는 방법에 대해서 배웠습니다. 그럼 move-only argument를 전달 받으려면 어떻게 해야할까요? 정답은 대부분의 경우에 별도의 조치를 취하지 않아도 잘 동작한다는 것입니다. 혹시나 문제가 되는 부분이 있다고 해도 compile error를 통해 미리 확인할 수 있을 것입니다. `Return`, [lambda, functor](cook_book.md#functionsfunctorslambda를-action으로-사용하기)도 언제든 사용할 수 있습니다.

```cpp
  using ::testing::Unused;

  EXPECT_CALL(mock_buzzer_, ShareBuzz(NotNull(), _)).WillOnce(Return(true));
  EXPECT_TRUE(mock_buzzer_.ShareBuzz(MakeUnique<Buzz>(AccessLevel::kInternal)),
              0);

  EXPECT_CALL(mock_buzzer_, ShareBuzz(_, _)).WillOnce(
      [](std::unique_ptr<Buzz> buzz, Unused) { return buzz != nullptr; });
  EXPECT_FALSE(mock_buzzer_.ShareBuzz(nullptr, 0));
```

`WithArgs`, `WithoutArgs`, `DeleteArg`, `SaveArg`를 비롯한 많은 built-in action들이 이미 move-only argument를 받을 수 있도록 변화하고 있습니다. 다만 아직 구현이 완료된 것은 아니기 때문에 사용중에 문제가 발생한다면 이슈로 등록해 주시기 바랍니다.

`DoAll`과 같은 일부 action은 내부적으로 argument의 복사본을 만듭니다. 따라서 복사가 불가능한 object는 `DoAll`에 사용할 수 없습니다. 이런 경우에는 functor를 사용해야 합니다.

##### Legacy : move-only type 해결방법

Move-only argument를 지원하는 기능은 2017년 4월에 gMock에 추가되었습니다. 따라서 그보다 예전 gMock을 사용하고 있는 사용자는 아래와 같은 방법을 사용하시기 바랍니다. (사실 더 이상 필요하지는 않지만 참조용으로 남겨둔 내용입니다.)

아래 예제는 move-only argument를 전달받는 `SharedBuzz()`라는 method를 mocking하는 코드입니다. 이를 위해서는 `SharedBuzz()`의 역할을 대신 수행하기 위한 method를 추가해야 합니다. 즉, 아래 코드의 `DoShareBuzz()`입니다. 그런 후에 `ShareBuzz()` 대신에 `DoShareBuzz()`를 mocking 하면 됩니다.

```cpp
class MockBuzzer : public Buzzer {
 public:
  MOCK_METHOD(bool, DoShareBuzz, (Buzz* buzz, Time timestamp));
  bool ShareBuzz(std::unique_ptr<Buzz> buzz, Time timestamp) override {
    return DoShareBuzz(buzz.get(), timestamp);
  }
};
```

아래 예제는 위의 mock class를 사용하는 코드입니다.

```cpp
  MockBuzzer mock_buzzer_;
  EXPECT_CALL(mock_buzzer_, DoShareBuzz(NotNull(), _));

  // When one calls ShareBuzz() on the MockBuzzer like this, the call is
  // forwarded to DoShareBuzz(), which is mocked.  Therefore this statement
  // will trigger the above EXPECT_CALL.
  mock_buzzer_.ShareBuzz(MakeUnique<Buzz>(AccessLevel::kInternal), 0);
```

#### 컴파일을 빠르게 하기 ####

모든 사람이 동의할지는 모르겠지만, mock class를 컴파일하는 시간의 대부분은 constructor와 destructor를 만드는데 사용됩니다. 왜냐하면 constructor나 destructor에서 expectation 검증과 같은 중요한 일들을 수행하기 때문이기도 하고 더 직접적으로는 mock method가 여러가지 타입에 대응해야 한다면 constructor나 destructor도 각각의 타입에 대해 컴파일되기 때문입니다. 결과적으로 다양한 타입을 사용하는 method가 많을수록 컴파일 속도가 느려질 확률이 큽니다.

만약 사용자가 컴파일 속도로 인해 어려움을 겪고 있다면 mock class의 constructor와 destructor에 구현된 내용을 class body 밖으로 빼내서 별도의 `.cpp` 파일로 이동시키는 것도 괜찮은 옵션 중 하나입니다. 이렇게 하면 mock class가 정의된 헤더파일을 여러 곳에서 `#include` 하더라도 속도가 느려지지 않습니다. 즉, constructor와 destructor가 한 번만 컴파일되기 때문에 그 속도도 빨라질 것입니다.

아래에서 관련 예제들을 확인하도록 하겠습니다.

```cpp
// File mock_foo.h.
...
class MockFoo : public Foo {
 public:
  // Since we don't declare the constructor or the destructor,
  // the compiler will generate them in every translation unit
  // where this mock class is used.

  MOCK_METHOD(int, DoThis, (), (override));
  MOCK_METHOD(bool, DoThat, (const char* str), (override));
  ... more mock methods ...
};
```

먼저 위의 코드는 개선사항을 적용하기 전입니다. Class에 constructor와 destructor가 없기 때문에 컴파일러가 직접 생성해줘야 하며 이를 위해 많은 컴파일 시간이 소모됩니다. 

반면에 아래 코드는 constructor와 destructor를 직접 선언했지만 정의(구현)는 하지 않았습니다.

```cpp
// File mock_foo.h.
...
class MockFoo : public Foo {
 public:
  // The constructor and destructor are declared, but not defined, here.
  MockFoo();
  virtual ~MockFoo();

  MOCK_METHOD(int, DoThis, (), (override));
  MOCK_METHOD(bool, DoThat, (const char* str), (override));
  ... more mock methods ...
};
```

그런 후에 constructor와 destructor의 정의를 `mock_foo.cpp`라는 별도 파일에 구현하면 컴파일속도를 개선할 수 있습니다.

```cpp
// File mock_foo.cc.
#include "path/to/mock_foo.h"

// The definitions may appear trivial, but the functions actually do a
// lot of things through the constructors/destructors of the member
// variables used to implement the mock methods.
MockFoo::MockFoo() {}
MockFoo::~MockFoo() {}
```

#### 검증을 바로 수행하기 ####

Mock object는 자신이 소멸되는 시점에 연관된 모든 expectation의 수행결과를 종합하여 알려줍니다. 이런 방식을 통해 사용자가 직접 모든 expectation이 만족되었는지 확인해야하는 수고를 덜어줍니다. 다만 이것은 어디까지나 mock object가 정상적으로 소멸된다는 가정하에서 의미가 있는 내용입니다.

만약, mock object가 소멸되지 않는다면 어떤일이 발생할까요? 알 수 없는 bug로 인해서 mock object가 소멸되지 않았다고 가정해봅시다. 그렇게 되면 실제로 문제가 발생했는데도 이를 눈치채지 못하고 넘어갈 수가 있습니다.

이러한 문제를 완화시키기 위해서 heap checker를 사용하는 것도 좋은 방법입니다. Heap checker는 mock object의 소멸여부를 알려주는 역할을 수행합니다. 다만, 사실 heap checker의 구현도 100% 완벽하지는 않기 때문에 gMock은 사용자가 직접 검증을 수행할 수 있는 방법을 제공하고 있습니다. 이런 경우에는 `Mock::VerifyAndClearExpectations(&mock_object)`를 사용하시기 바랍니다.

```cpp
TEST(MyServerTest, ProcessesRequest) {
  using ::testing::Mock;

  MockFoo* const foo = new MockFoo;
  EXPECT_CALL(*foo, ...)...;
  // ... other expectations ...

  // server now owns foo.
  MyServer server(foo);
  server.ProcessRequest(...);

  // In case that server's destructor will forget to delete foo,
  // this will verify the expectations anyway.
  Mock::VerifyAndClearExpectations(foo);
}  // server is destroyed when it goes out of scope here.
```

**Tip:** `Mock::VerifyAndClearExpectations()`은 검증의 성공여부를 `bool` 타입으로 반환합니다. 따라서 해당 mock object의 검증결과를 `ASSERT_TRUE()`를 통해서 다시 확인하는 것도 가능합니다.

#### Check Point 사용하기 ####

어떤 mock object에 설정한 다양한 내용들을 "reset" 하고 싶을 때도 있습니다. 그러한 시점을 "check point"라고 부릅니다. 즉, 설정된 expectation들이 만족되었는지 한 차례 확인한 후에 다시 새로운 expectation들을 지정하는 것입니다. 이러한 방법은 mock object가 "phases" 개념 위에서 동작하도록 만들어 줍니다. 쉽게 말해서 단계별로 검증해야 할 항목들을 분류하는 것이 가능해집니다.

가능한 예상 시나리오는 먼저 `SetUp()`에서 1차적인 검증을 수행한 후에 개별 `TEST_F`를 정의할 때는 `SetUp()`에서 사용한 기존의 expectation들을 초기화하고 새롭게 지정하는 것입니다.

위에서 확인한 것처럼 `Mock::VerifyAndClearExpectations()`함수는 호출되는 시점에 바로 검증을 수행하므로 이러한 경우에 사용하기 적합합니다. 혹시, `ON_CALL()`을 사용해서 default action을 지정했다가 특정시점에 변경하는 방법을 사용하고 있다면 `Mock::VerifyAndClear(&mock_object)`를 사용하기를 추천합니다. 이 함수는 `Mock::VerfiyAndClearExpectations(&mock_object)`와 동일한 동작을 함과 동시에 추가적으로 `ON_CALL()`을 통해 설정한 내용도 초기화 해주기 때문입니다.

또 다른 방법은 expectation을 sequence안에 두는 것입니다. 그런 다음 dummy "check-point"를 만들어서 검증하려는 함수의 호출순서를 강제화하기 위해 사용할 수 있습니다. 예를 들어 아래코드를 검증한다고 해봅시다.

```cpp
  Foo(1);
  Foo(2);
  Foo(3);
```

위의 function들을 통해서 검증하려는 것은 `Foo(1)`과 `Foo(3)`이 `mock.Bar("a")`를 호출해야 한다는 것과 `Foo(2)`는 아무것도 호출하지 않기를 바란다는 것입니다. 이제 아래처럼 구현하면 됩니다.

```cpp
using ::testing::MockFunction;

TEST(FooTest, InvokesBarCorrectly) {
  MyMock mock;
  // Class MockFunction<F> has exactly one mock method. It is named
  // Call() and has type F.
  MockFunction<void(string check_point_name)> check;
  {
    InSequence s;

    EXPECT_CALL(mock, Bar("a"));
    EXPECT_CALL(check, Call("1"));
    EXPECT_CALL(check, Call("2"));
    EXPECT_CALL(mock, Bar("a"));
  }
  Foo(1);
  check.Call("1");
  Foo(2);
  check.Call("2");
  Foo(3);
}
```

위의 expectation은 첫번째 `Bar("a")`가 check point "1" 이전에 수행되기를 기대하고 두번째 `Bar("a")`는 check point "2" 이후에 수행되기를 기대합니다. check point "1" 과 check point "2" 사이에는 `Bar("a")` 가 호출되면 안됩니다. 이렇게 명시적으로 check point를 만들어서 `Bar("a")`가 3개의 `Foo()` 호출 중 어느것에 매칭되어야 하는지 표현할 수 있습니다.

#### Destructor Mocking하기 ####

때때로 mock object가 원하는 시점에 딱 소멸되기를 바랄 수 있습니다. 이를 위해서는 destructor가 언제 호출되는지 확인하는 것이 가장 좋은 방법일 것입니다. 그럼 mock object가 `bar->A()`, `bar->B()` 호출의 중간시점에 소멸되기를 기대한다고 해보겠습니다. Mock function이 호출되는 순서에 대해서는 이미 배웠기 때문에 destructor를 mocking하는 방법만 배우면 구현할 수 있을 것입니다.

한 가지만 제외하면 그다지 어렵지 않습니다. 먼저 destructor는 일반적인 함수와는 문법이 좀 다릅니다. 따라서 `MOCK_METHOD`를 사용할 수가 없습니다.

```cpp
MOCK_METHOD(void, ~MockFoo, ());  // Won't compile!
```

좋은 소식은 간단한 패턴을 적용하면 동일한 효과를 얻을 수 있다는 것입니다. 먼저 `Die()`라는 mock function을 하나 생성합니다. 그런 다음 `Die()`를 destructor에서 호출하도록 구현합니다.

```cpp
class MockFoo : public Foo {
  ...
  // Add the following two lines to the mock class.
  MOCK_METHOD(void, Die, ());
  virtual ~MockFoo() { Die(); }
};
```

`Die()`라는 이름을 이미 사용중이면 다른 이름을 사용해도 됩니다. 이제 끝났습니다. `MockFoo` class의 destructor가 호출되는 시점을 확인할 수 있게 되었습니다. 아래와 같이 구현하면 `Die()`가 `bar->A()`, `bar->B()` 중간에 호출되는지를 검증할 수 있습니다.

```cpp
  MockFoo* foo = new MockFoo;
  MockBar* bar = new MockBar;
  ...
  {
    InSequence s;

    // Expects *foo to die after bar->A() and before bar->B().
    EXPECT_CALL(*bar, A());
    EXPECT_CALL(*foo, Die());
    EXPECT_CALL(*bar, B());
  }
```

#### gMock과 Thread 사용하기 ####

사실 **unit** test는 single-threaded context에 구현하는 것이 제일 좋습니다. 그렇게 해야 race condition과 deadlock을 피할 수 있고, 디버깅하기가 훨씬 쉽기 때문입니다.

그러나 실제로 많은 프로그램은 multi-threaded context로 구현되어 있고 이를 위한 테스트도 필요한 것이 사실입니다. gMock은 관련한 테스트 방법들도 제공하고 있습니다.

먼저 Mock을 사용하는 과정을 떠올려 봅시다.

1. `foo`라는 mock object를 생성합니다.
2. `ON_CALL()` 또는 `EXPECT_CALL()`을 이용해서 `foo`의 method들에 기대하는 내용(default action, expectation)을 지정합니다.
3. 테스트 대상코드가 `foo`의 method를 실제로 호출합니다.
4. (선택사항) 검증하고 mock을 초기화합니다.
5. Mock을 직접 소멸시키거나 테스트 대상코드에서 소멸시키도록 합니다. 이제 destructor가 검증을 수행하게 됩니다.

이러한 다섯 단계를 진행하는 과정에서 아래 규칙들만 잘 지켜진다면 multi-threads를 적용하는데도 큰 무리는 없습니다.

- 테스트 코드는 하나의 thread에서 시작합니다.(테스트 대상코드는 아닐수도 있지만)
- 위 1단계에서 동기화도구(lock 종류)는 사용하지 않습니다.
- 위 2단계, 5단계를 수행할 때, 다른 thread가 `foo`에 접근하지 못하도록 합니다.
- 위 3단계, 4단계는 원하는 환경으로 수행할 수 있습니다.(single thread 혹은 multi-threads) gMock이 내부적으로 동기화 해주기 때문에 테스트코드에서 필요한 것이 아니라면 굳이 직접 동기화를 구현할 필요는 없습니다.

규칙 위반에 대해서는 미정의 동작이기 때문에 문제가 발생할 수 있습니다. 예를 들어 `foo`의 어떤 method에 대해 expectation을 설정하고 있을 때, 다른 thread가 해당 method를 호출하면 안됩니다. Multi-threaded context에서는 이러한 내용들을 항상 주의하시기 바랍니다.

다음으로 gMock은 어떤 mock function의 action이 해당 mock function을 호출한 thread와 동일한 thread에서 수행되는 것을 보장합니다. 아래 예제를 보겠습니다.

```cpp
  EXPECT_CALL(mock, Foo(1))
      .WillOnce(action1);
  EXPECT_CALL(mock, Foo(2))
      .WillOnce(action2);
```

위의 코드에서 `Foo(1)`이 thread #1에서 호출되고 `Foo(2)`가 thread #2에서 호출된다면 `action1`은 thread #1에서 수행되고 `action2`는 thread #2에서 수행됨이 보장됩니다.

gMock은 같은 sequence에 속한 action들이 서로 다른 thread에서 수행되는 것을 *허용하지 않습니다.* 만약 그렇게 되면 action간에 협력이 필요한 경우에 deadlock을 유발할 수도 있기 때문입니다. 반대로 생각하면 위 예제의 `action1`과 `action2`에는 sequence를 사용하지 않았기 때문에 서로 다른 thread에서 수행해도 된다는 것을 의미하기도 합니다. 이러한 내용들과는 별개로 사용자의 구현 과정에서 발생하는 동기화 문제들에 대해서는 사용자가 직접 동기화 로직을 추가하고 thread-safe로 만들어줘야 합니다.

마지막으로 `DefaultValue<T>`를 사용하면 현재 test program에서 살아있는 모든 mock object에 영향을 준다는 것입니다. 따라서 multiple threads 환경이거나 수행중인 action이 있을 때는 사용하지 않는 것이 좋습니다.

#### gMock이 출력할 정보의 양 조절하기 ####

gMock은 잠재적으로 error 가능성이 있는 부분에 대해 warning message를 출력해 줍니다. 대표적인 예로 uninteresting call이 있습니다. 즉, expectation이 없는 mock function에 대해서 error로 여기지는 않지만 사용자가 실수했을 수도 있음을 알리기 위해 warning message로 알려주는 것입니다. 이 때, 해당 uninteresting function에 전달된 argument와 return value 등의 정보를 출력해줌으로써 사용자는 이것이 실제로 문제인지 아닌지 다시 한 번 검토해 볼 수 있습니다.

만약, 현재 테스트코드에 문제가 없다고 생각되는 순간이 오면 이와 같은 정보가 달갑지 않을 것입니다. 출력물이 지저분해지기 때문이죠. 하지만 반대로 테스트코드를 디버깅하고 있거나 googletest를 통해 테스트대상의 동작을 공부하는 중이라면 그러한 정보가 많을 수록 좋을 것입니다. 쉽게 말해서 사람마다 필요한 정보의 양의 다르다는 것입니다.

따라서 gMock은 이러한 정보의 양을 조절하는 방법을 제공합니다. 실행시점에 `--gmock_verbose=LEVEL`이라는 flag를 사용하면 됩니다. 여기서 `LEVEL`에는 3개의 값을 지정할 수 있습니다.

- `info`: gMock은 warning, error등의 모든 정보를 최대한 자세하게 출력합니다. 또한, `ON_CALL()/EXPECT_CALL()` 사용에 대한 log도 출력해 줍니다.
- `warning`: gMock은 warning, error를 출력해 줍니다. 단, 그 내용이 `info` 모드보다 자세하지는 않습니다. 현재 gMock의 기본설정입니다.
- `error`: gMock은 error만 출력해줍니다.

Flag 외에 코드에서 아래와 같은 변수를 수정해도 동일한 기능을 적용할 수 있습니다.

```cpp
  ::testing::FLAGS_gmock_verbose = "error";
```

이제, 어떤 모드가 본인에게 제일 적합한지 잘 판단하고 사용하기 바랍니다.

#### Mock 호출을 자세하게 들여다보기 ####

gMock은 mock function에 지정한 expectation이 만족되지 않으면 이를 알려줍니다. 그러나 왜 그런 문제가 발생했는지가 궁금한 사용자도 있을 것입니다. 예를 들어 matcher를 사용할 때 어떤 오타가 있었는지? `EXPECT_CALL()`의 순서가 틀렸는지? 혹은 테스트 대상코드에 문제가 있었는지? 등등이 있을텐데요. 어떻게 하면 이런 내용을 알아낼 수 있을까요?

마치 X-ray를 보듯이 모든 `EXPECT_CALL()`과 mock method 호출을 추적할 수 있다면 좋지 않을까요? 각각의 호출에서 실제 전달된 argument도 궁금하고 어떤 `EXPECT_CALL()`과 매칭되었는지도 궁금합니다. 어떻게 해야할까요?

`--gmock_verbose=info` flag를 통해서 테스트코드에도 X-ray를 사용할 수 있습니다. 아래의 코드를 먼저 보겠습니다.

```cpp
#include "gmock/gmock.h"

using testing::_;
using testing::HasSubstr;
using testing::Return;

class MockFoo {
 public:
  MOCK_METHOD(void, F, (const string& x, const string& y));
};

TEST(Foo, Bar) {
  MockFoo mock;
  EXPECT_CALL(mock, F(_, _)).WillRepeatedly(Return());
  EXPECT_CALL(mock, F("a", "b"));
  EXPECT_CALL(mock, F("c", HasSubstr("d")));

  mock.F("a", "good");
  mock.F("a", "b");
}
```

이제 위 test program을 `--gmock_verbose=info` flag와 함께 실행하면 아래와 같은 정보가 출력될 것입니다.

```bash
[ RUN       ] Foo.Bar

foo_test.cc:14: EXPECT_CALL(mock, F(_, _)) invoked
Stack trace: ...

foo_test.cc:15: EXPECT_CALL(mock, F("a", "b")) invoked
Stack trace: ...

foo_test.cc:16: EXPECT_CALL(mock, F("c", HasSubstr("d"))) invoked
Stack trace: ...

foo_test.cc:14: Mock function call matches EXPECT_CALL(mock, F(_, _))...
    Function call: F(@0x7fff7c8dad40"a",@0x7fff7c8dad10"good")
Stack trace: ...

foo_test.cc:15: Mock function call matches EXPECT_CALL(mock, F("a", "b"))...
    Function call: F(@0x7fff7c8dada0"a",@0x7fff7c8dad70"b")
Stack trace: ...

foo_test.cc:16: Failure
Actual function call count doesn't match EXPECT_CALL(mock, F("c", HasSubstr("d")))...
         Expected: to be called once
           Actual: never called - unsatisfied and active
[  FAILED  ] Foo.Bar
```

여기서 3번째 `EXPECT_CALL()`의 `"c"`가 원래 `"a"`를 쓰려다가 잘못 쓴 오타라고 가정해봅시다. 즉, `mock.F("a", "good")`라는 호출은 원래 3번째 `EXPECT_CALL()`과 매칭되었어야 합니다. 이제 위에 출력된 정보를 보면 `mock.F("a", "good")`이 첫번째 `EXPECT_CALL()`과 매칭되어 문제가 됐음을 바로 알 수 있습니다. 기존에는 불가능했던 일입니다.

만약, mock call trace은 보고싶지만 stack trace는 보고 싶지 않다면 test program을 실행할 때 flag 2개를 조합(`--gmock_verbose=info --gtest_stack_trace_depth=0`)하여 실행하시기 바랍니다.

#### Emacs에서 테스트 실행하기 ####

Emacs에서 `M-x google-complie` 명령을 통해 테스트를 구현하고 실행하는 사용자들은 googletest 혹은 gMock error가 발생했을 때, 관련 소스파일의 위치가 강조되어 출력되는걸 볼 수 있습니다. 그 때, 강조된 부분에서 `<Enter>`를 누르면 바로 해당 위치로 이동하게 될 것입니다. 그리고 `C-x`를 누르게 되면 다음 error 위치로 바로 이동할 수 있습니다.

`~/.emacs` 파일에 아래 내용을 추가하면 좀 더 편리하게 사용할 수 있습니다.

```bash
(global-set-key "\M-m"  'google-compile)  ; m is for make
(global-set-key [M-down] 'next-error)
(global-set-key [M-up]  '(lambda () (interactive) (next-error -1)))
```

이제 `M-m`을 눌러서 빌드를 시작하고, `M-up` / `M-down`을 눌러서 error 간에 이동할 수 있습니다. 더불어 빌드할 때마다 테스트도 같이 수행하고 싶다면 `M-m` 명령의 build command 설정부분에 `foo_test.run` 혹은 `runtests`라는 내용을 추가하시기 바랍니다.

### Extending gMock ###

#### 새로운 Matcher 구현하기 ####

WARNING: gMock은 matcher가 언제 몇 번 호출될지를 보장하지 않습니다. 따라서 모든 matcher는 순수하게 기능적인 동작만 수행하도록 구현해야 합니다. 즉, 프로그램 내의 다른 정보에 대한 side effect나 의존성을 가지면 안된다는 것입니다. 이와 관련된 내용은 [여기](cook_book.md#matcher는-부수효과side-effect를-가지면-안됩니다)에서 좀 더 자세하게 설명하고 있습니다.

gMock이 제공하는 `MATCHER*` macro는 사용자가 새로운 matcher를 만들 수 있도록 도와줍니다. 기본적인 문법은 아래와 같습니다.

```cpp
MATCHER(name, description_string_expression) { statements; }
```

위 macro는 `statements`를 실행하는 matcher를 만들어 줍니다. 이렇게 생성된 matcher는 성공, 실패를 알려야 하기 때문에 항상 `bool`값을 반환합니다. 또한 macro에 전달하는 `statements`에는 특수 argument들도 사용할 수 있습니다. 예를 들어 `arg`, `arg_type`은 각각 mock function에서 matcher로 전달되는 argument와 argument type을 의미합니다.

`description_string_expression`은 `string` 타입으로서 matcher가 무엇을 해야하는지 알려주는 용도로 사용되며 그 내용이 failure message를 출력할 때도 역시 사용됩니다. 더불어 이 부분을 구현할 때 `negation`이라는 `bool` 타입의 특수 argument를 사용할 수 있는데 이것은 matcher의 성공, 실패 여부에 따라 관련정보를 다르게 출력하기 위한 용도로 주로 사용합니다. 아래에서 관련예제와 함께 다시 설명하겠습니다.

사용편의성을 위해서 `description_string_expression`에는 (`""`)와 같이 빈 문자열도 사용 가능합니다. 만약 이렇게 (`""`)를 사용하면 gMock은 `name`을 조금 변경하여 description을 대체해줍니다.

아래에 간단한 matcher를 정의한 예제가 있습니다.

```cpp
MATCHER(IsDivisibleBy7, "") { return (arg % 7) == 0; }
```

이렇게 정의된 matcher는 built-in matcher을 사용하듯이 동일한 방법으로 사용하면 됩니다.

```cpp
  // Expects mock_foo.Bar(n) to be called where n is divisible by 7.
  EXPECT_CALL(mock_foo, Bar(IsDivisibleBy7()));
```

`EXPECT_THAT`을 사용하는 것도 가능합니다.

```cpp
  using ::testing::Not;
  ...
  // Verifies that two values are divisible by 7.
  EXPECT_THAT(some_expression, IsDivisibleBy7());
  EXPECT_THAT(some_other_expression, Not(IsDivisibleBy7()));
```

위 assertion들이 실패하면, 아래와 같은 failure message가 출력될 것입니다.

```bash
  Value of: some_expression
  Expected: is divisible by 7
    Actual: 27
  ...
  Value of: some_other_expression
  Expected: not (is divisible by 7)
    Actual: 21
```

Expected 부분에서 볼 수 있는 `"is divisible by 7"`과 `"not (is divisible by 7)"`은 matcher의 이름인 `IsDivisibleBy7`이라는 이름을 통해서 자동적으로 생성되는 내용입니다.

만약, 자동으로 생성되는 출력문에 만족하지 못하는 경우에는 위에서 설명한 것처럼 `description_string_expression`를 사용자가 재정의하면 됩니다. 이 때 바로 `negation`을 사용할 수 있습니다. 아래 예제가 있습니다.

```cpp
MATCHER(IsDivisibleBy7,
        absl::StrCat(negation ? "isn't" : "is", " divisible by 7")) {
  return (arg % 7) == 0;
}
```

또한, `result_listener`라는 숨겨진 argument를 사용하면 추가적인 정보를 출력하게 할 수 있습니다. 예를 들어서 `IsDivisibleBy7`이 수행한 동작에 대한 부연설명을 아래처럼 추가하는 것이 가능합니다.

```cpp
MATCHER(IsDivisibleBy7, "") {
  if ((arg % 7) == 0)
    return true;

  *result_listener << "the remainder is " << (arg % 7);
  return false;
}
```

이제 바뀐 출력정보를 확인할 수 있습니다.

```bash
  Value of: some_expression
  Expected: is divisible by 7
    Actual: 27 (the remainder is 6)
```

`result_listener`는 사용자에게 도움을 주는 내용이라면 무엇이든 출력해도 상관 없습니다. 다만, 한 가지 기억해야 할 점은 매칭이 성공이라면 왜 성공했는지에 대한 정보까지 알려줘야 유용하다는 것입니다. 특히 해당 matcher가 `Not()`과 함께 사용되었을 때 도움이 될 것입니다. 마지막으로 argument의 값을 출력하는 부분은 gMock에 이미 구현되어 있으므로 따로 구현하지 않아도 됩니다.

**Notes:** `arg_type`은 matcher를 사용하는 context 혹은 compiler에 따라서 달라질 수 있긴 하지만 사용자가 특별히 신경 쓸 필요는 없습니다. 이러한 동작방식은 matcher를 polymorphic하게 사용하기 위해 채택되었습니다. 예를 들어 `IsDivisibleBy7()`과 같은 예제에서 `(arg % 7)`을 계산하고 그 결과를 `bool` 타입으로 반환할 수만 있다면 `arg_type`은 어떤 타입이 되더라도 괜찮습니다. 이를 통해서 (암시적인 형변환까지 포함해서) `%` 연산을 수행할 수 있는 타입이라면 모두 사용할 수 있는 matcher가 만들어지는 것입니다. 예제코드의 `Bar(IsDivisibleBy7())`를 보면, `Bar()`라는 method는 `int` 타입 argument를 전달받고 있기 때문에 `arg_type`도 역시 `int`로 결정됩니다. 이 때, argument가 `unsigned long`이라고 해도 문제가 없는 것입니다. 단지 `arg_type`이 `unsigned long`이 되는 것 뿐입니다.

#### 새로운 Parameterized Matcher 구현하기 ####

Matcher 자체적으로 parameter를 가지길 원할 수도 있습니다. 본격적인 설명에 앞서 matcher에서 사용하는 argument와 parameter라는 용어를 구분할 필요가 있습니다. 먼저, argument란 mock function으로부터 matcher로 전달되는 값들을 의미합니다. 그러나 parameter는 matcher 자신이 스스로 관리하고 사용하려는 목적의 값들을 의미합니다. 즉, parameter는 mock function과 직접적인 연관은 없습니다. 여기서는 matcher에 parameter를 사용하려면 어떻게 구현해야 하는지 소개합니다. 그 시작은 아래의 macro입니다.

```cpp
MATCHER_P(name, param_name, description_string) { statements; }
```

위의 `MATCHER_P`를 보면 `description_string`에는 string를 전달할 수 있습니다. `""`도 역시 사용 가능합니다. 더불어 `description_string`을 구현할 때는 `negation`, `param_name`과 같은 특수 argument를 사용하는 것도 가능합니다.

아래는 `MATHCER_P`를 사용해 간단한 parameterized matcher를 구현한 예제입니다.

```cpp
MATCHER_P(HasAbsoluteValue, value, "") { return abs(arg) == value; }
```

이렇게 정의된 `HasAbsoluteValue`라는 matcher는 기존의 matcher처럼 사용하기만 하면 됩니다. 

```cpp
  EXPECT_THAT(Blah("a"), HasAbsoluteValue(n));
```

여기서 `10`이라는 값이 바로 matcher로 전달되는 parameter이며 parameter name(`param_name`)이 `value`가 되는 것입니다. 실행하면 아래와 같은 출력문을 확인할 수 있습니다.

```bash
  Value of: Blah("a")
  Expected: has absolute value 10
    Actual: -9
```

출력문의 Expected 부분에 matcher description과 parameter가 함께 출력되는 것을 볼 수 있습니다. 이렇게 출력되는 결과를 통해서 사용자가 문제를 이해하게 쉽게 도와주고 있습니다.

`MATCHER_P`의 `statements`를 구현할 때는 parameter의 타입을 참조하는 것도 가능합니다. 예를 들어 `value`라는 parameter의 타입을 확인하려면 `value_type`을 사용하면 됩니다.

또한, gMock은 parameter를 여러개 사용하기 위한 macro도 지원합니다. 각 macro의 이름은 `MATCHER_P2`, `MATCHER_P3`, ... 와 같습니다. 단, `MATCHER_P10`까지만 지원하기 때문에 최대로 사용할 수 있는 paratmeter의 개수는 10개입니다.

```cpp
MATCHER_Pk(name, param_1, ..., param_k, description_string) { statements; }
```

위에서 `description_string`은 matcher의 **instance**마다 다를 수 있음을 기억하기 바랍니다. 위에서 얘기했듯이 `description_string`이 `param_name`을 참조하기 때문에 `param_name`의 값에 따라 출력되는 내용이 달라질 수 있다는 의미입니다. 그리고 필요하다면 `description_string`에서 parameter를 참조하여 출력하는 방식을 직접 구현할 수도 있습니다.

예제를 보겠습니다.

```cpp
using ::testing::PrintToString;
MATCHER_P2(InClosedRange, low, hi,
           absl::StrFormat("%s in range [%s, %s]", negation ? "isn't" : "is",
                           PrintToString(low), PrintToString(hi))) {
  return low <= arg && arg <= hi;
}
...
EXPECT_THAT(3, InClosedRange(4, 6));
```

위의 코드는 `low`, `hi`라는 2개의 `param_name`을 사용하고 있으며 `description_string`에서 이것들을 참조하고 있습니다. 추가로 `negation`도 참조가능함을 볼 수 있는데요, `negation`의 의미가 *부정*이기 때문에 값이 `false`일 때가 곧 매칭 성공을 의미한다는 점은 주의하기 바랍니다.

```bash
  Expected: is in range [4, 6]
```

만약, 위와 같이 `description_string`을 직접 제공하지 않고, `""`를 전달한다면 gMock이 출력문의 Expected 부분을 자동으로 출력해 줍니다. 이 때에는 matcher name을 단어별로 먼저 출력하고 다음으로 parameter에 저장된 값을 출력해 줍니다.

```cpp
  MATCHER_P2(InClosedRange, low, hi, "") { ... }
  ...
  EXPECT_THAT(3, InClosedRange(4, 6));
```

위와 같이 `description_string`에 `""`를 전달해서 만든 matcher는 아래와 같은 출력문을 보여줍니다. 이렇게 출력물에 matcher name을 이용하기 때문에 사용자들은 matcher name을 신중하게 고민해서 작성할 필요가 있습니다.

```bash
  Expected: in closed range (4, 6)
```

`MATCHER_Pk` macro는 parameter의 개수에 따라 아래와 같이 일반화된 형태로 표현할 수 있습니다.

```cpp
MATCHER_Pk(Foo, p1, ..., pk, description_string) { ... }
```

단, 위의 macro는 축약형이며 실제로는 아래와 같은 모습이긴 합니다.

```cpp
template <typename p1_type, ..., typename pk_type>
FooMatcherPk<p1_type, ..., pk_type>
Foo(p1_type p1, ..., pk_type pk) { ... }
```

바로 위 예제에서 정의된 matcher를 `Foo(v1, .., vk)`와 같이 구현해서 사용한다고 해봅시다. 이 때, `v1`, ... , `vk`의 타입은 일반적인 template function의 동작방식처럼 compiler에 의해 추론되는데요. 만약, 이러한 타입추론이 불편한 경우에는 명시적으로 직접 지정해도 괜찮습니다. 예를 들면 `Foo<long, bool>(5, false)`와 같이 구현하면 됩니다. 다만, 이것은 어디까지나 parameter type에 대한 얘기이며 argument type(`arg_type`)은 matcher를 사용하는 context에 따라 달라지게 됩니다. Parameter와 argument를 확실히 구별하시기 바랍니다.

`Foo(p1, ..., pk)`와 같은 matcher의 결과를 반환할 때는 `FooMatcherPk<p1_type, ..., pk_type>`을 사용해야 합니다. 이런 구현은 여러개의 matcher를 묶어서 사용할 떄 유용합니다. 단, parameter가 없거나 한 개만 있는 matcher는 조금 다르게 구현합니다. 예를 들어 parameter가 없는 matcher `Foo()`가 있다면 이것을 반환할때는 `FooMatcher`라는 타입으로 반환해야 합니다. 더불어 `Foo(p)`와 같이 parameter가 1개 있는 경우에는 `FooMatcherP<p_type>`라는 타입으로 반환하면 됩니다.

Matcher template을 참조타입 parameter로 초기화할 수도 있지만, 포인터 타입을 사용하는 것이 가독성 측면에서 더 좋습니다. 왜냐하면 참조타입으로 전달하게 되면 failure message가 출력되었을 때 가리키는 대상의 주소는 볼 수 없고 값만 볼 수 있기 때문입니다. 

마지막으로 이미 눈치챘겠지만 matcher는 parameter의 개수에 따라 overloaded function으로 확장됩니다.

```cpp
MATCHER_P(Blah, a, description_string_1) { ... }
MATCHER_P2(Blah, a, b, description_string_2) { ... }
```

네, 상당히 많은 내용을 다뤘는데요. 지금까지 배운 것처럼 `MATCHER*` macro를 이용하면 빠르고 편리하게 사용자만의 matcher를 만들 수 있을 것입니다. 주의할 점은 새로운 matcher를 구현하는 방법이 `MATCHER*`만 있는 것은 아니라는 점입니다. 이렇게 새로운 기능을 구현해야 할 때는 가능한 여러가지 방법들을 고민해보고 적절한 방법을 선택하는 것이 중요합니다. 특히, 바로 다음에 설명하는 `MatcherInterface`, `MakePolymorphicMatcher()`도 꽤 괜찮은 방법들입니다. 물론 `MATCHER*`에 비해서 해야할 일이 많긴 하지만 그와 동시에 좀 더 세밀한 작업도 가능합니다. 예를 들어 타입지정을 다양하게 조절할 수도 있고 compile error message도 좀 더 깔끔합니다. 마지막으로 matcher를 parameter 개수 뿐만 아니라 parameter 타입에 대해서도 overload 할 수 있게 도와줄 것입니다.

#### 새로운 Monomorphic Matcher 구현하기 ####

gMock에서 제공하는 `::testing::MatcherInterface<T>`를 상속받고 구현하면 `T` 타입의 argument를 전달받는 matcher를 만들 수 있습니다. 이렇게 만들어진 matcher는 2가지 일을 가능하게 합니다. 먼저, argument type(`T`)과 argument value를 같이 검사할 수 있습니다. 다음으로는 출력문을 자유롭게 구현할 수 있습니다. 즉, expectation을 만족하지 못했을 때 어떤 값들을 비교했는지와 같은 정보를 보다 상세히 알려줄 수 있습니다.

이를 위한 interface인 `MatcherInterface<T>`는 아래처럼 선언되어 있습니다.

```cpp
class MatchResultListener {
 public:
  ...
  // Streams x to the underlying ostream; does nothing if the ostream
  // is NULL.
  template <typename T>
  MatchResultListener& operator<<(const T& x);

  // Returns the underlying ostream.
  ::std::ostream* stream();
};

template <typename T>
class MatcherInterface {
 public:
  virtual ~MatcherInterface();

  // Returns true if the matcher matches x; also explains the match
  // result to 'listener'.
  virtual bool MatchAndExplain(T x, MatchResultListener* listener) const = 0;

  // Describes this matcher to an ostream.
  virtual void DescribeTo(::std::ostream* os) const = 0;

  // Describes the negation of this matcher to an ostream.
  virtual void DescribeNegationTo(::std::ostream* os) const;
};
```

새로운 matcher를 만들고 싶은데 `Truly()`가 맘에 들지 않나요? (`Truly()`는 function이나 functor를 matcher로 변환해주는 기능입니다.) `Truly()`가 생성해주는 출력문으로 부족한가요? 앞으로는 2단계만 거치면 원하는 matcher를 좀 더 자유롭게 만들 수 있습니다. 첫번째 단계는 `MatcherInteface<T>`를 상속받아 구현하는 것이고 두번째 단계는 해당 matcher의 instance를 생성하기 위한 factory function을 정의하는 것입니다. 여기서 두번째 단계는 matcher를 편리하게 사용하기 위해 필요한 것이지 꼭 해야하는 것도 아닙니다.

그럼 예제를 보겠습니다. 아래코드는 어떤 `int` 타입 값이 7로 나누어 떨어지는지 검사하는 matcher를 구현한 것입니다.

```cpp
using ::testing::MakeMatcher;
using ::testing::Matcher;
using ::testing::MatcherInterface;
using ::testing::MatchResultListener;

class DivisibleBy7Matcher : public MatcherInterface<int> {
 public:
  bool MatchAndExplain(int n,
                       MatchResultListener* /* listener */) const override {
    return (n % 7) == 0;
  }

  void DescribeTo(::std::ostream* os) const override {
    *os << "is divisible by 7";
  }

  void DescribeNegationTo(::std::ostream* os) const override {
    *os << "is not divisible by 7";
  }
};

Matcher<int> DivisibleBy7() {
  return MakeMatcher(new DivisibleBy7Matcher);
}

...
  EXPECT_CALL(foo, Bar(DivisibleBy7()));
```

위의 예제코드에서 보이듯이 출력문은 기본적으로 `Describe*` 함수에 구현합니다. 그러나 `MatchAndExplain()`에서 직접 출력문 내용을 변경하고 싶을 수도 있습니다. 그런 경우에는 `listener` argument를 사용하면 출력문에 내용을 추가할 수 있습니다. 예제코드는 아래와 같습니다.

```cpp
class DivisibleBy7Matcher : public MatcherInterface<int> {
 public:
  bool MatchAndExplain(int n,
                       MatchResultListener* listener) const override {
    const int remainder = n % 7;
    if (remainder != 0) {
      *listener << "the remainder is " << remainder;
    }
    return remainder == 0;
  }
  ...
};
```

최종적으로 `EXPECT_THAT(x, DivisibleBy7());` 은 아래와 같은 결과를 출력할 것입니다.

```bash
Value of: x
Expected: is divisible by 7
  Actual: 23 (the remainder is 2)
```

#### 새로운 Polymorphic Matcher 구현하기 ####

새로운 matcher를 만드는 방법들을 계속 배웠습니다. 하지만 문제는 `MakeMatcher()`를 통해 만들었던 matcher들은 한가지 타입에만 사용이 가능하다는 것입니다. `Eq(x)`처럼 어떤 타입에도 사용가능한(`==`연산자가 구현되어 있는) matcher는 어떻게 만들 수 있을까요? 이제부터는 *polymorphic* matcher를 정의함으로써 다양한 타입에도 사용가능한 matcher를 만들어 보도록 하겠습니다. 관련 기술들이 `"testing/base/public/gmock-matchers.h"`에 구현되어 있으나 내용이 조금 어려울 수 있습니다.

가장 쉬운 방법은 `MakePolymorphicMatcher()`를 사용하는 것입니다. 아래에 나오는 `NotNull()`이라는 polymorphic matcher 예제를 참조하시기 바랍니다.

```cpp
using ::testing::MakePolymorphicMatcher;
using ::testing::MatchResultListener;
using ::testing::PolymorphicMatcher;

class NotNullMatcher {
 public:
  // To implement a polymorphic matcher, first define a COPYABLE class
  // that has three members MatchAndExplain(), DescribeTo(), and
  // DescribeNegationTo(), like the following.

  // In this example, we want to use NotNull() with any pointer, so
  // MatchAndExplain() accepts a pointer of any type as its first argument.
  // In general, you can define MatchAndExplain() as an ordinary method or
  // a method template, or even overload it.
  template <typename T>
  bool MatchAndExplain(T* p,
                       MatchResultListener* /* listener */) const {
    return p != NULL;
  }

  // Describes the property of a value matching this matcher.
  void DescribeTo(::std::ostream* os) const { *os << "is not NULL"; }

  // Describes the property of a value NOT matching this matcher.
  void DescribeNegationTo(::std::ostream* os) const { *os << "is NULL"; }
};

// To construct a polymorphic matcher, pass an instance of the class
// to MakePolymorphicMatcher().  Note the return type.
PolymorphicMatcher<NotNullMatcher> NotNull() {
  return MakePolymorphicMatcher(NotNullMatcher());
}

...

  EXPECT_CALL(foo, Bar(NotNull()));  // The argument must be a non-NULL pointer.
```

**Note:** polymorphic matcher class를 구현할 때는 `MatcherInterface`를 비롯해 어떤 class도 상속받지 않습니다.(monomorphic matcher와 다른 점입니다.) 그리고 method를 virtual method로 선언할 필요도 없습니다.

그럼에도 monomorphic matcher처럼 `MatchAndExplain()`의 argument인 `listener`를 통해 출력문 내용을 추가하는 것도 가능합니다.

#### 새로운 Cardinality 구현하기 ####

Cardinality(호출횟수)는 `Times()`와 함께 사용되며 gMock에 mock function이 몇 번 호출되기를 기대하는지 알려주는 용도로 사용합니다. 다만, 꼭 정확한 횟수를 지정해야 하는 것은 아닙니다. 예를 들어 `AtLeast(5)` 또는 `Between(2, 4)`와 같이 특정범위를 지정하는 cardinality도 제공하고 있습니다. 

혹시, 기본제공되는 built-in cardinality가 부족하다고 생각된다면 직접 정의하는 것도 가능합니다. 이를 위해서는 `testing` namespace에 있는 아래 interface를 구현하면 됩니다.

```cpp
class CardinalityInterface {
 public:
  virtual ~CardinalityInterface();

  // Returns true iff call_count calls will satisfy this cardinality.
  virtual bool IsSatisfiedByCallCount(int call_count) const = 0;

  // Returns true iff call_count calls will saturate this cardinality.
  virtual bool IsSaturatedByCallCount(int call_count) const = 0;

  // Describes self to an ostream.
  virtual void DescribeTo(::std::ostream* os) const = 0;
};
```

아래 코드는 `CardinalityInterface`를 상속받은 후에 짝수단위로만 호출되기를 원하는 cardinality를 구현한 것입니다.

```cpp
using ::testing::Cardinality;
using ::testing::CardinalityInterface;
using ::testing::MakeCardinality;

class EvenNumberCardinality : public CardinalityInterface {
 public:
  bool IsSatisfiedByCallCount(int call_count) const override {
    return (call_count % 2) == 0;
  }

  bool IsSaturatedByCallCount(int call_count) const override {
    return false;
  }

  void DescribeTo(::std::ostream* os) const {
    *os << "called even number of times";
  }
};

Cardinality EvenNumber() {
  return MakeCardinality(new EvenNumberCardinality);
}

...
  EXPECT_CALL(foo, Bar(3))
      .Times(EvenNumber());
```

#### 새로운 Action 구현하기

Built-in action만으로 부족하다고 느끼는 사용자가 있다면 action을 직접 구현하는 것도 어렵지 않습니다. Action의 signature와 매칭되는 functor class를 정의하면 됩니다.(template도 가능합니다.) 간단한 action을 구현예제를 아래에서 확인하세요.

```c++
struct Increment {
  template <typename T>
  T operator()(T* arg) {
    return ++(*arg);
  }
}
```

같은 방법으로 stateful functor를 구현하는 것도 가능합니다.

```c++
struct MultiplyBy {
  template <typename T>
  T operator()(T arg) { return arg * multiplier; }

  int multiplier;
}

// Then use:
// EXPECT_CALL(...).WillOnce(MultiplyBy{7});
```

##### Legacy : 새로운 Action 구현하기 #####

C++11 이전에는 functor를 기반으로 한 action을 지원하지 않았으며 주로 `ACTION*`이라는 macro를 통해서 action을 정의할 수 있었습니다. 다만, 이제는 새로운 기능으로 대부분 대체되었습니다. 물론 현재도 사용할 수는 있지만 언젠가 compile error가 발생할 수도 있으며 되도록이면 새로운 방법을 사용하기를 권장합니다. 비록 지금은 대체된 기능이긴 하지만 관련 내용을 공유합니다.

먼저, `ACTION*` macro는 아래와 같은 모습을 가지고 있습니다.

```cpp
ACTION(name) { statements; }
```

어떤 namespace 내에서 (class, function 내부는 안됩니다) 위의 macro를 사용하면, `name` 이라는 이름을 가지면서 `statements` 코드를 수행하는 action을 생성해줍니다. 여기서 `statements`의 반환값은 곧 action의 반환값이 됩니다. 또한, `statements`를 구현할 때 mock function의 argument를 참조할 수도 있습니다. 이를 위해서 gMock에서는 mock function의 K번째 인수라는 의미로 `argK`라는 특수 argument를 제공하고 있습니다. 아래에 예제코드가 있습니다.

```cpp
ACTION(IncrementArg1) { return ++(*arg1); }
```

이렇게 정의된 action은 built-in action과 동일한 방식으로 사용하면 됩니다.

```cpp
... WillOnce(IncrementArg1());
```

여기서는 mock function으로부터 전달되는 argument type을 따로 명세하지는 않고 있습니다. 그렇지만 어떤 타입이 전달되든지 코드는 type-safe 합니다. 왜냐하면 예제와 같은 코드는 `*arg1`가 `++` 연산자를 지원하지 않는다면 compile error를 발생시켜주기 때문입니다. 또한 `++(*arg1)`이라는 코드가 mock function의 return type과 맞지 않아도 compile error가 발생할 것입니다.

또 다른 예제를 보면 좀 더 익숙해 질 것입니다.

```cpp
ACTION(Foo) {
  (*arg2)(5);
  Blah();
  *arg1 = 0;
  return arg0;
}
```

위 코드는 먼저 `Foo()`라는 action을 정의합니다. `Foo`는 mock function의 2번째 argument를 받아서 호출합니다. 여기서 2번째 argument는 function pointer이기 때문에 호출이 가능합니다. 계속해서 `Blah()`를 호출하고 1번째 argument에는 0을 대입합니다. 그리고 마지막으로 0번째 argument인 `arg0`을 반환합니다.

`ACTION` macro를 좀 더 편리하게 사용하기 위해서 아래와 같은 pre-defined symbol을 알아두면 편리합니다. 이들은 action statement 안에서 자유롭게 사용할 수 있습니다.

| Pre-defined Symbol | Description                                                   |
| :----------------- | :------------------------------------------------------------ |
| `argK`             | The value of the K-th (0-based) argument of the mock function |
| `argK_type`        | The type of the K-th (0-based) argument of the mock function  |
| `args`             | All arguments of the mock function as a tuple                 |
| `args_type`        | The type of all arguments of the mock function as a tuple     |
| `return_type`      | The return type of the mock function                          |
| `function_type`    | The type of the mock function                                 |

예를 들어 아래와 같은 mock function을 위한 action을 구현한다고 해봅시다.

```cpp
int DoSomething(bool flag, int* ptr);
```

그러면 `ACTION` macro의 `statement`에서 사용가능한 pre-defined symbol들과 그 값은 아래와 같습니다.

| **Pre-defined Symbol** | **Is Bound To**                         |
| :--------------------- | :-------------------------------------- |
| `arg0`                 | the value of `flag`                     |
| `arg0_type`            | the type `bool`                         |
| `arg1`                 | the value of `ptr`                      |
| `arg1_type`            | the type `int*`                         |
| `args`                 | the tuple `(flag, ptr)`                 |
| `args_type`            | the type `::testing::tuple<bool, int*>` |
| `return_type`          | the type `int`                          |
| `function_type`        | the type `int(bool, int*)`              |

##### Legacy : 새로운 Parameterized Action 구현하기 #####

Action macro가 parameter를 받을 수 있도록 구현해야 할 필요도 있을 것입니다. 이를 위해서 `ACTION_P*` 계열 macro를 제공하고 있습니다.

```cpp
ACTION_P(name, param) { statements; }
```

사용방법은 아래와 같습니다.

```cpp
ACTION_P(Add, n) { return arg0 + n; }
```

이제 `Add`라는 action이 parameter를 전달받을 수 있게 되었습니다.

```cpp
// Returns argument #0 + 5.
... WillOnce(Add(5));
```

*argument*와 *parameter*가 여전히 헷갈릴 수 있습니다. 다시 한 번 정리하면 argument는 mock function이 전달받는 것들입니다. 필요에 따라 action으로 전달할 수도 있고, 안하는 것도 가능한 경우입니다. 반면에 parameter는 mock function에서 전달되는게 아니라 테스트코드에서 action으로 직접 전달되는 것들을 의미합니다.

사용자가 parameter type을 직접 제공할 필요는 없습니다. 예를 들어서 사용자가 `ACTION_P*` 계열을 이용해 `param`이라는 parameter를 정의했다면 `param`의 타입은 compiler에 의해 추론되며 이렇게 추론된 것을 다시 gMock에서 `param_type`이라는 명칭으로 제공하여 사용자가 사용할 수 있도록 합니다. 위와 같이 `ACTION_P(Add, n)`라고 구현한 코드에서 `n`의 타입은 compiler와 gMock에 의해 `n_type`이라는 변수로 사용자에게 제공됩니다.

또한, `ACTION_P2`, `ACTION_P3`와 같이 parameter의 개수에 따라 다른 macro를 사용할 수 있습니다.

```cpp
ACTION_P2(ReturnDistanceTo, x, y) {
  double dx = arg0 - x;
  double dy = arg1 - y;
  return sqrt(dx*dx + dy*dy);
}
```

위에서 정의한 action은 아래처럼 사용하면 됩니다.

```cpp
... WillOnce(ReturnDistanceTo(5.0, 26.5));
```

즉, action을 생성을 돕는 가장 기본 macro인 `ACTION`은 parameter의 개수가 하나도 없는 parameterized action이라고 봐도 무방할 것입니다.

더불어 parameterized action은 내부적으로 overloaded function이 된다는 점도 알아두시기 바랍니다.

```cpp
ACTION_P(Plus, a) { ... }
ACTION_P2(Plus, a, b) { ... }
```

#### ACTION으로 전달되는 argument나 parameter의 타입을 제한하기 ####

최대한 간결하고 재사용성을 높이기 위해서 `ACTION*` macro는 mock function의 argument type과 action parameter를 요구하지 않습니다. 대신, compiler가 그런 타입을 추론할 수 있도록 구현해 두었습니다.

다만, 모호한 경우 등을 대비해 argument type을 명시적으로 표현하는 방법도 제공하고 있습니다. 아래 예제를 보겠습니다.

```cpp
ACTION(Foo) {
  // Makes sure arg0 can be converted to int.
  int n = arg0;
  ... use n instead of arg0 here ...
}

ACTION_P(Bar, param) {
  // Makes sure the type of arg1 is const char*.
  ::testing::StaticAssertTypeEq<const char*, arg1_type>();

  // Makes sure param can be converted to bool.
  bool flag = param;
}
```

위의 코드는 첫번째 argument의 타입을 의미하는 `arg1_type`를 통해서 action의 시작부분에서 타입을 확인하고 있습니다. 이를 위해서 `StaticAssertTypeEq`라는 googletest의 타입비교 기능을 사용할 수 있습니다. (compile time에 비교를 수행합니다.)

#### 새로운 Action Template 구현하기 ####

Action을 정의할 때, action으로 전달되는 parameter의 타입을 추론하기 어려운 경우가 있습니다. 이런 경우에는 template parameter을 통해 명시적으로 parameter의 타입을 지정해야 합니다. gMock의 `ACTION_TEMPLATE()`은 이러한 기능을 지원합니다. 이름에서 알 수 있듯이 `ACTION()`과 `ACTION_P*()`의 확장판이라고 생각하면 됩니다.

```cpp
ACTION_TEMPLATE(ActionName,
                HAS_m_TEMPLATE_PARAMS(kind1, name1, ..., kind_m, name_m),
                AND_n_VALUE_PARAMS(p1, ..., p_n)) { statements; }
```

이 macro를 통해 생성되는 action은 *m*개의 template parameter와 *n*개의 value parameter를 전달 받습니다. 여기서 *m*의 범위는 1~10까지이고 *n*의 범위는 0~10까지이기 때문에 약간 다르다는 점에 유의하십시오. 다음으로 `name_i`는 i번째 template parameter를 의미하며 `kind_i`는 곧 해당 template parameter의 타입을 의미합니다. `p_i`는 i번째 value parameter이며 이미 말한것처럼 `p_i`의 타입은 compiler에 의해 추론될 것이기 때문에 `kind_i`처럼 명시적으로 적는 부분은 없습니다.

그럼 예제를 보겠습니다.

```cpp
// DuplicateArg<k, T>(output) converts the k-th argument of the mock
// function to type T and copies it to *output.
ACTION_TEMPLATE(DuplicateArg,
                // Note the comma between int and k:
                HAS_2_TEMPLATE_PARAMS(int, k, typename, T),
                AND_1_VALUE_PARAMS(output)) {
  *output = T(::std::get<k>(args));
}
```

위에서 만든 action template을 사용하려면, 아래처럼 instance를 생성하면 됩니다.

```cpp
ActionName<t1, ..., t_m>(v1, ..., v_n)
```

`t`는 template argument이고 `v`는 value argument 입니다.

```cpp
using ::testing::_;
...
  int n;
  EXPECT_CALL(mock, Foo).WillOnce(DuplicateArg<1, unsigned char>(&n));
```

Value argument의 타입은 compiler가 추론한다고 말했지만, 만약 value argument의 타입도 명시적으로 지정하고 싶은 사용자가 있다면 아래와 같이 template argument에 관련내용을 직접 추가하는 것도 가능합니다.

```cpp
ActionName<t1, ..., t_m, u1, ..., u_k>(v1, ..., v_n)
```

`u_i`는 `v_i`의 타입을 의미합니다.(예상되는 타입)

`ACTION_TEMPLATE`과 `ACTION / ACTION_P*`는 value parameter의 개수에 따라서 overloading 될 것입니다.

```cpp
  OverloadedAction<int, bool>(x);
```

위 코드를 overloading 할 때, template parameter의 개수가 아닌 value parameter의 개수에 따라 overloading함을 주의하십시오. 왜냐하면 template parameter를 기준으로 하면 아래와 같이 2가지 해석이 가능해지므로 문제가 됩니다. 즉, 모호함이 발생할 수 있으므로 안됩니다.

* template parameter 1개 (`int`) + value paramter 1개 (`bool x`)
* template parameter 2개 (`int`, `bool`) + value parameter 1개 (임의의 타입 `x` , 컴파일러가 나중에 추론)

#### ACTION Object의 타입 확인하기 ####

만약, `ACTION` object를 반환하는 function을 구현하려 한다면, 해당 `ACTION` object의 타입을 알아야만 합니다. 이러한 타입은 어떤 macro를 사용했는지에 따라 달라지긴 하지만 다행히도 그 규칙은 비교적 간단하며 아래 표에서 확인할 수 있습니다.

| **Given Definition**                                         | **Expression**                             | **Has Type**                           |
| :----------------------------------------------------------- | :----------------------------------------- | :------------------------------------- |
| `ACTION(Foo)`                                                | `Foo()`                                    | `FooAction`                            |
| `ACTION_TEMPLATE(Foo, HAS_m_TEMPLATE_PARAMS(...), AND_0_VALUE_PARAMS())` | `Foo<t1, ..., t_m>()`                      | `FooAction<t1, ..., t_m>`              |
| `ACTION_P(Bar, param)`                                       | `Bar(int_value)`                           | `BarActionP<int>`                      |
| `ACTION_TEMPLATE(Bar, HAS_m_TEMPLATE_PARAMS(...), AND_1_VALUE_PARAMS(p1))` | `Bar<t1, ..., t_m>(int_value)`             | `FooActionP<t1, ..., t_m, int>`        |
| `ACTION_P2(Baz, p1, p2)`                                     | `Baz(bool_value, int_value)`               | `BazActionP2<bool, int>`               |
| `ACTION_TEMPLATE(Baz, HAS_m_TEMPLATE_PARAMS(...), AND_2_VALUE_PARAMS(p1, p2))` | `Baz<t1, ..., t_m>(bool_value, int_value)` | `FooActionP2<t1, ..., t_m, bool, int>` |
| ...                                                          | ...                                        | ...                                    |

위 표를 보면 `ACTION`, `ACTION_P`, `ACTION_P2`와 같이 macro의 suffix들이 서로 다릅니다. 이 때, macro의 이름은 action으로 전달되는 parameter 개수로 구분되며 overload를 방지하는 효과가 있습니다.

#### 새로운 Monomorphic Action 구현하기 ####

`ACTION*` macro는 편리하지만, 사용하기에 적절하지 않은 경우도 있습니다. 왜냐하면 `ACTION*` macro를 사용하면 mock function argument와 action parameter를 직접적으로 지정할 수가 없습니다. 이런 이유로 인해 compile error가 발생하기도 하며 사용자를 혼란스럽게 합니다. 또한, `ACTION*` macro만 가지고는 action을 overloading 하기가 상당히 까다롭습니다. (할 수는 있지만)

이런 상황에서는 `::testing::ActionInterface<F>`를 사용하는 것도 괜찮은 방법입니다. 여기서 `F`는 action을 사용하게 될 mock function의 function type입니다. 아래 예제를 통해 확인하세요.

```c++
template <typename F>
class ActionInterface {
 public:
  virtual ~ActionInterface();

  // Performs the action.  Result is the return type of function type
  // F, and ArgumentTuple is the tuple of arguments of F.
  //

  // For example, if F is int(bool, const string&), then Result would
  // be int, and ArgumentTuple would be ::std::tuple<bool, const string&>.
  virtual Result Perform(const ArgumentTuple& args) = 0;
};
```

```cpp
using ::testing::_;
using ::testing::Action;
using ::testing::ActionInterface;
using ::testing::MakeAction;

typedef int IncrementMethod(int*);

class IncrementArgumentAction : public ActionInterface<IncrementMethod> {
 public:
  int Perform(const ::std::tuple<int*>& args) override {
    int* p = ::std::get<0>(args);  // Grabs the first argument.
    return *p++;
  }
};

Action<IncrementMethod> IncrementArgument() {
  return MakeAction(new IncrementArgumentAction);
}

...
  EXPECT_CALL(foo, Baz(_))
      .WillOnce(IncrementArgument());

  int n = 5;
  foo.Baz(&n);  // Should return 5 and change n to 6.
```

#### 새로운 Polymorphic Action 구현하기 ####

바로 위에서 action을 직접 구현하는 방법에 대해 배웠습니다. 다만, 그건 어디까지나 mock function의 function type을 알고 있을 때만 가능한 방법입니다. 그렇다면 `Return()`, `SetArgPointee()`처럼 *여러가지 타입*에 적용할 수 있는 action을 만들기 위해서는 어떻게 하면 될까요?

Action을 여러가지 mock function에 사용하기 위해서는(즉, *polymorphic action*을 만들기 위해서는) template function인 `MakePolymorphicAction()`을 사용해야 합니다. 사용법은 어렵지 않습니다.

```cpp
namespace testing {
template <typename Impl>
PolymorphicAction<Impl> MakePolymorphicAction(const Impl& impl);
}  // namespace testing
```

이제 `MakePolymorphicAction()`이 사용할 action 만들어 보겠습니다. 예를 들어 mock function의 argument들 중에서 2번째 argument를 반환하는 action을 만들려고 합니다. 제일 먼저 해당 implementation class를 구현합니다.

```cpp
class ReturnSecondArgumentAction {
 public:
  template <typename Result, typename ArgumentTuple>
  Result Perform(const ArgumentTuple& args) const {
    // To get the i-th (0-based) argument, use ::std::get(args).
    return ::std::get<1>(args);
  }
};
```

위의 class는 monomorphic action과는 다르게 어떤 class로부터도 상속받지 않았습니다. 또한, 중요한 점은 `Perform()`이라는 template method는 꼭 구현해야 한다는 것입니다. `Perform()`은 mock function의 argument를 전부 모아서 tuple 형태의 변수 **하나**로 전달받게 됩니다. 그런 다음 action의 결과를 반환합니다. 이 function은 `const`로 만들어도 되고 꼭 아니어도 괜찮습니다. 다만 template argument들을 잘 맞춰줘야 합니다. 즉, `R`이 mock function의 return type이고 `args`가 mock function의 argument들일 때, `Perform<R>(args)`이라는 명령으로 호출될 수 있어야 합니다.

이제 `MakePolymorphicAction()`을 통해서 바로 위에서 만든 class를 polymorphic action으로 사용할 것입니다. 이름이 좀 일반적이니 적절하게 wrapper로 감싸주면 사용하기가 더 편리할 것입니다. 아래처럼 해줍니다.

```cpp
using ::testing::MakePolymorphicAction;
using ::testing::PolymorphicAction;

PolymorphicAction<ReturnSecondArgumentAction> ReturnSecondArgument() {
  return MakePolymorphicAction(ReturnSecondArgumentAction());
}
```

모두 끝났습니다. `ReturnSecondArgument()`를 built-in action과 똑같은 방법으로 사용하기만 하면 됩니다.

```cpp
using ::testing::_;

class MockFoo : public Foo {
 public:
  MOCK_METHOD(int, DoThis, (bool flag, int n), (override));
  MOCK_METHOD(string, DoThat, (int x, const char* str1, const char* str2),
              (override));
};

  ...
  MockFoo foo;
  EXPECT_CALL(foo, DoThis).WillOnce(ReturnSecondArgument());
  EXPECT_CALL(foo, DoThat).WillOnce(ReturnSecondArgument());
  ...
  foo.DoThis(true, 5);  // Will return 5.
  foo.DoThat(1, "Hi", "Bye");  // Will return "Hi".
```

#### gMock이 사용자타입 정보도 출력 가능하게 만들기 ####

gMock은 uninteresting call이나 unexpected call이 발생하면 해당 mock function으로 전달된 argument와 stack trace 정보를 출력해 줍니다. 마찬가지로 `EXPECT_THAT`, `EXPECT_EQ`과 같은 macro도 assertion 실패 시에 관련 정보를 출력해 줍니다. googletest와 gMock은 user-extensible value printer를 통해 이러한 동작을 구현하고 있습니다.

다만, 위의 printer가 출력할 수 있는 대상은 built-in C++ type, array,  STL container, 그리고 `<<` 연산자를 정의한 타입들만 해당됩니다. 다시 말하면 그 외의 사용자정의 타입들은 관련정보를 출력할 수 없으며 단순히 byte dump만 출력하도록 구현되어 있습니다. 이러한 문제를 개선하려면 [googletests`s advanced guide](../../../googletest/docs/kr/advanced.md#googletest의-디버깅정보-출력방식-변경하기)를 참고하여 더 많은 정보를 출력할 수 있도록 변경할 수 있습니다.

### Useful Mocks Created Using gMock

#### std::function Mocking하기

C++11 부터는 general function type으로 `std::function`을 사용할 수 있게 되었습니다. 또한, general function type은 callback agrument에 사용하는 것도 가능하기 때문에 많은 개발자들이 즐겨 사용하고 있습니다. 이처럼 callback argument를 전달할 때 pointer 타입으로 전달하게 되면 mocking하기가 약간 까다로운 부분이 있었는데, 이제는 그런 두려움을 가지지 않아도 됩니다. gMock의 `MockFunction`을 사용하면 이러한 어려움을 극복할 수 있습니다.

먼저 `MockFunction<R(T1, .., Tn)>`은 `Call()`이라는 mock method를 가지고 있습니다.

```c++
  R Call(T1, ..., Tn);
```

또한, `std::function` proxy롤 생성해주는 `AsStdFunction()`이라는 method도 가지고 있습니다. 

```c++
  std::function<R(T1, ..., Tn)> AsStdFunction();
```

`MockFunction`을 사용하기 위해서는 먼저 `MockFunction` object를 생성하고 `Call()`을 통해서 expectation을 지정합니다. 그 다음에 `AsStdFunction()`을 통해서 `std::function`의 proxy를 확보하고 이를 테스트 대상코드에 전달하면 됩니다. 예제코드는 아래와 같습니다.

```cpp
TEST(FooTest, RunsCallbackWithBarArgument) {
  // 1. Create a mock object.
  MockFunction<int(string)> mock_function;

  // 2. Set expectations on Call() method.
  EXPECT_CALL(mock_function, Call("bar")).WillOnce(Return(1));

  // 3. Exercise code that uses std::function.
  Foo(mock_function.AsStdFunction());
  // Foo's signature can be either of:
  // void Foo(const std::function<int(string)>& fun);
  // void Foo(std::function<int(string)> fun);

  // 4. All expectations will be verified when mock_function
  //     goes out of scope and is destroyed.
}
```

`AsStdFunction()`를 통해서 생성한 function object는 `mock_function`의 proxy일 뿐입니다. 따라서 여러번 생성한다고 해도 별도의 expectation을 새로 만드는 것이 아님을 유의하시기 바랍니다. Proxy가 아무리 여러번 생성되어도 결국엔 `EXPECT_CALL(mock_function, Call("bar")).WillOnce(Return(1));`을 사용하게 됩니다.

마지막으로 C++의 `std::function`에는 argument 개수에 제한이 없지만 `MockFunction`은 10개로 제한하고 있다는 점도 유의하시기 바랍니다.
