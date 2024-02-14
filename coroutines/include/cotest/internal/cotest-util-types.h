#ifndef COROUTINES_INCLUDE_CORO_INTERNAL_COTEST_UTIL_TYPES_H_
#define COROUTINES_INCLUDE_CORO_INTERNAL_COTEST_UTIL_TYPES_H_

#include <functional>
#include <type_traits>

namespace testing {
namespace internal {

template <typename R>
using LaunchLambdaType = std::function<R()>;
using LaunchYielder = std::function<void(const void *)>;
using LaunchLambdaWrapperType = std::function<void(LaunchYielder)>;

template <typename R>
static void YieldWithAddressOfReturn(R &&returned_object, LaunchYielder yielder) {
    // Get past refusing to take the address of a temporary by turning
    // it into a named rvalue. Returned_object lasts until yielder returns,
    // at which point we will have yielded the value back to the coro
    // via its untyped address. Obviously the temporary only lasts as
    // long as a single yield cycle of the launch coro.
    yielder(static_cast<const void *>(&returned_object));
}

template <typename R, typename = void>
struct CotestTypeUtils {
    using StorableR = typename std::remove_reference<R>::type;

    static const void *Generalise(R &&typed_value) { return &typed_value; }

    static const void *Generalise(R &typed_value) { return &typed_value; }

    static R &&Specialise(const void *untyped_value) {
        if (!untyped_value) std::terminate();  // Implementation should use NullCheck() first
        auto untyped_return_value_nc = const_cast<void *>(untyped_value);
        // Returning std::move() is required because R may be unique_ptr or similar
        return std::move(*static_cast<StorableR *>(untyped_return_value_nc));
    }

    static bool NullCheck(const void *untyped_value)  // return true if correct
    {
        return !!untyped_value;  // non-NULL is correct
    }

    static LaunchLambdaWrapperType WrapLaunchLambda(LaunchLambdaType<R> user_lambda) {
        return [user_lambda](LaunchYielder yielder) { YieldWithAddressOfReturn(user_lambda(), yielder); };
    }
};

template <typename R>
struct CotestTypeUtils<R, typename std::enable_if<std::is_reference<R>::value>::type> {
    using StorableR = typename std::remove_reference<R>::type;

    static const void *Generalise(R &typed_value) { return &typed_value; }

    static R Specialise(const void *untyped_value) {
        if (!untyped_value) std::terminate();  // Implementation should use NullCheck() first
        auto untyped_return_value_nc = const_cast<void *>(untyped_value);
        return *static_cast<StorableR *>(untyped_return_value_nc);
    }

    static bool NullCheck(const void *untyped_value)  // return true if correct
    {
        return !!untyped_value;  // non-NULL is correct
    }

    static LaunchLambdaWrapperType WrapLaunchLambda(LaunchLambdaType<R> user_lambda) {
        return [user_lambda](LaunchYielder yielder) { YieldWithAddressOfReturn(user_lambda(), yielder); };
    }
};

template <>
struct CotestTypeUtils<void> {
    inline static void Specialise(const void *untyped_value) {
        if (untyped_value) std::terminate();  // Implementation should use NullCheck() first
        (void)untyped_value;
        return;
    }

    static bool NullCheck(const void *untyped_value)  // return true if correct
    {
        return !untyped_value;  // NULL is correct
    }

    inline static LaunchLambdaWrapperType WrapLaunchLambda(LaunchLambdaType<void> user_lambda) {
        return [user_lambda](LaunchYielder yielder) {
            user_lambda();
            yielder(nullptr);
        };
    }
};

}  // namespace internal
}  // namespace testing

#endif
