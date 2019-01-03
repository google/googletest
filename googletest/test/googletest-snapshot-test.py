#!/usr/bin/env python
#
# Copyright 2018, Google Inc.
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

"""Tests Google Test's EXPECT_EQ_SNAPSHOT.

This script invokes googletest-snapshot-test_ (a program written with
Google Test) with different environments and command line flags.
"""

import os
import sys
import gtest_test_utils


# Constants.

# Path to the googletest-snapshot-test_ program.
EXE_PATH = gtest_test_utils.GetTestExecutablePath(
    'googletest-snapshot-test_')


# Utilities.

def PrettyPrintDict(d):
  return ','.join([ '%s=%s' % (k, v) for k,v in d.items()])


def Run(command, env=None):
  """Runs a command; returns True/False if its exit code is/isn't 0."""

  print('Running "%s" with environment "%s"' % (' '.join(command),
    PrettyPrintDict(env) ))
  p = gtest_test_utils.Subprocess(command=command, env=env)
  return p.exited and p.exit_code == 0


class SnapshotTest(gtest_test_utils.TestCase):
  """Tests EXPECT_EQ_SNAPSHOT. Test order matters."""

  def RunAndVerify(self, env, should_fail):
    """Runs googletest-snapshot-test_ and verifies that it does
    (or does not) exit with a non-zero code.

    Args:
      env:              map of environment variables
      should_fail:      True iff the program is expected to fail.
    """

    command = [EXE_PATH]

    if should_fail:
      should_or_not = 'should'
    else:
      should_or_not = 'should not'

    failed = not Run(command, env)

    msg = ('with env %s, an assertion failure in "%s" %s cause a non-zero '
           'exit code.' %
           (PrettyPrintDict(env), ' '.join(command),
            should_or_not))
    self.assert_(failed == should_fail, msg)

  def testFailWithNoSnapshot(self):
    """Fail if no snapshot exists"""
    self.RunAndVerify(
        env={ 'GREETERTEST_PARAMS': 'Alpha Bravo' },
        should_fail=True)

  def testSucceedWithGoodSnapshots(self):
    """Succeed if snapshot is correct"""
    self.RunAndVerify(
        env={ 'GREETERTEST_PARAMS': 'Charlie Delta' },
        should_fail=False)

  def testFailWithBadSnapshots(self):
    """Fail if snapshot is incorrect"""
    self.RunAndVerify(
        env={ 'GREETERTEST_PARAMS': 'Echo Foxtrot' },
        should_fail=True)

if __name__ == '__main__':
  gtest_test_utils.Main()
