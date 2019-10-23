#!/usr/bin/env python
#
# Copyright 2007, Google Inc.
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

"""C++ keywords and helper utilities for determining keywords."""

try:
    # Python 3.x
    import builtins
except ImportError:
    # Python 2.x
    import __builtin__ as builtins


if not hasattr(builtins, 'set'):
    # Nominal support for Python 2.3.
    from sets import Set as set


TYPES = set('bool char int long short double float void wchar_t unsigned signed'.split())
TYPE_MODIFIERS = set('auto register const inline extern static virtual volatile mutable'.split())
ACCESS = set('public protected private friend'.split())

CASTS = set('static_cast const_cast dynamic_cast reinterpret_cast'.split())

OTHERS = set('true false asm class namespace using explicit this operator sizeof'.split())
OTHER_TYPES = set('new delete typedef struct union enum typeid typename template'.split())

CONTROL = set('case switch default if else return goto'.split())
EXCEPTION = set('try catch throw'.split())
LOOP = set('while do for break continue'.split())

ALL = TYPES | TYPE_MODIFIERS | ACCESS | CASTS | OTHERS | OTHER_TYPES | CONTROL | EXCEPTION | LOOP


def IsKeyword(token):
    return token in ALL

def IsBuiltinType(token):
    if token in ('virtual', 'inline'):
        # These only apply to methods, they can't be types by themselves.
        return False
    return token in TYPES or token in TYPE_MODIFIERS
