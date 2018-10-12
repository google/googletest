// Copyright 2018, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "gtest/gtest.h"

namespace {
  class DummyTestClass : public ::testing::Test {
    virtual ~DummyTestClass() {}
    void TestBody() {}
  };
}

#define CREATE_FREE_TEST_INFO(test_case_name, test_name)\
  GTEST_CREATE_FREE_TEST_INFO_(\
    test_case_name, test_name, \
    ::testing::Test, ::testing::internal::GetTestTypeId())

#define GTEST_CREATE_FREE_TEST_INFO_(\
  test_case_name, test_name, parent_class, parent_id)\
  ::testing::internal::MakeAndRegisterTestInfo(\
    #test_case_name, #test_name, NULL, NULL, \
    ::testing::internal::CodeLocation(__FILE__, __LINE__), \
    (parent_id), \
    parent_class::SetUpTestCase, \
    parent_class::TearDownTestCase, \
    new ::testing::internal::TestFactoryImpl<DummyTestClass>);

#define DUMMY_TEST_CASE_NAME DummyTestCase
#define DUMMY_TEST_NAME DummyTest

#define STRINGISE_(symbol) #symbol
#define STRINGISE(symbol) STRINGISE_(symbol)


TEST(RemoveDuplicateTestsTest, DoesNotAffectNonDuplicateTests) {
  ::testing::UnitTest* unit_test = ::testing::UnitTest::GetInstance();

  const auto total_test_case_count = unit_test->total_test_case_count();
  const auto total_test_count = unit_test->total_test_count();

  std::vector<std::pair<std::string, std::string>> test_names;

  // Collect test case and test names
  {
    for (int i = 0; i < total_test_case_count; ++i) {
      const auto test_case = unit_test->GetTestCase(i);
      const auto test_case_name = test_case->name();

      for (int j = 0; j < test_case->total_test_count(); ++j) {
        const auto test_info = test_case->GetTestInfo(j);

        test_names.emplace_back(
          std::make_pair(test_case_name, test_info->name()));
      }
    }
  }

  unit_test->RemoveDuplicateTests();

  ASSERT_EQ(unit_test->total_test_case_count(), total_test_case_count);
  ASSERT_EQ(unit_test->total_test_count(), total_test_count);

  auto name_iter = test_names.begin();

  for (int i = 0; i < total_test_case_count; ++i) {
    const auto test_case = unit_test->GetTestCase(i);
    const auto test_case_name = test_case->name();

    for (int j = 0; j < test_case->total_test_count(); ++j, ++name_iter) {
      const auto test_info = test_case->GetTestInfo(j);
      const auto test_name =
        std::make_pair(test_case_name, test_info->name());

      EXPECT_EQ(test_name.first, (*name_iter).first);
    }
  }
}

TEST(RemoveDuplicateTestsTest, DuplicateTestsGetRemoved) {
  ::testing::UnitTest* unit_test = ::testing::UnitTest::GetInstance();

  const auto total_test_case_count = unit_test->total_test_case_count();
  const auto total_test_count = unit_test->total_test_count();

  CREATE_FREE_TEST_INFO(DUMMY_TEST_CASE_NAME, DUMMY_TEST_NAME);
  CREATE_FREE_TEST_INFO(DUMMY_TEST_CASE_NAME, DUMMY_TEST_NAME);

  const auto dummy_test_case =
      unit_test->GetTestCase(unit_test->total_test_case_count() - 1);

  // Check preconditions after inserting duplicate tests for the test
  // arrangement
  {
    ASSERT_EQ(unit_test->total_test_case_count(), total_test_case_count + 1);
    ASSERT_EQ(unit_test->total_test_count(), total_test_count + 2);
    ASSERT_STREQ(dummy_test_case->name(), STRINGISE(DUMMY_TEST_CASE_NAME));

    for (int i = 0; i < dummy_test_case->total_test_count(); ++i) {
      ASSERT_STREQ(dummy_test_case->GetTestInfo(i)->name(),
                   STRINGISE(DUMMY_TEST_NAME));
    }
  }

  unit_test->RemoveDuplicateTests();

  // The test case remains, the duplicate test does not.
  EXPECT_EQ(unit_test->total_test_case_count(), total_test_case_count + 1);
  EXPECT_EQ(unit_test->total_test_count(), total_test_count + 1);
  EXPECT_EQ(dummy_test_case->total_test_count(), 1);
  EXPECT_STREQ(dummy_test_case->GetTestInfo(0)->name(),
               STRINGISE(DUMMY_TEST_NAME));
}


int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
