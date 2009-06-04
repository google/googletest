#!/usr/bin/env python
#
# Copyright 2009 Neal Norwitz All Rights Reserved.
# Portions Copyright 2009 Google Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Tests for gmock.scripts.generator.cpp.gmock_class."""

__author__ = 'nnorwitz@google.com (Neal Norwitz)'


import os
import sys
import unittest

# Allow the cpp imports below to work when run as a standalone script.
sys.path.append(os.path.join(os.path.dirname(__file__), '..'))

from cpp import ast
from cpp import gmock_class


class TestCase(unittest.TestCase):
  """Helper class that adds assert methods."""

  def assertEqualIgnoreLeadingWhitespace(self, expected_lines, lines):
    """Specialized assert that ignores the indent level."""
    stripped_lines = '\n'.join([s.lstrip() for s in lines.split('\n')])
    self.assertEqual(expected_lines, stripped_lines)


class GenerateMethodsTest(TestCase):

  def GenerateMethodSource(self, cpp_source):
    """Helper method to convert C++ source to gMock output source lines."""
    method_source_lines = []
    # <test> is a pseudo-filename, it is not read or written.
    builder = ast.BuilderFromSource(cpp_source, '<test>')
    ast_list = list(builder.Generate())
    gmock_class._GenerateMethods(method_source_lines, cpp_source, ast_list[0])
    return ''.join(method_source_lines)

  def testStrangeNewlineInParameter(self):
    source = """
class Foo {
 public:
  virtual void Bar(int
a) = 0;
};
"""
    self.assertEqualIgnoreLeadingWhitespace(
        'MOCK_METHOD1(Bar,\nvoid(int a));',
        self.GenerateMethodSource(source))

  def testDoubleSlashCommentsInParameterListAreRemoved(self):
    source = """
class Foo {
 public:
  virtual void Bar(int a,  // inline comments should be elided.
                   int b   // inline comments should be elided.
                   ) const = 0;
};
"""
    self.assertEqualIgnoreLeadingWhitespace(
        'MOCK_CONST_METHOD2(Bar,\nvoid(int a, int b));',
        self.GenerateMethodSource(source))

  def testCStyleCommentsInParameterListAreNotRemoved(self):
    # NOTE(nnorwitz): I'm not sure if it's the best behavior to keep these
    # comments.  Also note that C style comments after the last parameter
    # are still elided.
    source = """
class Foo {
 public:
  virtual const string& Bar(int /* keeper */, int b);
};
"""
    self.assertEqualIgnoreLeadingWhitespace(
        'MOCK_METHOD2(Bar,\nconst string&(int /* keeper */, int b));',
        self.GenerateMethodSource(source))


class GenerateMocksTest(TestCase):

  def GenerateMocks(self, cpp_source):
    """Helper method to convert C++ source to complete gMock output source."""
    # <test> is a pseudo-filename, it is not read or written.
    filename = '<test>'
    builder = ast.BuilderFromSource(cpp_source, filename)
    ast_list = list(builder.Generate())
    lines = gmock_class._GenerateMocks(filename, cpp_source, ast_list, None)
    return '\n'.join(lines)

  def testNamespaces(self):
    source = """
namespace Foo {
namespace Bar { class Forward; }
namespace Baz {

class Test {
 public:
  virtual void Foo();
};

}  // namespace Baz
}  // namespace Foo
"""
    expected = """\
namespace Foo {
namespace Baz {

class MockTest : public Test {
public:
MOCK_METHOD0(Foo,
void());
};

}  // namespace Baz
}  // namespace Foo
"""
    self.assertEqualIgnoreLeadingWhitespace(
        expected, self.GenerateMocks(source))


if __name__ == '__main__':
  unittest.main()
