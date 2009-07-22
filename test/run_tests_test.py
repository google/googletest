#!/usr/bin/env python
#
# Copyright 2009 Google Inc. All Rights Reserved.
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

"""Tests for run_tests.py test runner script."""

__author__ = 'vladl@google.com (Vlad Losev)'

import os
import re
import sets
import sys
import unittest

sys.path.append(os.path.join(os.path.dirname(sys.argv[0]), os.pardir))
import run_tests


GTEST_DBG_DIR = 'scons/build/dbg/scons'
GTEST_OPT_DIR = 'scons/build/opt/scons'
GTEST_OTHER_DIR = 'scons/build/other/scons'


def AddExeExtension(path):
  """Appends .exe to the path on Windows or Cygwin."""

  if run_tests.IS_WINDOWS or run_tests.IS_CYGWIN:
    return path + '.exe'
  else:
    return path


class FakePath(object):
  """A fake os.path module for testing."""

  def __init__(self, current_dir=os.getcwd(), known_paths=None):
    self.current_dir = current_dir
    self.tree = {}
    self.path_separator = os.sep

    # known_paths contains either absolute or relative paths. Relative paths
    # are absolutized with self.current_dir.
    if known_paths:
      self._AddPaths(known_paths)

  def _AddPath(self, path):
    ends_with_slash = path.endswith('/')
    path = self.abspath(path)
    if ends_with_slash:
      path += self.path_separator
    name_list = path.split(self.path_separator)
    tree = self.tree
    for name in name_list[:-1]:
      if not name:
        continue
      if name in tree:
        tree = tree[name]
      else:
        tree[name] = {}
        tree = tree[name]

    name = name_list[-1]
    if name:
      if name in tree:
        assert tree[name] == 1
      else:
        tree[name] = 1

  def _AddPaths(self, paths):
    for path in paths:
      self._AddPath(path)

  def PathElement(self, path):
    """Returns an internal representation of directory tree entry for path."""
    tree = self.tree
    name_list = self.abspath(path).split(self.path_separator)
    for name in name_list:
      if not name:
        continue
      tree = tree.get(name, None)
      if tree is None:
        break

    return tree

  def normpath(self, path):
    return os.path.normpath(path)

  def abspath(self, path):
    return self.normpath(os.path.join(self.current_dir, path))

  def isfile(self, path):
    return self.PathElement(self.abspath(path)) == 1

  def isdir(self, path):
    return type(self.PathElement(self.abspath(path))) == type(dict())

  def basename(self, path):
    return os.path.basename(path)

  def dirname(self, path):
    return os.path.dirname(path)

  def join(self, *kargs):
    return os.path.join(*kargs)


class FakeOs(object):
  """A fake os module for testing."""
  P_WAIT = os.P_WAIT

  def __init__(self, fake_path_module):
    self.path = fake_path_module

    # Some methods/attributes are delegated to the real os module.
    self.environ = os.environ

  def listdir(self, path):
    assert self.path.isdir(path)
    return self.path.PathElement(path).iterkeys()

  def spawnv(self, wait, executable, *kargs):
    assert wait == FakeOs.P_WAIT
    return self.spawn_impl(executable, kargs)


class GetTestsToRunTest(unittest.TestCase):
  """Exercises TestRunner.GetTestsToRun."""

  def NormalizeGetTestsToRunResults(self, results):
    """Normalizes path data returned from GetTestsToRun for comparison."""

    def NormalizePythonTestPair(pair):
      """Normalizes path data in the (directory, python_script) pair."""

      return (os.path.normpath(pair[0]), os.path.normpath(pair[1]))

    def NormalizeBinaryTestPair(pair):
      """Normalizes path data in the (directory, binary_executable) pair."""

      directory, executable = map(os.path.normpath, pair)

      # On Windows and Cygwin, the test file names have the .exe extension, but
      # they can be invoked either by name or by name+extension. Our test must
      # accommodate both situations.
      if run_tests.IS_WINDOWS or run_tests.IS_CYGWIN:
        executable = re.sub(r'\.exe$', '', executable)
      return (directory, executable)

    python_tests = sets.Set(map(NormalizePythonTestPair, results[0]))
    binary_tests = sets.Set(map(NormalizeBinaryTestPair, results[1]))
    return (python_tests, binary_tests)

  def AssertResultsEqual(self, results, expected):
    """Asserts results returned by GetTestsToRun equal to expected results."""

    self.assertEqual(self.NormalizeGetTestsToRunResults(results),
                     self.NormalizeGetTestsToRunResults(expected),
                     'Incorrect set of tests returned:\n%s\nexpected:\n%s' %
                     (results, expected))

  def setUp(self):
    self.fake_os = FakeOs(FakePath(
        current_dir=os.path.abspath(os.path.dirname(run_tests.__file__)),
        known_paths=[AddExeExtension(GTEST_DBG_DIR + '/gtest_unittest'),
                     AddExeExtension(GTEST_OPT_DIR + '/gtest_unittest'),
                     'test/gtest_color_test.py']))
    self.fake_configurations = ['dbg', 'opt']
    self.test_runner = run_tests.TestRunner(injected_os=self.fake_os,
                                            injected_subprocess=None,
                                            injected_script_dir='.')

  def testBinaryTestsOnly(self):
    """Exercises GetTestsToRun with parameters designating binary tests only."""

    # A default build.
    self.AssertResultsEqual(
        self.test_runner.GetTestsToRun(
            ['gtest_unittest'],
            '',
            False,
            available_configurations=self.fake_configurations),
        ([],
         [(GTEST_DBG_DIR, GTEST_DBG_DIR + '/gtest_unittest')]))

    # An explicitly specified directory.
    self.AssertResultsEqual(
        self.test_runner.GetTestsToRun(
            [GTEST_DBG_DIR, 'gtest_unittest'],
            '',
            False,
            available_configurations=self.fake_configurations),
        ([],
         [(GTEST_DBG_DIR, GTEST_DBG_DIR + '/gtest_unittest')]))

    # A particular configuration.
    self.AssertResultsEqual(
        self.test_runner.GetTestsToRun(
            ['gtest_unittest'],
            'other',
            False,
            available_configurations=self.fake_configurations),
        ([],
         [(GTEST_OTHER_DIR, GTEST_OTHER_DIR + '/gtest_unittest')]))

    # All available configurations
    self.AssertResultsEqual(
        self.test_runner.GetTestsToRun(
            ['gtest_unittest'],
            'all',
            False,
            available_configurations=self.fake_configurations),
        ([],
         [(GTEST_DBG_DIR, GTEST_DBG_DIR + '/gtest_unittest'),
          (GTEST_OPT_DIR, GTEST_OPT_DIR + '/gtest_unittest')]))

    # All built configurations (unbuilt don't cause failure).
    self.AssertResultsEqual(
        self.test_runner.GetTestsToRun(
            ['gtest_unittest'],
            '',
            True,
            available_configurations=self.fake_configurations + ['unbuilt']),
        ([],
         [(GTEST_DBG_DIR, GTEST_DBG_DIR + '/gtest_unittest'),
          (GTEST_OPT_DIR, GTEST_OPT_DIR + '/gtest_unittest')]))

    # A combination of an explicit directory and a configuration.
    self.AssertResultsEqual(
        self.test_runner.GetTestsToRun(
            [GTEST_DBG_DIR, 'gtest_unittest'],
            'opt',
            False,
            available_configurations=self.fake_configurations),
        ([],
         [(GTEST_DBG_DIR, GTEST_DBG_DIR + '/gtest_unittest'),
          (GTEST_OPT_DIR, GTEST_OPT_DIR + '/gtest_unittest')]))

    # Same test specified in an explicit directory and via a configuration.
    self.AssertResultsEqual(
        self.test_runner.GetTestsToRun(
            [GTEST_DBG_DIR, 'gtest_unittest'],
            'dbg',
            False,
            available_configurations=self.fake_configurations),
        ([],
         [(GTEST_DBG_DIR, GTEST_DBG_DIR + '/gtest_unittest')]))

    # All built configurations + explicit directory + explicit configuration.
    self.AssertResultsEqual(
        self.test_runner.GetTestsToRun(
            [GTEST_DBG_DIR, 'gtest_unittest'],
            'opt',
            True,
            available_configurations=self.fake_configurations),
        ([],
         [(GTEST_DBG_DIR, GTEST_DBG_DIR + '/gtest_unittest'),
          (GTEST_OPT_DIR, GTEST_OPT_DIR + '/gtest_unittest')]))

  def testPythonTestsOnly(self):
    """Exercises GetTestsToRun with parameters designating Python tests only."""

    # A default build.
    self.AssertResultsEqual(
        self.test_runner.GetTestsToRun(
            ['gtest_color_test.py'],
            '',
            False,
            available_configurations=self.fake_configurations),
        ([(GTEST_DBG_DIR, 'test/gtest_color_test.py')],
         []))

    # An explicitly specified directory.
    self.AssertResultsEqual(
        self.test_runner.GetTestsToRun(
            [GTEST_DBG_DIR, 'test/gtest_color_test.py'],
            '',
            False,
            available_configurations=self.fake_configurations),
        ([(GTEST_DBG_DIR, 'test/gtest_color_test.py')],
         []))

    # A particular configuration.
    self.AssertResultsEqual(
        self.test_runner.GetTestsToRun(
            ['gtest_color_test.py'],
            'other',
            False,
            available_configurations=self.fake_configurations),
        ([(GTEST_OTHER_DIR, 'test/gtest_color_test.py')],
         []))

    # All available configurations
    self.AssertResultsEqual(
        self.test_runner.GetTestsToRun(
            ['test/gtest_color_test.py'],
            'all',
            False,
            available_configurations=self.fake_configurations),
        ([(GTEST_DBG_DIR, 'test/gtest_color_test.py'),
          (GTEST_OPT_DIR, 'test/gtest_color_test.py')],
         []))

    # All built configurations (unbuilt don't cause failure).
    self.AssertResultsEqual(
        self.test_runner.GetTestsToRun(
            ['gtest_color_test.py'],
            '',
            True,
            available_configurations=self.fake_configurations + ['unbuilt']),
        ([(GTEST_DBG_DIR, 'test/gtest_color_test.py'),
          (GTEST_OPT_DIR, 'test/gtest_color_test.py')],
         []))

    # A combination of an explicit directory and a configuration.
    self.AssertResultsEqual(
        self.test_runner.GetTestsToRun(
            [GTEST_DBG_DIR, 'gtest_color_test.py'],
            'opt',
            False,
            available_configurations=self.fake_configurations),
        ([(GTEST_DBG_DIR, 'test/gtest_color_test.py'),
          (GTEST_OPT_DIR, 'test/gtest_color_test.py')],
         []))

    # Same test specified in an explicit directory and via a configuration.
    self.AssertResultsEqual(
        self.test_runner.GetTestsToRun(
            [GTEST_DBG_DIR, 'gtest_color_test.py'],
            'dbg',
            False,
            available_configurations=self.fake_configurations),
        ([(GTEST_DBG_DIR, 'test/gtest_color_test.py')],
         []))

    # All built configurations + explicit directory + explicit configuration.
    self.AssertResultsEqual(
        self.test_runner.GetTestsToRun(
            [GTEST_DBG_DIR, 'gtest_color_test.py'],
            'opt',
            True,
            available_configurations=self.fake_configurations),
        ([(GTEST_DBG_DIR, 'test/gtest_color_test.py'),
          (GTEST_OPT_DIR, 'test/gtest_color_test.py')],
         []))

  def testCombinationOfBinaryAndPythonTests(self):
    """Exercises GetTestsToRun with mixed binary/Python tests."""

    # Use only default configuration for this test.

    # Neither binary nor Python tests are specified so find all.
    self.AssertResultsEqual(
        self.test_runner.GetTestsToRun(
            [],
            '',
            False,
            available_configurations=self.fake_configurations),
        ([(GTEST_DBG_DIR, 'test/gtest_color_test.py')],
         [(GTEST_DBG_DIR, GTEST_DBG_DIR + '/gtest_unittest')]))

    # Specifying both binary and Python tests.
    self.AssertResultsEqual(
        self.test_runner.GetTestsToRun(
            ['gtest_unittest', 'gtest_color_test.py'],
            '',
            False,
            available_configurations=self.fake_configurations),
        ([(GTEST_DBG_DIR, 'test/gtest_color_test.py')],
         [(GTEST_DBG_DIR, GTEST_DBG_DIR + '/gtest_unittest')]))

    # Specifying binary tests suppresses Python tests.
    self.AssertResultsEqual(
        self.test_runner.GetTestsToRun(
            ['gtest_unittest'],
            '',
            False,
            available_configurations=self.fake_configurations),
        ([],
         [(GTEST_DBG_DIR, GTEST_DBG_DIR + '/gtest_unittest')]))

   # Specifying Python tests suppresses binary tests.
    self.AssertResultsEqual(
        self.test_runner.GetTestsToRun(
            ['gtest_color_test.py'],
            '',
            False,
            available_configurations=self.fake_configurations),
        ([(GTEST_DBG_DIR, 'test/gtest_color_test.py')],
         []))

  def testIgnoresNonTestFiles(self):
    """Verifies that GetTestsToRun ignores non-test files in the filesystem."""

    self.fake_os = FakeOs(FakePath(
        current_dir=os.path.abspath(os.path.dirname(run_tests.__file__)),
        known_paths=[AddExeExtension(GTEST_DBG_DIR + '/gtest_nontest'),
                     'test/']))
    self.test_runner = run_tests.TestRunner(injected_os=self.fake_os,
                                            injected_subprocess=None,
                                            injected_script_dir='.')
    self.AssertResultsEqual(
        self.test_runner.GetTestsToRun(
            [],
            '',
            True,
            available_configurations=self.fake_configurations),
        ([], []))

  def testWorksFromDifferentDir(self):
    """Exercises GetTestsToRun from a directory different from run_test.py's."""

    # Here we simulate an test script in directory /d/ called from the
    # directory /a/b/c/.
    self.fake_os = FakeOs(FakePath(
        current_dir=os.path.abspath('/a/b/c'),
        known_paths=[
            '/a/b/c/',
            AddExeExtension('/d/' + GTEST_DBG_DIR + '/gtest_unittest'),
            AddExeExtension('/d/' + GTEST_OPT_DIR + '/gtest_unittest'),
            '/d/test/gtest_color_test.py']))
    self.fake_configurations = ['dbg', 'opt']
    self.test_runner = run_tests.TestRunner(injected_os=self.fake_os,
                                            injected_subprocess=None,
                                            injected_script_dir='/d/')
    # A binary test.
    self.AssertResultsEqual(
        self.test_runner.GetTestsToRun(
            ['gtest_unittest'],
            '',
            False,
            available_configurations=self.fake_configurations),
        ([],
         [('/d/' + GTEST_DBG_DIR, '/d/' + GTEST_DBG_DIR + '/gtest_unittest')]))

    # A Python test.
    self.AssertResultsEqual(
        self.test_runner.GetTestsToRun(
            ['gtest_color_test.py'],
            '',
            False,
            available_configurations=self.fake_configurations),
        ([('/d/' + GTEST_DBG_DIR, '/d/test/gtest_color_test.py')], []))


  def testNonTestBinary(self):
    """Exercises GetTestsToRun with a non-test parameter."""

    self.assert_(
        not self.test_runner.GetTestsToRun(
            ['gtest_unittest_not_really'],
            '',
            False,
            available_configurations=self.fake_configurations))

  def testNonExistingPythonTest(self):
    """Exercises GetTestsToRun with a non-existent Python test parameter."""

    self.assert_(
        not self.test_runner.GetTestsToRun(
            ['nonexistent_test.py'],
            '',
            False,
            available_configurations=self.fake_configurations))

  if run_tests.IS_WINDOWS or run_tests.IS_CYGWIN:
    def testDoesNotPickNonExeFilesOnWindows(self):
      """Verifies that GetTestsToRun does not find _test files on Windows."""

      self.fake_os = FakeOs(FakePath(
          current_dir=os.path.abspath(os.path.dirname(run_tests.__file__)),
          known_paths=['/d/' + GTEST_DBG_DIR + '/gtest_test', 'test/']))
      self.test_runner = run_tests.TestRunner(injected_os=self.fake_os,
                                              injected_subprocess=None,
                                              injected_script_dir='.')
      self.AssertResultsEqual(
          self.test_runner.GetTestsToRun(
              [],
              '',
              True,
              available_configurations=self.fake_configurations),
          ([], []))


class RunTestsTest(unittest.TestCase):
  """Exercises TestRunner.RunTests."""

  def SpawnSuccess(self, unused_executable, unused_argv):
    """Fakes test success by returning 0 as an exit code."""

    self.num_spawn_calls += 1
    return 0

  def SpawnFailure(self, unused_executable, unused_argv):
    """Fakes test success by returning 1 as an exit code."""

    self.num_spawn_calls += 1
    return 1

  def setUp(self):
    self.fake_os = FakeOs(FakePath(
        current_dir=os.path.abspath(os.path.dirname(run_tests.__file__)),
        known_paths=[
            AddExeExtension(GTEST_DBG_DIR + '/gtest_unittest'),
            AddExeExtension(GTEST_OPT_DIR + '/gtest_unittest'),
            'test/gtest_color_test.py']))
    self.fake_configurations = ['dbg', 'opt']
    self.test_runner = run_tests.TestRunner(injected_os=self.fake_os,
                                            injected_subprocess=None)
    self.num_spawn_calls = 0  # A number of calls to spawn.

  def testRunPythonTestSuccess(self):
    """Exercises RunTests to handle a Python test success."""

    self.fake_os.spawn_impl = self.SpawnSuccess
    self.assertEqual(
        self.test_runner.RunTests(
            [(GTEST_DBG_DIR, 'test/gtest_color_test.py')],
            []),
        0)
    self.assertEqual(self.num_spawn_calls, 1)

  def testRunBinaryTestSuccess(self):
    """Exercises RunTests to handle a binary test success."""

    self.fake_os.spawn_impl = self.SpawnSuccess
    self.assertEqual(
        self.test_runner.RunTests(
            [],
            [(GTEST_DBG_DIR, GTEST_DBG_DIR + '/gtest_unittest')]),
        0)
    self.assertEqual(self.num_spawn_calls, 1)

  def testRunPythonTestFauilure(self):
    """Exercises RunTests to handle a Python test failure."""

    self.fake_os.spawn_impl = self.SpawnFailure
    self.assertEqual(
        self.test_runner.RunTests(
            [(GTEST_DBG_DIR, 'test/gtest_color_test.py')],
            []),
        1)
    self.assertEqual(self.num_spawn_calls, 1)

  def testRunBinaryTestFailure(self):
    """Exercises RunTests to handle a binary test failure."""

    self.fake_os.spawn_impl = self.SpawnFailure
    self.assertEqual(
        self.test_runner.RunTests(
            [],
            [(GTEST_DBG_DIR, GTEST_DBG_DIR + '/gtest_unittest')]),
        1)
    self.assertEqual(self.num_spawn_calls, 1)

  def testCombinedTestSuccess(self):
    """Exercises RunTests to handle a success of both Python and binary test."""

    self.fake_os.spawn_impl = self.SpawnSuccess
    self.assertEqual(
        self.test_runner.RunTests(
            [(GTEST_DBG_DIR, GTEST_DBG_DIR + '/gtest_unittest')],
            [(GTEST_DBG_DIR, GTEST_DBG_DIR + '/gtest_unittest')]),
        0)
    self.assertEqual(self.num_spawn_calls, 2)

  def testCombinedTestSuccessAndFailure(self):
    """Exercises RunTests to handle a success of both Python and binary test."""

    def SpawnImpl(executable, argv):
      self.num_spawn_calls += 1
      # Simulates failure of a Python test and success of a binary test.
      if '.py' in executable or '.py' in argv[0]:
        return 1
      else:
        return 0

    self.fake_os.spawn_impl = SpawnImpl
    self.assertEqual(
        self.test_runner.RunTests(
            [(GTEST_DBG_DIR, GTEST_DBG_DIR + '/gtest_unittest')],
            [(GTEST_DBG_DIR, GTEST_DBG_DIR + '/gtest_unittest')]),
        0)
    self.assertEqual(self.num_spawn_calls, 2)


if __name__ == '__main__':
  unittest.main()
