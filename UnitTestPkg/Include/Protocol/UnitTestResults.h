/** @file
  DXE Unit Test Library header file

@copyright
  Copyright (c) 2015 Intel Corporation. All rights reserved
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#ifndef _UNIT_TEST_RESULTS_H_
#define _UNIT_TEST_RESULTS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <Base.h>

typedef struct {
  CHAR8               *UnitTestName;
  CHAR8               *FailureMessage;
  CHAR8               *FailureFile;
  UINT32              FailureLine;
  BOOLEAN             Failure;
  UINT8               Reserved[3];
} DXE_TEST_RESULT;

typedef struct {
  CHAR8               *TestCaseName;
  DXE_TEST_RESULT     *UnitTestResults;
  UINT16              NumberUnitTests;
  UINT16              Reserved;
} TEST_CASE_RESULT;

typedef struct {
  CHAR8               *TestSuiteName;
  TEST_CASE_RESULT    *TestCaseResults;
  UINT16              NumberTestCases;
  UINT16              Reserved;
} EFI_UNIT_TEST_RESULTS_PROTOCOL;

extern EFI_GUID gEfiUnitTestResultsProtocol;

#ifdef __cplusplus
}
#endif
#endif