#ifndef COROUTINES_INCLUDE_CORO_INTERNAL_COTEST_CRF_LAUNCH_H_
#define COROUTINES_INCLUDE_CORO_INTERNAL_COTEST_CRF_LAUNCH_H_

#include <map>
#include <memory>
#include <queue>
#include <string>

#include "cotest-crf-core.h"
#include "cotest-crf-mock.h"
#include "cotest-crf-payloads.h"
#include "cotest-util-types.h"

namespace testing {
namespace crf {

// ------------------ Classes ------------------

class MockRoutingSession;

class InteriorEventSession;

template <typename T>
class InteriorSignatureMockCS;

class InteriorLaunchSessionBase;

template <typename T>
class InteriorLaunchSession;

class TestCoroutine;

class LaunchCoroutine final : public CoroutineBase,
                              public MockSource,
                              public std::enable_shared_from_this<LaunchCoroutine> {
   public:
    LaunchCoroutine(std::string name_);
    ~LaunchCoroutine();

    void Body();
    ReplyPair ReceiveMessage(std::unique_ptr<Payload> &&to_node) final;
    ReplyPair IterateServer(std::unique_ptr<Payload> &&to_coro);
    std::shared_ptr<MockRoutingSession> CreateMockRoutingSession(UntypedMockerPointer mocker_,
                                                                 UntypedMockObjectPointer mock_obj_,
                                                                 const char *name_) final;
    std::shared_ptr<InteriorLaunchSessionBase> TryGetCurrentLaunchSession();
    std::shared_ptr<const InteriorLaunchSessionBase> TryGetCurrentLaunchSession() const;
    LaunchCoroutine *GetAsCoroutine() final;
    const LaunchCoroutine *GetAsCoroutine() const final;
    std::string DebugString() const final;

   private:
    std::weak_ptr<InteriorLaunchSessionBase> current_launch_session;
};

class LaunchCoroutinePool final : public virtual MessageNode {
   public:
    using PoolType = std::map<coro_impl::InteriorInterface *, std::shared_ptr<LaunchCoroutine>>;

    LaunchCoroutine *TryGetUnusedLaunchCoro();
    LaunchCoroutine *Allocate(std::string launch_text);
    std::shared_ptr<MockSource> FindActiveMockSource();
    ReplyPair ReceiveMessage(std::unique_ptr<Payload> &&to_node) final;
    std::string DebugString() const final;
    int CleanUp();

    static LaunchCoroutinePool *GetInstance();

   private:
    PoolType pool;
};

}  // namespace crf
}  // namespace testing

#endif
