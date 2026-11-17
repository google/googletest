#!/usr/bin/env python
#
# Copyright 2026 MongoDB Inc.  All Rights Reserved.
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
"""Tests diagnostics for failures to create or remove premature-exit markers."""

import errno
import os
import re
import tempfile

from googletest.test import gtest_test_utils

EXE_PATH = gtest_test_utils.GetTestExecutablePath(
    'gtest_premature_exit_diagnostics_test_'
)


class PrematureExitFileDiagnosticsTest(gtest_test_utils.TestCase):

  def RunTest(self, marker, replace_marker=False):
    env = os.environ.copy()
    env['TEST_PREMATURE_EXIT_FILE'] = marker
    # The child must run independently of any sharding or output settings.
    for name in (
        'GTEST_FILTER',
        'GTEST_TOTAL_SHARDS',
        'GTEST_SHARD_INDEX',
        'GTEST_SHARD_STATUS_FILE',
        'GTEST_OUTPUT',
        'XML_OUTPUT_FILE',
    ):
      env.pop(name, None)
    command = [EXE_PATH]
    if replace_marker:
      command += [
          '--gtest_also_run_disabled_tests',
          '--gtest_filter=*ReplacesMarkerWithNonemptyDirectory',
      ]
    else:
      command += ['--gtest_filter=*RunsTests']
    result = gtest_test_utils.Subprocess(command, env=env)
    self.assertTrue(result.exited, result.output)
    self.assertEqual(result.exit_code, 0, result.output)
    return result.output

  def testWritableMarkerIsRemoved(self):
    with tempfile.TemporaryDirectory() as directory:
      marker = os.path.join(directory, 'marker')
      output = self.RunTest(marker)
      self.assertFalse(os.path.exists(marker))
      self.assertNotIn('Failed to open premature exit filepath', output)
      self.assertNotIn('Failed to remove premature exit filepath', output)

  def testOpenFailureReportsPathAndError(self):
    with tempfile.TemporaryDirectory() as directory:
      marker = os.path.join(directory, 'missing', 'marker')
      output = self.RunTest(marker)
      self.assertIn(
          'Failed to open premature exit filepath "%s" with error %s'
          % (marker, os.strerror(errno.ENOENT)),
          output,
      )
      self.assertNotIn('Failed to remove premature exit filepath', output)
      self.assertFalse(os.path.exists(marker))

  def testRemoveFailureReportsPathAndError(self):
    with tempfile.TemporaryDirectory() as directory:
      marker = os.path.join(directory, 'marker')
      output = self.RunTest(marker, replace_marker=True)
      self.assertTrue(os.path.isdir(marker))
      # remove() may report a nonempty directory or denied access by platform.
      errors = '|'.join(
          re.escape(os.strerror(error))
          for error in (errno.ENOTEMPTY, errno.EEXIST, errno.EACCES)
      )
      self.assertRegex(
          output,
          re.escape(
              'Failed to remove premature exit filepath "%s" with error ' % marker
          ) + '(' + errors + ')',
      )


if __name__ == '__main__':
  gtest_test_utils.Main()
