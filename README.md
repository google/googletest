# GoogleTest

### Announcements

#### Live at Head

GoogleTest now follows the
[Abseil Live at Head philosophy](https://abseil.io/about/philosophy#upgrade-support).
We recommend
[updating to the latest commit in the `main` branch as often as possible](https://github.com/abseil/abseil-cpp/blob/master/FAQ.md#what-is-live-at-head-and-how-do-i-do-it).
We do publish occasional semantic versions, tagged with
`v${major}.${minor}.${patch}` (e.g. `v1.16.0`).

#### Documentation Updates

Our documentation is now live on GitHub Pages at
https://google.github.io/googletest/. We recommend browsing the documentation on
GitHub Pages rather than directly in the repository.

#### Release 1.16.0

[Release 1.16.0](https://github.com/google/googletest/releases/tag/v1.16.0) is
now available.

The 1.16.x branch requires at least C++14.

#### Continuous Integration

We use Google's internal systems for continuous integration.

#### Coming Soon

*   We are planning to take a dependency on
    [Abseil](https://github.com/abseil/abseil-cpp).

## Welcome to **GoogleTest**, Google's C++ test framework!

This repository is a merger of the formerly separate GoogleTest and GoogleMock
projects. These were so closely related that it makes sense to maintain and
release them together.

### Getting Started

See the [GoogleTest User's Guide](https://google.github.io/googletest/) for
documentation. We recommend starting with the
[GoogleTest Primer](https://google.github.io/googletest/primer.html).

More information about building GoogleTest can be found at
[googletest/README.md](googletest/README.md).

## Features

*   xUnit test framework: \
    Googletest is based on the [xUnit](https://en.wikipedia.org/wiki/XUnit)
    testing framework, a popular architecture for unit testing
*   Test discovery: \
    Googletest automatically discovers and runs your tests, eliminating the need
    to manually register your tests
*   Rich set of assertions: \
    Googletest provides a variety of assertions, such as equality, inequality,
    exceptions, and more, making it easy to test your code
*   User-defined assertions: \
    You can define your own assertions with Googletest, making it simple to
    write tests that are specific to your code
*   Death tests: \
    Googletest supports death tests, which verify that your code exits in a
    certain way, making it useful for testing error-handling code
*   Fatal and non-fatal failures: \
    You can specify whether a test failure should be treated as fatal or
    non-fatal with Googletest, allowing tests to continue running even if a
    failure occurs
*   Value-parameterized tests: \
    Googletest supports value-parameterized tests, which run multiple times with
    different input values, making it useful for testing functions that take
    different inputs
*   Type-parameterized tests: \
    Googletest also supports type-parameterized tests, which run with different
    data types, making it useful for testing functions that work with different
    data types
*   Various options for running tests: \
    Googletest provides many options for running tests including running
    individual tests, running tests in a specific order and running tests in
    parallel

## Supported Platforms

GoogleTest follows Google's
[Foundational C++ Support Policy](https://opensource.google/documentation/policies/cplusplus-support).
See
[this table](https://github.com/google/oss-policies-info/blob/main/foundational-cxx-support-matrix.md)
for a list of currently supported versions of compilers, platforms, and build
tools.

## Who Is Using GoogleTest?

In addition to many internal projects at Google, GoogleTest is also used by the
following notable projects:

*   The [Chromium projects](https://www.chromium.org/) (behind the Chrome
    browser and Chrome OS).
*   The [LLVM](https://llvm.org/) compiler.
*   [Protocol Buffers](https://github.com/google/protobuf), Google's data
    interchange format.
*   The [OpenCV](https://opencv.org/) computer vision library.

## Related Open Source Projects

[GTest Runner](https://github.com/nholthaus/gtest-runner) is a Qt5 based
automated test-runner and Graphical User Interface with powerful features for
Windows and Linux platforms.

[GoogleTest UI](https://github.com/ospector/gtest-gbar) is a test runner that
runs your test binary, allows you to track its progress via a progress bar, and
displays a list of test failures. Clicking on one shows failure text. GoogleTest
UI is written in C#.

[GTest TAP Listener](https://github.com/kinow/gtest-tap-listener) is an event
listener for GoogleTest that implements the
[TAP protocol](https://en.wikipedia.org/wiki/Test_Anything_Protocol) for test
result output. If your test runner understands TAP, you may find it useful.

[gtest-parallel](https://github.com/google/gtest-parallel) is a test runner that
runs tests from your binary in parallel to provide significant speed-up.

[GoogleTest Adapter](https://marketplace.visualstudio.com/items?itemName=DavidSchuldenfrei.gtest-adapter)
is a VS Code extension allowing to view GoogleTest in a tree view and run/debug
your tests.

[C++ TestMate](https://github.com/matepek/vscode-catch2-test-adapter) is a VS
Code extension allowing to view GoogleTest in a tree view and run/debug your
tests.

[Cornichon](https://pypi.org/project/cornichon/) is a small Gherkin DSL parser
that generates stub code for GoogleTest.

## Contributing Changes

Please read
[`CONTRIBUTING.md`](https://github.com/google/googletest/blob/main/CONTRIBUTING.md)
for details on how to contribute to this project.

Happy testing!
