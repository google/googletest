#!/usr/bin/env python
#
# Copyright 2009, Google Inc.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#     * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following disclaimer
# in the documentation and/or other materials provided with the
# distribution.
#     * Neither the name of Google Inc. nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

"""Tests that leaked mock objects can be caught be Google Mock."""

__author__ = 'wan@google.com (Zhanyong Wan)'

import gmock_test_utils
import os
import unittest

IS_WINDOWS = os.name == 'nt'

if IS_WINDOWS:
  # TODO(wan@google.com): test the opt build too.  We should do it
  # when Vlad Losev's work on Google Test's Python test driver is
  # done, such that we can reuse the work.
  PROGRAM = r'..\build.dbg\gmock_leak_test_.exe'
else:
  PROGRAM = 'gmock_leak_test_'

PROGRAM_PATH = os.path.join(gmock_test_utils.GetBuildDir(), PROGRAM)
TEST_WITH_EXPECT_CALL = PROGRAM_PATH + ' --gtest_filter=*ExpectCall*'
TEST_WITH_ON_CALL = PROGRAM_PATH + ' --gtest_filter=*OnCall*'
TEST_MULTIPLE_LEAKS = PROGRAM_PATH + ' --gtest_filter=*MultipleLeaked*'


class GMockLeakTest(unittest.TestCase):

  def testCatchesLeakedMockByDefault(self):
    self.assertNotEqual(os.system(TEST_WITH_EXPECT_CALL), 0)
    self.assertNotEqual(os.system(TEST_WITH_ON_CALL), 0)

  def testDoesNotCatchLeakedMockWhenDisabled(self):
    self.assertEquals(
        0, os.system(TEST_WITH_EXPECT_CALL + ' --gmock_catch_leaked_mocks=0'))
    self.assertEquals(
        0, os.system(TEST_WITH_ON_CALL + ' --gmock_catch_leaked_mocks=0'))

  def testCatchesLeakedMockWhenEnabled(self):
    self.assertNotEqual(
        os.system(TEST_WITH_EXPECT_CALL + ' --gmock_catch_leaked_mocks'), 0)
    self.assertNotEqual(
        os.system(TEST_WITH_ON_CALL + ' --gmock_catch_leaked_mocks'), 0)

  def testCatchesLeakedMockWhenEnabledWithExplictFlagValue(self):
    self.assertNotEqual(
        os.system(TEST_WITH_EXPECT_CALL + ' --gmock_catch_leaked_mocks=1'), 0)

  def testCatchesMultipleLeakedMocks(self):
    self.assertNotEqual(
        os.system(TEST_MULTIPLE_LEAKS + ' --gmock_catch_leaked_mocks'), 0)


if __name__ == '__main__':
  gmock_test_utils.Main()
