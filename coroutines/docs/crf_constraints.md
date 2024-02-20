## CRF Constraints

### 1. Test setup in coroutine 
Test coroutine must run as soon as its constructed, so as to allow it to 
create mock objects, watches etc before the mock source runs. Also in phase 2, coroutine can create mock 
sources.
 
### 2. Mock args passed by ref 
MUT must not do anything between making a mock call and the coroutine body
receiving the args. Note that the coroutine CAN run during this period, since the args are owned
by the mock source.

### 3. Mock return passed by ref 
Test coroutine must not do anything between Return() and the mock source receiving the 
return value. Mock source could in principle run during this period, since coro owns the return object, but 
in practice it won't because it's waiting for the return value.

### 4. Updated cardinality state 
Coroutine must run so that it can do SATISFY(), RETIRE() or run to exit BEFORE 
allowing GMock/CotestWatcher code to query eg IsSatisfied() etc. This is extra_iteration_requested reason
inside TestCoroutine::DestructionIterations(). 

### 5. Global mock handler finding 
After getting an event that is a mock call, test coroutines must call DROP()
ACCEPT() or RETURN() on the event before doing anything elase with the cotest UI. This is because the
process of deciding which expectation or coroutine will handle the call is global and must complete before
anything happens that could generate more mock calls.

### 6. True event sequence 
Most of the time, a LAUNCH() or RETURN() should be followed by NEXT_EVENT() because 
they will usually lead to an event for the test coro to pick up and handle. There is an exception in the 
case of a second coroutine whose response is to exit. See test case ObserverQueue.
