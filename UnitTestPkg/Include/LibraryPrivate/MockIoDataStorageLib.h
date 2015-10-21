/** @file
  Mock I/O Data Storage Library header file

@copyright
  Copyright (c) 2015 Intel Corporation. All rights reserved
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#ifndef _MOCK_IO_DATA_STORAGE_LIB_H_
#define _MOCK_IO_DATA_STORAGE_LIB_H_

#include <Base.h>

#include <Library/MockIoLib.h>

/**
  Gets the Mock Storage Area.  If the Mock Storage Area does not exist yet, it creates it.

  @param[out] MockStorage         Returns the Pointer to the Mock Storage Area

  @retval EFI_SUCCESS             Succeeds.
  @retval EFI_OUT_OF_RESOURCES    Failed to Allocate Memory for the Mock storage
**/
EFI_STATUS
EFIAPI
GetMockStorage (
  OUT   VOID            **MockStorage
  );

/**
  Gets the number of Mocks Currently Stored in the Mock Storage Area.

  @param[in] MockStorage          Pointer to the Mock Storage Area

  @retval The number of Mocks Currently Stored in the Mock Storage Area
**/
UINTN
EFIAPI
GetMockCount (
  IN    VOID            *MockStorage
  );

/**
  Retrieve a Mock from the Mock Storage Area.  Treates the Mock Storage Area
  as if it were an array and retrieves the Mock at the given Index

  @param[in]  MockStorage         Pointer to the Mock Storage Area
  @param[in]  Index               Index of the Mock in the Mock Storage Area
  @param[out] Mock                Returns the pointer to the Mock, or NULL on error
**/
VOID
EFIAPI
GetMock (
  IN    VOID            *MockStorage,
  IN    UINTN           Index,
  OUT   MOCK            **Mock
  );

/**
  Appends a new Mock to the end of the Mock Storage Area.

  @param[in]  MockStorage         Pointer to the Mock Storage Area
  @param[in]  Mock                The new Mock to be added

  @retval EFI_SUCCESS             Succeeds.
  @retval EFI_INVALID_PARAMETER   MockStorage or Mock is a NULL pointer
  @retval EFI_OUT_OF_RESOURCES    Failed to Allocate Memory for the Mock storage
**/
EFI_STATUS
EFIAPI
AddMock (
  IN    VOID            *MockStorage,
  IN    MOCK            *Mock
  );

/**
  Deletes all Mocks from the Mock Storage Area.  If possible, reclaims memory.

  @param[in]  MockStorage         Pointer to the Mock Storage Area

  @retval EFI_SUCCESS             Succeeds.
  @retval EFI_INVALID_PARAMETER   MockStorage is a NULL pointer
**/
EFI_STATUS
EFIAPI
ClearMockStorage (
  IN    VOID            *MockStorage
  );

#endif
