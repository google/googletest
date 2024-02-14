#ifndef COROUTINES_INCLUDE_CORO_INTERNAL_INTEG_LAYER_H_
#define COROUTINES_INCLUDE_CORO_INTERNAL_INTEG_LAYER_H_

#include <list>

#include "cotest-crf-launch.h"
#include "cotest-crf-test.h"
#include "cotest-integ-finder.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace testing {

// ------------------ Classes ------------------

template <typename T>
class LaunchHandle;

class EventHandle;

template <typename T>
class SignatureHandle;

namespace internal {

class UntypedFunctionMockerBase;  // A GMock class

template <typename T>
class CotestExpectationFactory;

template <typename T>
class CotestWatcher;

class CotestCardinality;

class RAIISetFlag;

class Coroutine : public MockHandler {
   public:
    using BodyFunctionType = std::function<void(Coroutine *)>;

    Coroutine() = delete;
    Coroutine(const Coroutine &i) = delete;
    Coroutine(Coroutine &&i);
    Coroutine &operator=(const Coroutine &) = delete;
    Coroutine &operator=(Coroutine &&) = delete;
    ~Coroutine();

    Coroutine(BodyFunctionType body, std::string name);

    template <typename R>
    LaunchHandle<R> Launch(LaunchLambdaType<R> &&user_lambda, std::string name);

    void WatchCall(const char *file, int line, crf::UntypedMockObjectPointer obj = nullptr);

    template <typename R, typename... Args>
    TypedExpectation<R(Args...)> &WatchCall(MockSpec<R(Args...)> &&mock_spec, const char *file, int line,
                                            const char *obj, const char *call);
    void SetSatisfied();
    void Retire();
    bool IsRetired() const;
    std::string GetName() const override;

    EventHandle NextEvent(const char *file, int line);

    std::shared_ptr<crf::TestCoroutine> GetCRFTestCoroutine() override;

    void OnTestCoroExit();
    void OnWatcherDestruct();
    void AddWatcher(std::shared_ptr<ExpectationBase> watcher);
    static RAIISetFlag RAIIGMockMutexIsLocked();

   private:
    const MockHandlerScheme *GetMockHandlerScheme() const override;
    void DestructionIterations();

    const std::shared_ptr<crf::TestCoroutine> crf;
    const std::string name;
    std::list<std::weak_ptr<ExpectationBase>> my_watchers_all;
    const std::shared_ptr<CotestCardinality> my_cardinality;
    MockHandlerScheme my_untyped_watchers;
    bool retired = false;
    bool initial_activity_complete = false;

    static bool gmock_mutex_held;  // because the mutex is static

   public:
    // When cotest UI macros are used with an explicit object eg
    // my_coro.WATCH_CALL(), suppress the error caused by the macro
    // injecting cotest_coro_->
    Coroutine *const cotest_coro_;
};

template <typename R, typename... Args>
class CotestExpectationFactory<R(Args...)> : public SpecFactory<R(Args...)> {
   private:
    using F = R(Args...);

   public:
    CotestExpectationFactory() = delete;
    CotestExpectationFactory(Coroutine *coroutine_, CotestCardinality *cardinality_)
        : coroutine(coroutine_), cardinality(cardinality_) {}

    using ArgumentMatcherTuple = typename Function<F>::ArgumentMatcherTuple;

    OnCallSpec<F> *CreateOnCall(const char *a_file, int a_line, const ArgumentMatcherTuple &m) override;
    std::shared_ptr<TypedExpectation<F>> CreateExpectation(FunctionMocker<F> *owning_mocker, const char *a_file,
                                                           int a_line, const std::string &a_source_text,
                                                           const ArgumentMatcherTuple &m) override;

    Coroutine *const coroutine;
    CotestCardinality *const cardinality;
};

template <typename R, typename... Args>
class CotestWatcher<R(Args...)> : public TypedExpectation<R(Args...)> {
    using Result = typename Function<R(Args...)>::Result;
    using ArgumentTuple = typename Function<R(Args...)>::ArgumentTuple;
    using ArgumentMatcherTuple = typename Function<R(Args...)>::ArgumentMatcherTuple;

   public:
    CotestWatcher() = delete;
    CotestWatcher(const CotestWatcher &) = delete;
    CotestWatcher(CotestWatcher &&) = delete;
    CotestWatcher &operator=(const CotestWatcher &) = delete;
    CotestWatcher &operator=(CotestWatcher &&) = delete;
    ~CotestWatcher() override;

    CotestWatcher(Coroutine *const coroutine_, crf::UntypedMockObjectPointer watched_mock_object_,
                  FunctionMocker<R(Args...)> *owning_mocker_, const char *a_file, int a_line,
                  const std::string &a_source_text, const ArgumentMatcherTuple &m, CotestCardinality *cardinality_);

    void DetachCoroutine() override;

   private:
    // Decide whether the coroutine should accept the call. GMock mutex is
    // LOCKED during this call.
    bool ShouldHandleCall(const UntypedFunctionMockerBase *mocker, const void *untyped_args) override;

    // Call passed exterior matching and must be shown to the coroutine
    bool SeenMockCallLocked(const UntypedFunctionMockerBase *mocker, const void *untyped_args);

    // Similar side-effects to to GetActionForArguments() and GetCurrentAction()
    bool UpdateCardinality(const UntypedFunctionMockerBase *mocker, const void *untyped_args, ::std::ostream *what,
                           ::std::ostream *why) override;

    // Implement cotest action here
    bool TryPerformAction(const UntypedFunctionMockerBase *mocker, const void *untyped_args,
                          const void **untyped_return_value) override;

    // We need our own describe function because we haven't set up any
    // matchers. At minimum, reveal that a coroutine is being used.
    void ExplainMatchResultTo(const ArgumentTuple &args, ::std::ostream *os) const override;

   private:
    const crf::UntypedMockObjectPointer watched_mock_object;
    const bool is_typed_watcher;
    Coroutine *coroutine;
    CotestCardinality *const cardinality;
};

class CotestCardinality : public CardinalityInterface {
   public:
    CotestCardinality();

    bool IsSatisfiedByCallCount(int call_count) const override;
    bool IsSaturatedByCallCount(int call_count) const override;
    void DescribeTo(::std::ostream *os) const override;

    void InteriorSetSatisfied();
    void OnTestCoroExit();
    bool OnSeenMockCall();

   private:
    enum class State { Unsatisfied, SatisfiedByUser, SatisfiedByExit, Oversaturated };
    State state = State::Unsatisfied;
};

class RAIISetFlag {
   public:
    RAIISetFlag(bool *flag_) : flag(flag_), old_flag(*flag) {
        COTEST_ASSERT(flag);
        *flag = true;
    }
    RAIISetFlag(RAIISetFlag &) = delete;
    RAIISetFlag &operator=(RAIISetFlag &) = delete;
    RAIISetFlag(RAIISetFlag &&) = default;
    RAIISetFlag &operator=(RAIISetFlag &&) = delete;
    ~RAIISetFlag() {
        COTEST_ASSERT(*flag);
        *flag = old_flag;
    }

   private:
    bool *const flag;
    bool const old_flag;
};

template <typename F>
SignatureHandle<F> CreateSignatureHandle(MockSpec<F> &&mock_spec) {
    return SignatureHandle<F>();
}

// ------------------ Templated members ------------------

template <typename R>
LaunchHandle<R> Coroutine::Launch(LaunchLambdaType<R> &&user_lambda, std::string launch_text) {
    return LaunchHandle<R>(crf->Launch<R>(std::move(user_lambda), launch_text));
}

template <typename R, typename... Args>
TypedExpectation<R(Args...)> &Coroutine::WatchCall(MockSpec<R(Args...)> &&mock_spec, const char *file, int line,
                                                   const char *obj, const char *call) {
    CotestExpectationFactory<R(Args...)> factory(this, my_cardinality.get());
    TypedExpectation<R(Args...)> &new_exp = mock_spec.InternalExpectedAt(&factory, file, line, obj, call);

    // This is only to get the right GMock behaviour
    new_exp.set_repeated_action(DoDefault());

    // Hooking up backend to cardinality interface so we can
    // satisfy and saturate.
    new_exp.set_cardinality(Cardinality(my_cardinality));

    return new_exp;
}

template <typename R, typename... Args>
OnCallSpec<R(Args...)> *CotestExpectationFactory<R(Args...)>::CreateOnCall(const char *a_file, int a_line,
                                                                           const ArgumentMatcherTuple &m) {
    return new OnCallSpec<F>(a_file, a_line, m);
}

template <typename R, typename... Args>
std::shared_ptr<TypedExpectation<R(Args...)>> CotestExpectationFactory<R(Args...)>::CreateExpectation(
    FunctionMocker<F> *owning_mocker, const char *a_file, int a_line, const std::string &a_source_text,
    const ArgumentMatcherTuple &m) {
    const auto sp = std::make_shared<CotestWatcher<F>>(coroutine, nullptr, owning_mocker, a_file, a_line, a_source_text,
                                                       m, cardinality);
    coroutine->AddWatcher(sp);
    if (coroutine->IsRetired()) {
        MutexLock l(&g_gmock_mutex);
        sp->Retire();
    }
    return sp;
}

template <typename R, typename... Args>
CotestWatcher<R(Args...)>::CotestWatcher(Coroutine *const coroutine_,
                                         crf::UntypedMockObjectPointer watched_mock_object_,
                                         FunctionMocker<R(Args...)> *owning_mocker_, const char *a_file, int a_line,
                                         const std::string &a_source_text, const ArgumentMatcherTuple &m,
                                         CotestCardinality *cardinality__)
    : CotestWatcher<R(Args...)>::TypedExpectation(owning_mocker_, a_file, a_line, a_source_text, m),
      watched_mock_object(watched_mock_object_),
      // NOTE: the owning_mocker_ could fall out of scope and be destructed
      // while we're still alive, so try to avoid using it and if it's really
      // necessary, a DetachMocker() mechanism will be needed.
      is_typed_watcher(owning_mocker_ != nullptr),
      coroutine(coroutine_),
      cardinality(cardinality__) {}

template <typename R, typename... Args>
CotestWatcher<R(Args...)>::~CotestWatcher() {
    if (coroutine) coroutine->OnWatcherDestruct();
}

template <typename R, typename... Args>
void CotestWatcher<R(Args...)>::DetachCoroutine() {
    std::clog << COTEST_THIS << std::endl;

    coroutine = nullptr;

    if (!is_typed_watcher) {
        // Since untyped watches are not registered with GMock registry, we
        // need to directly request a cardinality state report.
        MutexLock l(&g_gmock_mutex);
        ExpectationBase::VerifyExpectationLocked();
    }
}

template <typename R, typename... Args>
bool CotestWatcher<R(Args...)>::ShouldHandleCall(const UntypedFunctionMockerBase *mocker,
                                                 crf::UntypedArgsPointer untyped_args) {
    RAIISetFlag gmmh(Coroutine::RAIIGMockMutexIsLocked());  // This is called from GMock with
                                                            // mutex held

    COTEST_ASSERT(coroutine && "coroutine object was destructed before being sent a call");
    std::clog << "CotestWatcher::ShouldHandleCall() (typed=" << is_typed_watcher << ")..." << std::endl;
    COTEST_ASSERT(mocker);

    // Respect retirement state and pre-requisites in gmock exp in case
    // they are chained.
    if (CotestWatcher<R(Args...)>::TypedExpectation::is_retired() ||
        !CotestWatcher<R(Args...)>::TypedExpectation::AllPrerequisitesAreSatisfied())
        return false;

    // Apply exterior filtering (main and extra) if we've got typed arguments
    if (is_typed_watcher) {
        // We can use types based on our template arguments
        const ArgumentTuple *typed_args = static_cast<const ArgumentTuple *>(untyped_args);
        if (this->Matches(*typed_args)) return SeenMockCallLocked(mocker, untyped_args);
    } else {
        // We cannot use types based on our template arguments
        if (!watched_mock_object || watched_mock_object == mocker->MockObjectLocked())
            return SeenMockCallLocked(mocker, untyped_args);
    }
    return false;
}

template <typename R, typename... Args>
bool CotestWatcher<R(Args...)>::SeenMockCallLocked(const UntypedFunctionMockerBase *mocker, const void *untyped_args) {
    std::clog << this << " CotestWatcher::SeenMockCallLocked()..." << std::endl;

    // If oversaturated, return in order to permit cmock to detect this and talk
    // to the user about it.
    if (cardinality->OnSeenMockCall()) return true;

    const std::shared_ptr<crf::MockSource> mock_source =
        crf::LaunchCoroutinePool::GetInstance()->FindActiveMockSource();
    const std::shared_ptr<crf::MockRoutingSession> crf_mrs = mock_source->GetCurrentMockRS();
    auto crf_tc = coroutine->GetCRFTestCoroutine();
    crf_mrs->Configure(crf_tc.get());

    // Permit coroutine to perform interior filtering
    const std::string name = mocker->NameLocked();
    const bool accepted = crf_mrs->SeenMockCallLocked(untyped_args);
    COTEST_ASSERT(crf::LaunchCoroutinePool::GetInstance()->FindActiveMockSource() ==
                  mock_source);  // should still be on same source after the yield

    if (!accepted || coroutine->GetCRFTestCoroutine()->IsCoroutineExited()) {
        crf_mrs->Configure(nullptr);
        return false;
    }

    return true;
}

template <typename R, typename... Args>
bool CotestWatcher<R(Args...)>::UpdateCardinality(const UntypedFunctionMockerBase *mocker, const void *untyped_args,
                                                  ::std::ostream *what, ::std::ostream *why) {
    // Note: code lifted from GetActionForArguments() and GetCurrentAction()
    // without any attempt at integration into cotest.
    std::clog << "CotestWatcher::UpdateCardinality() (typed=" << is_typed_watcher << ")..." << std::endl;
    g_gmock_mutex.AssertHeld();
    RAIISetFlag gmmh(Coroutine::RAIIGMockMutexIsLocked());  // This is called from GMock with
                                                            // mutex held

    using Base = ExpectationBase;

    const ::std::string &expectation_description = ExpectationBase::GetDescription();
    if (Base::IsSaturated()) {
        // We have an excessive call.
        Base::IncrementCallCount();
        *what << "Mock function ";
        if (!expectation_description.empty()) {
            *what << "\"" << expectation_description << "\" ";
        }
        *what << "called more times than expected - ";
        // mocker->DescribeDefaultActionTo(args, what);
        Base::DescribeCallCountTo(why);

        return false;
    }

    Base::IncrementCallCount();
    Base::RetireAllPreRequisites();

    if (Base::retires_on_saturation_ && Base::IsSaturated()) {
        Base::Retire();
    }

    // Must be done after IncrementCount()!
    *what << "Mock function ";
    if (!expectation_description.empty()) {
        *what << "\"" << expectation_description << "\" ";
    }
    *what << "call matches " << Base::source_text() << "...\n";

    const int count = Base::call_count();
    COTEST_ASSERT(count >= 1);
    //"call_count() is <= 0 when UpdateCardinality() is "
    //"called - this should never happen.");

    const int action_count = static_cast<int>(Base::untyped_actions_.size());
    if (action_count > 0 && !Base::repeated_action_specified_ && count > action_count) {
        // If there is at least one WillOnce() and no WillRepeatedly(),
        // we warn the user when the WillOnce() clauses ran out.
        ::std::stringstream ss;
        Base::DescribeLocationTo(&ss);
        ss << "Actions ran out in " << Base::source_text() << "...\n"
           << "Called " << count << " times, but only " << action_count << " WillOnce()"
           << (action_count == 1 ? " is" : "s are") << " specified - ";
        // mocker->DescribeDefaultActionTo(args, &ss);
        Log(kWarning, ss.str(), 1);
    }

    return Base::repeated_action_specified_;
}

template <typename R, typename... Args>
bool CotestWatcher<R(Args...)>::TryPerformAction(const UntypedFunctionMockerBase *mocker, const void *untyped_args,
                                                 const void **untyped_return_value) {
    // Note: this is called from GMock WITHOUT mutex held
    const std::shared_ptr<crf::MockSource> mock_source =
        crf::LaunchCoroutinePool::GetInstance()->FindActiveMockSource();
    const std::shared_ptr<crf::MockRoutingSession> crf_mrs = mock_source->GetCurrentMockRS();
    std::clog << "CotestWatcher::TryPerformAction() (typed=" << is_typed_watcher << " ms=" << mock_source << ")..."
              << std::endl;

    COTEST_ASSERT(mocker);
    COTEST_ASSERT(coroutine);  // coroutine object was destructed before being sent a call

    // Let the coroutine run and collect the return value
    *untyped_return_value = crf_mrs->ActionsAndReturnUnlocked();

    if (is_typed_watcher) COTEST_ASSERT(CotestTypeUtils<R>::NullCheck(*untyped_return_value));

    crf_mrs->Configure(nullptr);

    return true;
}

template <typename R, typename... Args>
void CotestWatcher<R(Args...)>::ExplainMatchResultTo(const CotestWatcher<R(Args...)>::ArgumentTuple &args,
                                                     ::std::ostream *os) const {
    RAIISetFlag gmmh(Coroutine::RAIIGMockMutexIsLocked());  // This is called from GMock with
                                                            // mutex held
    const std::shared_ptr<crf::MockSource> mock_source =
        crf::LaunchCoroutinePool::GetInstance()->FindActiveMockSource();

    // TOOD improve in phase 3

    if (CotestWatcher<R(Args...)>::TypedExpectation::is_retired()) {
        *os << "         Expected: the coroutine is active\n"
            << "           Actual: it is retired\n";
    } else {
        *os << "         Expected: determined by coroutine\n"
            << "           Actual: mock call dropped or not seen\n";
    }
}

}  // namespace internal
}  // namespace testing

#endif
