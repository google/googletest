# GoogleTest

#### OSS Builds Status

[![Build status](https://ci.appveyor.com/api/projects/status/4o38plt0xbo1ubc8/branch/master?svg=true)](https://ci.appveyor.com/project/GoogleTestAppVeyor/googletest/branch/master)

### Announcements

#### Live at Head

GoogleTest now follows the
[Abseil Live at Head philosophy](https://abseil.io/about/philosophy#upgrade-support).
We recommend using the latest commit in the `master` branch in your projects.

#### Documentation Updates

Our documentation is now live on GitHub Pages at
https://google.github.io/googletest/. We recommend browsing the documentation on
GitHub Pages rather than directly in the repository.

#### Release 1.10.x

[Release 1.10.x](https://github.com/google/googletest/releases/tag/release-1.10.0)
is now available.

#### Coming Soon

*   We are planning to take a dependency on
    [Abseil](https://github.com/abseil/abseil-cpp).
*   More documentation improvements are planned.

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

*   An [xUnit](https://en.wikipedia.org/wiki/XUnit) test framework.
*   Test discovery.
*   A rich set of assertions.
*   User-defined assertions.
*   Death tests.
*   Fatal and non-fatal failures.
*   Value-parameterized tests.
*   Type-parameterized tests.
*   Various options for running the tests.
*   XML test report generation.

## Supported Platforms

GoogleTest requires a codebase and compiler compliant with the C++11 standard or
newer.

The GoogleTest code is officially supported on the following platforms.
Operating systems or tools not listed below are community-supported. For
community-supported platforms, patches that do not complicate the code may be
considered.

If you notice any problems on your platform, please file an issue on the
[GoogleTest GitHub Issue Tracker](https://github.com/google/googletest/issues).
Pull requests containing fixes are welcome!

### Operating Systems

*   Linux
*   macOS
*   Windows

### Compilers

*   gcc 5.0+
*   clang 5.0+
*   MSVC 2015+

**macOS users:** Xcode 9.3+ provides clang 5.0+.

### Build Systems

*   [Bazel](https://bazel.build/)
*   [CMake](https://cmake.org/)

**Note:** Bazel is the build system used by the team internally and in tests.
CMake is supported on a best-effort basis and by the community.

## Who Is Using GoogleTest?

In addition to many internal projects at Google, GoogleTest is also used by the
following notable projects:

*   The [Chromium projects](http://www.chromium.org/) (behind the Chrome browser
    and Chrome OS).
*   The [LLVM](http://llvm.org/) compiler.
*   [Protocol Buffers](https://github.com/google/protobuf), Google's data
    interchange format.
*   The [OpenCV](http://opencv.org/) computer vision library.

## Related Open Source Projects

[GTest Runner](https://github.com/nholthaus/gtest-runner) is a Qt5 based
automated test-runner and Graphical User Interface with powerful features for
Windows and Linux platforms.

[GoogleTest UI](https://github.com/ospector/gtest-gbar) is a test runner that
runs your test binary, allows you to track its progress via a progress bar, and
displays a list of test failures. Clicking on one shows failure text. Google
Test UI is written in C#.

[GTest TAP Listener](https://github.com/kinow/gtest-tap-listener) is an event
listener for GoogleTest that implements the
[TAP protocol](https://en.wikipedia.org/wiki/Test_Anything_Protocol) for test
result output. If your test runner understands TAP, you may find it useful.

[gtest-parallel](https://github.com/google/gtest-parallel) is a test runner that
runs tests from your binary in parallel to provide significant speed-up.

[GoogleTest Adapter](https://marketplace.visualstudio.com/items?itemName=DavidSchuldenfrei.gtest-adapter)
is a VS Code extension allowing to view GoogleTest in a tree view, and run/debug
your tests.

[C++ TestMate](https://github.com/matepek/vscode-catch2-test-adapter) is a VS
Code extension allowing to view GoogleTest in a tree view, and run/debug your
tests.

[Cornichon](https://pypi.org/project/cornichon/) is a small Gherkin DSL parser
that generates stub code for GoogleTest.

## Contributing Changes

Please read
[`CONTRIBUTING.md`](https://github.com/google/googletest/blob/master/CONTRIBUTING.md)
for details on how to contribute to this project.

Happy testing!
