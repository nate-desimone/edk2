/** @file
  Sample PEI Unit Test Suite implementation.

@copyright
  Copyright (c) 2015, Intel Corporation. All rights reserved
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Base.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PeiServicesLib.h>

#include <Library/UnitTestLib.h>
#include <LibraryPrivate/PeiUnitTestLib.h>

VOID
RunTestSuite (
  IN    TEST_SUITE    *TestSuite,
  IN    TEST_RESULT   *TestResults
  )
{
  EFI_STATUS                  Status;
  UINT32                      TestResultsIndex;
  UINT32                      TestResultsLength;
  UINT32                      TestCaseIndex;
  UINT32                      UnitTestIndex;
  UINT32                      HobSize;
  UINT8                       *Hob;
  UINT8                       *HobPtr;
  TEST_SUITE_RESULTS_HOB      *SuiteHeader;
  TEST_CASE_RESULTS_HEADER    *CaseHeader;
  PEI_TEST_RESULT             *UnitResult;
  FAILURE_MESSAGE             *FailureMessage;
  UINT16                      NumberFailures;
  UINT16                      NextFailureOffset;

  //
  // Calculate the number of unit tests
  //
  TestResultsLength   = 0;
  for (TestCaseIndex  = 0; TestCaseIndex < TestSuite->TestCasesLength; TestCaseIndex++) {
    TestResultsLength += TestSuite->TestCases[TestCaseIndex]->UnitTestsLength;
  }
  ZeroMem (TestResults, (sizeof (TEST_RESULT) * TestResultsLength));
  //
  // Crash the program if it is ever possible for the results for this test
  // suite to exceed the maximum HOB size.  This will give the user instant
  // feedback that they need to add another Test Suite
  //
  if (0x10000 < (sizeof (TEST_SUITE_RESULTS_HOB)                      +
     (sizeof (TEST_CASE_RESULTS_HEADER) * TestSuite->TestCasesLength) +
     (sizeof (PEI_TEST_RESULT) * TestResultsLength)                   +
     (sizeof (FAILURE_MESSAGE) * TestResultsLength))) {
    DEBUG  ((DEBUG_ERROR, "Test Suite %a is too big.  Break it in to smaller ones.\n", TestSuite->Name));
    ASSERT (FALSE);
  }

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
  // Count the number of failures that occurred
  //
  NumberFailures        = 0;
  for (TestResultsIndex = 0; TestResultsIndex < TestResultsLength; TestResultsIndex++) {
    if (TestResults[TestResultsIndex].Failure) {
      NumberFailures++;
    }
  }
  //
  // Convert the resulting data to HOB format
  //
  HobSize = sizeof (TEST_SUITE_RESULTS_HOB)                                   +
            (sizeof (TEST_CASE_RESULTS_HEADER) * TestSuite->TestCasesLength)  +
            (sizeof (PEI_TEST_RESULT) * TestResultsLength)                    +
            (sizeof (FAILURE_MESSAGE) * NumberFailures);
  ASSERT (HobSize <= 0x10000);
  Status = PeiServicesCreateHob (
                             EFI_HOB_TYPE_GUID_EXTENSION,
                             (UINT16) HobSize,
                             (VOID **) &Hob
                             );
  ASSERT_EFI_ERROR (Status);
  TestResultsIndex    = 0;
  NextFailureOffset   = 0;
  HobPtr              = Hob;
  //
  // Calculate the start of the Failure Message Table
  //
  HobPtr    += sizeof (TEST_SUITE_RESULTS_HOB);
  HobPtr    += (sizeof (TEST_CASE_RESULTS_HEADER) * TestSuite->TestCasesLength);
  for (TestCaseIndex = 0; TestCaseIndex < TestSuite->TestCasesLength; TestCaseIndex++) {
    HobPtr  += (sizeof (PEI_TEST_RESULT) * TestSuite->TestCases[TestCaseIndex]->UnitTestsLength);
  }
  FailureMessage = (FAILURE_MESSAGE *) HobPtr;
  HobPtr         = Hob;
  //
  // Copy Test Suite Data
  //
  SuiteHeader                       = (TEST_SUITE_RESULTS_HOB *) HobPtr;
  SuiteHeader->EfiHobGuidType.Name  = gUnitTestResultsHobGuid;
  AsciiStrnCpyS (
    &SuiteHeader->TestSuiteName[0],
    MAX_UTR_HOB_STRING_SIZE,
    TestSuite->Name,
    MAX_UTR_HOB_STRING_SIZE - 1
    );
  SuiteHeader->NumberFailures   = NumberFailures;
  SuiteHeader->NumberTestCases  = (UINT16)TestSuite->TestCasesLength;
  HobPtr                       += sizeof (TEST_SUITE_RESULTS_HOB);
  //
  // Copy Test Case Data
  //
  for (TestCaseIndex = 0; TestCaseIndex < TestSuite->TestCasesLength; TestCaseIndex++) {
    CaseHeader                  = (TEST_CASE_RESULTS_HEADER *) HobPtr;
    AsciiStrnCpyS (
      &CaseHeader->TestCaseName[0],
      MAX_UTR_HOB_STRING_SIZE,
      TestSuite->TestCases[TestCaseIndex]->Name,
      MAX_UTR_HOB_STRING_SIZE - 1
      );
    CaseHeader->NumberUnitTests = (UINT16)TestSuite->TestCases[TestCaseIndex]->UnitTestsLength;
    CaseHeader->Reserved        = 0;
    HobPtr                     += sizeof (TEST_CASE_RESULTS_HEADER);
    //
    // Copy Unit Test Data
    //
    for (UnitTestIndex = 0; UnitTestIndex < TestSuite->TestCases[TestCaseIndex]->UnitTestsLength; UnitTestIndex++) {
      UnitResult  = (PEI_TEST_RESULT *) HobPtr;
      AsciiStrnCpyS (
        &UnitResult->UnitTestName[0],
        MAX_UTR_HOB_STRING_SIZE,
        TestSuite->TestCases[TestCaseIndex]->UnitTests[UnitTestIndex]->Name,
        MAX_UTR_HOB_STRING_SIZE - 1
        );
      UnitResult->Failure = TestResults[TestResultsIndex].Failure;
      ZeroMem (&(UnitResult->Reserved), sizeof (UnitResult->Reserved));
      if (UnitResult->Failure) {
        UnitResult->FailureOffset = NextFailureOffset;
        AsciiStrnCpyS (
          &FailureMessage->FailureMessage[0],
          MAX_FAILURE_MESSAGE_STRING_SIZE,
          TestResults[TestResultsIndex].Message,
          MAX_FAILURE_MESSAGE_STRING_SIZE - 1
          );
        AsciiStrnCpyS (
          &FailureMessage->FailureFile[0],
          MAX_FAILURE_FILE_STRING_SIZE,
          TestResults[TestResultsIndex].File,
          MAX_FAILURE_FILE_STRING_SIZE - 1
          );
        FailureMessage->FailureLine = TestResults[TestResultsIndex].Line;
        NextFailureOffset++;
        FailureMessage++;
      } else {
        UnitResult->FailureOffset = 0;
      }
      TestResultsIndex++;
      HobPtr  += sizeof (PEI_TEST_RESULT);
    }
  }
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
