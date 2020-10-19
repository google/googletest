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
// gtest_xml_outfile3_test_ writes some xml via TestProperty used by
// gtest_xml_outfiles_test.py using parameterized tests

#include "gtest/gtest.h"

class InvalidXMLTest : public testing::TestWithParam<const char*> {};

TEST_P(InvalidXMLTest, testInvalids) {
}

const char* forbiddenInXML[] = {
    "\xef\xbf\xbe", // forbidden non-character
    "\xef\xbf\xbe", // forbidden non-character
    "\xed\xa0\x80", // first high surrogate
    "\xed\xad\xbf", // last high surrogate
    "\xed\xb0\x80", // first low surrogate
    "\xed\xbf\xbf", // last low surrogate
    "\xed\xa9\x92", // some high surrogate
    "\xed\xb8\x92" // some low surrogate
};

INSTANTIATE_TEST_SUITE_P(InvalidXMLTestInstance, InvalidXMLTest,
                         ::testing::ValuesIn(forbiddenInXML));