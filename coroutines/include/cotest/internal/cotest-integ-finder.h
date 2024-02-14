#ifndef COROUTINES_INCLUDE_CORO_COTEST_INTEG_FINDER_H_
#define COROUTINES_INCLUDE_CORO_COTEST_INTEG_FINDER_H_

#include <cstdint>
#include <exception>
#include <iostream>
#include <map>
#include <set>

#include "cotest-crf-core.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace testing {
namespace internal {

using MockHandlerScheme = UntypedFunctionMockerBase::UntypedExpectations;

class MockHandler {
   protected:
    MockHandler();
    MockHandler(const MockHandler &) = default;
    MockHandler(MockHandler &&) = default;
    MockHandler &operator=(const MockHandler &) = default;
    MockHandler &operator=(MockHandler &&) = default;
    virtual ~MockHandler();

   public:
    virtual const MockHandlerScheme *GetMockHandlerScheme() const = 0;
    virtual std::string GetName() const = 0;
    virtual std::shared_ptr<crf::TestCoroutine> GetCRFTestCoroutine() = 0;
};

class CotestMockHandlerPool : public AlternateMockCallManager {
   public:
    using ShouldHandleCallPredicate = std::function<bool(ExpectationBase *)>;
    using ForTestCoroutineLambda = std::function<void(std::shared_ptr<crf::TestCoroutine> &&crf_sp)>;

    static CotestMockHandlerPool *GetOrCreateInstance();

    void AddOwnerLocked(MockHandler *owner);
    void RemoveOwnerLocked(MockHandler *owner);
    void AddExpectation(std::function<void(void)> creator);

    void PreMockUnlocked(const UntypedFunctionMockerBase *mocker, const void *mock_obj, const char *name) override;
    void ForAll(ForTestCoroutineLambda &&lambda);

    // Cannot be a template since we want a virtual interface to keep the
    // dependency into gmock weak
    bool IsUninteresting(const UntypedFunctionMockerBase *mocker, const void *untyped_args) const override;

    ExpectationBase *FindMatchingExpectationLocked(const UntypedFunctionMockerBase *mocker, const void *untyped_args,
                                                   bool *is_mocker_exp) const override;

    static ExpectationBase *Finder(std::vector<const MockHandlerScheme *> &schemes, ShouldHandleCallPredicate predicate,
                                   unsigned *which);

   private:
    bool got_untyped_watchers = false;
    std::set<MockHandler *> owners;
};

}  // namespace internal
}  // namespace testing

#endif
