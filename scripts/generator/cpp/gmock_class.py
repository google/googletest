#!/usr/bin/env python
#
# Copyright 2008 Google Inc.
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

"""Generate a Google Mock class from a production class.

This program will read in a C++ source file and output the Google Mock class
for the specified class.

Usage:
  gmock_class.py header-file.h ClassName

Output is sent to stdout.
"""

__author__ = 'nnorwitz@google.com (Neal Norwitz)'


import os
import re
import sys

from cpp import ast
from cpp import utils

# How many spaces to indent.  Can me set with INDENT environment variable.
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
        return_type = node.return_type.name
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
        args = re.sub('  +', ' ', source[start:end].replace('\n', ''))

      # Create the prototype.
      indent = ' ' * _INDENT
      line = ('%s%s(%s,\n%s%s(%s));' %
              (indent, prefix, node.name, indent*3, return_type, args))
      output_lines.append(line)


def _GenerateMock(filename, source, ast_list, class_name):
  lines = []
  for node in ast_list:
    if isinstance(node, ast.Class) and node.body and node.name == class_name:
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

  if lines:
    sys.stdout.write('\n'.join(lines))
  else:
    sys.stderr.write('Class %s not found\n' % class_name)


def main(argv=sys.argv):
  if len(argv) != 3:
    sys.stdout.write(__doc__)
    return 1

  global _INDENT
  try:
    _INDENT = int(os.environ['INDENT'])
  except KeyError:
    pass
  except:
    sys.stderr.write('Unable to use indent of %s\n' % os.environ.get('INDENT'))

  filename, class_name = argv[1:]
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
    _GenerateMock(filename, source, entire_ast, class_name)


if __name__ == '__main__':
  main(sys.argv)
