### Can I use gMock in multi-threaded programs?

googletest was designed with thread-safety in mind. It uses synchronization
primitives from `google3` to be thread-safe. If you work in `google3`, you can
use gMock in multiple threads safely. If you work outside of `google3` and need
gMock to be thread-safe, please let us know.

For more details on how to use gMock with threads, read this
[recipe](#UsingThreads).
