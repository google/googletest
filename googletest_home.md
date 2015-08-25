

Google's framework for writing C++ tests on a variety of platforms
(Linux, Mac OS X, Windows, Cygwin, Windows CE, and Symbian).  Based on
the xUnit architecture.  Supports automatic test discovery, a rich set
of assertions, user-defined assertions, death tests, fatal and
non-fatal failures, value- and type-parameterized tests, various
options for running the tests, and XML test report generation.

## Getting Started ##

After downloading Google Test, unpack it, read the README file and the documentation wiki pages (listed on the right side of this front page).

## Who Is Using Google Test? ##

In addition to many internal projects at Google, Google Test is also used by
the following notable projects:

  * The [Chromium projects](http://www.chromium.org/) (behind the Chrome browser and Chrome OS)
  * The [LLVM](http://llvm.org/) compiler
  * [Protocol Buffers](http://code.google.com/p/protobuf/) (Google's data interchange format)
  * The [OpenCV](http://opencv.org/) computer vision library

If you know of a project that's using Google Test and want it to be listed here, please let
`googletestframework@googlegroups.com` know.

## Google Test-related open source projects ##

[Google Test UI](http://code.google.com/p/gtest-gbar/) is test runner that runs your test binary, allows you to track its progress via a progress bar, and displays a list of test failures. Clicking on one shows failure text. Google Test UI is written in C#.

[GTest TAP Listener](https://github.com/kinow/gtest-tap-listener) is an event listener for Google Test that implements the [TAP protocol](http://en.wikipedia.org/wiki/Test_Anything_Protocol) for test result output. If your test runner understands TAP, you may find it useful.