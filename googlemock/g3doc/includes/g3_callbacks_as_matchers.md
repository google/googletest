#### Using Callbacks as Matchers

Callbacks are widely used in `google3`. Conceptually, a `ResultCallback1<bool,
T>` is just a predicate on argument of type `T`. Naturally, we sometimes would
want to use such a callback as a matcher.

gMock gives you two function templates in namespace `testing` to turn callbacks
into matchers.

The first is `Truly(callback)`. It matches `argument` iff
`callback->Run(argument)` returns `true`.

The second is `AddressSatisfies(callback)`, which matches `argument` whenever
`callback->Run(&argument)` returns `true`.

The callbacks used in `Truly()` and `AddressSatisfies()` must be permanent (e.g.
those returned by `NewPermanentCallback()`), or you'll get a run-time error. The
matcher takes ownership of the callback, so you don't need to worry about
deleting it.

For examples, see
google3/testing/base/internal/gmock_utils/callback-matchers_test.cc.
