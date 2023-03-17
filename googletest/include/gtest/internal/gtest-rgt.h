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

// This file defines macros and classes for detecting Rotten Green Tests
// constructed using Google Test.

#ifndef GTEST_INCLUDE_GTEST_INTERNAL_GTEST_RGT_H_
#define GTEST_INCLUDE_GTEST_INTERNAL_GTEST_RGT_H_

#if GTEST_HAS_RGT

// A Rotten Green Test is a test assertion that _looks_ like it verifies
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

#include "gtest/internal/gtest-port.h"

// Overview:
//
// Rotten Green Test checking involves four phases. First, statically
// identify all test assertions. Second, record which test assertions
// are executed as the test runs. Third, do a quick cheap parse of the
// test source to identify source ranges for each Test. Fourth, detect
// any un-executed (rotten) assertions within the range of an executed
// Test, and report those.
//
// In order to avoid false positives, we do not report any test that was
// filtered out, skipped, disabled, or otherwise not run. We also don't
// report any failed tests, because failures (especially with ASSERT*
// macros) might well skip other assertions, and so reporting those as
// rotten isn't really useful. (This is Rotten *Green* Test detection,
// after all.)

// Implementation:
//
// Each assertion macro statically records the source location of the
// assertion, and also an "executed" flag. When the assertion executes
// successfully, it sets "executed" to true. After Test execution, a
// simple parse of the source identifies the source range for the Test,
// and any un-executed assertions in that range are reported.
//
// The implementation depends on having local static data allocated and
// constant-initialized at compile/link time, in order to identify each
// test assertion. Because it's _local_ static data, we can't rely on
// ordered initialization; that works only for non-local data. We also
// can't rely on dynamic initialization, because dynamic initialization
// requires that control pass over the definition, and the whole point
// is to detect when that *doesn't* happen. Getting this to work requires
// a variety of environment-dependent tricks, described below.

namespace testing {

class TestInfo;

namespace internal {

// The data to record per assertion site. All members of RgtStaticItem
// must use static initialization.
struct RgtStaticItem {
  const char *file;
  int line;
  bool executed;
  // constexpr to guarantee compile-time initialization.
  constexpr RgtStaticItem(const char *a_file, int a_line) :
      file(a_file), line(a_line), executed(false) {}
};

// Each RgtStaticItem that describes an assertion within a Test function
// has its address recorded in a vector. We also build other structures
// out of these to make access more efficient after startup.
using RgtAssertInfo = std::vector<RgtStaticItem *>;

// RgtReportRotten looks for the RgtStaticItem instances associated with
// the specified Test's source range, and reports un-executed items as
// rotten results.
void RgtReportRotten(TestInfo &test_info);

// We need to be able to find all those static items, so they (or their
// addresses) need to be somewhere that has a name we can use at runtime
// to find them. Unfortunately, there doesn't seem to be one consistent
// way to do this across toolchains.
//
// For Clang (non-Windows), we allocate the static data into a custom
// section that has a name that is a legal C identifier. Then we can
// use symbols that the linker defines for us to find the start and end
// of the section. This allows us to allocate static data piecemeal in
// the source, and still have effectively a single array of all such
// data at runtime. This tactic works using GNU linkers or LLD.
//
// With gcc, due to how it handles data allocation for items in inline
// functions, we can't put local static data in a custom section. See:
// https://stackoverflow.com/questions/35091862/inline-static-data-causes-a-section-type-conflict
// In order to address this, we apply the solution described here:
// https://stackoverflow.com/questions/29903391/find-unexecuted-lines-of-c-code
// We use the .init_array hack to collect the addresses of all the data
// items, which are allocated without any special section or attributes
// on them. This lets us have each item be registered automatically,
// and because .init_array is a predefined section, gcc doesn't give us
// any trouble with section attributes.
//
// For MSVC, allocating arbitrary data to a custom section seems to have
// padding issues, and we need to be able to treat the custom section as
// an array. So, we do a gcc-like thing, allocating the data normally,
// and capturing a pointer to the data in the custom section. Capturing
// pointers doesn't seem to have the padding issues--or not as badly, as
// we do end up with some null pointers in there. We use documented
// section-ordering rules to define our own symbols for the array start
// and end, and call all the initializer functions manually.

#if defined(__GNUC__) || defined(GTEST_OS_WINDOWS)
#define GTEST_RGT_RUNTIME_INIT_ 1
#else
#define GTEST_RGT_RUNTIME_INIT_ 0
#endif

#if GTEST_RGT_RUNTIME_INIT_

// Record the existence of a test point, when we can't arrange for that
// using a custom section.
GTEST_API_ void RgtRecord(RgtStaticItem *item);

#endif // GTEST_RGT_RUNTIME_INIT_

// Finish off the RGT initialization; each RgtStaticItem ends up in a
// per-file table, sorted by line number. Returns the number of items
// it found.
size_t RgtInit();

} // end namespace internal
} // end namespace testing

// Define the section name (on Windows, the prefix) to use for RGT data.
// Note, the non-Windows case requires this be a legal C identifier.
#define GTEST_RGT_SECTION_NAME_BASE_ GTEST_RGT

#if GTEST_RGT_RUNTIME_INIT_

// Conjure up a function to do startup-time initialization, given that
// we can't arrange for static initialization.

#ifdef __GNUC__

#define GTEST_RGT_RECORD_ITEM_(ITEM)                                \
  struct RgtRecorderHelper_##ITEM {                                 \
    static void Record() { ::testing::internal::RgtRecord(&ITEM); } \
  };                                                                \
  __asm__(                                                          \
  ".pushsection .init_array" "\n"                                   \
  ".quad %c0" "\n"                                                  \
  ".popsection" "\n"                                                \
  : : "i"(RgtRecorderHelper_##ITEM::Record));

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
  GTEST_RGT_SECTION_NAME_4(NAME ## SUFFIX)
#define GTEST_RGT_SECTION_NAME_4(NAME) #NAME

#define GTEST_RGT_SECTION_NAME_ GTEST_RGT_SECTION_NAME_WITH_SUFFIX_($d)

namespace testing {
namespace internal {

// Because MSVC has no equivalent of attribute(used) we need to escape
// the pointer so it will look used.
GTEST_API_ extern void *rgt_fake_use;

} // namespace internal
} // namespace testing

// MSVC requires the section to be declared before being used, but simply
// using a global #pragma seems not to work (despite the documentation).
// Hence the __pragma declaration here.
#define GTEST_RGT_RECORD_ITEM_(ITEM) \
  { \
    __pragma(section(GTEST_RGT_SECTION_NAME_,read,write)) \
    __declspec(allocate(GTEST_RGT_SECTION_NAME_)) \
    static ::testing::internal::RgtStaticItem *rgt_item_ptr = &ITEM; \
    ::testing::internal::rgt_fake_use = (void*)&rgt_item_ptr; \
  }

#endif // __GNUC__

// In the runtime-init case, the actual item doesn't go anywhere special.
#define GTEST_RGT_SECTION_ATTR

#else // GTEST_RGT_RUNTIME_INIT_

// The "normal" case, allocate local static data to a custom section
// which we can then iterate over as part of reporting.
#define GTEST_RGT_SECTION_NAME_ GTEST_RGT_SECTION_NAME_BASE_

// Define how to decorate the data declarations.
#define GTEST_RGT_SECTION_ATTR GTEST_RGT_SECTION_ATTR_2(GTEST_RGT_SECTION_NAME_)
#define GTEST_RGT_SECTION_ATTR_2(NAME) GTEST_RGT_SECTION_ATTR_3(NAME)
#define GTEST_RGT_SECTION_ATTR_3(NAME) __attribute__((section(#NAME),used))

// No special "record" action needed.
#define GTEST_RGT_RECORD_ITEM_(ITEM)

#endif // GTEST_RGT_RUNTIME_INIT_

// Common macro to declare one test assertion's data item and initialize it.
// Statically initialize 'executed' to false, dynamically set it true.

#define GTEST_RGT_DECLARE_                                         \
  GTEST_RGT_SECTION_ATTR static ::testing::internal::RgtStaticItem \
      gtest_rgt_item(__FILE__, __LINE__);                          \
  gtest_rgt_item.executed = true;                                  \
  GTEST_RGT_RECORD_ITEM_(gtest_rgt_item)

// EXPECT_*_FAILURE macros run a failure test, which appears un-executed
// because the executed flag is set only on the success path.
// Instrument these macros with their own record, which will correctly
// show it was executed. The reporting code will group all records in the
// source range of EXPECT_*_FAILURE, so the net result will be correct.
// Use a different item name to avoid warnings about shadowing.
#define GTEST_RGT_EXPECT_FAILURE_ \
  GTEST_RGT_SECTION_ATTR static ::testing::internal::RgtStaticItem \
      gtest_expect_item(__FILE__, __LINE__);                       \
  gtest_expect_item.executed = true;                               \
  GTEST_RGT_RECORD_ITEM_(gtest_expect_item)

#else // GTEST_HAS_RGT

// With RGT disabled, don't instrument anything.

#define GTEST_RGT_DECLARE_
#define GTEST_RGT_EXPECT_FAILURE_

#endif // GTEST_HAS_RGT

#endif  // GTEST_INCLUDE_GTEST_INTERNAL_GTEST_RGT_H_
