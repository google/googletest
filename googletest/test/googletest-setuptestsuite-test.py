#!/usr/bin/env python
#
# Copyright 2019, Google Inc.
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

"""Verifies that SetUpTestSuite and TearDownTestSuite errors are noticed."""

import gtest_test_utils

COMMAND = gtest_test_utils.GetTestExecutablePath(
    'googletest-setuptestsuite-test_')


class GTestSetUpTestSuiteTest(gtest_test_utils.TestCase):

  def testSetupErrorAndTearDownError(self):
    p = gtest_test_utils.Subprocess(COMMAND)
    self.assertNotEqual(p.exit_code, 0, msg=p.output)

    self.assertIn(
        '[  FAILED  ] SetupFailTest: SetUpTestSuite or TearDownTestSuite\n'
        '[  FAILED  ] TearDownFailTest: SetUpTestSuite or TearDownTestSuite\n'
        '\n'
        ' 2 FAILED TEST SUITES\n',
        p.output)

if __name__ == '__main__':
  gtest_test_utils.Main()
