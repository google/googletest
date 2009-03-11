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

"""Unit test utilities for Google C++ Testing Framework."""

__author__ = 'wan@google.com (Zhanyong Wan)'

import os
import sys
import unittest

try:
  import subprocess
  _SUBPROCESS_MODULE_AVAILABLE = True
except:
  import popen2
  _SUBPROCESS_MODULE_AVAILABLE = False


# Initially maps a flag to its default value.  After
# _ParseAndStripGTestFlags() is called, maps a flag to its actual
# value.
_flag_map = {'gtest_source_dir': os.path.dirname(sys.argv[0]),
             'gtest_build_dir': os.path.dirname(sys.argv[0])}
_gtest_flags_are_parsed = False


def _ParseAndStripGTestFlags(argv):
  """Parses and strips Google Test flags from argv.  This is idempotent."""

  global _gtest_flags_are_parsed
  if _gtest_flags_are_parsed:
    return

  _gtest_flags_are_parsed = True
  for flag in _flag_map:
    # The environment variable overrides the default value.
    if flag.upper() in os.environ:
      _flag_map[flag] = os.environ[flag.upper()]

    # The command line flag overrides the environment variable.
    i = 1  # Skips the program name.
    while i < len(argv):
      prefix = '--' + flag + '='
      if argv[i].startswith(prefix):
        _flag_map[flag] = argv[i][len(prefix):]
        del argv[i]
        break
      else:
        # We don't increment i in case we just found a --gtest_* flag
        # and removed it from argv.
        i += 1


def GetFlag(flag):
  """Returns the value of the given flag."""

  # In case GetFlag() is called before Main(), we always call
  # _ParseAndStripGTestFlags() here to make sure the --gtest_* flags
  # are parsed.
  _ParseAndStripGTestFlags(sys.argv)

  return _flag_map[flag]


def GetSourceDir():
  """Returns the absolute path of the directory where the .py files are."""

  return os.path.abspath(GetFlag('gtest_source_dir'))


def GetBuildDir():
  """Returns the absolute path of the directory where the test binaries are."""

  return os.path.abspath(GetFlag('gtest_build_dir'))


def GetExitStatus(exit_code):
  """Returns the argument to exit(), or -1 if exit() wasn't called.

  Args:
    exit_code: the result value of os.system(command).
  """

  if os.name == 'nt':
    # On Windows, os.WEXITSTATUS() doesn't work and os.system() returns
    # the argument to exit() directly.
    return exit_code
  else:
    # On Unix, os.WEXITSTATUS() must be used to extract the exit status
    # from the result of os.system().
    if os.WIFEXITED(exit_code):
      return os.WEXITSTATUS(exit_code)
    else:
      return -1


class Subprocess:
  def __init__(self, command, working_dir=None):
    """Changes into a specified directory, if provided, and executes a command.
    Restores the old directory afterwards. Execution results are returned
    via the following attributes:
      terminated_by_sygnal   True iff the child process has been terminated
                             by a signal.
      signal                 Sygnal that terminated the child process.
      exited                 True iff the child process exited normally.
      exit_code              The code with which the child proces exited.
      output                 Child process's stdout and stderr output
                             combined in a string.

    Args:
      command: A command to run, in the form of sys.argv.
      working_dir: A directory to change into.
    """

    # The subprocess module is the preferrable way of running programs
    # since it is available and behaves consistently on all platforms,
    # including Windows. But it is only available starting in python 2.4.
    # In earlier python versions, we revert to the popen2 module, which is
    # available in python 2.0 and later but doesn't provide required
    # functionality (Popen4) under Windows. This allows us to support Mac
    # OS X 10.4 Tiger, which has python 2.3 installed.
    if _SUBPROCESS_MODULE_AVAILABLE:
      p = subprocess.Popen(command,
                           stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                           cwd=working_dir, universal_newlines=True)
      # communicate returns a tuple with the file obect for the child's
      # output.
      self.output = p.communicate()[0]
      self._return_code = p.returncode
    else:
      old_dir = os.getcwd()
      try:
        if working_dir is not None:
          os.chdir(working_dir)
        p = popen2.Popen4(command)
        p.tochild.close()
        self.output = p.fromchild.read()
        ret_code = p.wait()
      finally:
        os.chdir(old_dir)
      # Converts ret_code to match the semantics of
      # subprocess.Popen.returncode.
      if os.WIFSIGNALED(ret_code):
        self._return_code = -os.WTERMSIG(ret_code)
      else:  # os.WIFEXITED(ret_code) should return True here.
        self._return_code = os.WEXITSTATUS(ret_code)

    if self._return_code < 0:
      self.terminated_by_signal = True
      self.exited = False
      self.signal = -self._return_code
    else:
      self.terminated_by_signal = False
      self.exited = True
      self.exit_code = self._return_code


def Main():
  """Runs the unit test."""

  # We must call _ParseAndStripGTestFlags() before calling
  # unittest.main().  Otherwise the latter will be confused by the
  # --gtest_* flags.
  _ParseAndStripGTestFlags(sys.argv)
  unittest.main()
