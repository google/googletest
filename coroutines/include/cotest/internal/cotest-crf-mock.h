#ifndef COROUTINES_INCLUDE_CORO_INTERNAL_COTEST_CRF_MOCK_H_
#define COROUTINES_INCLUDE_CORO_INTERNAL_COTEST_CRF_MOCK_H_

#include <map>
#include <memory>
#include <set>
#include <string>

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

class MockRoutingSession : public std::enable_shared_from_this<MockRoutingSession> {
   public:
    MockRoutingSession(const MockRoutingSession &i) = delete;
    MockRoutingSession(MockRoutingSession &&i) = delete;
    MockRoutingSession &operator=(const MockRoutingSession &) = delete;
    MockRoutingSession &operator=(MockRoutingSession &&) = delete;
    ~MockRoutingSession() = default;

    MockRoutingSession(UntypedMockerPointer mocker_, UntypedMockObjectPointer mock_object_, std::string name_);

    void PreMockUnlocked();

    void Configure(TestCoroutine *handling_coroutine_);

    bool SeenMockCallLocked(UntypedArgsPointer args);
    UntypedReturnValuePointer ActionsAndReturnUnlocked();

    TestCoroutine *GetHandlingTestCoro() const;
    virtual std::shared_ptr<MockSource> GetMockSource() const = 0;
    std::string GetName() const;

   private:
    virtual std::unique_ptr<Payload> SendMessageToSynchroniser(std::unique_ptr<Payload> &&to_tc) const = 0;
    virtual std::unique_ptr<Payload> SendMessageToHandlingCoro(std::unique_ptr<Payload> &&to_tc) const = 0;

    const UntypedMockerPointer mocker;
    const UntypedMockObjectPointer mock_object;
    const std::string name;

    std::weak_ptr<InteriorMockCallSession> call_session;
    std::set<const TestCoroutine *> handlers_that_dropped;

    TestCoroutine *handling_coroutine;
};

// Note on the extraction of these subclasses: we could get by using virtuals on
// the MockSource, but it's desirable to name the final classes for the constext
// in which their methods will run: ExteriorMockRS runs in main, and
// InteriorMockRS runs in a launch coroutine's interior.
class ExteriorMockRS final : public MockRoutingSession {
   public:
    using MockRoutingSession::MockRoutingSession;

   private:
    std::shared_ptr<MockSource> GetMockSource() const final;
    std::unique_ptr<Payload> SendMessageToSynchroniser(std::unique_ptr<Payload> &&to_tc) const final;
    std::unique_ptr<Payload> SendMessageToHandlingCoro(std::unique_ptr<Payload> &&to_tc) const final;
};

class InteriorMockRS final : public MockRoutingSession {
   public:
    InteriorMockRS(std::shared_ptr<LaunchCoroutine> launch_coro_, UntypedMockerPointer mocker_,
                   UntypedMockObjectPointer mock_object_, std::string name_);

   private:
    std::shared_ptr<MockSource> GetMockSource() const final;
    std::unique_ptr<Payload> SendMessageToSynchroniser(std::unique_ptr<Payload> &&to_tc) const final;
    std::unique_ptr<Payload> SendMessageToHandlingCoro(std::unique_ptr<Payload> &&to_tc) const final;

    const std::weak_ptr<LaunchCoroutine> launch_coro;
};

}  // namespace crf
}  // namespace testing

#endif
