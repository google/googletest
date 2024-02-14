#include "cotest/internal/cotest-crf-launch.h"

#include <memory>

#include "cotest/internal/cotest-crf-synch.h"
#include "cotest/internal/cotest-crf-test.h"
#include "cotest/internal/cotest-integ-finder.h"
#include "cotest/internal/cotest-util-logging.h"

namespace testing {
namespace crf {

using coro_impl::PtrToString;

LaunchCoroutine::LaunchCoroutine(std::string name_) : CoroutineBase(std::bind(&LaunchCoroutine::Body, this), name_) {
    // Permit the coroutine to yield in order to get its first message
    std::unique_ptr<Payload> from_coro = Iterate(nullptr);
    COTEST_ASSERT(!from_coro);
}

LaunchCoroutine::~LaunchCoroutine() {
    if (!IsCoroutineExited()) Cancel();
}

void LaunchCoroutine::Body() {
    // Get the first launch
    std::unique_ptr<Payload> to_coro = Yield(nullptr);
    while (1) {
        COTEST_ASSERT(to_coro->GetKind() == PayloadKind::Launch);
        auto launch_payload = SpecialisePayload<LaunchPayload>(std::move(to_coro));

        internal::LaunchLambdaWrapperType lambda_wrapper = launch_payload->GetLambdaWrapper();

        // The idea is that something will invoke this "yielder" while the return
        // object is still in scope.
        internal::LaunchYielder yielder = [this, &launch_payload, &to_coro](UntypedReturnValuePointer rvp) {
            auto launch_result_payload =
                MakePayload<LaunchResultPayload>(launch_payload->GetOriginator(), shared_from_this(), rvp);
            to_coro = Yield(std::move(launch_result_payload));
        };

        // Invoking the MUT here
        std::clog << ActiveStr() << COTEST_THIS << " launching lambda_wrapper for \"" << GetName() << "\"" << std::endl;
        lambda_wrapper(yielder);

        std::clog << ActiveStr() << COTEST_THIS << " completed launch" << std::endl;
        COTEST_ASSERT(to_coro);  // Was set via lambda ref capture
    }
}

MessageNode::ReplyPair LaunchCoroutine::ReceiveMessage(std::unique_ptr<Payload> &&to_node) {
    // Collect a pointer to the current launch session, so that we can effectively
    // garbage-collect the launch lambda context which is holding the return value
    // as a temporary. We use a weak pointer, and will expire when the launch
    // session is destructed
    if (to_node && to_node->GetKind() == PayloadKind::Launch)
        current_launch_session = PeekPayload<LaunchPayload>(to_node).GetOriginator();

    return IterateServer(std::move(to_node));
}

MessageNode::ReplyPair LaunchCoroutine::IterateServer(std::unique_ptr<Payload> &&to_coro) {
    COTEST_ASSERT(!IsCoroutineExited());

    // It does not make sense to sent a waiting message into the launch coro, even
    // though the message emerged from a test coro. The infrastructure must deal
    // with it.
    if (to_coro) COTEST_ASSERT(to_coro->GetKind() != PayloadKind::ResumeMain);

    std::unique_ptr<Payload> from_coro = Iterate(std::move(to_coro));

    COTEST_ASSERT(!IsCoroutineExited());
    COTEST_ASSERT(from_coro);

    MessageNode *reply_dest;
    switch (from_coro->GetKind()) {
        case PayloadKind::PreMock: {
            std::shared_ptr<MockRoutingSession> orig = PeekPayload<PreMockPayload>(from_coro).GetOriginator().lock();
            COTEST_ASSERT(orig && "we have a pre mock but session already expired");
            SetCurrentMockRS(orig);

            reply_dest = PreMockSynchroniser::GetInstance();
            break;
        }
        case PayloadKind::MockSeen: {
            std::shared_ptr<MockRoutingSession> orig = PeekPayload<MockSeenPayload>(from_coro).GetOriginator().lock();
            COTEST_ASSERT(orig && "we have a mock call but session already expired");
            COTEST_ASSERT(GetCurrentMockRS().get() == orig.get());
            // To the test coro identified by GMock. MOCK_SEEN responses
            // shall be by return if they are for our originator.
            reply_dest = orig->GetHandlingTestCoro();
            break;
        }
        case PayloadKind::MockAction: {
            std::shared_ptr<MockRoutingSession> orig = PeekPayload<MockActionPayload>(from_coro).GetOriginator().lock();
            COTEST_ASSERT(orig && "we have a mock call but session already expired");
            COTEST_ASSERT(GetCurrentMockRS().get() == orig.get());
            // To the test coro identified by GMock.
            reply_dest = orig->GetHandlingTestCoro();
            break;
        }
        case PayloadKind::LaunchResult: {
            auto orig = PeekPayload<LaunchResultPayload>(from_coro).GetOriginator().lock();
            reply_dest = orig->GetParentTestCoroutine();
            break;
        }
        case PayloadKind::DropMock:
        case PayloadKind::AcceptMock:
        case PayloadKind::ReturnMock:
        case PayloadKind::TCExited:
        case PayloadKind::Launch:
        case PayloadKind::PreMockAck:
        case PayloadKind::ResumeMain:
        case PayloadKind::TCDestructing: {
            COTEST_ASSERT("!unhandled message from launch coroutine");
        }
    }

    COTEST_ASSERT(reply_dest);
    return std::make_pair(std::move(from_coro), reply_dest);
}

LaunchCoroutine *LaunchCoroutine::GetAsCoroutine() { return this; }

const LaunchCoroutine *LaunchCoroutine::GetAsCoroutine() const { return this; }

std::shared_ptr<MockRoutingSession> LaunchCoroutine::CreateMockRoutingSession(UntypedMockerPointer mocker_,
                                                                              UntypedMockObjectPointer mock_obj_,
                                                                              const char *name_) {
    return std::make_shared<InteriorMockRS>(shared_from_this(), mocker_, mock_obj_, name_);
}

std::shared_ptr<InteriorLaunchSessionBase> LaunchCoroutine::TryGetCurrentLaunchSession() {
    return current_launch_session.lock();
}

std::shared_ptr<const InteriorLaunchSessionBase> LaunchCoroutine::TryGetCurrentLaunchSession() const {
    return current_launch_session.lock();
}

std::string LaunchCoroutine::DebugString() const {
    std::stringstream ss;
    ss << "LaunchCoroutine(\"" << GetName() << "\""
       << ", cur_ls=" << PtrToString(current_launch_session.lock().get()) << ")";
    return ss.str();
}

LaunchCoroutine *LaunchCoroutinePool::Allocate(std::string launch_text) {
    // Leaky algorithm has no reclamation or limit but there is COTEST_CLEANUP()
    auto dc = std::make_shared<LaunchCoroutine>(launch_text);
    pool.insert(std::make_pair(dc->GetImpl(), dc));
    return dc.get();
}

std::shared_ptr<MockSource> LaunchCoroutinePool::FindActiveMockSource() {
    if (pool.empty()) return ProxyForMain::GetInstance();  // No launch coros so must be main

    // As long as all the coros in the pool have the same implementation type,
    // we are free to choose any one of them to call GetActive()
    auto active_ii = pool.begin()->first->GetActive();
    if (!active_ii) return ProxyForMain::GetInstance();

    PoolType::iterator it = pool.find(active_ii);
    if (it != pool.end()) return it->second;

    COTEST_ASSERT(!"Cannot invoke mock functions from within test coroutines");
}

LaunchCoroutine *LaunchCoroutinePool::TryGetUnusedLaunchCoro() {
    for (auto p : pool) {
        if (!p.second->TryGetCurrentLaunchSession()) return p.second.get();
    }
    return nullptr;
}

int LaunchCoroutinePool::CleanUp() {
    if (pool.empty()) return 0;
    COTEST_ASSERT(!pool.begin()->first->GetActive() && "Not allowed in coroutine interior");

    int num = 0;
    for (PoolType::const_iterator it = pool.cbegin(); it != pool.cend();) {
        if (!it->second->TryGetCurrentLaunchSession()) {
            it = pool.erase(it);
            num++;
        } else {
            ++it;
        }
    }
    return num;
}

MessageNode::ReplyPair LaunchCoroutinePool::ReceiveMessage(std::unique_ptr<Payload> &&to_node) {
    COTEST_ASSERT(to_node);
    COTEST_ASSERT(to_node->GetKind() == PayloadKind::Launch);

    const std::string launch_text = PeekPayload<LaunchPayload>(to_node).GetName();
    LaunchCoroutine *dc = TryGetUnusedLaunchCoro();
    if (dc)
        dc->SetName(launch_text);
    else
        dc = Allocate(launch_text);

    return make_pair(std::move(to_node), dc);
}

std::string LaunchCoroutinePool::DebugString() const {
    std::stringstream ss;
    ss << "LaunchCoroutinePool( pool_size=" << pool.size() << ")";
    return ss.str();
}

LaunchCoroutinePool *LaunchCoroutinePool::GetInstance() {
    static LaunchCoroutinePool instance;
    return &instance;
}

}  // namespace crf
}  // namespace testing
