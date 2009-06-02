#!/usr/bin/env python
#
# Copyright 2008 Google Inc.  All Rights Reserved.
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

"""Generate Google Mock classes from base classes.

This program will read in a C++ source file and output the Google Mock
classes for the specified classes.  If no class is specified, all
classes in the source file are emitted.

Usage:
  gmock_class.py header-file.h [ClassName]...

Output is sent to stdout.
"""

__author__ = 'nnorwitz@google.com (Neal Norwitz)'


import os
import re
import sets
import sys

from cpp import ast
from cpp import utils

_VERSION = (1, 0, 1)  # The version of this script.
# How many spaces to indent.  Can set me with the INDENT environment variable.
_INDENT = 2


def _GenerateMethods(output_lines, source, class_node):
  function_type = ast.FUNCTION_VIRTUAL | ast.FUNCTION_PURE_VIRTUAL
  ctor_or_dtor = ast.FUNCTION_CTOR | ast.FUNCTION_DTOR

  for node in class_node.body:
    # We only care about virtual functions.
    if (isinstance(node, ast.Function) and
        node.modifiers & function_type and
        not node.modifiers & ctor_or_dtor):
      # Pick out all the elements we need from the original function.
      const = ''
      if node.modifiers & ast.FUNCTION_CONST:
        const = 'CONST_'
      return_type = 'void'
      if node.return_type:
        # Add modifiers like 'const'.
        modifiers = ''
        if node.return_type.modifiers:
          modifiers = ' '.join(node.return_type.modifiers) + ' '
        return_type = modifiers + node.return_type.name
        if node.return_type.pointer:
          return_type += '*'
        if node.return_type.reference:
          return_type += '&'
      prefix = 'MOCK_%sMETHOD%d' % (const, len(node.parameters))
      args = ''
      if node.parameters:
        # Get the full text of the parameters from the start
        # of the first parameter to the end of the last parameter.
        start = node.parameters[0].start
        end = node.parameters[-1].end
        # Remove // comments.
        args_strings = re.sub(r'//.*', '', source[start:end])
        # Condense multiple spaces and eliminate newlines putting the
        # parameters together on a single line.  Ensure there is a
        # space in an argument which is split by a newline without
        # intervening whitespace, e.g.: int\nBar
        args = re.sub('  +', ' ', args_strings.replace('\n', ' '))

      # Create the prototype.
      indent = ' ' * _INDENT
      line = ('%s%s(%s,\n%s%s(%s));' %
              (indent, prefix, node.name, indent*3, return_type, args))
      output_lines.append(line)


def _GenerateMocks(filename, source, ast_list, desired_class_names):
  processed_class_names = sets.Set()
  lines = []
  for node in ast_list:
    if (isinstance(node, ast.Class) and node.body and
        # desired_class_names being None means that all classes are selected.
        (not desired_class_names or node.name in desired_class_names)):
      class_name = node.name
      processed_class_names.add(class_name)
      class_node = node
      # Add namespace before the class.
      if class_node.namespace:
        lines.extend(['namespace %s {' % n for n in class_node.namespace])  # }
        lines.append('')

      # Add the class prolog.
      lines.append('class Mock%s : public %s {' % (class_name, class_name))  # }
      lines.append('%spublic:' % (' ' * (_INDENT // 2)))

      # Add all the methods.
      _GenerateMethods(lines, source, class_node)

      # Close the class.
      if lines:
        # If there are no virtual methods, no need for a public label.
        if len(lines) == 2:
          del lines[-1]

        # Only close the class if there really is a class.
        lines.append('};')
        lines.append('')  # Add an extra newline.

      # Close the namespace.
      if class_node.namespace:
        for i in range(len(class_node.namespace)-1, -1, -1):
          lines.append('}  // namespace %s' % class_node.namespace[i])
        lines.append('')  # Add an extra newline.

  if desired_class_names:
    missing_class_name_list = list(desired_class_names - processed_class_names)
    if missing_class_name_list:
      missing_class_name_list.sort()
      sys.stderr.write('Class(es) not found in %s: %s\n' %
                       (filename, ', '.join(missing_class_name_list)))
  elif not processed_class_names:
    sys.stderr.write('No class found in %s\n' % filename)

  return lines


def main(argv=sys.argv):
  if len(argv) < 2:
    sys.stderr.write('Google Mock Class Generator v%s\n\n' %
                     '.'.join(map(str, _VERSION)))
    sys.stderr.write(__doc__)
    return 1

  global _INDENT
  try:
    _INDENT = int(os.environ['INDENT'])
  except KeyError:
    pass
  except:
    sys.stderr.write('Unable to use indent of %s\n' % os.environ.get('INDENT'))

  filename = argv[1]
  desired_class_names = None  # None means all classes in the source file.
  if len(argv) >= 3:
    desired_class_names = sets.Set(argv[2:])
  source = utils.ReadFile(filename)
  if source is None:
    return 1

  builder = ast.BuilderFromSource(source, filename)
  try:
    entire_ast = filter(None, builder.Generate())
  except KeyboardInterrupt:
    return
  except:
    # An error message was already printed since we couldn't parse.
    pass
  else:
    lines = _GenerateMocks(filename, source, entire_ast, desired_class_names)
    sys.stdout.write('\n'.join(lines))


if __name__ == '__main__':
  main(sys.argv)
