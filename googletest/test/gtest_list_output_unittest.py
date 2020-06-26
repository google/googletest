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
--gtest_list_tests flag. If output is requested, via --gtest_output=xml
or --gtest_output=json, the tests are listed, with extra information in the
output file.
This script tests such functionality by invoking gtest_list_output_unittest_
 (a program written with Google Test) the command line flags.
"""

import os
import re
import gtest_test_utils

GTEST_LIST_TESTS_FLAG = '--gtest_list_tests'
GTEST_OUTPUT_FLAG = '--gtest_output'

EXPECTED_XML = """<\?xml version="1.0" encoding="UTF-8"\?>
<testsuites tests="16" name="AllTests">
  <testsuite name="FooTest" tests="2">
    <testcase name="Test1" file=".*gtest_list_output_unittest_.cc" line="43" />
    <testcase name="Test2" file=".*gtest_list_output_unittest_.cc" line="45" />
  </testsuite>
  <testsuite name="FooTestFixture" tests="2">
    <testcase name="Test3" file=".*gtest_list_output_unittest_.cc" line="48" />
    <testcase name="Test4" file=".*gtest_list_output_unittest_.cc" line="49" />
  </testsuite>
  <testsuite name="TypedTest/0" tests="2">
    <testcase name="Test7" type_param="int" file=".*gtest_list_output_unittest_.cc" line="61" />
    <testcase name="Test8" type_param="int" file=".*gtest_list_output_unittest_.cc" line="62" />
  </testsuite>
  <testsuite name="TypedTest/1" tests="2">
    <testcase name="Test7" type_param="bool" file=".*gtest_list_output_unittest_.cc" line="61" />
    <testcase name="Test8" type_param="bool" file=".*gtest_list_output_unittest_.cc" line="62" />
  </testsuite>
  <testsuite name="Single/TypeParameterizedTestSuite/0" tests="2">
    <testcase name="Test9" type_param="int" file=".*gtest_list_output_unittest_.cc" line="69" />
    <testcase name="Test10" type_param="int" file=".*gtest_list_output_unittest_.cc" line="70" />
  </testsuite>
  <testsuite name="Single/TypeParameterizedTestSuite/1" tests="2">
    <testcase name="Test9" type_param="bool" file=".*gtest_list_output_unittest_.cc" line="69" />
    <testcase name="Test10" type_param="bool" file=".*gtest_list_output_unittest_.cc" line="70" />
  </testsuite>
  <testsuite name="ValueParam/ValueParamTest" tests="4">
    <testcase name="Test5/0" value_param="33" file=".*gtest_list_output_unittest_.cc" line="52" />
    <testcase name="Test5/1" value_param="42" file=".*gtest_list_output_unittest_.cc" line="52" />
    <testcase name="Test6/0" value_param="33" file=".*gtest_list_output_unittest_.cc" line="53" />
    <testcase name="Test6/1" value_param="42" file=".*gtest_list_output_unittest_.cc" line="53" />
  </testsuite>
</testsuites>
"""

EXPECTED_JSON = """{
  "tests": 16,
  "name": "AllTests",
  "testsuites": \[
    {
      "name": "FooTest",
      "tests": 2,
      "testsuite": \[
        {
          "name": "Test1",
          "file": ".*gtest_list_output_unittest_.cc",
          "line": 43
        },
        {
          "name": "Test2",
          "file": ".*gtest_list_output_unittest_.cc",
          "line": 45
        }
      \]
    },
    {
      "name": "FooTestFixture",
      "tests": 2,
      "testsuite": \[
        {
          "name": "Test3",
          "file": ".*gtest_list_output_unittest_.cc",
          "line": 48
        },
        {
          "name": "Test4",
          "file": ".*gtest_list_output_unittest_.cc",
          "line": 49
        }
      \]
    },
    {
      "name": "TypedTest\\\\/0",
      "tests": 2,
      "testsuite": \[
        {
          "name": "Test7",
          "type_param": "int",
          "file": ".*gtest_list_output_unittest_.cc",
          "line": 61
        },
        {
          "name": "Test8",
          "type_param": "int",
          "file": ".*gtest_list_output_unittest_.cc",
          "line": 62
        }
      \]
    },
    {
      "name": "TypedTest\\\\/1",
      "tests": 2,
      "testsuite": \[
        {
          "name": "Test7",
          "type_param": "bool",
          "file": ".*gtest_list_output_unittest_.cc",
          "line": 61
        },
        {
          "name": "Test8",
          "type_param": "bool",
          "file": ".*gtest_list_output_unittest_.cc",
          "line": 62
        }
      \]
    },
    {
      "name": "Single\\\\/TypeParameterizedTestSuite\\\\/0",
      "tests": 2,
      "testsuite": \[
        {
          "name": "Test9",
          "type_param": "int",
          "file": ".*gtest_list_output_unittest_.cc",
          "line": 69
        },
        {
          "name": "Test10",
          "type_param": "int",
          "file": ".*gtest_list_output_unittest_.cc",
          "line": 70
        }
      \]
    },
    {
      "name": "Single\\\\/TypeParameterizedTestSuite\\\\/1",
      "tests": 2,
      "testsuite": \[
        {
          "name": "Test9",
          "type_param": "bool",
          "file": ".*gtest_list_output_unittest_.cc",
          "line": 69
        },
        {
          "name": "Test10",
          "type_param": "bool",
          "file": ".*gtest_list_output_unittest_.cc",
          "line": 70
        }
      \]
    },
    {
      "name": "ValueParam\\\\/ValueParamTest",
      "tests": 4,
      "testsuite": \[
        {
          "name": "Test5\\\\/0",
          "value_param": "33",
          "file": ".*gtest_list_output_unittest_.cc",
          "line": 52
        },
        {
          "name": "Test5\\\\/1",
          "value_param": "42",
          "file": ".*gtest_list_output_unittest_.cc",
          "line": 52
        },
        {
          "name": "Test6\\\\/0",
          "value_param": "33",
          "file": ".*gtest_list_output_unittest_.cc",
          "line": 53
        },
        {
          "name": "Test6\\\\/1",
          "value_param": "42",
          "file": ".*gtest_list_output_unittest_.cc",
          "line": 53
        }
      \]
    }
  \]
}
"""


class GTestListTestsOutputUnitTest(gtest_test_utils.TestCase):
  """Unit test for Google Test's list tests with output to file functionality.
  """

  def testXml(self):
    """Verifies XML output for listing tests in a Google Test binary.

    Runs a test program that generates an empty XML output, and
    tests that the XML output is expected.
    """
    self._TestOutput('xml', EXPECTED_XML)

  def testJSON(self):
    """Verifies XML output for listing tests in a Google Test binary.

    Runs a test program that generates an empty XML output, and
    tests that the XML output is expected.
    """
    self._TestOutput('json', EXPECTED_JSON)

  def _GetOutput(self, out_format):
    file_path = os.path.join(gtest_test_utils.GetTempDir(),
                             'test_out.' + out_format)
    gtest_prog_path = gtest_test_utils.GetTestExecutablePath(
        'gtest_list_output_unittest_')

    command = ([
        gtest_prog_path,
        '%s=%s:%s' % (GTEST_OUTPUT_FLAG, out_format, file_path),
        '--gtest_list_tests'
    ])
    environ_copy = os.environ.copy()
    p = gtest_test_utils.Subprocess(
        command, env=environ_copy, working_dir=gtest_test_utils.GetTempDir())

    self.assertTrue(p.exited)
    self.assertEqual(0, p.exit_code)
    self.assertTrue(os.path.isfile(file_path))
    with open(file_path) as f:
      result = f.read()
    return result

  def _TestOutput(self, test_format, expected_output):
    actual = self._GetOutput(test_format)
    actual_lines = actual.splitlines()
    expected_lines = expected_output.splitlines()
    line_count = 0
    for actual_line in actual_lines:
      expected_line = expected_lines[line_count]
      expected_line_re = re.compile(expected_line.strip())
      self.assertTrue(
          expected_line_re.match(actual_line.strip()),
          ('actual output of "%s",\n'
           'which does not match expected regex of "%s"\n'
           'on line %d' % (actual, expected_output, line_count)))
      line_count = line_count + 1


if __name__ == '__main__':
  os.environ['GTEST_STACK_TRACE_DEPTH'] = '1'
  gtest_test_utils.Main()
