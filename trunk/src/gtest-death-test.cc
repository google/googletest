// Copyright 2005, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Author: wan@google.com (Zhanyong Wan), vladl@losev.com (Vlad Losev)
//
// This file implements death tests.

#include <gtest/gtest-death-test.h>
#include <gtest/internal/gtest-port.h>

#if GTEST_HAS_DEATH_TEST
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdarg.h>

#if GTEST_OS_WINDOWS
#include <windows.h>
#else
#include <sys/mman.h>
#endif  // GTEST_OS_WINDOWS

#endif  // GTEST_HAS_DEATH_TEST

#include <gtest/gtest-message.h>
#include <gtest/internal/gtest-string.h>

// Indicates that this translation unit is part of Google Test's
// implementation.  It must come before gtest-internal-inl.h is
// included, or there will be a compiler error.  This trick is to
// prevent a user from accidentally including gtest-internal-inl.h in
// his code.
#define GTEST_IMPLEMENTATION_ 1
#include "src/gtest-internal-inl.h"
#undef GTEST_IMPLEMENTATION_

namespace testing {

// Constants.

// The default death test style.
static const char kDefaultDeathTestStyle[] = "fast";

GTEST_DEFINE_string_(
    death_test_style,
    internal::StringFromGTestEnv("death_test_style", kDefaultDeathTestStyle),
    "Indicates how to run a death test in a forked child process: "
    "\"threadsafe\" (child process re-executes the test binary "
    "from the beginning, running only the specific death test) or "
    "\"fast\" (child process runs the death test immediately "
    "after forking).");

GTEST_DEFINE_bool_(
    death_test_use_fork,
    internal::BoolFromGTestEnv("death_test_use_fork", false),
    "Instructs to use fork()/_exit() instead of clone() in death tests. "
    "Useful when running under valgrind or similar tools if those "
    "do not support clone(). Valgrind 3.3.1 will just fail if "
    "it sees an unsupported combination of clone() flags. "
    "It is not recommended to use this flag w/o valgrind though it will "
    "work in 99% of the cases. Once valgrind is fixed, this flag will "
    "most likely be removed.");

namespace internal {
GTEST_DEFINE_string_(
    internal_run_death_test, "",
    "Indicates the file, line number, temporal index of "
    "the single death test to run, and a file descriptor to "
    "which a success code may be sent, all separated by "
    "colons.  This flag is specified if and only if the current "
    "process is a sub-process launched for running a thread-safe "
    "death test.  FOR INTERNAL USE ONLY.");
}  // namespace internal

#if GTEST_HAS_DEATH_TEST

// ExitedWithCode constructor.
ExitedWithCode::ExitedWithCode(int exit_code) : exit_code_(exit_code) {
}

// ExitedWithCode function-call operator.
bool ExitedWithCode::operator()(int exit_status) const {
#if GTEST_OS_WINDOWS
  return exit_status == exit_code_;
#else
  return WIFEXITED(exit_status) && WEXITSTATUS(exit_status) == exit_code_;
#endif  // GTEST_OS_WINDOWS
}

#if !GTEST_OS_WINDOWS
// KilledBySignal constructor.
KilledBySignal::KilledBySignal(int signum) : signum_(signum) {
}

// KilledBySignal function-call operator.
bool KilledBySignal::operator()(int exit_status) const {
  return WIFSIGNALED(exit_status) && WTERMSIG(exit_status) == signum_;
}
#endif  // !GTEST_OS_WINDOWS

namespace internal {

// Utilities needed for death tests.

// Generates a textual description of a given exit code, in the format
// specified by wait(2).
static String ExitSummary(int exit_code) {
  Message m;
#if GTEST_OS_WINDOWS
  m << "Exited with exit status " << exit_code;
#else
  if (WIFEXITED(exit_code)) {
    m << "Exited with exit status " << WEXITSTATUS(exit_code);
  } else if (WIFSIGNALED(exit_code)) {
    m << "Terminated by signal " << WTERMSIG(exit_code);
  }
#ifdef WCOREDUMP
  if (WCOREDUMP(exit_code)) {
    m << " (core dumped)";
  }
#endif
#endif  // GTEST_OS_WINDOWS
  return m.GetString();
}

// Returns true if exit_status describes a process that was terminated
// by a signal, or exited normally with a nonzero exit code.
bool ExitedUnsuccessfully(int exit_status) {
  return !ExitedWithCode(0)(exit_status);
}

#if !GTEST_OS_WINDOWS
// Generates a textual failure message when a death test finds more than
// one thread running, or cannot determine the number of threads, prior
// to executing the given statement.  It is the responsibility of the
// caller not to pass a thread_count of 1.
static String DeathTestThreadWarning(size_t thread_count) {
  Message msg;
  msg << "Death tests use fork(), which is unsafe particularly"
      << " in a threaded context. For this test, " << GTEST_NAME_ << " ";
  if (thread_count == 0)
    msg << "couldn't detect the number of threads.";
  else
    msg << "detected " << thread_count << " threads.";
  return msg.GetString();
}
#endif  // !GTEST_OS_WINDOWS

// Flag characters for reporting a death test that did not die.
static const char kDeathTestLived = 'L';
static const char kDeathTestReturned = 'R';
static const char kDeathTestInternalError = 'I';

// An enumeration describing all of the possible ways that a death test
// can conclude.  DIED means that the process died while executing the
// test code; LIVED means that process lived beyond the end of the test
// code; and RETURNED means that the test statement attempted a "return,"
// which is not allowed.  IN_PROGRESS means the test has not yet
// concluded.
enum DeathTestOutcome { IN_PROGRESS, DIED, LIVED, RETURNED };

// Routine for aborting the program which is safe to call from an
// exec-style death test child process, in which case the error
// message is propagated back to the parent process.  Otherwise, the
// message is simply printed to stderr.  In either case, the program
// then exits with status 1.
void DeathTestAbort(const String& message) {
  // On a POSIX system, this function may be called from a threadsafe-style
  // death test child process, which operates on a very small stack.  Use
  // the heap for any additional non-minuscule memory requirements.
  const InternalRunDeathTestFlag* const flag =
      GetUnitTestImpl()->internal_run_death_test_flag();
  if (flag != NULL) {
// Suppress MSVC complaints about POSIX functions.
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4996)
#endif  // _MSC_VER
    FILE* parent = fdopen(flag->status_fd(), "w");
#ifdef _MSC_VER
#pragma warning(pop)
#endif  // _MSC_VER
    fputc(kDeathTestInternalError, parent);
    fprintf(parent, "%s", message.c_str());
    fflush(parent);
    _exit(1);
  } else {
    fprintf(stderr, "%s", message.c_str());
    fflush(stderr);
    abort();
  }
}

// A replacement for CHECK that calls DeathTestAbort if the assertion
// fails.
#define GTEST_DEATH_TEST_CHECK_(expression) \
  do { \
    if (!(expression)) { \
      DeathTestAbort(::testing::internal::String::Format(\
          "CHECK failed: File %s, line %d: %s", \
          __FILE__, __LINE__, #expression)); \
    } \
  } while (0)

// This macro is similar to GTEST_DEATH_TEST_CHECK_, but it is meant for
// evaluating any system call that fulfills two conditions: it must return
// -1 on failure, and set errno to EINTR when it is interrupted and
// should be tried again.  The macro expands to a loop that repeatedly
// evaluates the expression as long as it evaluates to -1 and sets
// errno to EINTR.  If the expression evaluates to -1 but errno is
// something other than EINTR, DeathTestAbort is called.
#define GTEST_DEATH_TEST_CHECK_SYSCALL_(expression) \
  do { \
    int gtest_retval; \
    do { \
      gtest_retval = (expression); \
    } while (gtest_retval == -1 && errno == EINTR); \
    if (gtest_retval == -1) { \
      DeathTestAbort(::testing::internal::String::Format(\
          "CHECK failed: File %s, line %d: %s != -1", \
          __FILE__, __LINE__, #expression)); \
    } \
  } while (0)

// Returns the message describing the last system error, regardless of the
// platform.
String GetLastSystemErrorMessage() {
#if GTEST_OS_WINDOWS
    const DWORD error_num = ::GetLastError();

    if (error_num == NULL)
      return String("");

    char* message_ptr;

    ::FormatMessageA(
        // The caller does not provide a buffer. The function will allocate one.
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
            // The function must look up an error message in its system error
            // message table.
            FORMAT_MESSAGE_FROM_SYSTEM |
            // Do not expand insert sequences in the message definition.
            FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,  // Message source. Ignored in this call.
        error_num,
        0x0,  // Use system-default language.
        reinterpret_cast<LPSTR>(&message_ptr),
        0,  // Buffer size. Ignored in this call.
        NULL);  // Message arguments. Ignored in this call.

    const String message = message_ptr;
    ::LocalFree(message_ptr);
    return message;
#else
    return errno == 0 ? String("") : String(strerror(errno));
#endif  // GTEST_OS_WINDOWS
}

// TODO(vladl@google.com): Move the definition of FailFromInternalError
// here.
#if GTEST_OS_WINDOWS
static void FailFromInternalError(HANDLE handle);
#else
static void FailFromInternalError(int fd);
#endif  // GTEST_OS_WINDOWS

// Death test constructor.  Increments the running death test count
// for the current test.
DeathTest::DeathTest() {
  TestInfo* const info = GetUnitTestImpl()->current_test_info();
  if (info == NULL) {
    DeathTestAbort("Cannot run a death test outside of a TEST or "
                   "TEST_F construct");
  }
}

// Creates and returns a death test by dispatching to the current
// death test factory.
bool DeathTest::Create(const char* statement, const RE* regex,
                       const char* file, int line, DeathTest** test) {
  return GetUnitTestImpl()->death_test_factory()->Create(
      statement, regex, file, line, test);
}

const char* DeathTest::LastMessage() {
  return last_death_test_message_.c_str();
}

void DeathTest::set_last_death_test_message(const String& message) {
  last_death_test_message_ = message;
}

String DeathTest::last_death_test_message_;

// Provides cross platform implementation for some death functionality.
// TODO(vladl@google.com): Merge this class with DeathTest in
// gtest-death-test-internal.h.
class DeathTestImpl : public DeathTest {
 protected:
  DeathTestImpl(const char* statement, const RE* regex)
      : statement_(statement),
        regex_(regex),
        spawned_(false),
        status_(-1),
        outcome_(IN_PROGRESS) {}

  virtual bool Passed(bool status_ok);

  const char* statement() const { return statement_; }
  const RE* regex() const { return regex_; }
  bool spawned() const { return spawned_; }
  void set_spawned(bool spawned) { spawned_ = spawned; }
  int status() const { return status_; }
  void set_status(int status) { status_ = status; }
  DeathTestOutcome outcome() const { return outcome_; }
  void set_outcome(DeathTestOutcome outcome) { outcome_ = outcome; }

 private:
  // The textual content of the code this object is testing.  This class
  // doesn't own this string and should not attempt to delete it.
  const char* const statement_;
  // The regular expression which test output must match.  DeathTestImpl
  // doesn't own this object and should not attempt to delete it.
  const RE* const regex_;
  // True if the death test child process has been successfully spawned.
  bool spawned_;
  // The exit status of the child process.
  int status_;
  // How the death test concluded.
  DeathTestOutcome outcome_;
};

// TODO(vladl@google.com): Move definition of DeathTestImpl::Passed() here.

#if GTEST_OS_WINDOWS
// WindowsDeathTest implements death tests on Windows. Due to the
// specifics of starting new processes on Windows, death tests there are
// always threadsafe, and Google Test considers the
// --gtest_death_test_style=fast setting to be equivalent to
// --gtest_death_test_style=threadsafe there.
//
// A few implementation notes:  Like the Linux version, the Windows
// implementation uses pipes for child-to-parent communication. But due to
// the specifics of pipes on Windows, some extra steps are required:
//
// 1. The parent creates a communication pipe and stores handles to both
//    ends of it.
// 2. The parent starts the child and provides it with the information
//    necessary to acquire the handle to the write end of the pipe.
// 3. The child acquires the write end of the pipe and signals the parent
//    using a Windows event.
// 4. Now the parent can release the write end of the pipe on its side. If
//    this is done before step 3, the object's reference count goes down to
//    0 and it is destroyed, preventing the child from acquiring it. The
//    parent now has to release it, or read operations on the read end of
//    the pipe will not return when the child terminates.
// 5. The parent reads child's output through the pipe (outcome code and
//    any possible error messages) from the pipe, and its stderr and then
//    determines whether to fail the test.
//
// Note: to distinguish Win32 API calls from the local method and function
// calls, the former are explicitly resolved in the global namespace.
//
class WindowsDeathTest : public DeathTestImpl {
 public:
  WindowsDeathTest(const char* statement,
                   const RE* regex,
                   const char* file,
                   int line)
      : DeathTestImpl(statement, regex), file_(file), line_(line) {}

  // All of these virtual functions are inherited from DeathTest.
  virtual int Wait();
  virtual void Abort(AbortReason reason);
  virtual TestRole AssumeRole();

 private:
  // The name of the file in which the death test is located.
  const char* const file_;
  // The line number on which the death test is located.
  const int line_;
  // Handle to the read end of the pipe to the child process.
  // The child keeps its write end of the pipe in the status_handle_
  // field of its InternalRunDeathTestFlag class.
  AutoHandle read_handle_;
  // Handle to the write end of the pipe to the child process.
  AutoHandle write_handle_;
  // Child process handle.
  AutoHandle child_handle_;
  // Event the child process uses to signal the parent that it has
  // acquired the handle to the write end of the pipe. After seeing this
  // event the parent can release its own handles to make sure its
  // ReadFile() calls return when the child terminates.
  AutoHandle event_handle_;
};

// Waits for the child in a death test to exit, returning its exit
// status, or 0 if no child process exists.  As a side effect, sets the
// outcome data member.
// TODO(vladl@google.com): Outcome classification logic is common with
//                         ForkingDeathTes::Wait(). Refactor it into a
//                         common function.
int WindowsDeathTest::Wait() {
  if (!spawned())
    return 0;

  // Wait until the child either signals that it has acquired the write end
  // of the pipe or it dies.
  const HANDLE wait_handles[2] = { child_handle_.Get(), event_handle_.Get() };
  switch (::WaitForMultipleObjects(2,
                                   wait_handles,
                                   FALSE,  // Waits for any of the handles.
                                   INFINITE)) {
    case WAIT_OBJECT_0:
    case WAIT_OBJECT_0 + 1:
      break;
    default:
      GTEST_DEATH_TEST_CHECK_(false);  // Should not get here.
  }

  // The child has acquired the write end of the pipe or exited.
  // We release the handle on our side and continue.
  write_handle_.Reset();
  event_handle_.Reset();

  // ReadFile() blocks until data is available (signifying the
  // failure of the death test) or until the pipe is closed (signifying
  // its success), so it's okay to call this in the parent before or
  // after the child process has exited.
  char flag;
  DWORD bytes_read;
  GTEST_DEATH_TEST_CHECK_(::ReadFile(read_handle_.Get(),
                                     &flag,
                                     1,
                                     &bytes_read,
                                     NULL) ||
                          ::GetLastError() == ERROR_BROKEN_PIPE);

  if (bytes_read == 0) {
    set_outcome(DIED);
  } else if (bytes_read == 1) {
    switch (flag) {
      case kDeathTestReturned:
        set_outcome(RETURNED);
        break;
      case kDeathTestLived:
        set_outcome(LIVED);
        break;
      case kDeathTestInternalError:
        FailFromInternalError(read_handle_.Get());  // Does not return.
        break;
      default:
        GTEST_LOG_(FATAL,
                   Message() << "Death test child process reported "
                   << " unexpected status byte ("
                   << static_cast<unsigned int>(flag) << ")");
    }
  } else {
    GTEST_LOG_(FATAL,
               Message() << "Read from death test child process failed: "
                         << GetLastSystemErrorMessage());
  }
  read_handle_.Reset();  // Done with reading.

  // Waits for the child process to exit if it haven't already. This
  // returns immediately if the child has already exited, regardless of
  // whether previous calls to WaitForMultipleObjects synchronized on this
  // handle or not.
  GTEST_DEATH_TEST_CHECK_(
      WAIT_OBJECT_0 == ::WaitForSingleObject(child_handle_.Get(),
                                             INFINITE));
  DWORD status;
  GTEST_DEATH_TEST_CHECK_(::GetExitCodeProcess(child_handle_.Get(),
                                               &status));
  child_handle_.Reset();
  set_status(static_cast<int>(status));
  return this->status();
}

// TODO(vladl@google.com): define a cross-platform way to write to
// status_fd to be used both here and in ForkingDeathTest::Abort().
//
// Signals that the death test did not die as expected. This is called
// from the child process only.
void WindowsDeathTest::Abort(AbortReason reason) {
  const InternalRunDeathTestFlag* const internal_flag =
      GetUnitTestImpl()->internal_run_death_test_flag();
  // The parent process considers the death test to be a failure if
  // it finds any data in our pipe.  So, here we write a single flag byte
  // to the pipe, then exit.
  const char status_ch =
      reason == TEST_DID_NOT_DIE ? kDeathTestLived : kDeathTestReturned;

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4996)
#endif  // _MSC_VER
  GTEST_DEATH_TEST_CHECK_SYSCALL_(write(internal_flag->status_fd(),
                                        &status_ch, 1));
#ifdef _MSC_VER
#pragma warning(pop)
#endif  // _MSC_VER

  // The write handle will be closed when the child terminates in _exit().
  _exit(1);  // Exits w/o any normal exit hooks (we were supposed to crash)
}

// The AssumeRole process for a Windows death test.  It creates a child
// process with the same executable as the current process to run the
// death test.  The child process is given the --gtest_filter and
// --gtest_internal_run_death_test flags such that it knows to run the
// current death test only.
DeathTest::TestRole WindowsDeathTest::AssumeRole() {
  const UnitTestImpl* const impl = GetUnitTestImpl();
  const InternalRunDeathTestFlag* const flag =
      impl->internal_run_death_test_flag();
  const TestInfo* const info = impl->current_test_info();
  const int death_test_index = info->result()->death_test_count();

  if (flag != NULL) {
    // ParseInternalRunDeathTestFlag() has performed all the necessary
    // processing.
    return EXECUTE_TEST;
  }

  // WindowsDeathTest uses an anonymous pipe to communicate results of
  // a death test.
  SECURITY_ATTRIBUTES handles_are_inheritable = {
    sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
  HANDLE read_handle, write_handle;
  GTEST_DEATH_TEST_CHECK_(::CreatePipe(&read_handle, &write_handle,
                                       &handles_are_inheritable,
                                       0));  // Default buffer size.
  read_handle_.Reset(read_handle);
  write_handle_.Reset(write_handle);
  event_handle_.Reset(::CreateEvent(
      &handles_are_inheritable,
      TRUE,    // The event will automatically reset to non-signaled state.
      FALSE,   // The initial state is non-signalled.
      NULL));  // The even is unnamed.
  GTEST_DEATH_TEST_CHECK_(event_handle_.Get() != NULL);
  const String filter_flag = String::Format("--%s%s=%s.%s",
                                            GTEST_FLAG_PREFIX_, kFilterFlag,
                                            info->test_case_name(),
                                            info->name());
  const String internal_flag = String::Format(
    "--%s%s=%s|%d|%d|%u|%Iu|%Iu",
      GTEST_FLAG_PREFIX_,
      kInternalRunDeathTestFlag,
      file_, line_,
      death_test_index,
      static_cast<unsigned int>(::GetCurrentProcessId()),
      // size_t has the same with as pointers on both 32-bit and 64-bit
      // Windows platforms.
      // See http://msdn.microsoft.com/en-us/library/tcxf1dw6.aspx.
      reinterpret_cast<size_t>(write_handle),
      reinterpret_cast<size_t>(event_handle_.Get()));

  char executable_path[_MAX_PATH + 1];  // NOLINT
  GTEST_DEATH_TEST_CHECK_(
      _MAX_PATH + 1 != ::GetModuleFileNameA(NULL,
                                            executable_path,
                                            _MAX_PATH));

  String command_line = String::Format("%s %s \"%s\"",
                                       ::GetCommandLineA(),
                                       filter_flag.c_str(),
                                       internal_flag.c_str());

  DeathTest::set_last_death_test_message("");

  CaptureStderr();
  // Flush the log buffers since the log streams are shared with the child.
  FlushInfoLog();

  // The child process will share the standard handles with the parent.
  STARTUPINFOA startup_info;
  memset(&startup_info, 0, sizeof(STARTUPINFO));
  startup_info.dwFlags = STARTF_USESTDHANDLES;
  startup_info.hStdInput = ::GetStdHandle(STD_INPUT_HANDLE);
  startup_info.hStdOutput = ::GetStdHandle(STD_OUTPUT_HANDLE);
  startup_info.hStdError = ::GetStdHandle(STD_ERROR_HANDLE);

  PROCESS_INFORMATION process_info;
  GTEST_DEATH_TEST_CHECK_(::CreateProcessA(
      executable_path,
      const_cast<char*>(command_line.c_str()),
      NULL,   // Retuned process handle is not inheritable.
      NULL,   // Retuned thread handle is not inheritable.
      TRUE,   // Child inherits all inheritable handles (for write_handle_).
      0x0,    // Default creation flags.
      NULL,   // Inherit the parent's environment.
      UnitTest::GetInstance()->original_working_dir(),
      &startup_info,
      &process_info));
  child_handle_.Reset(process_info.hProcess);
  ::CloseHandle(process_info.hThread);
  set_spawned(true);
  return OVERSEE_TEST;
}
#else  // We are not on Windows.

// ForkingDeathTest provides implementations for most of the abstract
// methods of the DeathTest interface.  Only the AssumeRole method is
// left undefined.
class ForkingDeathTest : public DeathTestImpl {
 public:
  ForkingDeathTest(const char* statement, const RE* regex);

  // All of these virtual functions are inherited from DeathTest.
  virtual int Wait();
  virtual void Abort(AbortReason reason);

 protected:
  void set_child_pid(pid_t child_pid) { child_pid_ = child_pid; }
  void set_read_fd(int fd) { read_fd_ = fd; }
  void set_write_fd(int fd) { write_fd_ = fd; }

 private:
  // PID of child process during death test; 0 in the child process itself.
  pid_t child_pid_;
  // File descriptors for communicating the death test's status byte.
  int read_fd_;   // Always -1 in the child process.
  int write_fd_;  // Always -1 in the parent process.
};

// Constructs a ForkingDeathTest.
ForkingDeathTest::ForkingDeathTest(const char* statement, const RE* regex)
    : DeathTestImpl(statement, regex),
      child_pid_(-1),
      read_fd_(-1),
      write_fd_(-1) {
}
#endif  // GTEST_OS_WINDOWS

// This is called from a death test parent process to read a failure
// message from the death test child process and log it with the FATAL
// severity. On Windows, the message is read from a pipe handle. On other
// platforms, it is read from a file descriptor.
// TODO(vladl@google.com): Re-factor the code to merge common parts after
// the reading code is abstracted.
#if GTEST_OS_WINDOWS
static void FailFromInternalError(HANDLE handle) {
  Message error;
  char buffer[256];

  bool read_succeeded = true;
  DWORD bytes_read;
  do {
    // ERROR_BROKEN_PIPE arises when the other end of the pipe has been
    // closed. This is a normal condition for us.
    bytes_read = 0;
    read_succeeded = ::ReadFile(handle,
                                buffer,
                                sizeof(buffer) - 1,
                                &bytes_read,
                                NULL) || ::GetLastError() == ERROR_BROKEN_PIPE;
    buffer[bytes_read] = 0;
    error << buffer;
  } while (read_succeeded && bytes_read > 0);

  if (read_succeeded) {
    GTEST_LOG_(FATAL, error);
  } else {
    const DWORD last_error = ::GetLastError();
    const String message = GetLastSystemErrorMessage();
    GTEST_LOG_(FATAL,
               Message() << "Error while reading death test internal: "
               << message << " [" << last_error << "]");
  }
}
#else
static void FailFromInternalError(int fd) {
  Message error;
  char buffer[256];
  ssize_t num_read;

  do {
    while ((num_read = read(fd, buffer, 255)) > 0) {
      buffer[num_read] = '\0';
      error << buffer;
    }
  } while (num_read == -1 && errno == EINTR);

  if (num_read == 0) {
    GTEST_LOG_(FATAL, error);
  } else {
    const int last_error = errno;
    const String message = GetLastSystemErrorMessage();
    GTEST_LOG_(FATAL,
               Message() << "Error while reading death test internal: "
               << message << " [" << last_error << "]");
  }
}
#endif  // GTEST_OS_WINDOWS

#if !GTEST_OS_WINDOWS
// Waits for the child in a death test to exit, returning its exit
// status, or 0 if no child process exists.  As a side effect, sets the
// outcome data member.
int ForkingDeathTest::Wait() {
  if (!spawned())
    return 0;

  // The read() here blocks until data is available (signifying the
  // failure of the death test) or until the pipe is closed (signifying
  // its success), so it's okay to call this in the parent before
  // the child process has exited.
  char flag;
  ssize_t bytes_read;

  do {
    bytes_read = read(read_fd_, &flag, 1);
  } while (bytes_read == -1 && errno == EINTR);

  if (bytes_read == 0) {
    set_outcome(DIED);
  } else if (bytes_read == 1) {
    switch (flag) {
      case kDeathTestReturned:
        set_outcome(RETURNED);
        break;
      case kDeathTestLived:
        set_outcome(LIVED);
        break;
      case kDeathTestInternalError:
        FailFromInternalError(read_fd_);  // Does not return.
        break;
      default:
        GTEST_LOG_(FATAL,
                   Message() << "Death test child process reported unexpected "
                   << "status byte (" << static_cast<unsigned int>(flag)
                   << ")");
    }
  } else {
    const String error_message = GetLastSystemErrorMessage();
    GTEST_LOG_(FATAL,
               Message() << "Read from death test child process failed: "
                         << error_message);
  }

  GTEST_DEATH_TEST_CHECK_SYSCALL_(close(read_fd_));
  int status;
  GTEST_DEATH_TEST_CHECK_SYSCALL_(waitpid(child_pid_, &status, 0));
  set_status(status);
  return status;
}
#endif  // !GTEST_OS_WINDOWS

// Assesses the success or failure of a death test, using both private
// members which have previously been set, and one argument:
//
// Private data members:
//   outcome:  An enumeration describing how the death test
//             concluded: DIED, LIVED, or RETURNED.  The death test fails
//             in the latter two cases.
//   status:   The exit status of the child process. On *nix, it is in the
//             in the format specified by wait(2). On Windows, this is the
//             value supplied to the ExitProcess() API or a numeric code
//             of the exception that terminated the program.
//   regex:    A regular expression object to be applied to
//             the test's captured standard error output; the death test
//             fails if it does not match.
//
// Argument:
//   status_ok: true if exit_status is acceptable in the context of
//              this particular death test, which fails if it is false
//
// Returns true iff all of the above conditions are met.  Otherwise, the
// first failing condition, in the order given above, is the one that is
// reported. Also sets the last death test message string.
bool DeathTestImpl::Passed(bool status_ok) {
  if (!spawned())
    return false;

#if GTEST_HAS_GLOBAL_STRING
  const ::string error_message = GetCapturedStderr();
#else
  const ::std::string error_message = GetCapturedStderr();
#endif  // GTEST_HAS_GLOBAL_STRING

  bool success = false;
  Message buffer;

  buffer << "Death test: " << statement() << "\n";
  switch (outcome()) {
    case LIVED:
      buffer << "    Result: failed to die.\n"
             << " Error msg: " << error_message;
      break;
    case RETURNED:
      buffer << "    Result: illegal return in test statement.\n"
             << " Error msg: " << error_message;
      break;
    case DIED:
      if (status_ok) {
        if (RE::PartialMatch(error_message, *regex())) {
          success = true;
        } else {
          buffer << "    Result: died but not with expected error.\n"
                 << "  Expected: " << regex()->pattern() << "\n"
                 << "Actual msg: " << error_message;
        }
      } else {
        buffer << "    Result: died but not with expected exit code:\n"
               << "            " << ExitSummary(status()) << "\n";
      }
      break;
    case IN_PROGRESS:
    default:
      GTEST_LOG_(FATAL,
                 "DeathTest::Passed somehow called before conclusion of test");
  }

  DeathTest::set_last_death_test_message(buffer.GetString());
  return success;
}

#if !GTEST_OS_WINDOWS
// Signals that the death test code which should have exited, didn't.
// Should be called only in a death test child process.
// Writes a status byte to the child's status file descriptor, then
// calls _exit(1).
void ForkingDeathTest::Abort(AbortReason reason) {
  // The parent process considers the death test to be a failure if
  // it finds any data in our pipe.  So, here we write a single flag byte
  // to the pipe, then exit.
  const char flag =
      reason == TEST_DID_NOT_DIE ? kDeathTestLived : kDeathTestReturned;
  GTEST_DEATH_TEST_CHECK_SYSCALL_(write(write_fd_, &flag, 1));
  GTEST_DEATH_TEST_CHECK_SYSCALL_(close(write_fd_));
  _exit(1);  // Exits w/o any normal exit hooks (we were supposed to crash)
}

// A concrete death test class that forks, then immediately runs the test
// in the child process.
class NoExecDeathTest : public ForkingDeathTest {
 public:
  NoExecDeathTest(const char* statement, const RE* regex) :
      ForkingDeathTest(statement, regex) { }
  virtual TestRole AssumeRole();
};

// The AssumeRole process for a fork-and-run death test.  It implements a
// straightforward fork, with a simple pipe to transmit the status byte.
DeathTest::TestRole NoExecDeathTest::AssumeRole() {
  const size_t thread_count = GetThreadCount();
  if (thread_count != 1) {
    GTEST_LOG_(WARNING, DeathTestThreadWarning(thread_count));
  }

  int pipe_fd[2];
  GTEST_DEATH_TEST_CHECK_(pipe(pipe_fd) != -1);

  DeathTest::set_last_death_test_message("");
  CaptureStderr();
  // When we fork the process below, the log file buffers are copied, but the
  // file descriptors are shared.  We flush all log files here so that closing
  // the file descriptors in the child process doesn't throw off the
  // synchronization between descriptors and buffers in the parent process.
  // This is as close to the fork as possible to avoid a race condition in case
  // there are multiple threads running before the death test, and another
  // thread writes to the log file.
  FlushInfoLog();

  const pid_t child_pid = fork();
  GTEST_DEATH_TEST_CHECK_(child_pid != -1);
  set_child_pid(child_pid);
  if (child_pid == 0) {
    GTEST_DEATH_TEST_CHECK_SYSCALL_(close(pipe_fd[0]));
    set_write_fd(pipe_fd[1]);
    // Redirects all logging to stderr in the child process to prevent
    // concurrent writes to the log files.  We capture stderr in the parent
    // process and append the child process' output to a log.
    LogToStderr();
    return EXECUTE_TEST;
  } else {
    GTEST_DEATH_TEST_CHECK_SYSCALL_(close(pipe_fd[1]));
    set_read_fd(pipe_fd[0]);
    set_spawned(true);
    return OVERSEE_TEST;
  }
}

// A concrete death test class that forks and re-executes the main
// program from the beginning, with command-line flags set that cause
// only this specific death test to be run.
class ExecDeathTest : public ForkingDeathTest {
 public:
  ExecDeathTest(const char* statement, const RE* regex,
                const char* file, int line) :
      ForkingDeathTest(statement, regex), file_(file), line_(line) { }
  virtual TestRole AssumeRole();
 private:
  // The name of the file in which the death test is located.
  const char* const file_;
  // The line number on which the death test is located.
  const int line_;
};

// Utility class for accumulating command-line arguments.
class Arguments {
 public:
  Arguments() {
    args_.push_back(NULL);
  }

  ~Arguments() {
    for (std::vector<char*>::iterator i = args_.begin(); i != args_.end();
         ++i) {
      free(*i);
    }
  }
  void AddArgument(const char* argument) {
    args_.insert(args_.end() - 1, strdup(argument));
  }

  template <typename Str>
  void AddArguments(const ::std::vector<Str>& arguments) {
    for (typename ::std::vector<Str>::const_iterator i = arguments.begin();
         i != arguments.end();
         ++i) {
      args_.insert(args_.end() - 1, strdup(i->c_str()));
    }
  }
  char* const* Argv() {
    return &args_[0];
  }
 private:
  std::vector<char*> args_;
};

// A struct that encompasses the arguments to the child process of a
// threadsafe-style death test process.
struct ExecDeathTestArgs {
  char* const* argv;  // Command-line arguments for the child's call to exec
  int close_fd;       // File descriptor to close; the read end of a pipe
};

// The main function for a threadsafe-style death test child process.
// This function is called in a clone()-ed process and thus must avoid
// any potentially unsafe operations like malloc or libc functions.
static int ExecDeathTestChildMain(void* child_arg) {
  ExecDeathTestArgs* const args = static_cast<ExecDeathTestArgs*>(child_arg);
  GTEST_DEATH_TEST_CHECK_SYSCALL_(close(args->close_fd));

  // We need to execute the test program in the same environment where
  // it was originally invoked.  Therefore we change to the original
  // working directory first.
  const char* const original_dir =
      UnitTest::GetInstance()->original_working_dir();
  // We can safely call chdir() as it's a direct system call.
  if (chdir(original_dir) != 0) {
    DeathTestAbort(String::Format("chdir(\"%s\") failed: %s",
                                  original_dir,
                                  GetLastSystemErrorMessage().c_str()));
    return EXIT_FAILURE;
  }

  // We can safely call execve() as it's a direct system call.  We
  // cannot use execvp() as it's a libc function and thus potentially
  // unsafe.  Since execve() doesn't search the PATH, the user must
  // invoke the test program via a valid path that contains at least
  // one path separator.
  execve(args->argv[0], args->argv, environ);
  DeathTestAbort(String::Format("execve(%s, ...) in %s failed: %s",
                                args->argv[0],
                                original_dir,
                                GetLastSystemErrorMessage().c_str()));
  return EXIT_FAILURE;
}

// Two utility routines that together determine the direction the stack
// grows.
// This could be accomplished more elegantly by a single recursive
// function, but we want to guard against the unlikely possibility of
// a smart compiler optimizing the recursion away.
static bool StackLowerThanAddress(const void* ptr) {
  int dummy;
  return &dummy < ptr;
}

static bool StackGrowsDown() {
  int dummy;
  return StackLowerThanAddress(&dummy);
}

// A threadsafe implementation of fork(2) for threadsafe-style death tests
// that uses clone(2).  It dies with an error message if anything goes
// wrong.
static pid_t ExecDeathTestFork(char* const* argv, int close_fd) {
  static const bool stack_grows_down = StackGrowsDown();
  const size_t stack_size = getpagesize();
  void* const stack = mmap(NULL, stack_size, PROT_READ | PROT_WRITE,
                           MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  GTEST_DEATH_TEST_CHECK_(stack != MAP_FAILED);
  void* const stack_top =
      static_cast<char*>(stack) + (stack_grows_down ? stack_size : 0);
  ExecDeathTestArgs args = { argv, close_fd };
  pid_t child_pid;
  if (GTEST_FLAG(death_test_use_fork)) {
    // Valgrind-friendly version. As of valgrind 3.3.1 the clone() call below
    // is not supported (valgrind will fail with an error message).
    if ((child_pid = fork()) == 0) {
      ExecDeathTestChildMain(&args);
      _exit(0);
    }
  } else {
    child_pid = clone(&ExecDeathTestChildMain, stack_top,
                      SIGCHLD, &args);
  }
  GTEST_DEATH_TEST_CHECK_(child_pid != -1);
  GTEST_DEATH_TEST_CHECK_(munmap(stack, stack_size) != -1);
  return child_pid;
}

// The AssumeRole process for a fork-and-exec death test.  It re-executes the
// main program from the beginning, setting the --gtest_filter
// and --gtest_internal_run_death_test flags to cause only the current
// death test to be re-run.
DeathTest::TestRole ExecDeathTest::AssumeRole() {
  const UnitTestImpl* const impl = GetUnitTestImpl();
  const InternalRunDeathTestFlag* const flag =
      impl->internal_run_death_test_flag();
  const TestInfo* const info = impl->current_test_info();
  const int death_test_index = info->result()->death_test_count();

  if (flag != NULL) {
    set_write_fd(flag->status_fd());
    return EXECUTE_TEST;
  }

  int pipe_fd[2];
  GTEST_DEATH_TEST_CHECK_(pipe(pipe_fd) != -1);
  // Clear the close-on-exec flag on the write end of the pipe, lest
  // it be closed when the child process does an exec:
  GTEST_DEATH_TEST_CHECK_(fcntl(pipe_fd[1], F_SETFD, 0) != -1);

  const String filter_flag =
      String::Format("--%s%s=%s.%s",
                     GTEST_FLAG_PREFIX_, kFilterFlag,
                     info->test_case_name(), info->name());
  const String internal_flag =
      String::Format("--%s%s=%s|%d|%d|%d",
                     GTEST_FLAG_PREFIX_, kInternalRunDeathTestFlag,
                     file_, line_, death_test_index, pipe_fd[1]);
  Arguments args;
  args.AddArguments(GetArgvs());
  args.AddArgument(filter_flag.c_str());
  args.AddArgument(internal_flag.c_str());

  DeathTest::set_last_death_test_message("");

  CaptureStderr();
  // See the comment in NoExecDeathTest::AssumeRole for why the next line
  // is necessary.
  FlushInfoLog();

  const pid_t child_pid = ExecDeathTestFork(args.Argv(), pipe_fd[0]);
  GTEST_DEATH_TEST_CHECK_SYSCALL_(close(pipe_fd[1]));
  set_child_pid(child_pid);
  set_read_fd(pipe_fd[0]);
  set_spawned(true);
  return OVERSEE_TEST;
}

#endif  // !GTEST_OS_WINDOWS

// Creates a concrete DeathTest-derived class that depends on the
// --gtest_death_test_style flag, and sets the pointer pointed to
// by the "test" argument to its address.  If the test should be
// skipped, sets that pointer to NULL.  Returns true, unless the
// flag is set to an invalid value.
bool DefaultDeathTestFactory::Create(const char* statement, const RE* regex,
                                     const char* file, int line,
                                     DeathTest** test) {
  UnitTestImpl* const impl = GetUnitTestImpl();
  const InternalRunDeathTestFlag* const flag =
      impl->internal_run_death_test_flag();
  const int death_test_index = impl->current_test_info()
      ->increment_death_test_count();

  if (flag != NULL) {
    if (death_test_index > flag->index()) {
      DeathTest::set_last_death_test_message(String::Format(
          "Death test count (%d) somehow exceeded expected maximum (%d)",
          death_test_index, flag->index()));
      return false;
    }

    if (!(flag->file() == file && flag->line() == line &&
          flag->index() == death_test_index)) {
      *test = NULL;
      return true;
    }
  }

#if GTEST_OS_WINDOWS
  if (GTEST_FLAG(death_test_style) == "threadsafe" ||
      GTEST_FLAG(death_test_style) == "fast") {
    *test = new WindowsDeathTest(statement, regex, file, line);
  }
#else
  if (GTEST_FLAG(death_test_style) == "threadsafe") {
    *test = new ExecDeathTest(statement, regex, file, line);
  } else if (GTEST_FLAG(death_test_style) == "fast") {
    *test = new NoExecDeathTest(statement, regex);
  }
#endif  // GTEST_OS_WINDOWS
  else {  // NOLINT - this is more readable than unbalanced brackets inside #if.
    DeathTest::set_last_death_test_message(String::Format(
        "Unknown death test style \"%s\" encountered",
        GTEST_FLAG(death_test_style).c_str()));
    return false;
  }

  return true;
}

// Splits a given string on a given delimiter, populating a given
// vector with the fields.  GTEST_HAS_DEATH_TEST implies that we have
// ::std::string, so we can use it here.
// TODO(vladl@google.com): Get rid of std::vector to be able to build on
// Visual C++ 7.1 with exceptions disabled.
static void SplitString(const ::std::string& str, char delimiter,
                        ::std::vector< ::std::string>* dest) {
  ::std::vector< ::std::string> parsed;
  ::std::string::size_type pos = 0;
  while (true) {
    const ::std::string::size_type colon = str.find(delimiter, pos);
    if (colon == ::std::string::npos) {
      parsed.push_back(str.substr(pos));
      break;
    } else {
      parsed.push_back(str.substr(pos, colon - pos));
      pos = colon + 1;
    }
  }
  dest->swap(parsed);
}

#if GTEST_OS_WINDOWS
// Recreates the pipe and event handles from the provided parameters,
// signals the event, and returns a file descriptor wrapped around the pipe
// handle. This function is called in the child process only.
int GetStatusFileDescriptor(unsigned int parent_process_id,
                            size_t status_handle_as_size_t,
                            size_t event_handle_as_size_t) {
  AutoHandle parent_process_handle(::OpenProcess(PROCESS_DUP_HANDLE,
                                                   FALSE,  // Non-inheritable.
                                                   parent_process_id));
  if (parent_process_handle.Get() == INVALID_HANDLE_VALUE) {
    DeathTestAbort(String::Format("Unable to open parent process %u",
                                  parent_process_id));
  }

  // TODO(vladl@google.com): Replace the following check with a
  // compile-time assertion when available.
  GTEST_CHECK_(sizeof(HANDLE) <= sizeof(size_t));

  const HANDLE status_handle =
      reinterpret_cast<HANDLE>(status_handle_as_size_t);
  HANDLE dup_status_handle;

  // The newly initialized handle is accessible only in in the parent
  // process. To obtain one accessible within the child, we need to use
  // DuplicateHandle.
  if (!::DuplicateHandle(parent_process_handle.Get(), status_handle,
                         ::GetCurrentProcess(), &dup_status_handle,
                         0x0,    // Requested privileges ignored since
                                 // DUPLICATE_SAME_ACCESS is used.
                         FALSE,  // Request non-inheritable handler.
                         DUPLICATE_SAME_ACCESS)) {
    DeathTestAbort(String::Format(
        "Unable to duplicate the pipe handle %Iu from the parent process %u",
        status_handle_as_size_t, parent_process_id));
  }

  const HANDLE event_handle = reinterpret_cast<HANDLE>(event_handle_as_size_t);
  HANDLE dup_event_handle;

  if (!::DuplicateHandle(parent_process_handle.Get(), event_handle,
                         ::GetCurrentProcess(), &dup_event_handle,
                         0x0,
                         FALSE,
                         DUPLICATE_SAME_ACCESS)) {
    DeathTestAbort(String::Format(
        "Unable to duplicate the event handle %Iu from the parent process %u",
        event_handle_as_size_t, parent_process_id));
  }

  const int status_fd =
      ::_open_osfhandle(reinterpret_cast<intptr_t>(dup_status_handle),
                      O_APPEND | O_TEXT);
  if (status_fd == -1) {
    DeathTestAbort(String::Format(
        "Unable to convert pipe handle %Iu to a file descriptor",
        status_handle_as_size_t));
  }

  // Signals the parent that the write end of the pipe has been acquired
  // so the parent can release its own write end.
  ::SetEvent(dup_event_handle);

  return status_fd;
}
#endif  // GTEST_OS_WINDOWS

// Returns a newly created InternalRunDeathTestFlag object with fields
// initialized from the GTEST_FLAG(internal_run_death_test) flag if
// the flag is specified; otherwise returns NULL.
InternalRunDeathTestFlag* ParseInternalRunDeathTestFlag() {
  if (GTEST_FLAG(internal_run_death_test) == "") return NULL;

  // GTEST_HAS_DEATH_TEST implies that we have ::std::string, so we
  // can use it here.
  int line = -1;
  int index = -1;
  ::std::vector< ::std::string> fields;
  SplitString(GTEST_FLAG(internal_run_death_test).c_str(), '|', &fields);
  int status_fd = -1;

#if GTEST_OS_WINDOWS
  unsigned int parent_process_id = 0;
  size_t status_handle_as_size_t = 0;
  size_t event_handle_as_size_t = 0;

  if (fields.size() != 6
      || !ParseNaturalNumber(fields[1], &line)
      || !ParseNaturalNumber(fields[2], &index)
      || !ParseNaturalNumber(fields[3], &parent_process_id)
      || !ParseNaturalNumber(fields[4], &status_handle_as_size_t)
      || !ParseNaturalNumber(fields[5], &event_handle_as_size_t)) {
    DeathTestAbort(String::Format(
        "Bad --gtest_internal_run_death_test flag: %s",
        GTEST_FLAG(internal_run_death_test).c_str()));
  }
  status_fd = GetStatusFileDescriptor(parent_process_id,
                                      status_handle_as_size_t,
                                      event_handle_as_size_t);
#else
  if (fields.size() != 4
      || !ParseNaturalNumber(fields[1], &line)
      || !ParseNaturalNumber(fields[2], &index)
      || !ParseNaturalNumber(fields[3], &status_fd)) {
    DeathTestAbort(String::Format(
        "Bad --gtest_internal_run_death_test flag: %s",
        GTEST_FLAG(internal_run_death_test).c_str()));
  }
#endif  // GTEST_OS_WINDOWS
  return new InternalRunDeathTestFlag(fields[0], line, index, status_fd);
}

}  // namespace internal

#endif  // GTEST_HAS_DEATH_TEST

}  // namespace testing
