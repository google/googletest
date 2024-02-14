#include "cotest/internal/cotest-crf-core.h"

#include <memory>

#include "cotest/internal/cotest-crf-mock.h"
#include "cotest/internal/cotest-crf-synch.h"
#include "cotest/internal/cotest-integ-finder.h"
#include "cotest/internal/cotest-util-logging.h"

namespace testing {
namespace crf {

using coro_impl::PtrToString;

std::unique_ptr<Payload> MessageNode::SendMessageFromMain(MessageNode *dest, std::unique_ptr<Payload> &&to_node) {
    // Fill in main as sender and then check return is for main
    return MessageLoop(dest, std::move(to_node));
}

std::unique_ptr<Payload> MessageNode::MessageLoop(MessageNode *dest, std::unique_ptr<Payload> &&to_node) {
    // std::clog << __func__ << "() starts: dispatching " << to_node.get() << " to
    // " << dest->DebugString() << std::endl;

    while (1) {
        // Dispatch a message
        COTEST_ASSERT(dest);

        std::unique_ptr<Payload> reply;
        MessageNode *reply_dest;
        std::tie(reply, reply_dest) = dest->ReceiveMessage(std::move(to_node));

        // std::clog << __func__ << "() iterates: dispatching " << reply.get() << "
        // to " << (reply_dest ? reply_dest->DebugString() : "NULL") << std::endl;
        COTEST_ASSERT(reply_dest && "MessageNode did not provide reply destination");

        // Messages for main are just returned to caller, which should be main
        if (reply_dest == ProxyForMain::GetInstance().get()) return reply;

        // For the next iteration...
        dest = reply_dest;
        to_node = std::move(reply);
    }

    // std::clog << __func__ << "() returns to main" << std::endl;
}

CoroutineBase::~CoroutineBase() { COTEST_ASSERT(IsCoroutineExited()); }

CoroutineBase::CoroutineBase(coro_impl::BodyFunction cofn, std::string name_)
    : name(std::move(name_)), impl(std::make_unique<CoroImplType>(cofn, name)) {}

std::unique_ptr<Payload> CoroutineBase::Iterate(std::unique_ptr<Payload> &&to_coro) {
    // std::clog << ActiveStr() << COTEST_THIS << " iterates with " <<
    // to_coro.get() << std::endl;
    initial = false;
    std::unique_ptr<coro_impl::Payload> from_coro_base = impl->Iterate(std::move(to_coro));
    // if( IsCoroutineExited() )
    //	std::clog << ActiveStr() << COTEST_THIS << " exited" << std::endl;
    if (from_coro_base)
        return SpecialisePayload<Payload>(std::move(from_coro_base));
    else
        return nullptr;
}

void CoroutineBase::Cancel() {
    // std::clog << ActiveStr() << COTEST_THIS << " cancelling" << std::endl;
    COTEST_ASSERT(!IsCoroutineExited());
    impl->Cancel();
}

bool CoroutineBase::IsCoroutineExited() const { return impl->IsCoroutineExited(); }

std::unique_ptr<Payload> CoroutineBase::Yield(std::unique_ptr<Payload> &&from_coro) {
    // std::clog << ActiveStr() << COTEST_THIS << " yields "  << from_coro.get()
    // << std::endl;
    std::unique_ptr<coro_impl::Payload> to_coro_base = impl->Yield(std::move(from_coro));
    // We have to assume this is a CRF payload. In practice, they all are.
    if (to_coro_base)
        return SpecialisePayload<Payload>(std::move(to_coro_base));
    else
        return nullptr;
}

void CoroutineBase::SetName(std::string name_) {
    name = name_;
    impl->SetName(name);
}

std::string CoroutineBase::GetName() const { return name; }

std::string CoroutineBase::ActiveStr() const {
    auto *active = static_cast<CoroImplType *>(impl->GetActive());
    if (!active)
        return ">< ";  // running in main context
    else if (active == impl.get())
        return "[] ";
    else
        return "[" + active->GetName() + "!!] ";  // Wrong coro is active - probably an internal error
}

coro_impl::InteriorInterface *CoroutineBase::GetImpl() {
    return static_cast<coro_impl::InteriorInterface *>(impl.get());
}

void MockSource::SetCurrentMockRS(std::shared_ptr<MockRoutingSession> current_mock_call_) {
    COTEST_ASSERT(current_mock_call_);
    current_mock_rs = current_mock_call_;
}

std::shared_ptr<MockRoutingSession> MockSource::GetCurrentMockRS() const {
    COTEST_ASSERT(current_mock_rs);
    return current_mock_rs;
}

std::shared_ptr<MockRoutingSession> ProxyForMain::CreateMockRoutingSession(UntypedMockerPointer mocker_,
                                                                           UntypedMockObjectPointer mock_obj_,
                                                                           const char *name_) {
    auto mrs_sp = std::make_shared<ExteriorMockRS>(mocker_, mock_obj_, name_);
    SetCurrentMockRS(mrs_sp);
    return mrs_sp;
}

LaunchCoroutine *ProxyForMain::GetAsCoroutine() {
    COTEST_ASSERT(!"Internal error: attempting to get a coroutine for main (as a mock source)");
}

const LaunchCoroutine *ProxyForMain::GetAsCoroutine() const {
    COTEST_ASSERT(!"Internal error: attempting to get a coroutine for main (as a mock source)");
}

std::shared_ptr<ProxyForMain> ProxyForMain::GetInstance() {
    static auto instance = std::make_shared<ProxyForMain>();
    return instance;
}

MessageNode::ReplyPair ProxyForMain::ReceiveMessage(std::unique_ptr<Payload> &&to_node) {
    COTEST_ASSERT(!"Messages should be dispatched to main by returning from the message loop (internal error)");
}

std::string ProxyForMain::DebugString() const {
    std::stringstream ss;
    ss << "ProxyForMain()";
    return ss.str();
}

}  // namespace crf
}  // namespace testing
