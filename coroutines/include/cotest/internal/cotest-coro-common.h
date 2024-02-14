#ifndef COROUTINES_INCLUDE_CORO_COTEST_CORO_COMMON_H_
#define COROUTINES_INCLUDE_CORO_COTEST_CORO_COMMON_H_

#include <exception>
#include <functional>
#include <iostream>

#include "cotest/internal/cotest-util-logging.h"

namespace coro_impl {

class Payload {
   public:
    Payload() = default;
    Payload(const Payload &) = delete;
    Payload(Payload &&) = delete;
    Payload &operator=(const Payload &) = delete;
    Payload &operator=(Payload &&) = delete;
    virtual ~Payload() = default;
    virtual std::string DebugString() const = 0;
};

template <typename PAY, typename ORIG>
const PAY &PeekPayload(const std::unique_ptr<ORIG> &p) {
    return *static_cast<PAY *>(p.get());
}

template <typename PAY, typename ORIG>
std::unique_ptr<PAY> SpecialisePayload(std::unique_ptr<ORIG> &&p) {
    return std::unique_ptr<PAY>(static_cast<PAY *>(p.release()));
}

template <typename PAY, typename... Args>
std::unique_ptr<PAY> MakePayload(Args &&... args) {
    return std::make_unique<PAY>(std::forward<Args>(args)...);
}

class ExteriorInterface {
   public:
    ExteriorInterface() = default;
    ExteriorInterface(const ExteriorInterface &i) = delete;
    ExteriorInterface(ExteriorInterface &&i) = delete;
    ExteriorInterface &operator=(const ExteriorInterface &) = delete;
    ExteriorInterface &operator=(ExteriorInterface &&) = delete;
    virtual ~ExteriorInterface() = default;

    virtual std::unique_ptr<Payload> Iterate(std::unique_ptr<Payload> &&to_coro) = 0;
    virtual std::unique_ptr<Payload> ThrowIn(std::exception_ptr in_ex) = 0;
    virtual void Cancel() = 0;
    virtual bool IsCoroutineExited() const = 0;
    virtual void SetName(std::string name_) = 0;
    virtual std::string GetName() const = 0;
};

class InteriorInterface {
   public:
    InteriorInterface() = default;
    InteriorInterface(const InteriorInterface &i) = delete;
    InteriorInterface(InteriorInterface &&i) = delete;
    InteriorInterface &operator=(const InteriorInterface &) = delete;
    InteriorInterface &operator=(InteriorInterface &&) = delete;
    virtual ~InteriorInterface() = default;

    virtual std::unique_ptr<Payload> Yield(std::unique_ptr<Payload> &&from_coro) = 0;

    // Find out which coro is currently running. Any of the instances of
    // the same concrete type as the one called on, or NULL.
    virtual InteriorInterface *GetActive() = 0;
};

using BodyFunction = std::function<void()>;

class CancellationException : public std::exception {};

// std::exception_ptr lets us be flexible with exception objects without
// having to worry about slicing. The object is stored in the compiler's
// special place and should be considered const - it *cannot* be accessed
// through the exception_ptr - all you can do is pass it around and
// throw it using std::rethrow_exception(). Note that exception_ptr is
// nullable (use = nullptr) but you shouldn't rethrow a null one. The
// exception object could be copied, so don't use identity semantics.
template <class ETYPE, class... Args>
std::exception_ptr MakeException(Args &&... args) try {
    throw ETYPE(args...);
    COTEST_ASSERT(false);  // wut
} catch (ETYPE &)          // Don't catch exceptions thrown in ETYPE's constructor
{
    return std::current_exception();
}

inline std::ostream &operator<<(std::ostream &os, const Payload *payload) {
    if (payload) os << payload->DebugString();
    os << PtrToString(payload);
    return os;
}

}  // namespace coro_impl

#endif
