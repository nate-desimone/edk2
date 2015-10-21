/** @file
  Component description file for PEI Unit Test HOB to DXE Unit
  Test Protocol Conversion Driver

@copyright
  Copyright (c) 2015 Intel Corporation. All rights reserved
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Base.h>
#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/HobLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <LibraryPrivate/PeiUnitTestLib.h>
#include <Protocol/UnitTestResults.h>

EFI_STATUS
ConvertUnitTestHobToProtocol (
  IN TEST_SUITE_RESULTS_HOB   *UnitTestResultsHob
  )
{
  EFI_STATUS                        Status;
  EFI_HANDLE                        Handle;
  EFI_UNIT_TEST_RESULTS_PROTOCOL    *UnitTestResult;
  TEST_CASE_RESULT                  *TestCaseResults;
  DXE_TEST_RESULT                   *UnitTestResults;
  TEST_CASE_RESULTS_HEADER          *TestCaseHeader;
  PEI_TEST_RESULT                   *PeiTestResult;
  FAILURE_MESSAGE                   *FailureMessages;
  UINT8                             *Hob;
  CHAR8                             *Buffer;
  UINTN                             BufferSize;
  UINT32                            TestCaseIndex;
  UINT32                            UnitTestIndex;

  //
  // Calculate the start of the Failure Message Table
  //
  Hob  = (UINT8 *)UnitTestResultsHob;
  Hob += sizeof (TEST_SUITE_RESULTS_HOB);
  for (TestCaseIndex = 0; TestCaseIndex < UnitTestResultsHob->NumberTestCases; TestCaseIndex++) {
    Hob             += sizeof (PEI_TEST_RESULT) * (((TEST_CASE_RESULTS_HEADER *)Hob)->NumberUnitTests);
    Hob             += sizeof (TEST_CASE_RESULTS_HEADER);
  }
  FailureMessages = (FAILURE_MESSAGE *)Hob;

  //
  // Create EFI_UNIT_TEST_RESULTS_PROTOCOL from the Test Results
  //
  UnitTestResult = AllocateZeroPool (sizeof (EFI_UNIT_TEST_RESULTS_PROTOCOL));
  if (UnitTestResult == NULL) {
    DEBUG ((DEBUG_ERROR, "Out of memory\n"));
    ASSERT (FALSE);
    return EFI_OUT_OF_RESOURCES;
  }
  BufferSize  = AsciiStrLen (&UnitTestResultsHob->TestSuiteName[0]) + 1;
  Buffer      = AllocateZeroPool (BufferSize * sizeof (CHAR8));
  if (Buffer == NULL) {
    DEBUG ((DEBUG_ERROR, "Out of memory\n"));
    ASSERT (FALSE);
    return EFI_OUT_OF_RESOURCES;
  }
  ASSERT_EFI_ERROR (AsciiStrCpyS (Buffer, BufferSize, &UnitTestResultsHob->TestSuiteName[0]));
  UnitTestResult->TestSuiteName = Buffer;
  TestCaseResults = AllocateZeroPool (UnitTestResultsHob->NumberTestCases * sizeof (TEST_CASE_RESULT));
  if (TestCaseResults == NULL) {
    DEBUG ((DEBUG_ERROR, "Out of memory\n"));
    ASSERT (FALSE);
    return EFI_OUT_OF_RESOURCES;
  }
  UnitTestResult->TestCaseResults  = TestCaseResults;
  UnitTestResult->NumberTestCases  = UnitTestResultsHob->NumberTestCases;
  //
  // Copy Test Case Data
  //
  Hob   = (UINT8 *)UnitTestResultsHob;
  Hob  += sizeof (TEST_SUITE_RESULTS_HOB);
  for (TestCaseIndex = 0; TestCaseIndex < UnitTestResultsHob->NumberTestCases; TestCaseIndex++) {
    TestCaseHeader  = (TEST_CASE_RESULTS_HEADER *)Hob;
    BufferSize      = AsciiStrLen (&TestCaseHeader->TestCaseName[0]) + 1;
    Buffer          = AllocateZeroPool (BufferSize * sizeof (CHAR8));
    if (Buffer == NULL) {
      DEBUG ((DEBUG_ERROR, "Out of memory\n"));
      ASSERT (FALSE);
      return EFI_OUT_OF_RESOURCES;
    }
    ASSERT_EFI_ERROR (AsciiStrCpyS (Buffer, BufferSize, &TestCaseHeader->TestCaseName[0]));
    TestCaseResults[TestCaseIndex].TestCaseName     = Buffer;
    TestCaseResults[TestCaseIndex].NumberUnitTests  = TestCaseHeader->NumberUnitTests;
    UnitTestResults = AllocateZeroPool (TestCaseHeader->NumberUnitTests * sizeof (DXE_TEST_RESULT));
    if (UnitTestResults == NULL) {
      DEBUG ((DEBUG_ERROR, "Out of memory\n"));
      ASSERT (FALSE);
      return EFI_OUT_OF_RESOURCES;
    }
    //
    // Copy Unit Test Data
    //
    TestCaseResults[TestCaseIndex].UnitTestResults = UnitTestResults;
    Hob                                           += sizeof (TEST_CASE_RESULTS_HEADER);
    for (UnitTestIndex = 0; UnitTestIndex < TestCaseHeader->NumberUnitTests; UnitTestIndex++) {
      PeiTestResult = (PEI_TEST_RESULT *) Hob;
      BufferSize  = AsciiStrLen (&PeiTestResult->UnitTestName[0]) + 1;
      Buffer      = AllocateZeroPool (BufferSize * sizeof (CHAR8));
      if (Buffer == NULL) {
        DEBUG ((DEBUG_ERROR, "Out of memory\n"));
        ASSERT (FALSE);
        return EFI_OUT_OF_RESOURCES;
      }
      ASSERT_EFI_ERROR (AsciiStrCpyS (Buffer, BufferSize, &PeiTestResult->UnitTestName[0]));
      UnitTestResults[UnitTestIndex].UnitTestName = Buffer;
      UnitTestResults[UnitTestIndex].Failure      = PeiTestResult->Failure;
      if (UnitTestResults[UnitTestIndex].Failure) {
        BufferSize  = AsciiStrLen (&FailureMessages[PeiTestResult->FailureOffset].FailureMessage[0]) + 1;
        Buffer      = AllocateZeroPool (BufferSize * sizeof (CHAR8));
        if (Buffer == NULL) {
          DEBUG ((DEBUG_ERROR, "Out of memory\n"));
          ASSERT (FALSE);
          return EFI_OUT_OF_RESOURCES;
        }
        ASSERT_EFI_ERROR (AsciiStrCpyS (Buffer, BufferSize, &FailureMessages[PeiTestResult->FailureOffset].FailureMessage[0]));
        UnitTestResults[UnitTestIndex].FailureMessage = Buffer;
        BufferSize  = AsciiStrLen (&FailureMessages[PeiTestResult->FailureOffset].FailureFile[0]) + 1;
        Buffer      = AllocateZeroPool (BufferSize * sizeof (CHAR8));
        if (Buffer == NULL) {
          DEBUG ((DEBUG_ERROR, "Out of memory\n"));
          ASSERT (FALSE);
          return EFI_OUT_OF_RESOURCES;
        }
        ASSERT_EFI_ERROR (AsciiStrCpyS (Buffer, BufferSize, &FailureMessages[PeiTestResult->FailureOffset].FailureFile[0]));
        UnitTestResults[UnitTestIndex].FailureFile = Buffer;
        UnitTestResults[UnitTestIndex].FailureLine = FailureMessages[PeiTestResult->FailureOffset].FailureLine;
      }
      Hob += sizeof (PEI_TEST_RESULT);
    }
  }
  //
  // Install the protocol
  //
  Handle = NULL;
  Status = gBS->InstallProtocolInterface (
                  &Handle,
                  &gEfiUnitTestResultsProtocol,
                  EFI_NATIVE_INTERFACE,
                  (VOID *) UnitTestResult
                  );
  return Status;
}

EFI_STATUS
ConvertAllUnitTestHobsToProtocols (
  IN VOID                     *HobListStart
  )
{
  EFI_STATUS    Status;
  VOID          *Hob;

  Hob = GetNextGuidHob (&gUnitTestResultsHobGuid, HobListStart);
  while (Hob != NULL) {
    Status  = ConvertUnitTestHobToProtocol ((TEST_SUITE_RESULTS_HOB *) Hob);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    Hob = GET_NEXT_HOB (Hob);
    Hob = GetNextGuidHob (&gUnitTestResultsHobGuid, Hob);
  }
  return EFI_SUCCESS;
}

/**
  The Entry point of the HOB to Protocol Converter

  @param[in] ImageHandle
  @param[in] SystemTable

  @retval    EFI_SUCCESS
**/
EFI_STATUS
UnitTestHobEntry (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS    Status;
  VOID          *HobList;

  Status = EfiGetSystemConfigurationTable (&gEfiHobListGuid, &HobList);
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  Status = ConvertAllUnitTestHobsToProtocols (HobList);
  ASSERT_EFI_ERROR (Status);

  return Status;
}
