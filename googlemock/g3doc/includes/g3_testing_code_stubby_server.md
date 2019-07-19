#### Testing Code that Uses a Stubby Server

(Contributed by JonWray on 2008/3/11; updated by ZhanyongWan later.)

I'm testing a C++ frontend that calls several different backends, but I'll just
include an example for one to keep this relatively short. This example is
mocking a `CacheServer` backend. An explanation follows the code.

```cpp
using ::testing::_;
using ::testing::BeDone;
using ::testing::EqualsProto;
using ::testing::RespondWith;

class MockCacheServer : public CacheServerRPC {
 public:
  MockCacheServer(HTTPServer *hs) {
    rpc2::EnableRPC2(this, rpc2::ServiceParameters());
    CacheServerRPC::ExportService(hs);
    ON_CALL(*this, Insert).WillByDefault(BeDone());
    ON_CALL(*this, Lookup).WillByDefault(BeDone());
  }

  MOCK_METHOD(void, Insert,
              (RPC*, const CacheInsertCommandProto*, CacheInsertResultsProto*,
               Closure*),
              (override));
  MOCK_METHOD(void, Lookup,
              (RPC*, const CacheLookupCommandProto*, CacheLookupResultsProto*,
               Closure*),
              (override));
};
...
  // This is in the test fixture.
  MockCacheServer cacheserver_;
...
    // Now the code that uses it:
    CacheLookupCommandProto command;
    // Init command
    CacheLookupResultsProto results;
    // Init results

    EXPECT_CALL(cacheserver_, Lookup(_, EqualsProto(command), _, _))
        .WillOnce(RespondWith(results));
```

In the success case, the command matches the `EXPECT_CALL`, so results is set
and the callback is called.

In the failure case, the command matches the default `ON_CALL`, the results are
not set, and the done closure is called (don't want the test to hang).

So it's a bit ugly, but given that I need to mock five backends, I think it's
better than doing this manually. The best part is the nicely formatted error
messages when the expected call is incorrect. Once all this scaffolding is in
place, it's easy to churn out test suites.

**Discussions:**

*   ZhanyongWan: `StubbySimulator` by Mike Bland might also be useful:
    google3/testing/lib/net/rpc/stubby_simulator.h.
*   JonWray: This is turning a mock into a fake, isn't it? All requests are
    accepted, and you can write logic to set the reply conditionally on the
    request. The interesting thing is the logic moves from the mock class into
    the test suite.
*   MikeBland: It's sort of a Frankenstein, but it works well for my purposes.
    It collaborates with a mock Stubby server, which sets the expectation and
    does the actual intercepting of the args, and then gives you the freedom to
    fiddle with the results while the RPC is conceptually "in flight". This is
    especially handy for "pausing" the RPC and having it return a state other
    than `RPC::OK`. This sort of approach to splitting RPC calls into separate
    objects from ControlFlows was first explored in
    [TotT Episode 46](http://tott/2007/06/episode-46-encapsulated-rpcs-or.html).
*   PiotrKaminski: The [Service Mocker](http://go/servicemocker) is a gMock-like
    framework that specializes in mocking Stubby services. It allows for small,
    in-process tests, doesn't require manually writing a service mock, and can
    deal with async clients, streaming and testing for cancellations.
