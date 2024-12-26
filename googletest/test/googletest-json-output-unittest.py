#!/usr/bin/env python
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

"""Unit test for the gtest_json_output module."""

import datetime
import errno
import json
import os
import re
import sys

from googletest.test import gtest_json_test_utils
from googletest.test import gtest_test_utils

GTEST_FILTER_FLAG = '--gtest_filter'
GTEST_LIST_TESTS_FLAG = '--gtest_list_tests'
GTEST_OUTPUT_FLAG = '--gtest_output'
GTEST_DEFAULT_OUTPUT_FILE = 'test_detail.json'
GTEST_PROGRAM_NAME = 'gtest_xml_output_unittest_'

# The flag indicating stacktraces are not supported
NO_STACKTRACE_SUPPORT_FLAG = '--no_stacktrace_support'

SUPPORTS_STACK_TRACES = NO_STACKTRACE_SUPPORT_FLAG not in sys.argv

if SUPPORTS_STACK_TRACES:
  STACK_TRACE_TEMPLATE = '\nStack trace:\n*'
else:
  STACK_TRACE_TEMPLATE = '\n'

EXPECTED_NON_EMPTY = {
    'tests': 28,
    'failures': 5,
    'disabled': 2,
    'errors': 0,
    'timestamp': '*',
    'time': '*',
    'ad_hoc_property': '42',
    'name': 'AllTests',
    'testsuites': [
        {
            'name': 'SuccessfulTest',
            'tests': 1,
            'failures': 0,
            'disabled': 0,
            'errors': 0,
            'time': '*',
            'timestamp': '*',
            'testsuite': [{
                'name': 'Succeeds',
                'file': 'gtest_xml_output_unittest_.cc',
                'line': 53,
                'status': 'RUN',
                'result': 'COMPLETED',
                'time': '*',
                'timestamp': '*',
                'classname': 'SuccessfulTest',
            }],
        },
        {
            'name': 'FailedTest',
            'tests': 1,
            'failures': 1,
            'disabled': 0,
            'errors': 0,
            'time': '*',
            'timestamp': '*',
            'testsuite': [{
                'name': 'Fails',
                'file': 'gtest_xml_output_unittest_.cc',
                'line': 61,
                'status': 'RUN',
                'result': 'COMPLETED',
                'time': '*',
                'timestamp': '*',
                'classname': 'FailedTest',
                'failures': [{
                    'failure': (
                        'gtest_xml_output_unittest_.cc:*\n'
                        'Expected equality of these values:\n'
                        '  1\n  2'
                        + STACK_TRACE_TEMPLATE
                    ),
                    'type': '',
                }],
            }],
        },
        {
            'name': 'DisabledTest',
            'tests': 1,
            'failures': 0,
            'disabled': 1,
            'errors': 0,
            'time': '*',
            'timestamp': '*',
            'testsuite': [{
                'name': 'DISABLED_test_not_run',
                'file': 'gtest_xml_output_unittest_.cc',
                'line': 68,
                'status': 'NOTRUN',
                'result': 'SUPPRESSED',
                'time': '*',
                'timestamp': '*',
                'classname': 'DisabledTest',
            }],
        },
        {
            'name': 'SkippedTest',
            'tests': 3,
            'failures': 1,
            'disabled': 0,
            'errors': 0,
            'time': '*',
            'timestamp': '*',
            'testsuite': [
                {
                    'name': 'Skipped',
                    'file': 'gtest_xml_output_unittest_.cc',
                    'line': 75,
                    'status': 'RUN',
                    'result': 'SKIPPED',
                    'time': '*',
                    'timestamp': '*',
                    'classname': 'SkippedTest',
                    'skipped': [
                        {'message': 'gtest_xml_output_unittest_.cc:*\n\n'}
                    ],
                },
                {
                    'name': 'SkippedWithMessage',
                    'file': 'gtest_xml_output_unittest_.cc',
                    'line': 79,
                    'status': 'RUN',
                    'result': 'SKIPPED',
                    'time': '*',
                    'timestamp': '*',
                    'classname': 'SkippedTest',
                    'skipped': [{
                        'message': (
                            'gtest_xml_output_unittest_.cc:*\n'
                            'It is good practice to tell why you skip a test.\n'
                        )
                    }],
                },
                {
                    'name': 'SkippedAfterFailure',
                    'file': 'gtest_xml_output_unittest_.cc',
                    'line': 83,
                    'status': 'RUN',
                    'result': 'COMPLETED',
                    'time': '*',
                    'timestamp': '*',
                    'classname': 'SkippedTest',
                    'failures': [{
                        'failure': (
                            'gtest_xml_output_unittest_.cc:*\n'
                            'Expected equality of these values:\n'
                            '  1\n  2'
                            + STACK_TRACE_TEMPLATE
                        ),
                        'type': '',
                    }],
                    'skipped': [{
                        'message': (
                            'gtest_xml_output_unittest_.cc:*\n'
                            'It is good practice to tell why you skip a test.\n'
                        )
                    }],
                },
            ],
        },
        {
            'name': 'MixedResultTest',
            'tests': 3,
            'failures': 1,
            'disabled': 1,
            'errors': 0,
            'time': '*',
            'timestamp': '*',
            'testsuite': [
                {
                    'name': 'Succeeds',
                    'file': 'gtest_xml_output_unittest_.cc',
                    'line': 88,
                    'status': 'RUN',
                    'result': 'COMPLETED',
                    'time': '*',
                    'timestamp': '*',
                    'classname': 'MixedResultTest',
                },
                {
                    'name': 'Fails',
                    'file': 'gtest_xml_output_unittest_.cc',
                    'line': 93,
                    'status': 'RUN',
                    'result': 'COMPLETED',
                    'time': '*',
                    'timestamp': '*',
                    'classname': 'MixedResultTest',
                    'failures': [
                        {
                            'failure': (
                                'gtest_xml_output_unittest_.cc:*\n'
                                'Expected equality of these values:\n'
                                '  1\n  2'
                                + STACK_TRACE_TEMPLATE
                            ),
                            'type': '',
                        },
                        {
                            'failure': (
                                'gtest_xml_output_unittest_.cc:*\n'
                                'Expected equality of these values:\n'
                                '  2\n  3'
                                + STACK_TRACE_TEMPLATE
                            ),
                            'type': '',
                        },
                    ],
                },
                {
                    'name': 'DISABLED_test',
                    'file': 'gtest_xml_output_unittest_.cc',
                    'line': 98,
                    'status': 'NOTRUN',
                    'result': 'SUPPRESSED',
                    'time': '*',
                    'timestamp': '*',
                    'classname': 'MixedResultTest',
                },
            ],
        },
        {
            'name': 'XmlQuotingTest',
            'tests': 1,
            'failures': 1,
            'disabled': 0,
            'errors': 0,
            'time': '*',
            'timestamp': '*',
            'testsuite': [{
                'name': 'OutputsCData',
                'file': 'gtest_xml_output_unittest_.cc',
                'line': 102,
                'status': 'RUN',
                'result': 'COMPLETED',
                'time': '*',
                'timestamp': '*',
                'classname': 'XmlQuotingTest',
                'failures': [{
                    'failure': (
                        'gtest_xml_output_unittest_.cc:*\n'
                        'Failed\nXML output: <?xml encoding="utf-8">'
                        '<top><![CDATA[cdata text]]></top>'
                        + STACK_TRACE_TEMPLATE
                    ),
                    'type': '',
                }],
            }],
        },
        {
            'name': 'InvalidCharactersTest',
            'tests': 1,
            'failures': 1,
            'disabled': 0,
            'errors': 0,
            'time': '*',
            'timestamp': '*',
            'testsuite': [{
                'name': 'InvalidCharactersInMessage',
                'file': 'gtest_xml_output_unittest_.cc',
                'line': 109,
                'status': 'RUN',
                'result': 'COMPLETED',
                'time': '*',
                'timestamp': '*',
                'classname': 'InvalidCharactersTest',
                'failures': [{
                    'failure': (
                        'gtest_xml_output_unittest_.cc:*\n'
                        'Failed\nInvalid characters in brackets'
                        ' [\x01\x02]'
                        + STACK_TRACE_TEMPLATE
                    ),
                    'type': '',
                }],
            }],
        },
        {
            'name': 'PropertyRecordingTest',
            'tests': 4,
            'failures': 0,
            'disabled': 0,
            'errors': 0,
            'time': '*',
            'timestamp': '*',
            'SetUpTestSuite': 'yes',
            'TearDownTestSuite': 'aye',
            'testsuite': [
                {
                    'name': 'OneProperty',
                    'file': 'gtest_xml_output_unittest_.cc',
                    'line': 121,
                    'status': 'RUN',
                    'result': 'COMPLETED',
                    'time': '*',
                    'timestamp': '*',
                    'classname': 'PropertyRecordingTest',
                    'key_1': '1',
                },
                {
                    'name': 'IntValuedProperty',
                    'file': 'gtest_xml_output_unittest_.cc',
                    'line': 125,
                    'status': 'RUN',
                    'result': 'COMPLETED',
                    'time': '*',
                    'timestamp': '*',
                    'classname': 'PropertyRecordingTest',
                    'key_int': '1',
                },
                {
                    'name': 'ThreeProperties',
                    'file': 'gtest_xml_output_unittest_.cc',
                    'line': 129,
                    'status': 'RUN',
                    'result': 'COMPLETED',
                    'time': '*',
                    'timestamp': '*',
                    'classname': 'PropertyRecordingTest',
                    'key_1': '1',
                    'key_2': '2',
                    'key_3': '3',
                },
                {
                    'name': 'TwoValuesForOneKeyUsesLastValue',
                    'file': 'gtest_xml_output_unittest_.cc',
                    'line': 135,
                    'status': 'RUN',
                    'result': 'COMPLETED',
                    'time': '*',
                    'timestamp': '*',
                    'classname': 'PropertyRecordingTest',
                    'key_1': '2',
                },
            ],
        },
        {
            'name': 'NoFixtureTest',
            'tests': 3,
            'failures': 0,
            'disabled': 0,
            'errors': 0,
            'time': '*',
            'timestamp': '*',
            'testsuite': [
                {
                    'name': 'RecordProperty',
                    'file': 'gtest_xml_output_unittest_.cc',
                    'line': 140,
                    'status': 'RUN',
                    'result': 'COMPLETED',
                    'time': '*',
                    'timestamp': '*',
                    'classname': 'NoFixtureTest',
                    'key': '1',
                },
                {
                    'name': 'ExternalUtilityThatCallsRecordIntValuedProperty',
                    'file': 'gtest_xml_output_unittest_.cc',
                    'line': 153,
                    'status': 'RUN',
                    'result': 'COMPLETED',
                    'time': '*',
                    'timestamp': '*',
                    'classname': 'NoFixtureTest',
                    'key_for_utility_int': '1',
                },
                {
                    'name': (
                        'ExternalUtilityThatCallsRecordStringValuedProperty'
                    ),
                    'file': 'gtest_xml_output_unittest_.cc',
                    'line': 157,
                    'status': 'RUN',
                    'result': 'COMPLETED',
                    'time': '*',
                    'timestamp': '*',
                    'classname': 'NoFixtureTest',
                    'key_for_utility_string': '1',
                },
            ],
        },
        {
            'name': 'SetupFailTest',
            'tests': 1,
            'failures': 0,
            'disabled': 0,
            'errors': 0,
            'time': '*',
            'timestamp': '*',
            'testsuite': [
                {
                    'name': 'NoopPassingTest',
                    'file': 'gtest_xml_output_unittest_.cc',
                    'line': 168,
                    'status': 'RUN',
                    'result': 'SKIPPED',
                    'timestamp': '*',
                    'time': '*',
                    'classname': 'SetupFailTest',
                    'skipped': [
                        {'message': 'gtest_xml_output_unittest_.cc:*\n'}
                    ],
                },
                {
                    'name': '',
                    'status': 'RUN',
                    'result': 'COMPLETED',
                    'timestamp': '*',
                    'time': '*',
                    'classname': '',
                    'failures': [{
                        'failure': (
                            'gtest_xml_output_unittest_.cc:*\nExpected equality'
                            ' of these values:\n  1\n  2'
                            + STACK_TRACE_TEMPLATE
                        ),
                        'type': '',
                    }],
                },
            ],
        },
        {
            'name': 'TearDownFailTest',
            'tests': 1,
            'failures': 0,
            'disabled': 0,
            'errors': 0,
            'timestamp': '*',
            'time': '*',
            'testsuite': [
                {
                    'name': 'NoopPassingTest',
                    'file': 'gtest_xml_output_unittest_.cc',
                    'line': 175,
                    'status': 'RUN',
                    'result': 'COMPLETED',
                    'timestamp': '*',
                    'time': '*',
                    'classname': 'TearDownFailTest',
                },
                {
                    'name': '',
                    'status': 'RUN',
                    'result': 'COMPLETED',
                    'timestamp': '*',
                    'time': '*',
                    'classname': '',
                    'failures': [{
                        'failure': (
                            'gtest_xml_output_unittest_.cc:*\nExpected equality'
                            ' of these values:\n  1\n  2'
                            + STACK_TRACE_TEMPLATE
                        ),
                        'type': '',
                    }],
                },
            ],
        },
        {
            'name': 'TypedTest/0',
            'tests': 1,
            'failures': 0,
            'disabled': 0,
            'errors': 0,
            'time': '*',
            'timestamp': '*',
            'testsuite': [{
                'name': 'HasTypeParamAttribute',
                'type_param': 'int',
                'file': 'gtest_xml_output_unittest_.cc',
                'line': 189,
                'status': 'RUN',
                'result': 'COMPLETED',
                'time': '*',
                'timestamp': '*',
                'classname': 'TypedTest/0',
            }],
        },
        {
            'name': 'TypedTest/1',
            'tests': 1,
            'failures': 0,
            'disabled': 0,
            'errors': 0,
            'time': '*',
            'timestamp': '*',
            'testsuite': [{
                'name': 'HasTypeParamAttribute',
                'type_param': 'long',
                'file': 'gtest_xml_output_unittest_.cc',
                'line': 189,
                'status': 'RUN',
                'result': 'COMPLETED',
                'time': '*',
                'timestamp': '*',
                'classname': 'TypedTest/1',
            }],
        },
        {
            'name': 'Single/TypeParameterizedTestSuite/0',
            'tests': 1,
            'failures': 0,
            'disabled': 0,
            'errors': 0,
            'time': '*',
            'timestamp': '*',
            'testsuite': [{
                'name': 'HasTypeParamAttribute',
                'type_param': 'int',
                'file': 'gtest_xml_output_unittest_.cc',
                'line': 196,
                'status': 'RUN',
                'result': 'COMPLETED',
                'time': '*',
                'timestamp': '*',
                'classname': 'Single/TypeParameterizedTestSuite/0',
            }],
        },
        {
            'name': 'Single/TypeParameterizedTestSuite/1',
            'tests': 1,
            'failures': 0,
            'disabled': 0,
            'errors': 0,
            'time': '*',
            'timestamp': '*',
            'testsuite': [{
                'name': 'HasTypeParamAttribute',
                'type_param': 'long',
                'file': 'gtest_xml_output_unittest_.cc',
                'line': 196,
                'status': 'RUN',
                'result': 'COMPLETED',
                'time': '*',
                'timestamp': '*',
                'classname': 'Single/TypeParameterizedTestSuite/1',
            }],
        },
        {
            'name': 'Single/ValueParamTest',
            'tests': 4,
            'failures': 0,
            'disabled': 0,
            'errors': 0,
            'time': '*',
            'timestamp': '*',
            'testsuite': [
                {
                    'name': 'HasValueParamAttribute/0',
                    'value_param': '33',
                    'file': 'gtest_xml_output_unittest_.cc',
                    'line': 180,
                    'status': 'RUN',
                    'result': 'COMPLETED',
                    'time': '*',
                    'timestamp': '*',
                    'classname': 'Single/ValueParamTest',
                },
                {
                    'name': 'HasValueParamAttribute/1',
                    'value_param': '42',
                    'file': 'gtest_xml_output_unittest_.cc',
                    'line': 180,
                    'status': 'RUN',
                    'result': 'COMPLETED',
                    'time': '*',
                    'timestamp': '*',
                    'classname': 'Single/ValueParamTest',
                },
                {
                    'name': 'AnotherTestThatHasValueParamAttribute/0',
                    'value_param': '33',
                    'file': 'gtest_xml_output_unittest_.cc',
                    'line': 181,
                    'status': 'RUN',
                    'result': 'COMPLETED',
                    'time': '*',
                    'timestamp': '*',
                    'classname': 'Single/ValueParamTest',
                },
                {
                    'name': 'AnotherTestThatHasValueParamAttribute/1',
                    'value_param': '42',
                    'file': 'gtest_xml_output_unittest_.cc',
                    'line': 181,
                    'status': 'RUN',
                    'result': 'COMPLETED',
                    'time': '*',
                    'timestamp': '*',
                    'classname': 'Single/ValueParamTest',
                },
            ],
        },
    ],
}

EXPECTED_FILTERED = {
    'tests': 1,
    'failures': 0,
    'disabled': 0,
    'errors': 0,
    'time': '*',
    'timestamp': '*',
    'name': 'AllTests',
    'ad_hoc_property': '42',
    'testsuites': [{
        'name': 'SuccessfulTest',
        'tests': 1,
        'failures': 0,
        'disabled': 0,
        'errors': 0,
        'time': '*',
        'timestamp': '*',
        'testsuite': [{
            'name': 'Succeeds',
            'file': 'gtest_xml_output_unittest_.cc',
            'line': 53,
            'status': 'RUN',
            'result': 'COMPLETED',
            'time': '*',
            'timestamp': '*',
            'classname': 'SuccessfulTest',
        }],
    }],
}

EXPECTED_NO_TEST = {
    'tests': 0,
    'failures': 0,
    'disabled': 0,
    'errors': 0,
    'time': '*',
    'timestamp': '*',
    'name': 'AllTests',
    'testsuites': [{
        'name': 'NonTestSuiteFailure',
        'tests': 1,
        'failures': 1,
        'disabled': 0,
        'skipped': 0,
        'errors': 0,
        'time': '*',
        'timestamp': '*',
        'testsuite': [{
            'name': '',
            'status': 'RUN',
            'result': 'COMPLETED',
            'time': '*',
            'timestamp': '*',
            'classname': '',
            'failures': [{
                'failure': (
                    'gtest_no_test_unittest.cc:*\n'
                    'Expected equality of these values:\n'
                    '  1\n  2'
                    + STACK_TRACE_TEMPLATE
                ),
                'type': '',
            }],
        }],
    }],
}

GTEST_PROGRAM_PATH = gtest_test_utils.GetTestExecutablePath(GTEST_PROGRAM_NAME)

SUPPORTS_TYPED_TESTS = (
    'TypedTest'
    in gtest_test_utils.Subprocess(
        [GTEST_PROGRAM_PATH, GTEST_LIST_TESTS_FLAG], capture_stderr=False
    ).output
)


class GTestJsonOutputUnitTest(gtest_test_utils.TestCase):
  """Unit test for Google Test's JSON output functionality."""

  # This test currently breaks on platforms that do not support typed and
  # type-parameterized tests, so we don't run it under them.
  if SUPPORTS_TYPED_TESTS:

    def testNonEmptyJsonOutput(self):
      """Verifies JSON output for a Google Test binary with non-empty output.

      Runs a test program that generates a non-empty JSON output, and
      tests that the JSON output is expected.
      """
      self._TestJsonOutput(GTEST_PROGRAM_NAME, EXPECTED_NON_EMPTY, 1)

  def testNoTestJsonOutput(self):
    """Verifies JSON output for a Google Test binary without actual tests.

    Runs a test program that generates an JSON output for a binary with no
    tests, and tests that the JSON output is expected.
    """

    self._TestJsonOutput('gtest_no_test_unittest', EXPECTED_NO_TEST, 0)

  def testTimestampValue(self):
    """Checks whether the timestamp attribute in the JSON output is valid.

    Runs a test program that generates an empty JSON output, and checks if
    the timestamp attribute in the testsuites tag is valid.
    """
    actual = self._GetJsonOutput('gtest_no_test_unittest', [], 0)
    date_time_str = actual['timestamp']
    # datetime.strptime() is only available in Python 2.5+ so we have to
    # parse the expected datetime manually.
    match = re.match(r'(\d+)-(\d\d)-(\d\d)T(\d\d):(\d\d):(\d\d)', date_time_str)
    self.assertTrue(
        re.match,
        'JSON datettime string %s has incorrect format' % date_time_str,
    )
    date_time_from_json = datetime.datetime(
        year=int(match.group(1)),
        month=int(match.group(2)),
        day=int(match.group(3)),
        hour=int(match.group(4)),
        minute=int(match.group(5)),
        second=int(match.group(6)),
    )

    time_delta = abs(datetime.datetime.now() - date_time_from_json)
    # timestamp value should be near the current local time
    self.assertTrue(
        time_delta < datetime.timedelta(seconds=600),
        'time_delta is %s' % time_delta,
    )

  def testDefaultOutputFile(self):
    """Verifies the default output file name.

    Confirms that Google Test produces an JSON output file with the expected
    default name if no name is explicitly specified.
    """
    output_file = os.path.join(
        gtest_test_utils.GetTempDir(), GTEST_DEFAULT_OUTPUT_FILE
    )
    gtest_prog_path = gtest_test_utils.GetTestExecutablePath(
        'gtest_no_test_unittest'
    )
    try:
      os.remove(output_file)
    except OSError:
      e = sys.exc_info()[1]
      if e.errno != errno.ENOENT:
        raise

    p = gtest_test_utils.Subprocess(
        [gtest_prog_path, '%s=json' % GTEST_OUTPUT_FLAG],
        working_dir=gtest_test_utils.GetTempDir(),
    )
    self.assertTrue(p.exited)
    self.assertEqual(0, p.exit_code)
    self.assertTrue(os.path.isfile(output_file))

  def testSuppressedJsonOutput(self):
    """Verifies that no JSON output is generated.

    Tests that no JSON file is generated if the default JSON listener is
    shut down before RUN_ALL_TESTS is invoked.
    """

    json_path = os.path.join(
        gtest_test_utils.GetTempDir(), GTEST_PROGRAM_NAME + 'out.json'
    )
    if os.path.isfile(json_path):
      os.remove(json_path)

    command = [
        GTEST_PROGRAM_PATH,
        '%s=json:%s' % (GTEST_OUTPUT_FLAG, json_path),
        '--shut_down_xml',
    ]
    p = gtest_test_utils.Subprocess(command)
    if p.terminated_by_signal:
      # p.signal is available only if p.terminated_by_signal is True.
      self.assertFalse(
          p.terminated_by_signal,
          '%s was killed by signal %d' % (GTEST_PROGRAM_NAME, p.signal),
      )
    else:
      self.assertTrue(p.exited)
      self.assertEqual(
          1,
          p.exit_code,
          "'%s' exited with code %s, which doesn't match "
          'the expected exit code %s.' % (command, p.exit_code, 1),
      )

    self.assertTrue(not os.path.isfile(json_path))

  def testFilteredTestJsonOutput(self):
    """Verifies JSON output when a filter is applied.

    Runs a test program that executes only some tests and verifies that
    non-selected tests do not show up in the JSON output.
    """

    self._TestJsonOutput(
        GTEST_PROGRAM_NAME,
        EXPECTED_FILTERED,
        0,
        extra_args=['%s=SuccessfulTest.*' % GTEST_FILTER_FLAG],
    )

  def _GetJsonOutput(self, gtest_prog_name, extra_args, expected_exit_code):
    """Returns the JSON output generated by running the program gtest_prog_name.

    Furthermore, the program's exit code must be expected_exit_code.

    Args:
      gtest_prog_name: Google Test binary name.
      extra_args: extra arguments to binary invocation.
      expected_exit_code: program's exit code.
    """
    json_path = os.path.join(
        gtest_test_utils.GetTempDir(), gtest_prog_name + 'out.json'
    )
    gtest_prog_path = gtest_test_utils.GetTestExecutablePath(gtest_prog_name)

    command = [
        gtest_prog_path,
        '%s=json:%s' % (GTEST_OUTPUT_FLAG, json_path),
    ] + extra_args
    p = gtest_test_utils.Subprocess(command)
    if p.terminated_by_signal:
      self.assertTrue(
          False, '%s was killed by signal %d' % (gtest_prog_name, p.signal)
      )
    else:
      self.assertTrue(p.exited)
      self.assertEqual(
          expected_exit_code,
          p.exit_code,
          "'%s' exited with code %s, which doesn't match "
          'the expected exit code %s.'
          % (command, p.exit_code, expected_exit_code),
      )
    with open(json_path) as f:
      actual = json.load(f)
    return actual

  def _TestJsonOutput(
      self, gtest_prog_name, expected, expected_exit_code, extra_args=None
  ):
    """Checks the JSON output generated by the Google Test binary.

    Asserts that the JSON document generated by running the program
    gtest_prog_name matches expected_json, a string containing another
    JSON document.  Furthermore, the program's exit code must be
    expected_exit_code.

    Args:
      gtest_prog_name: Google Test binary name.
      expected: expected output.
      expected_exit_code: program's exit code.
      extra_args: extra arguments to binary invocation.
    """

    actual = self._GetJsonOutput(
        gtest_prog_name, extra_args or [], expected_exit_code
    )
    self.assertEqual(expected, gtest_json_test_utils.normalize(actual))


if __name__ == '__main__':
  if NO_STACKTRACE_SUPPORT_FLAG in sys.argv:
    # unittest.main() can't handle unknown flags
    sys.argv.remove(NO_STACKTRACE_SUPPORT_FLAG)

  os.environ['GTEST_STACK_TRACE_DEPTH'] = '1'
  gtest_test_utils.Main()
