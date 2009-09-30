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

// Utility functions and classes used by the Google C++ testing framework.
//
// Author: wan@google.com (Zhanyong Wan)
//
// This file contains purely Google Test's internal implementation.  Please
// DO NOT #INCLUDE IT IN A USER PROGRAM.

#ifndef GTEST_SRC_GTEST_INTERNAL_INL_H_
#define GTEST_SRC_GTEST_INTERNAL_INL_H_

// GTEST_IMPLEMENTATION_ is defined to 1 iff the current translation unit is
// part of Google Test's implementation; otherwise it's undefined.
#if !GTEST_IMPLEMENTATION_
// A user is trying to include this from his code - just say no.
#error "gtest-internal-inl.h is part of Google Test's internal implementation."
#error "It must not be included except by Google Test itself."
#endif  // GTEST_IMPLEMENTATION_

#ifndef _WIN32_WCE
#include <errno.h>
#endif  // !_WIN32_WCE
#include <stddef.h>
#include <stdlib.h>  // For strtoll/_strtoul64/malloc/free.
#include <string.h>  // For memmove.

#include <string>

#include <gtest/internal/gtest-port.h>

#if GTEST_OS_WINDOWS
#include <windows.h>  // For DWORD.
#endif  // GTEST_OS_WINDOWS

#include <gtest/gtest.h>
#include <gtest/gtest-spi.h>

namespace testing {

// Declares the flags.
//
// We don't want the users to modify this flag in the code, but want
// Google Test's own unit tests to be able to access it. Therefore we
// declare it here as opposed to in gtest.h.
GTEST_DECLARE_bool_(death_test_use_fork);

namespace internal {

// The value of GetTestTypeId() as seen from within the Google Test
// library.  This is solely for testing GetTestTypeId().
extern const TypeId kTestTypeIdInGoogleTest;

// Names of the flags (needed for parsing Google Test flags).
const char kAlsoRunDisabledTestsFlag[] = "also_run_disabled_tests";
const char kBreakOnFailureFlag[] = "break_on_failure";
const char kCatchExceptionsFlag[] = "catch_exceptions";
const char kColorFlag[] = "color";
const char kFilterFlag[] = "filter";
const char kListTestsFlag[] = "list_tests";
const char kOutputFlag[] = "output";
const char kPrintTimeFlag[] = "print_time";
const char kRandomSeedFlag[] = "random_seed";
const char kRepeatFlag[] = "repeat";
const char kShuffleFlag[] = "shuffle";
const char kThrowOnFailureFlag[] = "throw_on_failure";

// A valid random seed must be in [1, kMaxRandomSeed].
const int kMaxRandomSeed = 99999;

// Returns the current time in milliseconds.
TimeInMillis GetTimeInMillis();

// Returns a random seed in range [1, kMaxRandomSeed] based on the
// given --gtest_random_seed flag value.
inline int GetRandomSeedFromFlag(Int32 random_seed_flag) {
  const unsigned int raw_seed = (random_seed_flag == 0) ?
      static_cast<unsigned int>(GetTimeInMillis()) :
      static_cast<unsigned int>(random_seed_flag);

  // Normalizes the actual seed to range [1, kMaxRandomSeed] such that
  // it's easy to type.
  const int normalized_seed =
      static_cast<int>((raw_seed - 1U) %
                       static_cast<unsigned int>(kMaxRandomSeed)) + 1;
  return normalized_seed;
}

// Returns the first valid random seed after 'seed'.  The behavior is
// undefined if 'seed' is invalid.  The seed after kMaxRandomSeed is
// considered to be 1.
inline int GetNextRandomSeed(int seed) {
  GTEST_CHECK_(1 <= seed && seed <= kMaxRandomSeed)
      << "Invalid random seed " << seed << " - must be in [1, "
      << kMaxRandomSeed << "].";
  const int next_seed = seed + 1;
  return (next_seed > kMaxRandomSeed) ? 1 : next_seed;
}

// This class saves the values of all Google Test flags in its c'tor, and
// restores them in its d'tor.
class GTestFlagSaver {
 public:
  // The c'tor.
  GTestFlagSaver() {
    also_run_disabled_tests_ = GTEST_FLAG(also_run_disabled_tests);
    break_on_failure_ = GTEST_FLAG(break_on_failure);
    catch_exceptions_ = GTEST_FLAG(catch_exceptions);
    color_ = GTEST_FLAG(color);
    death_test_style_ = GTEST_FLAG(death_test_style);
    death_test_use_fork_ = GTEST_FLAG(death_test_use_fork);
    filter_ = GTEST_FLAG(filter);
    internal_run_death_test_ = GTEST_FLAG(internal_run_death_test);
    list_tests_ = GTEST_FLAG(list_tests);
    output_ = GTEST_FLAG(output);
    print_time_ = GTEST_FLAG(print_time);
    random_seed_ = GTEST_FLAG(random_seed);
    repeat_ = GTEST_FLAG(repeat);
    shuffle_ = GTEST_FLAG(shuffle);
    throw_on_failure_ = GTEST_FLAG(throw_on_failure);
  }

  // The d'tor is not virtual.  DO NOT INHERIT FROM THIS CLASS.
  ~GTestFlagSaver() {
    GTEST_FLAG(also_run_disabled_tests) = also_run_disabled_tests_;
    GTEST_FLAG(break_on_failure) = break_on_failure_;
    GTEST_FLAG(catch_exceptions) = catch_exceptions_;
    GTEST_FLAG(color) = color_;
    GTEST_FLAG(death_test_style) = death_test_style_;
    GTEST_FLAG(death_test_use_fork) = death_test_use_fork_;
    GTEST_FLAG(filter) = filter_;
    GTEST_FLAG(internal_run_death_test) = internal_run_death_test_;
    GTEST_FLAG(list_tests) = list_tests_;
    GTEST_FLAG(output) = output_;
    GTEST_FLAG(print_time) = print_time_;
    GTEST_FLAG(random_seed) = random_seed_;
    GTEST_FLAG(repeat) = repeat_;
    GTEST_FLAG(shuffle) = shuffle_;
    GTEST_FLAG(throw_on_failure) = throw_on_failure_;
  }
 private:
  // Fields for saving the original values of flags.
  bool also_run_disabled_tests_;
  bool break_on_failure_;
  bool catch_exceptions_;
  String color_;
  String death_test_style_;
  bool death_test_use_fork_;
  String filter_;
  String internal_run_death_test_;
  bool list_tests_;
  String output_;
  bool print_time_;
  bool pretty_;
  internal::Int32 random_seed_;
  internal::Int32 repeat_;
  bool shuffle_;
  bool throw_on_failure_;
} GTEST_ATTRIBUTE_UNUSED_;

// Converts a Unicode code point to a narrow string in UTF-8 encoding.
// code_point parameter is of type UInt32 because wchar_t may not be
// wide enough to contain a code point.
// The output buffer str must containt at least 32 characters.
// The function returns the address of the output buffer.
// If the code_point is not a valid Unicode code point
// (i.e. outside of Unicode range U+0 to U+10FFFF) it will be output
// as '(Invalid Unicode 0xXXXXXXXX)'.
char* CodePointToUtf8(UInt32 code_point, char* str);

// Converts a wide string to a narrow string in UTF-8 encoding.
// The wide string is assumed to have the following encoding:
//   UTF-16 if sizeof(wchar_t) == 2 (on Windows, Cygwin, Symbian OS)
//   UTF-32 if sizeof(wchar_t) == 4 (on Linux)
// Parameter str points to a null-terminated wide string.
// Parameter num_chars may additionally limit the number
// of wchar_t characters processed. -1 is used when the entire string
// should be processed.
// If the string contains code points that are not valid Unicode code points
// (i.e. outside of Unicode range U+0 to U+10FFFF) they will be output
// as '(Invalid Unicode 0xXXXXXXXX)'. If the string is in UTF16 encoding
// and contains invalid UTF-16 surrogate pairs, values in those pairs
// will be encoded as individual Unicode characters from Basic Normal Plane.
String WideStringToUtf8(const wchar_t* str, int num_chars);

// Returns the number of active threads, or 0 when there is an error.
size_t GetThreadCount();

// Reads the GTEST_SHARD_STATUS_FILE environment variable, and creates the file
// if the variable is present. If a file already exists at this location, this
// function will write over it. If the variable is present, but the file cannot
// be created, prints an error and exits.
void WriteToShardStatusFileIfNeeded();

// Checks whether sharding is enabled by examining the relevant
// environment variable values. If the variables are present,
// but inconsistent (e.g., shard_index >= total_shards), prints
// an error and exits. If in_subprocess_for_death_test, sharding is
// disabled because it must only be applied to the original test
// process. Otherwise, we could filter out death tests we intended to execute.
bool ShouldShard(const char* total_shards_str, const char* shard_index_str,
                 bool in_subprocess_for_death_test);

// Parses the environment variable var as an Int32. If it is unset,
// returns default_val. If it is not an Int32, prints an error and
// and aborts.
Int32 Int32FromEnvOrDie(const char* env_var, Int32 default_val);

// Given the total number of shards, the shard index, and the test id,
// returns true iff the test should be run on this shard. The test id is
// some arbitrary but unique non-negative integer assigned to each test
// method. Assumes that 0 <= shard_index < total_shards.
bool ShouldRunTestOnShard(int total_shards, int shard_index, int test_id);

// Vector is an ordered container that supports random access to the
// elements.
//
// We cannot use std::vector, as Visual C++ 7.1's implementation of
// STL has problems compiling when exceptions are disabled.  There is
// a hack to work around the problems, but we've seen cases where the
// hack fails to work.
//
// The element type must support copy constructor and operator=.
template <typename E>  // E is the element type.
class Vector {
 public:
  // Creates an empty Vector.
  Vector() : elements_(NULL), capacity_(0), size_(0) {}

  // D'tor.
  virtual ~Vector() { Clear(); }

  // Clears the Vector.
  void Clear() {
    if (elements_ != NULL) {
      for (int i = 0; i < size_; i++) {
        delete elements_[i];
      }

      free(elements_);
      elements_ = NULL;
      capacity_ = size_ = 0;
    }
  }

  // Gets the number of elements.
  int size() const { return size_; }

  // Adds an element to the end of the Vector.  A copy of the element
  // is created using the copy constructor, and then stored in the
  // Vector.  Changes made to the element in the Vector doesn't affect
  // the source object, and vice versa.
  void PushBack(const E& element) { Insert(element, size_); }

  // Adds an element to the beginning of this Vector.
  void PushFront(const E& element) { Insert(element, 0); }

  // Removes an element from the beginning of this Vector.  If the
  // result argument is not NULL, the removed element is stored in the
  // memory it points to.  Otherwise the element is thrown away.
  // Returns true iff the vector wasn't empty before the operation.
  bool PopFront(E* result) {
    if (size_ == 0)
      return false;

    if (result != NULL)
      *result = GetElement(0);

    Erase(0);
    return true;
  }

  // Inserts an element at the given index.  It's the caller's
  // responsibility to ensure that the given index is in the range [0,
  // size()].
  void Insert(const E& element, int index) {
    GrowIfNeeded();
    MoveElements(index, size_ - index, index + 1);
    elements_[index] = new E(element);
    size_++;
  }

  // Erases the element at the specified index, or aborts the program if the
  // index is not in range [0, size()).
  void Erase(int index) {
    GTEST_CHECK_(0 <= index && index < size_)
        << "Invalid Vector index " << index << ": must be in range [0, "
        << (size_ - 1) << "].";

    delete elements_[index];
    MoveElements(index + 1, size_ - index - 1, index);
    size_--;
  }

  // Returns the number of elements that satisfy a given predicate.
  // The parameter 'predicate' is a Boolean function or functor that
  // accepts a 'const E &', where E is the element type.
  template <typename P>  // P is the type of the predicate function/functor
  int CountIf(P predicate) const {
    int count = 0;
    for (int i = 0; i < size_; i++) {
      if (predicate(*(elements_[i]))) {
        count++;
      }
    }

    return count;
  }

  // Applies a function/functor to each element in the Vector.  The
  // parameter 'functor' is a function/functor that accepts a 'const
  // E &', where E is the element type.  This method does not change
  // the elements.
  template <typename F>  // F is the type of the function/functor
  void ForEach(F functor) const {
    for (int i = 0; i < size_; i++) {
      functor(*(elements_[i]));
    }
  }

  // Returns the first node whose element satisfies a given predicate,
  // or NULL if none is found.  The parameter 'predicate' is a
  // function/functor that accepts a 'const E &', where E is the
  // element type.  This method does not change the elements.
  template <typename P>  // P is the type of the predicate function/functor.
  const E* FindIf(P predicate) const {
    for (int i = 0; i < size_; i++) {
      if (predicate(*elements_[i])) {
        return elements_[i];
      }
    }
    return NULL;
  }

  template <typename P>
  E* FindIf(P predicate) {
    for (int i = 0; i < size_; i++) {
      if (predicate(*elements_[i])) {
        return elements_[i];
      }
    }
    return NULL;
  }

  // Returns the i-th element of the Vector, or aborts the program if i
  // is not in range [0, size()).
  const E& GetElement(int i) const {
    GTEST_CHECK_(0 <= i && i < size_)
        << "Invalid Vector index " << i << ": must be in range [0, "
        << (size_ - 1) << "].";

    return *(elements_[i]);
  }

  // Returns a mutable reference to the i-th element of the Vector, or
  // aborts the program if i is not in range [0, size()).
  E& GetMutableElement(int i) {
    GTEST_CHECK_(0 <= i && i < size_)
        << "Invalid Vector index " << i << ": must be in range [0, "
        << (size_ - 1) << "].";

    return *(elements_[i]);
  }

  // Returns the i-th element of the Vector, or default_value if i is not
  // in range [0, size()).
  E GetElementOr(int i, E default_value) const {
    return (i < 0 || i >= size_) ? default_value : *(elements_[i]);
  }

  // Swaps the i-th and j-th elements of the Vector.  Crashes if i or
  // j is invalid.
  void Swap(int i, int j) {
    GTEST_CHECK_(0 <= i && i < size_)
        << "Invalid first swap element " << i << ": must be in range [0, "
        << (size_ - 1) << "].";
    GTEST_CHECK_(0 <= j && j < size_)
        << "Invalid second swap element " << j << ": must be in range [0, "
        << (size_ - 1) << "].";

    E* const temp = elements_[i];
    elements_[i] = elements_[j];
    elements_[j] = temp;
  }

  // Performs an in-place shuffle of a range of this Vector's nodes.
  // 'begin' and 'end' are element indices as an STL-style range;
  // i.e. [begin, end) are shuffled, where 'end' == size() means to
  // shuffle to the end of the Vector.
  void ShuffleRange(internal::Random* random, int begin, int end) {
    GTEST_CHECK_(0 <= begin && begin <= size_)
        << "Invalid shuffle range start " << begin << ": must be in range [0, "
        << size_ << "].";
    GTEST_CHECK_(begin <= end && end <= size_)
        << "Invalid shuffle range finish " << end << ": must be in range ["
        << begin << ", " << size_ << "].";

    // Fisher-Yates shuffle, from
    // http://en.wikipedia.org/wiki/Fisher-Yates_shuffle
    for (int range_width = end - begin; range_width >= 2; range_width--) {
      const int last_in_range = begin + range_width - 1;
      const int selected = begin + random->Generate(range_width);
      Swap(selected, last_in_range);
    }
  }

  // Performs an in-place shuffle of this Vector's nodes.
  void Shuffle(internal::Random* random) {
    ShuffleRange(random, 0, size());
  }

  // Returns a copy of this Vector.
  Vector* Clone() const {
    Vector* const clone = new Vector;
    clone->Reserve(size_);
    for (int i = 0; i < size_; i++) {
      clone->PushBack(GetElement(i));
    }
    return clone;
  }

 private:
  // Makes sure this Vector's capacity is at least the given value.
  void Reserve(int new_capacity) {
    if (new_capacity <= capacity_)
      return;

    capacity_ = new_capacity;
    elements_ = static_cast<E**>(
        realloc(elements_, capacity_*sizeof(elements_[0])));
  }

  // Grows the buffer if it is not big enough to hold one more element.
  void GrowIfNeeded() {
    if (size_ < capacity_)
      return;

    // Exponential bump-up is necessary to ensure that inserting N
    // elements is O(N) instead of O(N^2).  The factor 3/2 means that
    // no more than 1/3 of the slots are wasted.
    const int new_capacity = 3*(capacity_/2 + 1);
    GTEST_CHECK_(new_capacity > capacity_)  // Does the new capacity overflow?
        << "Cannot grow a Vector with " << capacity_ << " elements already.";
    Reserve(new_capacity);
  }

  // Moves the give consecutive elements to a new index in the Vector.
  void MoveElements(int source, int count, int dest) {
    memmove(elements_ + dest, elements_ + source, count*sizeof(elements_[0]));
  }

  E** elements_;
  int capacity_;  // The number of elements allocated for elements_.
  int size_;      // The number of elements; in the range [0, capacity_].

  // We disallow copying Vector.
  GTEST_DISALLOW_COPY_AND_ASSIGN_(Vector);
};  // class Vector

// A function for deleting an object.  Handy for being used as a
// functor.
template <typename T>
static void Delete(T * x) {
  delete x;
}

// A predicate that checks the key of a TestProperty against a known key.
//
// TestPropertyKeyIs is copyable.
class TestPropertyKeyIs {
 public:
  // Constructor.
  //
  // TestPropertyKeyIs has NO default constructor.
  explicit TestPropertyKeyIs(const char* key)
      : key_(key) {}

  // Returns true iff the test name of test property matches on key_.
  bool operator()(const TestProperty& test_property) const {
    return String(test_property.key()).Compare(key_) == 0;
  }

 private:
  String key_;
};

class TestInfoImpl {
 public:
  TestInfoImpl(TestInfo* parent, const char* test_case_name,
               const char* name, const char* test_case_comment,
               const char* comment, TypeId fixture_class_id,
               internal::TestFactoryBase* factory);
  ~TestInfoImpl();

  // Returns true if this test should run.
  bool should_run() const { return should_run_; }

  // Sets the should_run member.
  void set_should_run(bool should) { should_run_ = should; }

  // Returns true if this test is disabled. Disabled tests are not run.
  bool is_disabled() const { return is_disabled_; }

  // Sets the is_disabled member.
  void set_is_disabled(bool is) { is_disabled_ = is; }

  // Returns true if this test matches the filter specified by the user.
  bool matches_filter() const { return matches_filter_; }

  // Sets the matches_filter member.
  void set_matches_filter(bool matches) { matches_filter_ = matches; }

  // Returns the test case name.
  const char* test_case_name() const { return test_case_name_.c_str(); }

  // Returns the test name.
  const char* name() const { return name_.c_str(); }

  // Returns the test case comment.
  const char* test_case_comment() const { return test_case_comment_.c_str(); }

  // Returns the test comment.
  const char* comment() const { return comment_.c_str(); }

  // Returns the ID of the test fixture class.
  TypeId fixture_class_id() const { return fixture_class_id_; }

  // Returns the test result.
  TestResult* result() { return &result_; }
  const TestResult* result() const { return &result_; }

  // Creates the test object, runs it, records its result, and then
  // deletes it.
  void Run();

  // Clears the test result.
  void ClearResult() { result_.Clear(); }

  // Clears the test result in the given TestInfo object.
  static void ClearTestResult(TestInfo * test_info) {
    test_info->impl()->ClearResult();
  }

 private:
  // These fields are immutable properties of the test.
  TestInfo* const parent_;          // The owner of this object
  const String test_case_name_;     // Test case name
  const String name_;               // Test name
  const String test_case_comment_;  // Test case comment
  const String comment_;            // Test comment
  const TypeId fixture_class_id_;   // ID of the test fixture class
  bool should_run_;                 // True iff this test should run
  bool is_disabled_;                // True iff this test is disabled
  bool matches_filter_;             // True if this test matches the
                                    // user-specified filter.
  internal::TestFactoryBase* const factory_;  // The factory that creates
                                              // the test object

  // This field is mutable and needs to be reset before running the
  // test for the second time.
  TestResult result_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(TestInfoImpl);
};

// Class UnitTestOptions.
//
// This class contains functions for processing options the user
// specifies when running the tests.  It has only static members.
//
// In most cases, the user can specify an option using either an
// environment variable or a command line flag.  E.g. you can set the
// test filter using either GTEST_FILTER or --gtest_filter.  If both
// the variable and the flag are present, the latter overrides the
// former.
class UnitTestOptions {
 public:
  // Functions for processing the gtest_output flag.

  // Returns the output format, or "" for normal printed output.
  static String GetOutputFormat();

  // Returns the absolute path of the requested output file, or the
  // default (test_detail.xml in the original working directory) if
  // none was explicitly specified.
  static String GetAbsolutePathToOutputFile();

  // Functions for processing the gtest_filter flag.

  // Returns true iff the wildcard pattern matches the string.  The
  // first ':' or '\0' character in pattern marks the end of it.
  //
  // This recursive algorithm isn't very efficient, but is clear and
  // works well enough for matching test names, which are short.
  static bool PatternMatchesString(const char *pattern, const char *str);

  // Returns true iff the user-specified filter matches the test case
  // name and the test name.
  static bool FilterMatchesTest(const String &test_case_name,
                                const String &test_name);

#if GTEST_OS_WINDOWS
  // Function for supporting the gtest_catch_exception flag.

  // Returns EXCEPTION_EXECUTE_HANDLER if Google Test should handle the
  // given SEH exception, or EXCEPTION_CONTINUE_SEARCH otherwise.
  // This function is useful as an __except condition.
  static int GTestShouldProcessSEH(DWORD exception_code);
#endif  // GTEST_OS_WINDOWS

  // Returns true if "name" matches the ':' separated list of glob-style
  // filters in "filter".
  static bool MatchesFilter(const String& name, const char* filter);
};

// Returns the current application's name, removing directory path if that
// is present.  Used by UnitTestOptions::GetOutputFile.
FilePath GetCurrentExecutableName();

// The role interface for getting the OS stack trace as a string.
class OsStackTraceGetterInterface {
 public:
  OsStackTraceGetterInterface() {}
  virtual ~OsStackTraceGetterInterface() {}

  // Returns the current OS stack trace as a String.  Parameters:
  //
  //   max_depth  - the maximum number of stack frames to be included
  //                in the trace.
  //   skip_count - the number of top frames to be skipped; doesn't count
  //                against max_depth.
  virtual String CurrentStackTrace(int max_depth, int skip_count) = 0;

  // UponLeavingGTest() should be called immediately before Google Test calls
  // user code. It saves some information about the current stack that
  // CurrentStackTrace() will use to find and hide Google Test stack frames.
  virtual void UponLeavingGTest() = 0;

 private:
  GTEST_DISALLOW_COPY_AND_ASSIGN_(OsStackTraceGetterInterface);
};

// A working implementation of the OsStackTraceGetterInterface interface.
class OsStackTraceGetter : public OsStackTraceGetterInterface {
 public:
  OsStackTraceGetter() : caller_frame_(NULL) {}
  virtual String CurrentStackTrace(int max_depth, int skip_count);
  virtual void UponLeavingGTest();

  // This string is inserted in place of stack frames that are part of
  // Google Test's implementation.
  static const char* const kElidedFramesMarker;

 private:
  Mutex mutex_;  // protects all internal state

  // We save the stack frame below the frame that calls user code.
  // We do this because the address of the frame immediately below
  // the user code changes between the call to UponLeavingGTest()
  // and any calls to CurrentStackTrace() from within the user code.
  void* caller_frame_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(OsStackTraceGetter);
};

// Information about a Google Test trace point.
struct TraceInfo {
  const char* file;
  int line;
  String message;
};

// This is the default global test part result reporter used in UnitTestImpl.
// This class should only be used by UnitTestImpl.
class DefaultGlobalTestPartResultReporter
  : public TestPartResultReporterInterface {
 public:
  explicit DefaultGlobalTestPartResultReporter(UnitTestImpl* unit_test);
  // Implements the TestPartResultReporterInterface. Reports the test part
  // result in the current test.
  virtual void ReportTestPartResult(const TestPartResult& result);

 private:
  UnitTestImpl* const unit_test_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(DefaultGlobalTestPartResultReporter);
};

// This is the default per thread test part result reporter used in
// UnitTestImpl. This class should only be used by UnitTestImpl.
class DefaultPerThreadTestPartResultReporter
    : public TestPartResultReporterInterface {
 public:
  explicit DefaultPerThreadTestPartResultReporter(UnitTestImpl* unit_test);
  // Implements the TestPartResultReporterInterface. The implementation just
  // delegates to the current global test part result reporter of *unit_test_.
  virtual void ReportTestPartResult(const TestPartResult& result);

 private:
  UnitTestImpl* const unit_test_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(DefaultPerThreadTestPartResultReporter);
};

// The private implementation of the UnitTest class.  We don't protect
// the methods under a mutex, as this class is not accessible by a
// user and the UnitTest class that delegates work to this class does
// proper locking.
class UnitTestImpl {
 public:
  explicit UnitTestImpl(UnitTest* parent);
  virtual ~UnitTestImpl();

  // There are two different ways to register your own TestPartResultReporter.
  // You can register your own repoter to listen either only for test results
  // from the current thread or for results from all threads.
  // By default, each per-thread test result repoter just passes a new
  // TestPartResult to the global test result reporter, which registers the
  // test part result for the currently running test.

  // Returns the global test part result reporter.
  TestPartResultReporterInterface* GetGlobalTestPartResultReporter();

  // Sets the global test part result reporter.
  void SetGlobalTestPartResultReporter(
      TestPartResultReporterInterface* reporter);

  // Returns the test part result reporter for the current thread.
  TestPartResultReporterInterface* GetTestPartResultReporterForCurrentThread();

  // Sets the test part result reporter for the current thread.
  void SetTestPartResultReporterForCurrentThread(
      TestPartResultReporterInterface* reporter);

  // Gets the number of successful test cases.
  int successful_test_case_count() const;

  // Gets the number of failed test cases.
  int failed_test_case_count() const;

  // Gets the number of all test cases.
  int total_test_case_count() const;

  // Gets the number of all test cases that contain at least one test
  // that should run.
  int test_case_to_run_count() const;

  // Gets the number of successful tests.
  int successful_test_count() const;

  // Gets the number of failed tests.
  int failed_test_count() const;

  // Gets the number of disabled tests.
  int disabled_test_count() const;

  // Gets the number of all tests.
  int total_test_count() const;

  // Gets the number of tests that should run.
  int test_to_run_count() const;

  // Gets the elapsed time, in milliseconds.
  TimeInMillis elapsed_time() const { return elapsed_time_; }

  // Returns true iff the unit test passed (i.e. all test cases passed).
  bool Passed() const { return !Failed(); }

  // Returns true iff the unit test failed (i.e. some test case failed
  // or something outside of all tests failed).
  bool Failed() const {
    return failed_test_case_count() > 0 || ad_hoc_test_result()->Failed();
  }

  // Gets the i-th test case among all the test cases. i can range from 0 to
  // total_test_case_count() - 1. If i is not in that range, returns NULL.
  const TestCase* GetTestCase(int i) const {
    const int index = test_case_indices_.GetElementOr(i, -1);
    return index < 0 ? NULL : test_cases_.GetElement(i);
  }

  // Gets the i-th test case among all the test cases. i can range from 0 to
  // total_test_case_count() - 1. If i is not in that range, returns NULL.
  TestCase* GetMutableTestCase(int i) {
    const int index = test_case_indices_.GetElementOr(i, -1);
    return index < 0 ? NULL : test_cases_.GetElement(index);
  }

  // Provides access to the event listener list.
  TestEventListeners* listeners() { return &listeners_; }

  // Returns the TestResult for the test that's currently running, or
  // the TestResult for the ad hoc test if no test is running.
  TestResult* current_test_result();

  // Returns the TestResult for the ad hoc test.
  const TestResult* ad_hoc_test_result() const { return &ad_hoc_test_result_; }

  // Sets the OS stack trace getter.
  //
  // Does nothing if the input and the current OS stack trace getter
  // are the same; otherwise, deletes the old getter and makes the
  // input the current getter.
  void set_os_stack_trace_getter(OsStackTraceGetterInterface* getter);

  // Returns the current OS stack trace getter if it is not NULL;
  // otherwise, creates an OsStackTraceGetter, makes it the current
  // getter, and returns it.
  OsStackTraceGetterInterface* os_stack_trace_getter();

  // Returns the current OS stack trace as a String.
  //
  // The maximum number of stack frames to be included is specified by
  // the gtest_stack_trace_depth flag.  The skip_count parameter
  // specifies the number of top frames to be skipped, which doesn't
  // count against the number of frames to be included.
  //
  // For example, if Foo() calls Bar(), which in turn calls
  // CurrentOsStackTraceExceptTop(1), Foo() will be included in the
  // trace but Bar() and CurrentOsStackTraceExceptTop() won't.
  String CurrentOsStackTraceExceptTop(int skip_count);

  // Finds and returns a TestCase with the given name.  If one doesn't
  // exist, creates one and returns it.
  //
  // Arguments:
  //
  //   test_case_name: name of the test case
  //   set_up_tc:      pointer to the function that sets up the test case
  //   tear_down_tc:   pointer to the function that tears down the test case
  TestCase* GetTestCase(const char* test_case_name,
                        const char* comment,
                        Test::SetUpTestCaseFunc set_up_tc,
                        Test::TearDownTestCaseFunc tear_down_tc);

  // Adds a TestInfo to the unit test.
  //
  // Arguments:
  //
  //   set_up_tc:    pointer to the function that sets up the test case
  //   tear_down_tc: pointer to the function that tears down the test case
  //   test_info:    the TestInfo object
  void AddTestInfo(Test::SetUpTestCaseFunc set_up_tc,
                   Test::TearDownTestCaseFunc tear_down_tc,
                   TestInfo * test_info) {
    // In order to support thread-safe death tests, we need to
    // remember the original working directory when the test program
    // was first invoked.  We cannot do this in RUN_ALL_TESTS(), as
    // the user may have changed the current directory before calling
    // RUN_ALL_TESTS().  Therefore we capture the current directory in
    // AddTestInfo(), which is called to register a TEST or TEST_F
    // before main() is reached.
    if (original_working_dir_.IsEmpty()) {
      original_working_dir_.Set(FilePath::GetCurrentDir());
      GTEST_CHECK_(!original_working_dir_.IsEmpty())
          << "Failed to get the current working directory.";
    }

    GetTestCase(test_info->test_case_name(),
                test_info->test_case_comment(),
                set_up_tc,
                tear_down_tc)->AddTestInfo(test_info);
  }

#if GTEST_HAS_PARAM_TEST
  // Returns ParameterizedTestCaseRegistry object used to keep track of
  // value-parameterized tests and instantiate and register them.
  internal::ParameterizedTestCaseRegistry& parameterized_test_registry() {
    return parameterized_test_registry_;
  }
#endif  // GTEST_HAS_PARAM_TEST

  // Sets the TestCase object for the test that's currently running.
  void set_current_test_case(TestCase* current_test_case) {
    current_test_case_ = current_test_case;
  }

  // Sets the TestInfo object for the test that's currently running.  If
  // current_test_info is NULL, the assertion results will be stored in
  // ad_hoc_test_result_.
  void set_current_test_info(TestInfo* current_test_info) {
    current_test_info_ = current_test_info;
  }

  // Registers all parameterized tests defined using TEST_P and
  // INSTANTIATE_TEST_P, creating regular tests for each test/parameter
  // combination. This method can be called more then once; it has
  // guards protecting from registering the tests more then once.
  // If value-parameterized tests are disabled, RegisterParameterizedTests
  // is present but does nothing.
  void RegisterParameterizedTests();

  // Runs all tests in this UnitTest object, prints the result, and
  // returns 0 if all tests are successful, or 1 otherwise.  If any
  // exception is thrown during a test on Windows, this test is
  // considered to be failed, but the rest of the tests will still be
  // run.  (We disable exceptions on Linux and Mac OS X, so the issue
  // doesn't apply there.)
  int RunAllTests();

  // Clears the results of all tests, including the ad hoc test.
  void ClearResult() {
    test_cases_.ForEach(TestCase::ClearTestCaseResult);
    ad_hoc_test_result_.Clear();
  }

  enum ReactionToSharding {
    HONOR_SHARDING_PROTOCOL,
    IGNORE_SHARDING_PROTOCOL
  };

  // Matches the full name of each test against the user-specified
  // filter to decide whether the test should run, then records the
  // result in each TestCase and TestInfo object.
  // If shard_tests == HONOR_SHARDING_PROTOCOL, further filters tests
  // based on sharding variables in the environment.
  // Returns the number of tests that should run.
  int FilterTests(ReactionToSharding shard_tests);

  // Prints the names of the tests matching the user-specified filter flag.
  void ListTestsMatchingFilter();

  const TestCase* current_test_case() const { return current_test_case_; }
  TestInfo* current_test_info() { return current_test_info_; }
  const TestInfo* current_test_info() const { return current_test_info_; }

  // Returns the vector of environments that need to be set-up/torn-down
  // before/after the tests are run.
  internal::Vector<Environment*>* environments() { return &environments_; }
  internal::Vector<Environment*>* environments_in_reverse_order() {
    return &environments_in_reverse_order_;
  }

  // Getters for the per-thread Google Test trace stack.
  internal::Vector<TraceInfo>* gtest_trace_stack() {
    return gtest_trace_stack_.pointer();
  }
  const internal::Vector<TraceInfo>* gtest_trace_stack() const {
    return gtest_trace_stack_.pointer();
  }

#if GTEST_HAS_DEATH_TEST
  void InitDeathTestSubprocessControlInfo() {
    internal_run_death_test_flag_.reset(ParseInternalRunDeathTestFlag());
  }
  // Returns a pointer to the parsed --gtest_internal_run_death_test
  // flag, or NULL if that flag was not specified.
  // This information is useful only in a death test child process.
  // Must not be called before a call to InitGoogleTest.
  const InternalRunDeathTestFlag* internal_run_death_test_flag() const {
    return internal_run_death_test_flag_.get();
  }

  // Returns a pointer to the current death test factory.
  internal::DeathTestFactory* death_test_factory() {
    return death_test_factory_.get();
  }

  void SuppressTestEventsIfInSubprocess();

  friend class ReplaceDeathTestFactory;
#endif  // GTEST_HAS_DEATH_TEST

  // Initializes the event listener performing XML output as specified by
  // UnitTestOptions. Must not be called before InitGoogleTest.
  void ConfigureXmlOutput();

  // Performs initialization dependent upon flag values obtained in
  // ParseGoogleTestFlagsOnly.  Is called from InitGoogleTest after the call to
  // ParseGoogleTestFlagsOnly.  In case a user neglects to call InitGoogleTest
  // this function is also called from RunAllTests.  Since this function can be
  // called more than once, it has to be idempotent.
  void PostFlagParsingInit();

  // Gets the random seed used at the start of the current test iteration.
  int random_seed() const { return random_seed_; }

  // Gets the random number generator.
  internal::Random* random() { return &random_; }

  // Shuffles all test cases, and the tests within each test case,
  // making sure that death tests are still run first.
  void ShuffleTests();

  // Restores the test cases and tests to their order before the first shuffle.
  void UnshuffleTests();

 private:
  friend class ::testing::UnitTest;

  // The UnitTest object that owns this implementation object.
  UnitTest* const parent_;

  // The working directory when the first TEST() or TEST_F() was
  // executed.
  internal::FilePath original_working_dir_;

  // The default test part result reporters.
  DefaultGlobalTestPartResultReporter default_global_test_part_result_reporter_;
  DefaultPerThreadTestPartResultReporter
      default_per_thread_test_part_result_reporter_;

  // Points to (but doesn't own) the global test part result reporter.
  TestPartResultReporterInterface* global_test_part_result_repoter_;

  // Protects read and write access to global_test_part_result_reporter_.
  internal::Mutex global_test_part_result_reporter_mutex_;

  // Points to (but doesn't own) the per-thread test part result reporter.
  internal::ThreadLocal<TestPartResultReporterInterface*>
      per_thread_test_part_result_reporter_;

  // The vector of environments that need to be set-up/torn-down
  // before/after the tests are run.  environments_in_reverse_order_
  // simply mirrors environments_ in reverse order.
  internal::Vector<Environment*> environments_;
  internal::Vector<Environment*> environments_in_reverse_order_;

  // The vector of TestCases in their original order.  It owns the
  // elements in the vector.
  internal::Vector<TestCase*> test_cases_;

  // Provides a level of indirection for the test case list to allow
  // easy shuffling and restoring the test case order.  The i-th
  // element of this vector is the index of the i-th test case in the
  // shuffled order.
  internal::Vector<int> test_case_indices_;

#if GTEST_HAS_PARAM_TEST
  // ParameterizedTestRegistry object used to register value-parameterized
  // tests.
  internal::ParameterizedTestCaseRegistry parameterized_test_registry_;

  // Indicates whether RegisterParameterizedTests() has been called already.
  bool parameterized_tests_registered_;
#endif  // GTEST_HAS_PARAM_TEST

  // Index of the last death test case registered.  Initially -1.
  int last_death_test_case_;

  // This points to the TestCase for the currently running test.  It
  // changes as Google Test goes through one test case after another.
  // When no test is running, this is set to NULL and Google Test
  // stores assertion results in ad_hoc_test_result_.  Initially NULL.
  TestCase* current_test_case_;

  // This points to the TestInfo for the currently running test.  It
  // changes as Google Test goes through one test after another.  When
  // no test is running, this is set to NULL and Google Test stores
  // assertion results in ad_hoc_test_result_.  Initially NULL.
  TestInfo* current_test_info_;

  // Normally, a user only writes assertions inside a TEST or TEST_F,
  // or inside a function called by a TEST or TEST_F.  Since Google
  // Test keeps track of which test is current running, it can
  // associate such an assertion with the test it belongs to.
  //
  // If an assertion is encountered when no TEST or TEST_F is running,
  // Google Test attributes the assertion result to an imaginary "ad hoc"
  // test, and records the result in ad_hoc_test_result_.
  TestResult ad_hoc_test_result_;

  // The list of event listeners that can be used to track events inside
  // Google Test.
  TestEventListeners listeners_;

  // The OS stack trace getter.  Will be deleted when the UnitTest
  // object is destructed.  By default, an OsStackTraceGetter is used,
  // but the user can set this field to use a custom getter if that is
  // desired.
  OsStackTraceGetterInterface* os_stack_trace_getter_;

  // True iff PostFlagParsingInit() has been called.
  bool post_flag_parse_init_performed_;

  // The random number seed used at the beginning of the test run.
  int random_seed_;

  // Our random number generator.
  internal::Random random_;

  // How long the test took to run, in milliseconds.
  TimeInMillis elapsed_time_;

#if GTEST_HAS_DEATH_TEST
  // The decomposed components of the gtest_internal_run_death_test flag,
  // parsed when RUN_ALL_TESTS is called.
  internal::scoped_ptr<InternalRunDeathTestFlag> internal_run_death_test_flag_;
  internal::scoped_ptr<internal::DeathTestFactory> death_test_factory_;
#endif  // GTEST_HAS_DEATH_TEST

  // A per-thread stack of traces created by the SCOPED_TRACE() macro.
  internal::ThreadLocal<internal::Vector<TraceInfo> > gtest_trace_stack_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(UnitTestImpl);
};  // class UnitTestImpl

// Convenience function for accessing the global UnitTest
// implementation object.
inline UnitTestImpl* GetUnitTestImpl() {
  return UnitTest::GetInstance()->impl();
}

// Internal helper functions for implementing the simple regular
// expression matcher.
bool IsInSet(char ch, const char* str);
bool IsDigit(char ch);
bool IsPunct(char ch);
bool IsRepeat(char ch);
bool IsWhiteSpace(char ch);
bool IsWordChar(char ch);
bool IsValidEscape(char ch);
bool AtomMatchesChar(bool escaped, char pattern, char ch);
bool ValidateRegex(const char* regex);
bool MatchRegexAtHead(const char* regex, const char* str);
bool MatchRepetitionAndRegexAtHead(
    bool escaped, char ch, char repeat, const char* regex, const char* str);
bool MatchRegexAnywhere(const char* regex, const char* str);

// Parses the command line for Google Test flags, without initializing
// other parts of Google Test.
void ParseGoogleTestFlagsOnly(int* argc, char** argv);
void ParseGoogleTestFlagsOnly(int* argc, wchar_t** argv);

#if GTEST_HAS_DEATH_TEST

// Returns the message describing the last system error, regardless of the
// platform.
String GetLastErrnoDescription();

#if GTEST_OS_WINDOWS
// Provides leak-safe Windows kernel handle ownership.
class AutoHandle {
 public:
  AutoHandle() : handle_(INVALID_HANDLE_VALUE) {}
  explicit AutoHandle(HANDLE handle) : handle_(handle) {}

  ~AutoHandle() { Reset(); }

  HANDLE Get() const { return handle_; }
  void Reset() { Reset(INVALID_HANDLE_VALUE); }
  void Reset(HANDLE handle) {
    if (handle != handle_) {
      if (handle_ != INVALID_HANDLE_VALUE)
        ::CloseHandle(handle_);
      handle_ = handle;
    }
  }

 private:
  HANDLE handle_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(AutoHandle);
};
#endif  // GTEST_OS_WINDOWS

// Attempts to parse a string into a positive integer pointed to by the
// number parameter.  Returns true if that is possible.
// GTEST_HAS_DEATH_TEST implies that we have ::std::string, so we can use
// it here.
template <typename Integer>
bool ParseNaturalNumber(const ::std::string& str, Integer* number) {
  // Fail fast if the given string does not begin with a digit;
  // this bypasses strtoXXX's "optional leading whitespace and plus
  // or minus sign" semantics, which are undesirable here.
  if (str.empty() || !isdigit(str[0])) {
    return false;
  }
  errno = 0;

  char* end;
  // BiggestConvertible is the largest integer type that system-provided
  // string-to-number conversion routines can return.
#if GTEST_OS_WINDOWS && !defined(__GNU_C__)
  // MSVC and C++ Builder define __int64 instead of the standard long long.
  typedef unsigned __int64 BiggestConvertible;
  const BiggestConvertible parsed = _strtoui64(str.c_str(), &end, 10);
#else
  typedef unsigned long long BiggestConvertible;  // NOLINT
  const BiggestConvertible parsed = strtoull(str.c_str(), &end, 10);
#endif  // GTEST_OS_WINDOWS && !defined(__GNU_C__)
  const bool parse_success = *end == '\0' && errno == 0;

  // TODO(vladl@google.com): Convert this to compile time assertion when it is
  // available.
  GTEST_CHECK_(sizeof(Integer) <= sizeof(parsed));

  const Integer result = static_cast<Integer>(parsed);
  if (parse_success && static_cast<BiggestConvertible>(result) == parsed) {
    *number = result;
    return true;
  }
  return false;
}
#endif  // GTEST_HAS_DEATH_TEST

// TestResult contains some private methods that should be hidden from
// Google Test user but are required for testing. This class allow our tests
// to access them.
class TestResultAccessor {
 public:
  static void RecordProperty(TestResult* test_result,
                             const TestProperty& property) {
    test_result->RecordProperty(property);
  }

  static void ClearTestPartResults(TestResult* test_result) {
    test_result->ClearTestPartResults();
  }

  static const Vector<testing::TestPartResult>& test_part_results(
      const TestResult& test_result) {
    return test_result.test_part_results();
  }
};

}  // namespace internal
}  // namespace testing

#endif  // GTEST_SRC_GTEST_INTERNAL_INL_H_
