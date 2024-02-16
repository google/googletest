## Notes on hyper-flexible ordering

This analysis is of a deprecated version of cotest that allowed test corotuines more
flexibility, so that they could queue up incoming events and then pull them from the queue out of order. 

### NEXT_EVENT_ONLY_FROM(DC) or BLOCK_EVENT()
 - Consider launches A() and B() invoking mockA() and mockB() respectively. Suppose the
   test case requires:
   - A() must be called before B() eg because at least one has side effects and 
     they affect each other's behaviour.
   - But mockA()'s handling in the test depends on an arg to mockB()
   - Assume it isn't enough for mockB() to affect mockA()'s return value 
     (because we could leave mockA.RETURN() until after mockB.ACCEPT())
   - Assume it isn't enough to do checking using EXPECT_EQ() etc either
   - This leaves the case where the decision of whether to accept or drop
     mockA() depends on mockB()'s argument.
 - Now, based on GMock design:
   - In order to have mockB() arguments we need to have entered ShouldHandleCall() 
     on B(). If we look at B's PreMock, we might see arguments for a mock 
     call that is accepted by a higher prio expectation or watch. So
     higher prio expectations/watches on mockB() can affect the arg values we see.
   - We have to decide whether to accept or drop mockA() before we exit
     ShouldHandleCall() on A(). Our decision here will obviously affect 
     the lower prio expectations/watches on mockA().
 - So we're stuck with searching mockB() higher priorities before mockA() 
   lower priorities, which is out of sequence. Consequences:
   - Thread safety: we'd re-enter GMock's call hander search. At present this
     causes a deadlock, but that could be changed. 
   - Expectations would be OK, if we checked eg mockA() highers, 
     mockB() highers, mockA() lowers, mockB() lowers. This generalises
     to checking in sequence within each prio level. An expectation only 
     exists at one prio level, so it would see everything in the right order.
     Call this a shear ordering.
   - If another coro is watching mockA() at a lower prio and mockB() at 
     a higher prio, and it expects to see mockA() first, then we have
     conflicting requirements. If the original coro gets its way, then
     this coro will see mockB() and may drop it, fail a check, etc.
     So shear ordering is not good enough for coroutines.
   - Note that this is not an isue of side-effects inside ShouldHandleCall()
     and purely relates to the order in which mock calls are passed to it.

### Architectural interpretation
 - Coroutines should be able to infer the order in which mock calls
   are happening from the order in which they are passed to 
   ShouldHandleCall(). Shear ordering prevents this.
 - Without threads, this can be well-defined because coroutines have 
   well-defined ordering. 
 - With threads, we'd probably want an overall serialisation/rendez-vous point.
 - To provide for the above scenario without using shear ordering, 
   we need to intervene and change the global ordering as seen
   by GMock search. This should be explicit, since it will have side
   effects on other coroutines. 
 - Leads to the PreMockSynchroniser and CRF constraint #6
