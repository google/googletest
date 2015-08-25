

Please follow **Google Mock** to its new home on **Git Hub**!
> http://github.com/google/googlemock


---


Welcome to **Google C++ Mocking Framework**!

Inspired by [jMock](http://www.jmock.org/), [EasyMock](http://www.easymock.org/), and [Hamcrest](http://code.google.com/p/hamcrest/), and
designed with C++'s specifics in mind, Google C++ Mocking Framework
(or **Google Mock** for short) is a library for writing and using C++
mock classes.  Google Mock:

  * lets you create mock classes trivially using simple macros,
  * supports a rich set of matchers and actions,
  * handles unordered, partially ordered, or completely ordered expectations,
  * is extensible by users, and
  * works on Linux, Mac OS X, Windows, Windows Mobile, minGW, and Symbian.

We hope you find it useful!

## Who are using Google Mock? ##

We have enjoyed using Google Mock in many projects at Google.  Outside of Google, the most notable client is probably the [Chromium projects](http://www.chromium.org/) (behind the Chrome browser and Chrome OS).  If you know of a project that's using Google Mock and want it to be listed here, please let
`googlemock@googlegroups.com` know.


## System Requirements ##

Google Mock is not a testing framework itself.  Instead, it needs a
testing framework for writing tests.  Google Mock works seamlessly
with [Google Test](http://code.google.com/p/googletest/).  It comes
with a copy of Google Test bundled.  Starting with version 1.1.0,
you can also use it with [any C++ testing framework of your choice](ForDummies#Using_Google_Mock_with_Any_Testing_Framework.md).

Google Mock has been tested with **gcc 4.0+** and **Microsoft Visual C++ 8.0 SP1**.  Users
reported that it also works with **gcc 3.4**, **Microsoft Visual C++ 7.1**, and **Cygwin**, although we haven't tested it there ourselves.

## Getting Started ##

If you are new to the project, we suggest to read the user
documentation in the following order:

  * Learn the [basics](http://code.google.com/p/googletest/wiki/Primer) of Google Test, if you choose to use Google Mock with it (recommended).
  * Read [Google Mock for Dummies](ForDummies.md).
  * Read the instructions on how to [build Google Mock](http://code.google.com/p/googlemock/source/browse/trunk/README).

You can also watch Zhanyong's [talk](http://www.youtube.com/watch?v=sYpCyLI47rM) on Google Mock's usage and implementation.

Once you understand the basics, check out the rest of the docs:

  * CheatSheet - all the commonly used stuff at a glance.
  * CookBook - recipes for getting things done, including advanced techniques.

If you need help, please check the KnownIssues and FrequentlyAskedQuestions before
posting a question on the [googlemock](http://groups.google.com/group/googlemock)
discussion group.

We'd love to have your help!  Please
read the DevGuide if you are willing to contribute to the development.

Happy mocking!