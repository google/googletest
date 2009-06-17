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

"""Unit test for the gtest_xml_output module"""

__author__ = 'eefacm@gmail.com (Sean Mcafee)'

import errno
import os
import sys
from xml.dom import minidom, Node

import gtest_test_utils
import gtest_xml_test_utils


GTEST_OUTPUT_FLAG         = "--gtest_output"
GTEST_DEFAULT_OUTPUT_FILE = "test_detail.xml"

SUPPORTS_STACK_TRACES = False

if SUPPORTS_STACK_TRACES:
  STACK_TRACE_TEMPLATE = "\nStack trace:\n*"
else:
  STACK_TRACE_TEMPLATE = ""

EXPECTED_NON_EMPTY_XML = """<?xml version="1.0" encoding="UTF-8"?>
<testsuites tests="13" failures="2" disabled="2" errors="0" time="*" name="AllTests">
  <testsuite name="SuccessfulTest" tests="1" failures="0" disabled="0" errors="0" time="*">
    <testcase name="Succeeds" status="run" time="*" classname="SuccessfulTest"/>
  </testsuite>
  <testsuite name="FailedTest" tests="1" failures="1" disabled="0" errors="0" time="*">
    <testcase name="Fails" status="run" time="*" classname="FailedTest">
      <failure message="Value of: 2&#x0A;Expected: 1" type=""><![CDATA[gtest_xml_output_unittest_.cc:*
Value of: 2
Expected: 1%(stack)s]]></failure>
    </testcase>
  </testsuite>
  <testsuite name="MixedResultTest" tests="3" failures="1" disabled="1" errors="0" time="*">
    <testcase name="Succeeds" status="run" time="*" classname="MixedResultTest"/>
    <testcase name="Fails" status="run" time="*" classname="MixedResultTest">
      <failure message="Value of: 2&#x0A;Expected: 1" type=""><![CDATA[gtest_xml_output_unittest_.cc:*
Value of: 2
Expected: 1%(stack)s]]></failure>
      <failure message="Value of: 3&#x0A;Expected: 2" type=""><![CDATA[gtest_xml_output_unittest_.cc:*
Value of: 3
Expected: 2%(stack)s]]></failure>
    </testcase>
    <testcase name="DISABLED_test" status="notrun" time="*" classname="MixedResultTest"/>
  </testsuite>
  <testsuite name="DisabledTest" tests="1" failures="0" disabled="1" errors="0" time="*">
    <testcase name="DISABLED_test_not_run" status="notrun" time="*" classname="DisabledTest"/>
  </testsuite>
  <testsuite name="PropertyRecordingTest" tests="4" failures="0" disabled="0" errors="0" time="*">
    <testcase name="OneProperty" status="run" time="*" classname="PropertyRecordingTest" key_1="1"/>
    <testcase name="IntValuedProperty" status="run" time="*" classname="PropertyRecordingTest" key_int="1"/>
    <testcase name="ThreeProperties" status="run" time="*" classname="PropertyRecordingTest" key_1="1" key_2="2" key_3="3"/>
    <testcase name="TwoValuesForOneKeyUsesLastValue" status="run" time="*" classname="PropertyRecordingTest" key_1="2"/>
  </testsuite>
  <testsuite name="NoFixtureTest" tests="3" failures="0" disabled="0" errors="0" time="*">
     <testcase name="RecordProperty" status="run" time="*" classname="NoFixtureTest" key="1"/>
     <testcase name="ExternalUtilityThatCallsRecordIntValuedProperty" status="run" time="*" classname="NoFixtureTest" key_for_utility_int="1"/>
     <testcase name="ExternalUtilityThatCallsRecordStringValuedProperty" status="run" time="*" classname="NoFixtureTest" key_for_utility_string="1"/>
  </testsuite>
</testsuites>""" % {'stack': STACK_TRACE_TEMPLATE}


EXPECTED_EMPTY_XML = """<?xml version="1.0" encoding="UTF-8"?>
<testsuites tests="0" failures="0" disabled="0" errors="0" time="*" name="AllTests">
</testsuites>"""


class GTestXMLOutputUnitTest(gtest_xml_test_utils.GTestXMLTestCase):
  """
  Unit test for Google Test's XML output functionality.
  """

  def testNonEmptyXmlOutput(self):
    """
    Runs a test program that generates a non-empty XML output, and
    tests that the XML output is expected.
    """
    self._TestXmlOutput("gtest_xml_output_unittest_",
                        EXPECTED_NON_EMPTY_XML, 1)

  def testEmptyXmlOutput(self):
    """
    Runs a test program that generates an empty XML output, and
    tests that the XML output is expected.
    """

    self._TestXmlOutput("gtest_no_test_unittest",
                        EXPECTED_EMPTY_XML, 0)

  def testDefaultOutputFile(self):
    """
    Confirms that Google Test produces an XML output file with the expected
    default name if no name is explicitly specified.
    """
    output_file = os.path.join(gtest_test_utils.GetTempDir(),
                               GTEST_DEFAULT_OUTPUT_FILE)
    gtest_prog_path = gtest_test_utils.GetTestExecutablePath(
        "gtest_no_test_unittest")
    try:
      os.remove(output_file)
    except OSError, e:
      if e.errno != errno.ENOENT:
        raise

    p = gtest_test_utils.Subprocess(
        [gtest_prog_path, "%s=xml" % GTEST_OUTPUT_FLAG],
        working_dir=gtest_test_utils.GetTempDir())
    self.assert_(p.exited)
    self.assertEquals(0, p.exit_code)
    self.assert_(os.path.isfile(output_file))


  def _TestXmlOutput(self, gtest_prog_name, expected_xml, expected_exit_code):
    """
    Asserts that the XML document generated by running the program
    gtest_prog_name matches expected_xml, a string containing another
    XML document.  Furthermore, the program's exit code must be
    expected_exit_code.
    """
    xml_path = os.path.join(gtest_test_utils.GetTempDir(),
                            gtest_prog_name + "out.xml")
    gtest_prog_path = gtest_test_utils.GetTestExecutablePath(gtest_prog_name)

    command = [gtest_prog_path, "%s=xml:%s" % (GTEST_OUTPUT_FLAG, xml_path)]
    p = gtest_test_utils.Subprocess(command)
    if p.terminated_by_signal:
      self.assert_(False,
                   "%s was killed by signal %d" % (gtest_prog_name, p.signal))
    else:
      self.assert_(p.exited)
      self.assertEquals(expected_exit_code, p.exit_code,
                        "'%s' exited with code %s, which doesn't match "
                        "the expected exit code %s."
                        % (command, p.exit_code, expected_exit_code))

    expected = minidom.parseString(expected_xml)
    actual   = minidom.parse(xml_path)
    self.NormalizeXml(actual.documentElement)
    self.AssertEquivalentNodes(expected.documentElement,
                               actual.documentElement)
    expected.unlink()
    actual  .unlink()



if __name__ == '__main__':
  os.environ['GTEST_STACK_TRACE_DEPTH'] = '1'
  gtest_test_utils.Main()
