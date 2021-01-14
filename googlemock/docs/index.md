# googletest Home

go/gmock

Googletest is Google's C++ testing and mocking framework. Please note that there
are legacy names you may encounter "gUnit" and "gMock" - these names are now
merged into "googletest"

<!-- GOOGLETEST_CM0035 DO NOT DELETE -->

## Testimonials

> "I'm really enjoying trying it, and it's amazing to me how far you've taken
> this in C++. It's changed the way I program (and therefore changed my life ;),
> and one of my teams has adopted it for all/most tests (and I'm working on the
> other)." \
> -- *Derek Thomson*, Google Mountain View

<section></section>

> "I started using mocks with EasyMock in Java a few years ago and found them
> **invaluable** for making unit testing as painless and effective as possible.
> I'm very glad (and amazed) to see you've managed to create something similar
> for C++. It's making the transition much more pleasant." \
> -- *David Harkness*, Google Mountain View

<section></section>

> "I #included `gmock.h` and lived to tell the tale... Kept me from having to
> depend on `MockBigtable` thus far, which is **huge**." \
> -- *Matthew Simmons*, Google NYC

<section></section>

> "I like the approach of `EXPECT_CALL` much more than EasyMock's mock modes
> (record, replay). It's the best way to ensure the user will never forget to
> verify the expectations: do it automatically!" \
> -- *Tiago Silverira*, Google Brazil

<section></section>

> "It's by far the best mocking library for C++, by a long-shot." \
> -- *Joe Walnes*, co-creator of jMock, Google London

## Learning googletest mocking

Please see the [*googletest Users Guide*](guide.md) for the combined gMock
mocking documentation.

## Resources for Users

*   More docs:
    *   [Interview with gMock's Creator](http://www.corp.google.com/eng/testing/codegreen/v10/gMock.htm)
        on the
        [Feb 2008](http://www.corp.google.com/eng/testing/codegreen/v10/index.htm)
        issue of [Code Green](http://go/codegreen) - discusses gMock's history
        and philosophy.
    *   "Mockers of the (C++) world, delight!": TotT
        [episode 66](http://big.corp.google.com/~jmcmaster/testing/2007/12/episode-68-mockers-of-c-world-delight.html) -
        quick intro on gMock's benefits and usage
    *   "Mock logs better than gold": TotT
        [episode 76](http://big.corp.google.com/~jmcmaster/testing/2008/02/episode-76-mock-logs-better-than-gold_21.html) -
        how to test LOGs using gMock
    *   "Testing legacy code gently": TotT
        [episode 84](http://big.corp.google.com/~jmcmaster/testing/2008/04/episode-84-testing-legacy-code-gently.html) -
        using mock callbacks to test legacy code without a big refactoring
    *   "Literate testing with matchers": TotT
        [episode 135](http://big.corp.google.com/~jmcmaster/testing/2009/06/episode-135-literate-testing-with_08.html) -
        using matchers to get readable test code and readable test messages
    *   "Making a perfect matcher": TotT
        [episode 139](http://big.corp.google.com/~jmcmaster/testing/2009/07/episode-139-making-perfect-matcher.html) -
        defining custom matchers easily
*   Talks
    *   "Declarative C++ Testing Using DSLs" talk (6/4/2008):
        [abstract](https://wiki.corp.google.com/twiki/bin/view/Main/WanTalks#Declarative_C_Testing_Using_DSLs),
        [slides](http://wiki.corp.google.com/twiki/pub/Main/WanTalks/0806-declarative-cpp-testing.xul#Eva)
        (requires Firefox) - gMock's design and implementation tricks
    *   "Mocks made easy in C++ and Java" talk (4/23/2008):
        [slides](http://go/MockTalk),
        [fish](http://fish.corp.google.com/talks/8729/)
    *   "C++ mocks made easy - an introduction to gMock" talk (1/22/2008)):
        [slides](http://wiki.corp.google.com/twiki/pub/Main/WanTalks/0801-mv-gmock.xul#eva)
        (requires Firefox),
        [video](https://video.google.com/a/google.com/?AuthEventSource=SSO#/Play/contentId=bd07003d4193a646)
    *   "A preview to gMock" talk (6/28/2007):
        [PowerPoint slides](http://wiki.corp.google.com/twiki/pub/Main/WanTalks/0706-beijing-gmock-preview.ppt)
*   Tools
    *   `/google/src/head/depot/google3/third_party/googletest/googlemock/scripts/generator/gmock_gen.py
        *your.h ClassNames*` generates mocks for the given base classes (if no
        class name is given, all classes in the file are emitted).
*   Mocks
    *   [mock-log.h](http://s/?fileprint=//depot/google3/testing/base/public/mock-log.h) -
        a sample on using gMock to create a mock class
    *   [gmock-sample-mock-log.cc](http://s/?fileprint=//depot/google3/testing/base/internal/gmock-sample-mock-log.cc) -
        a sample on using gMock to test LOG()s
*   Folks
    *   Meet the
        [users](http://piano.kir.corp.google.com:8080/lica/?e=use%3Agmock).
    *   `gmock-users` list:
        [subscribe](https://groups.google.com/a/google.com/group/gmock-users/topics),
        [archive](https://groups.google.com/a/google.com/group/gmock-users/topics),
        [smile!](http://piano.kir.corp.google.com:8080/lica/?e=gmock-users) Send
        questions here if you still need help after consulting the on-line docs.
    *   `gmock-announce` list:
        [subscribe](https://groups.google.com/a/google.com/group/gmock-announce/topics)
        to this instead of `gmock-users` if you are interested in announcements
        only.

## Resources for Contributors

*   [Dashboard](http://unittest.corp.google.com/project/gunit-gmock/)
*   [*gMock Design*](design.md) (go/gmockdesign) - the design doc
*   `c-mock-dev` list (deprecated) -
    [old archive](https://mailman.corp.google.com/pipermail/c/c-mock-dev/),
    [new archive](https://g.corp.google.com/group/c-mock-dev-archive)
*   `opensource-gmock` list - discussions on the development of gMock:
    [subscribe](https://groups.google.com/a/google.com/group/opensource-gmock/subscribe),
    [archive](https://g.corp.google.com/group/opensource-gmock-archive),
    [smile!](http://piano.kir.corp.google.com:8080/lica/?e=opensource-gmock)

## Acknowledgments

We'd like to thank the following people for their contribution to gMock: Piotr
Kaminski, Jeffrey Yasskin (who/jyasskin), Joe Walnes, Bradford Cross, Keith Ray,
Craig Silverstein, Matthew Simmons (who/simmonmt), Hal Burch (who/hburch), Russ
Rufer, Rushabh Doshi (who/rdoshi), Gene Volovich (who/genev), Mike Bland, Neal
Norwitz (who/nnorwitz), Mark Zuber, Vadim Berman (who/vadimb).
