#ifndef COROUTINES_INCLUDE_CORO_COTEST_H_
#define COROUTINES_INCLUDE_CORO_COTEST_H_

#include "cotest/internal/cotest-integ-mock.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace testing {

// We use preprocessor macros when we need stringification or
// other trickery, and also when we expect to want to collect
// __FILE__ and __LINE__ (but not all have been added yet).

// ------------------ utils ------------------

// Credit for this goes to the GMock implementors
#define COTEST_PP_VARIADIC_CALL(_Macro, ...) \
    GMOCK_PP_IDENTITY(GMOCK_PP_CAT(_Macro, GMOCK_PP_NARG0(__VA_ARGS__))(__VA_ARGS__))

#define COTEST_STR(S) COTEST_STRW(S)
#define COTEST_STRW(S) #S

// ------------------ mock call session ------------------

#define DROP() Drop()

#define ACCEPT() Accept()

// Covers 0-arg and 1-arg cases including unprotected commas.
#define RETURN(...) Return(__VA_ARGS__)

// See also MOCK_CALL_HANDLE, IS_CALL and WAIT_FOR_CALL below

// ------------------ launch session -------------------------

// Launch the supplied expression and return launch session
#define LAUNCH(...)                                                                                \
    cotest_coro_->Launch<decltype(__VA_ARGS__)>([&]() -> decltype(auto) { return (__VA_ARGS__); }, \
                                                COTEST_STR(__VA_ARGS__))

// See also IS_RESULT and WAIT_FOR_RESULT below.
// Result obtained using call syntax:
// result = event_session( launch_session );

// Note on cotest_coro_
// cotest_coro_ is passed into coroutine body as a function parameter
// and is also a public member of a Coroutine. In both cases it just
// points to the coroutine. So, in the body we can use NEXT_EVENT()
// and outside, when we have a Coroutine, we can do coro->NEXT_EVENT().
// cotest_coro_ is also helpful for reducing pollution of the global
// namespace.

// ------------------ cardinality etc --------------------

// Indicate that the coroutine does not need to run to completion
// in order for the test to pass
#define SATISFY() cotest_coro_->SetSatisfied()

// Indicate that it is not an error for the coroutine to see further
// mock calls after exiting (they will be dropped)
#define RETIRE() cotest_coro_->Retire()

// Return from mthe coroutine body. Please use this in preference to return
// for forward-compatibility with C++20 coroutines (and so we can grab
// __FILE__ and __LINE__).
#define EXIT_COROUTINE() return

// --------------------- utilities -------------------------

// De-allocate unused launch coroutines. This plugs a memory leak but
// could slow things down if used frequently.
#define COTEST_CLEANUP() (crf::LaunchCoroutinePool::GetInstance()->CleanUp())

// ------------------ Declaring coroutines ------------------

// Use one of:
// auto my_coro = COROUTINE(){ ...code... };
// or
// auto my_coro = COROUTINE(MyCoroName){ ...code... };
#define COROUTINE(...) COTEST_PP_VARIADIC_CALL(COROUTINE_ARG_, __VA_ARGS__)

#define COROUTINE_ARG_0() ::testing::internal::LambdaCoroFactory() + [&](::testing::internal::Coroutine * cotest_coro_)

#define COROUTINE_ARG_1(NAME) \
    ::testing::internal::LambdaCoroFactory(COTEST_STR(NAME)) + [&](::testing::internal::Coroutine * cotest_coro_)

// Allocate a coroutine on the heap
#define NEW_COROUTINE(...) COTEST_PP_VARIADIC_CALL(NEW_COROUTINE_ARG_, __VA_ARGS__)

#define NEW_COROUTINE_ARG_0() \
    ::testing::internal::LambdaCoroFactory() - [&](::testing::internal::Coroutine * cotest_coro_)

#define NEW_COROUTINE_ARG_1(NAME) \
    ::testing::internal::LambdaCoroFactory(COTEST_STR(NAME)) - [&](::testing::internal::Coroutine * cotest_coro_)

// ------------------ watching for mock calls ------------------

#define WATCH_CALL(...) COTEST_PP_VARIADIC_CALL(WATCH_CALL_ARG_, __VA_ARGS__)

// WATCH_CALL(obj, call) is similar to EXPECT_CALL including implied priority
// scheme and With().
#define WATCH_CALL_ARG_2(obj, call) \
    cotest_coro_->WatchCall(std::move(GMOCK_GET_MOCKSPEC(obj, call, gmock)), __FILE__, __LINE__, #obj, #call)

// Wildcard version: any mocked call on supplied mock object
#define WATCH_CALL_ARG_1(obj) \
    cotest_coro_->WatchCall(__FILE__, __LINE__, static_cast<::testing::crf::UntypedMockObjectPointer>(&(obj)))

// Wildcard version: any mocked call
#define WATCH_CALL_ARG_0() cotest_coro_->WatchCall(__FILE__, __LINE__)

// ------------------ variadic MOCK_CALL_HANDLE() ------------------

#define MOCK_CALL_HANDLE(...) COTEST_PP_VARIADIC_CALL(MOCK_CALL_HANDLE_ARG_, __VA_ARGS__)

// NULL session for any mock call
#define MOCK_CALL_HANDLE_ARG_0() (::testing::EventHandle())

// No different than MOCK_CALL_HANDLE() - not for human use, only consistency.
#define MOCK_CALL_HANDLE_ARG_1(OBJ) (::testing::EventHandle())

// NULL call session for any mock call with signature matching the supplied mock
// method
#define MOCK_CALL_HANDLE_ARG_2(OBJ, METHOD) \
    (::testing::internal::CreateSignatureHandle(std::move(GMOCK_GET_MOCKSPEC(OBJ, METHOD, gmockq))))

// Can also use SignatureHandle<SIGNATURE> where SIGNATURE is eg int(void *)

// ------------------ variadic IS_CALL() ------------------

#define IS_CALL(...) COTEST_PP_VARIADIC_CALL(IS_CALL_ARG_, __VA_ARGS__)

// Is the event a mock call?
#define IS_CALL_ARG_0() IsMockCall()

// Is the event a mock call on the supplied mock object?
#define IS_CALL_ARG_1(obj) IsObject((::testing::crf::UntypedMockObjectPointer) & (obj))

// Is the mock call a match to the method. Matchers and With() are
// supported as with EXPECT_CALL().
#define IS_CALL_ARG_2(obj, call) CoTestIsCallImpl_(std::move(GMOCK_GET_MOCKSPEC(obj, call, gmockq)))

// ------------------ variadic IS_RESULT ------------------

#define IS_RESULT(...) COTEST_PP_VARIADIC_CALL(IS_RESULT_ARG_, __VA_ARGS__)

// Is this event a completed launch result?
#define IS_RESULT_ARG_0() IsLaunchResult()

// Is this event a completed launch result from the given launch
// session?
#define IS_RESULT_ARG_1(DC) IsLaunchResult(DC)

// Note: these operations also tell cotest that the launch return has
// been detected by the coroutine. If this does not happen, cotest will
// report an error.

// ---------------- wait for call ------------------

#define WAIT_FOR_CALL_NSE(...) COTEST_PP_VARIADIC_CALL(WAIT_FOR_CALL_NSE_ARG_, __VA_ARGS__)

#define WAIT_FOR_CALL(...) COTEST_PP_VARIADIC_CALL(WAIT_FOR_CALL_ARG_, __VA_ARGS__)

// Note: _NSE versions do not use the gcc statement expression extension.
// This extnesion permits function-like macros safely that can yield
// inside C++20 coroutines.

// ------------------ By method ------------------

// "Wait" for a mock call that satisfies a matcher similar to
// EXPECT_CALL(). With() not supported. This will drop non-matching
// calls and then when one matches it will accept it and return the
// session.

#define WAIT_FOR_CALL_NSE_ARG_3(CS, OBJ, METHOD)   \
    auto CS = MOCK_CALL_HANDLE_ARG_2(OBJ, METHOD); \
    do {                                           \
        auto cg_ = NEXT_EVENT();                   \
        CS = cg_.IS_CALL_ARG_2(OBJ, METHOD);       \
        if (!CS) cg_.DROP();                       \
    } while (!CS);                                 \
    CS.ACCEPT()

#define WAIT_FOR_CALL_ARG_2(OBJ, METHOD)          \
    ({                                            \
        WAIT_FOR_CALL_NSE_ARG_3(cs, OBJ, METHOD); \
        cs;                                       \
    })

// ------------------ By object ------------------

// "Wait" for a mock call on the given object. This will drop non-matching
// calls and then when one matches it will accept it and return the
// session.

#define WAIT_FOR_CALL_NSE_ARG_2(CG, OBJ) \
    auto CG = MOCK_CALL_HANDLE_ARG_0();  \
    do {                                 \
        auto cg_ = NEXT_EVENT();         \
        CG = cg_.IS_CALL_ARG_1(OBJ);     \
        if (!CG) cg_.DROP();             \
    } while (!CG);                       \
    CG.ACCEPT()

#define WAIT_FOR_CALL_ARG_1(OBJ)          \
    ({                                    \
        WAIT_FOR_CALL_NSE_ARG_2(cg, OBJ); \
        cg;                               \
    })

// ------------------- Any call -----------------------

// "Wait" for any mock call. This will accept it and return the
// session.

#define WAIT_FOR_CALL_NSE_ARG_1(CG)     \
    auto CG = MOCK_CALL_HANDLE_ARG_0(); \
    do {                                \
        auto cg_ = NEXT_EVENT();        \
        CG = cg_.IS_CALL_ARG_0();       \
        if (!CG) cg_.DROP();            \
    } while (!CG);                      \
    CG.ACCEPT()

#define WAIT_FOR_CALL_ARG_0()        \
    ({                               \
        WAIT_FOR_CALL_NSE_ARG_1(cg); \
        cg;                          \
    })

// -------------- wait for call from ----------------

#define WAIT_FOR_CALL_FROM_NSE(...) COTEST_PP_VARIADIC_CALL(WAIT_FOR_CALL_FROM_NSE_ARG_, __VA_ARGS__)

#define WAIT_FOR_CALL_FROM(...) COTEST_PP_VARIADIC_CALL(WAIT_FOR_CALL_FROM_ARG_, __VA_ARGS__)

// ------------------ By method ------------------

// As above, but from given lauch session only
#define WAIT_FOR_CALL_FROM_NSE_ARG_4(CS, OBJ, METHOD, DS) \
    auto CS = MOCK_CALL_HANDLE_ARG_2(OBJ, METHOD);        \
    do {                                                  \
        auto cg_ = NEXT_EVENT();                          \
        CS = cg_.IS_CALL_ARG_2(OBJ, METHOD).From(DS);     \
        if (!CS) cg_.DROP();                              \
    } while (!CS);                                        \
    CS.ACCEPT()

#define WAIT_FOR_CALL_FROM_ARG_3(OBJ, METHOD, DS)          \
    ({                                                     \
        WAIT_FOR_CALL_FROM_NSE_ARG_4(cs, OBJ, METHOD, DS); \
        cs;                                                \
    })

// ------------------ By object ------------------

// As above, but from given lauch session only
#define WAIT_FOR_CALL_FROM_NSE_ARG_3(CG, OBJ, DS) \
    auto CG = MOCK_CALL_HANDLE_ARG_0();           \
    do {                                          \
        auto cg_ = NEXT_EVENT();                  \
        CG = cg_.IS_CALL_ARG_1(OBJ).From(DS);     \
        if (!CG) cg_.DROP();                      \
    } while (!CG);                                \
    CG.ACCEPT()

#define WAIT_FOR_CALL_FROM_ARG_2(OBJ, DS)          \
    ({                                             \
        WAIT_FOR_CALL_FROM_NSE_ARG_3(cg, OBJ, DS); \
        cg;                                        \
    })

// ------------------- Any call -----------------------

// As above, but from given lauch session only

#define WAIT_FOR_CALL_FROM_NSE_ARG_2(CG, DS) \
    auto CG = MOCK_CALL_HANDLE_ARG_0();      \
    do {                                     \
        auto cg_ = NEXT_EVENT();             \
        CG = cg_.IS_CALL_ARG_0().From(DS);   \
        if (!CG) cg_.DROP();                 \
    } while (!CG);                           \
    CG.ACCEPT()

#define WAIT_FOR_CALL_FROM_ARG_1(DS)          \
    ({                                        \
        WAIT_FOR_CALL_FROM_NSE_ARG_2(cg, DS); \
        cg;                                   \
    })

// ---------------- wait for result ------------------

// Wait for a completed launch, dropping mock calls
#define WAIT_FOR_RESULT_NSE(CG)         \
    auto CG = ::testing::EventHandle(); \
    do {                                \
        auto cg_ = NEXT_EVENT();        \
        CG = cg_.IS_RESULT_ARG_0();     \
        if (!CG) cg_.DROP();            \
    } while (!CG);

#define WAIT_FOR_RESULT()        \
    ({                           \
        WAIT_FOR_RESULT_NSE(cg); \
        cg;                      \
    })

// ---------------------- COTEST ---------------------------

#define COTEST_TEST_CLASS_NAME_(TEST_SUITE_NAME, TEST_NAME) TEST_SUITE_NAME##_##TEST_NAME##_Cotest

// Declare a "pure" cotest test. Usage is similar to TEST(). The given body
// is the body of a coroutine. Testing assets should be declared within
// the coro body.
#define COTEST(TEST_SUITE_NAME, TEST_NAME)                                                                          \
    static void COTEST_TEST_CLASS_NAME_(TEST_SUITE_NAME, TEST_NAME)(::testing::internal::Coroutine * cotest_coro_); \
    TEST(TEST_SUITE_NAME, TEST_NAME) {                                                                              \
        auto c = ::testing::internal::Coroutine(COTEST_TEST_CLASS_NAME_(TEST_SUITE_NAME, TEST_NAME),                \
                                                COTEST_STR(TEST_NAME));                                             \
    }                                                                                                               \
    static void COTEST_TEST_CLASS_NAME_(TEST_SUITE_NAME, TEST_NAME)(::testing::internal::Coroutine * cotest_coro_)

// Note: this is not in fact the most flexible way to use cotest. A regular
// TEST() case can declare multiple coroutines, whereas a COTEST() only has one.

// ------------------ serverised API ------------------

// Returns the next valid event session. An event can be a mock call, or a launch return.
#define NEXT_EVENT() cotest_coro_->NextEvent(__FILE__, __LINE__)

// This is a lower-level alternative to the WAIT_FOR_ macros, and if the
// returned event is a mock call, the user is required to call DROP() or
// ACCEPT() on it before doing anything else with the cotest API, or exiting.
// NEXT_EVENT will return a mock call session for every event the coro
// can see: if WATCH_CALL() is used, this will be all mock calls not
// handled by a higher-priority watch or expectations.
//
// It is intended for use in a message loop, for when mock calls and launch
// completions must be handled in whatever order they arrive. This is
// termed serverised style.

// ------------------ Classes ------------------

// Handle for the session created by a LAUNCH(). Templated on the session
// return type, which is simply the decltype() of the supplied expression.
template <class RESULT_TYPE>
class LaunchHandle {
   public:
    LaunchHandle() = default;
    explicit LaunchHandle(std::shared_ptr<crf::InteriorLaunchSession<RESULT_TYPE>> crf_ls_);
    operator bool() const;

    crf::InteriorLaunchSession<RESULT_TYPE> *GetCRF_();

   private:
    std::shared_ptr<crf::InteriorLaunchSession<RESULT_TYPE>> crf_ls;
};

// Handle for any event received by NEXT_EVENT() which can be a call
// session or a launch result session.
class EventHandle {
   public:
    EventHandle() = default;
    explicit EventHandle(std::shared_ptr<crf::InteriorEventSession> crf_es_);

    EventHandle IsLaunchResult() const;
    template <typename RESULT_TYPE>
    EventHandle IsLaunchResult(LaunchHandle<RESULT_TYPE> launch_session) const;
    template <typename RESULT_TYPE>
    RESULT_TYPE operator()(LaunchHandle<RESULT_TYPE> launch_session) const;

    template <typename R, typename... Args>
    SignatureHandle<R(Args...)> CoTestIsCallImpl_(MockSpec<R(Args...)> &&mock_spec);
    EventHandle IsObject(crf::UntypedMockObjectPointer object);
    EventHandle IsMockCall();
    operator bool() const;

    EventHandle Drop();
    EventHandle Accept();
    EventHandle Return();
    template <typename RESULT_TYPE>
    EventHandle From(LaunchHandle<RESULT_TYPE> &source);
    EventHandle FromMain();

    std::string GetName() const;

   private:
    std::shared_ptr<crf::InteriorEventSession> crf_es;
};

// Handle for a mock call session when the function type is known. Templated
// on the function type eg int(char *)
template <typename R, typename... Args>
class SignatureHandle<R(Args...)> : public EventHandle {
   public:
    using ArgumentTuple = typename internal::Function<R(Args...)>::ArgumentTuple;

    SignatureHandle() = default;
    SignatureHandle(std::shared_ptr<crf::InteriorEventSession> crf_es_,
                    std::shared_ptr<crf::InteriorSignatureMockCS<R(Args...)>> &&crf_sig_);

    SignatureHandle IsMockCall();
    operator bool() const;

    SignatureHandle Drop();
    SignatureHandle Accept();
    SignatureHandle Return();
    template <typename RESULT_TYPE>
    SignatureHandle From(LaunchHandle<RESULT_TYPE> &source);
    SignatureHandle FromMain();

    template <typename U>
    SignatureHandle Return(U &&retval);
    const ArgumentTuple &GetArgs() const;
    template <size_t I>
    const typename internal::Function<R(Args...)>::Arg<I>::type &GetArg() const;
    SignatureHandle With(const Matcher<const ArgumentTuple &> &m);
    template <size_t I>
    SignatureHandle WithArg(const Matcher<const typename internal::Function<R(Args...)>::Arg<I>::type &> &m);

   private:
    std::shared_ptr<crf::InteriorSignatureMockCS<R(Args...)>> crf_sig;
};

// ------------------ Templated members ------------------

template <typename RESULT_TYPE>
LaunchHandle<RESULT_TYPE>::LaunchHandle(std::shared_ptr<crf::InteriorLaunchSession<RESULT_TYPE>> crf_ls_)
    : crf_ls(crf_ls_) {}

template <typename RESULT_TYPE>
LaunchHandle<RESULT_TYPE>::operator bool() const {
    return !!crf_ls;
}

template <typename RESULT_TYPE>
crf::InteriorLaunchSession<RESULT_TYPE> *LaunchHandle<RESULT_TYPE>::GetCRF_() {
    return crf_ls.get();
}

template <typename RESULT_TYPE>
EventHandle EventHandle::IsLaunchResult(LaunchHandle<RESULT_TYPE> launch_session) const {
    if (crf_es->IsLaunchResult(launch_session.GetCRF_()))
        return *this;
    else
        return EventHandle();
}

template <typename RESULT_TYPE>
RESULT_TYPE EventHandle::operator()(LaunchHandle<RESULT_TYPE> launch_session) const {
    if (crf_es->IsLaunchResult(launch_session.GetCRF_()))
        return launch_session.GetCRF_()->GetResult(crf_es.get());
    else
        COTEST_ASSERT(!"Test failure: event is not a launch result");
}

template <typename R, typename... Args>
SignatureHandle<R(Args...)> EventHandle::CoTestIsCallImpl_(MockSpec<R(Args...)> &&mock_spec) {
    const FunctionMocker<R(Args...)> *mocker = mock_spec.InternalGetMocker();
    crf::UntypedMockObjectPointer mock_object = mocker->MockObjectLocked();
    auto utmb = static_cast<const internal::UntypedFunctionMockerBase *>(mocker);
    auto untyped_mocker = static_cast<crf::UntypedMockerPointer>(utmb);

    COTEST_ASSERT(mock_object && "NULL Mock object used with IS_CALL()");

    COTEST_ASSERT(crf_es && "event session is NULL, check for failed test");
    if (!crf_es->IsMockCall()) return SignatureHandle<R(Args...)>();
    auto crf_mcs = std::static_pointer_cast<crf::InteriorMockCallSession>(crf_es);

    // Check the mock object
    if (mock_object != crf_mcs->GetMockObject()) return SignatureHandle<R(Args...)>();

    // Check whether the same method is used (accurate even when the name
    // and signature are the same due eg const overloading)
    if (untyped_mocker != crf_mcs->GetMocker()) return SignatureHandle<R(Args...)>();

    // This is the correct method, so try to match the values
    // We have a mock spec (on a NULL mock object) and can get the matchers tuple
    // from it
    auto matchers = mock_spec.InternalGetMatchers();

    // Get the arguments for matching and signature call session
    auto args_tuple = crf_mcs->GetArgumentTuple<R(Args...)>();

    // Let Google Test perform the matching
    if (!internal::TupleMatches(matchers, *args_tuple)) return SignatureHandle<R(Args...)>();

    auto crf_sig = std::make_shared<crf::InteriorSignatureMockCS<R(Args...)>>(crf_mcs.get(), args_tuple);
    return SignatureHandle<R(Args...)>(crf_es, std::move(crf_sig));
}

template <typename RESULT_TYPE>
EventHandle EventHandle::From(LaunchHandle<RESULT_TYPE> &source) {
    COTEST_ASSERT(crf_es);
    bool ok = crf_es->IsFrom(source.GetCRF_());
    return ok ? *this : EventHandle();
}

template <typename R, typename... Args>
SignatureHandle<R(Args...)>::SignatureHandle(std::shared_ptr<crf::InteriorEventSession> crf_es_,
                                             std::shared_ptr<crf::InteriorSignatureMockCS<R(Args...)>> &&crf_sig_)
    : EventHandle(crf_es_), crf_sig(std::move(crf_sig_)) {}

template <typename R, typename... Args>
SignatureHandle<R(Args...)> SignatureHandle<R(Args...)>::IsMockCall() {
    return *this;
}

template <typename R, typename... Args>
SignatureHandle<R(Args...)>::operator bool() const {
    return !!crf_sig;
}

template <typename R, typename... Args>
SignatureHandle<R(Args...)> SignatureHandle<R(Args...)>::Drop() {
    EventHandle::Drop();
    return *this;
}

template <typename R, typename... Args>
SignatureHandle<R(Args...)> SignatureHandle<R(Args...)>::Accept() {
    EventHandle::Accept();
    return *this;
}

template <typename R, typename... Args>
SignatureHandle<R(Args...)> SignatureHandle<R(Args...)>::Return() {
    EventHandle::Return();
    return *this;
}

template <typename R, typename... Args>
SignatureHandle<R(Args...)> SignatureHandle<R(Args...)>::FromMain() {
    bool ok = EventHandle::FromMain();
    return ok ? *this : SignatureHandle<R(Args...)>();
}

template <typename R, typename... Args>
template <typename RESULT_TYPE>
SignatureHandle<R(Args...)> SignatureHandle<R(Args...)>::From(LaunchHandle<RESULT_TYPE> &source) {
    bool ok = EventHandle::From(source);
    return ok ? *this : SignatureHandle<R(Args...)>();
}

template <typename R, typename... Args>
template <typename U>
SignatureHandle<R(Args...)> SignatureHandle<R(Args...)>::Return(U &&retval) {
    COTEST_ASSERT(crf_sig && "call session is NULL, check for failed test");
    crf_sig->Return(std::forward<U>(retval));
    return *this;
}

template <typename R, typename... Args>
const typename SignatureHandle<R(Args...)>::ArgumentTuple &SignatureHandle<R(Args...)>::GetArgs() const {
    COTEST_ASSERT(crf_sig && "call session is NULL, check for failed test");
    return *crf_sig->GetArgumentTuple();
}

template <typename R, typename... Args>
template <size_t I>
const typename internal::Function<R(Args...)>::Arg<I>::type &SignatureHandle<R(Args...)>::GetArg() const {
    return std::get<I>(GetArgs());
}

template <typename R, typename... Args>
SignatureHandle<R(Args...)> SignatureHandle<R(Args...)>::With(const Matcher<const ArgumentTuple &> &m) {
    if (!crf_sig) return SignatureHandle<R(Args...)>();  // may have already mismatched

    // Let Google Test perform the matching
    if (!m.Matches(GetArgs())) return SignatureHandle<R(Args...)>();

    return *this;
}

template <typename R, typename... Args>
template <size_t I>
SignatureHandle<R(Args...)> SignatureHandle<R(Args...)>::WithArg(
    const Matcher<const typename internal::Function<R(Args...)>::Arg<I>::type &> &m) {
    if (!crf_sig) return SignatureHandle<R(Args...)>();  // may have already mismatched

    // Let Google Test perform the matching
    if (!m.Matches(GetArg<I>())) return SignatureHandle<R(Args...)>();

    return *this;
}

namespace internal {

// I apologise for the operator overloads in this class.
class LambdaCoroFactory {
   public:
    LambdaCoroFactory(std::string name_ = "COROUTINE()") : name(name_) {}

    // Sorry about these
    Coroutine operator+(Coroutine::BodyFunctionType lambda) { return Coroutine(lambda, name); }

    Coroutine *operator-(Coroutine::BodyFunctionType lambda) { return new Coroutine(lambda, name); }

   private:
    const std::string name;
};

}  // namespace internal
}  // namespace testing

#endif
