#include "cotest/internal/cotest-crf-test.h"

#include <memory>

#include "cotest/internal/cotest-crf-core.h"
#include "cotest/internal/cotest-crf-launch.h"
#include "cotest/internal/cotest-crf-synch.h"
#include "cotest/internal/cotest-util-logging.h"

namespace testing {
namespace crf {

using coro_impl::PtrToString;

TestCoroutine::TestCoroutine(coro_impl::BodyFunction cofn_, std::string name_, OnExitFunction on_exit_function_)
    : CoroutineBase(cofn_, name_), on_exit_function(on_exit_function_) {
    // Comment this out for logging
    std::clog.setstate(std::ios_base::failbit);

    std::clog << ActiveStr() << COTEST_THIS << " constructing" << std::endl;
}

TestCoroutine::~TestCoroutine() { std::clog << ActiveStr() << COTEST_THIS << " destructing" << std::endl; }

MessageNode::ReplyPair TestCoroutine::ReceiveMessage(std::unique_ptr<Payload> &&to_node) {
    COTEST_ASSERT(!IsCoroutineExited());
    return IterateServer(std::move(to_node));
}

MessageNode::ReplyPair TestCoroutine::IterateServer(std::unique_ptr<Payload> &&to_coro) {
    COTEST_ASSERT(!IsCoroutineExited());

    extra_iteration_requested = false;  // requirement satisfied by this iteration
    std::unique_ptr<Payload> from_coro = Iterate(std::move(to_coro));

    if (IsCoroutineExited()) {
        on_exit_function();
        from_coro = MakePayload<TCExitedPayload>(shared_from_this());
    }

    if (!from_coro) return std::make_pair(std::move(from_coro), nullptr);

    MessageNode *reply_dest;
    switch (from_coro->GetKind()) {
        case PayloadKind::Launch:
            reply_dest = LaunchCoroutinePool::GetInstance();  // let the pool deal with it
            break;

        case PayloadKind::PreMockAck:
            reply_dest = PreMockSynchroniser::GetInstance();
            break;

        case PayloadKind::DropMock:
        case PayloadKind::AcceptMock:
        case PayloadKind::ReturnMock: {
            auto orig = PeekPayload<MockResponsePayload>(from_coro).GetOriginator().lock();
            COTEST_ASSERT(orig && "Call session expired unexpectedly");
            reply_dest = orig->GetMockSource().get();

            // Coroutine will need to run beyond the cs.RETURN() before we ask it too
            // many questions, in case it needs to SATISFY(), RETIRE() or exit. But we
            // can't iterate it now, since it currently owns the return value. CRF
            // constraints #3 and #4.
            extra_iteration_requested = true;
            break;
        }
        case PayloadKind::ResumeMain:
        case PayloadKind::TCExited:
            reply_dest = PreMockSynchroniser::GetInstance();
            break;
        case PayloadKind::PreMock:
        case PayloadKind::MockSeen:
        case PayloadKind::MockAction:
        case PayloadKind::LaunchResult:
        case PayloadKind::TCDestructing:
            COTEST_ASSERT(!"unhandled message from test coroutine");
    }

    COTEST_ASSERT(reply_dest);
    return std::make_pair(std::move(from_coro), reply_dest);
}

void TestCoroutine::InitialActivity() {
    COTEST_ASSERT(!IsCoroutineExited() && "Cannot reuse coroutine instance");
    std::unique_ptr<Payload> from_coro = SendMessageFromMain(this, nullptr);
    // We should return from this when the coro is blocked, eg waiting for
    // a mock call from somewhere other than a local launch
    COTEST_ASSERT(from_coro->GetKind() == PayloadKind::TCExited || from_coro->GetKind() == PayloadKind::ResumeMain);
}

static MockSource *GetMockSourceFromMessage(const std::unique_ptr<Payload> &payload) {
    COTEST_ASSERT(payload);
    switch (payload->GetKind()) {
        case PayloadKind::PreMock: {
            auto ext_mock_cs = PeekPayload<PreMockPayload>(payload).GetOriginator().lock();
            return ext_mock_cs->GetMockSource().get();
        }
        case PayloadKind::MockSeen: {
            auto ext_mock_cs = PeekPayload<MockSeenPayload>(payload).GetOriginator().lock();
            return ext_mock_cs->GetMockSource().get();
        }
        case PayloadKind::MockAction: {
            auto ext_mock_cs = PeekPayload<MockActionPayload>(payload).GetOriginator().lock();
            return ext_mock_cs->GetMockSource().get();
        }
        case PayloadKind::LaunchResult: {
            return PeekPayload<LaunchResultPayload>(payload).GetResponder().lock().get();
        }
        case PayloadKind::TCDestructing: {
            return ProxyForMain::GetInstance().get();
        }

        case PayloadKind::DropMock:
        case PayloadKind::AcceptMock:
        case PayloadKind::ReturnMock:
        case PayloadKind::Launch:
        case PayloadKind::PreMockAck:
        case PayloadKind::ResumeMain:
        case PayloadKind::TCExited:
            COTEST_ASSERT(!"Unhandled payload type");
            break;
    }
    return nullptr;
}

static std::pair<bool, std::shared_ptr<InteriorLaunchSessionBase>> GetLaunchSessionFromMessage(
    const std::unique_ptr<Payload> &payload) {
    MockSource *ms = GetMockSourceFromMessage(payload);
    if (ms == ProxyForMain::GetInstance().get()) return std::make_pair(true, nullptr);
    auto launch_coro = ms->GetAsCoroutine();
    return std::make_pair(false, launch_coro->TryGetCurrentLaunchSession());
}

void TestCoroutine::YieldServer(std::unique_ptr<Payload> &&from_coro) {
    // These outgoing payloads will release the mock lock
    if (from_coro->GetKind() == PayloadKind::DropMock || from_coro->GetKind() == PayloadKind::AcceptMock)
        mock_call_locked = false;

    std::unique_ptr<Payload> to_coro = Yield(std::move(from_coro));
    COTEST_ASSERT(to_coro);

    if (to_coro->GetKind() == PayloadKind::TCDestructing || to_coro->GetKind() == PayloadKind::MockAction)
        return;  // Discard these here - the interior event or call session ensures
                 // a return will be made

    COTEST_ASSERT(!next_payload && "Internal error: already have an event");
    next_payload = std::move(to_coro);
}

bool TestCoroutine::IsPendingEvent() { return !!next_payload; }

std::shared_ptr<InteriorEventSession> TestCoroutine::NextEvent(const char *file, int line) {
    COTEST_ASSERT(!mock_call_locked &&
                  "Cannot request a new event until mock call has been dropped, "
                  "accepted or returned");

    // Start loping, because we're going to handle NULL messages and PreMock
    // locally
    std::unique_ptr<InteriorMockCallSession> mock_call_event;
    while (!next_payload || next_payload->GetKind() == PayloadKind::PreMock) {
        std::unique_ptr<Payload> response;
        if (!next_payload) {
            std::clog << ActiveStr() << COTEST_THIS_FL
                      << " no event has been sent to this coro, so requesting "
                         "resumption of main test function"
                      << std::endl;
            response = MakePayload<ResumeMainPayload>(shared_from_this());
        } else if (next_payload->GetKind() == PayloadKind::PreMock) {
            // Acknowledge PreMock, and then grab the subsequent message. We can
            // get another PreMock for example if GMock doesn't show us the mock call
            // for the first one. We can also get somehting other than SeenMock this
            // way. PreMock acknowledged here rather than YieldServer() because we
            // need to be sure the test coro is really ready to handle a mock call.
            auto p = GetLaunchSessionFromMessage(next_payload);
            auto pm_payload = SpecialisePayload<PreMockPayload>(std::move(next_payload));
            response = MakePayload<PreMockAckPayload>(pm_payload->GetOriginator());
            mock_call_event = std::make_unique<InteriorMockCallSession>(this, p.first, p.second, std::move(pm_payload));
        }
        std::clog << ActiveStr() << COTEST_THIS_FL << " acknowledging PreMock: " << response.get() << std::endl;
        YieldServer(std::move(response));
    }

    if (next_payload->GetKind() == PayloadKind::MockSeen) {
        COTEST_ASSERT(mock_call_event && "Got MOCK_SEEN without a State::PreMock");
        mock_call_locked = true;
        auto call_payload = SpecialisePayload<MockSeenPayload>(std::move(next_payload));
        mock_call_event->SeenCall(call_payload->GetArgsUntyped());
        // Redundant info in messages just in case we need to match up messages eg
        // in future multi-threaded scenario.
        COTEST_ASSERT(call_payload->GetMocker() == mock_call_event->GetMocker());
        COTEST_ASSERT(call_payload->GetMockObject() == mock_call_event->GetMockObject());
        COTEST_ASSERT(call_payload->GetName() == mock_call_event->GetName());
        std::clog << ActiveStr() << COTEST_THIS_FL << " -> with InteriorMockCallSession "
                  << PtrToString(mock_call_event.get()) << std::endl;
        return mock_call_event;
    } else if (next_payload->GetKind() == PayloadKind::LaunchResult) {
        auto dr_payload = SpecialisePayload<LaunchResultPayload>(std::move(next_payload));
        auto launch_result_event = std::make_unique<InteriorLaunchResultSession>(this, std::move(dr_payload));
        std::clog << ActiveStr() << COTEST_THIS_FL << " -> InteriorLaunchResultSession "
                  << PtrToString(launch_result_event.get()) << std::endl;
        return launch_result_event;
    } else {
        COTEST_ASSERT(!"Unhandled payload type in NextEvent()");
    }
}

bool TestCoroutine::IsPostMockIterationRequested() { return extra_iteration_requested; }

void TestCoroutine::DestructionIterations() {
    COTEST_ASSERT(!IsCoroutineExited());
    // Note:
    // extra_iteration_requested is to allow a test coroutine to "get ahead" after
    // having returned from a mock call, for example so it can exit or set
    // satisfied flag.
    if (!extra_iteration_requested) return;

    extra_iteration_requested = false;

    std::unique_ptr<Payload> from_coro =
        SendMessageFromMain(this, MakePayload<TCDestructingPayload>(shared_from_this()));
    // This can cause coro to exit
    COTEST_ASSERT(from_coro->GetKind() == PayloadKind::TCExited || from_coro->GetKind() == PayloadKind::ResumeMain);
    if (from_coro->GetKind() == PayloadKind::TCExited) {
        auto orig = PeekPayload<TCExitedPayload>(from_coro).GetOriginator().lock();
        COTEST_ASSERT(orig.get() == this);
    }
    if (from_coro->GetKind() == PayloadKind::ResumeMain) {
        auto orig = PeekPayload<ResumeMainPayload>(from_coro).GetOriginator().lock();
        COTEST_ASSERT(orig.get() == this);
    }
}

std::string TestCoroutine::DebugString() const {
    std::stringstream ss;
    ss << "TestCoroutine(\"" << GetName() << "\""
       << ", next_payload=" << !next_payload.get() << ", eir=" << (extra_iteration_requested ? "true" : "false") << ")";
    return ss.str();
}

InteriorLaunchSessionBase::InteriorLaunchSessionBase(TestCoroutine *parent_coroutine_, std::string dc_name_)
    : parent_coroutine(parent_coroutine_), launch_text(dc_name_) {}

InteriorLaunchSessionBase::~InteriorLaunchSessionBase() {
    // Test coroutine has to "see" the result before we know for sure that the
    // launch actually did run to completion. Successful IsLaunchResult() or
    // IsLaunchResult(DC) is enough. We could detect launch completion earlier,
    // but the rule is that the test coro has to do something to ensure this.
    COTEST_ASSERT(launch_completed && "Launch session destructing before result confirmed");
}

void InteriorLaunchSessionBase::SetLaunchCompleted() { launch_completed = true; }

TestCoroutine *InteriorLaunchSessionBase::GetParentTestCoroutine() const { return parent_coroutine; }

std::string InteriorLaunchSessionBase::GetLaunchText() const { return launch_text; }

InteriorEventSession::InteriorEventSession(TestCoroutine *test_coroutine_, bool via_main_,
                                           std::shared_ptr<InteriorLaunchSessionBase> via_launch_)
    : test_coroutine(test_coroutine_), via_main(via_main_), via_launch(via_launch_) {
    // Bad times trying to extract via_main_ and via_launch_ from the payload in
    // this constructor: If we get the payload as a reference, the type of the
    // unique_ptr<> is wrong. If we use rvalue reference, we consume it before the
    // subclass can use it.
}

bool InteriorEventSession::IsFrom(InteriorLaunchSessionBase *source) {
    // source is NULL for IsFromMain()
    if (via_main) return !source;

    auto via_launch_locked = via_launch.lock();
    if (via_launch_locked)
        return source == via_launch_locked.get();
    else
        return false;  // Assuming source has not expired, the launch must be a
                       // different one
}

TestCoroutine *InteriorEventSession::GetTestCoroutine() const { return test_coroutine; }

InteriorMockCallSession::InteriorMockCallSession(TestCoroutine *test_coroutine_, bool via_main_,
                                                 std::shared_ptr<InteriorLaunchSessionBase> via_launch_,
                                                 std::unique_ptr<PreMockPayload> &&payload)
    : InteriorEventSession(test_coroutine_, via_main_, via_launch_),
      originator(payload->GetOriginator()),
      mocker(payload->GetMocker()),
      mock_object(payload->GetMockObject()),
      name(payload->GetName())

{}

InteriorMockCallSession::~InteriorMockCallSession() {
    COTEST_ASSERT(state == State::PreMock || state == State::Dropped || state == State::Returned);
}

std::string InteriorMockCallSession::GetName() const { return name; }

UntypedMockObjectPointer InteriorMockCallSession::GetMockObject() const { return mock_object; }

UntypedMockerPointer InteriorMockCallSession::GetMocker() const { return mocker; }

void InteriorMockCallSession::SeenCall(UntypedArgsPointer args_) {
    COTEST_ASSERT(state == State::PreMock);
    args = args_;
    state = State::Seen;
}

bool InteriorMockCallSession::IsLaunchResult() const { return false; }

bool InteriorMockCallSession::IsLaunchResult(InteriorLaunchSessionBase *launch_session) const { return false; }

bool InteriorMockCallSession::IsMockCall() const { return true; }

void InteriorMockCallSession::Drop() {
    COTEST_ASSERT(state == State::Seen && "in DROP()");
    state = State::Dropped;

    COTEST_ASSERT(!GetTestCoroutine()->IsPendingEvent() && "Internal error: event received during mock lock");
    GetTestCoroutine()->YieldServer(MakePayload<DropMockPayload>(originator, shared_from_this()));
}

void InteriorMockCallSession::Accept() {
    COTEST_ASSERT(state == State::Seen && "in ACCEPT()");
    state = State::Accepted;

    COTEST_ASSERT(!GetTestCoroutine()->IsPendingEvent() && "Internal error: event received during mock lock");
    GetTestCoroutine()->YieldServer(MakePayload<AcceptMockPayload>(originator, shared_from_this()));
}

void InteriorMockCallSession::Return() { ReturnImpl(nullptr); }

UntypedReturnValuePointer InteriorMockCallSession::GetUntypedLaunchResult() const {
    COTEST_ASSERT(!"Cannot get return value from a mock call session");
}

void InteriorMockCallSession::ReturnImpl(UntypedReturnValuePointer return_value) {
    COTEST_ASSERT((state == State::Seen || state == State::Accepted) && "in RETURN()");
    if (state == State::Seen) Accept();
    state = State::Returned;
    COTEST_ASSERT(!GetTestCoroutine()->IsPendingEvent() && "Return(): must use NextEvent() to collect an event first");
    GetTestCoroutine()->YieldServer(MakePayload<ReturnMockPayload>(originator, shared_from_this(), return_value));
}

bool InteriorMockCallSession::IsReturned() const { return state == State::Returned; }

InteriorLaunchResultSession::InteriorLaunchResultSession(TestCoroutine *test_coroutine_,
                                                         std::unique_ptr<LaunchResultPayload> &&payload)
    : InteriorEventSession(test_coroutine_, false, payload->GetOriginator().lock()),
      originator(payload->GetOriginator()),
      return_value(payload->GetResult()) {}

bool InteriorLaunchResultSession::IsLaunchResult() const {
    auto orig = originator.lock();
    if (orig) orig->SetLaunchCompleted();

    return true;
}

bool InteriorLaunchResultSession::IsLaunchResult(InteriorLaunchSessionBase *launch_session) const {
    COTEST_ASSERT(launch_session);
    std::shared_ptr<InteriorLaunchSessionBase> orig = originator.lock();

    if (!orig)
        return false;  // Rationale is: this launch session is still in existance,
                       // so the dead session must have been some other

    if (orig.get() != launch_session) return false;

    orig->SetLaunchCompleted();
    return true;
}

bool InteriorLaunchResultSession::IsMockCall() const { return false; }

void InteriorLaunchResultSession::Drop() {
    // Dropping sessions in error is hard to debug. If the handler comes later in
    // the coro, the coro will fall out at that point (dropped event) and CMock
    // will report unsatisfied. But we want to be shown where the unwanted drop
    // occurred. So fail the test immediately. There's no real case for fall-back
    // handling as with mock calls (where you can have a lower priority
    // EXPECT_CALL() or WATCH_CALL()) as far as I can see.
    COTEST_ASSERT(!"Dropping a launch result is not allowed");
}

void InteriorLaunchResultSession::Accept() { COTEST_ASSERT(!"No need to accept a launch result"); }

void InteriorLaunchResultSession::Return() { COTEST_ASSERT(!"Cannot return from a launch result"); }

UntypedReturnValuePointer InteriorLaunchResultSession::GetUntypedLaunchResult() const { return return_value; }

}  // namespace crf
}  // namespace testing
