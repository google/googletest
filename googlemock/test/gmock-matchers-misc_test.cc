// Copyright 2007, Google Inc.
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

// Google Mock - a framework for writing C++ mock classes.
//
// This file tests some commonly used argument matchers.

// Silence warning C4244: 'initializing': conversion from 'int' to 'short',
// possible loss of data and C4100, unreferenced local parameter
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4244)
#pragma warning(disable : 4100)
#endif

#include "test/gmock-matchers_test.h"

namespace testing {
namespace gmock_matchers_test {
namespace {

TEST(AddressTest, NonConst) {
  int n = 1;
  const Matcher<int> m = Address(Eq(&n));

  EXPECT_TRUE(m.Matches(n));

  int other = 5;

  EXPECT_FALSE(m.Matches(other));

  int& n_ref = n;

  EXPECT_TRUE(m.Matches(n_ref));
}

TEST(AddressTest, Const) {
  const int n = 1;
  const Matcher<int> m = Address(Eq(&n));

  EXPECT_TRUE(m.Matches(n));

  int other = 5;

  EXPECT_FALSE(m.Matches(other));
}

TEST(AddressTest, MatcherDoesntCopy) {
  std::unique_ptr<int> n(new int(1));
  const Matcher<std::unique_ptr<int>> m = Address(Eq(&n));

  EXPECT_TRUE(m.Matches(n));
}

TEST(AddressTest, Describe) {
  Matcher<int> matcher = Address(_);
  EXPECT_EQ("has address that is anything", Describe(matcher));
  EXPECT_EQ("does not have address that is anything",
            DescribeNegation(matcher));
}

// The following two tests verify that values without a public copy
// ctor can be used as arguments to matchers like Eq(), Ge(), and etc
// with the help of ByRef().

class NotCopyable {
 public:
  explicit NotCopyable(int a_value) : value_(a_value) {}

  int value() const { return value_; }

  bool operator==(const NotCopyable& rhs) const {
    return value() == rhs.value();
  }

  bool operator>=(const NotCopyable& rhs) const {
    return value() >= rhs.value();
  }

 private:
  int value_;

  NotCopyable(const NotCopyable&) = delete;
  NotCopyable& operator=(const NotCopyable&) = delete;
};

TEST(ByRefTest, AllowsNotCopyableConstValueInMatchers) {
  const NotCopyable const_value1(1);
  const Matcher<const NotCopyable&> m = Eq(ByRef(const_value1));

  const NotCopyable n1(1), n2(2);
  EXPECT_TRUE(m.Matches(n1));
  EXPECT_FALSE(m.Matches(n2));
}

TEST(ByRefTest, AllowsNotCopyableValueInMatchers) {
  NotCopyable value2(2);
  const Matcher<NotCopyable&> m = Ge(ByRef(value2));

  NotCopyable n1(1), n2(2);
  EXPECT_FALSE(m.Matches(n1));
  EXPECT_TRUE(m.Matches(n2));
}

TEST(IsEmptyTest, ImplementsIsEmpty) {
  vector<int> container;
  EXPECT_THAT(container, IsEmpty());
  container.push_back(0);
  EXPECT_THAT(container, Not(IsEmpty()));
  container.push_back(1);
  EXPECT_THAT(container, Not(IsEmpty()));
}

TEST(IsEmptyTest, WorksWithString) {
  std::string text;
  EXPECT_THAT(text, IsEmpty());
  text = "foo";
  EXPECT_THAT(text, Not(IsEmpty()));
  text = std::string("\0", 1);
  EXPECT_THAT(text, Not(IsEmpty()));
}

TEST(IsEmptyTest, CanDescribeSelf) {
  Matcher<vector<int>> m = IsEmpty();
  EXPECT_EQ("is empty", Describe(m));
  EXPECT_EQ("isn't empty", DescribeNegation(m));
}

TEST(IsEmptyTest, ExplainsResult) {
  Matcher<vector<int>> m = IsEmpty();
  vector<int> container;
  EXPECT_EQ("", Explain(m, container));
  container.push_back(0);
  EXPECT_EQ("whose size is 1", Explain(m, container));
}

TEST(IsEmptyTest, WorksWithMoveOnly) {
  ContainerHelper helper;
  EXPECT_CALL(helper, Call(IsEmpty()));
  helper.Call({});
}

TEST(IsTrueTest, IsTrueIsFalse) {
  EXPECT_THAT(true, IsTrue());
  EXPECT_THAT(false, IsFalse());
  EXPECT_THAT(true, Not(IsFalse()));
  EXPECT_THAT(false, Not(IsTrue()));
  EXPECT_THAT(0, Not(IsTrue()));
  EXPECT_THAT(0, IsFalse());
  EXPECT_THAT(nullptr, Not(IsTrue()));
  EXPECT_THAT(nullptr, IsFalse());
  EXPECT_THAT(-1, IsTrue());
  EXPECT_THAT(-1, Not(IsFalse()));
  EXPECT_THAT(1, IsTrue());
  EXPECT_THAT(1, Not(IsFalse()));
  EXPECT_THAT(2, IsTrue());
  EXPECT_THAT(2, Not(IsFalse()));
  int a = 42;
  EXPECT_THAT(a, IsTrue());
  EXPECT_THAT(a, Not(IsFalse()));
  EXPECT_THAT(&a, IsTrue());
  EXPECT_THAT(&a, Not(IsFalse()));
  EXPECT_THAT(false, Not(IsTrue()));
  EXPECT_THAT(true, Not(IsFalse()));
  EXPECT_THAT(std::true_type(), IsTrue());
  EXPECT_THAT(std::true_type(), Not(IsFalse()));
  EXPECT_THAT(std::false_type(), IsFalse());
  EXPECT_THAT(std::false_type(), Not(IsTrue()));
  EXPECT_THAT(nullptr, Not(IsTrue()));
  EXPECT_THAT(nullptr, IsFalse());
  std::unique_ptr<int> null_unique;
  std::unique_ptr<int> nonnull_unique(new int(0));
  EXPECT_THAT(null_unique, Not(IsTrue()));
  EXPECT_THAT(null_unique, IsFalse());
  EXPECT_THAT(nonnull_unique, IsTrue());
  EXPECT_THAT(nonnull_unique, Not(IsFalse()));
}

#if GTEST_HAS_TYPED_TEST
// Tests ContainerEq with different container types, and
// different element types.

template <typename T>
class ContainerEqTest : public testing::Test {};

typedef testing::Types<set<int>, vector<size_t>, multiset<size_t>, list<int>>
    ContainerEqTestTypes;

TYPED_TEST_SUITE(ContainerEqTest, ContainerEqTestTypes);

// Tests that the filled container is equal to itself.
TYPED_TEST(ContainerEqTest, EqualsSelf) {
  static const int vals[] = {1, 1, 2, 3, 5, 8};
  TypeParam my_set(vals, vals + 6);
  const Matcher<TypeParam> m = ContainerEq(my_set);
  EXPECT_TRUE(m.Matches(my_set));
  EXPECT_EQ("", Explain(m, my_set));
}

// Tests that missing values are reported.
TYPED_TEST(ContainerEqTest, ValueMissing) {
  static const int vals[] = {1, 1, 2, 3, 5, 8};
  static const int test_vals[] = {2, 1, 8, 5};
  TypeParam my_set(vals, vals + 6);
  TypeParam test_set(test_vals, test_vals + 4);
  const Matcher<TypeParam> m = ContainerEq(my_set);
  EXPECT_FALSE(m.Matches(test_set));
  EXPECT_EQ("which doesn't have these expected elements: 3",
            Explain(m, test_set));
}

// Tests that added values are reported.
TYPED_TEST(ContainerEqTest, ValueAdded) {
  static const int vals[] = {1, 1, 2, 3, 5, 8};
  static const int test_vals[] = {1, 2, 3, 5, 8, 46};
  TypeParam my_set(vals, vals + 6);
  TypeParam test_set(test_vals, test_vals + 6);
  const Matcher<const TypeParam&> m = ContainerEq(my_set);
  EXPECT_FALSE(m.Matches(test_set));
  EXPECT_EQ("which has these unexpected elements: 46", Explain(m, test_set));
}

// Tests that added and missing values are reported together.
TYPED_TEST(ContainerEqTest, ValueAddedAndRemoved) {
  static const int vals[] = {1, 1, 2, 3, 5, 8};
  static const int test_vals[] = {1, 2, 3, 8, 46};
  TypeParam my_set(vals, vals + 6);
  TypeParam test_set(test_vals, test_vals + 5);
  const Matcher<TypeParam> m = ContainerEq(my_set);
  EXPECT_FALSE(m.Matches(test_set));
  EXPECT_EQ(
      "which has these unexpected elements: 46,\n"
      "and doesn't have these expected elements: 5",
      Explain(m, test_set));
}

// Tests duplicated value -- expect no explanation.
TYPED_TEST(ContainerEqTest, DuplicateDifference) {
  static const int vals[] = {1, 1, 2, 3, 5, 8};
  static const int test_vals[] = {1, 2, 3, 5, 8};
  TypeParam my_set(vals, vals + 6);
  TypeParam test_set(test_vals, test_vals + 5);
  const Matcher<const TypeParam&> m = ContainerEq(my_set);
  // Depending on the container, match may be true or false
  // But in any case there should be no explanation.
  EXPECT_EQ("", Explain(m, test_set));
}
#endif  // GTEST_HAS_TYPED_TEST

// Tests that multiple missing values are reported.
// Using just vector here, so order is predictable.
TEST(ContainerEqExtraTest, MultipleValuesMissing) {
  static const int vals[] = {1, 1, 2, 3, 5, 8};
  static const int test_vals[] = {2, 1, 5};
  vector<int> my_set(vals, vals + 6);
  vector<int> test_set(test_vals, test_vals + 3);
  const Matcher<vector<int>> m = ContainerEq(my_set);
  EXPECT_FALSE(m.Matches(test_set));
  EXPECT_EQ("which doesn't have these expected elements: 3, 8",
            Explain(m, test_set));
}

// Tests that added values are reported.
// Using just vector here, so order is predictable.
TEST(ContainerEqExtraTest, MultipleValuesAdded) {
  static const int vals[] = {1, 1, 2, 3, 5, 8};
  static const int test_vals[] = {1, 2, 92, 3, 5, 8, 46};
  list<size_t> my_set(vals, vals + 6);
  list<size_t> test_set(test_vals, test_vals + 7);
  const Matcher<const list<size_t>&> m = ContainerEq(my_set);
  EXPECT_FALSE(m.Matches(test_set));
  EXPECT_EQ("which has these unexpected elements: 92, 46",
            Explain(m, test_set));
}

// Tests that added and missing values are reported together.
TEST(ContainerEqExtraTest, MultipleValuesAddedAndRemoved) {
  static const int vals[] = {1, 1, 2, 3, 5, 8};
  static const int test_vals[] = {1, 2, 3, 92, 46};
  list<size_t> my_set(vals, vals + 6);
  list<size_t> test_set(test_vals, test_vals + 5);
  const Matcher<const list<size_t>> m = ContainerEq(my_set);
  EXPECT_FALSE(m.Matches(test_set));
  EXPECT_EQ(
      "which has these unexpected elements: 92, 46,\n"
      "and doesn't have these expected elements: 5, 8",
      Explain(m, test_set));
}

// Tests to see that duplicate elements are detected,
// but (as above) not reported in the explanation.
TEST(ContainerEqExtraTest, MultiSetOfIntDuplicateDifference) {
  static const int vals[] = {1, 1, 2, 3, 5, 8};
  static const int test_vals[] = {1, 2, 3, 5, 8};
  vector<int> my_set(vals, vals + 6);
  vector<int> test_set(test_vals, test_vals + 5);
  const Matcher<vector<int>> m = ContainerEq(my_set);
  EXPECT_TRUE(m.Matches(my_set));
  EXPECT_FALSE(m.Matches(test_set));
  // There is nothing to report when both sets contain all the same values.
  EXPECT_EQ("", Explain(m, test_set));
}

// Tests that ContainerEq works for non-trivial associative containers,
// like maps.
TEST(ContainerEqExtraTest, WorksForMaps) {
  map<int, std::string> my_map;
  my_map[0] = "a";
  my_map[1] = "b";

  map<int, std::string> test_map;
  test_map[0] = "aa";
  test_map[1] = "b";

  const Matcher<const map<int, std::string>&> m = ContainerEq(my_map);
  EXPECT_TRUE(m.Matches(my_map));
  EXPECT_FALSE(m.Matches(test_map));

  EXPECT_EQ(
      "which has these unexpected elements: (0, \"aa\"),\n"
      "and doesn't have these expected elements: (0, \"a\")",
      Explain(m, test_map));
}

TEST(ContainerEqExtraTest, WorksForNativeArray) {
  int a1[] = {1, 2, 3};
  int a2[] = {1, 2, 3};
  int b[] = {1, 2, 4};

  EXPECT_THAT(a1, ContainerEq(a2));
  EXPECT_THAT(a1, Not(ContainerEq(b)));
}

TEST(ContainerEqExtraTest, WorksForTwoDimensionalNativeArray) {
  const char a1[][3] = {"hi", "lo"};
  const char a2[][3] = {"hi", "lo"};
  const char b[][3] = {"lo", "hi"};

  // Tests using ContainerEq() in the first dimension.
  EXPECT_THAT(a1, ContainerEq(a2));
  EXPECT_THAT(a1, Not(ContainerEq(b)));

  // Tests using ContainerEq() in the second dimension.
  EXPECT_THAT(a1, ElementsAre(ContainerEq(a2[0]), ContainerEq(a2[1])));
  EXPECT_THAT(a1, ElementsAre(Not(ContainerEq(b[0])), ContainerEq(a2[1])));
}

TEST(ContainerEqExtraTest, WorksForNativeArrayAsTuple) {
  const int a1[] = {1, 2, 3};
  const int a2[] = {1, 2, 3};
  const int b[] = {1, 2, 3, 4};

  const int* const p1 = a1;
  EXPECT_THAT(std::make_tuple(p1, 3), ContainerEq(a2));
  EXPECT_THAT(std::make_tuple(p1, 3), Not(ContainerEq(b)));

  const int c[] = {1, 3, 2};
  EXPECT_THAT(std::make_tuple(p1, 3), Not(ContainerEq(c)));
}

TEST(ContainerEqExtraTest, CopiesNativeArrayParameter) {
  std::string a1[][3] = {{"hi", "hello", "ciao"}, {"bye", "see you", "ciao"}};

  std::string a2[][3] = {{"hi", "hello", "ciao"}, {"bye", "see you", "ciao"}};

  const Matcher<const std::string(&)[2][3]> m = ContainerEq(a2);
  EXPECT_THAT(a1, m);

  a2[0][0] = "ha";
  EXPECT_THAT(a1, m);
}

namespace {

// Used as a check on the more complex max flow method used in the
// real testing::internal::FindMaxBipartiteMatching. This method is
// compatible but runs in worst-case factorial time, so we only
// use it in testing for small problem sizes.
template <typename Graph>
class BacktrackingMaxBPMState {
 public:
  // Does not take ownership of 'g'.
  explicit BacktrackingMaxBPMState(const Graph* g) : graph_(g) {}

  ElementMatcherPairs Compute() {
    if (graph_->LhsSize() == 0 || graph_->RhsSize() == 0) {
      return best_so_far_;
    }
    lhs_used_.assign(graph_->LhsSize(), kUnused);
    rhs_used_.assign(graph_->RhsSize(), kUnused);
    for (size_t irhs = 0; irhs < graph_->RhsSize(); ++irhs) {
      matches_.clear();
      RecurseInto(irhs);
      if (best_so_far_.size() == graph_->RhsSize()) break;
    }
    return best_so_far_;
  }

 private:
  static const size_t kUnused = static_cast<size_t>(-1);

  void PushMatch(size_t lhs, size_t rhs) {
    matches_.push_back(ElementMatcherPair(lhs, rhs));
    lhs_used_[lhs] = rhs;
    rhs_used_[rhs] = lhs;
    if (matches_.size() > best_so_far_.size()) {
      best_so_far_ = matches_;
    }
  }

  void PopMatch() {
    const ElementMatcherPair& back = matches_.back();
    lhs_used_[back.first] = kUnused;
    rhs_used_[back.second] = kUnused;
    matches_.pop_back();
  }

  bool RecurseInto(size_t irhs) {
    if (rhs_used_[irhs] != kUnused) {
      return true;
    }
    for (size_t ilhs = 0; ilhs < graph_->LhsSize(); ++ilhs) {
      if (lhs_used_[ilhs] != kUnused) {
        continue;
      }
      if (!graph_->HasEdge(ilhs, irhs)) {
        continue;
      }
      PushMatch(ilhs, irhs);
      if (best_so_far_.size() == graph_->RhsSize()) {
        return false;
      }
      for (size_t mi = irhs + 1; mi < graph_->RhsSize(); ++mi) {
        if (!RecurseInto(mi)) return false;
      }
      PopMatch();
    }
    return true;
  }

  const Graph* graph_;  // not owned
  std::vector<size_t> lhs_used_;
  std::vector<size_t> rhs_used_;
  ElementMatcherPairs matches_;
  ElementMatcherPairs best_so_far_;
};

template <typename Graph>
const size_t BacktrackingMaxBPMState<Graph>::kUnused;

}  // namespace

// Implement a simple backtracking algorithm to determine if it is possible
// to find one element per matcher, without reusing elements.
template <typename Graph>
ElementMatcherPairs FindBacktrackingMaxBPM(const Graph& g) {
  return BacktrackingMaxBPMState<Graph>(&g).Compute();
}

class BacktrackingBPMTest : public ::testing::Test {};

// Tests the MaxBipartiteMatching algorithm with square matrices.
// The single int param is the # of nodes on each of the left and right sides.
class BipartiteTest : public ::testing::TestWithParam<size_t> {};

// Verify all match graphs up to some moderate number of edges.
TEST_P(BipartiteTest, Exhaustive) {
  size_t nodes = GetParam();
  MatchMatrix graph(nodes, nodes);
  do {
    ElementMatcherPairs matches = internal::FindMaxBipartiteMatching(graph);
    EXPECT_EQ(FindBacktrackingMaxBPM(graph).size(), matches.size())
        << "graph: " << graph.DebugString();
    // Check that all elements of matches are in the graph.
    // Check that elements of first and second are unique.
    std::vector<bool> seen_element(graph.LhsSize());
    std::vector<bool> seen_matcher(graph.RhsSize());
    SCOPED_TRACE(PrintToString(matches));
    for (size_t i = 0; i < matches.size(); ++i) {
      size_t ilhs = matches[i].first;
      size_t irhs = matches[i].second;
      EXPECT_TRUE(graph.HasEdge(ilhs, irhs));
      EXPECT_FALSE(seen_element[ilhs]);
      EXPECT_FALSE(seen_matcher[irhs]);
      seen_element[ilhs] = true;
      seen_matcher[irhs] = true;
    }
  } while (graph.NextGraph());
}

INSTANTIATE_TEST_SUITE_P(AllGraphs, BipartiteTest,
                         ::testing::Range(size_t{0}, size_t{5}));

// Parameterized by a pair interpreted as (LhsSize, RhsSize).
class BipartiteNonSquareTest
    : public ::testing::TestWithParam<std::pair<size_t, size_t>> {};

TEST_F(BipartiteNonSquareTest, SimpleBacktracking) {
  //   .......
  // 0:-----\ :
  // 1:---\ | :
  // 2:---\ | :
  // 3:-\ | | :
  //  :.......:
  //    0 1 2
  MatchMatrix g(4, 3);
  constexpr std::array<std::array<size_t, 2>, 4> kEdges = {
      {{{0, 2}}, {{1, 1}}, {{2, 1}}, {{3, 0}}}};
  for (size_t i = 0; i < kEdges.size(); ++i) {
    g.SetEdge(kEdges[i][0], kEdges[i][1], true);
  }
  EXPECT_THAT(FindBacktrackingMaxBPM(g),
              ElementsAre(Pair(3, 0), Pair(AnyOf(1, 2), 1), Pair(0, 2)))
      << g.DebugString();
}

// Verify a few nonsquare matrices.
TEST_P(BipartiteNonSquareTest, Exhaustive) {
  size_t nlhs = GetParam().first;
  size_t nrhs = GetParam().second;
  MatchMatrix graph(nlhs, nrhs);
  do {
    EXPECT_EQ(FindBacktrackingMaxBPM(graph).size(),
              internal::FindMaxBipartiteMatching(graph).size())
        << "graph: " << graph.DebugString()
        << "\nbacktracking: " << PrintToString(FindBacktrackingMaxBPM(graph))
        << "\nmax flow: "
        << PrintToString(internal::FindMaxBipartiteMatching(graph));
  } while (graph.NextGraph());
}

INSTANTIATE_TEST_SUITE_P(
    AllGraphs, BipartiteNonSquareTest,
    testing::Values(std::make_pair(1, 2), std::make_pair(2, 1),
                    std::make_pair(3, 2), std::make_pair(2, 3),
                    std::make_pair(4, 1), std::make_pair(1, 4),
                    std::make_pair(4, 3), std::make_pair(3, 4)));

class BipartiteRandomTest
    : public ::testing::TestWithParam<std::pair<int, int>> {};

// Verifies a large sample of larger graphs.
TEST_P(BipartiteRandomTest, LargerNets) {
  int nodes = GetParam().first;
  int iters = GetParam().second;
  MatchMatrix graph(static_cast<size_t>(nodes), static_cast<size_t>(nodes));

  auto seed = static_cast<uint32_t>(GTEST_FLAG_GET(random_seed));
  if (seed == 0) {
    seed = static_cast<uint32_t>(time(nullptr));
  }

  for (; iters > 0; --iters, ++seed) {
    srand(static_cast<unsigned int>(seed));
    graph.Randomize();
    EXPECT_EQ(FindBacktrackingMaxBPM(graph).size(),
              internal::FindMaxBipartiteMatching(graph).size())
        << " graph: " << graph.DebugString()
        << "\nTo reproduce the failure, rerun the test with the flag"
           " --"
        << GTEST_FLAG_PREFIX_ << "random_seed=" << seed;
  }
}

// Test argument is a std::pair<int, int> representing (nodes, iters).
INSTANTIATE_TEST_SUITE_P(Samples, BipartiteRandomTest,
                         testing::Values(std::make_pair(5, 10000),
                                         std::make_pair(6, 5000),
                                         std::make_pair(7, 2000),
                                         std::make_pair(8, 500),
                                         std::make_pair(9, 100)));

// Tests IsReadableTypeName().

TEST(IsReadableTypeNameTest, ReturnsTrueForShortNames) {
  EXPECT_TRUE(IsReadableTypeName("int"));
  EXPECT_TRUE(IsReadableTypeName("const unsigned char*"));
  EXPECT_TRUE(IsReadableTypeName("MyMap<int, void*>"));
  EXPECT_TRUE(IsReadableTypeName("void (*)(int, bool)"));
}

TEST(IsReadableTypeNameTest, ReturnsTrueForLongNonTemplateNonFunctionNames) {
  EXPECT_TRUE(IsReadableTypeName("my_long_namespace::MyClassName"));
  EXPECT_TRUE(IsReadableTypeName("int [5][6][7][8][9][10][11]"));
  EXPECT_TRUE(IsReadableTypeName("my_namespace::MyOuterClass::MyInnerClass"));
}

TEST(IsReadableTypeNameTest, ReturnsFalseForLongTemplateNames) {
  EXPECT_FALSE(
      IsReadableTypeName("basic_string<char, std::char_traits<char> >"));
  EXPECT_FALSE(IsReadableTypeName("std::vector<int, std::alloc_traits<int> >"));
}

TEST(IsReadableTypeNameTest, ReturnsFalseForLongFunctionTypeNames) {
  EXPECT_FALSE(IsReadableTypeName("void (&)(int, bool, char, float)"));
}

// Tests FormatMatcherDescription().

TEST(FormatMatcherDescriptionTest, WorksForEmptyDescription) {
  EXPECT_EQ("is even",
            FormatMatcherDescription(false, "IsEven", {}, Strings()));
  EXPECT_EQ("not (is even)",
            FormatMatcherDescription(true, "IsEven", {}, Strings()));

  EXPECT_EQ("equals (a: 5)",
            FormatMatcherDescription(false, "Equals", {"a"}, {"5"}));

  EXPECT_EQ(
      "is in range (a: 5, b: 8)",
      FormatMatcherDescription(false, "IsInRange", {"a", "b"}, {"5", "8"}));
}

INSTANTIATE_GTEST_MATCHER_TEST_P(MatcherTupleTest);

TEST_P(MatcherTupleTestP, ExplainsMatchFailure) {
  stringstream ss1;
  ExplainMatchFailureTupleTo(
      std::make_tuple(Matcher<char>(Eq('a')), GreaterThan(5)),
      std::make_tuple('a', 10), &ss1);
  EXPECT_EQ("", ss1.str());  // Successful match.

  stringstream ss2;
  ExplainMatchFailureTupleTo(
      std::make_tuple(GreaterThan(5), Matcher<char>(Eq('a'))),
      std::make_tuple(2, 'b'), &ss2);
  EXPECT_EQ(
      "  Expected arg #0: is > 5\n"
      "           Actual: 2, which is 3 less than 5\n"
      "  Expected arg #1: is equal to 'a' (97, 0x61)\n"
      "           Actual: 'b' (98, 0x62)\n",
      ss2.str());  // Failed match where both arguments need explanation.

  stringstream ss3;
  ExplainMatchFailureTupleTo(
      std::make_tuple(GreaterThan(5), Matcher<char>(Eq('a'))),
      std::make_tuple(2, 'a'), &ss3);
  EXPECT_EQ(
      "  Expected arg #0: is > 5\n"
      "           Actual: 2, which is 3 less than 5\n",
      ss3.str());  // Failed match where only one argument needs
                   // explanation.
}

// Sample optional type implementation with minimal requirements for use with
// Optional matcher.
template <typename T>
class SampleOptional {
 public:
  using value_type = T;
  explicit SampleOptional(T value)
      : value_(std::move(value)), has_value_(true) {}
  SampleOptional() : value_(), has_value_(false) {}
  operator bool() const { return has_value_; }
  const T& operator*() const { return value_; }

 private:
  T value_;
  bool has_value_;
};

TEST(OptionalTest, DescribesSelf) {
  const Matcher<SampleOptional<int>> m = Optional(Eq(1));
  EXPECT_EQ("value is equal to 1", Describe(m));
}

TEST(OptionalTest, ExplainsSelf) {
  const Matcher<SampleOptional<int>> m = Optional(Eq(1));
  EXPECT_EQ("whose value 1 matches", Explain(m, SampleOptional<int>(1)));
  EXPECT_EQ("whose value 2 doesn't match", Explain(m, SampleOptional<int>(2)));
}

TEST(OptionalTest, MatchesNonEmptyOptional) {
  const Matcher<SampleOptional<int>> m1 = Optional(1);
  const Matcher<SampleOptional<int>> m2 = Optional(Eq(2));
  const Matcher<SampleOptional<int>> m3 = Optional(Lt(3));
  SampleOptional<int> opt(1);
  EXPECT_TRUE(m1.Matches(opt));
  EXPECT_FALSE(m2.Matches(opt));
  EXPECT_TRUE(m3.Matches(opt));
}

TEST(OptionalTest, DoesNotMatchNullopt) {
  const Matcher<SampleOptional<int>> m = Optional(1);
  SampleOptional<int> empty;
  EXPECT_FALSE(m.Matches(empty));
}

TEST(OptionalTest, WorksWithMoveOnly) {
  Matcher<SampleOptional<std::unique_ptr<int>>> m = Optional(Eq(nullptr));
  EXPECT_TRUE(m.Matches(SampleOptional<std::unique_ptr<int>>(nullptr)));
}

class SampleVariantIntString {
 public:
  SampleVariantIntString(int i) : i_(i), has_int_(true) {}
  SampleVariantIntString(const std::string& s) : s_(s), has_int_(false) {}

  template <typename T>
  friend bool holds_alternative(const SampleVariantIntString& value) {
    return value.has_int_ == std::is_same<T, int>::value;
  }

  template <typename T>
  friend const T& get(const SampleVariantIntString& value) {
    return value.get_impl(static_cast<T*>(nullptr));
  }

 private:
  const int& get_impl(int*) const { return i_; }
  const std::string& get_impl(std::string*) const { return s_; }

  int i_;
  std::string s_;
  bool has_int_;
};

TEST(VariantTest, DescribesSelf) {
  const Matcher<SampleVariantIntString> m = VariantWith<int>(Eq(1));
  EXPECT_THAT(Describe(m), ContainsRegex("is a variant<> with value of type "
                                         "'.*' and the value is equal to 1"));
}

TEST(VariantTest, ExplainsSelf) {
  const Matcher<SampleVariantIntString> m = VariantWith<int>(Eq(1));
  EXPECT_THAT(Explain(m, SampleVariantIntString(1)),
              ContainsRegex("whose value 1"));
  EXPECT_THAT(Explain(m, SampleVariantIntString("A")),
              HasSubstr("whose value is not of type '"));
  EXPECT_THAT(Explain(m, SampleVariantIntString(2)),
              "whose value 2 doesn't match");
}

TEST(VariantTest, FullMatch) {
  Matcher<SampleVariantIntString> m = VariantWith<int>(Eq(1));
  EXPECT_TRUE(m.Matches(SampleVariantIntString(1)));

  m = VariantWith<std::string>(Eq("1"));
  EXPECT_TRUE(m.Matches(SampleVariantIntString("1")));
}

TEST(VariantTest, TypeDoesNotMatch) {
  Matcher<SampleVariantIntString> m = VariantWith<int>(Eq(1));
  EXPECT_FALSE(m.Matches(SampleVariantIntString("1")));

  m = VariantWith<std::string>(Eq("1"));
  EXPECT_FALSE(m.Matches(SampleVariantIntString(1)));
}

TEST(VariantTest, InnerDoesNotMatch) {
  Matcher<SampleVariantIntString> m = VariantWith<int>(Eq(1));
  EXPECT_FALSE(m.Matches(SampleVariantIntString(2)));

  m = VariantWith<std::string>(Eq("1"));
  EXPECT_FALSE(m.Matches(SampleVariantIntString("2")));
}

class SampleAnyType {
 public:
  explicit SampleAnyType(int i) : index_(0), i_(i) {}
  explicit SampleAnyType(const std::string& s) : index_(1), s_(s) {}

  template <typename T>
  friend const T* any_cast(const SampleAnyType* any) {
    return any->get_impl(static_cast<T*>(nullptr));
  }

 private:
  int index_;
  int i_;
  std::string s_;

  const int* get_impl(int*) const { return index_ == 0 ? &i_ : nullptr; }
  const std::string* get_impl(std::string*) const {
    return index_ == 1 ? &s_ : nullptr;
  }
};

TEST(AnyWithTest, FullMatch) {
  Matcher<SampleAnyType> m = AnyWith<int>(Eq(1));
  EXPECT_TRUE(m.Matches(SampleAnyType(1)));
}

TEST(AnyWithTest, TestBadCastType) {
  Matcher<SampleAnyType> m = AnyWith<std::string>(Eq("fail"));
  EXPECT_FALSE(m.Matches(SampleAnyType(1)));
}

TEST(AnyWithTest, TestUseInContainers) {
  std::vector<SampleAnyType> a;
  a.emplace_back(1);
  a.emplace_back(2);
  a.emplace_back(3);
  EXPECT_THAT(
      a, ElementsAreArray({AnyWith<int>(1), AnyWith<int>(2), AnyWith<int>(3)}));

  std::vector<SampleAnyType> b;
  b.emplace_back("hello");
  b.emplace_back("merhaba");
  b.emplace_back("salut");
  EXPECT_THAT(b, ElementsAreArray({AnyWith<std::string>("hello"),
                                   AnyWith<std::string>("merhaba"),
                                   AnyWith<std::string>("salut")}));
}
TEST(AnyWithTest, TestCompare) {
  EXPECT_THAT(SampleAnyType(1), AnyWith<int>(Gt(0)));
}

TEST(AnyWithTest, DescribesSelf) {
  const Matcher<const SampleAnyType&> m = AnyWith<int>(Eq(1));
  EXPECT_THAT(Describe(m), ContainsRegex("is an 'any' type with value of type "
                                         "'.*' and the value is equal to 1"));
}

TEST(AnyWithTest, ExplainsSelf) {
  const Matcher<const SampleAnyType&> m = AnyWith<int>(Eq(1));

  EXPECT_THAT(Explain(m, SampleAnyType(1)), ContainsRegex("whose value 1"));
  EXPECT_THAT(Explain(m, SampleAnyType("A")),
              HasSubstr("whose value is not of type '"));
  EXPECT_THAT(Explain(m, SampleAnyType(2)), "whose value 2 doesn't match");
}

// Tests Args<k0, ..., kn>(m).

TEST(ArgsTest, AcceptsZeroTemplateArg) {
  const std::tuple<int, bool> t(5, true);
  EXPECT_THAT(t, Args<>(Eq(std::tuple<>())));
  EXPECT_THAT(t, Not(Args<>(Ne(std::tuple<>()))));
}

TEST(ArgsTest, AcceptsOneTemplateArg) {
  const std::tuple<int, bool> t(5, true);
  EXPECT_THAT(t, Args<0>(Eq(std::make_tuple(5))));
  EXPECT_THAT(t, Args<1>(Eq(std::make_tuple(true))));
  EXPECT_THAT(t, Not(Args<1>(Eq(std::make_tuple(false)))));
}

TEST(ArgsTest, AcceptsTwoTemplateArgs) {
  const std::tuple<short, int, long> t(4, 5, 6L);  // NOLINT

  EXPECT_THAT(t, (Args<0, 1>(Lt())));
  EXPECT_THAT(t, (Args<1, 2>(Lt())));
  EXPECT_THAT(t, Not(Args<0, 2>(Gt())));
}

TEST(ArgsTest, AcceptsRepeatedTemplateArgs) {
  const std::tuple<short, int, long> t(4, 5, 6L);  // NOLINT
  EXPECT_THAT(t, (Args<0, 0>(Eq())));
  EXPECT_THAT(t, Not(Args<1, 1>(Ne())));
}

TEST(ArgsTest, AcceptsDecreasingTemplateArgs) {
  const std::tuple<short, int, long> t(4, 5, 6L);  // NOLINT
  EXPECT_THAT(t, (Args<2, 0>(Gt())));
  EXPECT_THAT(t, Not(Args<2, 1>(Lt())));
}

MATCHER(SumIsZero, "") {
  return std::get<0>(arg) + std::get<1>(arg) + std::get<2>(arg) == 0;
}

TEST(ArgsTest, AcceptsMoreTemplateArgsThanArityOfOriginalTuple) {
  EXPECT_THAT(std::make_tuple(-1, 2), (Args<0, 0, 1>(SumIsZero())));
  EXPECT_THAT(std::make_tuple(1, 2), Not(Args<0, 0, 1>(SumIsZero())));
}

TEST(ArgsTest, CanBeNested) {
  const std::tuple<short, int, long, int> t(4, 5, 6L, 6);  // NOLINT
  EXPECT_THAT(t, (Args<1, 2, 3>(Args<1, 2>(Eq()))));
  EXPECT_THAT(t, (Args<0, 1, 3>(Args<0, 2>(Lt()))));
}

TEST(ArgsTest, CanMatchTupleByValue) {
  typedef std::tuple<char, int, int> Tuple3;
  const Matcher<Tuple3> m = Args<1, 2>(Lt());
  EXPECT_TRUE(m.Matches(Tuple3('a', 1, 2)));
  EXPECT_FALSE(m.Matches(Tuple3('b', 2, 2)));
}

TEST(ArgsTest, CanMatchTupleByReference) {
  typedef std::tuple<char, char, int> Tuple3;
  const Matcher<const Tuple3&> m = Args<0, 1>(Lt());
  EXPECT_TRUE(m.Matches(Tuple3('a', 'b', 2)));
  EXPECT_FALSE(m.Matches(Tuple3('b', 'b', 2)));
}

// Validates that arg is printed as str.
MATCHER_P(PrintsAs, str, "") { return testing::PrintToString(arg) == str; }

TEST(ArgsTest, AcceptsTenTemplateArgs) {
  EXPECT_THAT(std::make_tuple(0, 1L, 2, 3L, 4, 5, 6, 7, 8, 9),
              (Args<9, 8, 7, 6, 5, 4, 3, 2, 1, 0>(
                  PrintsAs("(9, 8, 7, 6, 5, 4, 3, 2, 1, 0)"))));
  EXPECT_THAT(std::make_tuple(0, 1L, 2, 3L, 4, 5, 6, 7, 8, 9),
              Not(Args<9, 8, 7, 6, 5, 4, 3, 2, 1, 0>(
                  PrintsAs("(0, 8, 7, 6, 5, 4, 3, 2, 1, 0)"))));
}

TEST(ArgsTest, DescirbesSelfCorrectly) {
  const Matcher<std::tuple<int, bool, char>> m = Args<2, 0>(Lt());
  EXPECT_EQ(
      "are a tuple whose fields (#2, #0) are a pair where "
      "the first < the second",
      Describe(m));
}

TEST(ArgsTest, DescirbesNestedArgsCorrectly) {
  const Matcher<const std::tuple<int, bool, char, int>&> m =
      Args<0, 2, 3>(Args<2, 0>(Lt()));
  EXPECT_EQ(
      "are a tuple whose fields (#0, #2, #3) are a tuple "
      "whose fields (#2, #0) are a pair where the first < the second",
      Describe(m));
}

TEST(ArgsTest, DescribesNegationCorrectly) {
  const Matcher<std::tuple<int, char>> m = Args<1, 0>(Gt());
  EXPECT_EQ(
      "are a tuple whose fields (#1, #0) aren't a pair "
      "where the first > the second",
      DescribeNegation(m));
}

TEST(ArgsTest, ExplainsMatchResultWithoutInnerExplanation) {
  const Matcher<std::tuple<bool, int, int>> m = Args<1, 2>(Eq());
  EXPECT_EQ("whose fields (#1, #2) are (42, 42)",
            Explain(m, std::make_tuple(false, 42, 42)));
  EXPECT_EQ("whose fields (#1, #2) are (42, 43)",
            Explain(m, std::make_tuple(false, 42, 43)));
}

// For testing Args<>'s explanation.
class LessThanMatcher : public MatcherInterface<std::tuple<char, int>> {
 public:
  void DescribeTo(::std::ostream* /*os*/) const override {}

  bool MatchAndExplain(std::tuple<char, int> value,
                       MatchResultListener* listener) const override {
    const int diff = std::get<0>(value) - std::get<1>(value);
    if (diff > 0) {
      *listener << "where the first value is " << diff
                << " more than the second";
    }
    return diff < 0;
  }
};

Matcher<std::tuple<char, int>> LessThan() {
  return MakeMatcher(new LessThanMatcher);
}

TEST(ArgsTest, ExplainsMatchResultWithInnerExplanation) {
  const Matcher<std::tuple<char, int, int>> m = Args<0, 2>(LessThan());
  EXPECT_EQ(
      "whose fields (#0, #2) are ('a' (97, 0x61), 42), "
      "where the first value is 55 more than the second",
      Explain(m, std::make_tuple('a', 42, 42)));
  EXPECT_EQ("whose fields (#0, #2) are ('\\0', 43)",
            Explain(m, std::make_tuple('\0', 42, 43)));
}

// Tests for the MATCHER*() macro family.

// Tests that a simple MATCHER() definition works.

MATCHER(IsEven, "") { return (arg % 2) == 0; }

TEST(MatcherMacroTest, Works) {
  const Matcher<int> m = IsEven();
  EXPECT_TRUE(m.Matches(6));
  EXPECT_FALSE(m.Matches(7));

  EXPECT_EQ("is even", Describe(m));
  EXPECT_EQ("not (is even)", DescribeNegation(m));
  EXPECT_EQ("", Explain(m, 6));
  EXPECT_EQ("", Explain(m, 7));
}

// This also tests that the description string can reference 'negation'.
MATCHER(IsEven2, negation ? "is odd" : "is even") {
  if ((arg % 2) == 0) {
    // Verifies that we can stream to result_listener, a listener
    // supplied by the MATCHER macro implicitly.
    *result_listener << "OK";
    return true;
  } else {
    *result_listener << "% 2 == " << (arg % 2);
    return false;
  }
}

// This also tests that the description string can reference matcher
// parameters.
MATCHER_P2(EqSumOf, x, y,
           std::string(negation ? "doesn't equal" : "equals") + " the sum of " +
               PrintToString(x) + " and " + PrintToString(y)) {
  if (arg == (x + y)) {
    *result_listener << "OK";
    return true;
  } else {
    // Verifies that we can stream to the underlying stream of
    // result_listener.
    if (result_listener->stream() != nullptr) {
      *result_listener->stream() << "diff == " << (x + y - arg);
    }
    return false;
  }
}

// Tests that the matcher description can reference 'negation' and the
// matcher parameters.
TEST(MatcherMacroTest, DescriptionCanReferenceNegationAndParameters) {
  const Matcher<int> m1 = IsEven2();
  EXPECT_EQ("is even", Describe(m1));
  EXPECT_EQ("is odd", DescribeNegation(m1));

  const Matcher<int> m2 = EqSumOf(5, 9);
  EXPECT_EQ("equals the sum of 5 and 9", Describe(m2));
  EXPECT_EQ("doesn't equal the sum of 5 and 9", DescribeNegation(m2));
}

// Tests explaining match result in a MATCHER* macro.
TEST(MatcherMacroTest, CanExplainMatchResult) {
  const Matcher<int> m1 = IsEven2();
  EXPECT_EQ("OK", Explain(m1, 4));
  EXPECT_EQ("% 2 == 1", Explain(m1, 5));

  const Matcher<int> m2 = EqSumOf(1, 2);
  EXPECT_EQ("OK", Explain(m2, 3));
  EXPECT_EQ("diff == -1", Explain(m2, 4));
}

// Tests that the body of MATCHER() can reference the type of the
// value being matched.

MATCHER(IsEmptyString, "") {
  StaticAssertTypeEq<::std::string, arg_type>();
  return arg.empty();
}

MATCHER(IsEmptyStringByRef, "") {
  StaticAssertTypeEq<const ::std::string&, arg_type>();
  return arg.empty();
}

TEST(MatcherMacroTest, CanReferenceArgType) {
  const Matcher<::std::string> m1 = IsEmptyString();
  EXPECT_TRUE(m1.Matches(""));

  const Matcher<const ::std::string&> m2 = IsEmptyStringByRef();
  EXPECT_TRUE(m2.Matches(""));
}

// Tests that MATCHER() can be used in a namespace.

namespace matcher_test {
MATCHER(IsOdd, "") { return (arg % 2) != 0; }
}  // namespace matcher_test

TEST(MatcherMacroTest, WorksInNamespace) {
  Matcher<int> m = matcher_test::IsOdd();
  EXPECT_FALSE(m.Matches(4));
  EXPECT_TRUE(m.Matches(5));
}

// Tests that Value() can be used to compose matchers.
MATCHER(IsPositiveOdd, "") {
  return Value(arg, matcher_test::IsOdd()) && arg > 0;
}

TEST(MatcherMacroTest, CanBeComposedUsingValue) {
  EXPECT_THAT(3, IsPositiveOdd());
  EXPECT_THAT(4, Not(IsPositiveOdd()));
  EXPECT_THAT(-1, Not(IsPositiveOdd()));
}

// Tests that a simple MATCHER_P() definition works.

MATCHER_P(IsGreaterThan32And, n, "") { return arg > 32 && arg > n; }

TEST(MatcherPMacroTest, Works) {
  const Matcher<int> m = IsGreaterThan32And(5);
  EXPECT_TRUE(m.Matches(36));
  EXPECT_FALSE(m.Matches(5));

  EXPECT_EQ("is greater than 32 and (n: 5)", Describe(m));
  EXPECT_EQ("not (is greater than 32 and (n: 5))", DescribeNegation(m));
  EXPECT_EQ("", Explain(m, 36));
  EXPECT_EQ("", Explain(m, 5));
}

// Tests that the description is calculated correctly from the matcher name.
MATCHER_P(_is_Greater_Than32and_, n, "") { return arg > 32 && arg > n; }

TEST(MatcherPMacroTest, GeneratesCorrectDescription) {
  const Matcher<int> m = _is_Greater_Than32and_(5);

  EXPECT_EQ("is greater than 32 and (n: 5)", Describe(m));
  EXPECT_EQ("not (is greater than 32 and (n: 5))", DescribeNegation(m));
  EXPECT_EQ("", Explain(m, 36));
  EXPECT_EQ("", Explain(m, 5));
}

// Tests that a MATCHER_P matcher can be explicitly instantiated with
// a reference parameter type.

class UncopyableFoo {
 public:
  explicit UncopyableFoo(char value) : value_(value) { (void)value_; }

  UncopyableFoo(const UncopyableFoo&) = delete;
  void operator=(const UncopyableFoo&) = delete;

 private:
  char value_;
};

MATCHER_P(ReferencesUncopyable, variable, "") { return &arg == &variable; }

TEST(MatcherPMacroTest, WorksWhenExplicitlyInstantiatedWithReference) {
  UncopyableFoo foo1('1'), foo2('2');
  const Matcher<const UncopyableFoo&> m =
      ReferencesUncopyable<const UncopyableFoo&>(foo1);

  EXPECT_TRUE(m.Matches(foo1));
  EXPECT_FALSE(m.Matches(foo2));

  // We don't want the address of the parameter printed, as most
  // likely it will just annoy the user.  If the address is
  // interesting, the user should consider passing the parameter by
  // pointer instead.
  EXPECT_EQ("references uncopyable (variable: 1-byte object <31>)",
            Describe(m));
}

// Tests that the body of MATCHER_Pn() can reference the parameter
// types.

MATCHER_P3(ParamTypesAreIntLongAndChar, foo, bar, baz, "") {
  StaticAssertTypeEq<int, foo_type>();
  StaticAssertTypeEq<long, bar_type>();  // NOLINT
  StaticAssertTypeEq<char, baz_type>();
  return arg == 0;
}

TEST(MatcherPnMacroTest, CanReferenceParamTypes) {
  EXPECT_THAT(0, ParamTypesAreIntLongAndChar(10, 20L, 'a'));
}

// Tests that a MATCHER_Pn matcher can be explicitly instantiated with
// reference parameter types.

MATCHER_P2(ReferencesAnyOf, variable1, variable2, "") {
  return &arg == &variable1 || &arg == &variable2;
}

TEST(MatcherPnMacroTest, WorksWhenExplicitlyInstantiatedWithReferences) {
  UncopyableFoo foo1('1'), foo2('2'), foo3('3');
  const Matcher<const UncopyableFoo&> const_m =
      ReferencesAnyOf<const UncopyableFoo&, const UncopyableFoo&>(foo1, foo2);

  EXPECT_TRUE(const_m.Matches(foo1));
  EXPECT_TRUE(const_m.Matches(foo2));
  EXPECT_FALSE(const_m.Matches(foo3));

  const Matcher<UncopyableFoo&> m =
      ReferencesAnyOf<UncopyableFoo&, UncopyableFoo&>(foo1, foo2);

  EXPECT_TRUE(m.Matches(foo1));
  EXPECT_TRUE(m.Matches(foo2));
  EXPECT_FALSE(m.Matches(foo3));
}

TEST(MatcherPnMacroTest,
     GeneratesCorretDescriptionWhenExplicitlyInstantiatedWithReferences) {
  UncopyableFoo foo1('1'), foo2('2');
  const Matcher<const UncopyableFoo&> m =
      ReferencesAnyOf<const UncopyableFoo&, const UncopyableFoo&>(foo1, foo2);

  // We don't want the addresses of the parameters printed, as most
  // likely they will just annoy the user.  If the addresses are
  // interesting, the user should consider passing the parameters by
  // pointers instead.
  EXPECT_EQ(
      "references any of (variable1: 1-byte object <31>, variable2: 1-byte "
      "object <32>)",
      Describe(m));
}

// Tests that a simple MATCHER_P2() definition works.

MATCHER_P2(IsNotInClosedRange, low, hi, "") { return arg < low || arg > hi; }

TEST(MatcherPnMacroTest, Works) {
  const Matcher<const long&> m = IsNotInClosedRange(10, 20);  // NOLINT
  EXPECT_TRUE(m.Matches(36L));
  EXPECT_FALSE(m.Matches(15L));

  EXPECT_EQ("is not in closed range (low: 10, hi: 20)", Describe(m));
  EXPECT_EQ("not (is not in closed range (low: 10, hi: 20))",
            DescribeNegation(m));
  EXPECT_EQ("", Explain(m, 36L));
  EXPECT_EQ("", Explain(m, 15L));
}

// Tests that MATCHER*() definitions can be overloaded on the number
// of parameters; also tests MATCHER_Pn() where n >= 3.

MATCHER(EqualsSumOf, "") { return arg == 0; }
MATCHER_P(EqualsSumOf, a, "") { return arg == a; }
MATCHER_P2(EqualsSumOf, a, b, "") { return arg == a + b; }
MATCHER_P3(EqualsSumOf, a, b, c, "") { return arg == a + b + c; }
MATCHER_P4(EqualsSumOf, a, b, c, d, "") { return arg == a + b + c + d; }
MATCHER_P5(EqualsSumOf, a, b, c, d, e, "") { return arg == a + b + c + d + e; }
MATCHER_P6(EqualsSumOf, a, b, c, d, e, f, "") {
  return arg == a + b + c + d + e + f;
}
MATCHER_P7(EqualsSumOf, a, b, c, d, e, f, g, "") {
  return arg == a + b + c + d + e + f + g;
}
MATCHER_P8(EqualsSumOf, a, b, c, d, e, f, g, h, "") {
  return arg == a + b + c + d + e + f + g + h;
}
MATCHER_P9(EqualsSumOf, a, b, c, d, e, f, g, h, i, "") {
  return arg == a + b + c + d + e + f + g + h + i;
}
MATCHER_P10(EqualsSumOf, a, b, c, d, e, f, g, h, i, j, "") {
  return arg == a + b + c + d + e + f + g + h + i + j;
}

TEST(MatcherPnMacroTest, CanBeOverloadedOnNumberOfParameters) {
  EXPECT_THAT(0, EqualsSumOf());
  EXPECT_THAT(1, EqualsSumOf(1));
  EXPECT_THAT(12, EqualsSumOf(10, 2));
  EXPECT_THAT(123, EqualsSumOf(100, 20, 3));
  EXPECT_THAT(1234, EqualsSumOf(1000, 200, 30, 4));
  EXPECT_THAT(12345, EqualsSumOf(10000, 2000, 300, 40, 5));
  EXPECT_THAT("abcdef",
              EqualsSumOf(::std::string("a"), 'b', 'c', "d", "e", 'f'));
  EXPECT_THAT("abcdefg",
              EqualsSumOf(::std::string("a"), 'b', 'c', "d", "e", 'f', 'g'));
  EXPECT_THAT("abcdefgh", EqualsSumOf(::std::string("a"), 'b', 'c', "d", "e",
                                      'f', 'g', "h"));
  EXPECT_THAT("abcdefghi", EqualsSumOf(::std::string("a"), 'b', 'c', "d", "e",
                                       'f', 'g', "h", 'i'));
  EXPECT_THAT("abcdefghij",
              EqualsSumOf(::std::string("a"), 'b', 'c', "d", "e", 'f', 'g', "h",
                          'i', ::std::string("j")));

  EXPECT_THAT(1, Not(EqualsSumOf()));
  EXPECT_THAT(-1, Not(EqualsSumOf(1)));
  EXPECT_THAT(-12, Not(EqualsSumOf(10, 2)));
  EXPECT_THAT(-123, Not(EqualsSumOf(100, 20, 3)));
  EXPECT_THAT(-1234, Not(EqualsSumOf(1000, 200, 30, 4)));
  EXPECT_THAT(-12345, Not(EqualsSumOf(10000, 2000, 300, 40, 5)));
  EXPECT_THAT("abcdef ",
              Not(EqualsSumOf(::std::string("a"), 'b', 'c', "d", "e", 'f')));
  EXPECT_THAT("abcdefg ", Not(EqualsSumOf(::std::string("a"), 'b', 'c', "d",
                                          "e", 'f', 'g')));
  EXPECT_THAT("abcdefgh ", Not(EqualsSumOf(::std::string("a"), 'b', 'c', "d",
                                           "e", 'f', 'g', "h")));
  EXPECT_THAT("abcdefghi ", Not(EqualsSumOf(::std::string("a"), 'b', 'c', "d",
                                            "e", 'f', 'g', "h", 'i')));
  EXPECT_THAT("abcdefghij ",
              Not(EqualsSumOf(::std::string("a"), 'b', 'c', "d", "e", 'f', 'g',
                              "h", 'i', ::std::string("j"))));
}

// Tests that a MATCHER_Pn() definition can be instantiated with any
// compatible parameter types.
TEST(MatcherPnMacroTest, WorksForDifferentParameterTypes) {
  EXPECT_THAT(123, EqualsSumOf(100L, 20, static_cast<char>(3)));
  EXPECT_THAT("abcd", EqualsSumOf(::std::string("a"), "b", 'c', "d"));

  EXPECT_THAT(124, Not(EqualsSumOf(100L, 20, static_cast<char>(3))));
  EXPECT_THAT("abcde", Not(EqualsSumOf(::std::string("a"), "b", 'c', "d")));
}

// Tests that the matcher body can promote the parameter types.

MATCHER_P2(EqConcat, prefix, suffix, "") {
  // The following lines promote the two parameters to desired types.
  std::string prefix_str(prefix);
  char suffix_char = static_cast<char>(suffix);
  return arg == prefix_str + suffix_char;
}

TEST(MatcherPnMacroTest, SimpleTypePromotion) {
  Matcher<std::string> no_promo = EqConcat(std::string("foo"), 't');
  Matcher<const std::string&> promo = EqConcat("foo", static_cast<int>('t'));
  EXPECT_FALSE(no_promo.Matches("fool"));
  EXPECT_FALSE(promo.Matches("fool"));
  EXPECT_TRUE(no_promo.Matches("foot"));
  EXPECT_TRUE(promo.Matches("foot"));
}

// Verifies the type of a MATCHER*.

TEST(MatcherPnMacroTest, TypesAreCorrect) {
  // EqualsSumOf() must be assignable to a EqualsSumOfMatcher variable.
  EqualsSumOfMatcher a0 = EqualsSumOf();

  // EqualsSumOf(1) must be assignable to a EqualsSumOfMatcherP variable.
  EqualsSumOfMatcherP<int> a1 = EqualsSumOf(1);

  // EqualsSumOf(p1, ..., pk) must be assignable to a EqualsSumOfMatcherPk
  // variable, and so on.
  EqualsSumOfMatcherP2<int, char> a2 = EqualsSumOf(1, '2');
  EqualsSumOfMatcherP3<int, int, char> a3 = EqualsSumOf(1, 2, '3');
  EqualsSumOfMatcherP4<int, int, int, char> a4 = EqualsSumOf(1, 2, 3, '4');
  EqualsSumOfMatcherP5<int, int, int, int, char> a5 =
      EqualsSumOf(1, 2, 3, 4, '5');
  EqualsSumOfMatcherP6<int, int, int, int, int, char> a6 =
      EqualsSumOf(1, 2, 3, 4, 5, '6');
  EqualsSumOfMatcherP7<int, int, int, int, int, int, char> a7 =
      EqualsSumOf(1, 2, 3, 4, 5, 6, '7');
  EqualsSumOfMatcherP8<int, int, int, int, int, int, int, char> a8 =
      EqualsSumOf(1, 2, 3, 4, 5, 6, 7, '8');
  EqualsSumOfMatcherP9<int, int, int, int, int, int, int, int, char> a9 =
      EqualsSumOf(1, 2, 3, 4, 5, 6, 7, 8, '9');
  EqualsSumOfMatcherP10<int, int, int, int, int, int, int, int, int, char> a10 =
      EqualsSumOf(1, 2, 3, 4, 5, 6, 7, 8, 9, '0');

  // Avoid "unused variable" warnings.
  (void)a0;
  (void)a1;
  (void)a2;
  (void)a3;
  (void)a4;
  (void)a5;
  (void)a6;
  (void)a7;
  (void)a8;
  (void)a9;
  (void)a10;
}

// Tests that matcher-typed parameters can be used in Value() inside a
// MATCHER_Pn definition.

// Succeeds if arg matches exactly 2 of the 3 matchers.
MATCHER_P3(TwoOf, m1, m2, m3, "") {
  const int count = static_cast<int>(Value(arg, m1)) +
                    static_cast<int>(Value(arg, m2)) +
                    static_cast<int>(Value(arg, m3));
  return count == 2;
}

TEST(MatcherPnMacroTest, CanUseMatcherTypedParameterInValue) {
  EXPECT_THAT(42, TwoOf(Gt(0), Lt(50), Eq(10)));
  EXPECT_THAT(0, Not(TwoOf(Gt(-1), Lt(1), Eq(0))));
}

// Tests Contains().Times().

INSTANTIATE_GTEST_MATCHER_TEST_P(ContainsTimes);

TEST(ContainsTimes, ListMatchesWhenElementQuantityMatches) {
  list<int> some_list;
  some_list.push_back(3);
  some_list.push_back(1);
  some_list.push_back(2);
  some_list.push_back(3);
  EXPECT_THAT(some_list, Contains(3).Times(2));
  EXPECT_THAT(some_list, Contains(2).Times(1));
  EXPECT_THAT(some_list, Contains(Ge(2)).Times(3));
  EXPECT_THAT(some_list, Contains(Ge(2)).Times(Gt(2)));
  EXPECT_THAT(some_list, Contains(4).Times(0));
  EXPECT_THAT(some_list, Contains(_).Times(4));
  EXPECT_THAT(some_list, Not(Contains(5).Times(1)));
  EXPECT_THAT(some_list, Contains(5).Times(_));  // Times(_) always matches
  EXPECT_THAT(some_list, Not(Contains(3).Times(1)));
  EXPECT_THAT(some_list, Contains(3).Times(Not(1)));
  EXPECT_THAT(list<int>{}, Not(Contains(_)));
}

TEST_P(ContainsTimesP, ExplainsMatchResultCorrectly) {
  const int a[2] = {1, 2};
  Matcher<const int(&)[2]> m = Contains(2).Times(3);
  EXPECT_EQ(
      "whose element #1 matches but whose match quantity of 1 does not match",
      Explain(m, a));

  m = Contains(3).Times(0);
  EXPECT_EQ("has no element that matches and whose match quantity of 0 matches",
            Explain(m, a));

  m = Contains(3).Times(4);
  EXPECT_EQ(
      "has no element that matches and whose match quantity of 0 does not "
      "match",
      Explain(m, a));

  m = Contains(2).Times(4);
  EXPECT_EQ(
      "whose element #1 matches but whose match quantity of 1 does not "
      "match",
      Explain(m, a));

  m = Contains(GreaterThan(0)).Times(2);
  EXPECT_EQ("whose elements (0, 1) match and whose match quantity of 2 matches",
            Explain(m, a));

  m = Contains(GreaterThan(10)).Times(Gt(1));
  EXPECT_EQ(
      "has no element that matches and whose match quantity of 0 does not "
      "match",
      Explain(m, a));

  m = Contains(GreaterThan(0)).Times(GreaterThan<size_t>(5));
  EXPECT_EQ(
      "whose elements (0, 1) match but whose match quantity of 2 does not "
      "match, which is 3 less than 5",
      Explain(m, a));
}

TEST(ContainsTimes, DescribesItselfCorrectly) {
  Matcher<vector<int>> m = Contains(1).Times(2);
  EXPECT_EQ("quantity of elements that match is equal to 1 is equal to 2",
            Describe(m));

  Matcher<vector<int>> m2 = Not(m);
  EXPECT_EQ("quantity of elements that match is equal to 1 isn't equal to 2",
            Describe(m2));
}

// Tests AllOfArray()

TEST(AllOfArrayTest, BasicForms) {
  // Iterator
  std::vector<int> v0{};
  std::vector<int> v1{1};
  std::vector<int> v2{2, 3};
  std::vector<int> v3{4, 4, 4};
  EXPECT_THAT(0, AllOfArray(v0.begin(), v0.end()));
  EXPECT_THAT(1, AllOfArray(v1.begin(), v1.end()));
  EXPECT_THAT(2, Not(AllOfArray(v1.begin(), v1.end())));
  EXPECT_THAT(3, Not(AllOfArray(v2.begin(), v2.end())));
  EXPECT_THAT(4, AllOfArray(v3.begin(), v3.end()));
  // Pointer +  size
  int ar[6] = {1, 2, 3, 4, 4, 4};
  EXPECT_THAT(0, AllOfArray(ar, 0));
  EXPECT_THAT(1, AllOfArray(ar, 1));
  EXPECT_THAT(2, Not(AllOfArray(ar, 1)));
  EXPECT_THAT(3, Not(AllOfArray(ar + 1, 3)));
  EXPECT_THAT(4, AllOfArray(ar + 3, 3));
  // Array
  // int ar0[0];  Not usable
  int ar1[1] = {1};
  int ar2[2] = {2, 3};
  int ar3[3] = {4, 4, 4};
  // EXPECT_THAT(0, Not(AllOfArray(ar0)));  // Cannot work
  EXPECT_THAT(1, AllOfArray(ar1));
  EXPECT_THAT(2, Not(AllOfArray(ar1)));
  EXPECT_THAT(3, Not(AllOfArray(ar2)));
  EXPECT_THAT(4, AllOfArray(ar3));
  // Container
  EXPECT_THAT(0, AllOfArray(v0));
  EXPECT_THAT(1, AllOfArray(v1));
  EXPECT_THAT(2, Not(AllOfArray(v1)));
  EXPECT_THAT(3, Not(AllOfArray(v2)));
  EXPECT_THAT(4, AllOfArray(v3));
  // Initializer
  EXPECT_THAT(0, AllOfArray<int>({}));  // Requires template arg.
  EXPECT_THAT(1, AllOfArray({1}));
  EXPECT_THAT(2, Not(AllOfArray({1})));
  EXPECT_THAT(3, Not(AllOfArray({2, 3})));
  EXPECT_THAT(4, AllOfArray({4, 4, 4}));
}

TEST(AllOfArrayTest, Matchers) {
  // vector
  std::vector<Matcher<int>> matchers{Ge(1), Lt(2)};
  EXPECT_THAT(0, Not(AllOfArray(matchers)));
  EXPECT_THAT(1, AllOfArray(matchers));
  EXPECT_THAT(2, Not(AllOfArray(matchers)));
  // initializer_list
  EXPECT_THAT(0, Not(AllOfArray({Ge(0), Ge(1)})));
  EXPECT_THAT(1, AllOfArray({Ge(0), Ge(1)}));
}

INSTANTIATE_GTEST_MATCHER_TEST_P(AnyOfArrayTest);

TEST(AnyOfArrayTest, BasicForms) {
  // Iterator
  std::vector<int> v0{};
  std::vector<int> v1{1};
  std::vector<int> v2{2, 3};
  EXPECT_THAT(0, Not(AnyOfArray(v0.begin(), v0.end())));
  EXPECT_THAT(1, AnyOfArray(v1.begin(), v1.end()));
  EXPECT_THAT(2, Not(AnyOfArray(v1.begin(), v1.end())));
  EXPECT_THAT(3, AnyOfArray(v2.begin(), v2.end()));
  EXPECT_THAT(4, Not(AnyOfArray(v2.begin(), v2.end())));
  // Pointer +  size
  int ar[3] = {1, 2, 3};
  EXPECT_THAT(0, Not(AnyOfArray(ar, 0)));
  EXPECT_THAT(1, AnyOfArray(ar, 1));
  EXPECT_THAT(2, Not(AnyOfArray(ar, 1)));
  EXPECT_THAT(3, AnyOfArray(ar + 1, 2));
  EXPECT_THAT(4, Not(AnyOfArray(ar + 1, 2)));
  // Array
  // int ar0[0];  Not usable
  int ar1[1] = {1};
  int ar2[2] = {2, 3};
  // EXPECT_THAT(0, Not(AnyOfArray(ar0)));  // Cannot work
  EXPECT_THAT(1, AnyOfArray(ar1));
  EXPECT_THAT(2, Not(AnyOfArray(ar1)));
  EXPECT_THAT(3, AnyOfArray(ar2));
  EXPECT_THAT(4, Not(AnyOfArray(ar2)));
  // Container
  EXPECT_THAT(0, Not(AnyOfArray(v0)));
  EXPECT_THAT(1, AnyOfArray(v1));
  EXPECT_THAT(2, Not(AnyOfArray(v1)));
  EXPECT_THAT(3, AnyOfArray(v2));
  EXPECT_THAT(4, Not(AnyOfArray(v2)));
  // Initializer
  EXPECT_THAT(0, Not(AnyOfArray<int>({})));  // Requires template arg.
  EXPECT_THAT(1, AnyOfArray({1}));
  EXPECT_THAT(2, Not(AnyOfArray({1})));
  EXPECT_THAT(3, AnyOfArray({2, 3}));
  EXPECT_THAT(4, Not(AnyOfArray({2, 3})));
}

TEST(AnyOfArrayTest, Matchers) {
  // We negate test AllOfArrayTest.Matchers.
  // vector
  std::vector<Matcher<int>> matchers{Lt(1), Ge(2)};
  EXPECT_THAT(0, AnyOfArray(matchers));
  EXPECT_THAT(1, Not(AnyOfArray(matchers)));
  EXPECT_THAT(2, AnyOfArray(matchers));
  // initializer_list
  EXPECT_THAT(0, AnyOfArray({Lt(0), Lt(1)}));
  EXPECT_THAT(1, Not(AllOfArray({Lt(0), Lt(1)})));
}

TEST_P(AnyOfArrayTestP, ExplainsMatchResultCorrectly) {
  // AnyOfArray and AllOfArray use the same underlying template-template,
  // thus it is sufficient to test one here.
  const std::vector<int> v0{};
  const std::vector<int> v1{1};
  const std::vector<int> v2{2, 3};
  const Matcher<int> m0 = AnyOfArray(v0);
  const Matcher<int> m1 = AnyOfArray(v1);
  const Matcher<int> m2 = AnyOfArray(v2);
  EXPECT_EQ("", Explain(m0, 0));
  EXPECT_EQ("", Explain(m1, 1));
  EXPECT_EQ("", Explain(m1, 2));
  EXPECT_EQ("", Explain(m2, 3));
  EXPECT_EQ("", Explain(m2, 4));
  EXPECT_EQ("()", Describe(m0));
  EXPECT_EQ("(is equal to 1)", Describe(m1));
  EXPECT_EQ("(is equal to 2) or (is equal to 3)", Describe(m2));
  EXPECT_EQ("()", DescribeNegation(m0));
  EXPECT_EQ("(isn't equal to 1)", DescribeNegation(m1));
  EXPECT_EQ("(isn't equal to 2) and (isn't equal to 3)", DescribeNegation(m2));
  // Explain with matchers
  const Matcher<int> g1 = AnyOfArray({GreaterThan(1)});
  const Matcher<int> g2 = AnyOfArray({GreaterThan(1), GreaterThan(2)});
  // Explains the first positive match and all prior negative matches...
  EXPECT_EQ("which is 1 less than 1", Explain(g1, 0));
  EXPECT_EQ("which is the same as 1", Explain(g1, 1));
  EXPECT_EQ("which is 1 more than 1", Explain(g1, 2));
  EXPECT_EQ("which is 1 less than 1, and which is 2 less than 2",
            Explain(g2, 0));
  EXPECT_EQ("which is the same as 1, and which is 1 less than 2",
            Explain(g2, 1));
  EXPECT_EQ("which is 1 more than 1",  // Only the first
            Explain(g2, 2));
}

MATCHER(IsNotNull, "") { return arg != nullptr; }

// Verifies that a matcher defined using MATCHER() can work on
// move-only types.
TEST(MatcherMacroTest, WorksOnMoveOnlyType) {
  std::unique_ptr<int> p(new int(3));
  EXPECT_THAT(p, IsNotNull());
  EXPECT_THAT(std::unique_ptr<int>(), Not(IsNotNull()));
}

MATCHER_P(UniquePointee, pointee, "") { return *arg == pointee; }

// Verifies that a matcher defined using MATCHER_P*() can work on
// move-only types.
TEST(MatcherPMacroTest, WorksOnMoveOnlyType) {
  std::unique_ptr<int> p(new int(3));
  EXPECT_THAT(p, UniquePointee(3));
  EXPECT_THAT(p, Not(UniquePointee(2)));
}

MATCHER(EnsureNoUnusedButMarkedUnusedWarning, "") { return (arg % 2) == 0; }

TEST(MockMethodMockFunctionTest, EnsureNoUnusedButMarkedUnusedWarning) {
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic error "-Wused-but-marked-unused"
#endif
  // https://github.com/google/googletest/issues/4055
  EXPECT_THAT(0, EnsureNoUnusedButMarkedUnusedWarning());
#ifdef __clang__
#pragma clang diagnostic pop
#endif
}

#if GTEST_HAS_EXCEPTIONS

// std::function<void()> is used below for compatibility with older copies of
// GCC. Normally, a raw lambda is all that is needed.

// Test that examples from documentation compile
TEST(ThrowsTest, Examples) {
  EXPECT_THAT(
      std::function<void()>([]() { throw std::runtime_error("message"); }),
      Throws<std::runtime_error>());

  EXPECT_THAT(
      std::function<void()>([]() { throw std::runtime_error("message"); }),
      ThrowsMessage<std::runtime_error>(HasSubstr("message")));
}

TEST(ThrowsTest, PrintsExceptionWhat) {
  EXPECT_THAT(
      std::function<void()>([]() { throw std::runtime_error("ABC123XYZ"); }),
      ThrowsMessage<std::runtime_error>(HasSubstr("ABC123XYZ")));
}

TEST(ThrowsTest, DoesNotGenerateDuplicateCatchClauseWarning) {
  EXPECT_THAT(std::function<void()>([]() { throw std::exception(); }),
              Throws<std::exception>());
}

TEST(ThrowsTest, CallableExecutedExactlyOnce) {
  size_t a = 0;

  EXPECT_THAT(std::function<void()>([&a]() {
                a++;
                throw 10;
              }),
              Throws<int>());
  EXPECT_EQ(a, 1u);

  EXPECT_THAT(std::function<void()>([&a]() {
                a++;
                throw std::runtime_error("message");
              }),
              Throws<std::runtime_error>());
  EXPECT_EQ(a, 2u);

  EXPECT_THAT(std::function<void()>([&a]() {
                a++;
                throw std::runtime_error("message");
              }),
              ThrowsMessage<std::runtime_error>(HasSubstr("message")));
  EXPECT_EQ(a, 3u);

  EXPECT_THAT(std::function<void()>([&a]() {
                a++;
                throw std::runtime_error("message");
              }),
              Throws<std::runtime_error>(
                  Property(&std::runtime_error::what, HasSubstr("message"))));
  EXPECT_EQ(a, 4u);
}

TEST(ThrowsTest, Describe) {
  Matcher<std::function<void()>> matcher = Throws<std::runtime_error>();
  std::stringstream ss;
  matcher.DescribeTo(&ss);
  auto explanation = ss.str();
  EXPECT_THAT(explanation, HasSubstr("std::runtime_error"));
}

TEST(ThrowsTest, Success) {
  Matcher<std::function<void()>> matcher = Throws<std::runtime_error>();
  StringMatchResultListener listener;
  EXPECT_TRUE(matcher.MatchAndExplain(
      []() { throw std::runtime_error("error message"); }, &listener));
  EXPECT_THAT(listener.str(), HasSubstr("std::runtime_error"));
}

TEST(ThrowsTest, FailWrongType) {
  Matcher<std::function<void()>> matcher = Throws<std::runtime_error>();
  StringMatchResultListener listener;
  EXPECT_FALSE(matcher.MatchAndExplain(
      []() { throw std::logic_error("error message"); }, &listener));
  EXPECT_THAT(listener.str(), HasSubstr("std::logic_error"));
  EXPECT_THAT(listener.str(), HasSubstr("\"error message\""));
}

TEST(ThrowsTest, FailWrongTypeNonStd) {
  Matcher<std::function<void()>> matcher = Throws<std::runtime_error>();
  StringMatchResultListener listener;
  EXPECT_FALSE(matcher.MatchAndExplain([]() { throw 10; }, &listener));
  EXPECT_THAT(listener.str(),
              HasSubstr("throws an exception of an unknown type"));
}

TEST(ThrowsTest, FailNoThrow) {
  Matcher<std::function<void()>> matcher = Throws<std::runtime_error>();
  StringMatchResultListener listener;
  EXPECT_FALSE(matcher.MatchAndExplain([]() { (void)0; }, &listener));
  EXPECT_THAT(listener.str(), HasSubstr("does not throw any exception"));
}

class ThrowsPredicateTest
    : public TestWithParam<Matcher<std::function<void()>>> {};

TEST_P(ThrowsPredicateTest, Describe) {
  Matcher<std::function<void()>> matcher = GetParam();
  std::stringstream ss;
  matcher.DescribeTo(&ss);
  auto explanation = ss.str();
  EXPECT_THAT(explanation, HasSubstr("std::runtime_error"));
  EXPECT_THAT(explanation, HasSubstr("error message"));
}

TEST_P(ThrowsPredicateTest, Success) {
  Matcher<std::function<void()>> matcher = GetParam();
  StringMatchResultListener listener;
  EXPECT_TRUE(matcher.MatchAndExplain(
      []() { throw std::runtime_error("error message"); }, &listener));
  EXPECT_THAT(listener.str(), HasSubstr("std::runtime_error"));
}

TEST_P(ThrowsPredicateTest, FailWrongType) {
  Matcher<std::function<void()>> matcher = GetParam();
  StringMatchResultListener listener;
  EXPECT_FALSE(matcher.MatchAndExplain(
      []() { throw std::logic_error("error message"); }, &listener));
  EXPECT_THAT(listener.str(), HasSubstr("std::logic_error"));
  EXPECT_THAT(listener.str(), HasSubstr("\"error message\""));
}

TEST_P(ThrowsPredicateTest, FailWrongTypeNonStd) {
  Matcher<std::function<void()>> matcher = GetParam();
  StringMatchResultListener listener;
  EXPECT_FALSE(matcher.MatchAndExplain([]() { throw 10; }, &listener));
  EXPECT_THAT(listener.str(),
              HasSubstr("throws an exception of an unknown type"));
}

TEST_P(ThrowsPredicateTest, FailNoThrow) {
  Matcher<std::function<void()>> matcher = GetParam();
  StringMatchResultListener listener;
  EXPECT_FALSE(matcher.MatchAndExplain([]() {}, &listener));
  EXPECT_THAT(listener.str(), HasSubstr("does not throw any exception"));
}

INSTANTIATE_TEST_SUITE_P(
    AllMessagePredicates, ThrowsPredicateTest,
    Values(Matcher<std::function<void()>>(
        ThrowsMessage<std::runtime_error>(HasSubstr("error message")))));

// Tests that Throws<E1>(Matcher<E2>{}) compiles even when E2 != const E1&.
TEST(ThrowsPredicateCompilesTest, ExceptionMatcherAcceptsBroadType) {
  {
    Matcher<std::function<void()>> matcher =
        ThrowsMessage<std::runtime_error>(HasSubstr("error message"));
    EXPECT_TRUE(
        matcher.Matches([]() { throw std::runtime_error("error message"); }));
    EXPECT_FALSE(
        matcher.Matches([]() { throw std::runtime_error("wrong message"); }));
  }

  {
    Matcher<uint64_t> inner = Eq(10);
    Matcher<std::function<void()>> matcher = Throws<uint32_t>(inner);
    EXPECT_TRUE(matcher.Matches([]() { throw (uint32_t)10; }));
    EXPECT_FALSE(matcher.Matches([]() { throw (uint32_t)11; }));
  }
}

// Tests that ThrowsMessage("message") is equivalent
// to ThrowsMessage(Eq<std::string>("message")).
TEST(ThrowsPredicateCompilesTest, MessageMatcherAcceptsNonMatcher) {
  Matcher<std::function<void()>> matcher =
      ThrowsMessage<std::runtime_error>("error message");
  EXPECT_TRUE(
      matcher.Matches([]() { throw std::runtime_error("error message"); }));
  EXPECT_FALSE(matcher.Matches(
      []() { throw std::runtime_error("wrong error message"); }));
}

#endif  // GTEST_HAS_EXCEPTIONS

}  // namespace
}  // namespace gmock_matchers_test
}  // namespace testing

#ifdef _MSC_VER
#pragma warning(pop)
#endif
