# Building from Source: CMake

Sometimes, it may be desirable to produce one's own local build and install
of GoogleTest. For example, one may want to check out a particular release tag,
e.g. `v1.15.2`, and then build and install GoogleTest to one's specification.

For example, on Windows platforms, where the complication of
[different C runtimes](https://learn.microsoft.com/en-us/cpp/c-runtime-library/crt-library-features?view=msvc-170)
exists and where there can be issues if ABI-incompatible C runtimes
[are mixed](https://learn.microsoft.com/en-us/cpp/c-runtime-library/crt-library-features?view=msvc-170#what-problems-exist-if-an-application-uses-more-than-one-crt-version),
developers will often have a debug version of a dependency built, linked against
the debug shared C runtime, with the release version of said dependency built
and linked against the release shared C runtime. By default, GoogleTest uses
static C runtime for static (the default) builds and the shared C runtime for
shared builds, e.g. with `BUILD_SHARED_LIBS` set to `ON`, which is sometimes
not the desired behavior on Windows.

## Prerequisites

To build and install GoogleTest from source, one needs:

*   A compatible operating system (e.g. Linux, macOS, Windows).
*   A compatible C++ compiler that supports at least C++14.
*   [CMake](https://cmake.org/) >= 3.14 and a compatible build tool.
    *   Compatible build tools include
        [Make](https://www.gnu.org/software/make/),
        [Ninja](https://ninja-build.org/), and others - see
        [CMake Generators](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html)
        for more information.
*   Optionally [Git](https://git-scm.com/) if one would like to build from the
    GoogleTest source tree.

See [Supported Platforms](platforms.md) for more information about platforms
compatible with GoogleTest.

If you don't already have CMake installed, see the
[CMake installation guide](https://cmake.org/install).

## Building for \*nix

To build on \*nix systems, after cloning the GoogleTest source from GitHub and
then checking out a particular commit or release tag, e.g. `v1.15.2`, usually
the following is sufficient for
[single-config generators](https://cmake.org/cmake/help/latest/manual/cmake-buildsystem.7.html#build-configurations):

```shell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

This will build the Release config static libraries in parallel using all
available processors, generally using Make as the underlying build tool. To use
a different single-config generator like Ninja, the configure step can be
changed to

```shell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
```

One can install into their desired installation root with something like this:

```shell
cmake --install build --prefix ../googletest-1.15.2
```

For multi-config generators like
[Ninja Multi-Config](https://cmake.org/cmake/help/latest/generator/Ninja%20Multi-Config.html),
the [`--config`](https://cmake.org/cmake/help/latest/manual/cmake.1.html#cmdoption-cmake-build-config) flag needs to specified to `cmake --build`
to select the build config that will actually be built. Furthermore, if one
wants to install the debug libraries side-by-side with the release libraries,
a debug suffix should be specified for the libraries, otherwise the files will
get clobbered.

{: .callout .note}
Note: On \*nix systems the utility of installing separate debug libraries is
less because unlike Windows, which has multiple C runtimes that are often
ABI-incompatible, there is typically only one `libc` and `libstdc++` or
`libc++` runtime being linked against, so ABI incompatibility is a non-issue.
Therefore, unless necessary, prefer using a single-config generator to build
and install GoogleTest from source.

To use a multi-config generator, in particular Ninja Multi-Config, generally
the following would be desirable:

```shell
cmake -S . -B build -G "Ninja Multi-Config" -DCMAKE_DEBUG_POSTFIX=d
cmake --build build --config Debug -j
cmake --build build --config Release -j
```

The `-DCMAKE_DEBUG_POSTFIX=d` ensures that debug libraries are built with a
`d` suffix in their names, e.g. `libgtestd.a`, to prevent clobbering on
installation. Then, one can install both configs into the installation root
with something like:

```shell
cmake --install build --prefix ../googletest-1.15.2 --config Debug
cmake --install build --prefix ../googletest-1.15.2 --config Release
```

{: .callout .note}
Note: The pkg-config files that will be installed will contain linker lines that
are non suffixed, e.g. `-lgtest`, so if you are using pkg-config, your builds
will link against `libgtest.a`. But if you are building GoogleTest using CMake,
generally you should be using GoogleTest via CMake, so this is not a big issue.

## Building for Windows

On Windows, the default generator selected by CMake will be the most recent
Visual Studio generator, e.g.
[Visual Studio 17 2022](https://cmake.org/cmake/help/latest/generator/Visual%20Studio%2017%202022.html),
which is a multi-config generator. Although one can use Ninja instead, as it is
bundled with recent Visual Studio installations, due to peculiarities of the
multiple Visual Studio developer command prompts available and the inability of
Ninja to select a particular target architecture with the CMake
[`-A`](https://cmake.org/cmake/help/latest/manual/cmake.1.html#cmdoption-cmake-A)
flag like the Visual Studio generators, we consider only the standard
Visual Studio multi-config generators.

To build 64-bit binaries, after `cd`-ing into the GoogleTest repo or
source distribution, generally the following is desirable:

{: .callout .note}
Note: To build 32-bit binaries, replace `-A x64` with `-A Win32` instead.

```
cmake -S . -B build -A x64 -DCMAKE_DEBUG_POSTFIX=d -Dgtest_force_shared_crt=ON
cmake --build build --config Debug -j
cmake --build build --config Release -j
```

Like the multi-config build instructions in the
[previous section](#building-for-nix) we ensure debug libraries have a `d`
suffix with `-DCMAKE_DEBUG_POSTFIX=d` while we ensure that the static
GoogleTest libraries are linked against the shared C runtime libraries with
`-Dgtest_force_shared_crt=ON`. GoogleTest by default will have static builds
link against the static C runtime, shared builds link against the shared C
runtime, which in some cases is undesirable and goes against the CMake defaults
for the
[`MSVC_RUNTIME_LIBRARY`](https://cmake.org/cmake/help/latest/prop_tgt/MSVC_RUNTIME_LIBRARY.html)
target property.

As previously shown, we can then install both configs with something like:

```
cmake --install build --prefix ..\googletest-1.15.2 --config Debug
cmake --install build --prefix ..\googletest-1.15.2 --config Release
```

## Usage from CMake

Now that you have a separate installation of Google Test built from source, you
may consume it from a `CMakeLists.txt` with
[`find_package`](https://cmake.org/cmake/help/latest/command/find_package.html)
in the usual manner:

```cmake
find_package(GTest 1.15.0 REQUIRED CONFIG)
```

If necessary, `GTEST_ROOT` can be set in the environment or as a CMake cache
variable to help CMake locate your installation as documented in the
[`FindGTest`](https://cmake.org/cmake/help/latest/module/FindGTest.html)
documentation.