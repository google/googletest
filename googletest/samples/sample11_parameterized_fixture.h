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

#ifndef GOOGLETEST_SAMPLES_SAMPLE11_PARAMETERIZED_FIXTURE_H_
#define GOOGLETEST_SAMPLES_SAMPLE11_PARAMETERIZED_FIXTURE_H_

#include <algorithm>
#include <cmath>
#include <functional>
#include <map>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

// A simple container that supports different storage strategies.
// This is the class under test for our type-parameterized tests.
template <typename T>
class SortedContainer {
 public:
  void Insert(const T& value) {
    auto it = std::lower_bound(data_.begin(), data_.end(), value);
    data_.insert(it, value);
  }

  bool Contains(const T& value) const {
    return std::binary_search(data_.begin(), data_.end(), value);
  }

  size_t Size() const { return data_.size(); }

  bool Empty() const { return data_.empty(); }

  const T& At(size_t index) const { return data_.at(index); }

  void Clear() { data_.clear(); }

  // Returns elements in sorted order
  std::vector<T> GetAll() const { return data_; }

  // Remove first occurrence of value
  bool Remove(const T& value) {
    auto it = std::lower_bound(data_.begin(), data_.end(), value);
    if (it != data_.end() && *it == value) {
      data_.erase(it);
      return true;
    }
    return false;
  }

 private:
  std::vector<T> data_;
};

// A numeric statistics calculator for value-parameterized tests
class StatsCalculator {
 public:
  explicit StatsCalculator(int precision = 6) : precision_(precision) {}

  double Mean(const std::vector<double>& data) const {
    if (data.empty()) return 0.0;
    double sum = std::accumulate(data.begin(), data.end(), 0.0);
    return RoundToPrecision(sum / static_cast<double>(data.size()));
  }

  double Variance(const std::vector<double>& data) const {
    if (data.size() < 2) return 0.0;
    double m = Mean(data);
    double sumSq = 0.0;
    for (const auto& v : data) {
      double diff = v - m;
      sumSq += diff * diff;
    }
    return RoundToPrecision(sumSq / static_cast<double>(data.size() - 1));
  }

  double StandardDeviation(const std::vector<double>& data) const {
    return RoundToPrecision(std::sqrt(Variance(data)));
  }

  double Median(std::vector<double> data) const {
    if (data.empty()) return 0.0;
    std::sort(data.begin(), data.end());
    size_t mid = data.size() / 2;
    if (data.size() % 2 == 0) {
      return RoundToPrecision((data[mid - 1] + data[mid]) / 2.0);
    }
    return data[mid];
  }

  int GetPrecision() const { return precision_; }

 private:
  double RoundToPrecision(double value) const {
    double factor = std::pow(10.0, precision_);
    return std::round(value * factor) / factor;
  }

  int precision_;
};

// Configuration struct for parameterized tests
struct TestConfig {
  std::string name;
  int iterations;
  bool verbose;
  double tolerance;

  TestConfig(const std::string& n, int iter, bool v, double tol)
      : name(n), iterations(iter), verbose(v), tolerance(tol) {}
};

#endif  // GOOGLETEST_SAMPLES_SAMPLE11_PARAMETERIZED_FIXTURE_H_
