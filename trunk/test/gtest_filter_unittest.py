#!/usr/bin/env python
#
# Copyright 2005, Google Inc.
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

"""Unit test for Google Test test filters.

A user can specify which test(s) in a Google Test program to run via either
the GTEST_FILTER environment variable or the --gtest_filter flag.
This script tests such functionality by invoking
gtest_filter_unittest_ (a program written with Google Test) with different
environments and command line flags.

Note that test sharding may also influence which tests are filtered. Therefore,
we test that here also.
"""

__author__ = 'wan@google.com (Zhanyong Wan)'

import os
import re
import sets
import tempfile
import unittest
import gtest_test_utils

# Constants.

# The environment variable for specifying the test filters.
FILTER_ENV_VAR = 'GTEST_FILTER'

# The environment variables for test sharding.
TOTAL_SHARDS_ENV_VAR = 'GTEST_TOTAL_SHARDS'
SHARD_INDEX_ENV_VAR = 'GTEST_SHARD_INDEX'
SHARD_STATUS_FILE_ENV_VAR = 'GTEST_SHARD_STATUS_FILE'

# The command line flag for specifying the test filters.
FILTER_FLAG = 'gtest_filter'

# The command line flag for including disabled tests.
ALSO_RUN_DISABED_TESTS_FLAG = 'gtest_also_run_disabled_tests'

# Command to run the gtest_filter_unittest_ program.
COMMAND = os.path.join(gtest_test_utils.GetBuildDir(),
                       'gtest_filter_unittest_')

# Regex for determining whether parameterized tests are enabled in the binary.
PARAM_TEST_REGEX = re.compile(r'/ParamTest')

# Regex for parsing test case names from Google Test's output.
TEST_CASE_REGEX = re.compile(r'^\[\-+\] \d+ tests? from (\w+(/\w+)?)')

# Regex for parsing test names from Google Test's output.
TEST_REGEX = re.compile(r'^\[\s*RUN\s*\].*\.(\w+(/\w+)?)')

# Full names of all tests in gtest_filter_unittests_.
PARAM_TESTS = [
    'SeqP/ParamTest.TestX/0',
    'SeqP/ParamTest.TestX/1',
    'SeqP/ParamTest.TestY/0',
    'SeqP/ParamTest.TestY/1',
    'SeqQ/ParamTest.TestX/0',
    'SeqQ/ParamTest.TestX/1',
    'SeqQ/ParamTest.TestY/0',
    'SeqQ/ParamTest.TestY/1',
    ]

DISABLED_TESTS = [
    'BarTest.DISABLED_TestFour',
    'BarTest.DISABLED_TestFive',
    'BazTest.DISABLED_TestC',
    'DISABLED_FoobarTest.Test1',
    'DISABLED_FoobarTest.DISABLED_Test2',
    'DISABLED_FoobarbazTest.TestA',
    ]

# All the non-disabled tests.
ACTIVE_TESTS = [
    'FooTest.Abc',
    'FooTest.Xyz',

    'BarTest.TestOne',
    'BarTest.TestTwo',
    'BarTest.TestThree',

    'BazTest.TestOne',
    'BazTest.TestA',
    'BazTest.TestB',

    'HasDeathTest.Test1',
    'HasDeathTest.Test2',
    ] + PARAM_TESTS

param_tests_present = None

# Utilities.


def SetEnvVar(env_var, value):
  """Sets the env variable to 'value'; unsets it when 'value' is None."""

  if value is not None:
    os.environ[env_var] = value
  elif env_var in os.environ:
    del os.environ[env_var]


def Run(command):
  """Runs a Google Test program and returns a list of full names of the
  tests that were run along with the test exit code.
  """

  stdout_file = os.popen(command, 'r')
  tests_run = []
  test_case = ''
  test = ''
  for line in stdout_file:
    match = TEST_CASE_REGEX.match(line)
    if match is not None:
      test_case = match.group(1)
    else:
      match = TEST_REGEX.match(line)
      if match is not None:
        test = match.group(1)
        tests_run += [test_case + '.' + test]
  exit_code = stdout_file.close()
  return (tests_run, exit_code)


def InvokeWithModifiedEnv(extra_env, function, *args, **kwargs):
  """Runs the given function and arguments in a modified environment."""
  try:
    original_env = os.environ.copy()
    os.environ.update(extra_env)
    return function(*args, **kwargs)
  finally:
    for key in extra_env.iterkeys():
      if key in original_env:
        os.environ[key] = original_env[key]
      else:
        del os.environ[key]


def RunWithSharding(total_shards, shard_index, command):
  """Runs the Google Test program shard and returns a list of full names of the
  tests that were run along with the exit code.
  """

  extra_env = {SHARD_INDEX_ENV_VAR: str(shard_index),
               TOTAL_SHARDS_ENV_VAR: str(total_shards)}
  return InvokeWithModifiedEnv(extra_env, Run, command)

# The unit test.


class GTestFilterUnitTest(unittest.TestCase):
  """Tests using the GTEST_FILTER environment variable or the
  --gtest_filter flag to filter tests.
  """

  # Utilities.

  def AssertSetEqual(self, lhs, rhs):
    """Asserts that two sets are equal."""

    for elem in lhs:
      self.assert_(elem in rhs, '%s in %s' % (elem, rhs))

    for elem in rhs:
      self.assert_(elem in lhs, '%s in %s' % (elem, lhs))

  def AssertPartitionIsValid(self, set_var, list_of_sets):
    """Asserts that list_of_sets is a valid partition of set_var."""

    full_partition = []
    for slice_var in list_of_sets:
      full_partition.extend(slice_var)
    self.assertEqual(len(set_var), len(full_partition))
    self.assertEqual(sorted(set_var), sorted(full_partition))

  def RunAndVerify(self, gtest_filter, tests_to_run):
    """Runs gtest_flag_unittest_ with the given filter, and verifies
    that the right set of tests were run.
    """
    # Adjust tests_to_run in case value parameterized tests are disabled
    # in the binary.
    global param_tests_present
    if not param_tests_present:
      tests_to_run = list(sets.Set(tests_to_run) - sets.Set(PARAM_TESTS))

    # First, tests using GTEST_FILTER.

    SetEnvVar(FILTER_ENV_VAR, gtest_filter)
    tests_run = Run(COMMAND)[0]
    SetEnvVar(FILTER_ENV_VAR, None)

    self.AssertSetEqual(tests_run, tests_to_run)

    # Next, tests using --gtest_filter.

    if gtest_filter is None:
      command = COMMAND
    else:
      command = '%s --%s=%s' % (COMMAND, FILTER_FLAG, gtest_filter)

    tests_run = Run(command)[0]
    self.AssertSetEqual(tests_run, tests_to_run)

  def RunAndVerifyWithSharding(self, gtest_filter, total_shards, tests_to_run,
                               command=COMMAND, check_exit_0=False):
    """Runs all shards of gtest_flag_unittest_ with the given filter, and
    verifies that the right set of tests were run. The union of tests run
    on each shard should be identical to tests_to_run, without duplicates.
    If check_exit_0, make sure that all shards returned 0.
    """
    SetEnvVar(FILTER_ENV_VAR, gtest_filter)
    partition = []
    for i in range(0, total_shards):
      (tests_run, exit_code) = RunWithSharding(total_shards, i, command)
      if check_exit_0:
        self.assert_(exit_code is None)
      partition.append(tests_run)

    self.AssertPartitionIsValid(tests_to_run, partition)
    SetEnvVar(FILTER_ENV_VAR, None)

  def RunAndVerifyAllowingDisabled(self, gtest_filter, tests_to_run):
    """Runs gtest_flag_unittest_ with the given filter, and enables
    disabled tests. Verifies that the right set of tests were run.
    """
    # Construct the command line.
    command = '%s --%s' % (COMMAND, ALSO_RUN_DISABED_TESTS_FLAG)
    if gtest_filter is not None:
      command = '%s --%s=%s' % (command, FILTER_FLAG, gtest_filter)

    tests_run = Run(command)[0]
    self.AssertSetEqual(tests_run, tests_to_run)

  def setUp(self):
    """Sets up test case. Determines whether value-parameterized tests are
    enabled in the binary and sets flags accordingly.
    """
    global param_tests_present
    if param_tests_present is None:
      param_tests_present = PARAM_TEST_REGEX.search(
          '\n'.join(os.popen(COMMAND, 'r').readlines())) is not None

  def testDefaultBehavior(self):
    """Tests the behavior of not specifying the filter."""

    self.RunAndVerify(None, ACTIVE_TESTS)

  def testDefaultBehaviorWithShards(self):
    """Tests the behavior of not specifying the filter, with sharding
    enabled.
    """
    self.RunAndVerifyWithSharding(None, 1, ACTIVE_TESTS)
    self.RunAndVerifyWithSharding(None, 2, ACTIVE_TESTS)
    self.RunAndVerifyWithSharding(None, len(ACTIVE_TESTS) - 1, ACTIVE_TESTS)
    self.RunAndVerifyWithSharding(None, len(ACTIVE_TESTS), ACTIVE_TESTS)
    self.RunAndVerifyWithSharding(None, len(ACTIVE_TESTS) + 1, ACTIVE_TESTS)

  def testEmptyFilter(self):
    """Tests an empty filter."""

    self.RunAndVerify('', [])
    self.RunAndVerifyWithSharding('', 1, [])
    self.RunAndVerifyWithSharding('', 2, [])

  def testBadFilter(self):
    """Tests a filter that matches nothing."""

    self.RunAndVerify('BadFilter', [])
    self.RunAndVerifyAllowingDisabled('BadFilter', [])

  def testFullName(self):
    """Tests filtering by full name."""

    self.RunAndVerify('FooTest.Xyz', ['FooTest.Xyz'])
    self.RunAndVerifyAllowingDisabled('FooTest.Xyz', ['FooTest.Xyz'])
    self.RunAndVerifyWithSharding('FooTest.Xyz', 5, ['FooTest.Xyz'])

  def testUniversalFilters(self):
    """Tests filters that match everything."""

    self.RunAndVerify('*', ACTIVE_TESTS)
    self.RunAndVerify('*.*', ACTIVE_TESTS)
    self.RunAndVerifyWithSharding('*.*', len(ACTIVE_TESTS) - 3, ACTIVE_TESTS)
    self.RunAndVerifyAllowingDisabled('*', ACTIVE_TESTS + DISABLED_TESTS)
    self.RunAndVerifyAllowingDisabled('*.*', ACTIVE_TESTS + DISABLED_TESTS)

  def testFilterByTestCase(self):
    """Tests filtering by test case name."""

    self.RunAndVerify('FooTest.*', ['FooTest.Abc', 'FooTest.Xyz'])

    BAZ_TESTS = ['BazTest.TestOne', 'BazTest.TestA', 'BazTest.TestB']
    self.RunAndVerify('BazTest.*', BAZ_TESTS)
    self.RunAndVerifyAllowingDisabled('BazTest.*',
                                      BAZ_TESTS + ['BazTest.DISABLED_TestC'])

  def testFilterByTest(self):
    """Tests filtering by test name."""

    self.RunAndVerify('*.TestOne', ['BarTest.TestOne', 'BazTest.TestOne'])

  def testFilterDisabledTests(self):
    """Select only the disabled tests to run."""

    self.RunAndVerify('DISABLED_FoobarTest.Test1', [])
    self.RunAndVerifyAllowingDisabled('DISABLED_FoobarTest.Test1',
                                      ['DISABLED_FoobarTest.Test1'])

    self.RunAndVerify('*DISABLED_*', [])
    self.RunAndVerifyAllowingDisabled('*DISABLED_*', DISABLED_TESTS)

    self.RunAndVerify('*.DISABLED_*', [])
    self.RunAndVerifyAllowingDisabled('*.DISABLED_*', [
        'BarTest.DISABLED_TestFour',
        'BarTest.DISABLED_TestFive',
        'BazTest.DISABLED_TestC',
        'DISABLED_FoobarTest.DISABLED_Test2',
        ])

    self.RunAndVerify('DISABLED_*', [])
    self.RunAndVerifyAllowingDisabled('DISABLED_*', [
        'DISABLED_FoobarTest.Test1',
        'DISABLED_FoobarTest.DISABLED_Test2',
        'DISABLED_FoobarbazTest.TestA',
        ])

  def testWildcardInTestCaseName(self):
    """Tests using wildcard in the test case name."""

    self.RunAndVerify('*a*.*', [
        'BarTest.TestOne',
        'BarTest.TestTwo',
        'BarTest.TestThree',

        'BazTest.TestOne',
        'BazTest.TestA',
        'BazTest.TestB',

        'HasDeathTest.Test1',
        'HasDeathTest.Test2', ] + PARAM_TESTS)

  def testWildcardInTestName(self):
    """Tests using wildcard in the test name."""

    self.RunAndVerify('*.*A*', ['FooTest.Abc', 'BazTest.TestA'])

  def testFilterWithoutDot(self):
    """Tests a filter that has no '.' in it."""

    self.RunAndVerify('*z*', [
        'FooTest.Xyz',

        'BazTest.TestOne',
        'BazTest.TestA',
        'BazTest.TestB',
        ])

  def testTwoPatterns(self):
    """Tests filters that consist of two patterns."""

    self.RunAndVerify('Foo*.*:*A*', [
        'FooTest.Abc',
        'FooTest.Xyz',

        'BazTest.TestA',
        ])

    # An empty pattern + a non-empty one
    self.RunAndVerify(':*A*', ['FooTest.Abc', 'BazTest.TestA'])

  def testThreePatterns(self):
    """Tests filters that consist of three patterns."""

    self.RunAndVerify('*oo*:*A*:*One', [
        'FooTest.Abc',
        'FooTest.Xyz',

        'BarTest.TestOne',

        'BazTest.TestOne',
        'BazTest.TestA',
        ])

    # The 2nd pattern is empty.
    self.RunAndVerify('*oo*::*One', [
        'FooTest.Abc',
        'FooTest.Xyz',

        'BarTest.TestOne',

        'BazTest.TestOne',
        ])

    # The last 2 patterns are empty.
    self.RunAndVerify('*oo*::', [
        'FooTest.Abc',
        'FooTest.Xyz',
        ])

  def testNegativeFilters(self):
    self.RunAndVerify('*-HasDeathTest.Test1', [
        'FooTest.Abc',
        'FooTest.Xyz',

        'BarTest.TestOne',
        'BarTest.TestTwo',
        'BarTest.TestThree',

        'BazTest.TestOne',
        'BazTest.TestA',
        'BazTest.TestB',

        'HasDeathTest.Test2',
        ] + PARAM_TESTS)

    self.RunAndVerify('*-FooTest.Abc:HasDeathTest.*', [
        'FooTest.Xyz',

        'BarTest.TestOne',
        'BarTest.TestTwo',
        'BarTest.TestThree',

        'BazTest.TestOne',
        'BazTest.TestA',
        'BazTest.TestB',
        ] + PARAM_TESTS)

    self.RunAndVerify('BarTest.*-BarTest.TestOne', [
        'BarTest.TestTwo',
        'BarTest.TestThree',
        ])

    # Tests without leading '*'.
    self.RunAndVerify('-FooTest.Abc:FooTest.Xyz:HasDeathTest.*', [
        'BarTest.TestOne',
        'BarTest.TestTwo',
        'BarTest.TestThree',

        'BazTest.TestOne',
        'BazTest.TestA',
        'BazTest.TestB',
        ] + PARAM_TESTS)

    # Value parameterized tests.
    self.RunAndVerify('*/*', PARAM_TESTS)

    # Value parameterized tests filtering by the sequence name.
    self.RunAndVerify('SeqP/*', [
        'SeqP/ParamTest.TestX/0',
        'SeqP/ParamTest.TestX/1',
        'SeqP/ParamTest.TestY/0',
        'SeqP/ParamTest.TestY/1',
        ])

    # Value parameterized tests filtering by the test name.
    self.RunAndVerify('*/0', [
        'SeqP/ParamTest.TestX/0',
        'SeqP/ParamTest.TestY/0',
        'SeqQ/ParamTest.TestX/0',
        'SeqQ/ParamTest.TestY/0',
        ])

  def testFlagOverridesEnvVar(self):
    """Tests that the --gtest_filter flag overrides the GTEST_FILTER
    environment variable.
    """

    SetEnvVar(FILTER_ENV_VAR, 'Foo*')
    command = '%s --%s=%s' % (COMMAND, FILTER_FLAG, '*One')
    tests_run = Run(command)[0]
    SetEnvVar(FILTER_ENV_VAR, None)

    self.AssertSetEqual(tests_run, ['BarTest.TestOne', 'BazTest.TestOne'])

  def testShardStatusFileIsCreated(self):
    """Tests that the shard file is created if specified in the environment."""

    test_tmpdir = tempfile.mkdtemp()
    shard_status_file = os.path.join(test_tmpdir, 'shard_status_file')
    self.assert_(not os.path.exists(shard_status_file))

    extra_env = {SHARD_STATUS_FILE_ENV_VAR: shard_status_file}
    stdout_file = InvokeWithModifiedEnv(extra_env, os.popen, COMMAND, 'r')
    try:
      stdout_file.readlines()
    finally:
      stdout_file.close()
      self.assert_(os.path.exists(shard_status_file))
      os.remove(shard_status_file)
      os.removedirs(test_tmpdir)

  def testShardStatusFileIsCreatedWithListTests(self):
    """Tests that the shard file is created with --gtest_list_tests."""

    test_tmpdir = tempfile.mkdtemp()
    shard_status_file = os.path.join(test_tmpdir, 'shard_status_file2')
    self.assert_(not os.path.exists(shard_status_file))

    extra_env = {SHARD_STATUS_FILE_ENV_VAR: shard_status_file}
    stdout_file = InvokeWithModifiedEnv(extra_env, os.popen,
                                        '%s --gtest_list_tests' % COMMAND, 'r')
    try:
      stdout_file.readlines()
    finally:
      stdout_file.close()
      self.assert_(os.path.exists(shard_status_file))
      os.remove(shard_status_file)
      os.removedirs(test_tmpdir)

  def testShardingWorksWithDeathTests(self):
    """Tests integration with death tests and sharding."""
    gtest_filter = 'HasDeathTest.*:SeqP/*'
    expected_tests = [
        'HasDeathTest.Test1',
        'HasDeathTest.Test2',

        'SeqP/ParamTest.TestX/0',
        'SeqP/ParamTest.TestX/1',
        'SeqP/ParamTest.TestY/0',
        'SeqP/ParamTest.TestY/1',
        ]

    for command in (COMMAND + ' --gtest_death_test_style=threadsafe',
                    COMMAND + ' --gtest_death_test_style=fast'):
      self.RunAndVerifyWithSharding(gtest_filter, 3, expected_tests,
                                    check_exit_0=True, command=command)
      self.RunAndVerifyWithSharding(gtest_filter, 5, expected_tests,
                                    check_exit_0=True, command=command)

if __name__ == '__main__':
  gtest_test_utils.Main()
