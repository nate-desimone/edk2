/** @file
  PEI Unit Test Library header file

@copyright
  Copyright (c) 2015 Intel Corporation. All rights reserved
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#ifndef _PEI_UNIT_TEST_LIB_H_
#define _PEI_UNIT_TEST_LIB_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <Base.h>

#define MAX_UTR_HOB_STRING_SIZE           32
#define MAX_FAILURE_MESSAGE_STRING_SIZE   64
#define MAX_FAILURE_FILE_STRING_SIZE      256

//
// HOB Layout
//
// +---------------------------+
// |  TEST_SUITE_RESULTS_HOB   |
// +---------------------------+  <- TEST_CASE_RESULTS[0]
// | TEST_CASE_RESULTS_HEADER  |
// | +----------------------+  |
// | |  PEI_TEST_RESULT[0]  |  |
// | +----------------------+  |
// | |  PEI_TEST_RESULT[1]  |  |
// | +----------------------+  |
// | |         ...          |  |
// | +----------------------+  |
// | |  PEI_TEST_RESULT[n]  |  |
// | +----------------------+  |
// +---------------------------+
// |   TEST_CASE_RESULTS[1]    |
// | +----------------------+  |
// | |         ...          |  |
// | +----------------------+  |
// +---------------------------+
// |            ...            |
// +---------------------------+
// |   TEST_CASE_RESULTS[n]    |
// | +----------------------+  |
// | |         ...          |  |
// | +----------------------+  |
// +---------------------------+
// |    FAILURE_MESSAGE[0]     |
// +---------------------------+
// |    FAILURE_MESSAGE[1]     |
// +---------------------------+
// |            ...            |
// +---------------------------+
// |    FAILURE_MESSAGE[n]     |
// +---------------------------+
//

typedef struct {
  EFI_HOB_GUID_TYPE   EfiHobGuidType;
  CHAR8               TestSuiteName[MAX_UTR_HOB_STRING_SIZE];
  UINT16              NumberTestCases;
  UINT16              NumberFailures;
} TEST_SUITE_RESULTS_HOB;

typedef struct {
  CHAR8               TestCaseName[MAX_UTR_HOB_STRING_SIZE];
  UINT16              NumberUnitTests;
  UINT16              Reserved;
} TEST_CASE_RESULTS_HEADER;

typedef struct {
  CHAR8               UnitTestName[MAX_UTR_HOB_STRING_SIZE];
  UINT16              FailureOffset;
  BOOLEAN             Failure;
  UINT8               Reserved;
} PEI_TEST_RESULT;

typedef struct {
  CHAR8               FailureMessage[MAX_FAILURE_MESSAGE_STRING_SIZE];
  CHAR8               FailureFile[MAX_FAILURE_FILE_STRING_SIZE];
  UINT32              FailureLine;
} FAILURE_MESSAGE;

extern EFI_GUID gUnitTestResultsHobGuid;

#ifdef __cplusplus
}
#endif
#endif