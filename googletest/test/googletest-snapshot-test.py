#!/usr/bin/env python
#
# Copyright 2018, Cristian Klein
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

# The environment variable where we store the source directory
TEST_SRCDIR = 'TEST_SRCDIR'

# The command line flag for updating snapshots
UPDATE_SNAPSHOT = 'gtest_update_snapshot'

# Path to the googletest-snapshot-test_ program.
EXE_PATH = gtest_test_utils.GetTestExecutablePath(
    'googletest-snapshot-test_')


# Utilities.


def SetEnvVar(env_var, value):
  """Sets an environment variable to a given value; unsets it when the
  given value is None.
  """

  env_var = env_var.upper()
  if value is not None:
    os.environ[env_var] = value
  elif env_var in os.environ:
    del os.environ[env_var]


def Run(command):
  """Runs a command; returns True/False if its exit code is/isn't 0."""

  print('Running "%s". . .' % ' '.join(command))
  p = gtest_test_utils.Subprocess(command)
  return p.exited and p.exit_code == 0


def RemoveSnapshot():
  for param in [ 'Alice', 'Bob' ]:
    snapshotFile = os.environ[TEST_SRCDIR] + \
      '/test/googletest-snapshot-test_.cc_{0}.snap'.format(param)
    try:
      os.remove(snapshotFile)
    except OSError:
      print('Could not remove snapshot file: {0}'.format(snapshotFile))
      # okey if snapshot file did not exist when test started
      pass


class SnapshotTest(gtest_test_utils.TestCase):
  """Tests EXPECT_EQ_SNAPSHOT. Test order matters."""

  def RunAndVerify(self, env_var_value, should_fail):
    """Runs googletest-snapshot-test_ and verifies that it does
    (or does not) exit with a non-zero code.

    Args:
      env_var_value:    value of the GTEST_UPDATE_SNAPSHOT environment
                        variable; None if the variable should be unset.
      should_fail:      True iff the program is expected to fail.
    """

    SetEnvVar(UPDATE_SNAPSHOT, env_var_value)

    if env_var_value is None:
      env_var_value_msg = ' is not set'
    else:
      env_var_value_msg = '=' + env_var_value

    command = [EXE_PATH]

    if should_fail:
      should_or_not = 'should'
    else:
      should_or_not = 'should not'

    failed = not Run(command)

    SetEnvVar(UPDATE_SNAPSHOT, None)

    msg = ('when %s%s, an assertion failure in "%s" %s cause a non-zero '
           'exit code.' %
           (UPDATE_SNAPSHOT, env_var_value_msg, ' '.join(command),
            should_or_not))
    self.assert_(failed == should_fail, msg)

  @classmethod
  def setUpClass(cls):
    """Start from scratch, make sure snapshots do not exists."""
    RemoveSnapshot()

  @classmethod
  def tearDownClass(cls):
    """Leave a tidy source tree after us."""
    RemoveSnapshot()

  def test00FailWithNoSnapshot(self):
    """Starting case, no snapshot exists"""
    self.RunAndVerify(env_var_value=None, should_fail=True)

  def test01SucceedWithSnapshotUpdate(self):
    """Developer generates first snapshot"""
    self.RunAndVerify(env_var_value="1", should_fail=False)

  def test02SucceedWithExistingSnapshot(self):
    """Developer uses snapshot in subsequent tests"""
    self.RunAndVerify(env_var_value=None, should_fail=False)

if __name__ == '__main__':
  # Get source dir (using a poor man's argparse)
  for arg in sys.argv:
    if '--sourcedir=' in arg:
      os.environ[TEST_SRCDIR] = arg.split('=')[1]
      sys.argv.remove(arg)
      break

  gtest_test_utils.Main()
