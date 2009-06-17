#!/usr/bin/env python
#
# Copyright 2008, Google Inc.
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

"""Verifies that Google Test correctly parses environment variables."""

__author__ = 'wan@google.com (Zhanyong Wan)'

import os
import gtest_test_utils


IS_WINDOWS = os.name == 'nt'
IS_LINUX = os.name == 'posix' and os.uname()[0] == 'Linux'

COMMAND = gtest_test_utils.GetTestExecutablePath('gtest_env_var_test_')


def AssertEq(expected, actual):
  if expected != actual:
    print 'Expected: %s' % (expected,)
    print '  Actual: %s' % (actual,)
    raise AssertionError


def SetEnvVar(env_var, value):
  """Sets the env variable to 'value'; unsets it when 'value' is None."""

  if value is not None:
    os.environ[env_var] = value
  elif env_var in os.environ:
    del os.environ[env_var]


def GetFlag(command, flag):
  """Runs gtest_env_var_test_ and returns its output."""

  cmd = command
  if flag is not None:
    cmd += ' %s' % (flag,)
  stdin, stdout = os.popen2(cmd, 'b')
  stdin.close()
  line = stdout.readline()
  stdout.close()
  return line


def TestFlag(command, flag, test_val, default_val):
  """Verifies that the given flag is affected by the corresponding env var."""

  env_var = 'GTEST_' + flag.upper()
  SetEnvVar(env_var, test_val)
  AssertEq(test_val, GetFlag(command, flag))
  SetEnvVar(env_var, None)
  AssertEq(default_val, GetFlag(command, flag))


def TestEnvVarAffectsFlag(command):
  """An environment variable should affect the corresponding flag."""

  TestFlag(command, 'break_on_failure', '1', '0')
  TestFlag(command, 'color', 'yes', 'auto')
  TestFlag(command, 'filter', 'FooTest.Bar', '*')
  TestFlag(command, 'output', 'tmp/foo.xml', '')
  TestFlag(command, 'print_time', '0', '1')
  TestFlag(command, 'repeat', '999', '1')
  TestFlag(command, 'throw_on_failure', '1', '0')
  TestFlag(command, 'death_test_style', 'threadsafe', 'fast')

  if IS_WINDOWS:
    TestFlag(command, 'catch_exceptions', '1', '0')

  if IS_LINUX:
    TestFlag(command, 'death_test_use_fork', '1', '0')
    TestFlag(command, 'stack_trace_depth', '0', '100')


class GTestEnvVarTest(gtest_test_utils.TestCase):
  def testEnvVarAffectsFlag(self):
    TestEnvVarAffectsFlag(COMMAND)


if __name__ == '__main__':
  gtest_test_utils.Main()
