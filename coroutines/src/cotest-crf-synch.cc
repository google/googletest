#include "cotest/internal/cotest-crf-synch.h"

#include <memory>

#include "cotest/internal/cotest-crf-core.h"
#include "cotest/internal/cotest-crf-mock.h"
#include "cotest/internal/cotest-crf-test.h"
#include "cotest/internal/cotest-integ-finder.h"
#include "cotest/internal/cotest-util-logging.h"

namespace testing {
namespace crf {

using coro_impl::PtrToString;

MessageNode::ReplyPair PreMockSynchroniser::ReceiveMessage(std::unique_ptr<Payload> &&to_node) {
    // Fall-through machine with early return on reply.
    // Each state action is inside its own if-statement, meaning that multiple
    // actions and state transitions can complete in a signle iteration. The order
    // of the if-statement is immaterial to the algorithm implmented by the
    // machine but does affect the states that can lead to an iteration
    // completing. Here we choose an ordering that does not end the iteration
    // until a reply message is ready. Then do ensure we do end in this case, we
    // use early returns.

    if (state == State::Idle) {
        // Handle incoming messages when we don't require acknowledgement
        switch (to_node->GetKind()) {
            case PayloadKind::PreMock:
                COTEST_ASSERT(!current_pre_mock);
                current_pre_mock = SpecialisePayload<PreMockPayload>(std::move(to_node));
                state = State::Start;
                break;
            case PayloadKind::PreMockAck:
                COTEST_ASSERT(!"Not expecting PRE_MOCK_ACK in State::Idle");
            case PayloadKind::ResumeMain:
            case PayloadKind::TCExited:
                state = State::PassToMain;
                break;
            case PayloadKind::MockSeen:
            case PayloadKind::AcceptMock:
            case PayloadKind::MockAction:
            case PayloadKind::DropMock:
            case PayloadKind::ReturnMock:
            case PayloadKind::Launch:
            case PayloadKind::LaunchResult:
            case PayloadKind::TCDestructing:
                COTEST_ASSERT(!"Unhanled message in State::Idle");
        }
    }

    if (state == State::WaitingForAck) {
        // Handle incoming messages when we require an acknowledgement
        switch (to_node->GetKind()) {
            case PayloadKind::PreMock:
                COTEST_ASSERT(!"Not expecting another PRE_MOCK while State::WaitingForAck. A TC may be delaying acknowlegement of PM");
                break;
            case PayloadKind::PreMockAck: {
                auto pmack = SpecialisePayload<PreMockAckPayload>(std::move(to_node));
                auto mock_source = pmack->GetOriginator().lock();
                auto expected_mock_source = current_pre_mock->GetOriginator().lock();
                COTEST_ASSERT(mock_source && expected_mock_source && "Mock source expired while synchronsing PRE_MOCK");
                COTEST_ASSERT(mock_source == expected_mock_source);  // should relate to the same mock source
                state = send_pm_to.empty() ? State::Complete : State::Working;
                break;
            }
            case PayloadKind::ResumeMain:
                // If we're waiting for an ack from a coroutine and instead get that it
                // has nothing to do, we may be headed for a deadlock. Pass to main in
                // case main can progress things. This path not covered by tests at the
                // time of writing.
                state = State::PassToMain;
                break;
            case PayloadKind::TCExited:
                // Discard ack requirement for exited coro
                state = send_pm_to.empty() ? State::Complete : State::Working;
                break;
            case PayloadKind::MockSeen:
            case PayloadKind::AcceptMock:
            case PayloadKind::MockAction:
            case PayloadKind::DropMock:
            case PayloadKind::ReturnMock:
            case PayloadKind::Launch:
            case PayloadKind::LaunchResult:
            case PayloadKind::TCDestructing:
                COTEST_ASSERT(!"Unhanled message in State::WaitingForAck");
        }
    }

    if (state == State::PassToMain) {
        // Forward the message to main
        state = State::Idle;
        return std::make_pair(std::move(to_node), ProxyForMain::GetInstance().get());
    }

    if (state == State::Start) {
        // Start a new synchronisation cycle
        auto pool = internal::CotestMockHandlerPool::GetOrCreateInstance();
        pool->ForAll([this](std::shared_ptr<TestCoroutine> &&crf_sp) { send_pm_to.push(std::move(crf_sp)); });
        state = send_pm_to.empty() ? State::Complete : State::Working;
    }

    if (state == State::Working) {
        // Continue the current synchronisation cycle
        COTEST_ASSERT(!send_pm_to.empty());
        ReplyPair p;
        p.first = std::unique_ptr<PreMockPayload>(current_pre_mock->Clone());
        auto pm_to_locked = send_pm_to.front().lock();
        COTEST_ASSERT(pm_to_locked);
        p.second = pm_to_locked.get();
        send_pm_to.pop();
        state = State::WaitingForAck;
        return p;
    }

    if (state == State::Complete) {
        // Complete the synchronisation cycle
        COTEST_ASSERT(send_pm_to.empty());
        auto mock_call_session = current_pre_mock->GetOriginator().lock();
        current_pre_mock.reset();
        state = State::Idle;
        COTEST_ASSERT(mock_call_session);
        // Reply to current mock source
        return std::make_pair(MakePayload<PreMockAckPayload>(mock_call_session),
                              mock_call_session->GetMockSource().get());
    }
    COTEST_ASSERT(!"We have to send a reply message on each iteration");
}

std::string PreMockSynchroniser::DebugString() const {
    std::stringstream ss;
    ss << "PreMockSynchroniser(PM=" << current_pre_mock.get() << " #TCs=" << send_pm_to.size() << ")";
    return ss.str();
}

PreMockSynchroniser *PreMockSynchroniser::GetInstance() { return &instance; }

PreMockSynchroniser PreMockSynchroniser::instance;

}  // namespace crf
}  // namespace testing
