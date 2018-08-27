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

//
// Internal helper functions for finding optimal edit transformations
// between strings.

#include "gtest/gtest.h"

#include <functional>
#include <list>
#include <ostream>  // NOLINT
#include <queue>
#include <vector>

namespace testing {
namespace internal {
namespace {

// The following implement data structures and code for a Dijkstra-search
// based implementation of optimal edit distance.

// Posible states a node can be in. Either a node is unsettled (it hasn't been
// drawn from the priority queue yet), or it is settled and a back-link to its
// parent node is fixed.
enum EditSearchState {
  kUnsettled,
  kMatchParent,
  kAddParent,
  kRemoveParent,
  kReplaceParent
};

// Custom container for search states. This is smaller and faster than a hash
// map, because the used states are dense along diagonals.
// Specifically, each state requires only 1 byte, whereas a hash_map would
// require storing the key, which would come to at least 8 bytes. std::map has
// an extra 32 bytes per node (3 pointers + 1 byte, padded), so even though
// there are circumstances where this class can have kBlockSize overhead per
// state, on average it does better than 40 bytes of overhead per state.
// In addition, in unopt builds (the usual way tests are run) the fewer
// allocations + better locality has this method running 10-50x faster than
// std::map for inputs that are large enough to measure.
class EditSearchMap {
 public:
  EditSearchMap(size_t left_size, size_t right_size)
      : left_size_(left_size), right_size_(right_size) {
    GTEST_CHECK_(left_size_ == left_size && right_size_ == right_size)
        << "Overflow in size: Arguments too large";
  }

  // Gets a mutable reference to a state - this is actually of type
  // EditSearchState - inserting if it does not exist.
  unsigned char& insert(UInt32 left, UInt32 right) {
    std::vector<UInt32>* vec;
    size_t index1;
    size_t index2;
    if (left > right) {
      vec = &left_nodes_;
      index1 = left - right - 1;
      index2 = right;
    } else {
      vec = &right_nodes_;
      index1 = right - left;
      index2 = left;
    }
    if (vec->size() <= index1) {
      GTEST_CHECK_(vec->size() == index1)
          << "Array diagonals should only grow by one " << vec->size() << " vs "
          << index1;
      vec->push_back(block_indices_.size());
      // Round up
      block_indices_.resize(
          block_indices_.size() +
              (DiagonalLength(left, right) + kBlockSize - 1) / kBlockSize,
          kUnallocatedBlock);
    }
    const size_t bucket = index2 / kBlockSize;
    const size_t pos_in_bucket = index2 % kBlockSize;
    UInt32& level2 = block_indices_[(*vec)[index1] + bucket];
    if (level2 == kUnallocatedBlock) {
      level2 = nodes_.size();
      size_t diagonal_length = DiagonalLength(left, right);
      GTEST_CHECK_(diagonal_length > index2)
          << diagonal_length << " " << index2;
      size_t block_size = kBlockSize;
      if (diagonal_length / kBlockSize == bucket) {
        // We can never get here if diagonal_length is a multiple of
        // kBlockSize, which is what we want, since this would evaluate to 0.
        block_size = diagonal_length % kBlockSize;
      }
      nodes_.resize(nodes_.size() + block_size);
    }
    return nodes_[level2 + pos_in_bucket];
  }

  size_t MemoryUsage() const {
    return nodes_.capacity() +
           sizeof(UInt32) * (left_nodes_.capacity() + right_nodes_.capacity() +
                             block_indices_.capacity());
  }

 private:
  enum { kBlockSize = 1024, kUnallocatedBlock = 0xFFFFFFFFul };

  size_t DiagonalLength(UInt32 left, UInt32 right) const {
    return std::min(left_size_ - left, right_size_ - right) +
           (left < right ? left : right);
  }

  // The state space is conceptually a left_size_ by right_size_ sparse matrix
  // of EditSearchStates. However, due to the access pattern of the search, it
  // is much better to store the nodes per diagonal rather than per row.
  UInt32 left_size_;
  UInt32 right_size_;
  // The nodes are stored by diagonals, split in two: Those to the left of the
  // main diagonal are in left_nodes_, and everything else is in right_nodes_.
  // The values are indices into block_indices_.
  std::vector<UInt32> left_nodes_;
  std::vector<UInt32> right_nodes_;
  // Every entry here is an offset into the beginning of a kBlockSize-sized
  // block in nodes_. An entire diagonal is allocated together here; for a
  // diagonal of length <= kBlockSize, that's just a single entry, but for
  // longer diagonals multiple contiguous index entries will be reserved at
  // once. Unused entries will be assigned kUnallocatedBlock; this
  // double-indirect scheme is used to save memory in the cases when an entire
  // diagonal isn't needed.
  std::vector<UInt32> block_indices_;
  // This stores the actual EditSearchState data, pointed to by block_indices_.
  std::vector<unsigned char> nodes_;
};

struct EditHeapEntry {
  EditHeapEntry(UInt32 l, UInt32 r, UInt64 c, EditSearchState s)
      : left(l), right(r), cost(c), state(s) {}

  UInt32 left;
  UInt32 right;
  UInt64 cost : 61;
  // The state that the node will get when this entry is settled. Therefore,
  // this can never be kUnsettled.
  UInt64 state : 3;

  bool operator>(const EditHeapEntry& other) const { return cost > other.cost; }
};

// Need a min-queue, so invert the comparator.
typedef std::priority_queue<EditHeapEntry, std::vector<EditHeapEntry>,
                            std::greater<EditHeapEntry>>
    EditHeap;

}  // namespace

std::vector<EditType> CalculateOptimalEdits(const std::vector<size_t>& left,
                                            const std::vector<size_t>& right,
                                            size_t* memory_usage) {
  const UInt64 kBaseCost = 100000;
  // We make replace a little more expensive than add/remove to lower
  // their priority.
  const UInt64 kReplaceCost = 100001;
  // In the common case where the vectors are the same (or almost the same)
  // size, we know that an add will have to be followed by some later remove
  // (or vice versa) in order to get the lengths to balance. We "borrow" some
  // of the cost of the later operation and bring it forward into the earlier
  // operation, to increase the cost of exploring (usually fruitlessly) around
  // the beginning of the graph.
  // However, there is a trade-off: This cheapens the cost of exploring around
  // the beginning of the graph (in one direction) when the vectors are
  // unequal in length. So we don't steal *all* the cost.
  // You can view this as a form of A*, using an admissable heuristic that has
  // been re-cast as a cost function that can be used in Dijkstra.
  const UInt64 kTowardsGoalCost = 50003;
  const UInt64 kAwayFromGoalCost = 2 * kBaseCost - kTowardsGoalCost;

  EditSearchMap node_map(left.size() + 1, right.size() + 1);
  EditHeap heap;
  heap.push(EditHeapEntry(0, 0, 0, kReplaceParent));

  while (!heap.empty()) {
    const EditHeapEntry current_entry = heap.top();
    heap.pop();

    UInt32 left_pos = current_entry.left;
    UInt32 right_pos = current_entry.right;
    unsigned char& current_state = node_map.insert(left_pos, right_pos);
    if (current_state != kUnsettled) {
      // Node was already settled by a previous entry in the priority queue,
      // this is a suboptimal path that should be ignored.
      continue;
    }
    current_state = current_entry.state;

    if (left_pos == left.size() && right_pos == right.size()) {
      // This is the normal exit point; if we terminate due to the heap being
      // empty, we'll fail a check later.
      break;
    }

    // Special case: Since the cost of a match is zero, we can immediately
    // settle the new node without putting it in the queue, since nothing can
    // have a smaller cost than it. Furthermore, we don't need to relax the
    // other two edges, since we know we don't need them: Any path from this
    // node that would use them has an path via the match that is at least as
    // cheap. Together, this means we can loop here until we stop matching.
    while (left_pos < left.size() && right_pos < right.size() &&
           left[left_pos] == right[right_pos]) {
      left_pos++;
      right_pos++;
      unsigned char& fast_forward_state = node_map.insert(left_pos, right_pos);
      if (fast_forward_state != kUnsettled) {
        // The search reached around and settled this node before settling the
        // base node. This means we're completely done with this iteration;
        // abort to the outer loop.
        goto outer_loop_bottom;
        // Otherwise, when can settle this node, even if it was created from
        // another state - we know the cost of settling it now is optimal.
      }
      fast_forward_state = kMatchParent;
    }

    // Relax adjacent nodes. We have no way to find or lower the cost of
    // existing entries in the heap, so we just push new entries and throw
    // them out at the top if the node is already settled. We *could* check to
    // see if they're already settled before pushing, but it turns out to be
    // ~not any faster, and more complicated to do so.
    //
    // If we're at an edge, there's only one node to relax.
    if (left_pos >= left.size()) {
      if (right_pos >= right.size()) {
        break;  // Can happen due to the fast-path loop above.
      }
      heap.push(EditHeapEntry(left_pos, right_pos + 1,
                              current_entry.cost + kTowardsGoalCost,
                              kAddParent));
      continue;
    }
    if (right_pos >= right.size()) {
      heap.push(EditHeapEntry(left_pos + 1, right_pos,
                              current_entry.cost + kTowardsGoalCost,
                              kRemoveParent));
      continue;
    }
    // General case: Relax 3 edges.
    heap.push(EditHeapEntry(
        left_pos, right_pos + 1,
        current_entry.cost + (right.size() + left_pos > right_pos + left.size()
                                  ? kTowardsGoalCost
                                  : kAwayFromGoalCost),
        kAddParent));
    heap.push(EditHeapEntry(
        left_pos + 1, right_pos,
        current_entry.cost + (right.size() + left_pos < right_pos + left.size()
                                  ? kTowardsGoalCost
                                  : kAwayFromGoalCost),
        kRemoveParent));
    heap.push(EditHeapEntry(left_pos + 1, right_pos + 1,
                            current_entry.cost + kReplaceCost, kReplaceParent));
  outer_loop_bottom : {}  // Need the curlies to form a statement.
  }

  // Reconstruct the best path. We do it in reverse order.
  std::vector<EditType> best_path;
  UInt32 left_pos = left.size();
  UInt32 right_pos = right.size();
  while (left_pos != 0 || right_pos != 0) {
    GTEST_CHECK_(left_pos <= left.size() && right_pos <= right.size());
    // The node must already exist, but if it somehow doesn't, it will be
    // added as kUnsettled, which will crash below.
    const unsigned char state = node_map.insert(left_pos, right_pos);
    switch (state) {
      case kAddParent:
        right_pos--;
        break;
      case kRemoveParent:
        left_pos--;
        break;
      case kMatchParent:
      case kReplaceParent:
        left_pos--;
        right_pos--;
        break;
      default:
        GTEST_LOG_(FATAL) << "Unsettled node at " << left_pos << ","
                          << right_pos;
    }
    best_path.push_back(static_cast<EditType>(state - 1));
  }
  std::reverse(best_path.begin(), best_path.end());
  if (memory_usage != NULL) {
    *memory_usage = node_map.MemoryUsage();
  }
  return best_path;
}

namespace {

// Helper class to convert string into ids with deduplication.
class InternalStrings {
 public:
  size_t GetId(const std::string* str) {
    IdMap::iterator it = ids_.find(str);
    if (it != ids_.end()) return it->second;
    size_t id = ids_.size();
    return ids_[str] = id;
  }

 private:
  struct IdMapCmp {
    bool operator()(const std::string* first, const std::string* second) const {
      return *first < *second;
    }
  };
  typedef std::map<const std::string*, size_t, IdMapCmp> IdMap;
  IdMap ids_;
};

}  // namespace

std::vector<EditType> CalculateOptimalEdits(
    const std::vector<std::string>& left,
    const std::vector<std::string>& right) {
  std::vector<size_t> left_ids, right_ids;
  {
    InternalStrings intern_table;
    for (size_t i = 0; i < left.size(); ++i) {
      left_ids.push_back(intern_table.GetId(&left[i]));
    }
    for (size_t i = 0; i < right.size(); ++i) {
      right_ids.push_back(intern_table.GetId(&right[i]));
    }
  }
  return CalculateOptimalEdits(left_ids, right_ids);
}

namespace {

// Helper class that holds the state for one hunk and prints it out to the
// stream.
// It reorders adds/removes when possible to group all removes before all
// adds. It also adds the hunk header before printing into the stream.
class Hunk {
 public:
  Hunk(size_t left_start, size_t right_start)
      : left_start_(left_start),
        right_start_(right_start),
        adds_(),
        removes_(),
        common_() {}

  void PushLine(char edit, const char* line) {
    switch (edit) {
      case ' ':
        ++common_;
        FlushEdits();
        hunk_.push_back(std::make_pair(' ', line));
        break;
      case '-':
        ++removes_;
        hunk_removes_.push_back(std::make_pair('-', line));
        break;
      case '+':
        ++adds_;
        hunk_adds_.push_back(std::make_pair('+', line));
        break;
    }
  }

  void PrintTo(std::ostream* os) {
    PrintHeader(os);
    FlushEdits();
    for (std::list<std::pair<char, const char*> >::const_iterator it =
             hunk_.begin();
         it != hunk_.end(); ++it) {
      *os << it->first << it->second << "\n";
    }
  }

  bool has_edits() const { return adds_ || removes_; }

 private:
  void FlushEdits() {
    hunk_.splice(hunk_.end(), hunk_removes_);
    hunk_.splice(hunk_.end(), hunk_adds_);
  }

  // Print a unified diff header for one hunk.
  // The format is
  //   "@@ -<left_start>,<left_length> +<right_start>,<right_length> @@"
  // where the left/right parts are omitted if unnecessary.
  void PrintHeader(std::ostream* ss) const {
    *ss << "@@ ";
    if (removes_) {
      *ss << "-" << left_start_ << "," << (removes_ + common_);
    }
    if (removes_ && adds_) {
      *ss << " ";
    }
    if (adds_) {
      *ss << "+" << right_start_ << "," << (adds_ + common_);
    }
    *ss << " @@\n";
  }

  size_t left_start_, right_start_;
  size_t adds_, removes_, common_;
  std::list<std::pair<char, const char*> > hunk_, hunk_adds_, hunk_removes_;
};

}  // namespace

// Create a list of diff hunks in Unified diff format.
// Each hunk has a header generated by PrintHeader above plus a body with
// lines prefixed with ' ' for no change, '-' for deletion and '+' for
// addition.
// 'context' represents the desired unchanged prefix/suffix around the diff.
// If two hunks are close enough that their contexts overlap, then they are
// joined into one hunk.
std::string CreateUnifiedDiff(const std::vector<std::string>& left,
                              const std::vector<std::string>& right,
                              size_t context) {
  const std::vector<EditType> edits = CalculateOptimalEdits(left, right);

  size_t l_i = 0, r_i = 0, edit_i = 0;
  std::stringstream ss;
  while (edit_i < edits.size()) {
    // Find first edit.
    while (edit_i < edits.size() && edits[edit_i] == kEditMatch) {
      ++l_i;
      ++r_i;
      ++edit_i;
    }

    // Find the first line to include in the hunk.
    const size_t prefix_context = std::min(l_i, context);
    Hunk hunk(l_i - prefix_context + 1, r_i - prefix_context + 1);
    for (size_t i = prefix_context; i > 0; --i) {
      hunk.PushLine(' ', left[l_i - i].c_str());
    }

    // Iterate the edits until we found enough suffix for the hunk or the input
    // is over.
    size_t n_suffix = 0;
    for (; edit_i < edits.size(); ++edit_i) {
      if (n_suffix >= context) {
        // Continue only if the next hunk is very close.
        std::vector<EditType>::const_iterator it = edits.begin() + edit_i;
        while (it != edits.end() && *it == kEditMatch) ++it;
        if (it == edits.end() || (it - edits.begin()) - edit_i >= context) {
          // There is no next edit or it is too far away.
          break;
        }
      }

      EditType edit = edits[edit_i];
      // Reset count when a non match is found.
      n_suffix = edit == kEditMatch ? n_suffix + 1 : 0;

      if (edit == kEditMatch || edit == kEditRemove || edit == kEditReplace) {
        hunk.PushLine(edit == kEditMatch ? ' ' : '-', left[l_i].c_str());
      }
      if (edit == kEditAdd || edit == kEditReplace) {
        hunk.PushLine('+', right[r_i].c_str());
      }

      // Advance indices, depending on edit type.
      l_i += edit != kEditAdd;
      r_i += edit != kEditRemove;
    }

    if (!hunk.has_edits()) {
      // We are done. We don't want this hunk.
      break;
    }

    hunk.PrintTo(&ss);
  }
  return ss.str();
}

}  // namespace internal
}  // namespace testing
