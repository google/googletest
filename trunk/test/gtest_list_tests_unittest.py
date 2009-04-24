#!/usr/bin/env python
#
# Copyright 2006, Google Inc.
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

"""Unit test for Google Test's --gtest_list_tests flag.

A user can ask Google Test to list all tests by specifying the
--gtest_list_tests flag.  This script tests such functionality
by invoking gtest_list_tests_unittest_ (a program written with
Google Test) the command line flags.
"""

__author__ = 'phanna@google.com (Patrick Hanna)'

import gtest_test_utils
import os
import re
import sys
import unittest


# Constants.

# The command line flag for enabling/disabling listing all tests.
LIST_TESTS_FLAG = 'gtest_list_tests'

# Path to the gtest_list_tests_unittest_ program.
EXE_PATH = gtest_test_utils.GetTestExecutablePath('gtest_list_tests_unittest_')

# The expected output when running gtest_list_tests_unittest_ with
# --gtest_list_tests
EXPECTED_OUTPUT_NO_FILTER = """FooDeathTest.
  Test1
Foo.
  Bar1
  Bar2
  DISABLED_Bar3
Abc.
  Xyz
  Def
FooBar.
  Baz
FooTest.
  Test1
  DISABLED_Test2
  Test3
"""

# The expected output when running gtest_list_tests_unittest_ with
# --gtest_list_tests and --gtest_filter=Foo*.
EXPECTED_OUTPUT_FILTER_FOO = """FooDeathTest.
  Test1
Foo.
  Bar1
  Bar2
  DISABLED_Bar3
FooBar.
  Baz
FooTest.
  Test1
  DISABLED_Test2
  Test3
"""

# Utilities.


def Run(command):
  """Runs a command and returns the list of tests printed."""

  stdout_file = os.popen(command, 'r')

  output = stdout_file.read()

  stdout_file.close()
  return output


# The unit test.

class GTestListTestsUnitTest(unittest.TestCase):
  """Tests using the --gtest_list_tests flag to list all tests."""

  def RunAndVerify(self, flag_value, expected_output, other_flag):
    """Runs gtest_list_tests_unittest_ and verifies that it prints
    the correct tests.

    Args:
      flag_value:       value of the --gtest_list_tests flag;
                        None if the flag should not be present.

      expected_output:  the expected output after running command;

      other_flag:       a different flag to be passed to command
                        along with gtest_list_tests;
                        None if the flag should not be present.
    """

    if flag_value is None:
      flag = ''
      flag_expression = "not set"
    elif flag_value == '0':
      flag = ' --%s=0' % LIST_TESTS_FLAG
      flag_expression = "0"
    else:
      flag = ' --%s' % LIST_TESTS_FLAG
      flag_expression = "1"

    command = EXE_PATH + flag

    if other_flag is not None:
      command += " " + other_flag

    output = Run(command)

    msg = ('when %s is %s, the output of "%s" is "%s".' %
           (LIST_TESTS_FLAG, flag_expression, command, output))

    if expected_output is not None:
      self.assert_(output == expected_output, msg)
    else:
      self.assert_(output != EXPECTED_OUTPUT_NO_FILTER, msg)

  def testDefaultBehavior(self):
    """Tests the behavior of the default mode."""

    self.RunAndVerify(flag_value=None,
                      expected_output=None,
                      other_flag=None)

  def testFlag(self):
    """Tests using the --gtest_list_tests flag."""

    self.RunAndVerify(flag_value='0',
                      expected_output=None,
                      other_flag=None)
    self.RunAndVerify(flag_value='1',
                      expected_output=EXPECTED_OUTPUT_NO_FILTER,
                      other_flag=None)

  def testOverrideNonFilterFlags(self):
    """Tests that --gtest_list_tests overrides the non-filter flags."""

    self.RunAndVerify(flag_value="1",
                      expected_output=EXPECTED_OUTPUT_NO_FILTER,
                      other_flag="--gtest_break_on_failure")

  def testWithFilterFlags(self):
    """Tests that --gtest_list_tests takes into account the
    --gtest_filter flag."""

    self.RunAndVerify(flag_value="1",
                      expected_output=EXPECTED_OUTPUT_FILTER_FOO,
                      other_flag="--gtest_filter=Foo*")


if __name__ == '__main__':
  gtest_test_utils.Main()
