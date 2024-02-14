#ifndef COROUTINES_INCLUDE_CORO_INTERNAL_COTEST_CRF_PAYLOADS_H_
#define COROUTINES_INCLUDE_CORO_INTERNAL_COTEST_CRF_PAYLOADS_H_

#include <memory>
#include <string>

#include "cotest-coro-common.h"
#include "cotest-util-logging.h"
#include "cotest-util-types.h"

namespace testing {
namespace crf {

using UntypedMockerPointer = const void *;
using UntypedMockObjectPointer = const void *;
using UntypedArgsPointer = const void *;
using UntypedReturnValuePointer = const void *;

// ------------------ Classes ------------------

class MockSource;
class TestCoroutine;
class LaunchCoroutine;
class MockRoutingSession;
class InteriorEventSession;
class InteriorLaunchSessionBase;
class InteriorMockCallSession;
class InteriorLaunchResultSession;

template <typename T>
class InteriorLaunchSession;

enum class PayloadKind {
    PreMock = 1000,  // =1000 just to make valid values easy to spot in debugger
    PreMockAck,
    MockSeen,
    DropMock,
    AcceptMock,
    MockAction,
    ReturnMock,
    Launch,
    LaunchResult,
    ResumeMain,
    TCExited,
    TCDestructing
};

class Payload : public coro_impl::Payload {
   public:
    virtual PayloadKind GetKind() const = 0;
};

class PreMockPayload final : public Payload {
   public:
    PreMockPayload(std::weak_ptr<MockRoutingSession> originator_, std::string name_,
                   UntypedMockObjectPointer mock_object_, UntypedMockerPointer mocker_);
    PayloadKind GetKind() const final;
    PreMockPayload *Clone() const;
    std::weak_ptr<MockRoutingSession> GetOriginator() const;
    std::string GetName() const;
    UntypedMockObjectPointer GetMockObject() const;
    UntypedMockerPointer GetMocker() const;
    std::string DebugString() const final;

   private:
    const std::weak_ptr<MockRoutingSession> originator;
    const std::string name;
    const UntypedMockObjectPointer mock_object;
    const UntypedMockerPointer mocker;
};

class PreMockAckPayload : public Payload {
   public:
    explicit PreMockAckPayload(std::weak_ptr<MockRoutingSession> originator_);
    PayloadKind GetKind() const;
    std::weak_ptr<MockRoutingSession> GetOriginator() const;
    std::string DebugString() const final;

   private:
    const std::weak_ptr<MockRoutingSession> originator;
};

class MockSeenPayload final : public Payload {
   public:
    MockSeenPayload(std::weak_ptr<MockRoutingSession> originator_, UntypedArgsPointer args_, std::string name_,
                    UntypedMockObjectPointer mock_object_, UntypedMockerPointer mocker_);
    PayloadKind GetKind() const final;
    std::weak_ptr<MockRoutingSession> GetOriginator() const;
    UntypedArgsPointer GetArgsUntyped() const;
    std::string GetName() const;
    UntypedMockObjectPointer GetMockObject() const;
    UntypedMockerPointer GetMocker() const;
    std::string DebugString() const final;

   private:
    const std::weak_ptr<MockRoutingSession> originator;
    const UntypedArgsPointer args;
    const std::string name;
    const UntypedMockObjectPointer mock_object;
    const UntypedMockerPointer mocker;
};

class MockResponsePayload : public Payload {
   public:
    MockResponsePayload(std::weak_ptr<MockRoutingSession> originator_,
                        std::weak_ptr<InteriorMockCallSession> responder_);
    std::weak_ptr<MockRoutingSession> GetOriginator() const;
    std::weak_ptr<InteriorMockCallSession> GetResponder() const;

   protected:
    const std::weak_ptr<MockRoutingSession> originator;
    const std::weak_ptr<InteriorMockCallSession> responder;
};

class DropMockPayload final : public MockResponsePayload {
   public:
    using MockResponsePayload::MockResponsePayload;
    PayloadKind GetKind() const final;
    std::string DebugString() const final;
};

class AcceptMockPayload final : public MockResponsePayload {
   public:
    using MockResponsePayload::MockResponsePayload;
    PayloadKind GetKind() const final;
    std::string DebugString() const final;
};

class MockActionPayload final : public MockResponsePayload {
   public:
    using MockResponsePayload::MockResponsePayload;
    PayloadKind GetKind() const final;
    std::string DebugString() const final;
};

class ReturnMockPayload final : public MockResponsePayload {
   public:
    ReturnMockPayload(std::weak_ptr<MockRoutingSession> originator_, std::weak_ptr<InteriorMockCallSession> responder_,
                      UntypedReturnValuePointer result_);
    PayloadKind GetKind() const final;
    UntypedReturnValuePointer GetResult();
    std::string DebugString() const final;

   private:
    const UntypedReturnValuePointer return_val_ptr;
};

class LaunchPayload final : public Payload {
   public:
    LaunchPayload(std::weak_ptr<InteriorLaunchSessionBase> originator_,
                  internal::LaunchLambdaWrapperType wrapper_lambda_, std::string name_);
    PayloadKind GetKind() const final;
    std::weak_ptr<InteriorLaunchSessionBase> GetOriginator() const;
    internal::LaunchLambdaWrapperType GetLambdaWrapper() const;
    std::string GetName() const;
    std::string DebugString() const final;

   private:
    const std::weak_ptr<InteriorLaunchSessionBase> originator;
    const internal::LaunchLambdaWrapperType wrapper_lambda;
    const std::string name;
};

class LaunchResultPayload final : public Payload {
   public:
    LaunchResultPayload(std::weak_ptr<InteriorLaunchSessionBase> originator_, std::weak_ptr<LaunchCoroutine> responder_,
                        UntypedReturnValuePointer result_);
    PayloadKind GetKind() const final;
    std::weak_ptr<InteriorLaunchSessionBase> GetOriginator() const;
    std::weak_ptr<LaunchCoroutine> GetResponder() const;
    UntypedReturnValuePointer GetResult();
    std::string DebugString() const final;

   private:
    const std::weak_ptr<InteriorLaunchSessionBase> originator;
    const std::weak_ptr<LaunchCoroutine> responder;
    const UntypedReturnValuePointer return_val_ptr;
};

class ResumeMainPayload : public Payload {
   public:
    explicit ResumeMainPayload(std::weak_ptr<TestCoroutine> originator_);
    PayloadKind GetKind() const;
    std::weak_ptr<TestCoroutine> GetOriginator() const;
    std::string DebugString() const final;

   private:
    const std::weak_ptr<TestCoroutine> originator;
};

// Test Coroutine Exited
class TCExitedPayload : public Payload {
   public:
    explicit TCExitedPayload(std::weak_ptr<TestCoroutine> originator_);
    PayloadKind GetKind() const;
    std::weak_ptr<TestCoroutine> GetOriginator() const;
    std::string DebugString() const final;

   private:
    const std::weak_ptr<TestCoroutine> originator;
};

// Test Coroutine Destructing
class TCDestructingPayload : public Payload {
   public:
    explicit TCDestructingPayload(std::weak_ptr<TestCoroutine> originator_);
    PayloadKind GetKind() const;
    std::weak_ptr<TestCoroutine> GetOriginator() const;
    std::string DebugString() const final;

   private:
    const std::weak_ptr<TestCoroutine> originator;
};

}  // namespace crf
}  // namespace testing

#endif
