## Design Notes

 - A "coroutine" is officially a class or factory object bound to a coro body. 
   It creates an RAII object, so an instance must be created and should be in 
   a local scope (i.e. of the test case). Coroutines are implemented as lambdas
   because a lambda can be a C++20 coroutine, and we can capture the mock objects.
   This breeches S/S guidelines, but the UI is designed to make unsafe usafe difficult.

 - All cardinality functionality belongs interior to the coroutine. Therefore WillOnce(), Times()
   etc are not supported when routing calls to a coroutine. That's why the interior UI 
   should not say "EXPECT". Filtering (ultimately) can be done in interior or exterior,
   so we don't want to be too imperitive in the terminology here because the coro can just
   drop the call. Call it WATCH_CALL().
