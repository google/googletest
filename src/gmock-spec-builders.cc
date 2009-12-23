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
//
// Author: wan@google.com (Zhanyong Wan)

// Google Mock - a framework for writing C++ mock classes.
//
// This file implements the spec builder syntax (ON_CALL and
// EXPECT_CALL).

#include <gmock/gmock-spec-builders.h>

#include <stdlib.h>
#include <iostream>  // NOLINT
#include <map>
#include <set>
#include <string>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#if GTEST_OS_CYGWIN || GTEST_OS_LINUX || GTEST_OS_MAC
#include <unistd.h>  // NOLINT
#endif

namespace testing {
namespace internal {

// Protects the mock object registry (in class Mock), all function
// mockers, and all expectations.
Mutex g_gmock_mutex(Mutex::NO_CONSTRUCTOR_NEEDED_FOR_STATIC_MUTEX);

// Constructs an ExpectationBase object.
ExpectationBase::ExpectationBase(const char* a_file,
                                 int a_line,
                                 const string& a_source_text)
    : file_(a_file),
      line_(a_line),
      source_text_(a_source_text),
      cardinality_specified_(false),
      cardinality_(Exactly(1)),
      call_count_(0),
      retired_(false) {
}

// Destructs an ExpectationBase object.
ExpectationBase::~ExpectationBase() {}

// Explicitly specifies the cardinality of this expectation.  Used by
// the subclasses to implement the .Times() clause.
void ExpectationBase::SpecifyCardinality(const Cardinality& a_cardinality) {
  cardinality_specified_ = true;
  cardinality_ = a_cardinality;
}

// Retires all pre-requisites of this expectation.
void ExpectationBase::RetireAllPreRequisites() {
  if (is_retired()) {
    // We can take this short-cut as we never retire an expectation
    // until we have retired all its pre-requisites.
    return;
  }

  for (ExpectationSet::const_iterator it = immediate_prerequisites_.begin();
       it != immediate_prerequisites_.end(); ++it) {
    ExpectationBase* const prerequisite = it->expectation_base().get();
    if (!prerequisite->is_retired()) {
      prerequisite->RetireAllPreRequisites();
      prerequisite->Retire();
    }
  }
}

// Returns true iff all pre-requisites of this expectation have been
// satisfied.
// L >= g_gmock_mutex
bool ExpectationBase::AllPrerequisitesAreSatisfied() const {
  g_gmock_mutex.AssertHeld();
  for (ExpectationSet::const_iterator it = immediate_prerequisites_.begin();
       it != immediate_prerequisites_.end(); ++it) {
    if (!(it->expectation_base()->IsSatisfied()) ||
        !(it->expectation_base()->AllPrerequisitesAreSatisfied()))
      return false;
  }
  return true;
}

// Adds unsatisfied pre-requisites of this expectation to 'result'.
// L >= g_gmock_mutex
void ExpectationBase::FindUnsatisfiedPrerequisites(
    ExpectationSet* result) const {
  g_gmock_mutex.AssertHeld();
  for (ExpectationSet::const_iterator it = immediate_prerequisites_.begin();
       it != immediate_prerequisites_.end(); ++it) {
    if (it->expectation_base()->IsSatisfied()) {
      // If *it is satisfied and has a call count of 0, some of its
      // pre-requisites may not be satisfied yet.
      if (it->expectation_base()->call_count_ == 0) {
        it->expectation_base()->FindUnsatisfiedPrerequisites(result);
      }
    } else {
      // Now that we know *it is unsatisfied, we are not so interested
      // in whether its pre-requisites are satisfied.  Therefore we
      // don't recursively call FindUnsatisfiedPrerequisites() here.
      *result += *it;
    }
  }
}

// Points to the implicit sequence introduced by a living InSequence
// object (if any) in the current thread or NULL.
ThreadLocal<Sequence*> g_gmock_implicit_sequence;

// Reports an uninteresting call (whose description is in msg) in the
// manner specified by 'reaction'.
void ReportUninterestingCall(CallReaction reaction, const string& msg) {
  switch (reaction) {
    case ALLOW:
      Log(INFO, msg, 3);
      break;
    case WARN:
      Log(WARNING, msg, 3);
      break;
    default:  // FAIL
      Expect(false, NULL, -1, msg);
  }
}

}  // namespace internal

// Class Mock.

namespace {

typedef std::set<internal::UntypedFunctionMockerBase*> FunctionMockers;

// The current state of a mock object.  Such information is needed for
// detecting leaked mock objects and explicitly verifying a mock's
// expectations.
struct MockObjectState {
  MockObjectState()
      : first_used_file(NULL), first_used_line(-1), leakable(false) {}

  // Where in the source file an ON_CALL or EXPECT_CALL is first
  // invoked on this mock object.
  const char* first_used_file;
  int first_used_line;
  ::std::string first_used_test_case;
  ::std::string first_used_test;
  bool leakable;  // true iff it's OK to leak the object.
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
  // tests in it have been run.  By then, there should be no mock
  // object alive.  Therefore we report any living object as test
  // failure, unless the user explicitly asked us to ignore it.
  ~MockObjectRegistry() {

    // "using ::std::cout;" doesn't work with Symbian's STLport, where cout is
    // a macro.

    if (!GMOCK_FLAG(catch_leaked_mocks))
      return;

    int leaked_count = 0;
    for (StateMap::const_iterator it = states_.begin(); it != states_.end();
         ++it) {
      if (it->second.leakable)  // The user said it's fine to leak this object.
        continue;

      // TODO(wan@google.com): Print the type of the leaked object.
      // This can help the user identify the leaked object.
      std::cout << "\n";
      const MockObjectState& state = it->second;
      std::cout << internal::FormatFileLocation(state.first_used_file,
                                                state.first_used_line);
      std::cout << " ERROR: this mock object";
      if (state.first_used_test != "") {
        std::cout << " (used in test " << state.first_used_test_case << "."
             << state.first_used_test << ")";
      }
      std::cout << " should be deleted but never is. Its address is @"
           << it->first << ".";
      leaked_count++;
    }
    if (leaked_count > 0) {
      std::cout << "\nERROR: " << leaked_count
           << " leaked mock " << (leaked_count == 1 ? "object" : "objects")
           << " found at program exit.\n";
      std::cout.flush();
      ::std::cerr.flush();
      // RUN_ALL_TESTS() has already returned when this destructor is
      // called.  Therefore we cannot use the normal Google Test
      // failure reporting mechanism.
      _exit(1);  // We cannot call exit() as it is not reentrant and
                 // may already have been called.
    }
  }

  StateMap& states() { return states_; }
 private:
  StateMap states_;
};

// Protected by g_gmock_mutex.
MockObjectRegistry g_mock_object_registry;

// Maps a mock object to the reaction Google Mock should have when an
// uninteresting method is called.  Protected by g_gmock_mutex.
std::map<const void*, internal::CallReaction> g_uninteresting_call_reaction;

// Sets the reaction Google Mock should have when an uninteresting
// method of the given mock object is called.
// L < g_gmock_mutex
void SetReactionOnUninterestingCalls(const void* mock_obj,
                                     internal::CallReaction reaction) {
  internal::MutexLock l(&internal::g_gmock_mutex);
  g_uninteresting_call_reaction[mock_obj] = reaction;
}

}  // namespace

// Tells Google Mock to allow uninteresting calls on the given mock
// object.
// L < g_gmock_mutex
void Mock::AllowUninterestingCalls(const void* mock_obj) {
  SetReactionOnUninterestingCalls(mock_obj, internal::ALLOW);
}

// Tells Google Mock to warn the user about uninteresting calls on the
// given mock object.
// L < g_gmock_mutex
void Mock::WarnUninterestingCalls(const void* mock_obj) {
  SetReactionOnUninterestingCalls(mock_obj, internal::WARN);
}

// Tells Google Mock to fail uninteresting calls on the given mock
// object.
// L < g_gmock_mutex
void Mock::FailUninterestingCalls(const void* mock_obj) {
  SetReactionOnUninterestingCalls(mock_obj, internal::FAIL);
}

// Tells Google Mock the given mock object is being destroyed and its
// entry in the call-reaction table should be removed.
// L < g_gmock_mutex
void Mock::UnregisterCallReaction(const void* mock_obj) {
  internal::MutexLock l(&internal::g_gmock_mutex);
  g_uninteresting_call_reaction.erase(mock_obj);
}

// Returns the reaction Google Mock will have on uninteresting calls
// made on the given mock object.
// L < g_gmock_mutex
internal::CallReaction Mock::GetReactionOnUninterestingCalls(
    const void* mock_obj) {
  internal::MutexLock l(&internal::g_gmock_mutex);
  return (g_uninteresting_call_reaction.count(mock_obj) == 0) ?
      internal::WARN : g_uninteresting_call_reaction[mock_obj];
}

// Tells Google Mock to ignore mock_obj when checking for leaked mock
// objects.
// L < g_gmock_mutex
void Mock::AllowLeak(const void* mock_obj) {
  internal::MutexLock l(&internal::g_gmock_mutex);
  g_mock_object_registry.states()[mock_obj].leakable = true;
}

// Verifies and clears all expectations on the given mock object.  If
// the expectations aren't satisfied, generates one or more Google
// Test non-fatal failures and returns false.
// L < g_gmock_mutex
bool Mock::VerifyAndClearExpectations(void* mock_obj) {
  internal::MutexLock l(&internal::g_gmock_mutex);
  return VerifyAndClearExpectationsLocked(mock_obj);
}

// Verifies all expectations on the given mock object and clears its
// default actions and expectations.  Returns true iff the
// verification was successful.
// L < g_gmock_mutex
bool Mock::VerifyAndClear(void* mock_obj) {
  internal::MutexLock l(&internal::g_gmock_mutex);
  ClearDefaultActionsLocked(mock_obj);
  return VerifyAndClearExpectationsLocked(mock_obj);
}

// Verifies and clears all expectations on the given mock object.  If
// the expectations aren't satisfied, generates one or more Google
// Test non-fatal failures and returns false.
// L >= g_gmock_mutex
bool Mock::VerifyAndClearExpectationsLocked(void* mock_obj) {
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

// Registers a mock object and a mock method it owns.
// L < g_gmock_mutex
void Mock::Register(const void* mock_obj,
                    internal::UntypedFunctionMockerBase* mocker) {
  internal::MutexLock l(&internal::g_gmock_mutex);
  g_mock_object_registry.states()[mock_obj].function_mockers.insert(mocker);
}

// Tells Google Mock where in the source code mock_obj is used in an
// ON_CALL or EXPECT_CALL.  In case mock_obj is leaked, this
// information helps the user identify which object it is.
// L < g_gmock_mutex
void Mock::RegisterUseByOnCallOrExpectCall(
    const void* mock_obj, const char* file, int line) {
  internal::MutexLock l(&internal::g_gmock_mutex);
  MockObjectState& state = g_mock_object_registry.states()[mock_obj];
  if (state.first_used_file == NULL) {
    state.first_used_file = file;
    state.first_used_line = line;
    const TestInfo* const test_info =
        UnitTest::GetInstance()->current_test_info();
    if (test_info != NULL) {
      // TODO(wan@google.com): record the test case name when the
      // ON_CALL or EXPECT_CALL is invoked from SetUpTestCase() or
      // TearDownTestCase().
      state.first_used_test_case = test_info->test_case_name();
      state.first_used_test = test_info->name();
    }
  }
}

// Unregisters a mock method; removes the owning mock object from the
// registry when the last mock method associated with it has been
// unregistered.  This is called only in the destructor of
// FunctionMockerBase.
// L >= g_gmock_mutex
void Mock::UnregisterLocked(internal::UntypedFunctionMockerBase* mocker) {
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
// L >= g_gmock_mutex
void Mock::ClearDefaultActionsLocked(void* mock_obj) {
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

Expectation::Expectation() {}

Expectation::Expectation(
    const internal::linked_ptr<internal::ExpectationBase>& an_expectation_base)
    : expectation_base_(an_expectation_base) {}

Expectation::~Expectation() {}

// Adds an expectation to a sequence.
void Sequence::AddExpectation(const Expectation& expectation) const {
  if (*last_expectation_ != expectation) {
    if (last_expectation_->expectation_base() != NULL) {
      expectation.expectation_base()->immediate_prerequisites_
          += *last_expectation_;
    }
    *last_expectation_ = expectation;
  }
}

// Creates the implicit sequence if there isn't one.
InSequence::InSequence() {
  if (internal::g_gmock_implicit_sequence.get() == NULL) {
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
    internal::g_gmock_implicit_sequence.set(NULL);
  }
}

}  // namespace testing
