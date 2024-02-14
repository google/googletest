#ifndef COROUTINES_INCLUDE_CORO_INTERNAL_COTEST_CRF_TEST_H_
#define COROUTINES_INCLUDE_CORO_INTERNAL_COTEST_CRF_TEST_H_

#include <memory>
#include <string>

#include "cotest-crf-core.h"
#include "cotest-crf-payloads.h"
#include "cotest-util-types.h"
#include "gmock/internal/gmock-internal-utils.h"

namespace testing {
namespace crf {

// ------------------ Classes ------------------

using coro_impl::MakePayload;
using coro_impl::PeekPayload;
using coro_impl::PtrToString;
using coro_impl::SpecialisePayload;

class MockRoutingSession;

class InteriorEventSession;

template <typename T>
class InteriorSignatureMockCS;

class InteriorLaunchSessionBase;

template <typename T>
class InteriorLaunchSession;

class TestCoroutine : public CoroutineBase, public std::enable_shared_from_this<TestCoroutine> {
   public:
    using OnExitFunction = std::function<void()>;

    using CoroutineBase::CoroutineBase;
    TestCoroutine(coro_impl::BodyFunction cofn_, std::string name_, OnExitFunction on_exit_function_);
    ~TestCoroutine();

    ReplyPair ReceiveMessage(std::unique_ptr<Payload> &&to_node) override;
    ReplyPair IterateServer(std::unique_ptr<Payload> &&to_coro);
    void InitialActivity();

    void YieldServer(std::unique_ptr<Payload> &&from_coro);
    bool IsPendingEvent();

    template <typename R>
    std::shared_ptr<InteriorLaunchSession<R>> Launch(internal::LaunchLambdaType<R> &&user_lambda, std::string name);

    std::shared_ptr<InteriorEventSession> NextEvent(const char *file, int line);
    bool IsPostMockIterationRequested();
    void DestructionIterations();
    std::string DebugString() const override;

   private:
    const OnExitFunction on_exit_function;
    std::unique_ptr<Payload> next_payload;
    bool mock_call_locked = false;
    bool extra_iteration_requested = false;
};

class InteriorLaunchSessionBase : public std::enable_shared_from_this<InteriorLaunchSessionBase> {
   public:
    InteriorLaunchSessionBase() = default;
    InteriorLaunchSessionBase(const InteriorLaunchSessionBase &i) = delete;
    InteriorLaunchSessionBase(InteriorLaunchSessionBase &&i) = delete;
    InteriorLaunchSessionBase &operator=(const InteriorLaunchSessionBase &) = delete;
    InteriorLaunchSessionBase &operator=(InteriorLaunchSessionBase &&) = delete;
    ~InteriorLaunchSessionBase();

    InteriorLaunchSessionBase(TestCoroutine *test_coroutine_, std::string dc_name_);

    void SetLaunchCompleted();

    TestCoroutine *GetParentTestCoroutine() const;
    std::string GetLaunchText() const;

   private:
    TestCoroutine *const parent_coroutine;
    bool launch_completed = false;
    const std::string launch_text;
};

template <typename R>
class InteriorLaunchSession : public InteriorLaunchSessionBase {
   public:
    InteriorLaunchSession(TestCoroutine *test_coroutine_, std::string dc_name_);
    ~InteriorLaunchSession();

    void Launch(internal::LaunchLambdaType<R> &&user_lambda);
    R GetResult(const InteriorEventSession *event) const;
};

class InteriorEventSession {
   public:
    InteriorEventSession() = delete;
    InteriorEventSession(const InteriorEventSession &i) = delete;
    InteriorEventSession(InteriorEventSession &&i) = delete;
    InteriorEventSession &operator=(const InteriorEventSession &) = delete;
    InteriorEventSession &operator=(InteriorEventSession &&) = delete;
    virtual ~InteriorEventSession() = default;

    InteriorEventSession(TestCoroutine *test_coroutine_, bool via_main_,
                         std::shared_ptr<InteriorLaunchSessionBase> via_launch_);

    virtual bool IsLaunchResult() const = 0;
    virtual bool IsLaunchResult(InteriorLaunchSessionBase *launch_session) const = 0;
    virtual bool IsMockCall() const = 0;
    virtual void Drop() = 0;
    virtual void Accept() = 0;
    virtual void Return() = 0;
    bool IsFrom(InteriorLaunchSessionBase *source);
    virtual UntypedReturnValuePointer GetUntypedLaunchResult() const = 0;

   protected:
    TestCoroutine *GetTestCoroutine() const;

   private:
    TestCoroutine *const test_coroutine;
    const bool via_main;
    const std::weak_ptr<InteriorLaunchSessionBase> via_launch;
};

class InteriorMockCallSession : public InteriorEventSession,
                                public std::enable_shared_from_this<InteriorMockCallSession> {
   public:
    InteriorMockCallSession(TestCoroutine *test_coroutine_, bool via_main_,
                            std::shared_ptr<InteriorLaunchSessionBase> via_launch_,
                            std::unique_ptr<PreMockPayload> &&payload_);
    ~InteriorMockCallSession();

    bool IsLaunchResult() const override;
    bool IsLaunchResult(InteriorLaunchSessionBase *launch_session) const override;
    bool IsMockCall() const override;

    std::string GetName() const;
    UntypedMockObjectPointer GetMockObject() const;
    UntypedMockerPointer GetMocker() const;
    void SeenCall(UntypedArgsPointer args_);

    template <typename F>
    const typename internal::Function<F>::ArgumentTuple *GetArgumentTuple() const;

    void Drop() override;
    void Accept() override;
    void Return() override;
    UntypedReturnValuePointer GetUntypedLaunchResult() const override;

    void ReturnImpl(UntypedReturnValuePointer return_val_ptr);
    bool IsReturned() const;

   private:
    std::weak_ptr<MockRoutingSession> originator;
    UntypedMockerPointer mocker;
    UntypedMockObjectPointer mock_object;
    std::string name;
    enum class State { PreMock, Seen, Dropped, Accepted, Returned };
    State state = State::PreMock;
    UntypedArgsPointer args;
};

class InteriorLaunchResultSession final : public InteriorEventSession {
   public:
    InteriorLaunchResultSession(TestCoroutine *test_coroutine_, std::unique_ptr<LaunchResultPayload> &&payload_);

    bool IsLaunchResult() const override;
    bool IsLaunchResult(InteriorLaunchSessionBase *launch_session) const override;
    bool IsMockCall() const override;

    void Drop() override;
    void Accept() override;
    void Return() override;
    UntypedReturnValuePointer GetUntypedLaunchResult() const override;

   private:
    const std::weak_ptr<InteriorLaunchSessionBase> originator;

    UntypedReturnValuePointer const return_value = nullptr;
};

template <typename R, typename... Args>
class InteriorSignatureMockCS<R(Args...)> {
    using FN = typename internal::Function<R(Args...)>;
    using ArgumentTuple = typename FN::ArgumentTuple;

   public:
    ~InteriorSignatureMockCS();

    InteriorSignatureMockCS(InteriorMockCallSession *mcs_, const ArgumentTuple *args_tuple_);

    const ArgumentTuple *GetArgumentTuple() const;

    template <typename U>
    void Return(U &&retval);

   private:
    InteriorMockCallSession *const mcs;
    const ArgumentTuple *const args_tuple;
};

// ------------------ Templated members ------------------

template <typename R>
std::shared_ptr<InteriorLaunchSession<R>> TestCoroutine::Launch(internal::LaunchLambdaType<R> &&user_lambda,
                                                                std::string df_name) {
    COTEST_ASSERT(!next_payload && "Launch(): must use NextEvent() to collect an event first");
    const auto ils = std::make_shared<InteriorLaunchSession<R>>(this, df_name);
    ils->Launch(std::move(user_lambda));
    return ils;
}

template <typename R>
InteriorLaunchSession<R>::InteriorLaunchSession(TestCoroutine *test_coroutine_, std::string dc_name_)
    : InteriorLaunchSessionBase(test_coroutine_, dc_name_) {}

template <typename R>
InteriorLaunchSession<R>::~InteriorLaunchSession() {}

template <typename R>
void InteriorLaunchSession<R>::Launch(internal::LaunchLambdaType<R> &&user_lambda) {
    internal::LaunchLambdaWrapperType wrapper_lambda =
        internal::CotestTypeUtils<R>::WrapLaunchLambda(std::move(user_lambda));
    // Note that user_lambda captures all by reference and these captures will not
    // be safe once launch coroutine yields eg to generate a mock call, so we need
    // to iterate the launch coroutine immediately. It is assumed that the lambda
    // is just a function call and this call should normally take arguments by
    // value in order to be safe. Args passed by reference need to be checked by
    // the user.
    auto call_payload = MakePayload<LaunchPayload>(shared_from_this(), wrapper_lambda, GetLaunchText());
    GetParentTestCoroutine()->YieldServer(std::move(call_payload));
}

template <typename R>
R InteriorLaunchSession<R>::GetResult(const InteriorEventSession *event) const {
    return internal::CotestTypeUtils<R>::Specialise(event->GetUntypedLaunchResult());
}

template <typename F>
const typename internal::Function<F>::ArgumentTuple *InteriorMockCallSession::GetArgumentTuple() const {
    COTEST_ASSERT(state != State::Returned);
    COTEST_ASSERT(IsMockCall());
    using FN = typename internal::Function<F>;
    return static_cast<const typename FN::ArgumentTuple *>(args);
}

template <typename R, typename... Args>
InteriorSignatureMockCS<R(Args...)>::InteriorSignatureMockCS(InteriorMockCallSession *const mcs_,
                                                             const ArgumentTuple *args_tuple_)
    : mcs(mcs_), args_tuple(args_tuple_) {}

template <typename R, typename... Args>
InteriorSignatureMockCS<R(Args...)>::~InteriorSignatureMockCS() {}

template <typename R, typename... Args>
const typename InteriorSignatureMockCS<R(Args...)>::ArgumentTuple *
InteriorSignatureMockCS<R(Args...)>::GetArgumentTuple() const {
    COTEST_ASSERT(!mcs->IsReturned());  // Cannot rely on args after return - take a copy!
    return args_tuple;
}

template <typename R, typename... Args>
template <typename U>
void InteriorSignatureMockCS<R(Args...)>::Return(U &&retval) {
    const UntypedReturnValuePointer p = internal::CotestTypeUtils<R>::Generalise(std::forward<U>(retval));
    mcs->ReturnImpl(p);
}

}  // namespace crf
}  // namespace testing

#endif
