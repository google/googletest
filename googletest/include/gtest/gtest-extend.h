#ifndef GTEST_INCLUDE_GTEST_EXTEND_GTEST_H_
#define GTEST_INCLUDE_GTEST_EXTEND_GTEST_H_

#include <iostream>

#define NONE "\033[0m"
#define GREEN "\033[32m"
#define RED "\033[31m"

#define GTEST_TEST_THROW_V_(statement, expected_exception, fail) \
  GTEST_AMBIGUOUS_ELSE_BLOCKER_ \
  if (::testing::internal::ConstCharPtr gtest_msg = "") { \
    bool gtest_caught_expected = false; \
    try { \
      GTEST_SUPPRESS_UNREACHABLE_CODE_WARNING_BELOW_(statement); \
    } \
    catch (expected_exception const& e) { \
	  std::cout << GREEN << "[ THROWED  ] " << NONE << e.what() << std::endl ; \
      gtest_caught_expected = true; \
    } \
    catch (...) { \
      gtest_msg.value = \
          "Expected: " #statement " throws an exception of type " \
          #expected_exception ".\n  Actual: it throws a different type."; \
      goto GTEST_CONCAT_TOKEN_(gtest_label_testthrow_, __LINE__); \
    } \
    if (!gtest_caught_expected) { \
      gtest_msg.value = \
          "Expected: " #statement " throws an exception of type " \
          #expected_exception ".\n  Actual: it throws nothing."; \
      goto GTEST_CONCAT_TOKEN_(gtest_label_testthrow_, __LINE__); \
    } \
  } else \
    GTEST_CONCAT_TOKEN_(gtest_label_testthrow_, __LINE__): \
      fail(gtest_msg.value)

#define ASSERT_THROW_V(statement, expected_exception) \
  GTEST_TEST_THROW_V_( statement, expected_exception, GTEST_FATAL_FAILURE_)


#define GTEST_TEST_NO_THROW_V_(statement, fail) \
  GTEST_AMBIGUOUS_ELSE_BLOCKER_ \
  if (::testing::internal::AlwaysTrue()) { \
    try { \
      GTEST_SUPPRESS_UNREACHABLE_CODE_WARNING_BELOW_(statement); \
	} catch(std::exception& e) { \
	  std::cout << RED << "[ THROWED  ] " << NONE << e.what() << std::endl ; \
      goto GTEST_CONCAT_TOKEN_(gtest_label_testnothrow_, __LINE__); \
    } catch (...) { \
      goto GTEST_CONCAT_TOKEN_(gtest_label_testnothrow_, __LINE__); \
    } \
  } else \
    GTEST_CONCAT_TOKEN_(gtest_label_testnothrow_, __LINE__): \
      fail("Expected: " #statement " doesn't throw an exception.\n" \
           "  Actual: it throws.")

#define ASSERT_NO_THROW_V(statement) \
  GTEST_TEST_NO_THROW_V_( statement , GTEST_NONFATAL_FAILURE_)


#endif
