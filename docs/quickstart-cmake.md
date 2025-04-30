# Quickstart: Building with CMake

This tutorial aims to get you up and running with GoogleTest using CMake. If
you're using GoogleTest for the first time or need a refresher, we recommend
this tutorial as a starting point. If your project uses Bazel, see the
[Quickstart for Bazel](quickstart-bazel.md) instead.

## Prerequisites

To complete this tutorial, you'll need:

*   A compatible operating system (e.g. Linux, macOS, Windows).
*   A compatible C++ compiler that supports at least C++14.
*   [CMake](https://cmake.org/) >= 3.14 and a compatible build tool for building
    the project.
    *   Compatible build tools include
        [Make](https://www.gnu.org/software/make/),
        [Ninja](https://ninja-build.org/), and others - see
        [CMake Generators](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html)
        for more information.

See [Supported Platforms](platforms.md) for more information about platforms
compatible with GoogleTest.

If you don't already have CMake installed, see the
[CMake installation guide](https://cmake.org/install).

{: .callout .note}
Note: The terminal commands in this tutorial show a Unix shell prompt, but the
commands work on the Windows command line as well.

## Set up a project

CMake uses a file named `CMakeLists.txt` to configure the build system for a
project. You'll use this file to set up your project and declare a dependency on
GoogleTest.

First, create a directory for your project:

```
$ mkdir my_project && cd my_project
```

Next, you'll create the `CMakeLists.txt` file and declare a dependency on
GoogleTest. There are many ways to express dependencies in the CMake ecosystem
but the two most common methods are

1.  Using the [`find_package`](https://cmake.org/cmake/help/latest/command/find_package.html)
    command
2.  Using the [`FetchContent`](https://cmake.org/cmake/help/latest/module/FetchContent.html)
    CMake module

We will cover both methods in their [respective](#using-find_package)
[subsections](#using-FetchContent) as each has their
advantages and disadvantages.

### Using `find_package`

One very common scenario is when you would like to consume a
standalone GoogleTest installation, e.g. one
[built and installed from source](source-build-cmake.md)
locally, or one provided by a system package manager (e.g. APT, etc. on
Debian-like systems). In this case,
[`find_package`](https://cmake.org/cmake/help/latest/command/find_package.html)
is the better fit.

We can write a simple `CMakeLists.txt` as follows, using `find_package` in
[config mode](https://cmake.org/cmake/help/latest/command/find_package.html#search-modes):

```cmake
cmake_minimum_required(VERSION 3.14)
project(my_project)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Since CMake 3.20 CONFIG can be omitted as the FindGTest find module will
# prefer the upstream (provided by Google Test) GTestConfig.cmake if available
find_package(GTest 1.15.0 REQUIRED CONFIG)
```

A custom installation root can be specified using `GTEST_ROOT` as an environment
or CMake cache variable as mentioned in the
[`FindGTest` docs](https://cmake.org/cmake/help/latest/module/FindGTest.html#cache-variables).

### Using `FetchContent`

Another common scenario is when one wants to absolutely ensure all dependencies
use the same compile and link flags by building all of them from source with
the same settings used by the project. There may be additional requirements such
as allowing tracking of upstream changes as they flow into these dependencies'
source trees.

In this case
[`FetchContent`](https://cmake.org/cmake/help/latest/module/FetchContent.html)
is the tool of choice, allowing one to download a specific source checkout into
their project build tree and then build it as a vendored component within the
project.

So to do this, in your project directory (`my_project`), create a
`CMakeLists.txt` with the following contents:

```cmake
cmake_minimum_required(VERSION 3.14)
project(my_project)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# Windows: Ensure C runtime linkage uses CMake defaults (shared C runtime).
# This can be omitted if you would like to use Google Test's preference of
# linking against static C runtime for static Google Test builds, shared C
# runtime for shared Google Test library builds.
if(MSVC)
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()
FetchContent_MakeAvailable(googletest)
```

The above configuration declares a dependency on GoogleTest which is downloaded
from GitHub. In the above example, `03597a01ee50ed33e9dfd640b249b4be3799d395` is
the Git commit hash of the GoogleTest version to use; we recommend updating the
hash often to point to the latest version.

For more information about how to create `CMakeLists.txt` files, see the
[CMake Tutorial](https://cmake.org/cmake/help/latest/guide/tutorial/index.html).

## Create and run a binary

With GoogleTest declared as a dependency, you can use GoogleTest code within
your own project.

As an example, create a file named `hello_test.cc` in your `my_project`
directory with the following contents:

```cpp
#include <gtest/gtest.h>

// Demonstrate some basic assertions.
TEST(HelloTest, BasicAssertions) {
  // Expect two strings not to be equal.
  EXPECT_STRNE("hello", "world");
  // Expect equality.
  EXPECT_EQ(7 * 6, 42);
}
```

GoogleTest provides [assertions](primer.md#assertions) that you use to test the
behavior of your code. The above sample includes the main GoogleTest header file
and demonstrates some basic assertions.

To build the code, add the following to the end of your `CMakeLists.txt` file:

```cmake
enable_testing()

add_executable(hello_test hello_test.cc)
target_link_libraries(hello_test PRIVATE GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(hello_test)
```

The above configuration enables testing in CMake, declares the C++ test binary
you want to build (`hello_test`), and links it to GoogleTest (`gtest_main`). The
last two lines enable CMake's test runner to discover the tests included in the
binary, using the
[`GoogleTest` CMake module](https://cmake.org/cmake/help/git-stage/module/GoogleTest.html).

Now you can build and run your test:

<pre>
<strong>my_project$ cmake -S . -B build</strong>
-- The C compiler identification is GNU 10.2.1
-- The CXX compiler identification is GNU 10.2.1
...
-- Build files have been written to: .../my_project/build

<strong>my_project$ cmake --build build</strong>
Scanning dependencies of target gtest
...
[100%] Built target gmock_main

<strong>my_project$ cd build && ctest</strong>
Test project .../my_project/build
    Start 1: HelloTest.BasicAssertions
1/1 Test #1: HelloTest.BasicAssertions ........   Passed    0.00 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =   0.01 sec
</pre>

Congratulations! You've successfully built and run a test binary using
GoogleTest.

## Next steps

*   [Check out the Primer](primer.md) to start learning how to write simple
    tests.
*   [See the code samples](samples.md) for more examples showing how to use a
    variety of GoogleTest features.
