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

# Google Mock - a framework for writing C++ mock classes.
#
# This file drives the negative compilation tests for script-generated
# Google Mock matchers.

"""Driver for the NC tests for script-generated Google Mock matchers."""

import os
import sys

IS_LINUX = os.name == "posix" and os.uname()[0] == "Linux"
if not IS_LINUX:
  sys.stderr.write(
      "WARNING: Negative compilation tests are not supported on this platform")
  sys.exit(0)

# Suppresses the 'Import not at the top of the file' lint complaint.
# pylint: disable-msg=C6204
from google3.testing.pybase import fake_target_util
from google3.testing.pybase import googletest
# pylint: enable-msg=C6204


class GMockGeneratedMatcherTest(googletest.TestCase):
  """Negative compilation tests for generated Google Mock matchers."""

  def testCompilerErrors(self):
    """Verifies that erroneous code leads to expected compiler messages."""

    # Defines a list of test specs, where each element is a tuple
    # (test name, list of regexes for matching the compiler errors).
    test_specs = [
        ("WRONG_ARG_TYPE_IN_MATCHER_MACRO",
         [r"invalid operands",
         ]),
        ("MATCHER_MACRO_IN_CLASS",
         [r"cannot define member function.*Bar.*within.*Foo",  # GCC
          r"MATCHER\(Bar, .*\)",                               # Clang
         ]),
        ("MATCHER_MACRO_IN_FUNCTION",
         [r"invalid declaration of member template in local class",  # GCC
          r"templates cannot be declared inside of a local class",   # Clang
         ]),
        ("SANITY",
         None),
        ]

    fake_target_util.AssertCcCompilerErrors(
        self,                                      # The current test case.
        "google3/third_party/googletest/googlemock/test/"
        "gmock-generated-matchers_nc",             # The fake target file.
        "gmock-generated-matchers_nc.o",           # The sub-target to build.
        test_specs                                 # List of test specs.
        )

if __name__ == "__main__":
  googletest.main()
