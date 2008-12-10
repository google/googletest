#!/usr/bin/python2.4
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

"""Unit test utilities for Google C++ Mocking Framework."""

__author__ = 'wan@google.com (Zhanyong Wan)'

import os
import sys
import unittest


# Initially maps a flag to its default value.  After
# _ParseAndStripGMockFlags() is called, maps a flag to its actual
# value.
_flag_map = {'gmock_source_dir': os.path.dirname(sys.argv[0]),
             'gmock_build_dir': os.path.dirname(sys.argv[0])}
_gmock_flags_are_parsed = False


def _ParseAndStripGMockFlags(argv):
  """Parses and strips Google Test flags from argv.  This is idempotent."""

  global _gmock_flags_are_parsed
  if _gmock_flags_are_parsed:
    return

  _gmock_flags_are_parsed = True
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
        # We don't increment i in case we just found a --gmock_* flag
        # and removed it from argv.
        i += 1


def GetFlag(flag):
  """Returns the value of the given flag."""

  # In case GetFlag() is called before Main(), we always call
  # _ParseAndStripGMockFlags() here to make sure the --gmock_* flags
  # are parsed.
  _ParseAndStripGMockFlags(sys.argv)

  return _flag_map[flag]


def GetSourceDir():
  """Returns the absolute path of the directory where the .py files are."""

  return os.path.abspath(GetFlag('gmock_source_dir'))


def GetBuildDir():
  """Returns the absolute path of the directory where the test binaries are."""

  return os.path.abspath(GetFlag('gmock_build_dir'))


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


def Main():
  """Runs the unit test."""

  # We must call _ParseAndStripGMockFlags() before calling
  # unittest.main().  Otherwise the latter will be confused by the
  # --gmock_* flags.
  _ParseAndStripGMockFlags(sys.argv)
  unittest.main()
