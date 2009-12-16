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

"""Runs the specified tests for Google Test.

This script requires Python 2.3 or higher.  To learn the usage, run it
with -h.
"""

import os
import sys

SCRIPT_DIR = os.path.dirname(__file__) or '.'

# Some Python tests don't work in all configurations.
PYTHON_TESTS_TO_SKIP = (
    ('win-dbg', 'gtest_throw_on_failure_test.py'),
    ('win-opt', 'gtest_throw_on_failure_test.py'),
    )

sys.path.append(os.path.join(SCRIPT_DIR, 'test'))
import run_tests_util


def _Main():
  """Runs all tests for Google Test."""

  options, args = run_tests_util.ParseArgs('gtest')
  test_runner = run_tests_util.TestRunner(script_dir=SCRIPT_DIR)
  tests = test_runner.GetTestsToRun(args,
                                    options.configurations,
                                    options.built_configurations,
                                    python_tests_to_skip=PYTHON_TESTS_TO_SKIP)
  if not tests:
    sys.exit(1)  # Incorrect parameters given, abort execution.

  sys.exit(test_runner.RunTests(tests[0], tests[1]))

if __name__ == '__main__':
  _Main()
