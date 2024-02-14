#ifndef COROUTINES_INCLUDE_CORO_INTERNAL_COTEST_CRF_CORE_H_
#define COROUTINES_INCLUDE_CORO_INTERNAL_COTEST_CRF_CORE_H_

#include <map>
#include <memory>
#include <set>
#include <string>

#include "cotest-coro-common.h"
#include "cotest-coro-thread.h"
#include "cotest-crf-payloads.h"
#include "cotest-util-logging.h"
#include "cotest-util-types.h"

namespace testing {
namespace crf {

// Select the thread-based coroutine impl for all the CRF coroutines.
using CoroImplType = coro_impl::CoroOnThread;

// ------------------ Classes ------------------

class MockRoutingSession;

class InteriorEventSession;

template <typename T>
class InteriorSignatureMockCS;

class InteriorLaunchSessionBase;

template <typename T>
class InteriorLaunchSession;

class TestCoroutine;

using coro_impl::MakePayload;
using coro_impl::PeekPayload;
using coro_impl::SpecialisePayload;

class MessageNode {
   public:
    // Reply payload, reply destination
    using ReplyPair = std::pair<std::unique_ptr<Payload>, MessageNode *>;

    MessageNode() = default;
    MessageNode(const MessageNode &i) = delete;
    MessageNode(MessageNode &&i) = delete;
    MessageNode &operator=(const MessageNode &) = delete;
    MessageNode &operator=(MessageNode &&) = delete;
    virtual ~MessageNode() = default;

    virtual ReplyPair ReceiveMessage(std::unique_ptr<Payload> &&to_node) = 0;
    virtual std::string DebugString() const = 0;

    static std::unique_ptr<Payload> SendMessageFromMain(MessageNode *dest, std::unique_ptr<Payload> &&to_node);

   private:
    static std::unique_ptr<Payload> MessageLoop(MessageNode *dest, std::unique_ptr<Payload> &&to_node);
};

inline std::ostream &operator<<(std::ostream &os, const MessageNode *payload) {
    if (payload) os << payload->DebugString();
    os << coro_impl::PtrToString(payload);
    return os;
}

class CoroutineBase : public virtual MessageNode {
   public:
    CoroutineBase(const CoroutineBase &i) = delete;
    CoroutineBase(CoroutineBase &&i) = delete;
    CoroutineBase &operator=(const CoroutineBase &) = delete;
    CoroutineBase &operator=(CoroutineBase &&) = delete;
    virtual ~CoroutineBase();

    CoroutineBase(coro_impl::BodyFunction cofn_, std::string name_);

    std::unique_ptr<Payload> Iterate(std::unique_ptr<Payload> &&to_coro);
    void Cancel();
    bool IsCoroutineExited() const;

    std::unique_ptr<Payload> Yield(std::unique_ptr<Payload> &&from_coro);

    coro_impl::InteriorInterface *GetImpl();
    void SetName(std::string name_);
    std::string GetName() const;
    std::string ActiveStr() const;

   private:
    std::string name;
    std::unique_ptr<CoroImplType> impl;
    bool initial = true;
};

class MockSource : public virtual MessageNode {
   public:
    MockSource() = default;
    MockSource(const MockSource &i) = delete;
    MockSource(MockSource &&i) = delete;
    MockSource &operator=(const MockSource &) = delete;
    MockSource &operator=(MockSource &&) = delete;
    virtual ~MockSource() = default;

    virtual std::shared_ptr<MockRoutingSession> CreateMockRoutingSession(UntypedMockerPointer mocker_,
                                                                         UntypedMockObjectPointer mock_obj_,
                                                                         const char *name_) = 0;
    void SetCurrentMockRS(std::shared_ptr<MockRoutingSession> current_mock_call_);
    std::shared_ptr<MockRoutingSession> GetCurrentMockRS() const;
    virtual LaunchCoroutine *GetAsCoroutine() = 0;
    virtual const LaunchCoroutine *GetAsCoroutine() const = 0;

   private:
    std::shared_ptr<MockRoutingSession> current_mock_rs;
};

class ProxyForMain : public MockSource, public std::enable_shared_from_this<ProxyForMain> {
   public:
    ReplyPair ReceiveMessage(std::unique_ptr<Payload> &&to_node) final;
    std::shared_ptr<MockRoutingSession> CreateMockRoutingSession(UntypedMockerPointer mocker_,
                                                                 UntypedMockObjectPointer mock_obj_,
                                                                 const char *name_) final;
    std::string DebugString() const final;
    LaunchCoroutine *GetAsCoroutine() final;
    const LaunchCoroutine *GetAsCoroutine() const final;

    static std::shared_ptr<ProxyForMain> GetInstance();
};

}  // namespace crf
}  // namespace testing

#endif
