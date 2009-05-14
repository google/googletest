#!/usr/bin/python2.4
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

"""Converts gcc errors in code using Google Mock to plain English."""

__author__ = 'wan@google.com (Zhanyong Wan)'

import re
import sys

_VERSION = '1.0.0'

_COMMON_GMOCK_SYMBOLS = [
    # Matchers
    '_',
    'A',
    'AddressSatisfies',
    'AllOf',
    'An',
    'AnyOf',
    'ContainsRegex',
    'DoubleEq',
    'EndsWith',
    'Eq',
    'Field',
    'FloatEq',
    'Ge',
    'Gt',
    'HasSubstr',
    'IsInitializedProto',
    'Le',
    'Lt',
    'MatcherCast',
    'MatchesRegex',
    'Ne',
    'Not',
    'NotNull',
    'Pointee',
    'PointeeIsInitializedProto',
    'Property',
    'Ref',
    'StartsWith',
    'StrCaseEq',
    'StrCaseNe',
    'StrEq',
    'StrNe',
    'Truly',
    'TypedEq',

    # Actions
    'ByRef',
    'DoAll',
    'DoDefault',
    'IgnoreResult',
    'Invoke',
    'InvokeArgument',
    'InvokeWithoutArgs',
    'Return',
    'ReturnNull',
    'ReturnRef',
    'SetArgumentPointee',
    'SetArrayArgument',
    'WithArgs',

    # Cardinalities
    'AnyNumber',
    'AtLeast',
    'AtMost',
    'Between',
    'Exactly',

    # Sequences
    'InSequence',
    'Sequence',

    # Misc
    'DefaultValue',
    'Mock',
    ]


def _FindAllMatches(regex, s):
  """Generates all matches of regex in string s."""

  r = re.compile(regex)
  return r.finditer(s)


def _GenericDiagnoser(short_name, long_name, regex, diagnosis, msg):
  """Diagnoses the given disease by pattern matching.

  Args:
    short_name: Short name of the disease.
    long_name:  Long name of the disease.
    regex:      Regex for matching the symptoms.
    diagnosis:  Pattern for formatting the diagnosis.
    msg:        Gcc's error messages.
  Yields:
    Tuples of the form
      (short name of disease, long name of disease, diagnosis).
  """

  for m in _FindAllMatches(regex, msg):
    yield (short_name, long_name, diagnosis % m.groupdict())


def _NeedToReturnReferenceDiagnoser(msg):
  """Diagnoses the NRR disease, given the error messages by gcc."""

  regex = (r'In member function \'testing::internal::ReturnAction<R>.*\n'
           r'(?P<file>.*):(?P<line>\d+):\s+instantiated from here\n'
           r'.*gmock-actions\.h.*error: creating array with negative size')
  diagnosis = """%(file)s:%(line)s:
You are using an Return() action in a function that returns a reference.
Please use ReturnRef() instead."""
  return _GenericDiagnoser('NRR', 'Need to Return Reference',
                           regex, diagnosis, msg)


def _NeedToReturnSomethingDiagnoser(msg):
  """Diagnoses the NRS disease, given the error messages by gcc."""

  regex = (r'(?P<file>.*):(?P<line>\d+):\s+'
           r'(instantiated from here\n.'
           r'*gmock-actions\.h.*error: void value not ignored)'
           r'|(error: control reaches end of non-void function)')
  diagnosis = """%(file)s:%(line)s:
You are using an action that returns void, but it needs to return
*something*.  Please tell it *what* to return.  Perhaps you can use
the pattern DoAll(some_action, Return(some_value))?"""
  return _GenericDiagnoser('NRS', 'Need to Return Something',
                           regex, diagnosis, msg)


def _NeedToReturnNothingDiagnoser(msg):
  """Diagnoses the NRN disease, given the error messages by gcc."""

  regex = (r'(?P<file>.*):(?P<line>\d+):\s+instantiated from here\n'
           r'.*gmock-actions\.h.*error: return-statement with a value, '
           r'in function returning \'void\'')
  diagnosis = """%(file)s:%(line)s:
You are using an action that returns *something*, but it needs to return
void.  Please use a void-returning action instead.

All actions but the last in DoAll(...) must return void.  Perhaps you need
to re-arrange the order of actions in a DoAll(), if you are using one?"""
  return _GenericDiagnoser('NRN', 'Need to Return Nothing',
                           regex, diagnosis, msg)


def _IncompleteByReferenceArgumentDiagnoser(msg):
  """Diagnoses the IBRA disease, given the error messages by gcc."""

  regex = (r'(?P<file>.*):(?P<line>\d+):\s+instantiated from here\n'
           r'.*gmock-printers\.h.*error: invalid application of '
           r'\'sizeof\' to incomplete type \'(?P<type>.*)\'')
  diagnosis = """%(file)s:%(line)s:
In order to mock this function, Google Mock needs to see the definition
of type "%(type)s" - declaration alone is not enough.  Either #include
the header that defines it, or change the argument to be passed
by pointer."""
  return _GenericDiagnoser('IBRA', 'Incomplete By-Reference Argument Type',
                           regex, diagnosis, msg)


def _OverloadedFunctionMatcherDiagnoser(msg):
  """Diagnoses the OFM disease, given the error messages by gcc."""

  regex = (r'(?P<file>.*):(?P<line>\d+): error: no matching function for '
           r'call to \'Truly\(<unresolved overloaded function type>\)')
  diagnosis = """%(file)s:%(line)s:
The argument you gave to Truly() is an overloaded function.  Please tell
gcc which overloaded version you want to use.

For example, if you want to use the version whose signature is
  bool Foo(int n);
you should write
  Truly(static_cast<bool (*)(int n)>(Foo))"""
  return _GenericDiagnoser('OFM', 'Overloaded Function Matcher',
                           regex, diagnosis, msg)


def _OverloadedFunctionActionDiagnoser(msg):
  """Diagnoses the OFA disease, given the error messages by gcc."""

  regex = (r'(?P<file>.*):(?P<line>\d+): error: '
           r'no matching function for call to \'Invoke\('
           r'<unresolved overloaded function type>')
  diagnosis = """%(file)s:%(line)s:
You are passing an overloaded function to Invoke().  Please tell gcc
which overloaded version you want to use.

For example, if you want to use the version whose signature is
  bool MyFunction(int n, double x);
you should write something like
  Invoke(static_cast<bool (*)(int n, double x)>(MyFunction))"""
  return _GenericDiagnoser('OFA', 'Overloaded Function Action',
                           regex, diagnosis, msg)


def _OverloadedMethodActionDiagnoser1(msg):
  """Diagnoses the OMA disease, given the error messages by gcc."""

  regex = (r'(?P<file>.*):(?P<line>\d+): error: '
           r'.*no matching function for call to \'Invoke\(.*, '
           r'unresolved overloaded function type>')
  diagnosis = """%(file)s:%(line)s:
The second argument you gave to Invoke() is an overloaded method.  Please
tell gcc which overloaded version you want to use.

For example, if you want to use the version whose signature is
  class Foo {
    ...
    bool Bar(int n, double x);
  };
you should write something like
  Invoke(foo, static_cast<bool (Foo::*)(int n, double x)>(&Foo::Bar))"""
  return _GenericDiagnoser('OMA', 'Overloaded Method Action',
                           regex, diagnosis, msg)


def _MockObjectPointerDiagnoser(msg):
  """Diagnoses the MOP disease, given the error messages by gcc."""

  regex = (r'(?P<file>.*):(?P<line>\d+): error: request for member '
           r'\'gmock_(?P<method>.+)\' in \'(?P<mock_object>.+)\', '
           r'which is of non-class type \'(.*::)*(?P<class_name>.+)\*\'')
  diagnosis = """%(file)s:%(line)s:
The first argument to ON_CALL() and EXPECT_CALL() must be a mock *object*,
not a *pointer* to it.  Please write '*(%(mock_object)s)' instead of
'%(mock_object)s' as your first argument.

For example, given the mock class:

  class %(class_name)s : public ... {
    ...
    MOCK_METHOD0(%(method)s, ...);
  };

and the following mock instance:

  %(class_name)s* mock_ptr = ...

you should use the EXPECT_CALL like this:

  EXPECT_CALL(*mock_ptr, %(method)s(...));"""
  return _GenericDiagnoser('MOP', 'Mock Object Pointer',
                           regex, diagnosis, msg)


def _OverloadedMethodActionDiagnoser2(msg):
  """Diagnoses the OMA disease, given the error messages by gcc."""

  regex = (r'(?P<file>.*):(?P<line>\d+): error: no matching function for '
           r'call to \'Invoke\(.+, <unresolved overloaded function type>\)')
  diagnosis = """%(file)s:%(line)s:
The second argument you gave to Invoke() is an overloaded method.  Please
tell gcc which overloaded version you want to use.

For example, if you want to use the version whose signature is
  class Foo {
    ...
    bool Bar(int n, double x);
  };
you should write something like
  Invoke(foo, static_cast<bool (Foo::*)(int n, double x)>(&Foo::Bar))"""
  return _GenericDiagnoser('OMA', 'Overloaded Method Action',
                           regex, diagnosis, msg)


def _NeedToUseSymbolDiagnoser(msg):
  """Diagnoses the NUS disease, given the error messages by gcc."""

  regex = (r'(?P<file>.*):(?P<line>\d+): error: \'(?P<symbol>.+)\' '
           r'(was not declared in this scope|has not been declared)')
  diagnosis = """%(file)s:%(line)s:
'%(symbol)s' is defined by Google Mock in the testing namespace.
Did you forget to write
  using testing::%(symbol)s;
?"""
  for m in _FindAllMatches(regex, msg):
    symbol = m.groupdict()['symbol']
    if symbol in _COMMON_GMOCK_SYMBOLS:
      yield ('NUS', 'Need to Use Symbol', diagnosis % m.groupdict())


def _NeedToUseReturnNullDiagnoser(msg):
  """Diagnoses the NRNULL disease, given the error messages by gcc."""

  regex = (r'(?P<file>.*):(?P<line>\d+):\s+instantiated from here\n'
           r'.*gmock-actions\.h.*error: invalid conversion from '
           r'\'long int\' to \'(?P<type>.+\*)')

  diagnosis = """%(file)s:%(line)s:
You are probably calling Return(NULL) and the compiler isn't sure how to turn
NULL into a %(type)s*. Use ReturnNull() instead.
Note: the line number may be off; please fix all instances of Return(NULL)."""
  return _GenericDiagnoser('NRNULL', 'Need to use ReturnNull',
                           regex, diagnosis, msg)


def _WrongMockMethodMacroDiagnoser(msg):
  """Diagnoses the WMM disease, given the error messages by gcc."""

  regex = (r'(?P<file>.*):(?P<line>\d+):\s+'
           r'.*this_method_does_not_take_(?P<wrong_args>\d+)_argument.*\n'
           r'.*\n'
           r'.*candidates are.*FunctionMocker<[^>]+A(?P<args>\d+)\)>'
           )

  diagnosis = """%(file)s:%(line)s:
You are using MOCK_METHOD%(wrong_args)s to define a mock method that has
%(args)s arguments. Use MOCK_METHOD%(args)s (or MOCK_CONST_METHOD%(args)s,
MOCK_METHOD%(args)s_T, MOCK_CONST_METHOD%(args)s_T as appropriate) instead."""
  return _GenericDiagnoser('WMM', 'Wrong MOCK_METHODn macro',
                           regex, diagnosis, msg)



_DIAGNOSERS = [
    _IncompleteByReferenceArgumentDiagnoser,
    _MockObjectPointerDiagnoser,
    _NeedToReturnNothingDiagnoser,
    _NeedToReturnReferenceDiagnoser,
    _NeedToReturnSomethingDiagnoser,
    _NeedToUseReturnNullDiagnoser,
    _NeedToUseSymbolDiagnoser,
    _OverloadedFunctionActionDiagnoser,
    _OverloadedFunctionMatcherDiagnoser,
    _OverloadedMethodActionDiagnoser1,
    _OverloadedMethodActionDiagnoser2,
    _WrongMockMethodMacroDiagnoser,
    ]


def Diagnose(msg):
  """Generates all possible diagnoses given the gcc error message."""

  for diagnoser in _DIAGNOSERS:
    for diagnosis in diagnoser(msg):
      yield '[%s - %s]\n%s' % diagnosis


def main():
  print ('Google Mock Doctor v%s - '
         'diagnoses problems in code using Google Mock.' % _VERSION)

  if sys.stdin.isatty():
    print ('Please copy and paste the compiler errors here.  Press c-D when '
           'you are done:')
  else:
    print 'Waiting for compiler errors on stdin . . .'

  msg = sys.stdin.read().strip()
  diagnoses = list(Diagnose(msg))
  count = len(diagnoses)
  if not count:
    print '\nGcc complained:'
    print '8<------------------------------------------------------------'
    print msg
    print '------------------------------------------------------------>8'
    print """
Uh-oh, I'm not smart enough to figure out what the problem is. :-(
However...
If you send your source code and gcc's error messages to
googlemock@googlegroups.com, you can be helped and I can get smarter --
win-win for us!"""
  else:
    print '------------------------------------------------------------'
    print 'Your code appears to have the following',
    if count > 1:
      print '%s diseases:' % (count,)
    else:
      print 'disease:'
    i = 0
    for d in diagnoses:
      i += 1
      if count > 1:
        print '\n#%s:' % (i,)
      print d
    print """
How did I do?  If you think I'm wrong or unhelpful, please send your
source code and gcc's error messages to googlemock@googlegroups.com.  Then
you can be helped and I can get smarter -- I promise I won't be upset!"""


if __name__ == '__main__':
  main()
