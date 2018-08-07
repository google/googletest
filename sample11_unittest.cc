#include <limits.h>
#include "sample1.h"
#include "gtest/gtest.h"

namespace {
    // Test primes with Fibonacci numbers
    TEST(IsPrimeTest, Positive) {
        EXPECT_TRUE(IsPrime(2))
        EXPECT_TRUE(IsPrime(3))
        EXPECT_TRUE(IsPrime(5))
        EXPECT_FALSE(IsPrime(8))
        EXPECT_TRUE(IsPrime(13))
    }

    // Test primes with even numbers starting from 2
    TEST(IsPrimeTest, Positive) {
        EXPECT_TRUE(IsPrime(2))
        EXPECT_TRUE(IsPrime(4))
        EXPECT_TRUE(IsPrime(6))
        EXPECT_TRUE(IsPrime(8))
    }
} // namespace

// Step 3. Call RUN_ALL_TESTS() in main().
//
// We do this by linking in src/gtest_main.cc file, which consists of
// a main() function which calls RUN_ALL_TESTS() for us.
//
// This runs all the tests you've defined, prints the result, and
// returns 0 if successful, or 1 otherwise.
//
// Did you notice that we didn't register the tests?  The
// RUN_ALL_TESTS() macro magically knows about all the tests we
// defined.  Isn't this convenient?