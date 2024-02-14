#include "cotest/internal/cotest-integ-mock.h"

#include "cotest/internal/cotest-util-logging.h"

namespace testing {
namespace internal {

using coro_impl::PtrToString;

Coroutine::Coroutine(BodyFunctionType body, std::string name_)
    : crf(std::make_shared<crf::TestCoroutine>(std::bind(body, this), name_,
                                               std::bind(&Coroutine::OnTestCoroExit, this))),
      name(name_),
      my_cardinality(std::make_shared<CotestCardinality>()),
      cotest_coro_(this) {
    // Strong RAII model: this call to InitialActivity() can complete the entire
    // test, leaving an exited coroutine and nothing to do but clean up.
    crf->InitialActivity();
    initial_activity_complete = true;
}

Coroutine::Coroutine(Coroutine&& i)
    : crf(std::move(i.crf)),
      name(std::move(i.name)),
      my_watchers_all(std::move(i.my_watchers_all)),
      my_cardinality(std::move(i.my_cardinality)),
      cotest_coro_(this) {
    // To move the coroutine after adding Watchers would invalidate the
    // coroutine pointers inside them, and we don't need to do it. This
    // move constructor is just to help with the UI syntax around creating
    // the coroutine.
    COTEST_ASSERT(my_watchers_all.empty());
}

Coroutine::~Coroutine() {
    // This destructor is doing a lot of stuff, including iterating coroutines.
    // Justification is that we want RAII style in the UI following GMock/GTest UI
    // design.
    DestructionIterations();

    if (!crf->IsCoroutineExited()) {
        // Generate this message as early as possible - assists with fiding cause
        if (my_cardinality->IsSatisfiedByCallCount(0))
            std::clog << "Cancelling a satisfied coroutine " << crf->GetName() << " - this is not an error."
                      << std::endl;
        else
            std::clog << "Cancelling an unsatisfied coroutine " << crf->GetName() << " - this will cause test failure."
                      << std::endl;
    }

    for (auto p_exp : my_watchers_all)
        if (auto p_exp_locked = p_exp.lock()) p_exp_locked->DetachCoroutine();

    if (!crf->IsCoroutineExited()) {
        crf->Cancel();
    }
}

void Coroutine::WatchCall(const char* file, int line, crf::UntypedMockObjectPointer obj) {
    MutexLock l(&g_gmock_mutex);

    // It's unfortunate that we have to provide a function type here, but we
    // require functionality available in the templated types (TypedExpectation<>
    // and CotestWatcher<>) and don't want to duplicate it. At least only one
    // instantiation of the templated code arises as a result.
    using UntypedWatcher = CotestWatcher<void()>;

    std::shared_ptr<UntypedWatcher> sp;

    // Add this expectation to our list, making use of the expectation
    // finder singleton so it can update the global priority level.
    auto cem = CotestMockHandlerPool::GetOrCreateInstance();
    cem->AddExpectation([&]() {
        sp = std::make_shared<UntypedWatcher>(this, obj, nullptr, file, line, "",
                                              Function<int()>::ArgumentMatcherTuple(), my_cardinality.get());
        my_untyped_watchers.push_back(sp);
    });

    AddWatcher(sp);

    // This is only to get the right GMock behaviour
    sp->set_repeated_action(DoDefault());

    // Hooking up backend to cardinality interface so we can
    // satisfy and saturate.
    sp->set_cardinality(Cardinality(my_cardinality));
}

void Coroutine::SetSatisfied() { my_cardinality->InteriorSetSatisfied(); }

void Coroutine::Retire() {
    // Lock if required since expectation retire flags are mutex-protected, so
    // we should not change one while another thread has a lock on it.
    std::unique_ptr<testing::internal::MutexLock> plock;
    if (!gmock_mutex_held) plock = std::make_unique<testing::internal::MutexLock>(&testing::internal::g_gmock_mutex);

    retired = true;

    for (auto p_exp : my_watchers_all) {
        if (auto p_exp_locked = p_exp.lock()) p_exp_locked->Retire();
    }
}

bool Coroutine::IsRetired() const { return retired; }

std::string Coroutine::GetName() const { return name; }

const MockHandlerScheme* Coroutine::GetMockHandlerScheme() const { return &my_untyped_watchers; }

void Coroutine::DestructionIterations() {
    if (crf->IsCoroutineExited()) return;  // no action required

    crf->DestructionIterations();
}

std::shared_ptr<crf::TestCoroutine> Coroutine::GetCRFTestCoroutine() {
    COTEST_ASSERT(crf);
    return crf;
}

void Coroutine::OnTestCoroExit() { my_cardinality->OnTestCoroExit(); }

void Coroutine::OnWatcherDestruct() {
    // It is safe to destruct a mock object:
    // - (a) if this coroutine is not watching it, or
    // - (b) before waiting for any activity in main, or
    // - (c) after coroutine body has exited, or
    // - (d) after the coroutine object has been destructed.
    //
    // This is to prevent GMock from verifying end-of-life cardinality
    // on a coroutine that might be about to call eg RETIRE()
    // or SATURATE() but has not, because of CRF constraint #3
    //
    // (b) Is OK because while in intial actions, everything is
    //     happening in coro body and therefore synchronised.
    // (c) An exited coro body is also synchronised.
    // (d) An extra isteration is provided in this case to allow
    //     corotuine to run and update cardinality state.

    // if coro destructed, this method is not called.
    bool ok = !initial_activity_complete || crf->IsCoroutineExited();
    COTEST_ASSERT(ok &&
                  "Mock object was destructed at an unsafe time: error reports "
                  "may be inaccurate");
}

void Coroutine::AddWatcher(std::shared_ptr<testing::internal::ExpectationBase> watcher) {
    my_watchers_all.push_back(watcher);
}

RAIISetFlag Coroutine::RAIIGMockMutexIsLocked() { return RAIISetFlag(&gmock_mutex_held); }

CotestCardinality::CotestCardinality() {}

bool CotestCardinality::IsSatisfiedByCallCount(int call_count) const {
    // These are our satisfied states. Final checks will pass if we destruct in
    // these states. State::Unsatisfied fails the test because it is assumed that
    // mock calls were expected but didn't arrive; State::Oversaturated is a fail
    // for the opposite reson and should cause this method to return false.
    return state == State::SatisfiedByUser || state == State::SatisfiedByExit;
}

bool CotestCardinality::IsSaturatedByCallCount(int call_count) const {
    // These states correspond to saturation and will cause GMock to
    // report that the coroutine was saturated.
    return state == State::SatisfiedByExit || state == State::Oversaturated;
}

void CotestCardinality::DescribeTo(::std::ostream* os) const { *os << "called as determined by coroutine"; }

void CotestCardinality::InteriorSetSatisfied() {
    // User has invoked SATISFY()
    switch (state) {
        case State::Unsatisfied:
            state = State::SatisfiedByUser;
            break;
        case State::SatisfiedByUser:
            break;
        case State::SatisfiedByExit:
            COTEST_ASSERT(!"Interior call while exited");
        case State::Oversaturated:
            COTEST_ASSERT(!"Interior call while exited");
    };
}

void CotestCardinality::OnTestCoroExit() {
    // Cotest policy: exiting a coroutine satisfies its cardinality
    // unless or until a mock call gets through exterior matching.
    switch (state) {
        case State::Unsatisfied:
        case State::SatisfiedByUser:
            state = State::SatisfiedByExit;
            break;
        case State::SatisfiedByExit:
            COTEST_ASSERT(!"Multiple OnTestCoroExit()");
        case State::Oversaturated:
            COTEST_ASSERT(!"Multiple OnTestCoroExit()");
    };
}

bool CotestCardinality::OnSeenMockCall() {
    // Cotest policy: if the coroutine exited and we've matched on
    // exterior criteria, report over-saturation. We will not be
    // able to perform interior matching if the coroutine exited,
    // so the user's intention must be considered undefined and we
    // fail. However, the user can prevent this by invoking
    // RETIRE() before exit.
    switch (state) {
        case State::Unsatisfied:
            break;
        case State::SatisfiedByUser:
            break;
        case State::SatisfiedByExit:
            state = State::Oversaturated;
            break;
        case State::Oversaturated:
            break;
    };

    return state == State::Oversaturated;  // proceed to interior matching
}

}  // namespace internal
}  // namespace testing
