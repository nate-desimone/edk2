/** @file
  Sample DXE Unit Test Suite implementation.

@copyright
  Copyright (c) 2015, Intel Corporation. All rights reserved
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Base.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UnitTestLib.h>

#include <Protocol/UnitTestResults.h>

VOID
RunTestSuite (
  IN    TEST_SUITE    *TestSuite,
  IN    TEST_RESULT   *TestResults
  )
{
  EFI_STATUS                      Status;
  EFI_HANDLE                      Handle;
  UINTN                           BufferSize;
  UINT32                          TestResultsIndex;
  UINT32                          TestResultsLength;
  UINT32                          TestCaseIndex;
  UINT32                          UnitTestIndex;
  EFI_UNIT_TEST_RESULTS_PROTOCOL  *UnitTestResultsProtocol;
  TEST_CASE_RESULT                *TestCaseResults;
  DXE_TEST_RESULT                 *UnitTestResults;
  CHAR8                           *Buffer;

  //
  // Calculate the number of unit tests
  //
  TestResultsLength   = 0;
  for (TestCaseIndex  = 0; TestCaseIndex < TestSuite->TestCasesLength; TestCaseIndex++) {
    TestResultsLength += TestSuite->TestCases[TestCaseIndex]->UnitTestsLength;
  }
  ZeroMem (TestResults, (sizeof (TEST_RESULT) * TestResultsLength));

  //
  // Run all the unit tests
  //
  TestResultsIndex    = 0;
  for (TestCaseIndex  = 0; TestCaseIndex < TestSuite->TestCasesLength; TestCaseIndex++) {
    if (TestSuite->TestCases[TestCaseIndex]->SetUpMethod == NULL) {
      DispatchTestCase (TestSuite->TestCases[TestCaseIndex], TestResults, TestResultsIndex, NULL);
    } else {
      TestSuite->TestCases[TestCaseIndex]->SetUpMethod (
                                            TestResults,
                                            TestResultsIndex,
                                            (struct TEST_CASE *) TestSuite->TestCases[TestCaseIndex]
                                            );
    }
    TestResultsIndex += TestSuite->TestCases[TestCaseIndex]->UnitTestsLength;
  }

  //
  // Create EFI_UNIT_TEST_RESULTS_PROTOCOL from the Test Results
  //
  TestResultsIndex        = 0;
  UnitTestResultsProtocol = AllocateZeroPool (sizeof (EFI_UNIT_TEST_RESULTS_PROTOCOL));
  if (UnitTestResultsProtocol == NULL) {
    DEBUG ((DEBUG_ERROR, "Out of memory\n"));
    ASSERT (FALSE);
    return;
  }
  BufferSize  = AsciiStrLen (TestSuite->Name) + 1;
  Buffer      = AllocateZeroPool (BufferSize * sizeof (CHAR8));
  if (Buffer == NULL) {
    DEBUG ((DEBUG_ERROR, "Out of memory\n"));
    ASSERT (FALSE);
    return;
  }
  ASSERT_EFI_ERROR (AsciiStrCpyS (Buffer, BufferSize, TestSuite->Name));
  UnitTestResultsProtocol->TestSuiteName  = Buffer;
  TestCaseResults = AllocateZeroPool (TestSuite->TestCasesLength * sizeof (TEST_CASE_RESULT));
  if (TestCaseResults == NULL) {
    DEBUG ((DEBUG_ERROR, "Out of memory\n"));
    ASSERT (FALSE);
    return;
  }
  UnitTestResultsProtocol->TestCaseResults  = TestCaseResults;
  UnitTestResultsProtocol->NumberTestCases  = (UINT16) TestSuite->TestCasesLength;
  //
  // Copy Test Case Data
  //
  for (TestCaseIndex = 0; TestCaseIndex < TestSuite->TestCasesLength; TestCaseIndex++) {
    BufferSize  = AsciiStrLen (TestSuite->TestCases[TestCaseIndex]->Name) + 1;
    Buffer      = AllocateZeroPool (BufferSize * sizeof (CHAR8));
    if (Buffer == NULL) {
      DEBUG ((DEBUG_ERROR, "Out of memory\n"));
      ASSERT (FALSE);
      return;
    }
    ASSERT_EFI_ERROR (AsciiStrCpyS (Buffer, BufferSize, TestSuite->TestCases[TestCaseIndex]->Name));
    TestCaseResults[TestCaseIndex].TestCaseName     = Buffer;
    TestCaseResults[TestCaseIndex].NumberUnitTests  = (UINT16) TestSuite->TestCases[TestCaseIndex]->UnitTestsLength;
    UnitTestResults = AllocateZeroPool (TestSuite->TestCases[TestCaseIndex]->UnitTestsLength * sizeof (DXE_TEST_RESULT));
    if (UnitTestResults == NULL) {
      DEBUG ((DEBUG_ERROR, "Out of memory\n"));
      ASSERT (FALSE);
      return;
    }
    //
    // Copy Unit Test Data
    //
    TestCaseResults[TestCaseIndex].UnitTestResults = UnitTestResults;
    for (UnitTestIndex = 0; UnitTestIndex < TestSuite->TestCases[TestCaseIndex]->UnitTestsLength; UnitTestIndex++) {
      BufferSize  = AsciiStrLen (TestSuite->TestCases[TestCaseIndex]->UnitTests[UnitTestIndex]->Name) + 1;
      Buffer      = AllocateZeroPool (BufferSize * sizeof (CHAR8));
      if (Buffer == NULL) {
        DEBUG ((DEBUG_ERROR, "Out of memory\n"));
        ASSERT (FALSE);
        return;
      }
      ASSERT_EFI_ERROR (AsciiStrCpyS (Buffer, BufferSize, TestSuite->TestCases[TestCaseIndex]->UnitTests[UnitTestIndex]->Name));
      UnitTestResults[UnitTestIndex].UnitTestName = Buffer;
      UnitTestResults[UnitTestIndex].Failure      = TestResults[TestResultsIndex].Failure;
      if (UnitTestResults[UnitTestIndex].Failure) {
        BufferSize  = AsciiStrLen (TestResults[TestResultsIndex].Message) + 1;
        Buffer      = AllocateZeroPool (BufferSize * sizeof (CHAR8));
        if (Buffer == NULL) {
          DEBUG ((DEBUG_ERROR, "Out of memory\n"));
          ASSERT (FALSE);
          return;
        }
        ASSERT_EFI_ERROR (AsciiStrCpyS (Buffer, BufferSize, TestResults[TestResultsIndex].Message));
        UnitTestResults[UnitTestIndex].FailureMessage = Buffer;
        BufferSize  = AsciiStrLen (TestResults[TestResultsIndex].File) + 1;
        Buffer      = AllocateZeroPool (BufferSize * sizeof (CHAR8));
        if (Buffer == NULL) {
          DEBUG ((DEBUG_ERROR, "Out of memory\n"));
          ASSERT (FALSE);
          return;
        }
        ASSERT_EFI_ERROR (AsciiStrCpyS (Buffer, BufferSize, TestResults[TestResultsIndex].File));
        UnitTestResults[UnitTestIndex].FailureFile = Buffer;
        UnitTestResults[UnitTestIndex].FailureLine = TestResults[TestResultsIndex].Line;
      }
      TestResultsIndex++;
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
                  (VOID *) UnitTestResultsProtocol
                  );
  ASSERT_EFI_ERROR (Status);
}

VOID
DispatchTestCase (
  IN    TEST_CASE     *TestCase,
  IN    TEST_RESULT   *TestResults,
  IN    UINT32        TestResultsIndex,
  IN    VOID          *TestFixtureState
  )
{
  UINT32    UnitTestIndex;

  for (UnitTestIndex = 0; UnitTestIndex < TestCase->UnitTestsLength; UnitTestIndex++) {
    TestCase->UnitTests[UnitTestIndex]->Method (
                                          TestFixtureState,
                                          &TestResults[TestResultsIndex]
                                          );
    TestResultsIndex++;
  }
}
