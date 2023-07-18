// (c)2023 Sony Interactive Entertainment Inc
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
//     * Neither the name of Sony Interactive Entertainment Inc. nor the
// names of its contributors may be used to endorse or promote products
// derived from this software without specific prior written permission.
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

// Support for Rotten Green Test detection within Google Test.

#include "gtest/internal/gtest-rgt.h"

#if GTEST_HAS_RGT

#include <algorithm>
#include <cassert>
#include <map>
#include <vector>

#include "gtest/gtest.h"
#include "gtest/internal/gtest-rgt-parser.h"
#include "src/gtest-internal-inl.h"

// When we can't statically allocate the array of items into a custom
// section, they get registered on startup and we keep pointers to the
// data in a vector. And for consistency, we build this vector at
// startup even when we do allocate into a custom section. Once we have
// all the items registered, we can start organizing them for efficiency.

using ::testing::internal::RgtAssertInfo;
using ::testing::internal::RgtStaticItem;

namespace {

// This wrapper avoids initialization ordering issues relative to the
// individual RgtStaticItems.
class RegisteredItems {
  static RgtAssertInfo *items_;
public:
  RegisteredItems() = default;
  RgtAssertInfo *getItems() {
    if (!items_)
      items_ = new RgtAssertInfo;
    return items_;
  }
  size_t size() { return getItems()->size(); }
  bool empty() { return getItems()->empty(); }
  auto begin() { return getItems()->begin(); }
  auto end() { return getItems()->end(); }
  void push_back(testing::internal::RgtStaticItem *item) {
    getItems()->push_back(item);
  }
};

RegisteredItems registered_items;
RgtAssertInfo *RegisteredItems::items_ = nullptr;

// Items for each file are in a vector sorted by line number.
// We have a map to find the vector for a given file, which needs
// a comparison function when the key is a string.
struct RgtFilenameCompare {
  bool operator()(const char *lhs, const char *rhs) const {
    return strcmp(lhs, rhs) < 0;
  }
};

using CollatedItems = std::map<const char *, RgtAssertInfo,
                               RgtFilenameCompare>;
CollatedItems collated_items;
} // end anonymous namespace

#if GTEST_RGT_RUNTIME_INIT_

GTEST_API_
void testing::internal::RgtRecord(testing::internal::RgtStaticItem *item) {
  registered_items.push_back(item);
}

#ifdef GTEST_OS_WINDOWS
// On Windows we have to allocate our own placeholder start/stop data items.
// The linker will sort these into the right order relative to real data.
// Because the section concatenation might be padded, we'll have to skip over
// any items with a null pointer.

#define START_SECTION_NAME GTEST_RGT_SECTION_NAME_WITH_SUFFIX_($a)
#define STOP_SECTION_NAME GTEST_RGT_SECTION_NAME_WITH_SUFFIX_($z)

// Define start and end data items to use for the "array" bounds.
#pragma section (START_SECTION_NAME,read,write)
__declspec(allocate(START_SECTION_NAME))
static RgtStaticItem *rgt_manual_init_start = nullptr;

#pragma section (STOP_SECTION_NAME,read,write)
__declspec(allocate(STOP_SECTION_NAME))
static RgtStaticItem *rgt_manual_init_stop = nullptr;

static void RgtInitManual() {
  RgtStaticItem **I = &rgt_manual_init_start;
  // Skip the first entry, don't use the last entry.
  for (++I; I < &rgt_manual_init_stop; ++I) {
    if (*I)
      RgtRecord(*I);
  }
}

// Define the global pointer so we can fake-use all the items.
GTEST_API_ void *testing::internal::rgt_fake_use = nullptr;

#else // GTEST_OS_WINDOWS

// In this case, .init_array has called RgtRecord for us.

static void RgtInitManual() {}

#endif // GTEST_OS_WINDOWS

#else // GTEST_RGT_RUNTIME_INIT_

// Non-Windows linkers provide __start_<section> and __stop_<section> symbols.
// These are unqualified global references.
#define START_NAME GTEST_CONCAT_TOKEN_(__start_, GTEST_RGT_SECTION_NAME_)
#define STOP_NAME GTEST_CONCAT_TOKEN_(__stop_, GTEST_RGT_SECTION_NAME_)

// extern "C" vars can't have qualified type names; we only care about
// the addresses, we'll do appropriate casts later.
extern "C" int START_NAME;
extern "C" int STOP_NAME;

// Build the registered_items vector by taking the address of each item
// in the custom section.

static void RgtInitManual() {
  RgtStaticItem *iter = (RgtStaticItem *)&START_NAME;
  RgtStaticItem *end = (RgtStaticItem *)&STOP_NAME;

  for (; iter != end; ++iter)
    registered_items.push_back(iter);
}

#endif // !GTEST_RGT_RUNTIME_INIT_

// We collate the static items by file, and sort them by line,
// to make the lookups a bit more efficient.
static void RgtCollateData() {
  for (RgtStaticItem *item : registered_items)
    collated_items[item->file].push_back(item);
  for (auto &M : collated_items)
    std::sort(M.second.begin(), M.second.end(),
              [](const RgtStaticItem *lhs, const RgtStaticItem *rhs) {
                return lhs->line < rhs->line;
              });
}

size_t testing::internal::RgtInit() {
  // Collect all the RgtStaticItem addresses into registered_items.
  RgtInitManual();

  // Collate items into per-file vectors, and sort the vectors by line.
  RgtCollateData();

  return registered_items.size();
}

// Read the source file and parse it to find things we care about.
// Cache parsed files for runtime efficiency.
static testing::internal::RgtParser &getParserFor(const char *filename) {
  static std::map<const char *, testing::internal::RgtParser, RgtFilenameCompare>
    parser_cache_;
  auto iter = parser_cache_.find(filename);
  if (iter != parser_cache_.end())
    return iter->second;
  testing::internal::RgtParser &p = parser_cache_[filename];
  p.Parse(filename);
  return p;
}

// Report rotten assertions within this Test using GTEST_MESSAGE_AT_.
void testing::internal::RgtReportRotten(testing::TestInfo &test_info) {
  // Find the line-range for this Test by calling the parser.
  RgtParser &parser = getParserFor(test_info.file());
  if (!parser.Parsed())
    return;
  const RgtTestInfo *test_range = parser.FindTest(test_info.line());
  if (!test_range)
    return;
  int start_line = test_range->lines.start;
  int end_line = test_range->lines.end;

  // Get the assertion info for this file, if any.
  auto fileinfo_it = collated_items.find(test_info.file());
  if (fileinfo_it == collated_items.end())
    return;
  RgtAssertInfo &fileinfo = fileinfo_it->second;

  // Get the subset of assertions associated with this line range.
  unsigned start_index = 0;
  for (; start_index < fileinfo.size(); ++start_index) {
    if (fileinfo[start_index]->line >= start_line)
      break;
  }
  if (start_index == fileinfo.size() ||
      fileinfo[start_index]->line > end_line)
    return;
  unsigned end_index = start_index;
  for (; end_index < fileinfo.size(); ++end_index) {
    if (fileinfo[end_index]->line > end_line)
      break;
  }

  // Check each assertion in the range; report any that were not executed.
  // There might be multiple entries for any given assertion; also, for
  // EXPECT_[NON]FATAL_FAILURE, it often spans multiple lines and we want
  // to fold together all entries for the whole thing.
  // The assertion counts as executed if any entry is marked executed.
  internal::UnitTestImpl* const impl = internal::GetUnitTestImpl();
  assert(impl->current_test_info() == nullptr);
  impl->set_current_test_info(&test_info);
  while (start_index < end_index) {
    int assert_end_line = parser.GetAssertEndLine(fileinfo[start_index]->line);
    unsigned next_index = start_index + 1;
    while (next_index < fileinfo.size() &&
           fileinfo[next_index]->line <= assert_end_line)
      ++next_index;
    bool executed = false;
    for (unsigned i = start_index; i < next_index; ++i)
      executed = executed || fileinfo[i]->executed;
    if (!executed)
      GTEST_MESSAGE_AT_(test_info.file(), assert_end_line, "",
                        testing::TestPartResult::kRotten);
    start_index = next_index;
  }
  impl->set_current_test_info(nullptr);
}

#endif // GTEST_HAS_RGT
