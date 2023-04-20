// Copyright 2008, Google Inc.
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
// gtest_xml_outfile2_test_ writes some xml via TestProperty used by
// gtest_xml_outfiles_test.py

#include <atomic>

#include "gtest/gtest.h"

class PropertyTwo : public testing::Test {
 protected:
  void SetUp() override { RecordProperty("SetUpProp", 2); }
  void TearDown() override { RecordProperty("TearDownProp", 2); }
};

TEST_F(PropertyTwo, TestInt64ConvertibleProperties) {
  float float_prop = 3.25;
  RecordProperty("TestFloatProperty", float_prop);

  double double_prop = 4.75;
  RecordProperty("TestDoubleProperty", double_prop);

  // Validate we can write an unsigned size_t as a property
  size_t size_t_prop = 5;
  RecordProperty("TestSizetProperty", size_t_prop);

  bool bool_prop = true;
  RecordProperty("TestBoolProperty", bool_prop);

  char char_prop = 'A';
  RecordProperty("TestCharProperty", char_prop);

  int16_t int16_prop = 6;
  RecordProperty("TestInt16Property", int16_prop);

  int32_t int32_prop = 7;
  RecordProperty("TestInt32Property", int32_prop);

  int64_t int64_prop = 8;
  RecordProperty("TestInt64Property", int64_prop);

  enum Foo {
    NINE = 9,
  };
  Foo enum_prop = NINE;
  RecordProperty("TestEnumProperty", enum_prop);

  std::atomic<int> atomic_int_prop(10);
  RecordProperty("TestAtomicIntProperty", atomic_int_prop);
}
