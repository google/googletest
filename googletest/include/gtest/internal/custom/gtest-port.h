// Copyright 2015, Google Inc.
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
// Injection point for custom user configurations. See README for details
//
// ** Custom implementation starts here **

#ifndef GTEST_INCLUDE_GTEST_INTERNAL_CUSTOM_GTEST_PORT_H_
#define GTEST_INCLUDE_GTEST_INTERNAL_CUSTOM_GTEST_PORT_H_
#include "gtest/internal/gtest-port-arch.h"

#include <string>

//   NOTE:
//   GUNIT_NO_GOOGLE3         - Only used when compiled on Linux.  Define it
//                              to remove dependency on google3 - some features
//                              are unavailable in this mode.

#define GTEST_FOR_GOOGLE_ 1  // Defined to 1 iff compiled inside Google.

// We define these even in non-google3 mode.

#define GTEST_FLAG(name) FLAGS_gunit_##name

#define GTEST_DEV_EMAIL_ "opensource-gtest@@google.com"
#define GTEST_FLAG_PREFIX_ "gunit_"
#define GTEST_FLAG_PREFIX_DASH_ "gunit-"
#define GTEST_FLAG_PREFIX_UPPER_ "GUNIT_"
#define GTEST_NAME_ "Google Test"
#define GTEST_PROJECT_URL_ "http://go/gunit/"
#define GTEST_DEFAULT_DEATH_TEST_STYLE "threadsafe"

// OS_MACOSX is defined by Blaze, when cross-compiling (on Linux) to
// Mac OS X.
// See also testing/base/public/BUILD for notes about GTEST_INTERNAL_PG3_MODE.
#if !defined(GUNIT_NO_GOOGLE3) && (GTEST_OS_CYGWIN || GTEST_OS_LINUX || \
                                   defined(OS_MACOSX))
# define GTEST_GOOGLE3_MODE_ 1
#endif

#if GTEST_GOOGLE3_MODE_

#define GTEST_HAS_ABSL 1

#define GTEST_INIT_GOOGLE_TEST_NAME_ "InitGoogle"

// Tell Google Test that hash_map/hash_set are available.
// Only used for testing.
#define GTEST_HAS_HASH_MAP_ 1
#define GTEST_HAS_HASH_SET_ 1

// base/commandlineoptions.h has its own --flagfile flag.
# define GTEST_USE_OWN_FLAGFILE_FLAG_ 0

// base/commandlineflags.h provides its own GetArgvs()
# define GTEST_CUSTOM_GET_ARGVS_() ::GetArgvs()

#include "base/callback.h"
#include "base/logging_extensions.h"
#include "base/synchronization.h"
#include "thread/thread.h"

// Provide the PCRE regex library.
#include "util/regexp/re2/re2.h"
# define GTEST_USES_PCRE 1
namespace testing {
namespace internal {
// This is almost 'using RE = ::RE2', except that it disambiguates
// RE::RE(std::string). It cannot be merged into the RE implementation below,
// since this version will support embedded NUL characters.
class RE {
 public:
  RE(absl::string_view regex) : regex_(new RE2(regex)) {}         // NOLINT
  RE(const char* regex) : RE(absl::string_view(regex)) {}         // NOLINT
  RE(const std::string& regex) : RE(absl::string_view(regex)) {}  // NOLINT
  RE(const RE& other) : RE(other.pattern()) {}

  // Returns the string representation of the regex.
  const std::string& pattern() const { return regex_->pattern(); }

  static bool FullMatch(absl::string_view str, const RE& re) {
    return RE2::FullMatch(str, *re.regex_);
  }
  static bool PartialMatch(absl::string_view str, const RE& re) {
    return RE2::PartialMatch(str, *re.regex_);
  }

 private:
  std::unique_ptr<RE2> regex_;
};

}  // namespace internal
}  // namespace testing

// For flags.
# include "base/commandlineflags.h"

#define GTEST_FLAG_SAVER_ absl::FlagSaver

// Macros for declaring flags.
# define GTEST_DECLARE_bool_(name)   DECLARE_bool(gunit_##name)
# define GTEST_DECLARE_int32_(name)  DECLARE_int32(gunit_##name)
# define GTEST_DECLARE_string_(name) DECLARE_string(gunit_##name)

// Macros for defining flags.
# define GTEST_DEFINE_bool_(name, default_val, doc) \
    DEFINE_bool(gunit_##name, default_val, doc)
# define GTEST_DEFINE_int32_(name, default_val, doc) \
    DEFINE_int32(gunit_##name, default_val, doc)
# define GTEST_DEFINE_string_(name, default_val, doc) \
    DEFINE_string(gunit_##name, default_val, doc)

# define GTEST_GET_BOOL_FROM_ENV_(flag, default_val) \
  ::BoolFromEnv(FlagToEnvVar(flag).c_str(), default_val)
# define GTEST_GET_INT32_FROM_ENV_(flag, default_val) \
  ::Int32FromEnv(FlagToEnvVar(flag).c_str(), default_val)
# define GTEST_GET_STRING_FROM_ENV_(flag, default_val) \
  ::StringFromEnv(FlagToEnvVar(flag).c_str(), default_val)

// For logging.
# include "third_party/absl/base/log_severity.h"
# include "base/logging.h"
# define GTEST_LOG_(severity) LOG(severity)
namespace testing {
namespace internal {
using ::LogToStderr;
inline void FlushInfoLog() { FlushLogFiles(base_logging::INFO); }
}  // namespace internal
}  // namespace testing

# define GTEST_CHECK_(condition) CHECK(condition)

// For CheckedDowncastToActualType
# include "base/casts.h"
# define GTEST_HAS_DOWNCAST_ 1

# define GTEST_HAS_NOTIFICATION_ 1
#include "absl/synchronization/notification.h"
namespace testing {
namespace internal {
using ::absl::Notification;
}  // namespace internal
}  // namespace testing

# define GTEST_HAS_MUTEX_AND_THREAD_LOCAL_ 1
# include "base/mutex.h"
# include "thread/threadlocal.h"
namespace testing {
namespace internal {
using absl::Mutex;
using absl::MutexLock;
using ::ThreadLocal;

// Forward-declares a static mutex.
# define GTEST_DECLARE_STATIC_MUTEX_(mutex) extern absl::Mutex mutex

// Defines and statically initializes a static mutex.
# define GTEST_DEFINE_STATIC_MUTEX_(mutex) \
  absl::Mutex mutex(absl::kConstInit)
}  // namespace internal
}  // namespace testing

// For thread annotations.
# include "third_party/absl/base/thread_annotations.h"
# define GTEST_EXCLUSIVE_LOCK_REQUIRED_(locks) \
   ABSL_EXCLUSIVE_LOCKS_REQUIRED(locks)
# define GTEST_LOCK_EXCLUDED_(locks) ABSL_LOCKS_EXCLUDED(locks)

#endif  // GTEST_GOOGLE3_MODE_

// Pre-r11 Android NDK releases for x86 and x86_64 do not have abi libraries.
# if GTEST_OS_LINUX_ANDROID && (defined(__i386__) || defined(__x86_64__))
#  define GTEST_HAS_CXXABI_H_ 0
# endif

#endif  // GTEST_INCLUDE_GTEST_INTERNAL_CUSTOM_GTEST_PORT_H_
