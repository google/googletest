#include "cotest/internal/cotest-crf-payloads.h"

#include <memory>

#include "cotest/internal/cotest-util-logging.h"

namespace testing {
namespace crf {

using coro_impl::PtrToString;

PreMockPayload::PreMockPayload(std::weak_ptr<MockRoutingSession> originator_, std::string name_,
                               UntypedMockObjectPointer mock_object_, UntypedMockerPointer mocker_)
    : originator(originator_), name(name_), mock_object(mock_object_), mocker(mocker_) {}

PayloadKind PreMockPayload::GetKind() const { return PayloadKind::PreMock; }

PreMockPayload *PreMockPayload::Clone() const { return new PreMockPayload(originator, name, mock_object, mocker); }

std::weak_ptr<MockRoutingSession> PreMockPayload::GetOriginator() const { return originator; }

std::string PreMockPayload::GetName() const {
    COTEST_ASSERT(!name.empty());
    return name;
}

UntypedMockObjectPointer PreMockPayload::GetMockObject() const {
    COTEST_ASSERT(mock_object);
    return mock_object;
}

UntypedMockerPointer PreMockPayload::GetMocker() const {
    COTEST_ASSERT(mocker);
    return mocker;
}

std::string PreMockPayload::DebugString() const {
    return "PayloadKind::PreMock(O=" + PtrToString(originator.lock().get()) + ", \"" + name + "\"" +
           ", mo=" + PtrToString(mock_object) + ", m=" + PtrToString(mocker) + ")";
}

PreMockAckPayload::PreMockAckPayload(std::weak_ptr<MockRoutingSession> originator_) : originator(originator_) {}

std::weak_ptr<MockRoutingSession> PreMockAckPayload::GetOriginator() const { return originator; }

PayloadKind PreMockAckPayload::GetKind() const { return PayloadKind::PreMockAck; }

std::string PreMockAckPayload::DebugString() const {
    return "PayloadKind::PreMockAck(O=" + PtrToString(originator.lock().get()) + ")";
}

MockSeenPayload::MockSeenPayload(std::weak_ptr<MockRoutingSession> originator_, UntypedArgsPointer args_,
                                 std::string name_, UntypedMockObjectPointer mock_object_, UntypedMockerPointer mocker_)
    : originator(originator_), args(args_), name(name_), mock_object(mock_object_), mocker(mocker_) {}

PayloadKind MockSeenPayload::GetKind() const { return PayloadKind::MockSeen; }

std::weak_ptr<MockRoutingSession> MockSeenPayload::GetOriginator() const { return originator; }

UntypedArgsPointer MockSeenPayload::GetArgsUntyped() const { return args; }

std::string MockSeenPayload::GetName() const {
    COTEST_ASSERT(!name.empty());
    return name;
}

UntypedMockObjectPointer MockSeenPayload::GetMockObject() const {
    COTEST_ASSERT(mock_object);
    return mock_object;
}

UntypedMockerPointer MockSeenPayload::GetMocker() const {
    COTEST_ASSERT(mocker);
    return mocker;
}

std::string MockSeenPayload::DebugString() const {
    return "PayloadKind::MockSeen(O=" + PtrToString(originator.lock().get()) + ", a=" + PtrToString(args) + ", \"" +
           name + "\"" + ", mo=" + PtrToString(mock_object) + ", m=" + PtrToString(mocker) + ")";
}

MockResponsePayload::MockResponsePayload(std::weak_ptr<MockRoutingSession> originator_,
                                         std::weak_ptr<InteriorMockCallSession> responder_)
    : originator(originator_), responder(responder_) {}

std::weak_ptr<MockRoutingSession> MockResponsePayload::GetOriginator() const { return originator; }

std::weak_ptr<InteriorMockCallSession> MockResponsePayload::GetResponder() const { return responder; }

PayloadKind DropMockPayload::GetKind() const { return PayloadKind::DropMock; }

std::string DropMockPayload::DebugString() const {
    return "PayloadKind::DropMock(O=" + PtrToString(originator.lock().get()) +
           ", R=" + PtrToString(responder.lock().get()) + ")";
}

PayloadKind AcceptMockPayload::GetKind() const { return PayloadKind::AcceptMock; }

std::string AcceptMockPayload::DebugString() const {
    return "PayloadKind::AcceptMock(O=" + PtrToString(originator.lock().get()) +
           ", R=" + PtrToString(responder.lock().get()) + ")";
}

PayloadKind MockActionPayload::GetKind() const { return PayloadKind::MockAction; }

std::string MockActionPayload::DebugString() const {
    return "PayloadKind::MockAction(O=" + PtrToString(originator.lock().get()) +
           ", R=" + PtrToString(responder.lock().get()) + ")";
}

ReturnMockPayload::ReturnMockPayload(std::weak_ptr<MockRoutingSession> originator_,
                                     std::weak_ptr<InteriorMockCallSession> responder_,
                                     UntypedReturnValuePointer return_val_ptr_)
    : MockResponsePayload(originator_, responder_), return_val_ptr(return_val_ptr_) {}

PayloadKind ReturnMockPayload::GetKind() const { return PayloadKind::ReturnMock; }

UntypedReturnValuePointer ReturnMockPayload::GetResult() { return return_val_ptr; }

std::string ReturnMockPayload::DebugString() const {
    return "PayloadKind::ReturnMock(O=" + PtrToString(originator.lock().get()) +
           ", R=" + PtrToString(responder.lock().get()) + ", rv=" + PtrToString(return_val_ptr) + ")";
}

LaunchPayload::LaunchPayload(std::weak_ptr<InteriorLaunchSessionBase> originator_,
                             internal::LaunchLambdaWrapperType wrapper_lambda_, std::string name_)
    : originator(originator_), wrapper_lambda(wrapper_lambda_), name(name_) {}

PayloadKind LaunchPayload::GetKind() const { return PayloadKind::Launch; }

std::weak_ptr<InteriorLaunchSessionBase> LaunchPayload::GetOriginator() const { return originator; }

internal::LaunchLambdaWrapperType LaunchPayload::GetLambdaWrapper() const { return wrapper_lambda; }

std::string LaunchPayload::GetName() const { return name; }

std::string LaunchPayload::DebugString() const {
    return "PayloadKind::Launch(O=" + PtrToString(originator.lock().get()) + ", \"" + name + "\"" + ")";
}

LaunchResultPayload::LaunchResultPayload(std::weak_ptr<InteriorLaunchSessionBase> originator_,
                                         std::weak_ptr<LaunchCoroutine> responder_,
                                         UntypedReturnValuePointer return_val_ptr_)
    : originator(originator_), responder(responder_), return_val_ptr(return_val_ptr_) {}

PayloadKind LaunchResultPayload::GetKind() const { return PayloadKind::LaunchResult; }

std::weak_ptr<InteriorLaunchSessionBase> LaunchResultPayload::GetOriginator() const { return originator; }

std::weak_ptr<LaunchCoroutine> LaunchResultPayload::GetResponder() const { return responder; }

UntypedReturnValuePointer LaunchResultPayload::GetResult() { return return_val_ptr; }

std::string LaunchResultPayload::DebugString() const {
    return "PayloadKind::LaunchResult(O=" + PtrToString(originator.lock().get()) +
           ", R=" + PtrToString(responder.lock().get()) + ", rv=" + PtrToString(return_val_ptr) + ")";
}

ResumeMainPayload::ResumeMainPayload(std::weak_ptr<TestCoroutine> originator_) : originator(originator_) {}

PayloadKind ResumeMainPayload::GetKind() const { return PayloadKind::ResumeMain; }

std::weak_ptr<TestCoroutine> ResumeMainPayload::GetOriginator() const { return originator; }

std::string ResumeMainPayload::DebugString() const { return "PayloadKind::ResumeMain()"; }

TCExitedPayload::TCExitedPayload(std::weak_ptr<TestCoroutine> originator_) : originator(originator_) {}

PayloadKind TCExitedPayload::GetKind() const { return PayloadKind::TCExited; }

std::weak_ptr<TestCoroutine> TCExitedPayload::GetOriginator() const { return originator; }

std::string TCExitedPayload::DebugString() const { return "PayloadKind::TCExited()"; }

TCDestructingPayload::TCDestructingPayload(std::weak_ptr<TestCoroutine> originator_) : originator(originator_) {}

PayloadKind TCDestructingPayload::GetKind() const { return PayloadKind::TCDestructing; }

std::weak_ptr<TestCoroutine> TCDestructingPayload::GetOriginator() const { return originator; }

std::string TCDestructingPayload::DebugString() const { return "PayloadKind::TCDestructing()"; }

}  // namespace crf
}  // namespace testing
