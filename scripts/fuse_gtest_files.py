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

"""fuse_gtest_files.py v0.1.0
Fuses Google Test source code into a .h file and a .cc file.

SYNOPSIS
       fuse_gtest_files.py [GTEST_ROOT_DIR] OUTPUT_DIR

       Scans GTEST_ROOT_DIR for Google Test source code, and generates
       two files: OUTPUT_DIR/gtest/gtest.h and OUTPUT_DIR/gtest/gtest-all.cc.
       Then you can build your tests by adding OUTPUT_DIR to the include
       search path and linking with OUTPUT_DIR/gtest/gtest-all.cc.  These
       two files contain everything you need to use Google Test.  Hence
       you can "install" Google Test by copying them to wherever you want.

       GTEST_ROOT_DIR can be omitted and defaults to the parent directory
       of the directory holding the fuse_gtest_files.py script.

EXAMPLES
       ./fuse_gtest_files.py fused_gtest
       ./fuse_gtest_files.py path/to/unpacked/gtest fused_gtest

This tool is experimental.  In particular, it assumes that there is no
conditional inclusion of Google Test headers.  Please report any
problems to googletestframework@googlegroups.com.  You can read
http://code.google.com/p/googletest/wiki/GoogleTestAdvancedGuide for
more information.
"""

__author__ = 'wan@google.com (Zhanyong Wan)'

import os
import re
import sets
import sys

# Regex for matching '#include <gtest/...>'.
INCLUDE_GTEST_FILE_REGEX = re.compile(r'^\s*#\s*include\s*<(gtest/.+)>')

# Regex for matching '#include "src/..."'.
INCLUDE_SRC_FILE_REGEX = re.compile(r'^\s*#\s*include\s*"(src/.+)"')

# Where to find the source files.
GTEST_H_SEED = 'include/gtest/gtest.h'
GTEST_SPI_H_SEED = 'include/gtest/gtest-spi.h'
GTEST_ALL_CC_SEED = 'src/gtest-all.cc'

# Where to put the generated files.
GTEST_H_OUTPUT = 'gtest/gtest.h'
GTEST_ALL_CC_OUTPUT = 'gtest/gtest-all.cc'


def GetGTestRootDir():
  """Returns the absolute path to the Google Test root directory.

  We assume that this script is in a sub-directory of the Google Test root.
  """

  my_path = sys.argv[0]  # Path to this script.
  my_dir = os.path.dirname(my_path)
  if not my_dir:
    my_dir = '.'

  return os.path.abspath(os.path.join(my_dir, '..'))


def ValidateGTestRootDir(gtest_root):
  """Makes sure gtest_root points to a valid gtest root directory.

  The function aborts the program on failure.
  """

  def VerifyFileExists(relative_path):
    """Verifies that the given file exists; aborts on failure.

    relative_path is the file path relative to the gtest root.
    """

    if not os.path.isfile(os.path.join(gtest_root, relative_path)):
      print 'ERROR: Cannot find %s in directory %s.' % (relative_path,
                                                        gtest_root)
      print ('Please either specify a valid Google Test root directory '
             'or omit it on the command line.')
      sys.exit(1)

  VerifyFileExists(GTEST_H_SEED)
  VerifyFileExists(GTEST_ALL_CC_SEED)


def ValidateOutputDir(output_dir):
  """Makes sure output_dir points to a valid output directory.

  The function aborts the program on failure.
  """

  def VerifyOutputFile(relative_path):
    """Verifies that the given output file path is valid.

    relative_path is relative to the output_dir directory.
    """

    # Makes sure the output file either doesn't exist or can be overwritten.
    output_file = os.path.join(output_dir, relative_path)
    if os.path.exists(output_file):
      print ('%s already exists in directory %s - overwrite it? (y/N) ' %
             (relative_path, output_dir))
      answer = sys.stdin.readline().strip()
      if answer not in ['y', 'Y']:
        print 'ABORTED.'
        sys.exit(1)

    # Makes sure the directory holding the output file exists; creates
    # it and all its ancestors if necessary.
    parent_directory = os.path.dirname(output_file)
    if not os.path.isdir(parent_directory):
      os.makedirs(parent_directory)

  VerifyOutputFile(GTEST_H_OUTPUT)
  VerifyOutputFile(GTEST_ALL_CC_OUTPUT)


def FuseGTestH(gtest_root, output_dir):
  """Scans folder gtest_root to generate gtest/gtest.h in output_dir."""

  output_file = file(os.path.join(output_dir, GTEST_H_OUTPUT), 'w')
  processed_files = sets.Set()  # Holds all gtest headers we've processed.

  def ProcessFile(gtest_header_path):
    """Processes the given gtest header file."""

    # We don't process the same header twice.
    if gtest_header_path in processed_files:
      return

    processed_files.add(gtest_header_path)

    # Reads each line in the given gtest header.
    for line in file(os.path.join(gtest_root, gtest_header_path), 'r'):
      m = INCLUDE_GTEST_FILE_REGEX.match(line)
      if m:
        # It's '#include <gtest/...>' - let's process it recursively.
        ProcessFile('include/' + m.group(1))
      else:
        # Otherwise we copy the line unchanged to the output file.
        output_file.write(line)

  ProcessFile(GTEST_H_SEED)
  output_file.close()


def FuseGTestAllCc(gtest_root, output_dir):
  """Scans folder gtest_root to generate gtest/gtest-all.cc in output_dir."""

  output_file = file(os.path.join(output_dir, GTEST_ALL_CC_OUTPUT), 'w')
  processed_files = sets.Set()

  def ProcessFile(gtest_source_file):
    """Processes the given gtest source file."""

    # We don't process the same #included file twice.
    if gtest_source_file in processed_files:
      return

    processed_files.add(gtest_source_file)

    # Reads each line in the given gtest source file.
    for line in file(os.path.join(gtest_root, gtest_source_file), 'r'):
      m = INCLUDE_GTEST_FILE_REGEX.match(line)
      if m:
        if 'include/' + m.group(1) == GTEST_SPI_H_SEED:
          # It's '#include <gtest/gtest-spi.h>'.  This file is not
          # #included by <gtest/gtest.h>, so we need to process it.
          ProcessFile(GTEST_SPI_H_SEED)
        else:
          # It's '#include <gtest/foo.h>' where foo is not gtest-spi.
          # We treat it as '#include <gtest/gtest.h>', as all other
          # gtest headers are being fused into gtest.h and cannot be
          # #included directly.

          # There is no need to #include <gtest/gtest.h> more than once.
          if not GTEST_H_SEED in processed_files:
            processed_files.add(GTEST_H_SEED)
            output_file.write('#include <%s>\n' % (GTEST_H_OUTPUT,))
      else:
        m = INCLUDE_SRC_FILE_REGEX.match(line)
        if m:
          # It's '#include "src/foo"' - let's process it recursively.
          ProcessFile(m.group(1))
        else:
          output_file.write(line)

  ProcessFile(GTEST_ALL_CC_SEED)
  output_file.close()


def FuseGTest(gtest_root, output_dir):
  ValidateGTestRootDir(gtest_root)
  ValidateOutputDir(output_dir)

  FuseGTestH(gtest_root, output_dir)
  FuseGTestAllCc(gtest_root, output_dir)


def main():
  argc = len(sys.argv)
  if argc == 2:
    # fuse_gtest_files.py OUTPUT_DIR
    FuseGTest(GetGTestRootDir(), sys.argv[1])
  elif argc == 3:
    # fuse_gtest_files.py GTEST_ROOT_DIR OUTPUT_DIR
    FuseGTest(sys.argv[1], sys.argv[2])
  else:
    print __doc__
    sys.exit(1)


if __name__ == '__main__':
  main()
