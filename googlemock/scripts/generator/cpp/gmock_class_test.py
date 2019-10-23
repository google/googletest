#!/usr/bin/env python
#
# Copyright 2009, Google Inc.
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

"""Tests for gmock.scripts.generator.cpp.gmock_class."""

import os
import sys
import unittest

# Allow the cpp imports below to work when run as a standalone script.
sys.path.append(os.path.join(os.path.dirname(__file__), '..'))

from cpp import ast
from cpp import gmock_class


class TestCase(unittest.TestCase):
  """Helper class that adds assert methods."""

  def StripLeadingWhitespace(self, lines):
    """Strip leading whitespace in each line in 'lines'."""
    return '\n'.join([s.lstrip() for s in lines.split('\n')])

  def assertEqualIgnoreLeadingWhitespace(self, expected_lines, lines):
    """Specialized assert that ignores the indent level."""
    self.assertEqual(expected_lines, self.StripLeadingWhitespace(lines))


class GenerateMethodsTest(TestCase):

  def GenerateMethodSource(self, cpp_source):
    """Convert C++ source to Google Mock output source lines."""
    method_source_lines = []
    # <test> is a pseudo-filename, it is not read or written.
    builder = ast.BuilderFromSource(cpp_source, '<test>')
    ast_list = list(builder.Generate())
    gmock_class._GenerateMethods(method_source_lines, cpp_source, ast_list[0])
    return '\n'.join(method_source_lines)

  def testSimpleMethod(self):
    source = """
class Foo {
 public:
  virtual int Bar();
};
"""
    self.assertEqualIgnoreLeadingWhitespace(
        'MOCK_METHOD0(Bar,\nint());',
        self.GenerateMethodSource(source))

  def testSimpleConstructorsAndDestructor(self):
    source = """
class Foo {
 public:
  Foo();
  Foo(int x);
  Foo(const Foo& f);
  Foo(Foo&& f);
  ~Foo();
  virtual int Bar() = 0;
};
"""
    # The constructors and destructor should be ignored.
    self.assertEqualIgnoreLeadingWhitespace(
        'MOCK_METHOD0(Bar,\nint());',
        self.GenerateMethodSource(source))

  def testVirtualDestructor(self):
    source = """
class Foo {
 public:
  virtual ~Foo();
  virtual int Bar() = 0;
};
"""
    # The destructor should be ignored.
    self.assertEqualIgnoreLeadingWhitespace(
        'MOCK_METHOD0(Bar,\nint());',
        self.GenerateMethodSource(source))

  def testExplicitlyDefaultedConstructorsAndDestructor(self):
    source = """
class Foo {
 public:
  Foo() = default;
  Foo(const Foo& f) = default;
  Foo(Foo&& f) = default;
  ~Foo() = default;
  virtual int Bar() = 0;
};
"""
    # The constructors and destructor should be ignored.
    self.assertEqualIgnoreLeadingWhitespace(
        'MOCK_METHOD0(Bar,\nint());',
        self.GenerateMethodSource(source))

  def testExplicitlyDeletedConstructorsAndDestructor(self):
    source = """
class Foo {
 public:
  Foo() = delete;
  Foo(const Foo& f) = delete;
  Foo(Foo&& f) = delete;
  ~Foo() = delete;
  virtual int Bar() = 0;
};
"""
    # The constructors and destructor should be ignored.
    self.assertEqualIgnoreLeadingWhitespace(
        'MOCK_METHOD0(Bar,\nint());',
        self.GenerateMethodSource(source))

  def testSimpleOverrideMethod(self):
    source = """
class Foo {
 public:
  int Bar() override;
};
"""
    self.assertEqualIgnoreLeadingWhitespace(
        'MOCK_METHOD0(Bar,\nint());',
        self.GenerateMethodSource(source))

  def testSimpleConstMethod(self):
    source = """
class Foo {
 public:
  virtual void Bar(bool flag) const;
};
"""
    self.assertEqualIgnoreLeadingWhitespace(
        'MOCK_CONST_METHOD1(Bar,\nvoid(bool flag));',
        self.GenerateMethodSource(source))

  def testExplicitVoid(self):
    source = """
class Foo {
 public:
  virtual int Bar(void);
};
"""
    self.assertEqualIgnoreLeadingWhitespace(
        'MOCK_METHOD0(Bar,\nint(void));',
        self.GenerateMethodSource(source))

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

  def testDefaultParameters(self):
    source = """
class Foo {
 public:
  virtual void Bar(int a, char c = 'x') = 0;
};
"""
    self.assertEqualIgnoreLeadingWhitespace(
        'MOCK_METHOD2(Bar,\nvoid(int, char));',
        self.GenerateMethodSource(source))

  def testMultipleDefaultParameters(self):
    source = """
class Foo {
 public:
  virtual void Bar(int a = 42, char c = 'x') = 0;
};
"""
    self.assertEqualIgnoreLeadingWhitespace(
        'MOCK_METHOD2(Bar,\nvoid(int, char));',
        self.GenerateMethodSource(source))

  def testRemovesCommentsWhenDefaultsArePresent(self):
    source = """
class Foo {
 public:
  virtual void Bar(int a = 42 /* a comment */,
                   char /* other comment */ c= 'x') = 0;
};
"""
    self.assertEqualIgnoreLeadingWhitespace(
        'MOCK_METHOD2(Bar,\nvoid(int, char));',
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

  def testArgsOfTemplateTypes(self):
    source = """
class Foo {
 public:
  virtual int Bar(const vector<int>& v, map<int, string>* output);
};"""
    self.assertEqualIgnoreLeadingWhitespace(
        'MOCK_METHOD2(Bar,\n'
        'int(const vector<int>& v, map<int, string>* output));',
        self.GenerateMethodSource(source))

  def testReturnTypeWithOneTemplateArg(self):
    source = """
class Foo {
 public:
  virtual vector<int>* Bar(int n);
};"""
    self.assertEqualIgnoreLeadingWhitespace(
        'MOCK_METHOD1(Bar,\nvector<int>*(int n));',
        self.GenerateMethodSource(source))

  def testReturnTypeWithManyTemplateArgs(self):
    source = """
class Foo {
 public:
  virtual map<int, string> Bar();
};"""
    # Comparing the comment text is brittle - we'll think of something
    # better in case this gets annoying, but for now let's keep it simple.
    self.assertEqualIgnoreLeadingWhitespace(
        '// The following line won\'t really compile, as the return\n'
        '// type has multiple template arguments.  To fix it, use a\n'
        '// typedef for the return type.\n'
        'MOCK_METHOD0(Bar,\nmap<int, string>());',
        self.GenerateMethodSource(source))

  def testSimpleMethodInTemplatedClass(self):
    source = """
template<class T>
class Foo {
 public:
  virtual int Bar();
};
"""
    self.assertEqualIgnoreLeadingWhitespace(
        'MOCK_METHOD0_T(Bar,\nint());',
        self.GenerateMethodSource(source))

  def testPointerArgWithoutNames(self):
    source = """
class Foo {
  virtual int Bar(C*);
};
"""
    self.assertEqualIgnoreLeadingWhitespace(
        'MOCK_METHOD1(Bar,\nint(C*));',
        self.GenerateMethodSource(source))

  def testReferenceArgWithoutNames(self):
    source = """
class Foo {
  virtual int Bar(C&);
};
"""
    self.assertEqualIgnoreLeadingWhitespace(
        'MOCK_METHOD1(Bar,\nint(C&));',
        self.GenerateMethodSource(source))

  def testArrayArgWithoutNames(self):
    source = """
class Foo {
  virtual int Bar(C[]);
};
"""
    self.assertEqualIgnoreLeadingWhitespace(
        'MOCK_METHOD1(Bar,\nint(C[]));',
        self.GenerateMethodSource(source))


class GenerateMocksTest(TestCase):

  def GenerateMocks(self, cpp_source):
    """Convert C++ source to complete Google Mock output source."""
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

  def testClassWithStorageSpecifierMacro(self):
    source = """
class STORAGE_SPECIFIER Test {
 public:
  virtual void Foo();
};
"""
    expected = """\
class MockTest : public Test {
public:
MOCK_METHOD0(Foo,
void());
};
"""
    self.assertEqualIgnoreLeadingWhitespace(
        expected, self.GenerateMocks(source))

  def testTemplatedForwardDeclaration(self):
    source = """
template <class T> class Forward;  // Forward declaration should be ignored.
class Test {
 public:
  virtual void Foo();
};
"""
    expected = """\
class MockTest : public Test {
public:
MOCK_METHOD0(Foo,
void());
};
"""
    self.assertEqualIgnoreLeadingWhitespace(
        expected, self.GenerateMocks(source))

  def testTemplatedClass(self):
    source = """
template <typename S, typename T>
class Test {
 public:
  virtual void Foo();
};
"""
    expected = """\
template <typename T0, typename T1>
class MockTest : public Test<T0, T1> {
public:
MOCK_METHOD0_T(Foo,
void());
};
"""
    self.assertEqualIgnoreLeadingWhitespace(
        expected, self.GenerateMocks(source))

  def testTemplateInATemplateTypedef(self):
    source = """
class Test {
 public:
  typedef std::vector<std::list<int>> FooType;
  virtual void Bar(const FooType& test_arg);
};
"""
    expected = """\
class MockTest : public Test {
public:
MOCK_METHOD1(Bar,
void(const FooType& test_arg));
};
"""
    self.assertEqualIgnoreLeadingWhitespace(
        expected, self.GenerateMocks(source))

  def testTemplateInATemplateTypedefWithComma(self):
    source = """
class Test {
 public:
  typedef std::function<void(
      const vector<std::list<int>>&, int> FooType;
  virtual void Bar(const FooType& test_arg);
};
"""
    expected = """\
class MockTest : public Test {
public:
MOCK_METHOD1(Bar,
void(const FooType& test_arg));
};
"""
    self.assertEqualIgnoreLeadingWhitespace(
        expected, self.GenerateMocks(source))

  def testEnumType(self):
    source = """
class Test {
 public:
  enum Bar {
    BAZ, QUX, QUUX, QUUUX
  };
  virtual void Foo();
};
"""
    expected = """\
class MockTest : public Test {
public:
MOCK_METHOD0(Foo,
void());
};
"""
    self.assertEqualIgnoreLeadingWhitespace(
        expected, self.GenerateMocks(source))

  def testEnumClassType(self):
    source = """
class Test {
 public:
  enum class Bar {
    BAZ, QUX, QUUX, QUUUX
  };
  virtual void Foo();
};
"""
    expected = """\
class MockTest : public Test {
public:
MOCK_METHOD0(Foo,
void());
};
"""
    self.assertEqualIgnoreLeadingWhitespace(
        expected, self.GenerateMocks(source))

  def testStdFunction(self):
    source = """
class Test {
 public:
  Test(std::function<int(std::string)> foo) : foo_(foo) {}

  virtual std::function<int(std::string)> foo();

 private:
  std::function<int(std::string)> foo_;
};
"""
    expected = """\
class MockTest : public Test {
public:
MOCK_METHOD0(foo,
std::function<int (std::string)>());
};
"""
    self.assertEqualIgnoreLeadingWhitespace(
        expected, self.GenerateMocks(source))

if __name__ == '__main__':
  unittest.main()
