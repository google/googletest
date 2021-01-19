// GOOGLETEST_CM0002 DO NOT DELETE
#ifndef GMOCK_INCLUDE_GMOCK_INTERNAL_CUSTOM_GMOCK_GENERATED_ACTIONS_H_
#define GMOCK_INCLUDE_GMOCK_INTERNAL_CUSTOM_GMOCK_GENERATED_ACTIONS_H_

#include "gmock/internal/gmock-port.h"
#if GTEST_GOOGLE3_MODE_

#include <memory>
#include <type_traits>

#include "base/callback.h"
#include "gmock/gmock-actions.h"

namespace testing {
namespace internal {

// Implements the Invoke(callback) action.
template <typename CallbackType, typename Signature>
class InvokeCallbackAction;

template <typename CallbackType, typename R, typename... Args>
class InvokeCallbackAction<CallbackType, R(Args...)> {
 public:
  // The c'tor takes ownership of the callback.
  explicit InvokeCallbackAction(CallbackType* callback) : callback_(callback) {
    callback->CheckIsRepeatable();  // Makes sure the callback is permanent.
  }

  R operator()(Args... args) const {
    return callback_->Run(std::forward<Args>(args)...);
  }

 private:
  const std::shared_ptr<CallbackType> callback_;
};

// Implements the InvokeWithoutArgs(callback) action.
template <typename CallbackType>
class InvokeCallbackWithoutArgsAction {
  const std::shared_ptr<CallbackType> callback_;

 public:
  // The c'tor takes ownership of the callback.
  explicit InvokeCallbackWithoutArgsAction(CallbackType* callback)
      : callback_(callback) {
    callback->CheckIsRepeatable();  // Makes sure the callback is permanent.
  }

  template <typename... Args>
  auto operator()(const Args&...) -> decltype(this->callback_->Run()) {
    return callback_->Run();
  }
};

template <typename T>
struct TypeIdentity {
  using type = T;
};

inline TypeIdentity<void()> CallbackSignatureImpl(Closure*);
template <typename R>
TypeIdentity<R()> CallbackSignatureImpl(ResultCallback<R>*);

template <typename... Args>
TypeIdentity<void(Args...)> CallbackSignatureImpl(Callback1<Args...>*);
template <typename R, typename... Args>
TypeIdentity<R(Args...)> CallbackSignatureImpl(ResultCallback1<R, Args...>*);

template <typename... Args>
TypeIdentity<void(Args...)> CallbackSignatureImpl(Callback2<Args...>*);
template <typename R, typename... Args>
TypeIdentity<R(Args...)> CallbackSignatureImpl(ResultCallback2<R, Args...>*);

template <typename... Args>
TypeIdentity<void(Args...)> CallbackSignatureImpl(Callback3<Args...>*);
template <typename R, typename... Args>
TypeIdentity<R(Args...)> CallbackSignatureImpl(ResultCallback3<R, Args...>*);

template <typename... Args>
TypeIdentity<void(Args...)> CallbackSignatureImpl(Callback4<Args...>*);
template <typename R, typename... Args>
TypeIdentity<R(Args...)> CallbackSignatureImpl(ResultCallback4<R, Args...>*);

template <typename... Args>
TypeIdentity<void(Args...)> CallbackSignatureImpl(Callback5<Args...>*);
template <typename R, typename... Args>
TypeIdentity<R(Args...)> CallbackSignatureImpl(ResultCallback5<R, Args...>*);

template <typename T>
using CallbackSignature = typename decltype(
    internal::CallbackSignatureImpl(std::declval<T*>()))::type;

// Specialization for protocol buffers.
// We support setting a proto2::Message, which doesn't have an assignment
// operator.
template <size_t N, typename A>
struct SetArgumentPointeeAction<
    N, A,
    typename std::enable_if<std::is_base_of<proto2::Message, A>::value>::type> {
  A value;

  template <typename... Args>
  void operator()(const Args&... args) const {
    ::std::get<N>(std::tie(args...))->CopyFrom(value);
  }
};

// Add Invoke overloads for google3 Callback types.

template <typename C, typename... Args,
          typename = internal::CallbackSignature<C>>
auto InvokeArgument(C* cb, Args... args) -> decltype(cb->Run(args...)) {
  return cb->Run(args...);
}

}  // namespace internal

// Add Invoke overloads for google3 Callback types.

// Creates an action that invokes the given callback with the mock
// function's arguments.  The action takes ownership of the callback
// and verifies that it's permanent.
//
// google3 doesn't support callbacks with more than 5
// arguments yet, so we only support invoking callbacks with up to
// 5 arguments.

template <typename Callback>
internal::InvokeCallbackAction<Callback, internal::CallbackSignature<Callback>>
Invoke(Callback* callback) {
  return internal::InvokeCallbackAction<Callback,
                                        internal::CallbackSignature<Callback>>(
      callback);
}

// Creates an action that invokes the given callback with no argument.
// The action takes ownership of the callback and verifies that it's
// permanent.
template <typename Callback, typename = internal::CallbackSignature<Callback>>
internal::InvokeCallbackWithoutArgsAction<Callback> InvokeWithoutArgs(
    Callback* callback) {
  return internal::InvokeCallbackWithoutArgsAction<Callback>(callback);
}

}  // namespace testing

#endif  // GTEST_GOOGLE3_MODE_

#endif  // GMOCK_INCLUDE_GMOCK_INTERNAL_CUSTOM_GMOCK_GENERATED_ACTIONS_H_
