### Generic Build Instructions

#### Setup

To build GoogleTest and your tests that use it, you need to tell your build
system where to find its headers and source files. The exact way to do it
depends on which build system you use, and is usually straightforward.

### Build with CMake

GoogleTest comes with a CMake build script
([CMakeLists.txt](https://github.com/google/googletest/blob/master/CMakeLists.txt))
that can be used on a wide range of platforms ("C" stands for cross-platform.).
If you don't have CMake installed already, you can download it for free from
<http://www.cmake.org/>.

CMake works by generating native makefiles or build projects that can be used in
the compiler environment of your choice. You can either build GoogleTest as a
standalone project or it can be incorporated into an existing CMake build for
another project.

#### Standalone CMake Project

When building GoogleTest as a standalone project, the typical workflow starts
with

```
git clone https://github.com/google/googletest.git -b release-1.10.0
cd googletest        # Main directory of the cloned repository.
mkdir build          # Create a directory to hold the build output.
cd build
cmake ..             # Generate native build scripts for GoogleTest.
```

The above command also includes GoogleMock by default. And so, if you want to
build only GoogleTest, you should replace the last command with

```
cmake .. -DBUILD_GMOCK=OFF
```

If you are on a \*nix system, you should now see a Makefile in the current
directory. Just type `make` to build GoogleTest. And then you can simply install
GoogleTest if you are a system administrator.

```
make
sudo make install    # Install in /usr/local/ by default
```

If you use Windows and have Visual Studio installed, a `gtest.sln` file and
several `.vcproj` files will be created. You can then build them using Visual
Studio.

On Mac OS X with Xcode installed, a `.xcodeproj` file will be generated.

#### Incorporating Into An Existing CMake Project

If you want to use GoogleTest in a project which already uses CMake, the easiest
way is to get installed libraries and headers.

*   Import GoogleTest by using `find_package` (or `pkg_check_modules`). For
    example, if `find_package(GTest CONFIG REQUIRED)` is succeed, you can use
    the libraries as `GTest::gtest`, `GTest::gmock`.

And a more robust and flexible approach is to build GoogleTest as part of that
project directly. This is done by making the GoogleTest source code available to
the main build and adding it using CMake's `add_subdirectory()` command. This
has the significant advantage that the same compiler and linker settings are
used between GoogleTest and the rest of your project, so issues associated with
using incompatible libraries (eg debug/release), etc. are avoided. This is
particularly useful on Windows. Making GoogleTest's source code available to the
main build can be done a few different ways:

*   Download the GoogleTest source code manually and place it at a known
    location. This is the least flexible approach and can make it more difficult
    to use with continuous integration systems, etc.
*   Embed the GoogleTest source code as a direct copy in the main project's
    source tree. This is often the simplest approach, but is also the hardest to
    keep up to date. Some organizations may not permit this method.
*   Add GoogleTest as a git submodule or equivalent. This may not always be
    possible or appropriate. Git submodules, for example, have their own set of
    advantages and drawbacks.
*   Use CMake to download GoogleTest as part of the build's configure step. This
    is just a little more complex, but doesn't have the limitations of the other
    methods.

The last of the above methods is implemented with a small piece of CMake code in
a separate file (e.g. `CMakeLists.txt.in`) which is copied to the build area and
then invoked as a sub-build _during the CMake stage_. That directory is then
pulled into the main build with `add_subdirectory()`. For example:

New file `CMakeLists.txt.in`:

```cmake
cmake_minimum_required(VERSION 2.8.12)

project(googletest-download NONE)

include(ExternalProject)
ExternalProject_Add(googletest
  GIT_REPOSITORY    https://github.com/google/googletest.git
  GIT_TAG           master
  SOURCE_DIR        "${CMAKE_CURRENT_BINARY_DIR}/googletest-src"
  BINARY_DIR        "${CMAKE_CURRENT_BINARY_DIR}/googletest-build"
  CONFIGURE_COMMAND ""
  BUILD_COMMAND     ""
  INSTALL_COMMAND   ""
  TEST_COMMAND      ""
)
```

Existing build's `CMakeLists.txt`:

```cmake
# Download and unpack googletest at configure time
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
                 ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
                 EXCLUDE_FROM_ALL)

# The gtest/gtest_main targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
if (CMAKE_VERSION VERSION_LESS 2.8.11)
  include_directories("${gtest_SOURCE_DIR}/include")
endif()

# Now simply link against gtest or gtest_main as needed. Eg
add_executable(example example.cpp)
target_link_libraries(example gtest_main)
add_test(NAME example_test COMMAND example)
```

Note that this approach requires CMake 2.8.2 or later due to its use of the
`ExternalProject_Add()` command. The above technique is discussed in more detail
in [this separate article](http://crascit.com/2015/07/25/cmake-gtest/) which
also contains a link to a fully generalized implementation of the technique.

##### Visual Studio Dynamic vs Static Runtimes

By default, new Visual Studio projects link the C runtimes dynamically but
GoogleTest links them statically. This will generate an error that looks
something like the following: gtest.lib(gtest-all.obj) : error LNK2038: mismatch
detected for 'RuntimeLibrary': value 'MTd_StaticDebug' doesn't match value
'MDd_DynamicDebug' in main.obj

GoogleTest already has a CMake option for this: `gtest_force_shared_crt`

Enabling this option will make gtest link the runtimes dynamically too, and
match the project in which it is included.

#### C++ Standard Version

An environment that supports C++11 is required in order to successfully build
GoogleTest. One way to ensure this is to specify the standard in the top-level
project, for example by using the `set(CMAKE_CXX_STANDARD 11)` command. If this
is not feasible, for example in a C project using GoogleTest for validation,
then it can be specified by adding it to the options for cmake via the
`DCMAKE_CXX_FLAGS` option.

### Tweaking GoogleTest

GoogleTest can be used in diverse environments. The default configuration may
not work (or may not work well) out of the box in some environments. However,
you can easily tweak GoogleTest by defining control macros on the compiler
command line. Generally, these macros are named like `GTEST_XYZ` and you define
them to either 1 or 0 to enable or disable a certain feature.

We list the most frequently used macros below. For a complete list, see file
[include/gtest/internal/gtest-port.h](https://github.com/google/googletest/blob/master/googletest/include/gtest/internal/gtest-port.h).

### Multi-threaded Tests

GoogleTest is thread-safe where the pthread library is available. After
`#include "gtest/gtest.h"`, you can check the
`GTEST_IS_THREADSAFE` macro to see whether this is the case (yes if the macro is
`#defined` to 1, no if it's undefined.).

If GoogleTest doesn't correctly detect whether pthread is available in your
environment, you can force it with

    -DGTEST_HAS_PTHREAD=1

or

    -DGTEST_HAS_PTHREAD=0

When GoogleTest uses pthread, you may need to add flags to your compiler and/or
linker to select the pthread library, or you'll get link errors. If you use the
CMake script, this is taken care of for you. If you use your own build script,
you'll need to read your compiler and linker's manual to figure out what flags
to add.

### As a Shared Library (DLL)

GoogleTest is compact, so most users can build and link it as a static library
for the simplicity. You can choose to use GoogleTest as a shared library (known
as a DLL on Windows) if you prefer.

To compile *gtest* as a shared library, add

    -DGTEST_CREATE_SHARED_LIBRARY=1

to the compiler flags. You'll also need to tell the linker to produce a shared
library instead - consult your linker's manual for how to do it.

To compile your *tests* that use the gtest shared library, add

    -DGTEST_LINKED_AS_SHARED_LIBRARY=1

to the compiler flags.

Note: while the above steps aren't technically necessary today when using some
compilers (e.g. GCC), they may become necessary in the future, if we decide to
improve the speed of loading the library (see
<http://gcc.gnu.org/wiki/Visibility> for details). Therefore you are recommended
to always add the above flags when using GoogleTest as a shared library.
Otherwise a future release of GoogleTest may break your build script.

### Avoiding Macro Name Clashes

In C++, macros don't obey namespaces. Therefore two libraries that both define a
macro of the same name will clash if you `#include` both definitions. In case a
GoogleTest macro clashes with another library, you can force GoogleTest to
rename its macro to avoid the conflict.

Specifically, if both GoogleTest and some other code define macro FOO, you can
add

    -DGTEST_DONT_DEFINE_FOO=1

to the compiler flags to tell GoogleTest to change the macro's name from `FOO`
to `GTEST_FOO`. Currently `FOO` can be `FAIL`, `SUCCEED`, or `TEST`. For
example, with `-DGTEST_DONT_DEFINE_TEST=1`, you'll need to write

    GTEST_TEST(SomeTest, DoesThis) { ... }

instead of

    TEST(SomeTest, DoesThis) { ... }

in order to define a test.
