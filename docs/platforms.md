# Supported Platforms

GoogleTest requires a codebase and compiler compliant with the C++11 standard or
newer.

The GoogleTest code is officially supported on the following platforms.
Operating systems or tools not listed below are community-supported. For
community-supported platforms, patches that do not complicate the code may be
considered.

If you notice any problems on your platform, please file an issue on the
[GoogleTest GitHub Issue Tracker](https://github.com/google/googletest/issues).
Pull requests containing fixes are welcome!

### Operating systems

*   Linux
*   macOS
*   Windows

### Compilers

*   gcc 5.0+
*   clang 5.0+
*   MSVC 2015+

**macOS users:** Xcode 9.3+ provides clang 5.0+.

### Build systems

*   [Bazel](https://bazel.build/)
*   [CMake](https://cmake.org/)

Bazel is the build system used by the team internally and in tests. CMake is
supported on a best-effort basis and by the community.
