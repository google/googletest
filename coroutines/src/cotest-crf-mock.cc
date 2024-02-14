#include "cotest/internal/cotest-crf-mock.h"

#include <memory>

#include "cotest/internal/cotest-crf-core.h"
#include "cotest/internal/cotest-crf-launch.h"
#include "cotest/internal/cotest-crf-synch.h"
#include "cotest/internal/cotest-crf-test.h"
#include "cotest/internal/cotest-integ-finder.h"
#include "cotest/internal/cotest-util-logging.h"

namespace testing {
namespace crf {

using coro_impl::PtrToString;

MockRoutingSession::MockRoutingSession(UntypedMockerPointer mocker_, UntypedMockObjectPointer mock_object_,
                                       std::string name_)
    : mocker(mocker_), mock_object(mock_object_), name(name_) {}

void MockRoutingSession::Configure(TestCoroutine *handling_coroutine_) { handling_coroutine = handling_coroutine_; }

void MockRoutingSession::PreMockUnlocked() {
    // Note: mock calls are not necessarily handled by the test coroutine that
    // owns the mock source. Indeed, when the mock source is main, no test
    // coroutine owns it. At pre-mock stage, we don't yet know which test coro
    // will handle the call.
    auto pre_call_payload = MakePayload<PreMockPayload>(shared_from_this(), name, mock_object, mocker);

    std::unique_ptr<Payload> response_payload = SendMessageToSynchroniser(std::move(pre_call_payload));

    COTEST_ASSERT(response_payload && response_payload->GetKind() == PayloadKind::PreMockAck);
    // We can't get the mock call session from this message, because the
    // synchroniser sent it to all the test coroutines - we don't know which will
    // handle until one accepts.
}

bool MockRoutingSession::SeenMockCallLocked(UntypedArgsPointer args) {
    // This is the mutexed iteration for filtering - in MT applications, we need
    // to get a DROP_MOCK or ACCEPT_MOCK while mutex is held.
    // We may get called on behalf of multiple candidate test coroutines, before
    // we know which will handle the call.

    if (handlers_that_dropped.count(handling_coroutine) > 0)
        return false;  // Multiple overlapping watches: drop again immediately

    auto mock_call_payload = MakePayload<MockSeenPayload>(shared_from_this(), args, name, mock_object, mocker);

    std::unique_ptr<Payload> response_payload = SendMessageToHandlingCoro(std::move(mock_call_payload));
    COTEST_ASSERT(response_payload);

    const PayloadKind kind = response_payload->GetKind();
    auto mock_response_payload = SpecialisePayload<MockResponsePayload>(std::move(response_payload));
    COTEST_ASSERT(mock_response_payload->GetOriginator().lock() = shared_from_this());

    switch (kind) {
        case PayloadKind::DropMock: {
            // TestCoroutine dropped the call: call not handled, there will not be a
            // return
            auto drop_payload = SpecialisePayload<DropMockPayload>(std::move(mock_response_payload));
            COTEST_ASSERT(drop_payload->GetOriginator().lock().get() == this);
            handlers_that_dropped.insert(handling_coroutine);
            return false;
        }
        case PayloadKind::AcceptMock: {
            // TestCoroutine accepted: can release mutex and let coroutine do actions
            auto accept_payload = SpecialisePayload<AcceptMockPayload>(std::move(mock_response_payload));
            COTEST_ASSERT(accept_payload->GetOriginator().lock().get() == this);
            call_session = accept_payload->GetResponder();
            return true;
        }
        case PayloadKind::ReturnMock:
            COTEST_ASSERT(!"Accept required before return");
        case PayloadKind::TCExited:
            COTEST_ASSERT(false);  // Can't exit while handling a mock call
            return false;
        case PayloadKind::Launch:
        case PayloadKind::PreMockAck:
        case PayloadKind::ResumeMain:
        case PayloadKind::PreMock:
        case PayloadKind::MockSeen:
        case PayloadKind::MockAction:
        case PayloadKind::LaunchResult:
        case PayloadKind::TCDestructing:
            COTEST_ASSERT(!"Bad response to mock call");
    }
    return false;
}

UntypedReturnValuePointer MockRoutingSession::ActionsAndReturnUnlocked() {
    // This is the un-mutexed iteration for actions - in MT applications, mutex
    // is released to avoid deadlocks.
    COTEST_ASSERT(!handling_coroutine->IsCoroutineExited());
    auto action_payload = MakePayload<MockActionPayload>(shared_from_this(), call_session);

    std::unique_ptr<Payload> response_payload = SendMessageToHandlingCoro(std::move(action_payload));

    COTEST_ASSERT(!handling_coroutine->IsCoroutineExited());
    COTEST_ASSERT(response_payload);

    COTEST_ASSERT(response_payload->GetKind() == PayloadKind::ReturnMock &&
                  "Coroutine must return mock call from main before main can "
                  "supply more events");
    auto return_payload = SpecialisePayload<ReturnMockPayload>(std::move(response_payload));
    COTEST_ASSERT(return_payload->GetOriginator().lock().get() == this);

    // Check return came from the correct coroutine
    COTEST_ASSERT(return_payload->GetResponder().lock() == call_session.lock());

    return return_payload->GetResult();
}

TestCoroutine *MockRoutingSession::GetHandlingTestCoro() const { return handling_coroutine; }

std::string MockRoutingSession::GetName() const { return name; }

std::shared_ptr<MockSource> ExteriorMockRS::GetMockSource() const { return ProxyForMain::GetInstance(); }

std::unique_ptr<Payload> ExteriorMockRS::SendMessageToSynchroniser(std::unique_ptr<Payload> &&to_tc) const {
    // We're in main, so start the message loop and provide destination
    return MessageNode::SendMessageFromMain(PreMockSynchroniser::GetInstance(), std::move(to_tc));
}

std::unique_ptr<Payload> ExteriorMockRS::SendMessageToHandlingCoro(std::unique_ptr<Payload> &&to_tc) const {
    // We're in main, so start the message loop and provide destination
    return MessageNode::SendMessageFromMain(GetHandlingTestCoro(), std::move(to_tc));
}

std::shared_ptr<MockSource> InteriorMockRS::GetMockSource() const {
    auto msl = launch_coro.lock();
    COTEST_ASSERT(msl);
    return msl;
}

InteriorMockRS::InteriorMockRS(std::shared_ptr<LaunchCoroutine> launch_coro_, UntypedMockerPointer mocker_,
                               UntypedMockObjectPointer mock_object_, std::string name_)
    : MockRoutingSession(mocker_, mock_object_, name_), launch_coro(launch_coro_) {}

std::unique_ptr<Payload> InteriorMockRS::SendMessageToSynchroniser(std::unique_ptr<Payload> &&to_tc) const {
    // We're in launch coro interior, so yield
    auto dcl = launch_coro.lock();
    COTEST_ASSERT(dcl);
    return dcl->Yield(std::move(to_tc));
}

std::unique_ptr<Payload> InteriorMockRS::SendMessageToHandlingCoro(std::unique_ptr<Payload> &&to_tc) const {
    // We're in launch coro interior, so yield
    auto dcl = launch_coro.lock();
    COTEST_ASSERT(dcl);
    return dcl->Yield(std::move(to_tc));
}

}  // namespace crf
}  // namespace testing
