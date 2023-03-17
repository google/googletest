#!/usr/bin/env python
#
# (c)2023 Sony Interactive Entertainment Inc
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
#     * Neither the name of Sony Interactive Entertainment Inc. nor the
# names of its contributors may be used to endorse or promote products
# derived from this software without specific prior written permission.
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

r"""Tests output of RGT detection for Google C++ Testing and Mocking Framework.

To update the golden file:
gtest_rgt_output_test.py --build_dir=BUILD/DIR --gengolden
where BUILD/DIR contains the built gtest_rgt_output_test_ file.
"""

import difflib
import os
import re
import sys
from googletest.test import gtest_test_utils


# The flag for generating the golden file
GENGOLDEN_FLAG = '--gengolden'
CATCH_EXCEPTIONS_ENV_VAR_NAME = 'GTEST_CATCH_EXCEPTIONS'

IS_LINUX = os.name == 'posix' and os.uname()[0] == 'Linux'
IS_WINDOWS = os.name == 'nt'

GOLDEN_NAME = 'gtest_rgt_output_test_golden_lin.txt'

PROGRAM_PATH = gtest_test_utils.GetTestExecutablePath('gtest_rgt_output_test_')

COMMAND_LIST_TESTS = ({}, [PROGRAM_PATH, '--gtest_list_tests'])
COMMAND_WITH_COLOR = ({}, [PROGRAM_PATH, '--gtest_color=yes'])
COMMAND_WITH_RGT_PASSING = ({}, [PROGRAM_PATH,
                                 '--gtest_treat_rotten_as_pass'])

GOLDEN_PATH = os.path.join(gtest_test_utils.GetSourceDir(), GOLDEN_NAME)


def ToUnixLineEnding(s):
  """Changes all Windows/Mac line endings in s to UNIX line endings."""

  return s.replace('\r\n', '\n').replace('\r', '\n')


def RemoveFileLocations(test_output):
  """Removes all file location info from a Google Test program's output.

  Args:
       test_output:  the output of a Google Test program.

  Returns:
       output with all file location info (in the form of
       'DIRECTORY/FILE_NAME:LINE_NUMBER: 'or
       'DIRECTORY\\FILE_NAME(LINE_NUMBER): ') replaced by
       'FILE_NAME:LINE_NUMBER: '.
  """

  test_output = re.sub(r'.*[/\\](gtest_main.cc)', r'\1', test_output)
  return re.sub(r'.*[/\\]((gtest_rgt_output_test_|gtest).cc)[:(](\d+)\)?\: ',
                r'\1:\3: ', test_output)


def ObscureLineNumbers(output):
  """Removes line numbers for error, Skipped and Failure notices.

  Args:
       output:  the output of a Google Test program.

  Returns:
       output with all file location info (in the form of
       'FILE_NAME:LINE_NUMBER: (error|Failure|Skipped)' replaced by
       'FILE_NAME:#: (error|Failure|Skipped)'.
  """

  return re.sub(r'((gtest_rgt_output_test_|gtest).cc):\d+: (error|Failure|Skipped)',
                r'\1:#: \3', output)


def RemoveTime(output):
  """Removes all time information from a Google Test program's output."""

  return re.sub(r'\(\d+ ms', '(? ms', output)


def NormalizeToCurrentPlatform(test_output):
  """Normalizes platform specific output details for easier comparison."""

  if IS_WINDOWS:
    # Removes the color information that is not present on Windows.
    test_output = re.sub('\x1b\\[(0;3\d)?m', '', test_output)
    # Changes failure message headers into the Windows format.
    test_output = re.sub(r': Failure\n', r': error: ', test_output)
    # Changes file(line_number) to file:line_number.
    test_output = re.sub(r'((\w|\.)+)\((\d+)\):', r'\1:\3:', test_output)

  return test_output


def RemoveTestCounts(output):
  """Removes test counts from a Google Test program's output."""

  output = re.sub(r'\d+ tests?, listed below',
                  '? tests, listed below', output)
  output = re.sub(r'\d+ FAILED TESTS',
                  '? FAILED TESTS', output)
  output = re.sub(r'\d+ tests? from \d+ test cases?',
                  '? tests from ? test cases', output)
  output = re.sub(r'\d+ tests? from ([a-zA-Z_])',
                  r'? tests from \1', output)
  return re.sub(r'\d+ tests?\.', '? tests.', output)


def NormalizeOutput(output):
  """Normalizes output (the output of gtest_rgt_output_test_.exe)."""

  output = ToUnixLineEnding(output)
  output = RemoveFileLocations(output)
  output = ObscureLineNumbers(output)
  output = RemoveTime(output)
  return output


def GetShellCommandOutput(env_cmd):
  """Runs a command in a sub-process, and returns its output in a string.

  Args:
    env_cmd: The shell command. A 2-tuple where element 0 is a dict of extra
             environment variables to set, and element 1 is a string with
             the command and any flags.

  Returns:
    A string with the command's combined standard and diagnostic output.
  """

  # Spawns cmd in a sub-process, and gets its standard I/O file objects.
  # Set and save the environment properly.
  environ = os.environ.copy()
  environ.update(env_cmd[0])
  p = gtest_test_utils.Subprocess(env_cmd[1], env=environ)

  return p.output


def GetCommandOutput(env_cmd):
  """Runs a command and returns its output with all file location
  info stripped off.

  Args:
    env_cmd:  The shell command. A 2-tuple where element 0 is a dict of extra
              environment variables to set, and element 1 is a string with
              the command and any flags.
  """

  # Disables exception pop-ups on Windows.
  environ, cmdline = env_cmd
  environ = dict(environ)  # Ensures we are modifying a copy.
  environ[CATCH_EXCEPTIONS_ENV_VAR_NAME] = '1'
  return NormalizeOutput(GetShellCommandOutput((environ, cmdline)))


def GetOutputOfAllCommands():
  """Returns concatenated output from several representative commands."""

  return (GetCommandOutput(COMMAND_WITH_COLOR) +
          GetCommandOutput(COMMAND_WITH_RGT_PASSING))


test_list = GetShellCommandOutput(COMMAND_LIST_TESTS)
SUPPORTS_RGT = 'Rotten' in test_list

CAN_GENERATE_GOLDEN_FILE = (SUPPORTS_RGT)

class GTestOutputTest(gtest_test_utils.TestCase):
  def testOutput(self):
    output = GetOutputOfAllCommands()

    golden_file = open(GOLDEN_PATH, 'rb')
    # A mis-configured source control system can cause \r appear in EOL
    # sequences when we read the golden file irrespective of an operating
    # system used. Therefore, we need to strip those \r's from newlines
    # unconditionally.
    golden = ToUnixLineEnding(golden_file.read().decode())
    golden_file.close()

    # We want the test to pass regardless of certain features being
    # supported or not.

    # We still have to remove type name specifics in all cases.
    normalized_actual = NormalizeToCurrentPlatform(output)
    normalized_golden = NormalizeToCurrentPlatform(golden)

    if CAN_GENERATE_GOLDEN_FILE:
      self.assertEqual(normalized_golden, normalized_actual,
                       '\n'.join(difflib.unified_diff(
                           normalized_golden.split('\n'),
                           normalized_actual.split('\n'),
                           'golden', 'actual')))
    else:
      normalized_actual = NormalizeToCurrentPlatform(
          RemoveTestCounts(normalized_actual))
      normalized_golden = NormalizeToCurrentPlatform(
          RemoveTestCounts(normalized_golden))

      # This code is very handy when debugging golden file differences:
      if os.getenv('DEBUG_GTEST_OUTPUT_TEST'):
        with open(os.path.join(
            gtest_test_utils.GetSourceDir(),
            '_gtest_rgt_output_test_normalized_actual.txt'), 'wb') as f:
          f.write(normalized_actual.encode())
        with open(os.path.join(
            gtest_test_utils.GetSourceDir(),
            '_gtest_rgt_output_test_normalized_golden.txt'), 'wb') as f:
          f.write(normalized_golden.encode())

      self.assertEqual(normalized_golden, normalized_actual)


if __name__ == '__main__':
  if GENGOLDEN_FLAG in sys.argv:
    if CAN_GENERATE_GOLDEN_FILE:
      output = GetOutputOfAllCommands()
      golden_file = open(GOLDEN_PATH, 'wb')
      golden_file.write(output.encode())
      golden_file.close()
    else:
      message = (
          """Unable to write a golden file when compiled in an environment
that does not support all the required features (rotten green tests).
Please build this test and generate the golden file using Blaze on Linux.""")

      sys.stderr.write(message)
      sys.exit(1)
  else:
    gtest_test_utils.Main()
