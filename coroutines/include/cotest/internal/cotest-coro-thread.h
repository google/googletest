#ifndef COROUTINES_INCLUDE_CORO_INTERNAL_COTEST_CORO_THREAD_H_
#define COROUTINES_INCLUDE_CORO_INTERNAL_COTEST_CORO_THREAD_H_

#include <condition_variable>
#include <memory>
#include <mutex>
#include <set>
#include <thread>

#include "cotest-coro-common.h"

namespace coro_impl {

/**
 * Implement stacky coroutines on C++ threads
 */
class CoroOnThread final : public ExteriorInterface, public InteriorInterface {
   public:
    // Rule of 5 but disallow copy and move. Immutable and non-nullable.
    CoroOnThread() = delete;
    CoroOnThread(const CoroOnThread &i) = delete;
    CoroOnThread(CoroOnThread &&i) = delete;
    CoroOnThread &operator=(const CoroOnThread &) = delete;
    CoroOnThread &operator=(CoroOnThread &&) = delete;
    ~CoroOnThread();

    CoroOnThread(BodyFunction cofn_, std::string name);

    // ExteriorInterface
    std::unique_ptr<Payload> Iterate(std::unique_ptr<Payload> &&to_coro) final;
    std::unique_ptr<Payload> ThrowIn(std::exception_ptr in_ex) final;
    void Cancel() final;
    bool IsCoroutineExited() const final;
    void SetName(std::string name_) final;
    std::string GetName() const final;

    // InteriorInterface
    std::unique_ptr<Payload> Yield(std::unique_ptr<Payload> &&from_coro) final;

    InteriorInterface *GetActive() final;

   private:
    enum class Phase { CoroutineRuns, MainRuns, CoroutineExited };

    void ThreadRun();
    void TrySetThreadName();
    void NotifyPhase(Phase new_phase);
    void WaitPhases(std::set<Phase> phases);

    BodyFunction coro_run_function;

    std::thread local_thread;

    Phase phase = Phase::MainRuns;
    mutable std::mutex phase_mutex;

    std::condition_variable cv;

    std::unique_ptr<Payload> payload;
    std::exception_ptr payload_ex;

    std::string name;

    static InteriorInterface *active;
};

}  // namespace coro_impl

#endif
