#!/usr/bin/env python
#
<<<<<<< HEAD
<<<<<<< HEAD:googletest/test/googletest-setuptestsuite-test.py
# Copyright 2019, Google Inc.
# All rights reserved.
=======
# Copyright 2019 Google LLC.  All Rights Reserved.
>>>>>>> 70989cf3f67042c181ac8f206e7cb91c0b0ba60f:googletest/test/gtest_skip_environment_check_output_test.py
=======
# Copyright 2019, Google Inc.
# All rights reserved.
>>>>>>> 70989cf3f67042c181ac8f206e7cb91c0b0ba60f
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
<<<<<<< HEAD
"""Tests Google Test's gtest skip in environment setup  behavior.

<<<<<<< HEAD:googletest/test/googletest-setuptestsuite-test.py
=======

>>>>>>> 70989cf3f67042c181ac8f206e7cb91c0b0ba60f
"""Verifies that SetUpTestSuite and TearDownTestSuite errors are noticed."""

from googletest.test import gtest_test_utils

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
<<<<<<< HEAD
=======
This script invokes gtest_skip_in_environment_setup_test_ and verifies its
output.
"""

from googletest.test import gtest_test_utils

# Path to the gtest_skip_in_environment_setup_test binary
EXE_PATH = gtest_test_utils.GetTestExecutablePath(
    'gtest_skip_in_environment_setup_test')

OUTPUT = gtest_test_utils.Subprocess([EXE_PATH]).output


# Test.
class SkipEntireEnvironmentTest(gtest_test_utils.TestCase):

  def testSkipEntireEnvironmentTest(self):
    self.assertIn('Skipping the entire environment', OUTPUT)
    self.assertNotIn('FAILED', OUTPUT)

>>>>>>> 70989cf3f67042c181ac8f206e7cb91c0b0ba60f:googletest/test/gtest_skip_environment_check_output_test.py
=======
>>>>>>> 70989cf3f67042c181ac8f206e7cb91c0b0ba60f

if __name__ == '__main__':
  gtest_test_utils.Main()
