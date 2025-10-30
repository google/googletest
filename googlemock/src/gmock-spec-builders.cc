// Copyright 2007, Google Inc.
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

// Google Mock - a framework for writing C++ mock classes.
//
// This file implements the spec builder syntax (ON_CALL and
// EXPECT_CALL).

#include "gmock/gmock-spec-builders.h"

#include <stdlib.h>

#include <iostream>  // NOLINT
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "gtest/internal/gtest-port.h"

#if defined(GTEST_OS_CYGWIN) || defined(GTEST_OS_LINUX) || defined(GTEST_OS_MAC)
#include <unistd.h>  // NOLINT
#endif
#ifdef GTEST_OS_QURT
#include <qurt_event.h>
#endif

// Silence C4800 (C4800: 'int *const ': forcing value
// to bool 'true' or 'false') for MSVC 15
#if defined(_MSC_VER) && (_MSC_VER == 1900)
GTEST_DISABLE_MSC_WARNINGS_PUSH_(4800)
#endif

namespace testing {
namespace internal {

// Protects the mock object registry (in class Mock), all function
// mockers, and all expectations.
GTEST_API_ GTEST_DEFINE_STATIC_MUTEX_(g_gmock_mutex);

// Logs a message including file and line number information.
GTEST_API_ void LogWithLocation(testing::internal::LogSeverity severity,
                                const char* file, int line,
                                const std::string& message) {
  ::std::ostringstream s;
  s << internal::FormatFileLocation(file, line) << " " << message
    << ::std::endl;
  Log(severity, s.str(), 0);
}

// Constructs an ExpectationBase object.
ExpectationBase::ExpectationBase(const char* a_file, int a_line,
                                 const std::string& a_source_text)
    : file_(a_file),
      line_(a_line),
      source_text_(a_source_text),
      cardinality_specified_(false),
      cardinality_(Exactly(1)),
      call_count_(0),
      retired_(false),
      extra_matcher_specified_(false),
      repeated_action_specified_(false),
      retires_on_saturation_(false),
      last_clause_(kNone),
      action_count_checked_(false) {}

// Destructs an ExpectationBase object.
ExpectationBase::~ExpectationBase() = default;

// Explicitly specifies the cardinality of this expectation.  Used by
// the subclasses to implement the .Times() clause.
void ExpectationBase::SpecifyCardinality(const Cardinality& a_cardinality) {
  cardinality_specified_ = true;
  cardinality_ = a_cardinality;
}

// Retires all pre-requisites of this expectation.
void ExpectationBase::RetireAllPreRequisites()
    GTEST_EXCLUSIVE_LOCK_REQUIRED_(g_gmock_mutex) {
  if (is_retired()) {
    // We can take this short-cut as we never retire an expectation
    // until we have retired all its pre-requisites.
    return;
  }

  ::std::vector<ExpectationBase*> expectations(1, this);
  while (!expectations.empty()) {
    ExpectationBase* exp = expectations.back();
    expectations.pop_back();

    for (ExpectationSet::const_iterator it =
             exp->immediate_prerequisites_.begin();
         it != exp->immediate_prerequisites_.end(); ++it) {
      ExpectationBase* next = it->expectation_base().get();
      if (!next->is_retired()) {
        next->Retire();
        expectations.push_back(next);
      }
    }
  }
}

// Returns true if and only if all pre-requisites of this expectation
// have been satisfied.
bool ExpectationBase::AllPrerequisitesAreSatisfied() const
    GTEST_EXCLUSIVE_LOCK_REQUIRED_(g_gmock_mutex) {
  g_gmock_mutex.AssertHeld();
  ::std::vector<const ExpectationBase*> expectations(1, this);
  while (!expectations.empty()) {
    const ExpectationBase* exp = expectations.back();
    expectations.pop_back();

    for (ExpectationSet::const_iterator it =
             exp->immediate_prerequisites_.begin();
         it != exp->immediate_prerequisites_.end(); ++it) {
      const ExpectationBase* next = it->expectation_base().get();
      if (!next->IsSatisfied()) return false;
      expectations.push_back(next);
    }
  }
  return true;
}

// Adds unsatisfied pre-requisites of this expectation to 'result'.
void ExpectationBase::FindUnsatisfiedPrerequisites(ExpectationSet* result) const
    GTEST_EXCLUSIVE_LOCK_REQUIRED_(g_gmock_mutex) {
  g_gmock_mutex.AssertHeld();
  ::std::vector<const ExpectationBase*> expectations(1, this);
  while (!expectations.empty()) {
    const ExpectationBase* exp = expectations.back();
    expectations.pop_back();

    for (ExpectationSet::const_iterator it =
             exp->immediate_prerequisites_.begin();
         it != exp->immediate_prerequisites_.end(); ++it) {
      const ExpectationBase* next = it->expectation_base().get();

      if (next->IsSatisfied()) {
        // If *it is satisfied and has a call count of 0, some of its
        // pre-requisites may not be satisfied yet.
        if (next->call_count_ == 0) {
          expectations.push_back(next);
        }
      } else {
        // Now that we know next is unsatisfied, we are not so interested
        // in whether its pre-requisites are satisfied.  Therefore we
        // don't iterate into it here.
        *result += *it;
      }
    }
  }
}

// Describes how many times a function call matching this
// expectation has occurred.
void ExpectationBase::DescribeCallCountTo(::std::ostream* os) const
    GTEST_EXCLUSIVE_LOCK_REQUIRED_(g_gmock_mutex) {
  g_gmock_mutex.AssertHeld();

  // Describes how many times the function is expected to be called.
  *os << "         Expected: to be ";
  cardinality().DescribeTo(os);
  *os << "\n           Actual: ";
  Cardinality::DescribeActualCallCountTo(call_count(), os);

  // Describes the state of the expectation (e.g. is it satisfied?
  // is it active?).
  *os << " - "
      << (IsOverSaturated() ? "over-saturated"
          : IsSaturated()   ? "saturated"
          : IsSatisfied()   ? "satisfied"
                            : "unsatisfied")
      << " and " << (is_retired() ? "retired" : "active");
}

// Checks the action count (i.e. the number of WillOnce() and
// WillRepeatedly() clauses) against the cardinality if this hasn't
// been done before.  Prints a warning if there are too many or too
// few actions.
void ExpectationBase::CheckActionCountIfNotDone() const
    GTEST_LOCK_EXCLUDED_(mutex_) {
  bool should_check = false;
  {
    MutexLock l(&mutex_);
    if (!action_count_checked_) {
      action_count_checked_ = true;
      should_check = true;
    }
  }

  if (should_check) {
    if (!cardinality_specified_) {
      // The cardinality was inferred - no need to check the action
      // count against it.
      return;
    }

    // The cardinality was explicitly specified.
    const int action_count = static_cast<int>(untyped_actions_.size());
    const int upper_bound = cardinality().ConservativeUpperBound();
    const int lower_bound = cardinality().ConservativeLowerBound();
    bool too_many;  // True if there are too many actions, or false
    // if there are too few.
    if (action_count > upper_bound ||
        (action_count == upper_bound && repeated_action_specified_)) {
      too_many = true;
    } else if (0 < action_count && action_count < lower_bound &&
               !repeated_action_specified_) {
      too_many = false;
    } else {
      return;
    }

    ::std::stringstream ss;
    DescribeLocationTo(&ss);
    ss << "Too " << (too_many ? "many" : "few") << " actions specified in "
       << source_text() << "...\n"
       << "Expected to be ";
    cardinality().DescribeTo(&ss);
    ss << ", but has " << (too_many ? "" : "only ") << action_count
       << " WillOnce()" << (action_count == 1 ? "" : "s");
    if (repeated_action_specified_) {
      ss << " and a WillRepeatedly()";
    }
    ss << ".";
    Log(kWarning, ss.str(), -1);  // -1 means "don't print stack trace".
  }
}

// Implements the .Times() clause.
void ExpectationBase::UntypedTimes(const Cardinality& a_cardinality) {
  if (last_clause_ == kTimes) {
    ExpectSpecProperty(false,
                       ".Times() cannot appear "
                       "more than once in an EXPECT_CALL().");
  } else {
    ExpectSpecProperty(
        last_clause_ < kTimes,
        ".Times() may only appear *before* .InSequence(), .WillOnce(), "
        ".WillRepeatedly(), or .RetiresOnSaturation(), not after.");
  }
  last_clause_ = kTimes;

  SpecifyCardinality(a_cardinality);
}

// Points to the implicit sequence introduced by a living InSequence
// object (if any) in the current thread or NULL.
GTEST_API_ ThreadLocal<Sequence*> g_gmock_implicit_sequence;

// Reports an uninteresting call (whose description is in msg) in the
// manner specified by 'reaction'.
void ReportUninterestingCall(CallReaction reaction, const std::string& msg) {
  // Include a stack trace only if --gmock_verbose=info is specified.
  const int stack_frames_to_skip =
      GMOCK_FLAG_GET(verbose) == kInfoVerbosity ? 3 : -1;
  switch (reaction) {
    case kAllow:
      Log(kInfo, msg, stack_frames_to_skip);
      break;
    case kWarn:
      Log(kWarning,
          msg +
              "\nNOTE: You can safely ignore the above warning unless this "
              "call should not happen.  Do not suppress it by adding "
              "an EXPECT_CALL() if you don't mean to enforce the call.  "
              "See "
              "https://github.com/google/googletest/blob/main/docs/"
              "gmock_cook_book.md#"
              "knowing-when-to-expect-useoncall for details.\n",
          stack_frames_to_skip);
      break;
    default:  // FAIL
      Expect(false, nullptr, -1, msg);
  }
}

UntypedFunctionMockerBase::UntypedFunctionMockerBase()
    : mock_obj_(nullptr), name_("") {}

UntypedFunctionMockerBase::~UntypedFunctionMockerBase() = default;

// Sets the mock object this mock method belongs to, and registers
// this information in the global mock registry.  Will be called
// whenever an EXPECT_CALL() or ON_CALL() is executed on this mock
// method.
void UntypedFunctionMockerBase::RegisterOwner(const void* mock_obj)
    GTEST_LOCK_EXCLUDED_(g_gmock_mutex) {
  {
    MutexLock l(&g_gmock_mutex);
    mock_obj_ = mock_obj;
  }
  Mock::Register(mock_obj, this);
}

// Sets the mock object this mock method belongs to, and sets the name
// of the mock function.  Will be called upon each invocation of this
// mock function.
void UntypedFunctionMockerBase::SetOwnerAndName(const void* mock_obj,
                                                const char* name)
    GTEST_LOCK_EXCLUDED_(g_gmock_mutex) {
  // We protect name_ under g_gmock_mutex in case this mock function
  // is called from two threads concurrently.
  MutexLock l(&g_gmock_mutex);
  mock_obj_ = mock_obj;
  name_ = name;
}

// Returns the name of the function being mocked.  Must be called
// after RegisterOwner() or SetOwnerAndName() has been called.
const void* UntypedFunctionMockerBase::MockObject() const
    GTEST_LOCK_EXCLUDED_(g_gmock_mutex) {
  const void* mock_obj;
  {
    // We protect mock_obj_ under g_gmock_mutex in case this mock
    // function is called from two threads concurrently.
    MutexLock l(&g_gmock_mutex);
    Assert(mock_obj_ != nullptr, __FILE__, __LINE__,
           "MockObject() must not be called before RegisterOwner() or "
           "SetOwnerAndName() has been called.");
    mock_obj = mock_obj_;
  }
  return mock_obj;
}

// Returns the name of this mock method.  Must be called after
// SetOwnerAndName() has been called.
const char* UntypedFunctionMockerBase::Name() const
    GTEST_LOCK_EXCLUDED_(g_gmock_mutex) {
  const char* name;
  {
    // We protect name_ under g_gmock_mutex in case this mock
    // function is called from two threads concurrently.
    MutexLock l(&g_gmock_mutex);
    Assert(name_ != nullptr, __FILE__, __LINE__,
           "Name() must not be called before SetOwnerAndName() has "
           "been called.");
    name = name_;
  }
  return name;
}

// Returns an Expectation object that references and co-owns exp,
// which must be an expectation on this mock function.
Expectation UntypedFunctionMockerBase::GetHandleOf(ExpectationBase* exp) {
  // See the definition of untyped_expectations_ for why access to it
  // is unprotected here.
  for (UntypedExpectations::const_iterator it = untyped_expectations_.begin();
       it != untyped_expectations_.end(); ++it) {
    if (it->get() == exp) {
      return Expectation(*it);
    }
  }

  Assert(false, __FILE__, __LINE__, "Cannot find expectation.");
  return Expectation();
  // The above statement is just to make the code compile, and will
  // never be executed.
}

// Verifies that all expectations on this mock function have been
// satisfied.  Reports one or more Google Test non-fatal failures
// and returns false if not.
bool UntypedFunctionMockerBase::VerifyAndClearExpectationsLocked()
    GTEST_EXCLUSIVE_LOCK_REQUIRED_(g_gmock_mutex) {
  g_gmock_mutex.AssertHeld();
  bool expectations_met = true;
  for (UntypedExpectations::const_iterator it = untyped_expectations_.begin();
       it != untyped_expectations_.end(); ++it) {
    ExpectationBase* const untyped_expectation = it->get();
    if (untyped_expectation->IsOverSaturated()) {
      // There was an upper-bound violation.  Since the error was
      // already reported when it occurred, there is no need to do
      // anything here.
      expectations_met = false;
    } else if (!untyped_expectation->IsSatisfied()) {
      expectations_met = false;
      ::std::stringstream ss;

      const ::std::string& expectation_name =
          untyped_expectation->GetDescription();
      ss << "Actual function ";
      if (!expectation_name.empty()) {
        ss << "\"" << expectation_name << "\" ";
      }
      ss << "call count doesn't match " << untyped_expectation->source_text()
         << "...\n";
      // No need to show the source file location of the expectation
      // in the description, as the Expect() call that follows already
      // takes care of it.
      untyped_expectation->MaybeDescribeExtraMatcherTo(&ss);
      untyped_expectation->DescribeCallCountTo(&ss);
      Expect(false, untyped_expectation->file(), untyped_expectation->line(),
             ss.str());
    }
  }

  // Deleting our expectations may trigger other mock objects to be deleted, for
  // example if an action contains a reference counted smart pointer to that
  // mock object, and that is the last reference. So if we delete our
  // expectations within the context of the global mutex we may deadlock when
  // this method is called again. Instead, make a copy of the set of
  // expectations to delete, clear our set within the mutex, and then clear the
  // copied set outside of it.
  UntypedExpectations expectations_to_delete;
  untyped_expectations_.swap(expectations_to_delete);

  g_gmock_mutex.unlock();
  expectations_to_delete.clear();
  g_gmock_mutex.lock();

  return expectations_met;
}

static CallReaction intToCallReaction(int mock_behavior) {
  if (mock_behavior >= kAllow && mock_behavior <= kFail) {
    return static_cast<internal::CallReaction>(mock_behavior);
  }
  return kWarn;
}

}  // namespace internal

// Class Mock.

namespace {

class MockObjectRegistry;
bool ReportMockObjectRegistryLeaks(MockObjectRegistry const& reg,
                                   std::string& msg);

typedef std::set<internal::UntypedFunctionMockerBase*> FunctionMockers;

// The current state of a mock object.  Such information is needed for
// detecting leaked mock objects and explicitly verifying a mock's
// expectations.
struct MockObjectState {
  MockObjectState()
      : first_used_file(nullptr), first_used_line(-1), leakable(false) {}

  // Where in the source file an ON_CALL or EXPECT_CALL is first
  // invoked on this mock object.
  const char* first_used_file;
  int first_used_line;
  ::std::string first_used_test_suite;
  ::std::string first_used_test;
  bool leakable;  // true if and only if it's OK to leak the object.
  FunctionMockers function_mockers;  // All registered methods of the object.
};

// A global registry holding the state of all mock objects that are
// alive.  A mock object is added to this registry the first time
// Mock::AllowLeak(), ON_CALL(), or EXPECT_CALL() is called on it.  It
// is removed from the registry in the mock object's destructor.
class MockObjectRegistry {
 public:
  // Maps a mock object (identified by its address) to its state.
  typedef std::map<const void*, MockObjectState> StateMap;

  // This destructor will be called when a program exits, after all
  // tests in it have been run. By then, there should be no mock
  // object alive. Therefore we report any living object as test
  // failure, unless the user explicitly asked us to ignore it.
  ~MockObjectRegistry() {
    internal::MutexLock l(&internal::g_gmock_mutex);
    std::string msg{};
    if (!ReportMockObjectRegistryLeaks(*this, msg)) {
      // RUN_ALL_TESTS() has already returned when this destructor is
      // called.  Therefore we cannot use the normal Google Test
      // failure reporting mechanism.
      std::cout << msg;
      std::cout.flush();
      ::std::cerr.flush();
#ifdef GTEST_OS_QURT
      qurt_exception_raise_fatal();
#else
      _Exit(1);  // We cannot call exit() as it is not reentrant and
                 // may already have been called.
#endif
    }
  }

  StateMap const& states() const { return states_; }
  StateMap& states() { return states_; }

 private:
  StateMap states_;
};

// Checks the given MockObjectRegistry for leaks (i.e. MockObjectStates objects
// which are still in the given MockObjectRegistry and are not marked as
// leakable) and returns a report in msg. Furthermore it returns false if leaks
// were determined and true otherwise.
// NOTE: It is the callers job to make calls
// on "reg" safe, e.g. locking its mutex (if there is any).
bool ReportMockObjectRegistryLeaks(MockObjectRegistry const& reg,
                                   std::string& msg) {
  // In case user specified to ignore leaks via a cl flag return true.
  if (!GMOCK_FLAG_GET(catch_leaked_mocks)) return true;

  typedef std::map<const void*, MockObjectState> StateMap;

  int leaked_count = 0;
  for (StateMap::const_iterator it = reg.states().begin();
       it != reg.states().end(); ++it) {
    if (it->second.leakable)  // The user said it's fine to leak this object.
      continue;

    // FIXME: Print the type of the leaked object.
    // This can help the user identify the leaked object.
    msg += "\n";
    const MockObjectState& state = it->second;
    msg += internal::FormatFileLocation(state.first_used_file,
                                        state.first_used_line);
    msg += " ERROR: this mock object";
    if (!state.first_used_test.empty()) {
      msg += " (used in test " + state.first_used_test_suite + "." +
             state.first_used_test + ")";
    }
    msg += " should be deleted but never is. Its address is @";
    std::ostringstream oss;
    oss << it->first;
    std::string address = oss.str();
    msg += address + ".";

    leaked_count++;
  }
  if (leaked_count > 0) {
    msg += "\nERROR: " + std::to_string(leaked_count) + " leaked mock " +
           (leaked_count == 1 ? "object" : "objects") +
           " found at program exit. Expectations on a mock object are "
           "verified when the object is destructed. Leaking a mock "
           "means that its expectations aren't verified, which is "
           "usually a test bug. If you really intend to leak a mock, "
           "you can suppress this error using "
           "testing::Mock::AllowLeak(mock_object), or you may use a "
           "fake or stub instead of a mock.\n";

    return false;
  }

  return true;
}

// Protected by g_gmock_mutex.
MockObjectRegistry g_mock_object_registry;

// Maps a mock object to the reaction Google Mock should have when an
// uninteresting method is called.  Protected by g_gmock_mutex.
std::unordered_map<uintptr_t, internal::CallReaction>&
UninterestingCallReactionMap() {
  static auto* map = new std::unordered_map<uintptr_t, internal::CallReaction>;
  return *map;
}

// Sets the reaction Google Mock should have when an uninteresting
// method of the given mock object is called.
void SetReactionOnUninterestingCalls(uintptr_t mock_obj,
                                     internal::CallReaction reaction)
    GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex) {
  internal::MutexLock l(&internal::g_gmock_mutex);
  UninterestingCallReactionMap()[mock_obj] = reaction;
}

}  // namespace

// Tells Google Mock to allow uninteresting calls on the given mock
// object.
void Mock::AllowUninterestingCalls(uintptr_t mock_obj)
    GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex) {
  SetReactionOnUninterestingCalls(mock_obj, internal::kAllow);
}

// Tells Google Mock to warn the user about uninteresting calls on the
// given mock object.
void Mock::WarnUninterestingCalls(uintptr_t mock_obj)
    GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex) {
  SetReactionOnUninterestingCalls(mock_obj, internal::kWarn);
}

// Tells Google Mock to fail uninteresting calls on the given mock
// object.
void Mock::FailUninterestingCalls(uintptr_t mock_obj)
    GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex) {
  SetReactionOnUninterestingCalls(mock_obj, internal::kFail);
}

// Tells Google Mock the given mock object is being destroyed and its
// entry in the call-reaction table should be removed.
void Mock::UnregisterCallReaction(uintptr_t mock_obj)
    GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex) {
  internal::MutexLock l(&internal::g_gmock_mutex);
  UninterestingCallReactionMap().erase(static_cast<uintptr_t>(mock_obj));
}

// Returns the reaction Google Mock will have on uninteresting calls
// made on the given mock object.
internal::CallReaction Mock::GetReactionOnUninterestingCalls(
    const void* mock_obj) GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex) {
  internal::MutexLock l(&internal::g_gmock_mutex);
  return (UninterestingCallReactionMap().count(
              reinterpret_cast<uintptr_t>(mock_obj)) == 0)
             ? internal::intToCallReaction(
                   GMOCK_FLAG_GET(default_mock_behavior))
             : UninterestingCallReactionMap()[reinterpret_cast<uintptr_t>(
                   mock_obj)];
}

// Tells Google Mock to ignore mock_obj when checking for leaked mock
// objects.
void Mock::AllowLeak(const void* mock_obj)
    GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex) {
  internal::MutexLock l(&internal::g_gmock_mutex);
  g_mock_object_registry.states()[mock_obj].leakable = true;
}

// Tells Google Mock to instantly check for leftover mock objects and report
// them if there are.
void Mock::CheckLeakInstant(void)
    GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex) {
  internal::MutexLock l(&internal::g_gmock_mutex);

  std::string msg{};
  // Check for leftover mock objects at this point.
  if (!ReportMockObjectRegistryLeaks(g_mock_object_registry, msg)) {
    // If there are leftover (leaked) mock objects, fail the current test and
    // report all leaks.
    auto const& first_state = g_mock_object_registry.states().begin();
    testing::internal::Expect(false, first_state->second.first_used_file,
                              first_state->second.first_used_line, msg);
  }
}

// Verifies and clears all expectations on the given mock object.  If
// the expectations aren't satisfied, generates one or more Google
// Test non-fatal failures and returns false.
bool Mock::VerifyAndClearExpectations(void* mock_obj)
    GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex) {
  internal::MutexLock l(&internal::g_gmock_mutex);
  return VerifyAndClearExpectationsLocked(mock_obj);
}

// Verifies all expectations on the given mock object and clears its
// default actions and expectations.  Returns true if and only if the
// verification was successful.
bool Mock::VerifyAndClear(void* mock_obj)
    GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex) {
  internal::MutexLock l(&internal::g_gmock_mutex);
  ClearDefaultActionsLocked(mock_obj);
  return VerifyAndClearExpectationsLocked(mock_obj);
}

// Verifies and clears all expectations on the given mock object.  If
// the expectations aren't satisfied, generates one or more Google
// Test non-fatal failures and returns false.
bool Mock::VerifyAndClearExpectationsLocked(void* mock_obj)
    GTEST_EXCLUSIVE_LOCK_REQUIRED_(internal::g_gmock_mutex) {
  internal::g_gmock_mutex.AssertHeld();
  if (g_mock_object_registry.states().count(mock_obj) == 0) {
    // No EXPECT_CALL() was set on the given mock object.
    return true;
  }

  // Verifies and clears the expectations on each mock method in the
  // given mock object.
  bool expectations_met = true;
  FunctionMockers& mockers =
      g_mock_object_registry.states()[mock_obj].function_mockers;
  for (FunctionMockers::const_iterator it = mockers.begin();
       it != mockers.end(); ++it) {
    if (!(*it)->VerifyAndClearExpectationsLocked()) {
      expectations_met = false;
    }
  }

  // We don't clear the content of mockers, as they may still be
  // needed by ClearDefaultActionsLocked().
  return expectations_met;
}

bool Mock::IsNaggy(void* mock_obj)
    GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex) {
  return Mock::GetReactionOnUninterestingCalls(mock_obj) == internal::kWarn;
}
bool Mock::IsNice(void* mock_obj)
    GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex) {
  return Mock::GetReactionOnUninterestingCalls(mock_obj) == internal::kAllow;
}
bool Mock::IsStrict(void* mock_obj)
    GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex) {
  return Mock::GetReactionOnUninterestingCalls(mock_obj) == internal::kFail;
}

// Registers a mock object and a mock method it owns.
void Mock::Register(const void* mock_obj,
                    internal::UntypedFunctionMockerBase* mocker)
    GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex) {
  internal::MutexLock l(&internal::g_gmock_mutex);
  g_mock_object_registry.states()[mock_obj].function_mockers.insert(mocker);
}

// Tells Google Mock where in the source code mock_obj is used in an
// ON_CALL or EXPECT_CALL.  In case mock_obj is leaked, this
// information helps the user identify which object it is.
void Mock::RegisterUseByOnCallOrExpectCall(const void* mock_obj,
                                           const char* file, int line)
    GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex) {
  internal::MutexLock l(&internal::g_gmock_mutex);
  MockObjectState& state = g_mock_object_registry.states()[mock_obj];
  if (state.first_used_file == nullptr) {
    state.first_used_file = file;
    state.first_used_line = line;
    const TestInfo* const test_info =
        UnitTest::GetInstance()->current_test_info();
    if (test_info != nullptr) {
      state.first_used_test_suite = test_info->test_suite_name();
      state.first_used_test = test_info->name();
    }
  }
}

// Unregisters a mock method; removes the owning mock object from the
// registry when the last mock method associated with it has been
// unregistered.  This is called only in the destructor of
// FunctionMockerBase.
void Mock::UnregisterLocked(internal::UntypedFunctionMockerBase* mocker)
    GTEST_EXCLUSIVE_LOCK_REQUIRED_(internal::g_gmock_mutex) {
  internal::g_gmock_mutex.AssertHeld();
  for (MockObjectRegistry::StateMap::iterator it =
           g_mock_object_registry.states().begin();
       it != g_mock_object_registry.states().end(); ++it) {
    FunctionMockers& mockers = it->second.function_mockers;
    if (mockers.erase(mocker) > 0) {
      // mocker was in mockers and has been just removed.
      if (mockers.empty()) {
        g_mock_object_registry.states().erase(it);
      }
      return;
    }
  }
}

// Clears all ON_CALL()s set on the given mock object.
void Mock::ClearDefaultActionsLocked(void* mock_obj)
    GTEST_EXCLUSIVE_LOCK_REQUIRED_(internal::g_gmock_mutex) {
  internal::g_gmock_mutex.AssertHeld();

  if (g_mock_object_registry.states().count(mock_obj) == 0) {
    // No ON_CALL() was set on the given mock object.
    return;
  }

  // Clears the default actions for each mock method in the given mock
  // object.
  FunctionMockers& mockers =
      g_mock_object_registry.states()[mock_obj].function_mockers;
  for (FunctionMockers::const_iterator it = mockers.begin();
       it != mockers.end(); ++it) {
    (*it)->ClearDefaultActionsLocked();
  }

  // We don't clear the content of mockers, as they may still be
  // needed by VerifyAndClearExpectationsLocked().
}

Expectation::Expectation() = default;

Expectation::Expectation(
    const std::shared_ptr<internal::ExpectationBase>& an_expectation_base)
    : expectation_base_(an_expectation_base) {}

Expectation::~Expectation() = default;

// Adds an expectation to a sequence.
void Sequence::AddExpectation(const Expectation& expectation) const {
  if (*last_expectation_ != expectation) {
    if (last_expectation_->expectation_base() != nullptr) {
      expectation.expectation_base()->immediate_prerequisites_ +=
          *last_expectation_;
    }
    *last_expectation_ = expectation;
  }
}

// Creates the implicit sequence if there isn't one.
InSequence::InSequence() {
  if (internal::g_gmock_implicit_sequence.get() == nullptr) {
    internal::g_gmock_implicit_sequence.set(new Sequence);
    sequence_created_ = true;
  } else {
    sequence_created_ = false;
  }
}

// Deletes the implicit sequence if it was created by the constructor
// of this object.
InSequence::~InSequence() {
  if (sequence_created_) {
    delete internal::g_gmock_implicit_sequence.get();
    internal::g_gmock_implicit_sequence.set(nullptr);
  }
}

}  // namespace testing

#if defined(_MSC_VER) && (_MSC_VER == 1900)
GTEST_DISABLE_MSC_WARNINGS_POP_()  // 4800
#endif
