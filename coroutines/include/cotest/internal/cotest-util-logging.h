#ifndef COROUTINES_INCLUDE_CORO_COTEST_UTIL_LOGGING_H_
#define COROUTINES_INCLUDE_CORO_COTEST_UTIL_LOGGING_H_

//#define COMPARING_LOGS

#include <iomanip>  // setfill etc
#include <sstream>  // std::stringstream

namespace coro_impl {

#define COTEST_STR(S) COTEST_STRW(S)
#define COTEST_STRW(S) #S
#define COTEST_ASSERT(COND)                                                          \
    do {                                                                             \
        if (!(COND)) {                                                               \
            std::cerr << __FILE__ << ":" << __LINE__ << std::endl                    \
                      << " COTEST_ASSERT failed: " << COTEST_STR(COND) << std::endl; \
            std::abort();                                                            \
        }                                                                            \
    } while (0)

inline std::string PtrToString(const void *p) {
    if (!p) return "@NULL";

#ifdef COMPARING_LOGS
    return "@PTR";
#else
    const int biggest_prime_below_1000 = 997;
    int ptrhash = reinterpret_cast<uintptr_t>(p) % biggest_prime_below_1000;

    std::stringstream ss;
    ss << "@" << std::setfill('0') << std::setw(3) << ptrhash;
    return ss.str();
#endif
}

template <typename P, typename Q>
inline std::string PtrToString(const std::pair<P, Q> &p) {
    return "(" + PtrToString(p.first) + ", " + PtrToString(p.second) + ")";
}

// TODO proper logging system
#define COTEST_THIS this << "::" << __func__ << "()"
#define COTEST_THIS_FL COTEST_THIS << " " << file << ":" << line

}  // namespace coro_impl

#endif
