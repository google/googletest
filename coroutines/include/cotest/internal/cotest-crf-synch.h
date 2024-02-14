#ifndef COROUTINES_INCLUDE_CORO_INTERNAL_COTEST_CRF_SYNCH_H_
#define COROUTINES_INCLUDE_CORO_INTERNAL_COTEST_CRF_SYNCH_H_

#include <memory>
#include <queue>

#include "cotest-crf-core.h"
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

class PreMockSynchroniser : public virtual MessageNode {
   public:
    ReplyPair ReceiveMessage(std::unique_ptr<Payload> &&to_node) override;
    std::string DebugString() const override;

    static PreMockSynchroniser *GetInstance();

   private:
    enum class State { Idle, PassToMain, Start, Working, Complete, WaitingForAck };
    State state = State::Idle;

    std::unique_ptr<PreMockPayload> current_pre_mock;
    std::queue<std::weak_ptr<TestCoroutine>> send_pm_to;
    static PreMockSynchroniser instance;
};

}  // namespace crf
}  // namespace testing

#endif
