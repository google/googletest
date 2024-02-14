#include "cotest/internal/cotest-coro-thread.h"

#include <unistd.h>

#include <iostream>

#include "cotest/internal/cotest-util-logging.h"

namespace coro_impl {

CoroOnThread::CoroOnThread(BodyFunction cofn_, std::string name_) : coro_run_function(cofn_), name(name_) {
    local_thread = std::thread(&CoroOnThread::ThreadRun, this);
    TrySetThreadName();
}

CoroOnThread::~CoroOnThread() {
    COTEST_ASSERT(IsCoroutineExited());
    local_thread.join();
}

std::unique_ptr<Payload> CoroOnThread::Iterate(std::unique_ptr<Payload> &&to_coro) {
    COTEST_ASSERT(phase != Phase::CoroutineExited);
    COTEST_ASSERT(!payload);
    COTEST_ASSERT(!payload_ex);
    payload = std::move(to_coro);
    NotifyPhase(Phase::CoroutineRuns);
    WaitPhases({Phase::MainRuns, Phase::CoroutineExited});
    return std::move(payload);
}

std::unique_ptr<Payload> CoroOnThread::ThrowIn(std::exception_ptr in_ex) {
    COTEST_ASSERT(phase != Phase::CoroutineExited);
    COTEST_ASSERT(!payload);
    COTEST_ASSERT(!payload_ex);
    payload_ex = std::move(in_ex);
    NotifyPhase(Phase::CoroutineRuns);
    WaitPhases({Phase::MainRuns, Phase::CoroutineExited});
    return std::move(payload);
}

void CoroOnThread::Cancel() {
    // We do not support yields in destructors in the coro
    std::unique_ptr<Payload> coro_resp = ThrowIn(MakeException<CancellationException>());
    COTEST_ASSERT(!coro_resp);  // not expecting response to cancellation
    COTEST_ASSERT(phase == Phase::CoroutineExited);
}

bool CoroOnThread::IsCoroutineExited() const {
    std::lock_guard<std::mutex> lk(phase_mutex);
    return phase == Phase::CoroutineExited;
}

void CoroOnThread::SetName(std::string name_) {
    name = name_;
    TrySetThreadName();
}

std::string CoroOnThread::GetName() const { return name; }

std::unique_ptr<Payload> CoroOnThread::Yield(std::unique_ptr<Payload> &&from_coro) {
    COTEST_ASSERT(!payload);
    payload = std::move(from_coro);
    NotifyPhase(Phase::MainRuns);
    WaitPhases({Phase::CoroutineRuns});
    COTEST_ASSERT(!(payload && payload_ex));
    if (payload_ex)
        std::rethrow_exception(payload_ex);
    else
        return std::move(payload);
}

void CoroOnThread::ThreadRun() {
    WaitPhases({Phase::CoroutineRuns});
    COTEST_ASSERT(!payload);  // coro starts up without a message
    try {
        if (payload_ex) std::rethrow_exception(payload_ex);
        coro_run_function();
        payload.reset();
    } catch (const CancellationException &exc) {
        payload_ex = nullptr;
    }
    // At present we don't catch other exceptions and so they cause a terminate.
    // We could re-throw into exterior, but should only catch outside the scope
    // of the coro, at which point the exception caused a cancel (effectively)
    // on both sides.
    NotifyPhase(Phase::CoroutineExited);
}

void CoroOnThread::TrySetThreadName() {
    std::thread::native_handle_type handle = local_thread.native_handle();
#ifdef __linux__
    // Do not fail for OS's that support setting a name
    const int max = 15;
    // The last n characters are more likely to be distinct
    std::string trimmed_name = name.size() <= max ? name : "..." + name.substr(name.size() - (max - 3));
    COTEST_ASSERT(pthread_setname_np(handle, trimmed_name.c_str()) == 0);
#endif
    // COT-10 to add support for MSVC
}

void CoroOnThread::NotifyPhase(Phase new_phase) {
    std::lock_guard<std::mutex> lk(phase_mutex);
    phase = new_phase;
    if (new_phase == Phase::CoroutineRuns) {
        COTEST_ASSERT(!active);
        active = this;
    } else {
        COTEST_ASSERT(active);
        active = nullptr;
    }
    cv.notify_one();
}

void CoroOnThread::WaitPhases(std::set<Phase> phases) {
    std::unique_lock<std::mutex> lk(phase_mutex);
    cv.wait(lk, [&] { return phases.count(phase) > 0; });
}

InteriorInterface *CoroOnThread::GetActive() { return active; }

InteriorInterface *CoroOnThread::active = nullptr;

}  // namespace coro_impl
