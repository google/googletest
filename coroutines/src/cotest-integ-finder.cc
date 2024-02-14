#include "cotest/internal/cotest-integ-finder.h"

#include <functional>
#include <iostream>

#include "cotest/internal/cotest-crf-core.h"
#include "cotest/internal/cotest-crf-launch.h"
#include "cotest/internal/cotest-crf-payloads.h"
#include "cotest/internal/cotest-crf-test.h"
#include "cotest/internal/cotest-integ-finder.h"
#include "cotest/internal/cotest-util-logging.h"

namespace testing {
namespace internal {

using coro_impl::PtrToString;

MockHandler::MockHandler() {
    MutexLock l(&g_gmock_mutex);
    CotestMockHandlerPool::GetOrCreateInstance()->AddOwnerLocked(this);
}

MockHandler::~MockHandler() {
    MutexLock l(&g_gmock_mutex);
    CotestMockHandlerPool::GetOrCreateInstance()->RemoveOwnerLocked(this);
}

CotestMockHandlerPool *CotestMockHandlerPool::GetOrCreateInstance() {
    if (!instance) instance = new CotestMockHandlerPool();

    auto cem = static_cast<CotestMockHandlerPool *>(instance);
    return cem;
}

void CotestMockHandlerPool::AddOwnerLocked(MockHandler *owner) { owners.insert(owner); }

void CotestMockHandlerPool::RemoveOwnerLocked(MockHandler *owner) { owners.erase(owner); }

void CotestMockHandlerPool::AddExpectation(std::function<void(void)> creator) {
    // Rather than increment once for each new expectation, we increment twice
    // around untyped watchers and not at all for other exps/watchers. This should
    // help to reduce wraps in light of legacy tests with very large numbers of
    // mocker exps. There should not be large numbers of untyped (=wildcard)
    // watchers because the complexity should be in their coroutines.
    next_global_priority++;

    creator();
    got_untyped_watchers = true;

    next_global_priority++;
}

void CotestMockHandlerPool::PreMockUnlocked(const UntypedFunctionMockerBase *mocker, const void *mock_obj,
                                            const char *name) {
    std::clog << "CotestMockHandlerPool::PreMockUnlocked() call is " << name << std::endl;
    const std::shared_ptr<crf::MockSource> mock_source =
        crf::LaunchCoroutinePool::GetInstance()->FindActiveMockSource();
    const auto crf_mock_routing_session =
        mock_source->CreateMockRoutingSession(static_cast<crf::UntypedMockerPointer>(mocker), mock_obj, name);

    crf_mock_routing_session->PreMockUnlocked();
}

void CotestMockHandlerPool::ForAll(ForTestCoroutineLambda &&lambda) {
    for (MockHandler *o : owners) {
        auto crf_sp = o->GetCRFTestCoroutine();
        if (crf_sp && !crf_sp->IsCoroutineExited()) lambda(std::move(crf_sp));
    }
}

bool CotestMockHandlerPool::IsUninteresting(const UntypedFunctionMockerBase *mocker, const void *untyped_args) const {
    MutexLock l(&g_gmock_mutex);
    // For now, assume any untyped watcher is full wild, making all calls
    // interesting
    return !got_untyped_watchers && mocker->GetMockHandlerScheme()->empty();
}

ExpectationBase *CotestMockHandlerPool::FindMatchingExpectationLocked(const UntypedFunctionMockerBase *mocker,
                                                                      const void *untyped_args,
                                                                      bool *is_mocker_exp) const {
    std::vector<const MockHandlerScheme *> schemes(1);
    schemes[0] = mocker->GetMockHandlerScheme();  // Be at index 0
    for (const MockHandler *o : owners) {
        schemes.push_back(o->GetMockHandlerScheme());
    }

    auto predicate = [&](ExpectationBase *exp) { return exp->ShouldHandleCall(mocker, untyped_args); };

    unsigned which = 0;
    auto exp = Finder(schemes, predicate, &which);
    *is_mocker_exp = (which == 0);
    return exp;
}

ExpectationBase *CotestMockHandlerPool::Finder(std::vector<const MockHandlerScheme *> &schemes,
                                               ShouldHandleCallPredicate predicate, unsigned *which) {
    std::vector<typename MockHandlerScheme::const_reverse_iterator> its;
    std::map<Priority, unsigned> state;

    for (unsigned i = 0; i < schemes.size(); i++) {
        const MockHandlerScheme *scheme = schemes.at(i);
        its.push_back(scheme->rbegin());  // begin with highest prio expectation in this scheme
        if (!scheme->empty()) {
            Priority initial_prio = (*its.at(i))->GetPriority();  // Prio of last element is highest
            COTEST_ASSERT(state.count(initial_prio) == 0);        // no prio value should be in more than one scheme
            state[initial_prio] = i;
        }
    }

    int num_remaining_schemes = state.size();
    while (num_remaining_schemes >= 1) {
        const auto state_it = std::prev(state.end());
        const Priority p = state_it->first;
        const unsigned i = state_it->second;  // index of scheme with highest next prio
        auto &it = its.at(i);                 // Its iterator
        if (predicate(it->get())) {
            // Found it
            *which = i;
            return it->get();
        }
        state.erase(state_it);
        const MockHandlerScheme *scheme = schemes.at(i);
        ++it;
        if (it == scheme->rend()) {
            // this scheme has run out of exps
            num_remaining_schemes--;
        } else {
            // this scheme has more, so re-insert
            Priority new_p = (*it)->GetPriority();
            COTEST_ASSERT(new_p <= p);  // prirorities must be ordered with each
                                        // scheme, and we're working downwards in prio
            state[new_p] = i;
        }
    }
    return nullptr;
}

}  // namespace internal
}  // namespace testing
