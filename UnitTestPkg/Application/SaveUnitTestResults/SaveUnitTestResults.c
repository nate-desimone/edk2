/** @file
  Unit Test Runner Test Results Output Application

@copyright
  Copyright (c) 2015 Intel Corporation. All rights reserved
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Protocol/SimpleFileSystem.h>

#include <Protocol/UnitTestResults.h>

EFI_STATUS
GetUnitTestResults (
  OUT   EFI_UNIT_TEST_RESULTS_PROTOCOL    ***UnitTestResults,
  OUT   UINTN                             *UnitTestResultsLength
  )
{
  EFI_UNIT_TEST_RESULTS_PROTOCOL  **TestResults;
  EFI_HANDLE                      *Handles;
  UINTN                           NumberHandles;
  UINTN                           Index;
  EFI_STATUS                      Status;

  Handles     = NULL;
  NumberHandles = 0;

  //
  // Get all instances of EFI_UNIT_TEST_RESULTS_PROTOCOL
  //
  Status = gBS->LocateHandle (
                  ByProtocol,
                  &gEfiUnitTestResultsProtocol,
                  NULL,
                  &NumberHandles,
                  Handles
                  );

  //
  // Make sure at least 1 handle in the system implements EFI_UNIT_TEST_RESULTS_PROTOCOL
  //
  if (Status != EFI_BUFFER_TOO_SMALL) {
    *UnitTestResults        = NULL;
    *UnitTestResultsLength  = 0;
    return EFI_SUCCESS;
  }

  //DEBUG ((
  //  DEBUG_INFO,
  //  "Number of Unit Test Results: %d\n",
  //  (INTN)(NumberHandles / sizeof (EFI_HANDLE))
  //  ));

  //
  // Get handles implementing EFI_UNIT_TEST_RESULTS_PROTOCOL
  //
  Handles = AllocateZeroPool (NumberHandles);
  if (Handles == NULL) {
    *UnitTestResults        = NULL;
    *UnitTestResultsLength  = 0;
    return EFI_OUT_OF_RESOURCES;
  }
  TestResults = AllocateZeroPool ((NumberHandles / sizeof (EFI_HANDLE)) * sizeof (EFI_UNIT_TEST_RESULTS_PROTOCOL *));
  if (TestResults == NULL) {
    FreePool (Handles);
    *UnitTestResults        = NULL;
    *UnitTestResultsLength  = 0;
    return EFI_OUT_OF_RESOURCES;
  }
  Status = gBS->LocateHandle (
                  ByProtocol,
                  &gEfiUnitTestResultsProtocol,
                  NULL,
                  &NumberHandles,
                  Handles
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "Error Getting EFI_UNIT_TEST_RESULTS_PROTOCOL Handles\n"));
    FreePool (Handles);
    FreePool (TestResults);
    *UnitTestResults        = NULL;
    *UnitTestResultsLength  = 0;
    return Status;
  }

  //
  // Get the actual protocols
  //
  NumberHandles /= sizeof (EFI_HANDLE);
  for (Index = 0; Index < NumberHandles; Index++) {
    Status = gBS->OpenProtocol (
                    Handles[Index],
                    &gEfiUnitTestResultsProtocol,
                    (VOID **) &(TestResults[Index]),
                    gImageHandle,
                    NULL,
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_INFO, "Error Opening EFI_UNIT_TEST_RESULTS_PROTOCOL: %r\n", Status));
      FreePool (Handles);
      FreePool (TestResults);
      *UnitTestResults        = NULL;
      *UnitTestResultsLength  = 0;
      return Status;
    }
  }
  FreePool (Handles);
  Handles = NULL;
  *UnitTestResults        = TestResults;
  *UnitTestResultsLength  = NumberHandles;
  return EFI_SUCCESS;
}

VOID
DebugPrintUnitTestResult (
  IN    EFI_UNIT_TEST_RESULTS_PROTOCOL    *UnitTestResult,
  OUT   UINTN                             *TotalTestsRun,
  OUT   UINTN                             *TotalTestFailures
  )
{
  DXE_TEST_RESULT   *TestResult;
  UINTN             TotalTests;
  UINTN             TotalFailures;
  INT32             Spaces;
  INT32             SpaceIndex;
  UINT32            TestCaseIndex;
  UINT32            UnitTestIndex;

  TotalTests    = 0;
  TotalFailures = 0;
  DEBUG ((DEBUG_ERROR, "Test Suite: %a\n", UnitTestResult->TestSuiteName));
  for (TestCaseIndex = 0; TestCaseIndex < UnitTestResult->NumberTestCases; TestCaseIndex++) {
    DEBUG ((
      DEBUG_ERROR,
      "  Test Case: %a\n",
      UnitTestResult->TestCaseResults[TestCaseIndex].TestCaseName
      ));
    for ( UnitTestIndex = 0;
          UnitTestIndex < UnitTestResult->TestCaseResults[TestCaseIndex].NumberUnitTests;
          UnitTestIndex++) {
      TestResult  = &(UnitTestResult->TestCaseResults[TestCaseIndex].UnitTestResults[UnitTestIndex]);
      Spaces      = 67 - ((INT32) AsciiStrLen (TestResult->UnitTestName));
      DEBUG ((DEBUG_ERROR, "    %a: ", TestResult->UnitTestName));
      for (SpaceIndex = 0; SpaceIndex < Spaces; SpaceIndex++) {
        DEBUG ((DEBUG_ERROR, " "));
      }
      if (TestResult->Failure) {
        DEBUG ((DEBUG_ERROR, "Failed\n"));
        DEBUG ((
          DEBUG_ERROR, "      %a(%d): %a\n",
          TestResult->FailureFile,
          (UINTN) TestResult->FailureLine,
          TestResult->FailureMessage
          ));
        TotalFailures++;
      } else {
        DEBUG ((DEBUG_ERROR, "Passed\n"));
      }
      TotalTests++;
    }
  }
  *TotalTestsRun      += TotalTests;
  *TotalTestFailures  += TotalFailures;
}

VOID
DebugPrintUnitTestResults (
  IN    EFI_UNIT_TEST_RESULTS_PROTOCOL    **UnitTestResults,
  IN    UINTN                             UnitTestResultsLength
  )
{
  UINTN            Index;
  UINTN            TotalTests;
  UINTN            TotalFailures;
  UINTN            PercentPassing;

  TotalTests    = 0;
  TotalFailures = 0;
  if (UnitTestResults != NULL) {
    for (Index = 0; Index < UnitTestResultsLength; Index++) {
      DebugPrintUnitTestResult (UnitTestResults[Index], &TotalTests, &TotalFailures);
    }
  }
  if (TotalTests <= 0) {
    PercentPassing = 100;
  } else {
    if (((TotalFailures * 100) % TotalTests) > 0) {
      PercentPassing = 99 - (TotalFailures * 100) / TotalTests;
    } else {
      PercentPassing = 100 - (TotalFailures * 100) / TotalTests;
    }
  }
  DEBUG ((
    DEBUG_ERROR,
    "%d tests run, %d tests failed, %d%% Passing\n",
    TotalTests,
    TotalFailures,
    PercentPassing
    ));
}

EFI_STATUS
GetFileSystem (
  OUT   EFI_SIMPLE_FILE_SYSTEM_PROTOCOL   **EfiFileSystem
  )
{
  EFI_STATUS                       Status;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL  *FileSystem;
  EFI_HANDLE                       *Handles;
  UINTN                            HandleCount;
  UINTN                            HandleIndex;

  Handles     = NULL;
  HandleCount = 0;
  FileSystem  = NULL;

  //
  // Get all instances of EFI_SIMPLE_FILE_SYSTEM_PROTOCOL
  //
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiSimpleFileSystemProtocolGuid,
                  NULL,
                  &HandleCount,
                  &Handles
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "Error locating EFI_SIMPLE_FILE_SYSTEM_PROTOCOL: %r\n", Status));
    return Status;
  }
  //
  // Return the first one that we are able to open successfully
  //
  for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {
    Status = gBS->OpenProtocol (
                    Handles[HandleIndex],
                    &gEfiSimpleFileSystemProtocolGuid,
                    (VOID **) &FileSystem,
                    gImageHandle,
                    NULL,
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                    );
    if (!EFI_ERROR (Status)) {
      break;
    }
  }
  FreePool (Handles);
  *EfiFileSystem = FileSystem;
  return Status;
}

EFI_STATUS
OpenXmlResultsFile (
  IN    CHAR16                            *FileName,
  OUT   EFI_FILE_PROTOCOL                 **File
  )
{
  EFI_STATUS                        Status;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL   *FileSystem;
  EFI_FILE_PROTOCOL                 *FileRoot;

  *File = NULL;
  Status = GetFileSystem (&FileSystem);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Status = FileSystem->OpenVolume (FileSystem, &(FileRoot));
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Status = FileRoot->Open (
            FileRoot,
            File,
            FileName,
            EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE,
            0
            );
  FileRoot->Close (FileRoot);
  return Status;
}

VOID
CountTests (
  IN    EFI_UNIT_TEST_RESULTS_PROTOCOL    **UnitTestResults,
  IN    UINTN                             UnitTestResultsLength,
  OUT   UINTN                             *Tests,
  OUT   UINTN                             *Failures
  )
{
  TEST_CASE_RESULT  *TestCaseResult;
  UINTN             Index;
  UINT32            TestCaseIndex;
  UINT32            UnitTestIndex;

  *Tests    = 0;
  *Failures = 0;
  for (Index = 0; Index < UnitTestResultsLength; Index++) {
    for (TestCaseIndex = 0;
         TestCaseIndex < UnitTestResults[Index]->NumberTestCases;
         TestCaseIndex++) {
      TestCaseResult = &(UnitTestResults[Index]->TestCaseResults[TestCaseIndex]);
      for (UnitTestIndex = 0;
           UnitTestIndex < TestCaseResult->NumberUnitTests;
           UnitTestIndex++) {
        (*Tests)++;
        if (TestCaseResult->UnitTestResults[UnitTestIndex].Failure) {
          (*Failures)++;
        }
      }
    }
  }
}

#define SPRINT_BUFFER_SIZE          4096
EFI_STATUS
OutputTestCase (
  IN    EFI_FILE_PROTOCOL                 *File,
  IN    CHAR8                             *TestSuiteName,
  IN    TEST_CASE_RESULT                  *TestCaseResult
  )
{
  CHAR8             Buffer[SPRINT_BUFFER_SIZE];
  UINTN             BufferLen;
  DXE_TEST_RESULT   *TestResult;
  EFI_STATUS        Status;
  UINT32            UnitTestIndex;

  for (UnitTestIndex = 0;
       UnitTestIndex < TestCaseResult->NumberUnitTests;
       UnitTestIndex++) {
    TestResult = &(TestCaseResult->UnitTestResults[UnitTestIndex]);
    AsciiSPrint (
      &(Buffer[0]),
      SPRINT_BUFFER_SIZE,
      "  <testcase name=\"%a.%a.%a\">\r\n",
      TestSuiteName,
      TestCaseResult->TestCaseName,
      TestResult->UnitTestName
      );
    BufferLen = AsciiStrLen (&(Buffer[0]));
    Status = File->Write (File, &BufferLen, (VOID *) &(Buffer[0]));
    if (EFI_ERROR (Status)) {
      return Status;
    }
    if (TestResult->Failure) {
      AsciiSPrint (
        &(Buffer[0]),
        SPRINT_BUFFER_SIZE,
        "    <failure>%a(%d): %a</failure>\r\n",
        TestResult->FailureFile,
        (UINTN) TestResult->FailureLine,
        TestResult->FailureMessage
        );
      BufferLen = AsciiStrLen (&(Buffer[0]));
      Status = File->Write (File, &BufferLen, (VOID *) &(Buffer[0]));
      if (EFI_ERROR (Status)) {
        return Status;
      }
    }
    AsciiStrCpyS (Buffer, SPRINT_BUFFER_SIZE, "  </testcase>\r\n");
    BufferLen = AsciiStrLen (&(Buffer[0]));
    Status = File->Write (File, &BufferLen, (VOID *) &(Buffer[0]));
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }
  return Status;
}

VOID
SaveUnitTestResultsToXml (
  IN    EFI_UNIT_TEST_RESULTS_PROTOCOL    **UnitTestResults,
  IN    UINTN                             UnitTestResultsLength
  )
{
  CHAR8                             Buffer[SPRINT_BUFFER_SIZE];
  UINTN                             BufferLen;
  UINTN                             Tests;
  UINTN                             Failures;
  UINTN                             Index;
  UINTN                             TestCaseIndex;
  EFI_FILE_PROTOCOL                 *File;
  CHAR16                            *FileName;
  EFI_STATUS                        Status;

  if (UnitTestResults == NULL) {
    return;
  }
  FileName  = (CHAR16 *) PcdGetPtr (PcdTestResultsXmlFileName);
  Status    = OpenXmlResultsFile (FileName, &File);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "Error opening XML file for writing: %r\n", Status));
    return;
  }
  AsciiStrCpyS (Buffer, SPRINT_BUFFER_SIZE, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n");
  BufferLen = AsciiStrLen (&(Buffer[0]));
  Status = File->Write (File, &BufferLen, (VOID *) &(Buffer[0]));
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "Error writting to XML file: %r\n", Status));
    File->Close (File);
    return;
  }
  CountTests (UnitTestResults, UnitTestResultsLength, &Tests, &Failures);
  AsciiSPrint (
    &(Buffer[0]),
    SPRINT_BUFFER_SIZE,
    "<testsuite name=\"UnitTests\" tests=\"%d\" failures=\"%d\" errors=\"0\">\r\n",
    Tests,
    Failures
    );
  BufferLen = AsciiStrLen (&(Buffer[0]));
  Status = File->Write (File, &BufferLen, (VOID *) &(Buffer[0]));
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "Error writting to XML file: %r\n", Status));
    File->Close (File);
    return;
  }
  for (Index = 0; Index < UnitTestResultsLength; Index++) {
    for (TestCaseIndex = 0;
         TestCaseIndex < UnitTestResults[Index]->NumberTestCases;
         TestCaseIndex++) {
      Status = OutputTestCase (
                File,
                UnitTestResults[Index]->TestSuiteName,
                &(UnitTestResults[Index]->TestCaseResults[TestCaseIndex])
                );
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_INFO, "Error writting to XML file: %r\n", Status));
        File->Close (File);
        return;
      }
    }
  }
  AsciiStrCpyS (Buffer, SPRINT_BUFFER_SIZE, "</testsuite>\r\n");
  BufferLen = AsciiStrLen (&(Buffer[0]));
  Status = File->Write (File, &BufferLen, (VOID *) &(Buffer[0]));
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "Error writting to XML file: %r\n", Status));
    File->Close (File);
    return;
  }
}

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_UNIT_TEST_RESULTS_PROTOCOL    **UnitTestResults;
  UINTN                             UnitTestResultsLength;
  EFI_STATUS                        Status;

  Status = GetUnitTestResults (&UnitTestResults, &UnitTestResultsLength);
  ASSERT_EFI_ERROR (Status);
  if (FeaturePcdGet (PcdOutputTestResultsToDebugLog)) {
    DEBUG ((DEBUG_ERROR, "=============Test Results=============\n"));
    DebugPrintUnitTestResults (UnitTestResults, UnitTestResultsLength);
  }
  if (FeaturePcdGet (PcdOutputTestResultsToXmlFile)) {
    SaveUnitTestResultsToXml (UnitTestResults, UnitTestResultsLength);
  }

  if (UnitTestResults != NULL) {
    FreePool (UnitTestResults);
  }
  if (FeaturePcdGet (PcdResetAfterTestResultsAreSaved)) {
    gRT->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, NULL);
  }

  return EFI_SUCCESS;
}
