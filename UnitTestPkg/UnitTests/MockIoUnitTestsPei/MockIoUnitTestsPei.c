/** @file
  Mock I/O Library PEI Unit Test Suite implementation.

@copyright
  Copyright (c) 2015, Intel Corporation. All rights reserved
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "MockIoUnitTestsPei.h"

/**
  The Entry point of the Mock I/O Library PEI Unit Test Suite

  @param[in]  FileHandle   Pointer to image file handle
  @param[in]  PeiServices  Pointer to PEI Services Table

  @return     EFI_SUCCESS
**/
EFI_STATUS
EFIAPI
MockIoUnitTestsEntry (
  IN       EFI_PEI_FILE_HANDLE  FileHandle,
  IN CONST EFI_PEI_SERVICES     **PeiServices
  )
{
  RunMockIoDataStoragePeiUnitTests ();
  RunMockIoPeiUnitTests ();
  return EFI_SUCCESS;
}
