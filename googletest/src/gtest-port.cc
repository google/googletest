// Copyright 2008, Google Inc.
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


#include "gtest/internal/gtest-port.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstdint>
#include <fstream>
#include <memory>

#if GTEST_OS_WINDOWS
# include <windows.h>
# include <io.h>
# include <sys/stat.h>
# include <map>  // Used in ThreadLocal.
# ifdef _MSC_VER
#  include <crtdbg.h>
# endif  // _MSC_VER
#else
# include <unistd.h>
#endif  // GTEST_OS_WINDOWS

#if GTEST_OS_MAC
# include <mach/mach_init.h>
# include <mach/task.h>
# include <mach/vm_map.h>
#endif  // GTEST_OS_MAC

#if GTEST_OS_DRAGONFLY || GTEST_OS_FREEBSD || GTEST_OS_GNU_KFREEBSD || \
    GTEST_OS_NETBSD || GTEST_OS_OPENBSD
# include <sys/sysctl.h>
# if GTEST_OS_DRAGONFLY || GTEST_OS_FREEBSD || GTEST_OS_GNU_KFREEBSD
#  include <sys/user.h>
# endif
#endif

#if GTEST_OS_QNX
# include <devctl.h>
# include <fcntl.h>
# include <sys/procfs.h>
#endif  // GTEST_OS_QNX

#if GTEST_OS_AIX
# include <procinfo.h>
# include <sys/types.h>
#endif  // GTEST_OS_AIX

#if GTEST_OS_FUCHSIA
# include <zircon/process.h>
# include <zircon/syscalls.h>
#endif  // GTEST_OS_FUCHSIA

#include "gtest/gtest-spi.h"
#include "gtest/gtest-message.h"
#include "gtest/internal/gtest-internal.h"
#include "gtest/internal/gtest-string.h"
#include "src/gtest-internal-inl.h"

namespace testing {
namespace internal {

#if defined(_MSC_VER) || defined(__BORLANDC__)
// MSVC and C++Builder do not provide a definition of STDERR_FILENO.
const int kStdOutFileno = 1;
const int kStdErrFileno = 2;
#else
const int kStdOutFileno = STDOUT_FILENO;
const int kStdErrFileno = STDERR_FILENO;
#endif  // _MSC_VER

#if GTEST_OS_LINUX

namespace {
template <typename T>
T ReadProcFileField(const std::string& filename, int field) {
  std::string dummy;
  std::ifstream file(filename.c_str());
  while (field-- > 0) {
    file >> dummy;
  }
  T output = 0;
  file >> output;
  return output;
}
}  // namespace

// Returns the number of active threads, or 0 when there is an error.
size_t GetThreadCount() {
  const std::string filename =
      (Message() << "/proc/" << getpid() << "/stat").GetString();
  return ReadProcFileField<size_t>(filename, 19);
}

#elif GTEST_OS_MAC

size_t GetThreadCount() {
  const task_t task = mach_task_self();
  mach_msg_type_number_t thread_count;
  thread_act_array_t thread_list;
  const kern_return_t status = task_threads(task, &thread_list, &thread_count);
  if (status == KERN_SUCCESS) {
    // task_threads allocates resources in thread_list and we need to free them
    // to avoid leaks.
    vm_deallocate(task,
                  reinterpret_cast<vm_address_t>(thread_list),
                  sizeof(thread_t) * thread_count);
    return static_cast<size_t>(thread_count);
  } else {
    return 0;
  }
}

#elif GTEST_OS_DRAGONFLY || GTEST_OS_FREEBSD || GTEST_OS_GNU_KFREEBSD || \
      GTEST_OS_NETBSD

#if GTEST_OS_NETBSD
#undef KERN_PROC
#define KERN_PROC KERN_PROC2
#define kinfo_proc kinfo_proc2
#endif

#if GTEST_OS_DRAGONFLY
#define KP_NLWP(kp) (kp.kp_nthreads)
#elif GTEST_OS_FREEBSD || GTEST_OS_GNU_KFREEBSD
#define KP_NLWP(kp) (kp.ki_numthreads)
#elif GTEST_OS_NETBSD
#define KP_NLWP(kp) (kp.p_nlwps)
#endif

// Returns the number of threads running in the process, or 0 to indicate that
// we cannot detect it.
size_t GetThreadCount() {
  int mib[] = {
    CTL_KERN,
    KERN_PROC,
    KERN_PROC_PID,
    getpid(),
#if GTEST_OS_NETBSD
    sizeof(struct kinfo_proc),
    1,
#endif
  };
  u_int miblen = sizeof(mib) / sizeof(mib[0]);
  struct kinfo_proc info;
  size_t size = sizeof(info);
  if (sysctl(mib, miblen, &info, &size, NULL, 0)) {
    return 0;
  }
  return static_cast<size_t>(KP_NLWP(info));
}
#elif GTEST_OS_OPENBSD

// Returns the number of threads running in the process, or 0 to indicate that
// we cannot detect it.
size_t GetThreadCount() {
  int mib[] = {
    CTL_KERN,
    KERN_PROC,
    KERN_PROC_PID | KERN_PROC_SHOW_THREADS,
    getpid(),
    sizeof(struct kinfo_proc),
    0,
  };
  u_int miblen = sizeof(mib) / sizeof(mib[0]);

  // get number of structs
  size_t size;
  if (sysctl(mib, miblen, NULL, &size, NULL, 0)) {
    return 0;
  }
  mib[5] = size / mib[4];

  // populate array of structs
  struct kinfo_proc info[mib[5]];
  if (sysctl(mib, miblen, &info, &size, NULL, 0)) {
    return 0;
  }

  // exclude empty members
  int nthreads = 0;
  for (size_t i = 0; i < size / mib[4]; i++) {
    if (info[i].p_tid != -1)
      nthreads++;
  }
  return nthreads;
}

#elif GTEST_OS_QNX

// Returns the number of threads running in the process, or 0 to indicate that
// we cannot detect it.
size_t GetThreadCount() {
  const int fd = open("/proc/self/as", O_RDONLY);
  if (fd < 0) {
    return 0;
  }
  procfs_info process_info;
  const int status =
      devctl(fd, DCMD_PROC_INFO, &process_info, sizeof(process_info), nullptr);
  close(fd);
  if (status == EOK) {
    return static_cast<size_t>(process_info.num_threads);
  } else {
    return 0;
  }
}

#elif GTEST_OS_AIX

size_t GetThreadCount() {
  struct procentry64 entry;
  pid_t pid = getpid();
  int status = getprocs64(&entry, sizeof(entry), nullptr, 0, &pid, 1);
  if (status == 1) {
    return entry.pi_thcount;
  } else {
    return 0;
  }
}

#elif GTEST_OS_FUCHSIA

size_t GetThreadCount() {
  int dummy_buffer;
  size_t avail;
  zx_status_t status = zx_object_get_info(
      zx_process_self(),
      ZX_INFO_PROCESS_THREADS,
      &dummy_buffer,
      0,
      nullptr,
      &avail);
  if (status == ZX_OK) {
    return avail;
  } else {
    return 0;
  }
}

#else

size_t GetThreadCount() {
  // There's no portable way to detect the number of threads, so we just
  // return 0 to indicate that we cannot detect it.
  return 0;
}

#endif  // GTEST_OS_LINUX

#if GTEST_IS_THREADSAFE && GTEST_OS_WINDOWS

void SleepMilliseconds(int n) {
  ::Sleep(static_cast<DWORD>(n));
}

AutoHandle::AutoHandle()
    : handle_(INVALID_HANDLE_VALUE) {}

AutoHandle::AutoHandle(Handle handle)
    : handle_(handle) {}

AutoHandle::~AutoHandle() {
  Reset();
}

AutoHandle::Handle AutoHandle::Get() const {
  return handle_;
}

void AutoHandle::Reset() {
  Reset(INVALID_HANDLE_VALUE);
}

void AutoHandle::Reset(HANDLE handle) {
  // Resetting with the same handle we already own is invalid.
  if (handle_ != handle) {
    if (IsCloseable()) {
      ::CloseHandle(handle_);
    }
    handle_ = handle;
  } else {
    GTEST_CHECK_(!IsCloseable())
        << "Resetting a valid handle to itself is likely a programmer error "
            "and thus not allowed.";
  }
}

bool AutoHandle::IsCloseable() const {
  // Different Windows APIs may use either of these values to represent an
  // invalid handle.
  return handle_ != nullptr && handle_ != INVALID_HANDLE_VALUE;
}

Notification::Notification()
    : event_(::CreateEvent(nullptr,     // Default security attributes.
                           TRUE,        // Do not reset automatically.
                           FALSE,       // Initially unset.
                           nullptr)) {  // Anonymous event.
  GTEST_CHECK_(event_.Get() != nullptr);
}

void Notification::Notify() {
  GTEST_CHECK_(::SetEvent(event_.Get()) != FALSE);
}

void Notification::WaitForNotification() {
  GTEST_CHECK_(
      ::WaitForSingleObject(event_.Get(), INFINITE) == WAIT_OBJECT_0);
}

Mutex::Mutex()
    : owner_thread_id_(0),
      type_(kDynamic),
      critical_section_init_phase_(0),
      critical_section_(new CRITICAL_SECTION) {
  ::InitializeCriticalSection(critical_section_);
}

Mutex::~Mutex() {
  // Static mutexes are leaked intentionally. It is not thread-safe to try
  // to clean them up.
  if (type_ == kDynamic) {
    ::DeleteCriticalSection(critical_section_);
    delete critical_section_;
    critical_section_ = nullptr;
  }
}

void Mutex::Lock() {
  ThreadSafeLazyInit();
  ::EnterCriticalSection(critical_section_);
  owner_thread_id_ = ::GetCurrentThreadId();
}

void Mutex::Unlock() {
  ThreadSafeLazyInit();
  // We don't protect writing to owner_thread_id_ here, as it's the
  // caller's responsibility to ensure that the current thread holds the
  // mutex when this is called.
  owner_thread_id_ = 0;
  ::LeaveCriticalSection(critical_section_);
}

// Does nothing if the current thread holds the mutex. Otherwise, crashes
// with high probability.
void Mutex::AssertHeld() {
  ThreadSafeLazyInit();
  GTEST_CHECK_(owner_thread_id_ == ::GetCurrentThreadId())
      << "The current thread is not holding the mutex @" << this;
}

namespace {

#ifdef _MSC_VER
// Use the RAII idiom to flag mem allocs that are intentionally never
// deallocated. The motivation is to silence the false positive mem leaks
// that are reported by the debug version of MS's CRT which can only detect
// if an alloc is missing a matching deallocation.
// Example:
//    MemoryIsNotDeallocated memory_is_not_deallocated;
//    critical_section_ = new CRITICAL_SECTION;
//
class MemoryIsNotDeallocated
{
 public:
  MemoryIsNotDeallocated() : old_crtdbg_flag_(0) {
    old_crtdbg_flag_ = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
    // Set heap allocation block type to _IGNORE_BLOCK so that MS debug CRT
    // doesn't report mem leak if there's no matching deallocation.
    _CrtSetDbgFlag(old_crtdbg_flag_ & ~_CRTDBG_ALLOC_MEM_DF);
  }

  ~MemoryIsNotDeallocated() {
    // Restore the original _CRTDBG_ALLOC_MEM_DF flag
    _CrtSetDbgFlag(old_crtdbg_flag_);
  }

 private:
  int old_crtdbg_flag_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(MemoryIsNotDeallocated);
};
#endif  // _MSC_VER

}  // namespace

// Initializes owner_thread_id_ and critical_section_ in static mutexes.
void Mutex::ThreadSafeLazyInit() {
  // Dynamic mutexes are initialized in the constructor.
  if (type_ == kStatic) {
    switch (
        ::InterlockedCompareExchange(&critical_section_init_phase_, 1L, 0L)) {
      case 0:
        // If critical_section_init_phase_ was 0 before the exchange, we
        // are the first to test it and need to perform the initialization.
        owner_thread_id_ = 0;
        {
          // Use RAII to flag that following mem alloc is never deallocated.
#ifdef _MSC_VER
          MemoryIsNotDeallocated memory_is_not_deallocated;
#endif  // _MSC_VER
          critical_section_ = new CRITICAL_SECTION;
        }
        ::InitializeCriticalSection(critical_section_);
        // Updates the critical_section_init_phase_ to 2 to signal
        // initialization complete.
        GTEST_CHECK_(::InterlockedCompareExchange(
                          &critical_section_init_phase_, 2L, 1L) ==
                      1L);
        break;
      case 1:
        // Somebody else is already initializing the mutex; spin until they
        // are done.
        while (::InterlockedCompareExchange(&critical_section_init_phase_,
                                            2L,
                                            2L) != 2L) {
          // Possibly yields the rest of the thread's time slice to other
          // threads.
          ::Sleep(0);
        }
        break;

      case 2:
        break;  // The mutex is already initialized and ready for use.

      default:
        GTEST_CHECK_(false)
            << "Unexpected value of critical_section_init_phase_ "
            << "while initializing a static mutex.";
    }
  }
}

namespace {

class ThreadWithParamSupport : public ThreadWithParamBase {
 public:
  static HANDLE CreateThread(Runnable* runnable,
                             Notification* thread_can_start) {
    ThreadMainParam* param = new ThreadMainParam(runnable, thread_can_start);
    DWORD thread_id;
    HANDLE thread_handle = ::CreateThread(
        nullptr,  // Default security.
        0,        // Default stack size.
        &ThreadWithParamSupport::ThreadMain,
        param,        // Parameter to ThreadMainStatic
        0x0,          // Default creation flags.
        &thread_id);  // Need a valid pointer for the call to work under Win98.
    GTEST_CHECK_(thread_handle != nullptr)
        << "CreateThread failed with error " << ::GetLastError() << ".";
    if (thread_handle == nullptr) {
      delete param;
    }
    return thread_handle;
  }

 private:
  struct ThreadMainParam {
    ThreadMainParam(Runnable* runnable, Notification* thread_can_start)
        : runnable_(runnable),
          thread_can_start_(thread_can_start) {
    }
    std::unique_ptr<Runnable> runnable_;
    // Does not own.
    Notification* thread_can_start_;
  };

  static DWORD WINAPI ThreadMain(void* ptr) {
    // Transfers ownership.
    std::unique_ptr<ThreadMainParam> param(static_cast<ThreadMainParam*>(ptr));
    if (param->thread_can_start_ != nullptr)
      param->thread_can_start_->WaitForNotification();
    param->runnable_->Run();
    return 0;
  }

  // Prohibit instantiation.
  ThreadWithParamSupport();

  GTEST_DISALLOW_COPY_AND_ASSIGN_(ThreadWithParamSupport);
};

}  // namespace

ThreadWithParamBase::ThreadWithParamBase(Runnable *runnable,
                                         Notification* thread_can_start)
      : thread_(ThreadWithParamSupport::CreateThread(runnable,
                                                     thread_can_start)) {
}

ThreadWithParamBase::~ThreadWithParamBase() {
  Join();
}

void ThreadWithParamBase::Join() {
  GTEST_CHECK_(::WaitForSingleObject(thread_.Get(), INFINITE) == WAIT_OBJECT_0)
      << "Failed to join the thread with error " << ::GetLastError() << ".";
}

// Maps a thread to a set of ThreadIdToThreadLocals that have values
// instantiated on that thread and notifies them when the thread exits.  A
// ThreadLocal instance is expected to persist until all threads it has
// values on have terminated.
class ThreadLocalRegistryImpl {
 public:
  // Registers thread_local_instance as having value on the current thread.
  // Returns a value that can be used to identify the thread from other threads.
  static ThreadLocalValueHolderBase* GetValueOnCurrentThread(
      const ThreadLocalBase* thread_local_instance) {
#ifdef _MSC_VER
    MemoryIsNotDeallocated memory_is_not_deallocated;
#endif  // _MSC_VER
    DWORD current_thread = ::GetCurrentThreadId();
    MutexLock lock(&mutex_);
    ThreadIdToThreadLocals* const thread_to_thread_locals =
        GetThreadLocalsMapLocked();
    ThreadIdToThreadLocals::iterator thread_local_pos =
        thread_to_thread_locals->find(current_thread);
    if (thread_local_pos == thread_to_thread_locals->end()) {
      thread_local_pos = thread_to_thread_locals->insert(
          std::make_pair(current_thread, ThreadLocalValues())).first;
      StartWatcherThreadFor(current_thread);
    }
    ThreadLocalValues& thread_local_values = thread_local_pos->second;
    ThreadLocalValues::iterator value_pos =
        thread_local_values.find(thread_local_instance);
    if (value_pos == thread_local_values.end()) {
      value_pos =
          thread_local_values
              .insert(std::make_pair(
                  thread_local_instance,
                  std::shared_ptr<ThreadLocalValueHolderBase>(
                      thread_local_instance->NewValueForCurrentThread())))
              .first;
    }
    return value_pos->second.get();
  }

  static void OnThreadLocalDestroyed(
      const ThreadLocalBase* thread_local_instance) {
    std::vector<std::shared_ptr<ThreadLocalValueHolderBase> > value_holders;
    // Clean up the ThreadLocalValues data structure while holding the lock, but
    // defer the destruction of the ThreadLocalValueHolderBases.
    {
      MutexLock lock(&mutex_);
      ThreadIdToThreadLocals* const thread_to_thread_locals =
          GetThreadLocalsMapLocked();
      for (ThreadIdToThreadLocals::iterator it =
          thread_to_thread_locals->begin();
          it != thread_to_thread_locals->end();
          ++it) {
        ThreadLocalValues& thread_local_values = it->second;
        ThreadLocalValues::iterator value_pos =
            thread_local_values.find(thread_local_instance);
        if (value_pos != thread_local_values.end()) {
          value_holders.push_back(value_pos->second);
          thread_local_values.erase(value_pos);
          // This 'if' can only be successful at most once, so theoretically we
          // could break out of the loop here, but we don't bother doing so.
        }
      }
    }
    // Outside the lock, let the destructor for 'value_holders' deallocate the
    // ThreadLocalValueHolderBases.
  }

  static void OnThreadExit(DWORD thread_id) {
    GTEST_CHECK_(thread_id != 0) << ::GetLastError();
    std::vector<std::shared_ptr<ThreadLocalValueHolderBase> > value_holders;
    // Clean up the ThreadIdToThreadLocals data structure while holding the
    // lock, but defer the destruction of the ThreadLocalValueHolderBases.
    {
      MutexLock lock(&mutex_);
      ThreadIdToThreadLocals* const thread_to_thread_locals =
          GetThreadLocalsMapLocked();
      ThreadIdToThreadLocals::iterator thread_local_pos =
          thread_to_thread_locals->find(thread_id);
      if (thread_local_pos != thread_to_thread_locals->end()) {
        ThreadLocalValues& thread_local_values = thread_local_pos->second;
        for (ThreadLocalValues::iterator value_pos =
            thread_local_values.begin();
            value_pos != thread_local_values.end();
            ++value_pos) {
          value_holders.push_back(value_pos->second);
        }
        thread_to_thread_locals->erase(thread_local_pos);
      }
    }
    // Outside the lock, let the destructor for 'value_holders' deallocate the
    // ThreadLocalValueHolderBases.
  }

 private:
  // In a particular thread, maps a ThreadLocal object to its value.
  typedef std::map<const ThreadLocalBase*,
                   std::shared_ptr<ThreadLocalValueHolderBase> >
      ThreadLocalValues;
  // Stores all ThreadIdToThreadLocals having values in a thread, indexed by
  // thread's ID.
  typedef std::map<DWORD, ThreadLocalValues> ThreadIdToThreadLocals;

  // Holds the thread id and thread handle that we pass from
  // StartWatcherThreadFor to WatcherThreadFunc.
  typedef std::pair<DWORD, HANDLE> ThreadIdAndHandle;

  static void StartWatcherThreadFor(DWORD thread_id) {
    // The returned handle will be kept in thread_map and closed by
    // watcher_thread in WatcherThreadFunc.
    HANDLE thread = ::OpenThread(SYNCHRONIZE | THREAD_QUERY_INFORMATION,
                                 FALSE,
                                 thread_id);
    GTEST_CHECK_(thread != nullptr);
    // We need to pass a valid thread ID pointer into CreateThread for it
    // to work correctly under Win98.
    DWORD watcher_thread_id;
    HANDLE watcher_thread = ::CreateThread(
        nullptr,  // Default security.
        0,        // Default stack size
        &ThreadLocalRegistryImpl::WatcherThreadFunc,
        reinterpret_cast<LPVOID>(new ThreadIdAndHandle(thread_id, thread)),
        CREATE_SUSPENDED, &watcher_thread_id);
    GTEST_CHECK_(watcher_thread != nullptr);
    // Give the watcher thread the same priority as ours to avoid being
    // blocked by it.
    ::SetThreadPriority(watcher_thread,
                        ::GetThreadPriority(::GetCurrentThread()));
    ::ResumeThread(watcher_thread);
    ::CloseHandle(watcher_thread);
  }

  // Monitors exit from a given thread and notifies those
  // ThreadIdToThreadLocals about thread termination.
  static DWORD WINAPI WatcherThreadFunc(LPVOID param) {
    const ThreadIdAndHandle* tah =
        reinterpret_cast<const ThreadIdAndHandle*>(param);
    GTEST_CHECK_(
        ::WaitForSingleObject(tah->second, INFINITE) == WAIT_OBJECT_0);
    OnThreadExit(tah->first);
    ::CloseHandle(tah->second);
    delete tah;
    return 0;
  }

  // Returns map of thread local instances.
  static ThreadIdToThreadLocals* GetThreadLocalsMapLocked() {
    mutex_.AssertHeld();
#ifdef _MSC_VER
    MemoryIsNotDeallocated memory_is_not_deallocated;
#endif  // _MSC_VER
    static ThreadIdToThreadLocals* map = new ThreadIdToThreadLocals();
    return map;
  }

  // Protects access to GetThreadLocalsMapLocked() and its return value.
  static Mutex mutex_;
  // Protects access to GetThreadMapLocked() and its return value.
  static Mutex thread_map_mutex_;
};

Mutex ThreadLocalRegistryImpl::mutex_(Mutex::kStaticMutex);
Mutex ThreadLocalRegistryImpl::thread_map_mutex_(Mutex::kStaticMutex);

ThreadLocalValueHolderBase* ThreadLocalRegistry::GetValueOnCurrentThread(
      const ThreadLocalBase* thread_local_instance) {
  return ThreadLocalRegistryImpl::GetValueOnCurrentThread(
      thread_local_instance);
}

void ThreadLocalRegistry::OnThreadLocalDestroyed(
      const ThreadLocalBase* thread_local_instance) {
  ThreadLocalRegistryImpl::OnThreadLocalDestroyed(thread_local_instance);
}

#endif  // GTEST_IS_THREADSAFE && GTEST_OS_WINDOWS

const char kUnknownFile[] = "unknown file";

// Formats a source file path and a line number as they would appear
// in an error message from the compiler used to compile this code.
GTEST_API_ ::std::string FormatFileLocation(const char* file, int line) {
  const std::string file_name(file == nullptr ? kUnknownFile : file);

  if (line < 0) {
    return file_name + ":";
  }
#ifdef _MSC_VER
  return file_name + "(" + StreamableToString(line) + "):";
#else
  return file_name + ":" + StreamableToString(line) + ":";
#endif  // _MSC_VER
}

// Formats a file location for compiler-independent XML output.
// Although this function is not platform dependent, we put it next to
// FormatFileLocation in order to contrast the two functions.
// Note that FormatCompilerIndependentFileLocation() does NOT append colon
// to the file location it produces, unlike FormatFileLocation().
GTEST_API_ ::std::string FormatCompilerIndependentFileLocation(
    const char* file, int line) {
  const std::string file_name(file == nullptr ? kUnknownFile : file);

  if (line < 0)
    return file_name;
  else
    return file_name + ":" + StreamableToString(line);
}

GTestLog::GTestLog(GTestLogSeverity severity, const char* file, int line)
    : severity_(severity) {
  const char* const marker =
      severity == GTEST_INFO ?    "[  INFO ]" :
      severity == GTEST_WARNING ? "[WARNING]" :
      severity == GTEST_ERROR ?   "[ ERROR ]" : "[ FATAL ]";
  GetStream() << ::std::endl << marker << " "
              << FormatFileLocation(file, line).c_str() << ": ";
}

// Flushes the buffers and, if severity is GTEST_FATAL, aborts the program.
GTestLog::~GTestLog() {
  GetStream() << ::std::endl;
  if (severity_ == GTEST_FATAL) {
    fflush(stderr);
    posix::Abort();
  }
}

// Disable Microsoft deprecation warnings for POSIX functions called from
// this class (creat, dup, dup2, and close)
GTEST_DISABLE_MSC_DEPRECATED_PUSH_()

#if GTEST_HAS_STREAM_REDIRECTION

// Object that captures an output stream (stdout/stderr).
class CapturedStream {
 public:
  // The ctor redirects the stream to a temporary file.
  explicit CapturedStream(int fd) : fd_(fd), uncaptured_fd_(dup(fd)) {
# if GTEST_OS_WINDOWS
    char temp_dir_path[MAX_PATH + 1] = { '\0' };  // NOLINT
    char temp_file_path[MAX_PATH + 1] = { '\0' };  // NOLINT

    ::GetTempPathA(sizeof(temp_dir_path), temp_dir_path);
    const UINT success = ::GetTempFileNameA(temp_dir_path,
                                            "gtest_redir",
                                            0,  // Generate unique file name.
                                            temp_file_path);
    GTEST_CHECK_(success != 0)
        << "Unable to create a temporary file in " << temp_dir_path;
    const int captured_fd = creat(temp_file_path, _S_IREAD | _S_IWRITE);
    GTEST_CHECK_(captured_fd != -1) << "Unable to open temporary file "
                                    << temp_file_path;
    filename_ = temp_file_path;
# else
    // There's no guarantee that a test has write access to the current
    // directory, so we create the temporary file in the /tmp directory
    // instead. We use /tmp on most systems, and /sdcard on Android.
    // That's because Android doesn't have /tmp.
#  if GTEST_OS_LINUX_ANDROID
    // Note: Android applications are expected to call the framework's
    // Context.getExternalStorageDirectory() method through JNI to get
    // the location of the world-writable SD Card directory. However,
    // this requires a Context handle, which cannot be retrieved
    // globally from native code. Doing so also precludes running the
    // code as part of a regular standalone executable, which doesn't
    // run in a Dalvik process (e.g. when running it through 'adb shell').
    //
    // The location /data/local/tmp is directly accessible from native code.
    // '/sdcard' and other variants cannot be relied on, as they are not
    // guaranteed to be mounted, or may have a delay in mounting.
    char name_template[] = "/data/local/tmp/gtest_captured_stream.XXXXXX";
#  else
    char name_template[] = "/tmp/captured_stream.XXXXXX";
#  endif  // GTEST_OS_LINUX_ANDROID
    const int captured_fd = mkstemp(name_template);
    if (captured_fd == -1) {
      GTEST_LOG_(WARNING)
          << "Failed to create tmp file " << name_template
          << " for test; does the test have access to the /tmp directory?";
    }
    filename_ = name_template;
# endif  // GTEST_OS_WINDOWS
    fflush(nullptr);
    dup2(captured_fd, fd_);
    close(captured_fd);
  }

  ~CapturedStream() {
    remove(filename_.c_str());
  }

  std::string GetCapturedString() {
    if (uncaptured_fd_ != -1) {
      // Restores the original stream.
      fflush(nullptr);
      dup2(uncaptured_fd_, fd_);
      close(uncaptured_fd_);
      uncaptured_fd_ = -1;
    }

    FILE* const file = posix::FOpen(filename_.c_str(), "r");
    if (file == nullptr) {
      GTEST_LOG_(FATAL) << "Failed to open tmp file " << filename_
                        << " for capturing stream.";
    }
    const std::string content = ReadEntireFile(file);
    posix::FClose(file);
    return content;
  }

 private:
  const int fd_;  // A stream to capture.
  int uncaptured_fd_;
  // Name of the temporary file holding the stderr output.
  ::std::string filename_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(CapturedStream);
};

GTEST_DISABLE_MSC_DEPRECATED_POP_()

static CapturedStream* g_captured_stderr = nullptr;
static CapturedStream* g_captured_stdout = nullptr;

// Starts capturing an output stream (stdout/stderr).
static void CaptureStream(int fd, const char* stream_name,
                          CapturedStream** stream) {
  if (*stream != nullptr) {
    GTEST_LOG_(FATAL) << "Only one " << stream_name
                      << " capturer can exist at a time.";
  }
  *stream = new CapturedStream(fd);
}

// Stops capturing the output stream and returns the captured string.
static std::string GetCapturedStream(CapturedStream** captured_stream) {
  const std::string content = (*captured_stream)->GetCapturedString();

  delete *captured_stream;
  *captured_stream = nullptr;

  return content;
}

// Starts capturing stdout.
void CaptureStdout() {
  CaptureStream(kStdOutFileno, "stdout", &g_captured_stdout);
}

// Starts capturing stderr.
void CaptureStderr() {
  CaptureStream(kStdErrFileno, "stderr", &g_captured_stderr);
}

// Stops capturing stdout and returns the captured string.
std::string GetCapturedStdout() {
  return GetCapturedStream(&g_captured_stdout);
}

// Stops capturing stderr and returns the captured string.
std::string GetCapturedStderr() {
  return GetCapturedStream(&g_captured_stderr);
}

#endif  // GTEST_HAS_STREAM_REDIRECTION





size_t GetFileSize(FILE* file) {
  fseek(file, 0, SEEK_END);
  return static_cast<size_t>(ftell(file));
}

std::string ReadEntireFile(FILE* file) {
  const size_t file_size = GetFileSize(file);
  char* const buffer = new char[file_size];

  size_t bytes_last_read = 0;  // # of bytes read in the last fread()
  size_t bytes_read = 0;       // # of bytes read so far

  fseek(file, 0, SEEK_SET);

  // Keeps reading the file until we cannot read further or the
  // pre-determined file size is reached.
  do {
    bytes_last_read = fread(buffer+bytes_read, 1, file_size-bytes_read, file);
    bytes_read += bytes_last_read;
  } while (bytes_last_read > 0 && bytes_read < file_size);

  const std::string content(buffer, bytes_read);
  delete[] buffer;

  return content;
}

#if GTEST_HAS_DEATH_TEST
static const std::vector<std::string>* g_injected_test_argvs =
    nullptr;  // Owned.

std::vector<std::string> GetInjectableArgvs() {
  if (g_injected_test_argvs != nullptr) {
    return *g_injected_test_argvs;
  }
  return GetArgvs();
}

void SetInjectableArgvs(const std::vector<std::string>* new_argvs) {
  if (g_injected_test_argvs != new_argvs) delete g_injected_test_argvs;
  g_injected_test_argvs = new_argvs;
}

void SetInjectableArgvs(const std::vector<std::string>& new_argvs) {
  SetInjectableArgvs(
      new std::vector<std::string>(new_argvs.begin(), new_argvs.end()));
}

void ClearInjectableArgvs() {
  delete g_injected_test_argvs;
  g_injected_test_argvs = nullptr;
}
#endif  // GTEST_HAS_DEATH_TEST

#if GTEST_OS_WINDOWS_MOBILE
namespace posix {
void Abort() {
  DebugBreak();
  TerminateProcess(GetCurrentProcess(), 1);
}
}  // namespace posix
#endif  // GTEST_OS_WINDOWS_MOBILE

// Returns the name of the environment variable corresponding to the
// given flag.  For example, FlagToEnvVar("foo") will return
// "GTEST_FOO" in the open-source version.
static std::string FlagToEnvVar(const char* flag) {
  const std::string full_flag =
      (Message() << GTEST_FLAG_PREFIX_ << flag).GetString();

  Message env_var;
  for (size_t i = 0; i != full_flag.length(); i++) {
    env_var << ToUpper(full_flag.c_str()[i]);
  }

  return env_var.GetString();
}

// Parses 'str' for a 32-bit signed integer.  If successful, writes
// the result to *value and returns true; otherwise leaves *value
// unchanged and returns false.
bool ParseInt32(const Message& src_text, const char* str, int32_t* value) {
  // Parses the environment variable as a decimal integer.
  char* end = nullptr;
  const long long_value = strtol(str, &end, 10);  // NOLINT

  // Has strtol() consumed all characters in the string?
  if (*end != '\0') {
    // No - an invalid character was encountered.
    Message msg;
    msg << "WARNING: " << src_text
        << " is expected to be a 32-bit integer, but actually"
        << " has value \"" << str << "\".\n";
    printf("%s", msg.GetString().c_str());
    fflush(stdout);
    return false;
  }

  // Is the parsed value in the range of an int32_t?
  const auto result = static_cast<int32_t>(long_value);
  if (long_value == LONG_MAX || long_value == LONG_MIN ||
      // The parsed value overflows as a long.  (strtol() returns
      // LONG_MAX or LONG_MIN when the input overflows.)
      result != long_value
      // The parsed value overflows as an int32_t.
      ) {
    Message msg;
    msg << "WARNING: " << src_text
        << " is expected to be a 32-bit integer, but actually"
        << " has value " << str << ", which overflows.\n";
    printf("%s", msg.GetString().c_str());
    fflush(stdout);
    return false;
  }

  *value = result;
  return true;
}

// Reads and returns the Boolean environment variable corresponding to
// the given flag; if it's not set, returns default_value.
//
// The value is considered true if and only if it's not "0".
bool BoolFromGTestEnv(const char* flag, bool default_value) {
#if defined(GTEST_GET_BOOL_FROM_ENV_)
  return GTEST_GET_BOOL_FROM_ENV_(flag, default_value);
#else
  const std::string env_var = FlagToEnvVar(flag);
  const char* const string_value = posix::GetEnv(env_var.c_str());
  return string_value == nullptr ? default_value
                                 : strcmp(string_value, "0") != 0;
#endif  // defined(GTEST_GET_BOOL_FROM_ENV_)
}

// Reads and returns a 32-bit integer stored in the environment
// variable corresponding to the given flag; if it isn't set or
// doesn't represent a valid 32-bit integer, returns default_value.
int32_t Int32FromGTestEnv(const char* flag, int32_t default_value) {
#if defined(GTEST_GET_INT32_FROM_ENV_)
  return GTEST_GET_INT32_FROM_ENV_(flag, default_value);
#else
  const std::string env_var = FlagToEnvVar(flag);
  const char* const string_value = posix::GetEnv(env_var.c_str());
  if (string_value == nullptr) {
    // The environment variable is not set.
    return default_value;
  }

  int32_t result = default_value;
  if (!ParseInt32(Message() << "Environment variable " << env_var,
                  string_value, &result)) {
    printf("The default value %s is used.\n",
           (Message() << default_value).GetString().c_str());
    fflush(stdout);
    return default_value;
  }

  return result;
#endif  // defined(GTEST_GET_INT32_FROM_ENV_)
}

// As a special case for the 'output' flag, if GTEST_OUTPUT is not
// set, we look for XML_OUTPUT_FILE, which is set by the Bazel build
// system.  The value of XML_OUTPUT_FILE is a filename without the
// "xml:" prefix of GTEST_OUTPUT.
// Note that this is meant to be called at the call site so it does
// not check that the flag is 'output'
// In essence this checks an env variable called XML_OUTPUT_FILE
// and if it is set we prepend "xml:" to its value, if it not set we return ""
std::string OutputFlagAlsoCheckEnvVar(){
  std::string default_value_for_output_flag = "";
  const char* xml_output_file_env = posix::GetEnv("XML_OUTPUT_FILE");
  if (nullptr != xml_output_file_env) {
    default_value_for_output_flag = std::string("xml:") + xml_output_file_env;
  }
  return default_value_for_output_flag;
}

// Reads and returns the string environment variable corresponding to
// the given flag; if it's not set, returns default_value.
const char* StringFromGTestEnv(const char* flag, const char* default_value) {
#if defined(GTEST_GET_STRING_FROM_ENV_)
  return GTEST_GET_STRING_FROM_ENV_(flag, default_value);
#else
  const std::string env_var = FlagToEnvVar(flag);
  const char* const value = posix::GetEnv(env_var.c_str());
  return value == nullptr ? default_value : value;
#endif  // defined(GTEST_GET_STRING_FROM_ENV_)
}

}  // namespace internal
}  // namespace testing
