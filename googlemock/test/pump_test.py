#!/usr/bin/env python
#
# Copyright 2010, Google Inc.
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

"""Tests for the Pump meta-programming tool."""

from google3.testing.pybase import googletest
import google3.third_party.googletest.googlemock.scripts.pump

pump = google3.third_party.googletest.googlemock.scripts.pump
Convert = pump.ConvertFromPumpSource
StripMetaComments = pump.StripMetaComments


class PumpTest(googletest.TestCase):

  def testConvertsEmptyToEmpty(self):
    self.assertEquals('', Convert('').strip())

  def testConvertsPlainCodeToSame(self):
    self.assertEquals('#include <stdio.h>\n',
                      Convert('#include <stdio.h>\n'))

  def testConvertsLongIWYUPragmaToSame(self):
    long_line = '// IWYU pragma: private, include "' + (80*'a') + '.h"\n'
    self.assertEquals(long_line, Convert(long_line))

  def testConvertsIWYUPragmaWithLeadingSpaceToSame(self):
    long_line = ' // IWYU pragma: private, include "' + (80*'a') + '.h"\n'
    self.assertEquals(long_line, Convert(long_line))

  def testConvertsIWYUPragmaWithSlashStarLeaderToSame(self):
    long_line = '/* IWYU pragma: private, include "' + (80*'a') + '.h"\n'
    self.assertEquals(long_line, Convert(long_line))

  def testConvertsIWYUPragmaWithSlashStarAndSpacesToSame(self):
    long_line = ' /* IWYU pragma: private, include "' + (80*'a') + '.h"\n'
    self.assertEquals(long_line, Convert(long_line))

  def testIgnoresMetaComment(self):
    self.assertEquals('',
                      Convert('$$ This is a Pump meta comment.\n').strip())

  def testSimpleVarDeclarationWorks(self):
    self.assertEquals('3\n',
                      Convert('$var m = 3\n'
                              '$m\n'))

  def testVarDeclarationCanReferenceEarlierVar(self):
    self.assertEquals('43 != 3;\n',
                      Convert('$var a = 42\n'
                              '$var b = a + 1\n'
                              '$var c = (b - a)*3\n'
                              '$b != $c;\n'))

  def testSimpleLoopWorks(self):
    self.assertEquals('1, 2, 3, 4, 5\n',
                      Convert('$var n = 5\n'
                              '$range i 1..n\n'
                              '$for i, [[$i]]\n'))

  def testSimpleLoopWithCommentWorks(self):
    self.assertEquals('1, 2, 3, 4, 5\n',
                      Convert('$var n = 5    $$ This is comment 1.\n'
                              '$range i 1..n $$ This is comment 2.\n'
                              '$for i, [[$i]]\n'))

  def testNonTrivialRangeExpressionsWork(self):
    self.assertEquals('1, 2, 3, 4\n',
                      Convert('$var n = 5\n'
                              '$range i (n/n)..(n - 1)\n'
                              '$for i, [[$i]]\n'))

  def testLoopWithoutSeparatorWorks(self):
    self.assertEquals('a + 1 + 2 + 3;\n',
                      Convert('$range i 1..3\n'
                              'a$for i [[ + $i]];\n'))

  def testCanGenerateDollarSign(self):
    self.assertEquals('$\n', Convert('$($)\n'))

  def testCanIterpolateExpressions(self):
    self.assertEquals('a[2] = 3;\n',
                      Convert('$var i = 1\n'
                              'a[$(i + 1)] = $(i*4 - 1);\n'))

  def testConditionalWithoutElseBranchWorks(self):
    self.assertEquals('true\n',
                      Convert('$var n = 5\n'
                              '$if n > 0 [[true]]\n'))

  def testConditionalWithElseBranchWorks(self):
    self.assertEquals('true -- really false\n',
                      Convert('$var n = 5\n'
                              '$if n > 0 [[true]]\n'
                              '$else [[false]] -- \n'
                              '$if n > 10 [[really true]]\n'
                              '$else [[really false]]\n'))

  def testConditionalWithCascadingElseBranchWorks(self):
    self.assertEquals('a\n',
                      Convert('$var n = 5\n'
                              '$if n > 0 [[a]]\n'
                              '$elif n > 10 [[b]]\n'
                              '$else [[c]]\n'))
    self.assertEquals('b\n',
                      Convert('$var n = 5\n'
                              '$if n > 10 [[a]]\n'
                              '$elif n > 0 [[b]]\n'
                              '$else [[c]]\n'))
    self.assertEquals('c\n',
                      Convert('$var n = 5\n'
                              '$if n > 10 [[a]]\n'
                              '$elif n > 8 [[b]]\n'
                              '$else [[c]]\n'))

  def testNestedLexicalBlocksWork(self):
    self.assertEquals('a = 5;\n',
                      Convert('$var n = 5\n'
                              'a = [[$if n > 0 [[$n]]]];\n'))


class StripMetaCommentsTest(googletest.TestCase):

  def testReturnsSameStringIfItContainsNoComment(self):
    self.assertEquals('', StripMetaComments(''))
    self.assertEquals(' blah ', StripMetaComments(' blah '))
    self.assertEquals('A single $ is fine.',
                      StripMetaComments('A single $ is fine.'))
    self.assertEquals('multiple\nlines',
                      StripMetaComments('multiple\nlines'))

  def testStripsSimpleComment(self):
    self.assertEquals('yes\n', StripMetaComments('yes $$ or no?\n'))

  def testStripsSimpleCommentWithMissingNewline(self):
    self.assertEquals('yes', StripMetaComments('yes $$ or no?'))

  def testStripsPureCommentLinesEntirely(self):
    self.assertEquals('yes\n',
                      StripMetaComments('$$ a pure comment line.\n'
                                        'yes $$ or no?\n'
                                        '    $$ another comment line.\n'))

  def testStripsCommentsFromMultiLineText(self):
    self.assertEquals('multi-\n'
                      'line\n'
                      'text is fine.',
                      StripMetaComments('multi- $$ comment 1\n'
                                        'line\n'
                                        'text is fine. $$ comment 2'))


if __name__ == '__main__':
  googletest.main()
