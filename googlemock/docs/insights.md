insights.md  
-a detailed look at some aspects of the proper usage of googlemocks/googletest

By [Gabriel Staples](https://github.com/ElectricRCAircraftGuy)  
30 Mar. 2020  

This answer was originally [posted on Stack Overflow here](https://stackoverflow.com/questions/44034633/google-mock-can-i-call-expect-call-multiple-times-on-same-mock-object/60905880#60905880) to answer this question: "**google mock - can I call EXPECT_CALL multiple times on same mock object?**"

It acts as a good intro. to the googletest/googlemock documentation, as well as an advanced guide with a bunch of insights into properly using gmock's `EXPECT_CALL()` macro.

Here it is:

------------------------------------------------------------------------------------------------------------------------

_All of the below code was tested with Googletest/Googlemock [v1.10.0][1], which was released 3 Oct. 2019._

If you'd like to run tests for yourself but don't have googletest or googlemock set up on your system, [here's a bare-bones project I created](https://github.com/ElectricRCAircraftGuy/eRCaGuy_gtest_practice) to get it up and running quickly on Ubuntu. Go clone it and play with it yourself. It may act as a starting point to help you get it running on Mac or Windows as well.

This is a really important question, so I feel compelled to have a hack at it. 

## The nuances:

Let me start by saying that Google Mock (gmock) is *nuanced*. That means there are lots of subtleties to understand, and this is difficult. Even the documentation is a bit scattered, and you kind of need to carefully read and study *it all* to really grasp some or even most of these nuances, since they don't do a good job of repeating certain important points in each of the documents. So, here is all of the official documentation: if you're doing this for work, tell your supervisor you are going to set aside several days to carefully go through the gtest and gmock documentation and to practice examples to get a strong grasp on it. 

## The documentation:
_As you read and study the below documentation, save each one as (print it to) a PDF, then use [Foxit Reader][3] for free on Windows, Mac, or Linux, to edit, take notes in, and highlight or underline the PDF as you go. This way you have notes of the most important things you need to remember. See my `*_GS_edit.pdf` PDFs [here][4] and [here][5] for examples of taking notes and marking up PDFs that I have done as I've learned Google Test and Google Mock._

**Official Google Documentation:**

1. gtest: it's all in this folder: https://github.com/google/googletest/tree/master/googletest/docs. The key documents to study, in this order probably, are the:
    1. **primer**
    1. **FAQ**
    1. **samples** (look at and carefully study the source code for *at least* the first 3 samples)
    1. advanced
2. gmock: it's all in this folder: https://github.com/google/googletest/tree/master/googlemock/docs. The key documents to study, in this order probably, are the:
    1. **for dummies**
    1. **cook book**
    1. **cheat sheet** - this is the best one-stop-shop or "summary of gmock rules" of all of the docs, but lacks some things that are even spelled out explicitly in (and only in) the "for dummies" manual that you will need in addition to this doc.
    1. **FAQ**
    1. **for dummies** <-- yes, AGAIN! AFTER doing and attempting to write a bunch of tests and mocks, then *come back* and re-read this document again! It will make sooo much more sense the 2nd time around after putting gtest and gmock principles into practice yourself first.

## Some subtle rules to remember in general:
1. "Remember that the test order is undefined, so your code can't depend on a test preceding or following another" (https://github.com/google/googletest/blob/master/googletest/docs/advanced.md#sharing-resources-between-tests-in-the-same-test-suite).
1. "**Important note:** gMock requires expectations to be set **before** the mock functions are called, otherwise the behavior is **undefined**. In particular, you mustn't interleave `EXPECT_CALL()`s and calls to the mock functions" (https://github.com/google/googletest/blob/master/googlemock/docs/for_dummies.md#using-mocks-in-tests)

---

## The answers:

### Question 1: "If I call `EXPECT_CALL` twice on the same mock object in the same `TEST_F` . . . what happens?"

A: First off, whether your're using the `TEST()` macro or the `TEST_F()` macro in this case makes no difference. The `TEST()` macro simply expands into a class which publicly inherits from the `::testing::Test` class, and the `TEST_F()` macro simply expands into a class which inherits from your test fixture class (the first parameter to `TEST_F()`), which must publicly inherit from the `::testing::Test` class.

Many `EXPECT_CALL`s can be called on the same mock object (mock class), from general to specific, as follows:

**The 3 rules of multiple `EXPECT_CALL`s on the same mock object:**  
From most generic --> most specific (AKA: "outer" --> "inner" scope).  

1. **You can have _at least_ one `EXPECT_CALL` per mock method:** One mock class can have many mocked methods, so each method may have one or more `EXPECT_CALL`s configuring the expected interaction with that method. Therefore, a mock class can have _at least_ one `EXPECT_CALL` per method.
1. **You _shouldn't have more than one_ `EXPECT_CALL` per matcher signature on a single mock method:** (Read more on this in Rule 3 below). Each mock method has many different parameter *values* that can be passed in, so you can have _up to_ one `EXPECT_CALL` per matcher signature (possible parameter value or combination of values, in the event of multiple input parameters). This means each mock method can have potentially many _thousands_ or even _millions or billions_ of _valid and unique_ `EXPECT_CALL`s attached to it, each matching a different set of "matchers", or input parameters to the mocked method. For example, this is perfectly valid:

        // Each `EXPECT_CALL()` in this example has a different and 
        // unique "matcher" signature, so every `EXPECT_CALL()` will
        // take effect for its matching parameter signature when
        // `myMockMethod()` is called.
        //                                    v--matchers
        EXPECT_CALL(myMockClass, myMockMethod(1));
        EXPECT_CALL(myMockClass, myMockMethod(2));
        EXPECT_CALL(myMockClass, myMockMethod(3));
        EXPECT_CALL(myMockClass, myMockMethod(4));
        EXPECT_CALL(myMockClass, myMockMethod(5));
        ...
        EXPECT_CALL(myMockClass, myMockMethod(1000));

    In particular, the above `EXPECT_CALL`s each specify that a call to `myMockMethod()` with that matching signature must occur _exactly 1 time_. That's because the [cardinality rules][6] in this case dictate than an implicit `.Times(1)` is present on each of those `EXPECT_CALL`s, even though you don't see it written.  

    To specify that you want a given `EXPECT_CALL` to mach *any* input value for a given parameter, use the `::testing::_` matcher, like this:

        using ::testing::_;

        EXPECT_CALL(myMockClass, myMockMethod(_));

1. **Don't have _duplicate `EXPECT_CALL`s with the same matcher signature_ on the same mock method, but _multiple_ `EXPECT_CALL`s with _overlapping/overriding_ (but NOT duplicate) matcher signatures on the same mock method are OK:** If you attach more than one `EXPECT_CALL` to the same *matching values*, only *the last one set* will have any effect. See [here][7], [here][8], and [here][9], for instance. This means if you have two or more `EXPECT_CALL`s with duplicate matcher signatures (the same parameters passed to the mock method), then ONLY THE LAST ONE WILL EVER GET ANY CALLS.

    Therefore, your test will ALWAYS FAIL except in the unusual case that all `EXPECT_CALL`s except the last one have a `.Times(0)` value, specifying that they will _never_ be called, as indeed this is the case: the last `EXPECT_CALL` will match all of the calls for these matchers and all duplicate `EXPECT_CALL`s above it will have _no_ matching calls! Here is an example of a test which will **always fail** as a result of this behavior. This is the main behavior that @luantkow focuses on [in his answer here](https://stackoverflow.com/a/44035118/4561887).

        using ::testing::_;

        // Notice they all have the same mock method parameter "matchers"
        // here, making only the last `EXPECT_CALL()` with this matcher
        // signature actually match and get called. Therefore, THIS TEST
        // WILL ***ALWAYS FAIL***, since EXPECT_CALL #1 expects to get 
        // called 1 time but is NEVER called, #2 through #1006, inclusive,
        // all expect to get called 2 times each but all of them are NEVER
        // called, etc.! Only #1007 is ever called, since it is last and
        // therefore always matches first.          
        //                                    v--matchers
        EXPECT_CALL(myMockClass, myMockMethod(_)).Times(1); // EXPECT_CALL #1
        EXPECT_CALL(myMockClass, myMockMethod(_)).Times(2); // EXPECT_CALL #2
        EXPECT_CALL(myMockClass, myMockMethod(_)).Times(2); // EXPECT_CALL #3
        EXPECT_CALL(myMockClass, myMockMethod(_)).Times(2); // EXPECT_CALL #4
        EXPECT_CALL(myMockClass, myMockMethod(_)).Times(2); // EXPECT_CALL #5
        EXPECT_CALL(myMockClass, myMockMethod(_)).Times(2); // EXPECT_CALL #6
        // ... duplicate the line just above 1000 more times here
        EXPECT_CALL(myMockClass, myMockMethod(_)).Times(3); // EXPECT_CALL #1007

    This weird exception, however, makes the test valid simply by setting all duplicate `EXPECT_CALL`s, _except for the last one_, to have a `.Times(0)` cardinal setting:

        using ::testing::_;

        // Notice they all have the same mock method parameter "matchers"
        // here, making only the last `EXPECT_CALL()` with this matcher
        // signature actually match and get called. However, since all previous
        // `EXCEPT_CALL` duplicates are set to `.Times(0)`, this test is valid
        // and can pass.          
        //                                    v--matchers
        EXPECT_CALL(myMockClass, myMockMethod(_)).Times(0); // EXPECT_CALL #1
        EXPECT_CALL(myMockClass, myMockMethod(_)).Times(0); // EXPECT_CALL #2
        EXPECT_CALL(myMockClass, myMockMethod(_)).Times(0); // EXPECT_CALL #3
        EXPECT_CALL(myMockClass, myMockMethod(_)).Times(0); // EXPECT_CALL #4
        EXPECT_CALL(myMockClass, myMockMethod(_)).Times(0); // EXPECT_CALL #5
        EXPECT_CALL(myMockClass, myMockMethod(_)).Times(0); // EXPECT_CALL #6
        // ... duplicate the line just above 1000 more times here
        EXPECT_CALL(myMockClass, myMockMethod(_)).Times(3); // EXPECT_CALL #1007

    Here, only `EXPECT_CALL` #1007 (the very last `EXPECT_CALL`) will ever match a call to `myMockMethod()`, and `Times(3)` will be in effect. Since all duplicate `EXPECT_CALL`s above this one will NEVER MATCH AND GET CALLED, since they are never reached, tests with duplicate `EXPECT_CALL`s for a given matcher would ALWAYS FAIL for any `.Times()` value other than `.Times(0)` for all non-last-place duplicate `EXPECT_CALL`s. 

    This effect of making later matchers have the ability to override earlier matchers is *intentional* and part of the Googlemock design, as it allows you to create a very useful kind of hierarchy of expected calls, based on value passed to the mock method, like this:

        using ::testing::_;

        // Most general matchers first (_ matches any input value)
        EXPECT_CALL(myMockClass, myMockMethod(_)).Times(1);
        // More specific matchers next, to override the more general matcher 
        // above if they match
        EXPECT_CALL(myMockClass, myMockMethod(7)).Times(2);
        EXPECT_CALL(myMockClass, myMockMethod(5)).Times(4);

    The various google documents say that matching `EXPECT_CALL`s are searched for in *reverse order*, from *bottom to top*. So, if `myMockMethod(8)` is called, it will be checked against the last `EXPECT_CALL` for this method, which is looking for `myMockMethod(5)`. That doesn't match, so it goes up one and checks against `myMockMethod(7)`. That doesn't match, so it goes up one and checks against `myMockMethod(_)`. This matches! So, it counts as the one call authorized by the `Times(1)` cardinal value. 

    So, what you've defined above is this: we expect `myMockMethod(5)` to be called 4 times, `myMockMethod(7)` to be called 2 times, and `myMockMethod(anything_other_than_5_or_7)` to be called 1 time. For more reading on this topic, see my other answer here: https://stackoverflow.com/questions/44249293/google-mock-how-to-say-function-must-be-called-once-with-a-certain-parameter/60896373#60896373.

**Key summary:** the main point to remember regarding the question "Can I call `EXPECT_CALL` multiple times on same mock object?", is this: you can only call `EXPECT_CALL` multiple times on the same mock object _and method_ if the matchers (arguments specified to be passed to the mocked method) are _different_ for each `EXPECT_CALL`. That is, of course, unless you set `.Times(0)` on all but the last duplicate `EXPECT_CALL`, which makes them useless, so just remember instead to not have duplicate `EXPECT_CALL`s with the same matchers.

That fully answers this question. 

---

### Question 2: "Are the expectations appended to the mock object or does the second call erase the effects of the first call?"

The above description answers this question as well. Essentially, the `EXPECT_CALL` expectations do NOT override the effects of any `EXPECT_CALL`s before them, **unless** the matchers (values specified to be passed to the mock methods) are identical or overlapping, in which case only the *last* `EXPECT_CALL` will ever get called at all, as it is always reached before the others in the matching sequence. Therefore, don't have duplicate `EXPECT_CALL`s with the same matchers on a given mocked method or else you could be inadvertently forcing the test to _always fail_, since the above `EXPECT_CALL`s will never get called. This is discussed at length in Question 1 above.

Again, for more reading on this topic, read above, and see my other answer here: https://stackoverflow.com/questions/44249293/google-mock-how-to-say-function-must-be-called-once-with-a-certain-parameter/60896373#60896373.

---

### Question 3: Can I call `EXPECT_CALL` to set some expectations on a mock method, call the mock method, then call `EXPECT_CALL` on the method again to change the expectations, then call the mock method again?

This question wasn't even *explicitly* asked by the OP, but the only reason I found this page is because I was searching for this answer for many hours and couldn't find it. My Google search was "[gmock multiple expect_call][10]." Therefore, others asking this question will also fall on this page and need a conclusive answer. 

A: NO, you can NOT do this! Although it may *seem to work in testing*, according to Google, it produces *undefined behavior*. See general rule #2 above!

> "**Important note:** gMock requires expectations to be set **before** the mock functions are called, otherwise the behavior is **undefined**. In particular, you mustn't interleave `EXPECT_CALL()`s and calls to the mock functions" (https://github.com/google/googletest/blob/master/googlemock/docs/for_dummies.md#using-mocks-in-tests)

Therefore, this is NOT ALLOWED!

    // EXAMPLE OF A BAD TEST THAT MAY SEEM TO WORK BUT IS RELYING ON *UNDEFINED* BEHAVIOR!
    // The goal is to ensure that `myMockMethod()` is only called 2x the first time by 
    // `myOtherFunc()`, 3x the second time, and 0x the last time.

    // Google states: "**Important note:** gMock requires expectations to be set 
    // **before** the mock functions are called, otherwise the behavior is **undefined**. 
    // In particular, you mustn't interleave `EXPECT_CALL()`s and calls to the mock functions"
    // (https://github.com/google/googletest/blob/master/googlemock/docs/for_dummies.md#using-mocks-in-tests)

    using ::testing::_;

    TEST_F(MyTestFixture, MyCustomTest) 
    {
        // `myMockMethod()` should be called only 2x here by `myOtherFunc()`,
        // despite calling `myOtherFunc()` repeatedly
        EXPECT_CALL(MyMockClass, myMockMethod(_, _))
            .Times(2);
        for (int i = 0; i < 10; i++)
        {
            myOtherFunc();
        }

        // UNDEFINED BEHAVIOR BEGINS HERE: you can't interleave calls to `EXPECT_CALL` with 
        // calls to the mocked functions (in this case: `myMockMethod()`,
        // which is called by `myOtherFunc()`).

        // THEN `myMockMethod()` should be called 3x here by `myOtherFunc()`
        EXPECT_CALL(MyMockClass, myMockMethod(_, _))
            .Times(3);
        for (int i = 0; i < 10; i++)
        {
            myOtherFunc();
        }

        // LAST, `myMockMethod()` should be called 0x here by `myOtherFunc()`
        EXPECT_CALL(MyMockClass, myMockMethod(_, _))
            .Times(0);
        for (int i = 0; i < 10; i++)
        {
            myOtherFunc();
        }
    }

So, what's a valid solution here? Well, if you can break this test up into 3 different, independent tests, just do that! But, what if these 3 tests are interconnected in such a way you cannot separate them? Example: you are trying to test a throttling function which throttles print output to only once per second, for instance, even if you try to print more often that that per second. Well, in this case there are some work-arounds. 

First, let's review: per the [Google Mock Cheat Sheet][11], here are the ways to configure an `EXPECT_CALL()`:

    EXPECT_CALL(mock-object, method (matchers)?)
         .With(multi-argument-matcher)  ?
         .Times(cardinality)            ?
         .InSequence(sequences)         *
         .After(expectations)           *
         .WillOnce(action)              *
         .WillRepeatedly(action)        ?
         .RetiresOnSaturation();        ?

> For each item above, `?` means it can be used at most once, while `*` means it can be used any number of times.

We need to use the `.WillRepeatedly(action)` option with an `action` which [produces side effects][12] or [calls a function, functor, or lambda][13] as an action.

Here are some work-arounds to safely and correctly perform the above test which had undefined behavior. _If you want to see the best approach first, just jump straight down to #3 below:_

1. **Use `Assign(&variable, value)`.** In this particular case, this is a bit hacky, but it does work properly. For a simpler test case you may have, this might be the perfect way to do what you need. Here's a viable solution:

    Side note: an error output I got while trying to run a gmock test said: 

    > `.Times()` cannot appear after `.InSequence()`, `.WillOnce()`, `.WillRepeatedly()`, or `.RetiresOnSaturation()`,

    ...so it turns out we don't need to (and we are not even _allowed to_) specify `.Times(::testing::AnyNumber())` here. Instead, gmock will figure it out automatically, according to [these cardinality rules][6], since we are using `.WillRepeatedly()`: 

    > **If you omit `Times()`, gMock will infer the cardinality for you.** The rules are easy to remember:

    > - If **neither** `WillOnce()` nor `WillRepeatedly()` is in the `EXPECT_CALL()`, the inferred cardinality is `Times(1)`.
    > - If there are _n_ `WillOnce()`'s but **no** `WillRepeatedly()`, where _n_ >= 1, the cardinality is `Times(n)`.
    > - If there are _n_ `WillOnce()`'s and **one** `WillRepeatedly()`, where _n_ >= 0, the cardinality is `Times(AtLeast(n))`.

    _This technique has actually been tested and proven to work on real code:_

        using ::testing::_;
        using ::testing::Assign;

        TEST_F(MyTestFixture, MyCustomTest) 
        {
            bool myMockMethodWasCalled = false;

            EXPECT_CALL(MyMockClass, myMockMethod(_, _))
                // Set `myMockMethodWasCalled` to true every time `myMockMethod()` is called with
                // *any* input parameters!
                .WillRepeatedly(Assign(&myMockMethodWasCalled, true));

            // Do any necessary setup here for the 1st sub-test 

            // Test that `myMockMethod()` is called only 2x here by `myOtherFunc()`,
            // despite calling `myOtherFunc()` repeatedly
            for (int i = 0; i < 10; i++)
            {
                myOtherFunc();

                if (i < 2)
                {
                    EXPECT_TRUE(myMockMethodWasCalled);
                    myMockMethodWasCalled = false;        // reset
                    EXPECT_FALSE(myMockMethodWasCalled);  // ensure reset works (sanity check)
                }
                else
                {
                    EXPECT_FALSE(myMockMethodWasCalled);
                }
            }

            // Do any necessary setup here for the 2nd sub-test

            // Test that `myMockMethod()` is called only 3x here by `myOtherFunc()`,
            // despite calling `myOtherFunc()` repeatedly
            for (int i = 0; i < 10; i++)
            {
                myOtherFunc();

                if (i < 3)
                {
                    EXPECT_TRUE(myMockMethodWasCalled);
                    myMockMethodWasCalled = false;        // reset
                    EXPECT_FALSE(myMockMethodWasCalled);  // ensure reset works (sanity check)
                }
                else
                {
                    EXPECT_FALSE(myMockMethodWasCalled);
                }
            }

            // Do any necessary setup here for the 3rd sub-test

            // Test that `myMockMethod()` is called 0x here by `myOtherFunc()`,
            // despite calling `myOtherFunc()` repeatedly
            for (int i = 0; i < 10; i++)
            {
                myOtherFunc();
                EXPECT_FALSE(myMockMethodWasCalled);
            }
        }

2. **Use `InvokeWithoutArgs(f)` with a global counter variable and a global counter function.** This works great, and is much easier to use and more versatile than the previous approach! Note that you could also migrate this global function and variable to be inside your test fixture class as well if you wanted, which would clean it up a bit.

    _This technique has actually been tested and proven to work on real code:_

        using ::testing::_;
        using ::testing::InvokeWithoutArgs;

        static uint32_t callCounter = 0;
        static void incrementCallCounter()
        {
            callCounter++;
        }

        TEST_F(MyTestFixture, MyCustomTest)
        {
            EXPECT_CALL(MyMockClass, myMockMethod(_, _))
                // Set gmock to increment the global `callCounter` variable every time 
                // `myMockMethod()` is called with *any* input parameters!
                .WillRepeatedly(InvokeWithoutArgs(incrementCallCounter));

            // Do any necessary setup here for the 1st sub-test 

            // Test that `myMockMethod()` is called only 2x here by `myOtherFunc()`,
            // despite calling `myOtherFunc()` repeatedly
            callCounter = 0; // ensure this is zero BEFORE you start the test!
            for (int i = 0; i < 10; i++)
            {
                myOtherFunc();
            }
            EXPECT_EQ(callCounter, 2);

            // Do any necessary setup here for the 2nd sub-test 

            // Test that `myMockMethod()` is called only 3x here by `myOtherFunc()`,
            // despite calling `myOtherFunc()` repeatedly
            callCounter = 0; // ensure this is zero BEFORE you start the test!
            for (int i = 0; i < 10; i++)
            {
                myOtherFunc();
            }
            EXPECT_EQ(callCounter, 3);

            // Do any necessary setup here for the 1st sub-test 

            // Test that `myMockMethod()` is called 0x here by `myOtherFunc()`,
            // despite calling `myOtherFunc()` repeatedly
            callCounter = 0; // ensure this is zero BEFORE you start the test!
            for (int i = 0; i < 10; i++)
            {
                myOtherFunc();
            }
            EXPECT_EQ(callCounter, 0);
        }

3. **[BEST TECHNIQUE] Use `InvokeWithoutArgs(f)` with a _local_ counter variable and a simple lambda function!** This works great, and is much easier to use and more versatile than the 1st approach, while avoiding the global variable and additional global function of the 2nd approach. It is for sure my favorite way to handle this, and works extremely well.

    _This technique has actually been tested and proven to work on real code:_

        using ::testing::_;
        using ::testing::InvokeWithoutArgs;

        TEST_F(MyTestFixture, MyCustomTest)
        {
            uint32_t callCounter;

            EXPECT_CALL(MyMockClass, myMockMethod(_, _))
                // Use a lambda function to set gmock to increment `callCounter` every 
                // time `myMockMethod()` is called with *any* input parameters!
                .WillRepeatedly(InvokeWithoutArgs([&callCounter](){ callCounter++; }));

            // Do any necessary setup here for the 1st sub-test 

            // Test that `myMockMethod()` is called only 2x here by `myOtherFunc()`,
            // despite calling `myOtherFunc()` repeatedly
            callCounter = 0; // ensure this is zero BEFORE you start the test!
            for (int i = 0; i < 10; i++)
            {
                myOtherFunc();
            }
            EXPECT_EQ(callCounter, 2);

            // Do any necessary setup here for the 2nd sub-test 

            // Test that `myMockMethod()` is called only 3x here by `myOtherFunc()`,
            // despite calling `myOtherFunc()` repeatedly
            callCounter = 0; // ensure this is zero BEFORE you start the test!
            for (int i = 0; i < 10; i++)
            {
                myOtherFunc();
            }
            EXPECT_EQ(callCounter, 3);

            // Do any necessary setup here for the 1st sub-test 

            // Test that `myMockMethod()` is called 0x here by `myOtherFunc()`,
            // despite calling `myOtherFunc()` repeatedly
            callCounter = 0; // ensure this is zero BEFORE you start the test!
            for (int i = 0; i < 10; i++)
            {
                myOtherFunc();
            }
            EXPECT_EQ(callCounter, 0);
        }

If you think this whole answer should be added as a stand-alone file among the Gmock docs (I propose we do this), click the github issue link just below and upvote it.

## Practice using gtest/gmock:
1. Use this project to practice writing and testing your own google tests and google mocks. This is also a good demo of how to get a new project up and running with Google's [Bazel build system][14]: https://github.com/ElectricRCAircraftGuy/eRCaGuy_gtest_practice.

## Related:
1. GitHub issue I opened to request to add this answer as a standalone document in their official documentation. If you agree, please go here and upvote this issue: https://github.com/google/googletest/issues/2775
1. https://stackoverflow.com/questions/44249293/google-mock-how-to-say-function-must-be-called-once-with-a-certain-parameter/60896373#60896373
1. https://stackoverflow.com/questions/5743236/google-mock-multiple-expectations-on-same-function-with-different-parameters/5899945
1. https://stackoverflow.com/questions/44249293/google-mock-how-to-say-function-must-be-called-once-with-a-certain-parameter


  [1]: https://github.com/google/googletest/releases
  [2]: https://stackoverflow.com/a/44035118/4561887
  [3]: https://www.foxitsoftware.com/pdf-reader/
  [4]: https://github.com/ElectricRCAircraftGuy/eRCaGuy_dotfiles/tree/master/googletest/gtest
  [5]: https://github.com/ElectricRCAircraftGuy/eRCaGuy_dotfiles/tree/master/googletest/gmock
  [6]: https://github.com/google/googletest/blob/master/googlemock/docs/for_dummies.md#cardinalities-how-many-times-will-it-be-called
  [7]: https://github.com/google/googletest/blob/master/googlemock/docs/for_dummies.md#using-multiple-expectations-multiexpectations
  [8]: https://github.com/google/googletest/blob/master/googlemock/docs/for_dummies.md#all-expectations-are-sticky-unless-said-otherwise-stickyexpectations
  [9]: https://github.com/google/googletest/blob/master/googlemock/docs/cook_book.md#performing-different-actions-based-on-the-arguments
  [10]: https://www.google.com/search?sxsrf=ALeKk00e6UHbLaPL3B5-042GF9KQRt9QhA%3A1585335955965&ei=k05-Xq7LOri10PEPwL-ciA8&q=gmock%20multiple%20expect_call&oq=gmock%20multiple%20&gs_l=psy-ab.3.1.0l3j0i22i30l7.2786111.2787311..2789283...0.4..0.305.1339.0j8j0j1......0....1..gws-wiz.......0i71.e3x6IuEQpb0
  [11]: https://github.com/google/googletest/blob/master/googlemock/docs/cheat_sheet.md#setting-expectations-expectcall
  [12]: https://github.com/google/googletest/blob/master/googlemock/docs/cheat_sheet.md#side-effects
  [13]: https://github.com/google/googletest/blob/master/googlemock/docs/cheat_sheet.md#using-a-function-functor-or-lambda-as-an-action
  [14]: https://bazel.build/

