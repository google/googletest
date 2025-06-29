# GoogleTest

**GoogleTest** is Google’s C++ test framework, designed for writing reliable and maintainable tests.

> 📄 **Documentation**: [https://google.github.io/googletest/](https://google.github.io/googletest/)  
> 🚀 **Latest Release**: [v1.17.0](https://github.com/google/googletest/releases/tag/v1.17.0)  
> ⚠️ **Minimum Requirement**: C++17  
> ✅ **CI**: Built using Google’s internal systems  
> 🔮 **Coming Soon**: Integration with [Abseil](https://github.com/abseil/abseil-cpp)

---

## 📚 Table of Contents

- [Getting Started](#getting-started)
- [Features](#features)
- [Supported Platforms](#supported-platforms)
- [Who’s Using GoogleTest?](#whos-using-googletest)
- [Related Tools & Projects](#related-tools--projects)
- [Contributing](#contributing)

---

## Getting Started

Visit the [GoogleTest User’s Guide](https://google.github.io/googletest/) for comprehensive documentation.

We recommend starting with the [GoogleTest Primer](https://google.github.io/googletest/primer.html) to get a feel for writing your first tests.

Instructions for building GoogleTest can be found in the [googletest/README.md](googletest/README.md) file.

---

## Features

GoogleTest offers a rich set of capabilities for testing C++ code:

- **xUnit Framework**  
  Follows the well-known [xUnit architecture](https://en.wikipedia.org/wiki/XUnit) for structuring tests.

- **Automatic Test Discovery**  
  Tests are automatically detected and executed—no manual registration required.

- **Comprehensive Assertions**  
  Includes a wide variety of assertions for testing equality, inequality, exceptions, boolean conditions, floating-point values, and more.

- **Custom Assertions**  
  Easily define your own assertions for application-specific checks.

- **Death Tests**  
  Validate that certain code paths cause the program to exit (useful for error handling).

- **Fatal vs. Non-Fatal Failures**  
  Choose whether a failure should abort the test or allow further execution.

- **Value-Parameterized Tests**  
  Run a test case multiple times with different input values.

- **Type-Parameterized Tests**  
  Run the same test logic over multiple data types.

- **Flexible Execution**  
  Run individual tests, filter tests by name, or run tests in parallel.

---

## Supported Platforms

GoogleTest adheres to Google’s [Foundational C++ Support Policy](https://opensource.google/documentation/policies/cplusplus-support).

See the [support matrix](https://github.com/google/oss-policies-info/blob/main/foundational-cxx-support-matrix.md) for details on supported compilers, platforms, and tools.

---

## Who’s Using GoogleTest?

GoogleTest is widely adopted both inside and outside of Google. Notable users include:

- [Chromium Project](https://www.chromium.org/)
- [LLVM](https://llvm.org/)
- [Protocol Buffers](https://github.com/protocolbuffers/protobuf)
- [OpenCV](https://opencv.org/)

---

## Related Tools & Projects

- 🎛 [GTest Runner](https://github.com/nholthaus/gtest-runner)  
  Qt-based GUI for running tests on Windows and Linux.

- 📊 [GoogleTest UI](https://github.com/ospector/gtest-gbar)  
  C# GUI that runs tests, displays results, and shows failure details interactively.

- 🧪 [GTest TAP Listener](https://github.com/kinow/gtest-tap-listener)  
  Outputs test results in [TAP format](https://en.wikipedia.org/wiki/Test_Anything_Protocol) for integration with TAP-compatible tools.

- ⚡ [gtest-parallel](https://github.com/google/gtest-parallel)  
  A parallel test runner for speeding up test execution.

- 🧩 [GoogleTest Adapter (VS Code)](https://marketplace.visualstudio.com/items?itemName=DavidSchuldenfrei.gtest-adapter)  
  View and run tests in a tree view in Visual Studio Code.

- 🧪 [C++ TestMate (VS Code)](https://github.com/matepek/vscode-catch2-test-adapter)  
  Advanced test runner extension with support for GoogleTest.

- 🥒 [Cornichon](https://pypi.org/project/cornichon/)  
  A Gherkin DSL parser that generates stub code for GoogleTest.

---

## Contributing

We welcome contributions!

Please read the [CONTRIBUTING.md](https://github.com/google/googletest/blob/main/CONTRIBUTING.md) guide for details on how to participate in development, report bugs, or submit pull requests.

---

**Happy testing!**
