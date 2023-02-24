# Quickstart: Building with Bazel

This tutorial aims to get you up and running with GoogleTest using the Bazel
build system. If you're using GoogleTest for the first time or need a refresher,
we recommend this tutorial as a starting point.

## Prerequisites

To complete this tutorial, you'll need:

*   A compatible operating system (e.g. Linux, macOS, Windows).
*   A compatible C++ compiler that supports at least C++14.
*   [Bazel](https://bazel.build/), the preferred build system used by the
    GoogleTest team.

See [Supported Platforms](platforms.md) for more information about platforms
compatible with GoogleTest.

If you don't already have Bazel installed, see the
[Bazel installation guide](https://bazel.build/install).

{: .callout .note} Note: The terminal commands in this tutorial show a Unix
shell prompt, but the commands work on the Windows command line as well.

## Set up a Bazel workspace

A
[Bazel workspace](https://docs.bazel.build/versions/main/build-ref.html#workspace)
is a directory on your filesystem that you use to manage source files for the
software you want to build. Each workspace directory has a text file named
`WORKSPACE` which may be empty, or may contain references to external
dependencies required to build the outputs.

First, create a directory for your workspace:

```
$ mkdir my_workspace && cd my_workspace
```

Next, you’ll create the `WORKSPACE` file to specify dependencies. A common and
recommended way to depend on GoogleTest is to use a
[Bazel external dependency](https://docs.bazel.build/versions/main/external.html)
via the
[`http_archive` rule](https://docs.bazel.build/versions/main/repo/http.html#http_archive).
To do this, in the root directory of your workspace (`my_workspace/`), create a
file named `WORKSPACE` with the following contents:

```
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
  name = "com_google_googletest",
  urls = ["https://github.com/google/googletest/archive/5ab508a01f9eb089207ee87fd547d290da39d015.zip"],
  strip_prefix = "googletest-5ab508a01f9eb089207ee87fd547d290da39d015",
)
```

The above configuration declares a dependency on GoogleTest which is downloaded
as a ZIP archive from GitHub. In the above example,
`5ab508a01f9eb089207ee87fd547d290da39d015` is the Git commit hash of the
GoogleTest version to use; we recommend updating the hash often to point to the
latest version. Use a recent hash on the `main` branch.

Now you're ready to build C++ code that uses GoogleTest.

## Create and run a binary

With your Bazel workspace set up, you can now use GoogleTest code within your
own project.

As an example, create a file named `hello_test.cc` in your `my_workspace`
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

To build the code, create a file named `BUILD` in the same directory with the
following contents:

```
cc_test(
  name = "hello_test",
  size = "small",
  srcs = ["hello_test.cc"],
  deps = ["@com_google_googletest//:gtest_main"],
)
```

This `cc_test` rule declares the C++ test binary you want to build, and links to
GoogleTest (`//:gtest_main`) using the prefix you specified in the `WORKSPACE`
file (`@com_google_googletest`). For more information about Bazel `BUILD` files,
see the
[Bazel C++ Tutorial](https://docs.bazel.build/versions/main/tutorial/cpp.html).

Now you can build and run your test:

<pre>
<strong>my_workspace$ bazel test --test_output=all //:hello_test</strong>
INFO: Analyzed target //:hello_test (26 packages loaded, 362 targets configured).
INFO: Found 1 test target...
INFO: From Testing //:hello_test:
==================== Test output for //:hello_test:
Running main() from gmock_main.cc
[==========] Running 1 test from 1 test suite.
[----------] Global test environment set-up.
[----------] 1 test from HelloTest
[ RUN      ] HelloTest.BasicAssertions
[       OK ] HelloTest.BasicAssertions (0 ms)
[----------] 1 test from HelloTest (0 ms total)

[----------] Global test environment tear-down
[==========] 1 test from 1 test suite ran. (0 ms total)
[  PASSED  ] 1 test.
================================================================================
Target //:hello_test up-to-date:
  bazel-bin/hello_test
INFO: Elapsed time: 4.190s, Critical Path: 3.05s
INFO: 27 processes: 8 internal, 19 linux-sandbox.
INFO: Build completed successfully, 27 total actions
//:hello_test                                                     PASSED in 0.1s

INFO: Build completed successfully, 27 total actions
</pre>

Congratulations! You've successfully built and run a test binary using
GoogleTest.

## Next steps

*   [Check out the Primer](primer.md) to start learning how to write simple
    tests.
*   [See the code samples](samples.md) for more examples showing how to use a
    variety of GoogleTest features.
