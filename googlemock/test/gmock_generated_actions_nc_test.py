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

# Google Mock - a framework for writing C++ mock classes.
#
# This file drives the negative compilation tests for script-generated
# Google Mock actions.

"""Driver for the NC tests for script-generated Google Mock actions."""

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


class GMockGeneratedActionTest(googletest.TestCase):
  """Negative compilation tests for generated Google Mock actions."""

  # The class body is intentionally empty.  The actual test*() methods
  # will be defined at run time by a call to
  # DefineNegativeCompilationTests() later.
  pass

# Defines a list of test specs, where each element is a tuple
# (test name, list of regexes for matching the compiler errors).
TEST_SPECS = [
    ("NULLARY_WITH_ARGS", [
        r"no matching function for call to 'WithArgs",
    ]),
    ("TOO_FEW_ARGS_FOR_WITH_ARGS", [
        r"no known conversion",
    ]),
    ("TOO_MANY_ARGS_FOR_WITH_ARGS", [
        r"no known conversion",
    ]),
    ("INCOMPATIBLE_ARG_TYPES_FOR_WITH_ARGS", [
        r"no known conversion",
    ]),
    ("WRONG_ARG_TYPE_IN_ACTION_MACRO", [
        r"invalid operands",
    ]),
    (
        "WRONG_RETURN_TYPE_IN_ACTION_MACRO",
        [
            r"invalid conversion",  # GCC
            r"cannot initialize return object",  # Clang
        ]),
    (
        "EXCESSIVE_ARG_IN_ACTION_MACRO",
        [
            r"no match for 'operator\+'",  # GCC
            r"invalid operands to binary expression",  # Clang
        ]),
    (
        "ACTION_MACRO_IN_CLASS",
        [
            r"cannot define member function.*Bar.*within.*Foo",  # GCC
            r"ACTION\(Bar\)",  # Clang
        ]),
    (
        "ACTION_MACRO_IN_FUNCTION",
        [
            r"invalid declaration of member template in local class",  # GCC
            r"templates cannot be declared inside of a local class",  # Clang
        ]),
    ("SET_ARG_REFEREE_MUST_BE_USED_WITH_REFERENCE",
     [r"Argument must be a reference type"]),
    (
        "DELETE_ARG_MUST_BE_USED_WITH_POINTER",
        [
            r"argument given to 'delete', expected pointer",  # GCC
            r"cannot delete expression of type",  # Clang
        ]),
    (
        "CANNOT_OVERLOAD_ACTION_TEMPLATE_ON_TEMPLATE_PARAM_NUMBER",
        [
            r"wrong number of template arguments",  # GCC
            r"too many template parameters",  # Clang
        ]),
    (
        "CANNOT_OVERLOAD_ACTION_AND_ACTION_TEMPLATE_W_SAME_VALUE_PS",
        [
            r"wrong number of template arguments",  # GCC
            r"too many template parameters",  # Clang
        ]),
    ("SANITY", None),
]

# Define a test method in GMockGeneratedActionTest for each element in
# TEST_SPECS.
fake_target_util.DefineNegativeCompilationTests(
    GMockGeneratedActionTest,
    "google3/third_party/googletest/googlemock/test/gmock-generated-actions_nc",  # fake target
    "gmock-generated-actions_nc.o",                               # object file
    TEST_SPECS)

if __name__ == "__main__":
  googletest.main()
