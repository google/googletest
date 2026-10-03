// Copyright 2024 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Sample #11 - Advanced parameterized testing with combined type and value
// parameters, custom name generators, and fixture lifecycle demonstration.
//
// This sample demonstrates:
//   1. Type-parameterized tests for generic containers
//   2. Value-parameterized tests with custom name generators
//   3. Fixture lifecycle (SetUp/TearDown ordering)
//   4. Multiple parameter combinations using Combine, Values, Range
//   5. INSTANTIATE_TEST_SUITE_P with various generators

#include "sample11_parameterized_fixture.h"

#include <iostream>
#include <string>
#include <tuple>
#include <vector>

#include "gtest/gtest.h"

// ---------------------------------------------------------------------------
// Part 1: Type-parameterized tests for SortedContainer<T>
// ---------------------------------------------------------------------------

template <typename T>
class SortedContainerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Track lifecycle
    setup_called_ = true;
  }

  void TearDown() override {
    container_.Clear();
    teardown_called_ = true;
  }

  SortedContainer<T> container_;
  bool setup_called_ = false;
  bool teardown_called_ = false;
};

using ContainerTypes = ::testing::Types<int, double, std::string>;
TYPED_TEST_SUITE(SortedContainerTest, ContainerTypes);

// Helper to create a test value for each type
template <typename T>
T MakeValue(int n);

template <>
int MakeValue<int>(int n) {
  return n * 10;
}
template <>
double MakeValue<double>(int n) {
  return n * 1.5;
}
template <>
std::string MakeValue<std::string>(int n) {
  return "item_" + std::to_string(n);
}

TYPED_TEST(SortedContainerTest, InsertMaintainsSortOrder) {
  ASSERT_TRUE(this->setup_called_);
  // Insert in reverse order
  for (int i = 5; i >= 1; --i) {
    this->container_.Insert(MakeValue<TypeParam>(i));
  }
  EXPECT_EQ(this->container_.Size(), 5u);
  // Verify sorted order
  auto all = this->container_.GetAll();
  for (size_t i = 1; i < all.size(); ++i) {
    EXPECT_LE(all[i - 1], all[i]) << "Elements not sorted at index " << i;
  }
}

TYPED_TEST(SortedContainerTest, ContainsFindsInsertedElements) {
  for (int i = 1; i <= 3; ++i) {
    this->container_.Insert(MakeValue<TypeParam>(i));
  }
  for (int i = 1; i <= 3; ++i) {
    EXPECT_TRUE(this->container_.Contains(MakeValue<TypeParam>(i)));
  }
  EXPECT_FALSE(this->container_.Contains(MakeValue<TypeParam>(99)));
}

TYPED_TEST(SortedContainerTest, RemoveDeletesElement) {
  this->container_.Insert(MakeValue<TypeParam>(1));
  this->container_.Insert(MakeValue<TypeParam>(2));
  this->container_.Insert(MakeValue<TypeParam>(3));

  EXPECT_TRUE(this->container_.Remove(MakeValue<TypeParam>(2)));
  EXPECT_EQ(this->container_.Size(), 2u);
  EXPECT_FALSE(this->container_.Contains(MakeValue<TypeParam>(2)));
  EXPECT_TRUE(this->container_.Contains(MakeValue<TypeParam>(1)));
  EXPECT_TRUE(this->container_.Contains(MakeValue<TypeParam>(3)));
}

TYPED_TEST(SortedContainerTest, EmptyContainerBehavior) {
  EXPECT_TRUE(this->container_.Empty());
  EXPECT_EQ(this->container_.Size(), 0u);
  EXPECT_FALSE(this->container_.Remove(MakeValue<TypeParam>(1)));
}

// ---------------------------------------------------------------------------
// Part 2: Value-parameterized tests for StatsCalculator
// ---------------------------------------------------------------------------

struct StatsTestData {
  std::string name;
  std::vector<double> data;
  double expected_mean;
  double expected_median;
  double tolerance;
};

class StatsCalculatorTest : public ::testing::TestWithParam<StatsTestData> {
 protected:
  void SetUp() override { calc_ = StatsCalculator(6); }

  void TearDown() override {
    // Lifecycle demonstration: TearDown called after each test
  }

  StatsCalculator calc_;
};

// Custom name generator for readable test names
struct StatsTestNameGenerator {
  std::string operator()(
      const ::testing::TestParamInfo<StatsTestData>& info) const {
    return info.param.name;
  }
};

TEST_P(StatsCalculatorTest, MeanCalculation) {
  const auto& param = GetParam();
  double result = calc_.Mean(param.data);
  EXPECT_NEAR(result, param.expected_mean, param.tolerance)
      << "Mean calculation failed for dataset: " << param.name;
}

TEST_P(StatsCalculatorTest, MedianCalculation) {
  const auto& param = GetParam();
  double result = calc_.Median(param.data);
  EXPECT_NEAR(result, param.expected_median, param.tolerance)
      << "Median calculation failed for dataset: " << param.name;
}

TEST_P(StatsCalculatorTest, VarianceIsNonNegative) {
  const auto& param = GetParam();
  double variance = calc_.Variance(param.data);
  EXPECT_GE(variance, 0.0) << "Variance should be non-negative";
}

TEST_P(StatsCalculatorTest, StdDevMatchesSqrtVariance) {
  const auto& param = GetParam();
  double variance = calc_.Variance(param.data);
  double stddev = calc_.StandardDeviation(param.data);
  EXPECT_NEAR(stddev, std::sqrt(variance), param.tolerance);
}

INSTANTIATE_TEST_SUITE_P(
    BasicDatasets, StatsCalculatorTest,
    ::testing::Values(
        StatsTestData{"Uniform", {1.0, 2.0, 3.0, 4.0, 5.0}, 3.0, 3.0, 1e-6},
        StatsTestData{"SingleElement", {42.0}, 42.0, 42.0, 1e-6},
        StatsTestData{"TwoElements", {10.0, 20.0}, 15.0, 15.0, 1e-6},
        StatsTestData{"Negative", {-5.0, -3.0, -1.0, 0.0, 2.0}, -1.4, -1.0,
                       1e-6},
        StatsTestData{"LargeValues", {1e6, 2e6, 3e6}, 2e6, 2e6, 1.0}),
    StatsTestNameGenerator());

// ---------------------------------------------------------------------------
// Part 3: Combined parameter tests with Combine and tuple parameters
// ---------------------------------------------------------------------------

class CombinedParamTest
    : public ::testing::TestWithParam<std::tuple<int, bool, std::string>> {
 protected:
  void SetUp() override {
    iterations_ = std::get<0>(GetParam());
    verbose_ = std::get<1>(GetParam());
    mode_ = std::get<2>(GetParam());
  }

  int iterations_;
  bool verbose_;
  std::string mode_;
};

// Custom name generator for combined parameters
struct CombinedTestNameGenerator {
  std::string operator()(
      const ::testing::TestParamInfo<
          std::tuple<int, bool, std::string>>& info) const {
    std::ostringstream oss;
    oss << "Iter" << std::get<0>(info.param) << "_"
        << (std::get<1>(info.param) ? "Verbose" : "Quiet") << "_"
        << std::get<2>(info.param);
    return oss.str();
  }
};

TEST_P(CombinedParamTest, IterationsArePositive) {
  EXPECT_GT(iterations_, 0) << "Iterations must be positive";
}

TEST_P(CombinedParamTest, ModeIsValid) {
  EXPECT_TRUE(mode_ == "fast" || mode_ == "balanced" || mode_ == "thorough")
      << "Unknown mode: " << mode_;
}

TEST_P(CombinedParamTest, ParameterCombinationWorks) {
  // Verify that each parameter combination produces a valid configuration
  TestConfig config(mode_, iterations_, verbose_, 0.001);
  EXPECT_EQ(config.iterations, iterations_);
  EXPECT_EQ(config.verbose, verbose_);
  EXPECT_EQ(config.name, mode_);
  EXPECT_DOUBLE_EQ(config.tolerance, 0.001);
}

INSTANTIATE_TEST_SUITE_P(
    AllCombinations, CombinedParamTest,
    ::testing::Combine(::testing::Values(1, 5, 10),
                       ::testing::Bool(),
                       ::testing::Values("fast", "balanced", "thorough")),
    CombinedTestNameGenerator());

// ---------------------------------------------------------------------------
// Part 4: Range-based parameterized tests
// ---------------------------------------------------------------------------

class RangeParamTest : public ::testing::TestWithParam<int> {
 protected:
  void SetUp() override {
    value_ = GetParam();
    container_.Clear();
  }

  void TearDown() override { container_.Clear(); }

  int value_;
  SortedContainer<int> container_;
};

TEST_P(RangeParamTest, InsertNElementsMaintainsCount) {
  for (int i = 0; i < value_; ++i) {
    container_.Insert(i * 7);  // Non-sequential values
  }
  EXPECT_EQ(container_.Size(), static_cast<size_t>(value_));
}

TEST_P(RangeParamTest, InsertNElementsMaintainsOrder) {
  // Insert in reverse order
  for (int i = value_ - 1; i >= 0; --i) {
    container_.Insert(i);
  }
  auto all = container_.GetAll();
  for (int i = 0; i < value_; ++i) {
    EXPECT_EQ(all[i], i);
  }
}

struct RangeTestNameGenerator {
  std::string operator()(const ::testing::TestParamInfo<int>& info) const {
    return "Count_" + std::to_string(info.param);
  }
};

INSTANTIATE_TEST_SUITE_P(SmallRange, RangeParamTest,
                         ::testing::Range(1, 8),
                         RangeTestNameGenerator());

INSTANTIATE_TEST_SUITE_P(LargeRange, RangeParamTest,
                         ::testing::Values(50, 100, 500),
                         RangeTestNameGenerator());

// ---------------------------------------------------------------------------
// Part 5: Fixture lifecycle demonstration
// ---------------------------------------------------------------------------

class LifecycleTest : public ::testing::Test {
 protected:
  static void SetUpTestSuite() { suite_setup_count_++; }

  static void TearDownTestSuite() { suite_teardown_count_++; }

  void SetUp() override { instance_setup_count_++; }

  void TearDown() override { instance_teardown_count_++; }

  static int suite_setup_count_;
  static int suite_teardown_count_;
  static int instance_setup_count_;
  static int instance_teardown_count_;
};

int LifecycleTest::suite_setup_count_ = 0;
int LifecycleTest::suite_teardown_count_ = 0;
int LifecycleTest::instance_setup_count_ = 0;
int LifecycleTest::instance_teardown_count_ = 0;

TEST_F(LifecycleTest, SuiteSetUpCalledOnce) {
  EXPECT_EQ(suite_setup_count_, 1)
      << "SetUpTestSuite should be called exactly once before all tests";
}

TEST_F(LifecycleTest, InstanceSetUpCalledPerTest) {
  EXPECT_GE(instance_setup_count_, 1)
      << "SetUp should be called at least once";
  EXPECT_EQ(instance_setup_count_, instance_teardown_count_ + 1)
      << "SetUp count should be one more than TearDown count during a test";
}
