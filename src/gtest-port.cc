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
//
// Author: wan@google.com (Zhanyong Wan)

#include <gtest/internal/gtest-port.h>

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef GTEST_HAS_DEATH_TEST
#include <regex.h>
#endif  // GTEST_HAS_DEATH_TEST

#if GTEST_USES_SIMPLE_RE
#include <string.h>
#endif

#ifdef _WIN32_WCE
#include <windows.h>  // For TerminateProcess()
#endif  // _WIN32_WCE

#include <gtest/gtest-spi.h>
#include <gtest/gtest-message.h>
#include <gtest/internal/gtest-string.h>

// Indicates that this translation unit is part of Google Test's
// implementation.  It must come before gtest-internal-inl.h is
// included, or there will be a compiler error.  This trick is to
// prevent a user from accidentally including gtest-internal-inl.h in
// his code.
#define GTEST_IMPLEMENTATION
#include "src/gtest-internal-inl.h"
#undef GTEST_IMPLEMENTATION

namespace testing {
namespace internal {

#if GTEST_USES_POSIX_RE

// Implements RE.  Currently only needed for death tests.

RE::~RE() {
  regfree(&partial_regex_);
  regfree(&full_regex_);
  free(const_cast<char*>(pattern_));
}

// Returns true iff regular expression re matches the entire str.
bool RE::FullMatch(const char* str, const RE& re) {
  if (!re.is_valid_) return false;

  regmatch_t match;
  return regexec(&re.full_regex_, str, 1, &match, 0) == 0;
}

// Returns true iff regular expression re matches a substring of str
// (including str itself).
bool RE::PartialMatch(const char* str, const RE& re) {
  if (!re.is_valid_) return false;

  regmatch_t match;
  return regexec(&re.partial_regex_, str, 1, &match, 0) == 0;
}

// Initializes an RE from its string representation.
void RE::Init(const char* regex) {
  pattern_ = strdup(regex);

  // Reserves enough bytes to hold the regular expression used for a
  // full match.
  const size_t full_regex_len = strlen(regex) + 10;
  char* const full_pattern = new char[full_regex_len];

  snprintf(full_pattern, full_regex_len, "^(%s)$", regex);
  is_valid_ = regcomp(&full_regex_, full_pattern, REG_EXTENDED) == 0;
  // We want to call regcomp(&partial_regex_, ...) even if the
  // previous expression returns false.  Otherwise partial_regex_ may
  // not be properly initialized can may cause trouble when it's
  // freed.
  is_valid_ = (regcomp(&partial_regex_, regex, REG_EXTENDED) == 0) && is_valid_;
  EXPECT_TRUE(is_valid_)
      << "Regular expression \"" << regex
      << "\" is not a valid POSIX Extended regular expression.";

  delete[] full_pattern;
}

#elif GTEST_USES_SIMPLE_RE

// Returns true iff ch appears anywhere in str (excluding the
// terminating '\0' character).
bool IsInSet(char ch, const char* str) {
  return ch != '\0' && strchr(str, ch) != NULL;
}

// Returns true iff ch belongs to the given classification.  Unlike
// similar functions in <ctype.h>, these aren't affected by the
// current locale.
bool IsDigit(char ch) { return '0' <= ch && ch <= '9'; }
bool IsPunct(char ch) {
  return IsInSet(ch, "^-!\"#$%&'()*+,./:;<=>?@[\\]_`{|}~");
}
bool IsRepeat(char ch) { return IsInSet(ch, "?*+"); }
bool IsWhiteSpace(char ch) { return IsInSet(ch, " \f\n\r\t\v"); }
bool IsWordChar(char ch) {
  return ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') ||
      ('0' <= ch && ch <= '9') || ch == '_';
}

// Returns true iff "\\c" is a supported escape sequence.
bool IsValidEscape(char c) {
  return (IsPunct(c) || IsInSet(c, "dDfnrsStvwW"));
}

// Returns true iff the given atom (specified by escaped and pattern)
// matches ch.  The result is undefined if the atom is invalid.
bool AtomMatchesChar(bool escaped, char pattern_char, char ch) {
  if (escaped) {  // "\\p" where p is pattern_char.
    switch (pattern_char) {
      case 'd': return IsDigit(ch);
      case 'D': return !IsDigit(ch);
      case 'f': return ch == '\f';
      case 'n': return ch == '\n';
      case 'r': return ch == '\r';
      case 's': return IsWhiteSpace(ch);
      case 'S': return !IsWhiteSpace(ch);
      case 't': return ch == '\t';
      case 'v': return ch == '\v';
      case 'w': return IsWordChar(ch);
      case 'W': return !IsWordChar(ch);
    }
    return IsPunct(pattern_char) && pattern_char == ch;
  }

  return (pattern_char == '.' && ch != '\n') || pattern_char == ch;
}

// Helper function used by ValidateRegex() to format error messages.
String FormatRegexSyntaxError(const char* regex, int index) {
  return (Message() << "Syntax error at index " << index
          << " in simple regular expression \"" << regex << "\": ").GetString();
}

// Generates non-fatal failures and returns false if regex is invalid;
// otherwise returns true.
bool ValidateRegex(const char* regex) {
  if (regex == NULL) {
    // TODO(wan@google.com): fix the source file location in the
    // assertion failures to match where the regex is used in user
    // code.
    ADD_FAILURE() << "NULL is not a valid simple regular expression.";
    return false;
  }

  bool is_valid = true;

  // True iff ?, *, or + can follow the previous atom.
  bool prev_repeatable = false;
  for (int i = 0; regex[i]; i++) {
    if (regex[i] == '\\') {  // An escape sequence
      i++;
      if (regex[i] == '\0') {
        ADD_FAILURE() << FormatRegexSyntaxError(regex, i - 1)
                      << "'\\' cannot appear at the end.";
        return false;
      }

      if (!IsValidEscape(regex[i])) {
        ADD_FAILURE() << FormatRegexSyntaxError(regex, i - 1)
                      << "invalid escape sequence \"\\" << regex[i] << "\".";
        is_valid = false;
      }
      prev_repeatable = true;
    } else {  // Not an escape sequence.
      const char ch = regex[i];

      if (ch == '^' && i > 0) {
        ADD_FAILURE() << FormatRegexSyntaxError(regex, i)
                      << "'^' can only appear at the beginning.";
        is_valid = false;
      } else if (ch == '$' && regex[i + 1] != '\0') {
        ADD_FAILURE() << FormatRegexSyntaxError(regex, i)
                      << "'$' can only appear at the end.";
        is_valid = false;
      } else if (IsInSet(ch, "()[]{}|")) {
        ADD_FAILURE() << FormatRegexSyntaxError(regex, i)
                      << "'" << ch << "' is unsupported.";
        is_valid = false;
      } else if (IsRepeat(ch) && !prev_repeatable) {
        ADD_FAILURE() << FormatRegexSyntaxError(regex, i)
                      << "'" << ch << "' can only follow a repeatable token.";
        is_valid = false;
      }

      prev_repeatable = !IsInSet(ch, "^$?*+");
    }
  }

  return is_valid;
}

// Matches a repeated regex atom followed by a valid simple regular
// expression.  The regex atom is defined as c if escaped is false,
// or \c otherwise.  repeat is the repetition meta character (?, *,
// or +).  The behavior is undefined if str contains too many
// characters to be indexable by size_t, in which case the test will
// probably time out anyway.  We are fine with this limitation as
// std::string has it too.
bool MatchRepetitionAndRegexAtHead(
    bool escaped, char c, char repeat, const char* regex,
    const char* str) {
  const size_t min_count = (repeat == '+') ? 1 : 0;
  const size_t max_count = (repeat == '?') ? 1 :
      static_cast<size_t>(-1) - 1;
  // We cannot call numeric_limits::max() as it conflicts with the
  // max() macro on Windows.

  for (size_t i = 0; i <= max_count; ++i) {
    // We know that the atom matches each of the first i characters in str.
    if (i >= min_count && MatchRegexAtHead(regex, str + i)) {
      // We have enough matches at the head, and the tail matches too.
      // Since we only care about *whether* the pattern matches str
      // (as opposed to *how* it matches), there is no need to find a
      // greedy match.
      return true;
    }
    if (str[i] == '\0' || !AtomMatchesChar(escaped, c, str[i]))
      return false;
  }
  return false;
}

// Returns true iff regex matches a prefix of str.  regex must be a
// valid simple regular expression and not start with "^", or the
// result is undefined.
bool MatchRegexAtHead(const char* regex, const char* str) {
  if (*regex == '\0')  // An empty regex matches a prefix of anything.
    return true;

  // "$" only matches the end of a string.  Note that regex being
  // valid guarantees that there's nothing after "$" in it.
  if (*regex == '$')
    return *str == '\0';

  // Is the first thing in regex an escape sequence?
  const bool escaped = *regex == '\\';
  if (escaped)
    ++regex;
  if (IsRepeat(regex[1])) {
    // MatchRepetitionAndRegexAtHead() calls MatchRegexAtHead(), so
    // here's an indirect recursion.  It terminates as the regex gets
    // shorter in each recursion.
    return MatchRepetitionAndRegexAtHead(
        escaped, regex[0], regex[1], regex + 2, str);
  } else {
    // regex isn't empty, isn't "$", and doesn't start with a
    // repetition.  We match the first atom of regex with the first
    // character of str and recurse.
    return (*str != '\0') && AtomMatchesChar(escaped, *regex, *str) &&
        MatchRegexAtHead(regex + 1, str + 1);
  }
}

// Returns true iff regex matches any substring of str.  regex must be
// a valid simple regular expression, or the result is undefined.
//
// The algorithm is recursive, but the recursion depth doesn't exceed
// the regex length, so we won't need to worry about running out of
// stack space normally.  In rare cases the time complexity can be
// exponential with respect to the regex length + the string length,
// but usually it's must faster (often close to linear).
bool MatchRegexAnywhere(const char* regex, const char* str) {
  if (regex == NULL || str == NULL)
    return false;

  if (*regex == '^')
    return MatchRegexAtHead(regex + 1, str);

  // A successful match can be anywhere in str.
  do {
    if (MatchRegexAtHead(regex, str))
      return true;
  } while (*str++ != '\0');
  return false;
}

// Implements the RE class.

RE::~RE() {
  free(const_cast<char*>(pattern_));
  free(const_cast<char*>(full_pattern_));
}

// Returns true iff regular expression re matches the entire str.
bool RE::FullMatch(const char* str, const RE& re) {
  return re.is_valid_ && MatchRegexAnywhere(re.full_pattern_, str);
}

// Returns true iff regular expression re matches a substring of str
// (including str itself).
bool RE::PartialMatch(const char* str, const RE& re) {
  return re.is_valid_ && MatchRegexAnywhere(re.pattern_, str);
}

// Initializes an RE from its string representation.
void RE::Init(const char* regex) {
  pattern_ = full_pattern_ = NULL;
  if (regex != NULL) {
#ifdef GTEST_OS_WINDOWS
    pattern_ = _strdup(regex);
#else
    pattern_ = strdup(regex);
#endif
  }

  is_valid_ = ValidateRegex(regex);
  if (!is_valid_) {
    // No need to calculate the full pattern when the regex is invalid.
    return;
  }

  const size_t len = strlen(regex);
  // Reserves enough bytes to hold the regular expression used for a
  // full match: we need space to prepend a '^', append a '$', and
  // terminate the string with '\0'.
  char* buffer = static_cast<char*>(malloc(len + 3));
  full_pattern_ = buffer;

  if (*regex != '^')
    *buffer++ = '^';  // Makes sure full_pattern_ starts with '^'.

  // We don't use snprintf or strncpy, as they trigger a warning when
  // compiled with VC++ 8.0.
  memcpy(buffer, regex, len);
  buffer += len;

  if (len == 0 || regex[len - 1] != '$')
    *buffer++ = '$';  // Makes sure full_pattern_ ends with '$'.

  *buffer = '\0';
}

#endif  // GTEST_USES_POSIX_RE

// Logs a message at the given severity level.
void GTestLog(GTestLogSeverity severity, const char* file,
              int line, const char* msg) {
  const char* const marker =
      severity == GTEST_INFO ?    "[  INFO ]" :
      severity == GTEST_WARNING ? "[WARNING]" :
      severity == GTEST_ERROR ?   "[ ERROR ]" : "[ FATAL ]";
  fprintf(stderr, "\n%s %s:%d: %s\n", marker, file, line, msg);
  if (severity == GTEST_FATAL) {
    abort();
  }
}

#ifdef GTEST_HAS_DEATH_TEST

// Defines the stderr capturer.

class CapturedStderr {
 public:
  // The ctor redirects stderr to a temporary file.
  CapturedStderr() {
    uncaptured_fd_ = dup(STDERR_FILENO);

    // There's no guarantee that a test has write access to the
    // current directory, so we create the temporary file in the /tmp
    // directory instead.
    char name_template[] = "/tmp/captured_stderr.XXXXXX";
    const int captured_fd = mkstemp(name_template);
    filename_ = name_template;
    fflush(NULL);
    dup2(captured_fd, STDERR_FILENO);
    close(captured_fd);
  }

  ~CapturedStderr() {
    remove(filename_.c_str());
  }

  // Stops redirecting stderr.
  void StopCapture() {
    // Restores the original stream.
    fflush(NULL);
    dup2(uncaptured_fd_, STDERR_FILENO);
    close(uncaptured_fd_);
    uncaptured_fd_ = -1;
  }

  // Returns the name of the temporary file holding the stderr output.
  // GTEST_HAS_DEATH_TEST implies that we have ::std::string, so we
  // can use it here.
  ::std::string filename() const { return filename_; }

 private:
  int uncaptured_fd_;
  ::std::string filename_;
};

static CapturedStderr* g_captured_stderr = NULL;

// Returns the size (in bytes) of a file.
static size_t GetFileSize(FILE * file) {
  fseek(file, 0, SEEK_END);
  return static_cast<size_t>(ftell(file));
}

// Reads the entire content of a file as a string.
// GTEST_HAS_DEATH_TEST implies that we have ::std::string, so we can
// use it here.
static ::std::string ReadEntireFile(FILE * file) {
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

  const ::std::string content(buffer, buffer+bytes_read);
  delete[] buffer;

  return content;
}

// Starts capturing stderr.
void CaptureStderr() {
  if (g_captured_stderr != NULL) {
    GTEST_LOG_(FATAL, "Only one stderr capturer can exist at one time.");
  }
  g_captured_stderr = new CapturedStderr;
}

// Stops capturing stderr and returns the captured string.
// GTEST_HAS_DEATH_TEST implies that we have ::std::string, so we can
// use it here.
::std::string GetCapturedStderr() {
  g_captured_stderr->StopCapture();
  FILE* const file = fopen(g_captured_stderr->filename().c_str(), "r");
  const ::std::string content = ReadEntireFile(file);
  fclose(file);

  delete g_captured_stderr;
  g_captured_stderr = NULL;

  return content;
}

// A copy of all command line arguments.  Set by InitGoogleTest().
::std::vector<String> g_argvs;

// Returns the command line as a vector of strings.
const ::std::vector<String>& GetArgvs() { return g_argvs; }

#endif  // GTEST_HAS_DEATH_TEST

#ifdef _WIN32_WCE
void abort() {
  DebugBreak();
  TerminateProcess(GetCurrentProcess(), 1);
}
#endif  // _WIN32_WCE

// Returns the name of the environment variable corresponding to the
// given flag.  For example, FlagToEnvVar("foo") will return
// "GTEST_FOO" in the open-source version.
static String FlagToEnvVar(const char* flag) {
  const String full_flag = (Message() << GTEST_FLAG_PREFIX << flag).GetString();

  Message env_var;
  for (int i = 0; i != full_flag.GetLength(); i++) {
    env_var << static_cast<char>(toupper(full_flag.c_str()[i]));
  }

  return env_var.GetString();
}

// Parses 'str' for a 32-bit signed integer.  If successful, writes
// the result to *value and returns true; otherwise leaves *value
// unchanged and returns false.
bool ParseInt32(const Message& src_text, const char* str, Int32* value) {
  // Parses the environment variable as a decimal integer.
  char* end = NULL;
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

  // Is the parsed value in the range of an Int32?
  const Int32 result = static_cast<Int32>(long_value);
  if (long_value == LONG_MAX || long_value == LONG_MIN ||
      // The parsed value overflows as a long.  (strtol() returns
      // LONG_MAX or LONG_MIN when the input overflows.)
      result != long_value
      // The parsed value overflows as an Int32.
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
// The value is considered true iff it's not "0".
bool BoolFromGTestEnv(const char* flag, bool default_value) {
  const String env_var = FlagToEnvVar(flag);
  const char* const string_value = GetEnv(env_var.c_str());
  return string_value == NULL ?
      default_value : strcmp(string_value, "0") != 0;
}

// Reads and returns a 32-bit integer stored in the environment
// variable corresponding to the given flag; if it isn't set or
// doesn't represent a valid 32-bit integer, returns default_value.
Int32 Int32FromGTestEnv(const char* flag, Int32 default_value) {
  const String env_var = FlagToEnvVar(flag);
  const char* const string_value = GetEnv(env_var.c_str());
  if (string_value == NULL) {
    // The environment variable is not set.
    return default_value;
  }

  Int32 result = default_value;
  if (!ParseInt32(Message() << "Environment variable " << env_var,
                  string_value, &result)) {
    printf("The default value %s is used.\n",
           (Message() << default_value).GetString().c_str());
    fflush(stdout);
    return default_value;
  }

  return result;
}

// Reads and returns the string environment variable corresponding to
// the given flag; if it's not set, returns default_value.
const char* StringFromGTestEnv(const char* flag, const char* default_value) {
  const String env_var = FlagToEnvVar(flag);
  const char* const value = GetEnv(env_var.c_str());
  return value == NULL ? default_value : value;
}

}  // namespace internal
}  // namespace testing
