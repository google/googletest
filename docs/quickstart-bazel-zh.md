# 快速入门：使用 Bazel 构建

这个教程旨在让你使用 Bazel 构建和运行 GoogleTest。如果你是第一次使用 GoogleTest，或者需要复习一下，我们建议将本教程作为起点。

## 前提条件

为了完成这个教程，你需要的工具有：

+ 一个兼容的操作系统（例如 Linux，macOS，Windows）。
+ 一个兼容的 C++ 编译器，至少支持 C++11。
+ [Bazel](https://bazel.build/)，GoogleTest 团队使用的首选构建系统。

更多关于 GoogleTest 兼容平台的信息，请参见 [支持的平台](platforms-zh.md)。

如果你没有安装 Bazel，参考 [Bazel installation guide](https://docs.bazel.build/versions/master/install.html)。

{: .callout .note}
注意: 本教程中的终端命令显示的是 Unix shell，但这些命令在 Windows 命令行是哪个也可以使用。

## 建立一个 Bazel 工作区

[Bazel 工作区](https://docs.bazel.build/versions/master/build-ref.html#workspace) 是你文件系统上的一个目录，用于管理你想要构建的系统的源文件。每个工作区有个文本文件叫做 `WORKSPACE`，它可能是空的，也可能包含构建需要的外部依赖的引用。

首先，为你的工作区创建一个目录：

```
$ mkdir my_workspace && cd my_workspace
```

接着，创建  `WORKSPACE`  文件来指定依赖关系。为了依赖 GoogleTest，一种常见且推荐方式是通过  [`http_archive` 规则](https://docs.bazel.build/versions/master/repo/http.html#http_archive)  使用  [Bazel 外部依赖](https://docs.bazel.build/versions/master/external.html)。为此，在你的工作空间根目录下（`my_workspace/`）创建一个名为`WORKSPACE`的文件，内容为：

```
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
  name = "com_google_googletest",
  urls = ["https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip"],
  strip_prefix = "googletest-609281088cfefc76f9d0ce82e1ff6c30cc3591e5",
)
```

上述配置声明了对 GoogleTest 的依赖，GoogleTest 将以 ZIP 文件的形式从 Gihub 中下载。上述例子中，`609281088cfefc76f9d0ce82e1ff6c30cc3591e5`即 GoogleTest 版本的 Git commit ID。建议经常更新该值，以指向最新的版本。

现在你已经准备好构建使用 GoogleTest 的 C++ 代码了。

## 创建并运行

随着 Bazel 工作区的建立，现在可以在你的项目中使用 GoogleTest 代码了。

举个例子，在 `my_workspace` 目录下创建 `hello_test.cc` 文件，内容为：

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

GoogleTest 提供了许多 [断言](primer.md#assertions)，可以用它们来测试代码的行为。上述示例中引入了 GoogleTest 的头文件并演示了一些基本的断言。

为了编译代码，请在相同目录下创建一个叫 `BUILD` 的文件，内容为：

```
cc_test(
  name = "hello_test",
  size = "small",
  srcs = ["hello_test.cc"],
  deps = ["@com_google_googletest//:gtest_main"],
)
```

`cc_test` 规则声明了你要建立的测试可执行文件，并通过在 `WORKSPACE` 文件中指定的前缀（`@com_google_googletest`）去链接 GoogleTest。更多关于 Bazel `BUILD`文件的内容，请参考 [Bazel C++ Tutorial](https://docs.bazel.build/versions/master/tutorial/cpp.html)。

现在你可以投建并运行你的测试了：

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
恭喜！你已经成功地使用 GoogleTest 构建并运行了一个测试。

## 下一步行动

*   [查看 Primer](primer.md) 开始学习如何编写简单的测试。
*   [看看更多例子](samples.md)学习如何使用 GoogleTest 的各种功能。
