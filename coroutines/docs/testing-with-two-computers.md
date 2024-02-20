# Testing With Two Computers
_This is a simplified overview of conventional and coroutine-based unit testing. We explore the steps taken by a convnetional test and then present the same test in a coroutine model. We use an analogy with computers that can "only do one thing at a time" and then discuss how coroutines fit in at the end._
## Conventional testing, using one computer
For this exercise, we will assume that the computers illustrated can only run a single "program" at a time (but see [footnote](#Footnote)).
![My test case makes a checklist of events that I will expect during the test.](/coroutines/docs/images/current_1.png)
Text after
![I am starting the code-under-test, with an injected dependancy back to me.](/coroutines/docs/images/current_2.png)
Text after
![I received a call on the injected dependency. Thinks: I can't just let my test case continue here. I will use the checklist to find out whether it is correct.](/coroutines/docs/images/current_3.png)
Text after
![The call was correct! Now the checklist tells me how to respond to the call.](/coroutines/docs/images/current_4.png)
Text after
![The code-under-test has completed and I check its output. Test passed!](/coroutines/docs/images/current_5.png)
## The same test, using two computers
![I say "please start the code-under-test". I start the code and inject dependency back to me.](/coroutines/docs/images/cotest_1.png)
Text after
![I say "I received this mock call". Inside my test case, I check that the call is correct.](/coroutines/docs/images/cotest_2.png)
Text after
![The call was correct! I say "please return this value". I return the mock call.](/coroutines/docs/images/cotest_3.png)
Text after
![I say "the code-under-test has completed and here is its output". I check the output. Test passed!](/coroutines/docs/images/cotest_4.png)
Text after
## How do we get to coroutines
## Footnote
The machine shown is a PDP-11/35 minicomputer system which does in fact support timesharing. You would not need two of them to run cotest!


