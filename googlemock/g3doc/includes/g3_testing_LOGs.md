#### Testing LOG()s {#TestingLogs}

LOG()s are widely used in `google3` programs. They make it possible to diagnose
a server crash when you don't have the luxury of reproducing the bug. They are
also great as a [tool for refactoring](http://go/log-pin).

Often we need to test how a piece of code calls LOG()s. Traditionally, this has
been done using [golden files](http://go/log-pin), which is tedious to set up
and brittle (what if a library you depend on starts to generate its own logs?).
The [`ScopedMemoryLog`](http://go/gunit-faq-scoped-mock-log) class was created
to allow writing robust LOG tests, but using it beyond the most basic scenario
can be awkward.

With gMock we have a better solution. `testing/base/public/mock-log.h` defines a
mock log sink class `ScopedMockLog`. A `ScopedMockLog` object intercepts all
LOG()s (except `LOG(FATAL)`) while it is alive. This object has a mock method of
this signature:

```cpp
  void Log(LogSeverity severity, const string& path, const string& message);
```

This file comes with gUnit and gMock, so there is no need to add any dependency
to your `BUILD` rule in order to use it.

Here are some ideas on how to make use of it:

To test that the code generates exactly one warning message (and nothing else):

```cpp
using ::testing::_;
using ::testing::kDoNotCaptureLogsYet;
using ::testing::ScopedMockLog;
...
  ScopedMockLog log(kDoNotCaptureLogsYet);
  EXPECT_CALL(log, Log(WARNING, _, "Expected warning."));
  log.StartCapturingLogs();
  ... code that LOG()s ...
```

To test that a particular message is logged exactly once (but there can be other
log messages with different contents):

```cpp
using ::testing::_;
using ::testing::kDoNotCaptureLogsYet;
using ::testing::AnyNumber;
using ::testing::ScopedMockLog;
...
  ScopedMockLog log(kDoNotCaptureLogsYet);
  EXPECT_CALL(log, Log).Times(AnyNumber());
  EXPECT_CALL(log, Log(INFO, _, "Expected message"));
  log.StartCapturingLogs();
  ... code that LOG()s ...
```

To test that no `ERROR` is logged (but there can be other log messages with
different severities):

```cpp
using ::testing::_;
using ::testing::kDoNotCaptureLogsYet;
using ::testing::AnyNumber;
using ::testing::ScopedMockLog;
...
  ScopedMockLog log(kDoNotCaptureLogsYet);
  EXPECT_CALL(log, Log).Times(AnyNumber());
  EXPECT_CALL(log, Log(ERROR, _, _))
      .Times(0);
  log.StartCapturingLogs();
  ... code that LOG()s ...
```

To test that a particular message is logged at least once (and there can be
other log messages):

```cpp
using ::testing::_;
using ::testing::kDoNotCaptureLogsYet;
using ::testing::AnyNumber;
using ::testing::AtLeast;
using ::testing::ScopedMockLog;
...
  ScopedMockLog log(kDoNotCaptureLogsYet);
  EXPECT_CALL(log, Log).Times(AnyNumber());
  EXPECT_CALL(log, Log(INFO, _, "Expected message"))
      .Times(AtLeast(1));
  log.StartCapturingLogs();
  ... code that LOG()s ...
```

To test that three LOG()s occur sequentially:

```cpp
using ::testing::_;
using ::testing::kDoNotCaptureLogsYet;
using ::testing::InSequence;
using ::testing::ScopedMockLog;
...
  ScopedMockLog log(kDoNotCaptureLogsYet);
  {
     InSequence s;
     EXPECT_CALL(log, Log(INFO, _, "Log #1"));
     EXPECT_CALL(log, Log(WARNING, _, "Log #2"));
     EXPECT_CALL(log, Log(INFO, _, "Log #3"));
  }
  log.StartCapturingLogs();
  ... code that LOG()s ...
```

To test that the log message contains a certain sub-string:

```cpp
using ::testing::_;
using ::testing::kDoNotCaptureLogsYet;
using ::testing::HasSubstr;
using ::testing::ScopedMockLog;
...
  ScopedMockLog log(kDoNotCaptureLogsYet);
  EXPECT_CALL(log, Log(WARNING, _, HasSubstr("needle")));
  log.StartCapturingLogs();
  ... code that LOG()s ...
```

To test that a given module generates a specific log:

```cpp
using ::testing::kDoNotCaptureLogsYet;
using ::testing::ScopedMockLog;
...
  ScopedMockLog log(kDoNotCaptureLogsYet);
  EXPECT_CALL(log, Log(WARNING, "path/to/my_module.cc", "Expected warning."));
  log.StartCapturingLogs();
  ... code that LOG()s ...
```

To test that code doesn't log anything at all:

```cpp
using ::testing::_;
using ::testing::kDoNotCaptureLogsYet;
using ::testing::ScopedMockLog;
...
  ScopedMockLog log(kDoNotCaptureLogsYet);
  EXPECT_CALL(log, Log).Times(0);
  log.StartCapturingLogs();
  ... code that does not LOG() ...
```

**Warning:** For robust tests, either ignore unexpected logs (loose), or ignore
logs in other modules (tight), otherwise your test may break if their logging
changes.

```cpp
using ::testing::_;
using ::testing::AnyNumber;
using ::testing::kDoNotCaptureLogsYet;
using ::testing::Not;
using ::testing::ScopedMockLog;

// ...

// Simple robust setup, ignores unexpected logs.
  ScopedMockLog log(kDoNotCaptureLogsYet);
  EXPECT_CALL(log, Log).Times(AnyNumber());  // Ignore unexpected logs.
  EXPECT_CALL(log, Log(ERROR, "path/to/my_file.cc", _))
      .Times(3);  // Verifies logs from my_file.cc.
  log.StartCapturingLogs();
  // ... code that LOG()s ...

// ...

// Tighter alternative.
  ScopedMockLog log(kDoNotCaptureLogsYet);
  EXPECT_CALL(log, Log(_, Not("path/to/my_file.cc"), _))
      .Times(AnyNumber());  // Ignores other modules.
  EXPECT_CALL(log, Log(ERROR, "path/to/my_file.cc", _))
      .Times(3);  // Verifies logs from my_file.cc.
  log.StartCapturingLogs();
  // ... code that LOG()s ...
```

To test `LOG(DFATAL)`, use
[`EXPECT_DFATAL`](/third_party/googletest/googletest/g3doc/advanced#testing-death-in-debug-mode)
instead.
