#!/usr/bin/env python
#
# Copyright 2006, Google Inc.
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

"""Unit test utilities for gtest_xml_output"""

__author__ = 'eefacm@gmail.com (Sean Mcafee)'

import re
import unittest

from xml.dom import minidom, Node

GTEST_OUTPUT_FLAG         = "--gtest_output"
GTEST_DEFAULT_OUTPUT_FILE = "test_detail.xml"

class GTestXMLTestCase(unittest.TestCase):
  """
  Base class for tests of Google Test's XML output functionality.
  """


  def _AssertEquivalentElements(self, expected_element, actual_element):
    """
    Asserts that actual_element (a DOM element object) is equivalent to
    expected_element (another DOM element object), in that it meets all
    of the following conditions:
    *  It has the same tag name as expected_element.
    *  It has the same set of attributes as expected_element, each with
       the same value as the corresponding attribute of expected_element.
       An exception is any attribute named "time", which need only be
       convertible to a long integer.
    *  Each child element is equivalent to a child element of
       expected_element.  Child elements are matched according to an
       attribute that varies depending on the element; "name" for
       <testsuite> and <testcase> elements, "message" for <failure>
       elements.  This matching is necessary because child elements are
       not guaranteed to be ordered in any particular way.
    """
    self.assertEquals(expected_element.tagName, actual_element.tagName)

    expected_attributes = expected_element.attributes
    actual_attributes   = actual_element  .attributes
    self.assertEquals(expected_attributes.length, actual_attributes.length)
    for i in range(expected_attributes.length):
      expected_attr = expected_attributes.item(i)
      actual_attr   = actual_attributes.get(expected_attr.name)
      self.assert_(actual_attr is not None)
      self.assertEquals(expected_attr.value, actual_attr.value)

    expected_child = self._GetChildElements(expected_element)
    actual_child   = self._GetChildElements(actual_element)
    self.assertEquals(len(expected_child), len(actual_child))
    for child_id, element in expected_child.iteritems():
      self.assert_(child_id in actual_child,
                   '<%s> is not in <%s>' % (child_id, actual_child))
      self._AssertEquivalentElements(element, actual_child[child_id])

  identifying_attribute = {
    "testsuite": "name",
    "testcase":  "name",
    "failure":   "message",
    }

  def _GetChildElements(self, element):
    """
    Fetches all of the Element type child nodes of element, a DOM
    Element object.  Returns them as the values of a dictionary keyed by
    the value of one of the node's attributes.  For <testsuite> and
    <testcase> elements, the identifying attribute is "name"; for
    <failure> elements, it is "message".  An exception is raised if
    any element other than the above three is encountered, if two
    child elements with the same identifying attributes are encountered,
    or if any other type of node is encountered, other than Text nodes
    containing only whitespace.
    """
    children = {}
    for child in element.childNodes:
      if child.nodeType == Node.ELEMENT_NODE:
        self.assert_(child.tagName in self.identifying_attribute,
                     "Encountered unknown element <%s>" % child.tagName)
        childID = child.getAttribute(self.identifying_attribute[child.tagName])
        self.assert_(childID not in children)
        children[childID] = child
      elif child.nodeType == Node.TEXT_NODE:
        self.assert_(child.nodeValue.isspace())
      else:
        self.fail("Encountered unexpected node type %d" % child.nodeType)
    return children

  def _NormalizeXml(self, element):
    """
    Normalizes Google Test's XML output to eliminate references to transient
    information that may change from run to run.

    *  The "time" attribute of <testsuite> and <testcase> elements is
       replaced with a single asterisk, if it contains only digit
       characters.
    *  The line number reported in the first line of the "message"
       attribute of <failure> elements is replaced with a single asterisk.
    *  The directory names in file paths are removed.
    """
    if element.tagName in ("testsuite", "testcase"):
      time = element.getAttributeNode("time")
      time.value = re.sub(r"^\d+$", "*", time.value)
    elif element.tagName == "failure":
      message = element.getAttributeNode("message")
      message.value = re.sub(r"^.*/(.*:)\d+\n", "\\1*\n", message.value)
    for child in element.childNodes:
      if child.nodeType == Node.ELEMENT_NODE:
        self._NormalizeXml(child)
