# GMock

<!-- GOOGLETEST_CM0035 DO NOT DELETE -->

## What is gMock?

gMock is Google's framework for creating and using C++ mock classes. It helps
you design better systems and write better tests. A mock object is an object
that you use in a test instead of a real object. A mock object implements the
same interface as a real object but lets you specify at run time how the object
will be used. When you write tests that use a mock, you define expectations
about how the mock's methods will be called. Your test then verifies how your
real code behaves when interacting with the mock. See the
[Mock Objects Best Practices Guide](http://go/mock-objects#mocks-stubs-fakes)
for a comparison of mocks with stubs, fakes, and other kinds of test doubles.

For example, gMock provides a simple syntax for declaring "I expect the
RetryQuery method on this mock object to be called three times in the course of
this test". Your test will fail if the expectation isn't met.

The gMock library provides a mock framework for C++ similar to jMock or
EasyMock[?](http://go/easymock-codelab) for Java. In gMock you use macros to
define methods for your mock objects and set expectations for those methods.
gMock runs on Linux, Windows, and Mac OS X.

## What is gMock good for?

Mocks in general are good for:

-   prototyping and designing new code and APIs.
-   removing unnecessary, expensive, or unreliable dependencies from your tests.

gMock in particular is good for writing quality C++ mocks. Without the help of a
mocking framework like gMock, good C++ mocks are hard to create.

## What is gMock NOT good for?

gMock is not good for testing the behavior of dependencies. The point of testing
with mocks is to test the classes that use the mocks, not the mocks themselves.
Objects that have working toy implementations are called fakes instead of mocks.
For example, you could use an in-memory file system to fake disk operations.

Mocks aren't useful for very simple classes like
[Dumb Data Objects](http://big.corp.google.com/~jmcmaster/testing/2011/04/episode-220-blast-from-tott-past-dont.html).
If it's more trouble to use a mock than the real class, just use the real class.

## Who uses gMock?

There are over 30K tests using gmock. Virtually every C++ test at Google that
needs a mock object uses gMock.

## Practical matters

gMock is bundled with [gUnit](/third_party/googletest/googletest/docs/). To use
gMock,
[include a dependency](/third_party/googletest/googletest/docs/howto_cpp#LinuxTarget)
on `//testing/base/public:gunit` in the BUILD rule for your mocks, and use the
following include statement in the file that defines your mock class:

```
#include "gmock/gmock.h"
```

&nbsp;                      | &nbsp;
--------------------------- | ------------------------------------------
**Implementation language** | C++
**Code location**           | google3/third_party/googletest/googlemock/
**Build target**            | //testing/base/public:gunit

## Best practices

Use [dependency injection](http://en.wikipedia.org/wiki/Dependency_injection) to
enable easy mocking. If you define dependencies as interfaces rather than
concrete classes, you can swap out the production version of a class for a mock
during testing.

You can also use gMock during the design phase for your system. By sketching
your architecture using mocks rather than full implementations, you can evolve
your design more quickly.

## History and evolution

In January 2007 Zhanyong Wan and the Testing Technology team met with
experienced C++ engineers to find out about C++ testing needs. The team learned
that creating mocks in C++ was a major pain point. They looked around for
existing frameworks but didn't find anything satisfactory. So Zhanyong Wan
tackled the problem of creating a usable C++ mocking framework.

C++ posed a unique problem for mocking: while
[reflection](http://en.wikipedia.org/wiki/Reflection_\(computer_programming\))
in Java and Python make it easy to generate a mock implementation of any
interface, C++ does not have reflection. Wan hit on macros as a way to simplify
mock writing in C++, and gMock was born.

## Who to contact

-   g/gmock-users
-   g/gmock-announce

## Additional resources

-   [gMock](http://go/gmock) - homepage
-   [GMock for Dummies](http://<!-- GOOGLETEST_CM0013 DO NOT DELETE -->) - gets you started with gMock
    quickly
-   [GMock Cookbook](http://<!-- GOOGLETEST_CM0012 DO NOT DELETE -->) - recipes for common scenarios; covers
    advanced usage.
-   [GMock Cheat Sheet](http://<!-- GOOGLETEST_CM0020 DO NOT DELETE -->) - a quick reference
-   [GMock FAQ](http://<!-- GOOGLETEST_CM0021 DO NOT DELETE -->) - frequently asked questions
-   [gUnit GDH page](http://go/gunit-overview)
-   [gUnit User's Guide](http://goto.corp.google.com/gunit) - gets you started
    with gUnit, which is closely related to gMock
