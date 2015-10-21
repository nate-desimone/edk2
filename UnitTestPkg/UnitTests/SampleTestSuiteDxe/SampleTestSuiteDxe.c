/** @file
  Sample DXE Unit Test Suite implementation.

@copyright
 Copyright (c) 2015 Intel Corporation. All rights reserved
 SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "SampleTestSuiteDxe.h"

/**
  The Entry point of the Sample DXE Unit Test Suite

  @param[in] ImageHandle
  @param[in] SystemTable

  @retval    EFI_SUCCESS
**/
EFI_STATUS
SampleTestSuiteEntry (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  RunSampleDxeUnitTests ();

  return EFI_SUCCESS;
}
