#!/usr/bin/python
################################################################################
## @file
## Automated Unit Test Run Script
##
## @copyright
## Copyright (c) 2015 Intel Corporation. All rights reserved
## SPDX-License-Identifier: BSD-2-Clause-Patent
################################################################################

import os
import sys
from subprocess import call, check_call
from xml.etree import ElementTree
import traceback

#
# Constant Strings
#
WIN = "win"
MAC = "mac"
LINUX = "linux"

TEST_SUITE_TAG = "testsuite"
TEST_CASE_TAG = "testcase"
FAILURE_TAG = "failure"
NAME_TAG = "name"

#
# Environment detection
#
if sys.platform == "win32":
    ostype = WIN
elif sys.platform == "darwin":
    ostype = MAC
elif sys.platform.startswith("linux"):
    ostype = LINUX
elif os.name == "posix":
    print("Warning: Unrecognized UNIX OS... treating as Linux")
    ostype = LINUX
else:
    raise EnvironmentError("Unsupported OS")

if ostype == WIN:
    from ctypes import windll, Structure, c_short, c_ushort, byref

    #Definitions to call Win32 API to change console colors
    class COORD(Structure):
        _fields_ = [("X", c_short), ("Y", c_short)]
    class SMALL_RECT(Structure):
        _fields_ = [("Left", c_short), ("Top", c_short), ("Right", c_short), ("Bottom", c_short)]
    class CONSOLE_SCREEN_BUFFER_INFO(Structure):
        _fields_ = [("dwSize", COORD), ("dwCursorPosition", COORD), ("wAttributes", c_ushort), ("srWindow", SMALL_RECT), ("dwMaximumWindowSize", COORD)]
    STD_OUTPUT_HANDLE          = -11
    GREEN                      = 0x000A
    RED                        = 0x000C
    StdoutHandle               = windll.kernel32.GetStdHandle(STD_OUTPUT_HANDLE)
    SetConsoleTextAttribute    = windll.kernel32.SetConsoleTextAttribute
    GetConsoleScreenBufferInfo = windll.kernel32.GetConsoleScreenBufferInfo

def SetXmlTestSuiteName (XmlFilePath, XmlTestSuiteName):
    tree = ElementTree.parse(XmlFilePath)
    root = tree.getroot()
    if root.tag != TEST_SUITE_TAG:
        raise ValueError ("Root tag is not %s"%TEST_SUITE_TAG)
    root.set(NAME_TAG, XmlTestSuiteName)
    tree.write(XmlFilePath)

def ParseXmlFile (XmlFilePath):
    testsuites = {}
    tree = ElementTree.parse(XmlFilePath)
    root = tree.getroot()
    for testCase in root.findall(TEST_CASE_TAG):
        name = testCase.get(NAME_TAG)
        (test_suite, test_case, unit_test) = name.split(".")
        testpass = True
        message = ""
        for failure in testCase.findall(FAILURE_TAG):
            testpass = False
            message = failure.text.strip()
        if test_suite not in testsuites:
            testsuites[test_suite] = {}
        if test_case not in testsuites[test_suite]:
            testsuites[test_suite][test_case] = {}
        testsuites[test_suite][test_case][unit_test] = { "Pass": testpass, "FailureMessage": message }
    return testsuites

def DisplayTestResults(TestResults):
    total_tests = 0
    total_failures = 0
    test_suites = list(TestResults.keys())
    test_suites.sort()
    for test_suite in test_suites:
        print("Test Suite: %s"%test_suite)
        test_cases = list(TestResults[test_suite].keys())
        test_cases.sort()
        for test_case in test_cases:
            print("  Test Case: %s"%test_case)
            unit_tests = list(TestResults[test_suite][test_case].keys())
            unit_tests.sort()
            for unit_test in unit_tests:
                total_tests += 1
                sys.stdout.write("    %s: "%unit_test)
                sys.stdout.write(" " * (67 - len(unit_test)))
                if TestResults[test_suite][test_case][unit_test]["Pass"]:
                    _PrintPassed()
                    print("")
                else:
                    total_failures += 1
                    _PrintFailed()
                    print("")
                    print("      %s"%TestResults[test_suite][test_case][unit_test]["FailureMessage"])
    if total_tests > 0:
        percentage = (1 - float(total_failures)/total_tests) * 100
    else:
        percentage = 100.0
    print("%d tests run, %d tests failed, %.2f%% Passing"%(total_tests, total_failures, percentage))

def _PrintPassed():
    sys.stdout.flush()
    if ostype == WIN:
        csbi = CONSOLE_SCREEN_BUFFER_INFO()
        GetConsoleScreenBufferInfo(StdoutHandle, byref(csbi))
        original_color = csbi.wAttributes
        SetConsoleTextAttribute(StdoutHandle, GREEN)
    sys.stdout.write("Passed")
    sys.stdout.flush()
    if ostype == WIN:
        SetConsoleTextAttribute(StdoutHandle, original_color)

def _PrintFailed():
    sys.stdout.flush()
    if ostype == WIN:
        csbi = CONSOLE_SCREEN_BUFFER_INFO()
        GetConsoleScreenBufferInfo(StdoutHandle, byref(csbi))
        original_color = csbi.wAttributes
        SetConsoleTextAttribute(StdoutHandle, RED)
    sys.stdout.write("Failed")
    sys.stdout.flush()
    if ostype == WIN:
        SetConsoleTextAttribute(StdoutHandle, original_color)

def _CopyFdFiles ():
    path = os.path.join("..", "..", "Build")
    path = os.path.join(path, os.environ["BUILD_DIR_NAME"])
    path = os.path.join(path, "%s_%s"%(os.environ["BUILD_TARGET"], os.environ["TOOL_CHAIN_TAG"]))
    path = os.path.join(path, "FV")
    path = os.path.abspath(path)
    _CopyFile(os.path.join(path, "UNITTEST32.fd"), os.path.join("IA32", "UNITTEST.fd"))
    _CopyFile(os.path.join(path, "UNITTEST64.fd"), os.path.join("X64", "UNITTEST.fd"))

def _RunSecMain ():
    path = os.path.abspath("IA32")
    check_call(os.path.join(path, "SecMain.exe"), cwd=path)
    path = os.path.abspath("X64")
    check_call(os.path.join(path, "SecMain.exe"), cwd=path)

def _MoveXmlFiles ():
    path = os.path.abspath("IA32")
    if os.path.isfile(os.path.join(path, "TestResults.xml")):
        _MoveFile(os.path.join(path, "TestResults.xml"), "TestResults32.xml")
        SetXmlTestSuiteName ("TestResults32.xml", "IA32-UnitTests")
    path = os.path.abspath("X64")
    if os.path.isfile(os.path.join(path, "TestResults.xml")):
        _MoveFile(os.path.join(path, "TestResults.xml"), "TestResults64.xml")
        SetXmlTestSuiteName ("TestResults64.xml", "X64-UnitTests")

def _DisplayXmlOutput ():
    if os.path.isfile("TestResults32.xml"):
        TestResults = ParseXmlFile("TestResults32.xml")
    else:
        TestResults = {}
    if os.path.isfile("TestResults64.xml"):
        TestResults.update(ParseXmlFile("TestResults64.xml"))
    DisplayTestResults(TestResults)

def _DeleteFdFiles ():
    _DeleteFile(os.path.join("IA32", "UNITTEST.fd"))
    _DeleteFile(os.path.join("X64", "UNITTEST.fd"))

#
# Filesystem Functions
#
def _DeleteFile(FilePath):
    if ostype == WIN:
        check_call("del /F %s"%FilePath, shell=True)
    else:
        check_call("rm -f %s"%FilePath, shell=True)

def _MoveFile(Source, Destination):
    if ostype == WIN:
        check_call("move /Y %s %s"%(Source, Destination), shell=True)
    else:
        check_call("mv -f %s %s"%(Source, Destination), shell=True)

def _CopyFile(Source, Destination):
    if ostype == WIN:
        check_call("copy /B /Y %s %s"%(Source, Destination), shell=True)
    else:
        check_call("cp -f %s %s"%(Source, Destination), shell=True)

def main():
    _CopyFdFiles()
    _RunSecMain()
    _MoveXmlFiles()
    _DisplayXmlOutput()
    _DeleteFdFiles()

if __name__ == "__main__":
    main()
