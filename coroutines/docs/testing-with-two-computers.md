# Testing With Two Computers
_This is a simplified overview of conventional and coroutine-based unit testing. We explore the steps taken by a convnetional test and then present the same test in a coroutine model. We use an analogy with computers that can "only do one thing at a time" and then discuss how coroutines fit in at the end._
## Conventional testing, using one computer
For this exercise, we will assume that the computers illustrated can only run a single "program" at a time. Therefore, whenever code calls a function, the computer is committed to running that function to completion before it can do anything else[^1].

The computer is running a test case, which is a section of code written by a test engineer, against an API provided by a testing framework. To begin with, the test uses this API to fill in a kind of _checklist_ (data structure) with information about what function calls should come _out_ of the code-under-test.
![My test case makes a checklist of events that I will expect during the test.](/coroutines/docs/images/current_1.png)
Having filled in the checklist, we are now ready to begin the test proper. We use a coding practice called _dependency injection_ to ensure that some or all of the _outgoing_ function calls from the code-under-test actually come to us and not their original intended destination. There are a few different ways to do this, and the best option generally depends on the programming language in use. We implement this and call the code-under-test.
![I am starting the code-under-test, with an injected dependancy back to me.](/coroutines/docs/images/current_2.png)
The code-under-test has begun running on our computer. We must wait for it to either complete, or make a call that will come to us becuse of our dependency injection. 

For the sake of example, let's assume a call comes to us. Because our computer is tied up with the call, it is not free to continue executing. Instead, it must handle the mock call in a specific place, separate from where it called the code-under-test. Therefore, it must use variables to keep track of how far through the test it is. This is why a checklist is used: we keep track by "ticking off" items on the checklist as we receive the corresponding calls.
![I received a call on the injected dependency. Thinks: I can't just let my test case continue here. I will use the checklist to find out whether it is correct.](/coroutines/docs/images/current_3.png)


It is natural for the checklist to also contain an action to be taken and the required return value for the call. We can insert our own code here, but it is not freely running code: it needs to be put in the place alloted to it. 
![The call was correct! Now the checklist tells me how to respond to the call.](/coroutines/docs/images/current_4.png)
We will assume that the code-under-test now completes after making one call to us. The function call we made earlier on to start the code-under-test now returns, and we can check the return value using free-running code. We are also free to begin another test.
![The code-under-test has completed and I check its output. Test passed!](/coroutines/docs/images/current_5.png)
### Alternative method
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
text

[^1]: The machine shown is a PDP-11/35 minicomputer system which does in fact support timesharing. You would not need two of them to run cotest!
