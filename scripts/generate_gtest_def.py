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

"""Generates the gtest.def file to build Google Test as a DLL on Windows.

SYNOPSIS
       Put diagnostic messages from building gtest_dll_test_.exe into
       BUILD_RESULTS_FILE and invoke

         generate_gtest_def.py <BUILD_RESULTS_FILE

       Reads output of VC++ linker and re-generates the src/gtest.def file
       required for building gtest as a DLL.

       Use this script if you modify Google Test's source code and Visual
       Studio linker starts complaining about unresolved external symbols.
       You may have to repeate the build/re-generate cycle several times
       because VC++ limits the number of unresolved external symbols it can
       report at a time.

EXAMPLES
         scons\scons.py | scripts\generate_gtest_def.py

This tool is experimental.  Please report any problems to
googletestframework@googlegroups.com.  You can read
http://code.google.com/p/googletest/wiki/GoogleTestAdvancedGuide for more
information.
"""

__author__ = 'vladl@google.com (Vlad Losev)'

import os
import re
import sets
import sys

# We assume that this file is in the scripts/ directory in the Google
# Test root directory.
GTEST_DEF_PATH = os.path.join(os.path.dirname(__file__), '../msvc/gtest.def')

# Locates the header of the EXPORTS section.
EXPORTS_SECTION_REGEX = re.compile(r'^EXPORTS\s*$', re.IGNORECASE)

# Determines if a line looks like an export definition in the EXPORTS
# section of a module definition file.
EXPORT_REGEX = re.compile(r'^\s+(\S+)')

# Determines if a given line contains an error message about unresolved
# linker symbol.
IS_UNRESOLVED_SYMBOL_REGEX = re.compile(r'\bunresolved external symbol\b')

# Fetches the symbol name from a line that contains an unresolved linker
# symbol message.
UNRESOLVED_SYMBOL_REGEX = re.compile(r'^.*?"[^"]+" \((\S+)\)')


def ReadDefExports(stream):
  """Reads contents of a def file and returns a list of exported symbols."""

  is_export = False
  exports = sets.Set()
  for line in stream:
    if EXPORTS_SECTION_REGEX.match(line):
      is_export = True
    elif EXPORT_REGEX.match(line):
      if is_export:
        exports.add(EXPORT_REGEX.match(line).group(1))
    else:
      is_export = False

  return exports


def ReadUnresolvedExternals(stream):
  """Reads linker output and returns list of unresolved linker symbols."""

  unresolved = sets.Set()

  for line in stream:
    if IS_UNRESOLVED_SYMBOL_REGEX.search(line):
      unresolved.add(UNRESOLVED_SYMBOL_REGEX.match(line).group(1))

  return unresolved


def AdjustExports(exports, unresolved):
  """Adjusts exports list based on the list of unresolved symbols."""

  if unresolved & exports:
    # There are symbols that are listed as exported but are also reported
    # unresolved. This is most likely because they have been removed from
    # Google Test but their mentions in gtest.def constitute references. We
    # need to remove such symbols from the EXPORTS section. Also, their
    # presence means that the Google Test DLL has failed to link and
    # consequently linking of the test .exe was not attempted, meaning that
    # at this time, there will be no unresolved externals that need to be
    # added to the exports list.
    exports -= unresolved
  else:
    # Finding unresolved exports means that the Google Test DLL had link
    # errors and the build script did not build gtest_dll_test_.exe. The user
    # has to build the test once again and run this script on the diagnostic
    # output of the build.
    exports |= unresolved

  return exports


def WriteGtestDefFile(stream, exports):
  """Writes contents of gtest.def given a list of exported symbols."""

  stream.write('; This file is auto-generated. DO NOT EDIT DIRECTLY.\n'
               '; For more information, see scripts/generate_gtest_def.py.\n'
               '\nLIBRARY\n'
               '\nEXPORTS\n')
  for symbol in sorted(exports):
    stream.write('  %s\n' % symbol)


def main():
  unresolved = ReadUnresolvedExternals(sys.stdin)
  if unresolved:
    try:
      gtest_def = open(GTEST_DEF_PATH, 'r')
      exports = ReadDefExports(gtest_def)
      gtest_def.close()
    except IOError:
      exports = sets.Set()

    exports = AdjustExports(exports, unresolved)
    WriteGtestDefFile(open(GTEST_DEF_PATH, 'w'), exports)
    sys.stderr.write('Updated gtest.def. Please clean the .dll file\n'
                     'produced by your Google Test DLL build, run the build\n'
                     'again and pass its diagnostic output to this script\n'
                     'unless the build succeeds.\n')
  else:
    sys.stderr.write('The build diagnostic output indicates no unresolved\n'
                     'externals.  gtest.def is likely up to date and\n'
                     'has not been updated.\n')

if __name__ == '__main__':
  main()
