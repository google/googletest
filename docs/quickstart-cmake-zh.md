# 快速入门：使用 CMake 构建

这个教程旨在让你使用 CMake 构建和运行 GoogleTest。如果你是第一次使用 GoogleTest，或者需要复习一下，我们建议将本教程作为起点。如果你的项目使用 Bazel，请参阅 [快速入门：使用 Bazel 构建](quickstart-bazel-zh.md)

## 前提条件

为了完成这个教程，你需要的工具有：

+ 一个兼容的操作系统（例如 Linux，macOS，Windows）。
+ 一个兼容的 C++ 编译器，至少支持 C++11。
+ [CMake](https://cmake.org/) 以及一系列用于构建的工具。
  + 构建工具包括 [Make](https://www.gnu.org/software/make/)，[Ninja](https://ninja-build.org/)，和其他，更多信息请参考[CMake Generators](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html。

更多关于 GoogleTest 兼容平台的信息，请参见 [支持的平台](platforms-zh.md) 。

如果你没有安装 CMake，参考 [CMake installation guide。](https://cmake.org/install)

{: .callout .note}
注意: 本教程中的终端命令显示的是 Unix shell，但这些命令在 Windows 命令行是哪个也可以使用。

## 建立一个项目

CMake 使用一个叫 `CMakeLists.txt` 的文件来配置项目的构建系统。使用此文件来设置项目并声明对 GoogleTest 的依赖。

首先，为你的项目创建一个目录：

```
$ mkdir my_project && cd my_project
```

接着，创建 `CMakeLists.txt` 文件并声明对 GoogleTest 的依赖。在 CMake 系统中，有很多方法来表达依赖关系；在本教程中，你将使用 [`FetchContent` CMake 模块](https://cmake.org/cmake/help/latest/module/FetchContent.html)。为此，在工程目录下（`my_project`）创建一个名为 `CMakeLists.txt` 的文件，其内容为：

```cmake
cmake_minimum_required(VERSION 3.14)
project(my_project)

# GoogleTest requires at least C++11
set(CMAKE_CXX_STANDARD 11)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
```

上述配置声明了对 GoogleTest 的依赖，其中 GoogleTest 将从 Github 上下载。上述例子中，`609281088cfefc76f9d0ce82e1ff6c30cc3591e5` 即 GoogleTest 版本的 Git commit ID。建议经常更新该值，以指向最新的版本。

关于如何创建 `CMakeLists.txt` 文件的更多信息，请参与 [CMake Tutorial](https://cmake.org/cmake/help/latest/guide/tutorial/index.html)。

## 创建并运行

有了 GoogleTest 的依赖后，你就可以在项目中使用 GoogleTest 了。

举个例子，在 `my_project` 目录下创建 `hello_test.cc` 文件，内容为：

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

为了编译代码，在 `CMakeLists.txt`  末尾添加以下内容：

```cmake
enable_testing()

add_executable(
  hello_test
  hello_test.cc
)
target_link_libraries(
  hello_test
  gtest_main
)

include(GoogleTest)
gtest_discover_tests(hello_test)
```

上述配置在 CMake 中启动了测试，声明了你想要建立的测试可执行文件（`hello_test`）并链接了 GoogleTest（`gtest_main`）。最后两行使用 [`GoogleTest` CMake 模块](https://cmake.org/cmake/help/git-stage/module/GoogleTest.html) 让 CMake 的测试运行器能找到并执行测试。

现在你可以投建并运行你的测试了：

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

恭喜！你已经成功地使用 GoogleTest 构建并运行了一个测试。

## 下一步行动

*   [查看 Primer](primer.md) 开始学习如何编写简单的测试。
*   [看看更多例子](samples.md)学习如何使用 GoogleTest 的各种功能。
