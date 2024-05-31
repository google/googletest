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

// This file defines macros and classes for detecting Rotten Green Tests
// in the LLVM unittests.

#ifndef GTEST_INCLUDE_GTEST_INTERNAL_GTEST_RGT_H_
#define GTEST_INCLUDE_GTEST_INTERNAL_GTEST_RGT_H_

#include "gtest/internal/gtest-port.h"

#if GTEST_HAS_RGT

// A Rotten Green Test is a test assertion that _looks_ like it is verifying
// something useful about code behavior, but in fact the assertion is never
// executed. Because the test didn't explicitly fail, it's assumed to have
// passed. This file supports instrumenting Google Tests to detect EXPECT_*
// and ASSERT_* calls that are not executed, indicating that they are
// Rotten Green Tests.
//
// Inspired by "Rotten Green Tests", Delplanque et al., ICSE 2019
// DOI 10.1109/ICSE.2019.00062

#include <map>
#include <string>
#include <utility>

// Overview:
//
// Rotten Green Test checking involves four phases. First, statically identify
// all test assertions. Second, record which test assertions are executed as
// the test runs. Third, any necessary data cleanup (for example: different
// template instantiations might not all use the same test assertions, so we
// merge results for multiple instances of the same assertion). Fourth, report
// un-executed (rotten) assertions.
//
// In order to avoid false positives, we do not report any test that was
// filtered out, skipped, disabled, or otherwise not run. We also don't report
// any failed tests, because failures (especially with ASSERT* macros) might
// well skip other assertions, and so reporting those as rotten isn't really
// useful.  (This is Rotten *Green* Test detection, after all.)

// Implementation:
//
// We instrument the assertion macros to statically record the source location
// of each assertion. We then attach this information to the associated
// TestInfo (if possible). During Test execution, each assertion records that
// it was executed. Finally, for each Test that was executed, we look for and
// report un-executed assertions.
//
// The implementation depends on having local static data allocated and
// constant-initialized at compile/link time, in order to identify each test
// assertion. Because it's _local_ static data, we can't rely on ordered
// initialization; that works only for non-local data. We also can't rely on
// dynamic initialization, because dynamic initialization requires that
// control pass over the definition, and the whole point is to detect when
// that *doesn't* happen. Getting this to work depends on a variety of
// environment-dependent tricks, described later.

// FIXME: During development of this feature, sometimes there were cases
// where the RGT mechanism failed to compile, or otherwise didn't work.
// You can disable checking for a problematic test by doing
//     #undef  GTEST_RGT_DECLARE
//     #define GTEST_RGT_DECLARE
// before the test, and re-enabling it with
//     #undef  GTEST_RGT_DECLARE
//     #define GTEST_RGT_DECLARE GTEST_RGT_DECLARE_
// afterward.
#define GTEST_RGT_DECLARE GTEST_RGT_DECLARE_

namespace testing {

class TestInfo;

namespace internal {

// The data to record per assertion site.  All members of RgtStaticItem must
// use static initialization, which is why we don't record TestInfo* directly;
// those are created dynamically.
struct RgtStaticItem {
  testing::TestInfo *const *test_info;
  const char *file;
  int line;
  bool executed;
  // constexpr to guarantee compile-time initialization.
  constexpr RgtStaticItem(testing::TestInfo *const *a_test_info,
                          const char *a_file, int a_line) :
      test_info(a_test_info), file(a_file), line(a_line), executed(false) {}
};

// Each RgtStaticItem that describes an assertion within a Test function is
// recorded in a vector attached to the appropriate TestInfo instance. Other
// assertions (e.g., from a helper function) are remembered on the side. We
// use vectors because we don't want to deduplicate items (yet).
using RgtAssertInfo = std::vector<RgtStaticItem *>;

// RgtReportRotten takes an RgtAssertInfo vector, deduplicates the info, and
// calls GTEST_MESSAGE_AT_ on each un-executed assertion. If the parameter is
// nullptr, it uses the helper method assertion vector. Returns the number of
// unique items in assert_info.
size_t RgtReportRotten(const RgtAssertInfo *assert_info);

// We need to be able to find all those static items, so they (or their
// addresses) need to be somewhere that has a name we can use at runtime to
// find them. Unfortunately, there doesn't seem to be one consistent way to
// do this across toolchains.
//
// For Clang (non-Windows), we allocate the static data into a custom section
// that has a name that is a legal C identifier. Then we can use symbols that
// the linker defines for us to find the start and end of the section. This
// allows us to allocate static data piecemeal in the source, and still have
// effectively a single array of all such data at runtime. This tactic works
// using GNU linkers or LLD.
//
// With gcc, due to how it handles data allocation for items in inline
// functions, we can't put local static data in a custom section. See:
// https://stackoverflow.com/questions/35091862/inline-static-data-causes-a-section-type-conflict
// and also https://gcc.gnu.org/bugzilla/show_bug.cgi?id=41091 for details.
// (It seems that GCC 14 fixes most cases, but still not inline functions.)
// In order to address this, we apply the solution described here:
// https://stackoverflow.com/questions/29903391/find-unexecuted-lines-of-c-code
// We use the .init_array hack to collect the addresses of all the data items,
// which are allocated without any special section or attributes on them. This
// lets us have each item be registered automatically, and because .init_array
// is a predefined section, gcc doesn't give us any trouble with section
// attributes.
//
// For MSVC, allocating arbitrary data to a custom section tends to have
// padding issues, and we need to be able to treat the custom section as an
// array. So, we do a gcc-like thing, allocating the data normally, and
// capturing a pointer to an initializer function in the custom section;
// capturing pointers doesn't seem to have the padding issues, or not as
// badly, as we do end up with some null pointers in there. We use an
// initializer function instead of just capturing a pointer to the data, in
// order to be more consistent with the gcc scheme. We use documented
// section-ordering rules to define our own symbols for the array start and
// end, and call all the initializer functions manually.

// Define a toggle to see which kind of initialization we're doing.
#if defined(__GNUC__) || defined(_WIN32)
#define GTEST_RGT_STARTUP_INIT_ 1
#else
#define GTEST_RGT_STARTUP_INIT_ 0
#endif // __GNUC__ || _WIN32

#if GTEST_RGT_STARTUP_INIT_
// Record the existence of a test point, when we can't arrange for that using
// a custom section.
GTEST_API_ void RgtRecord(RgtStaticItem *item);
#endif // GTEST_RGT_STARTUP_INIT_

// Finish off the RGT initialization; attach each RGT_item to its TestInfo
// instance, and remember the ones that aren't lexically in a Test. Called
// from OnTestProgramStart(). Returns the number of items it found.
size_t RgtInit();

#if GTEST_DEBUG_RGT
// For debugging: Dump the locations of all test assertions to the
// specified file.
void RgtDumpAllAssertionsTo(std::string &filename);
#endif // GTEST_DEBUG_RGT

} // end namespace internal
} // end namespace testing

// When a test assertion is lexically contained within a Test, an RgtStaticItem
// captures a pointer to the Test's static gtest_test_info_ member, allowing
// us to associate assertions with tests. However, assertions may also be in
// helper functions outside of a test; for those cases, we use a data item with
// the same name in the anonymous namespace, so that an unqualified use of the
// name will always be satisfied. Normally global variables in the anonymous
// namespace are not cool, but in this case it's exactly what we need.
namespace {
testing::TestInfo* const gtest_test_info_ = nullptr;
} // namespace

// Define the section name (on Windows, the prefix) to use for RGT data.
// Note, the non-Windows case requires this be a legal C identifier.
#define GTEST_RGT_SECTION_NAME_BASE_ GTEST_RGT

#if GTEST_RGT_STARTUP_INIT_

// Conjure up a function to do startup-time initialization, given that
// we can't arrange for static initialization.

#ifdef __GNUC__

#define GTEST_RGT_RECORD_ITEM_(ITEM)                                    \
  struct RgtRecorderHelper_##ITEM {                                     \
    static void Record() { ::testing::internal::RgtRecord(&ITEM); }     \
  };                                                                    \
  static auto RgtHelper2_##ITEM __attribute__((section(".init_array"))) \
    = RgtRecorderHelper_##ITEM::Record;                                 \
  (void)RgtHelper2_##ITEM;

#else // __GNUC__

// Windows doesn't automatically provide start/end symbol names for sections,
// so we roll our own start/end entries. Sections are sorted lexically, so we
// paste suffixes onto the base name to get correct sorting. Extra fun macro
// indirection required.

#define GTEST_RGT_SECTION_NAME_WITH_SUFFIX_(SUFFIX) \
  GTEST_RGT_SECTION_NAME_2(GTEST_RGT_SECTION_NAME_BASE_, SUFFIX)
#define GTEST_RGT_SECTION_NAME_2(NAME, SUFFIX) \
  GTEST_RGT_SECTION_NAME_3(NAME, SUFFIX)
#define GTEST_RGT_SECTION_NAME_3(NAME, SUFFIX) \
  GTEST_STRINGIFY_(NAME ## SUFFIX)

#define GTEST_RGT_SECTION_NAME_ GTEST_RGT_SECTION_NAME_WITH_SUFFIX_($d)

namespace testing {
namespace internal {

// The type of the functions we're going to track in the section that we're
// using instead of .init_array.
typedef void(*RgtRecorder)(void);

// Because MSVC has no equivalent of attribute(used) we need to fake up a
// pointer escaping so the function pointers will look used.
GTEST_API_ extern void *rgt_fake_use;

} // namespace internal
} // namespace testing

// MSVC requires the section to be declared before being used, but simply
// using a global #pragma seems not to work (despite the documentation).
// So, do a __pragma every time.

#define GTEST_RGT_RECORD_ITEM_(ITEM)                                \
  struct RgtRecorderHelper {                                        \
    static void record() { ::testing::internal::RgtRecord(&ITEM); } \
  };                                                                \
  __pragma(section(GTEST_RGT_SECTION_NAME_,read,write))             \
  __declspec(allocate(GTEST_RGT_SECTION_NAME_))                     \
  static ::testing::internal::RgtRecorder rgt_record_item =         \
      RgtRecorderHelper::record;                                    \
  ::testing::internal::rgt_fake_use = (void*)&rgt_record_item;

#endif // __GNUC__

// In the runtime-init case, the actual assertion tracking data doesn't go
// anywhere special.
#define GTEST_RGT_SECTION_ATTR

#else // GTEST_RGT_STARTUP_INIT_

// In the "normal" case, allocate local static data to a custom section which
// we can then iterate over when reporting.
#define GTEST_RGT_SECTION_NAME_ GTEST_RGT_SECTION_NAME_BASE_

// Define how to decorate the data declarations.
#define GTEST_RGT_SECTION_ATTR GTEST_RGT_SECTION_ATTR_2(GTEST_RGT_SECTION_NAME_)
#define GTEST_RGT_SECTION_ATTR_2(NAME) GTEST_RGT_SECTION_ATTR_3(NAME)
#define GTEST_RGT_SECTION_ATTR_3(NAME) __attribute__((section(#NAME),used))

// No special "record" action needed.
#define GTEST_RGT_RECORD_ITEM_(ITEM)

#endif // GTEST_RGT_STARTUP_INIT_

// Define the bookkeeping macro to use in the various assertion macros.
// Statically initialize 'executed' to false, dynamically set it true.

#define GTEST_RGT_DECLARE_                                         \
  GTEST_RGT_SECTION_ATTR static ::testing::internal::RgtStaticItem \
      gtest_rgt_item(&gtest_test_info_, __FILE__, __LINE__);       \
  gtest_rgt_item.executed = true;                                  \
  GTEST_RGT_RECORD_ITEM_(gtest_rgt_item)

// If the test uses an EXPECT_[NON]FATAL_FAILURE macro, the rotten-test
// tracking becomes unreliable, because those macros exercise assertions that
// are intended to fail, and therefore will appear rotten. Remember tests
// where these macros are used, to avoid false positives.
namespace testing {
namespace internal {
GTEST_API_ void RgtUsesExpectFailure(::testing::TestInfo* test_info);
} // namespace internal
} // namespace testing

#define GTEST_RGT_USES_EXPECT_FAILURE_ \
  ::testing::internal::RgtUsesExpectFailure(gtest_test_info_)

#else // GTEST_HAS_RGT

// With RGT disabled, don't instrument anything.

#define GTEST_RGT_DECLARE
#define GTEST_RGT_DECLARE_

#endif // GTEST_HAS_RGT

#endif  // GTEST_INCLUDE_GTEST_INTERNAL_GTEST_RGT_H_
