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

"""Tests the text output of Google C++ Testing Framework.

SYNOPSIS
       gtest_output_test.py --gtest_build_dir=BUILD/DIR --gengolden
         # where BUILD/DIR contains the built gtest_output_test_ file.
       gtest_output_test.py --gengolden
       gtest_output_test.py
"""

__author__ = 'wan@google.com (Zhanyong Wan)'

import os
import re
import string
import sys
import unittest
import gtest_test_utils


# The flag for generating the golden file
GENGOLDEN_FLAG = '--gengolden'

IS_WINDOWS = os.name == 'nt'

if IS_WINDOWS:
  PROGRAM = r'..\build.dbg8\gtest_output_test_.exe'
  GOLDEN_NAME = 'gtest_output_test_golden_win.txt'
else:
  PROGRAM = 'gtest_output_test_'
  GOLDEN_NAME = 'gtest_output_test_golden_lin.txt'

PROGRAM_PATH = os.path.join(gtest_test_utils.GetBuildDir(), PROGRAM)
COMMAND_WITH_COLOR = PROGRAM_PATH + ' --gtest_color=yes'
COMMAND_WITH_TIME = (PROGRAM_PATH + ' --gtest_print_time '
                     + '--gtest_filter="FatalFailureTest.*:LoggingTest.*"')
COMMAND_WITH_DISABLED = (PROGRAM_PATH + ' --gtest_also_run_disabled_tests '
                         + '--gtest_filter="*DISABLED_*"')

GOLDEN_PATH = os.path.join(gtest_test_utils.GetSourceDir(),
                           GOLDEN_NAME)


def ToUnixLineEnding(s):
  """Changes all Windows/Mac line endings in s to UNIX line endings."""

  return s.replace('\r\n', '\n').replace('\r', '\n')


def RemoveLocations(output):
  """Removes all file location info from a Google Test program's output.

  Args:
       output:  the output of a Google Test program.

  Returns:
       output with all file location info (in the form of
       'DIRECTORY/FILE_NAME:LINE_NUMBER: 'or
       'DIRECTORY\\FILE_NAME(LINE_NUMBER): ') replaced by
       'FILE_NAME:#: '.
  """

  return re.sub(r'.*[/\\](.+)(\:\d+|\(\d+\))\: ', r'\1:#: ', output)


def RemoveStackTraces(output):
  """Removes all stack traces from a Google Test program's output."""

  # *? means "find the shortest string that matches".
  return re.sub(r'Stack trace:(.|\n)*?\n\n',
                'Stack trace: (omitted)\n\n', output)


def RemoveTime(output):
  """Removes all time information from a Google Test program's output."""

  return re.sub(r'\(\d+ ms', '(? ms', output)


def RemoveTestCounts(output):
  """Removes test counts from a Google Test program's output."""

  output = re.sub(r'\d+ tests from \d+ test cases',
                  '? tests from ? test cases', output)
  return re.sub(r'\d+ tests\.', '? tests.', output)


def RemoveDeathTests(output):
  """Removes death test information from a Google Test program's output."""

  return re.sub(r'\n.*DeathTest.*', '', output)


def NormalizeOutput(output):
  """Normalizes output (the output of gtest_output_test_.exe)."""

  output = ToUnixLineEnding(output)
  output = RemoveLocations(output)
  output = RemoveStackTraces(output)
  output = RemoveTime(output)
  return output


def IterShellCommandOutput(cmd, stdin_string=None):
  """Runs a command in a sub-process, and iterates the lines in its STDOUT.

  Args:

    cmd:           The shell command.
    stdin_string:  The string to be fed to the STDIN of the sub-process;
                   If None, the sub-process will inherit the STDIN
                   from the parent process.
  """

  # Spawns cmd in a sub-process, and gets its standard I/O file objects.
  stdin_file, stdout_file = os.popen2(cmd, 'b')

  # If the caller didn't specify a string for STDIN, gets it from the
  # parent process.
  if stdin_string is None:
    stdin_string = sys.stdin.read()

  # Feeds the STDIN string to the sub-process.
  stdin_file.write(stdin_string)
  stdin_file.close()

  while True:
    line = stdout_file.readline()
    if not line:  # EOF
      stdout_file.close()
      break

    yield line


def GetShellCommandOutput(cmd, stdin_string=None):
  """Runs a command in a sub-process, and returns its STDOUT in a string.

  Args:

    cmd:           The shell command.
    stdin_string:  The string to be fed to the STDIN of the sub-process;
                   If None, the sub-process will inherit the STDIN
                   from the parent process.
  """

  lines = list(IterShellCommandOutput(cmd, stdin_string))
  return string.join(lines, '')


def GetCommandOutput(cmd):
  """Runs a command and returns its output with all file location
  info stripped off.

  Args:
    cmd:  the shell command.
  """

  # Disables exception pop-ups on Windows.
  os.environ['GTEST_CATCH_EXCEPTIONS'] = '1'
  return NormalizeOutput(GetShellCommandOutput(cmd, ''))


class GTestOutputTest(unittest.TestCase):
  def testOutput(self):
    output = (GetCommandOutput(COMMAND_WITH_COLOR) +
              GetCommandOutput(COMMAND_WITH_TIME) +
              GetCommandOutput(COMMAND_WITH_DISABLED))
    golden_file = open(GOLDEN_PATH, 'rb')
    golden = golden_file.read()
    golden_file.close()

    # We want the test to pass regardless of death tests being
    # supported or not.
    self.assert_(output == golden or
                 RemoveTestCounts(output) ==
                 RemoveTestCounts(RemoveDeathTests(golden)))


if __name__ == '__main__':
  if sys.argv[1:] == [GENGOLDEN_FLAG]:
    output = (GetCommandOutput(COMMAND_WITH_COLOR) +
              GetCommandOutput(COMMAND_WITH_TIME) +
              GetCommandOutput(COMMAND_WITH_DISABLED))
    golden_file = open(GOLDEN_PATH, 'wb')
    golden_file.write(output)
    golden_file.close()
  else:
    gtest_test_utils.Main()
