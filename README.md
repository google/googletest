
Welcome to **Google Test**, the Google C++ Test Framework!

This GitHub repository is a merger of the former <googletest> and <googlemock> products,
which are so closely related that it makes sense to maintain and release them together.

## GoogleTest ##

Based on the [XUnit](https://en.wikipedia.org/wiki/XUnit) architecture.
Supports automatic test discovery, a rich set of assertions, user-defined
assertions, death tests, fatal and non-fatal failures, value- and
type-parameterized tests, various options for running the tests, and XML test
report generation.

## Platforms ##

Google test has been used on a variety of platforms:

  * Linux
  * Mac OS X
  * Windows
  * Cygwin
  * MinGW
  * Windows Mobile
  * Symbian

## Who Is Using Google Test? ##

In addition to many internal projects at Google, Google Test is also used by
the following notable projects:

  * The [Chromium projects](http://www.chromium.org/) (behind the Chrome browser and Chrome OS)
  * The [LLVM](http://llvm.org/) compiler
  * [Protocol Buffers](http://code.google.com/p/protobuf/) (Google's data interchange format)
  * The [OpenCV](http://opencv.org/) computer vision library

## Google Test-related open source projects ##

[Google Test UI](http://code.google.com/p/gtest-gbar/) is test runner that runs
your test binary, allows you to track its progress via a progress bar, and
displays a list of test failures. Clicking on one shows failure text. Google
Test UI is written in C#.

[GTest TAP Listener](https://github.com/kinow/gtest-tap-listener) is an event
listener for Google Test that implements the
[TAP protocol](http://en.wikipedia.org/wiki/Test_Anything_Protocol) for test
result output. If your test runner understands TAP, you may find it useful.

## About Google Mock ##

**Google Mock** is an extension to Google Test for writing and using C++ mock classes.
It is inspired by [jMock](http://www.jmock.org/), [EasyMock](http://www.easymock.org/),
and [Hamcrest](http://code.google.com/p/hamcrest/), and designed with C++'s specifics in mind.

Google mock:

  * lets you create mock classes trivially using simple macros.
  * supports a rich set of matchers and actions.
  * handles unordered, partially ordered, or completely ordered expectations.
  * is extensible by users.

We hope you find it useful!

## Using Google Mock Without Google Test ## 
Google Mock is not a testing framework itself.  Instead, it needs a
testing framework for writing tests.  Google Mock works seamlessly
with [Google Test](http://code.google.com/p/googletest/), butj
you can also use it with [any C++ testing framework](googlemock/ForDummies.md#Using_Google_Mock_with_Any_Testing_Framework).

## Getting Started ##

If you are new to the project, we suggest that you read the user
documentation in the following order:

  * Learn the [basics](http://code.google.com/p/googletest/wiki/Primer) of Google Test, if you choose to use Google Mock with it (recommended).
  * Read [Google Mock for Dummies](ForDummies.md).
  * Read the instructions on how to [build Google Mock](http://code.google.com/p/googlemock/source/browse/trunk/README).

You can also watch Zhanyong's [talk](http://www.youtube.com/watch?v=sYpCyLI47rM) on Google Mock's usage and implementation.

Once you understand the basics, check out the rest of the docs:

  * [CheatSheet](googlemock/docs/CheatSheet.md) - all the commonly used stuff at a glance.
  * [CookBook](googlemock/docs/CookBook.md) - recipes for getting things done, including advanced techniques.

If you need help, please check the [KnownIssues](googlemock/docs/KnownIssues.md) and
[FrequentlyAskedQuestions](googlemock/docs/frequentlyaskedquestions.md) before
posting a question on the [googlemock discussion group](http://groups.google.com/group/googlemock).

We'd love to have your help! Please read the Developer Guides if you are willing to contribute to the development.

Happy mocking!
