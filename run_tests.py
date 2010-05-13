#!/usr/bin/env python
#
# Copyright 2008, Google Inc. All rights reserved.
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

"""Runs the specified tests for Google Mock.

This script requires Python 2.3 or higher.  To learn the usage, run it
with -h.
"""

__author__ = 'vladl@google.com (Vlad Losev)'


import os
import sys

SCRIPT_DIR = os.path.dirname(__file__) or '.'

# Path to the Google Test code this Google Mock will use.  We assume the
# gtest/ directory is either a subdirectory (possibly via a symbolic link)
# of gmock/ or a sibling.
#
# isdir resolves symbolic links.
if os.path.isdir(os.path.join(SCRIPT_DIR, 'gtest/test')):
  RUN_TESTS_UTIL_DIR = os.path.join(SCRIPT_DIR, 'gtest/test')
else:
  RUN_TESTS_UTIL_DIR = os.path.join(SCRIPT_DIR, '../gtest/test')

sys.path.append(RUN_TESTS_UTIL_DIR)
import run_tests_util

def GetGmockBuildDir(injected_os, script_dir, config):
  return injected_os.path.normpath(injected_os.path.join(script_dir,
                                                         'scons/build',
                                                         config,
                                                         'gmock/scons'))


def _Main():
  """Runs all tests for Google Mock."""

  options, args = run_tests_util.ParseArgs('gtest')
  test_runner = run_tests_util.TestRunner(
      script_dir=SCRIPT_DIR,
      injected_build_dir_finder=GetGmockBuildDir)
  tests = test_runner.GetTestsToRun(args,
                                    options.configurations,
                                    options.built_configurations)
  if not tests:
    sys.exit(1)  # Incorrect parameters given, abort execution.

  sys.exit(test_runner.RunTests(tests[0], tests[1]))

if __name__ == '__main__':
  _Main()
