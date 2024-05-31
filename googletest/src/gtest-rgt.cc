// Copyright (C) 2024 Sony Interactive Entertainment Inc.
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
#include "src/gtest-internal-inl.h"

namespace {

// Define the vector where we will keep RgtStaticItem-s that describe
// assertions from outside any Test (e.g., in a helper function).
testing::internal::RgtAssertInfo rgt_helper_asserts;

} // end anonymous namespace

// Report rotten assertions using GTEST_MESSAGE_AT_.
size_t testing::internal::RgtReportRotten(
    const testing::internal::RgtAssertInfo *assert_info) {
  if (!assert_info)
    assert_info = &rgt_helper_asserts;

  // Collect raw data into a map with filename as the key; the value is
  // another map with line number as the key; the value of the second key
  // is the logical-or of all "executed" flags with the same source
  // location. This de-duplicates things like template instantiations.
  struct RgtFilenameCompare {
    bool operator()(const char *lhs, const char *rhs) const {
      return strcmp(lhs, rhs) < 0;
    }
  };
  using RgtFileInfo = std::map<int, bool>;
  using RgtTestInfo = std::map<const char*, RgtFileInfo, RgtFilenameCompare>;
  RgtTestInfo executed_map;
  for (testing::internal::RgtStaticItem *item : *assert_info) {
    RgtFileInfo &fileinfo = executed_map[item->file];
    bool &executed = fileinfo[item->line];
    executed = executed || item->executed;
  }

  // For any assertion that wasn't executed, report it as kRotten.
  for (auto &M : executed_map) {
    // There are two helper methods within googletest itself;
    // don't report those, they are often unused.
    if (String::EndsWithCaseInsensitive(M.first, "gtest.cc") ||
        String::EndsWithCaseInsensitive(M.first, "gtest-port.cc"))
      continue;
    for (auto &E : M.second) {
      if (!E.second)
        GTEST_MESSAGE_AT_(M.first, E.first, "",
                          testing::TestPartResult::kRotten);
    }
  }
  return assert_info->size();
}

// When we can't statically allocate the array of items in a custom section,
// they get registered on startup and we keep pointers to the data in this
// vector. And for consistency, we build this vector at startup even when we
// do allocate into a custom section.

namespace {

using ItemVector = std::vector<testing::internal::RgtStaticItem *>;
class RegisteredItems {
  static ItemVector *items_;
public:
  RegisteredItems() = default;
  ItemVector *getItems() {
    if (!items_)
      items_ = new ItemVector;
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
ItemVector *RegisteredItems::items_ = nullptr;

} // end anonymous namespace

#if GTEST_RGT_STARTUP_INIT_

GTEST_API_
void testing::internal::RgtRecord(testing::internal::RgtStaticItem *item) {
  registered_items.push_back(item);
}

#ifdef _WIN32

// On Windows we have to allocate our own placeholder start/stop data items.
// The linker will sort these into the right order relative to real data.
#define START_SECTION_NAME GTEST_RGT_SECTION_NAME_WITH_SUFFIX_($a)
#define STOP_SECTION_NAME GTEST_RGT_SECTION_NAME_WITH_SUFFIX_($z)

using ::testing::internal::RgtRecorder;

#pragma section (START_SECTION_NAME,read,write)
__declspec(allocate(START_SECTION_NAME))
static RgtRecorder rgt_manual_init_start = nullptr;

#pragma section (STOP_SECTION_NAME,read,write)
__declspec(allocate(STOP_SECTION_NAME))
static RgtRecorder rgt_manual_init_stop = nullptr;

static void RgtInitManual() {
  const RgtRecorder *F = &rgt_manual_init_start;
  // Because the concatenated sections might be padded, we have to skip over
  // any null pointers. Also skip the (known null) start and end markers.
  for (++F; F < &rgt_manual_init_stop; ++F) {
    if (*F)
      (*F)();
  }
}

// Define the fake-use global that we use because MSVC has no "used" attribute.
GTEST_API_ void *::testing::internal::rgt_fake_use = nullptr;

#else // _WIN32

// In this case, .init_array has called RgtRecord for us.

static void RgtInitManual() {}

#endif // _WIN32

#else // GTEST_RGT_STARTUP_INIT_

// Non-Windows linkers provide __start_<section> and __stop_<section> symbols.
// These are unqualified global references.
#define START_NAME GTEST_CONCAT_TOKEN_(__start_, GTEST_RGT_SECTION_NAME_)
#define STOP_NAME GTEST_CONCAT_TOKEN_(__stop_, GTEST_RGT_SECTION_NAME_)

// extern "C" vars can't have qualified type names; we only care about their
// addresses, so we'll do appropriate casts later.
extern "C" int START_NAME;
extern "C" int STOP_NAME;

// Build the registered_items vector by taking the address of each item in the
// custom section.

static void RgtInitManual() {
  using testing::internal::RgtStaticItem;
  RgtStaticItem *I = (RgtStaticItem *)&START_NAME;
  RgtStaticItem *E = (RgtStaticItem *)&STOP_NAME;

  unsigned count = 0;
  for (; I != E; ++I) {
    registered_items.push_back(I);
    ++count;
  }
}

#endif // GTEST_RGT_STARTUP_INIT_

size_t testing::internal::RgtInit() {
  // Collect all the RgtStaticItem addresses into registered_items.
  RgtInitManual();

  // For each RgtStaticItem, if it has an associated TestInfo, attach it there;
  // if it doesn't, keep it on the side so we can check them at the end.
  for (RgtStaticItem *item : registered_items) {
    if (testing::TestInfo *TI = *item->test_info)
      TI->asserts_.push_back(item);
    else
      rgt_helper_asserts.push_back(item);
  }
  return registered_items.size();
}

// Remember if a test uses an EXPECT_[NON]FATAL_FAILURE macro.
GTEST_API_
void testing::internal::RgtUsesExpectFailure(::testing::TestInfo* test_info) {
  if (test_info)
    test_info->set_uses_expect_failure();
}

#if GTEST_DEBUG_RGT
// Dump source location of all identified assertions. For debugging.
// We sort and emit one per line, without deduplicating, for better diffing.

void testing::internal::RgtDumpAllAssertionsTo(std::string &filename) {
  if (filename.empty())
    return;

  // Following is based on UnitTestOptions::GetAbsolutePathToOutputFile()
  // which has a note regarding certain Windows paths not working.
  internal::FilePath log_path(filename);
  if (!log_path.IsAbsolutePath()) {
    log_path = internal::FilePath::ConcatPaths(
        internal::FilePath(UnitTest::GetInstance()->original_working_dir()),
        internal::FilePath(filename));
  }

  if (log_path.IsDirectory()) {
    fprintf(stderr, "Specified log file is a directory \"%s\"\n",
            filename.c_str());
    return;
  }
  FILE* logfile = posix::FOpen(log_path.c_str(), "w");
  if (!logfile) {
    fprintf(stderr, "Unable to open log file \"%s\"\n", log_path.c_str());
    return;
  }

  // registered_items is a vector of RgtStaticItem; sort it by filename and
  // then by line.
  struct RgtItemCompare {
    bool operator()(const RgtStaticItem *lhs, const RgtStaticItem *rhs) const {
      int Cmp = strcmp(lhs->file, rhs->file);
      return Cmp == 0 ? lhs->line < rhs->line : Cmp < 0;
    }
  };
  std::sort(registered_items.getItems()->begin(),
            registered_items.getItems()->end(), RgtItemCompare());
  for (auto *item : *registered_items.getItems())
    fprintf(logfile, "%s::%d\n", item->file, item->line);
  posix::FClose(logfile);
}

#endif // GTEST_DEBUG_RGT

#endif // GTEST_HAS_RGT
