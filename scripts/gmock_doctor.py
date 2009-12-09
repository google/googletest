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

"""Converts gcc errors in code using Google Mock to plain English."""

__author__ = 'wan@google.com (Zhanyong Wan)'

import re
import sys

_VERSION = '1.0.3'

_COMMON_GMOCK_SYMBOLS = [
    # Matchers
    '_',
    'A',
    'AddressSatisfies',
    'AllOf',
    'An',
    'AnyOf',
    'ContainerEq',
    'Contains',
    'ContainsRegex',
    'DoubleEq',
    'ElementsAre',
    'ElementsAreArray',
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
    'Matches',
    'MatchesRegex',
    'NanSensitiveDoubleEq',
    'NanSensitiveFloatEq',
    'Ne',
    'Not',
    'NotNull',
    'Pointee',
    'Property',
    'Ref',
    'ResultOf',
    'SafeMatcherCast',
    'StartsWith',
    'StrCaseEq',
    'StrCaseNe',
    'StrEq',
    'StrNe',
    'Truly',
    'TypedEq',
    'Value',

    # Actions
    'Assign',
    'ByRef',
    'DeleteArg',
    'DoAll',
    'DoDefault',
    'IgnoreResult',
    'Invoke',
    'InvokeArgument',
    'InvokeWithoutArgs',
    'Return',
    'ReturnNew',
    'ReturnNull',
    'ReturnRef',
    'SaveArg',
    'SetArgReferee',
    'SetArgumentPointee',
    'SetArrayArgument',
    'SetErrnoAndReturn',
    'Throw',
    'WithArg',
    'WithArgs',
    'WithoutArgs',

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

# Regex for matching source file path and line number in gcc's errors.
_FILE_LINE_RE = r'(?P<file>.*):(?P<line>\d+):\s+'


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

  diagnosis = '%(file)s:%(line)s:' + diagnosis
  for m in _FindAllMatches(regex, msg):
    yield (short_name, long_name, diagnosis % m.groupdict())


def _NeedToReturnReferenceDiagnoser(msg):
  """Diagnoses the NRR disease, given the error messages by gcc."""

  regex = (r'In member function \'testing::internal::ReturnAction<R>.*\n'
           + _FILE_LINE_RE + r'instantiated from here\n'
           r'.*gmock-actions\.h.*error: creating array with negative size')
  diagnosis = """
You are using an Return() action in a function that returns a reference.
Please use ReturnRef() instead."""
  return _GenericDiagnoser('NRR', 'Need to Return Reference',
                           regex, diagnosis, msg)


def _NeedToReturnSomethingDiagnoser(msg):
  """Diagnoses the NRS disease, given the error messages by gcc."""

  regex = (_FILE_LINE_RE +
           r'(instantiated from here\n.'
           r'*gmock.*actions\.h.*error: void value not ignored)'
           r'|(error: control reaches end of non-void function)')
  diagnosis = """
You are using an action that returns void, but it needs to return
*something*.  Please tell it *what* to return.  Perhaps you can use
the pattern DoAll(some_action, Return(some_value))?"""
  return _GenericDiagnoser('NRS', 'Need to Return Something',
                           regex, diagnosis, msg)


def _NeedToReturnNothingDiagnoser(msg):
  """Diagnoses the NRN disease, given the error messages by gcc."""

  regex = (_FILE_LINE_RE + r'instantiated from here\n'
           r'.*gmock-actions\.h.*error: instantiation of '
           r'\'testing::internal::ReturnAction<R>::Impl<F>::value_\' '
           r'as type \'void\'')
  diagnosis = """
You are using an action that returns *something*, but it needs to return
void.  Please use a void-returning action instead.

All actions but the last in DoAll(...) must return void.  Perhaps you need
to re-arrange the order of actions in a DoAll(), if you are using one?"""
  return _GenericDiagnoser('NRN', 'Need to Return Nothing',
                           regex, diagnosis, msg)


def _IncompleteByReferenceArgumentDiagnoser(msg):
  """Diagnoses the IBRA disease, given the error messages by gcc."""

  regex = (_FILE_LINE_RE + r'instantiated from here\n'
           r'.*gmock-printers\.h.*error: invalid application of '
           r'\'sizeof\' to incomplete type \'(?P<type>.*)\'')
  diagnosis = """
In order to mock this function, Google Mock needs to see the definition
of type "%(type)s" - declaration alone is not enough.  Either #include
the header that defines it, or change the argument to be passed
by pointer."""
  return _GenericDiagnoser('IBRA', 'Incomplete By-Reference Argument Type',
                           regex, diagnosis, msg)


def _OverloadedFunctionMatcherDiagnoser(msg):
  """Diagnoses the OFM disease, given the error messages by gcc."""

  regex = (_FILE_LINE_RE + r'error: no matching function for '
           r'call to \'Truly\(<unresolved overloaded function type>\)')
  diagnosis = """
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

  regex = (_FILE_LINE_RE + r'error: no matching function for call to \'Invoke\('
           r'<unresolved overloaded function type>')
  diagnosis = """
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

  regex = (_FILE_LINE_RE + r'error: '
           r'.*no matching function for call to \'Invoke\(.*, '
           r'unresolved overloaded function type>')
  diagnosis = """
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

  regex = (_FILE_LINE_RE + r'error: request for member '
           r'\'gmock_(?P<method>.+)\' in \'(?P<mock_object>.+)\', '
           r'which is of non-class type \'(.*::)*(?P<class_name>.+)\*\'')
  diagnosis = """
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

  regex = (_FILE_LINE_RE + r'error: no matching function for '
           r'call to \'Invoke\(.+, <unresolved overloaded function type>\)')
  diagnosis = """
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

  regex = (_FILE_LINE_RE + r'error: \'(?P<symbol>.+)\' '
           r'(was not declared in this scope|has not been declared)')
  diagnosis = """
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

  regex = ('instantiated from \'testing::internal::ReturnAction<R>'
           '::operator testing::Action<Func>\(\) const.*\n' +
           _FILE_LINE_RE + r'instantiated from here\n'
           r'.*error: no matching function for call to \'implicit_cast\('
           r'long int&\)')
  diagnosis = """
You are probably calling Return(NULL) and the compiler isn't sure how to turn
NULL into the right type. Use ReturnNull() instead.
Note: the line number may be off; please fix all instances of Return(NULL)."""
  return _GenericDiagnoser('NRNULL', 'Need to use ReturnNull',
                           regex, diagnosis, msg)


_TTB_DIAGNOSIS = """
In a mock class template, types or typedefs defined in the base class
template are *not* automatically visible.  This is how C++ works.  Before
you can use a type or typedef named %(type)s defined in base class Base<T>, you
need to make it visible.  One way to do it is:

  typedef typename Base<T>::%(type)s %(type)s;"""


def _TypeInTemplatedBaseDiagnoser1(msg):
  """Diagnoses the TTB disease, given the error messages by gcc.

  This version works when the type is used as the mock function's return
  type.
  """

  gcc_4_3_1_regex = (
      r'In member function \'int .*\n' + _FILE_LINE_RE +
      r'error: a function call cannot appear in a constant-expression')
  gcc_4_4_0_regex = (
      r'error: a function call cannot appear in a constant-expression'
      + _FILE_LINE_RE + r'error: template argument 1 is invalid\n')
  diagnosis = _TTB_DIAGNOSIS % {'type': 'Foo'}
  return (list(_GenericDiagnoser('TTB', 'Type in Template Base',
                                gcc_4_3_1_regex, diagnosis, msg)) +
          list(_GenericDiagnoser('TTB', 'Type in Template Base',
                                 gcc_4_4_0_regex, diagnosis, msg)))


def _TypeInTemplatedBaseDiagnoser2(msg):
  """Diagnoses the TTB disease, given the error messages by gcc.

  This version works when the type is used as the mock function's sole
  parameter type.
  """

  regex = (_FILE_LINE_RE +
           r'error: \'(?P<type>.+)\' was not declared in this scope\n'
           r'.*error: template argument 1 is invalid\n')
  return _GenericDiagnoser('TTB', 'Type in Template Base',
                           regex, _TTB_DIAGNOSIS, msg)


def _TypeInTemplatedBaseDiagnoser3(msg):
  """Diagnoses the TTB disease, given the error messages by gcc.

  This version works when the type is used as a parameter of a mock
  function that has multiple parameters.
  """

  regex = (r'error: expected `;\' before \'::\' token\n'
           + _FILE_LINE_RE +
           r'error: \'(?P<type>.+)\' was not declared in this scope\n'
           r'.*error: template argument 1 is invalid\n'
           r'.*error: \'.+\' was not declared in this scope')
  return _GenericDiagnoser('TTB', 'Type in Template Base',
                           regex, _TTB_DIAGNOSIS, msg)


def _WrongMockMethodMacroDiagnoser(msg):
  """Diagnoses the WMM disease, given the error messages by gcc."""

  regex = (_FILE_LINE_RE +
           r'.*this_method_does_not_take_(?P<wrong_args>\d+)_argument.*\n'
           r'.*\n'
           r'.*candidates are.*FunctionMocker<[^>]+A(?P<args>\d+)\)>')
  diagnosis = """
You are using MOCK_METHOD%(wrong_args)s to define a mock method that has
%(args)s arguments. Use MOCK_METHOD%(args)s (or MOCK_CONST_METHOD%(args)s,
MOCK_METHOD%(args)s_T, MOCK_CONST_METHOD%(args)s_T as appropriate) instead."""
  return _GenericDiagnoser('WMM', 'Wrong MOCK_METHODn Macro',
                           regex, diagnosis, msg)


def _WrongParenPositionDiagnoser(msg):
  """Diagnoses the WPP disease, given the error messages by gcc."""

  regex = (_FILE_LINE_RE +
           r'error:.*testing::internal::MockSpec<.* has no member named \''
           r'(?P<method>\w+)\'')
  diagnosis = """
The closing parenthesis of ON_CALL or EXPECT_CALL should be *before*
".%(method)s".  For example, you should write:
  EXPECT_CALL(my_mock, Foo(_)).%(method)s(...);
instead of:
  EXPECT_CALL(my_mock, Foo(_).%(method)s(...));"""
  return _GenericDiagnoser('WPP', 'Wrong Parenthesis Position',
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
    _TypeInTemplatedBaseDiagnoser1,
    _TypeInTemplatedBaseDiagnoser2,
    _TypeInTemplatedBaseDiagnoser3,
    _WrongMockMethodMacroDiagnoser,
    _WrongParenPositionDiagnoser,
    ]


def Diagnose(msg):
  """Generates all possible diagnoses given the gcc error message."""

  diagnoses = []
  for diagnoser in _DIAGNOSERS:
    for diag in diagnoser(msg):
      diagnosis = '[%s - %s]\n%s' % diag
      if not diagnosis in diagnoses:
        diagnoses.append(diagnosis)
  return diagnoses


def main():
  print ('Google Mock Doctor v%s - '
         'diagnoses problems in code using Google Mock.' % _VERSION)

  if sys.stdin.isatty():
    print ('Please copy and paste the compiler errors here.  Press c-D when '
           'you are done:')
  else:
    print 'Waiting for compiler errors on stdin . . .'

  msg = sys.stdin.read().strip()
  diagnoses = Diagnose(msg)
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
