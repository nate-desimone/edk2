/** @file
  Mock I/O DXE Data Storage Library implementation.

@copyright
  Copyright (c) 2015, Intel Corporation. All rights reserved
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "DxeMockIoDataStorageLib.h"

MOCK_STORAGE    gMockStorage = { NULL, 0, 0, };

/**
  Constructor for the Mock I/O DXE Data Storage Library.

  Initialize the library

  @param ImageHandle    The ImageHandle for the Driver/Application
  @param SystemTable    The EFI System Table pointer

  @retval EFI_SUCCESS   Initialization completed sucessfully
  @return others        An error ocurred during initialization
**/
EFI_STATUS
EFIAPI
DxeMockIoDataStorageLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return EFI_SUCCESS;
}

/**
  Destructor for the library.  Free any resources.

  @param ImageHandle    The ImageHandle for the Driver/Application
  @param SystemTable    The EFI System Table pointer

  @retval EFI_SUCCESS   Shut down was successful.
  @return others        An error occurred during shut down
**/
EFI_STATUS
EFIAPI
DxeMockIoDataStorageLibDestructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  ClearMockStorage (&gMockStorage);
  if (gMockStorage.Mocks != NULL) {
    FreePool ((VOID *) gMockStorage.Mocks);
    gMockStorage.Mocks = NULL;
    gMockStorage.Capacity = 0;
  }
  return EFI_SUCCESS;
}

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
  )
{
  *MockStorage = &gMockStorage;
  return EFI_SUCCESS;
}

/**
  Gets the number of Mocks Currently Stored in the Mock Storage Area.

  @param[in] MockStorage          Pointer to the Mock Storage Area

  @retval The number of Mocks Currently Stored in the Mock Storage Area
**/
UINTN
EFIAPI
GetMockCount (
  IN    VOID            *MockStorage
  )
{
  if (MockStorage == NULL) {
    return 0;
  }
  return ((MOCK_STORAGE *)MockStorage)->Count;
}

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
  )
{
  MOCK_STORAGE    *Storage;

  if (MockStorage == NULL) {
    *Mock = NULL;
    return;
  }
  Storage = (MOCK_STORAGE *)MockStorage;
  if (Storage->Mocks == NULL) {
    *Mock = NULL;
    return;
  }
  if (Index >= Storage->Count) {
    *Mock = NULL;
    return;
  }
  *Mock = &(Storage->Mocks[Index]);
}

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
  )
{
  MOCK_STORAGE    *Storage;
  MOCK            *NewMocks;
  UINTN           NewCapacity;

  if ((MockStorage == NULL) || (Mock == NULL)) {
    return EFI_INVALID_PARAMETER;
  }
  Storage = (MOCK_STORAGE *)MockStorage;
  if (Storage->Mocks == NULL) {
    //
    // Allocate the Initial Mock Array
    //
    Storage->Mocks = (MOCK *) AllocateZeroPool (sizeof (MOCK) * INITIAL_MOCK_STORAGE_MOCKS_CAPACITY);
    if (Storage->Mocks == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    Storage->Capacity = INITIAL_MOCK_STORAGE_MOCKS_CAPACITY;
    Storage->Count    = 0;
  } else if (Storage->Count >= Storage->Capacity) {
    //
    // Increase the capacity of the Mock Array
    //
    NewCapacity = (Storage->Capacity * MOCK_STORAGE_CAPACITY_INCREASE_MULTIPLIER) / MOCK_STORAGE_CAPACITY_INCREASE_DIVISOR;
    NewMocks = (MOCK *) AllocateZeroPool (sizeof (MOCK) * NewCapacity);
    if (NewMocks == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    CopyMem ((VOID *) NewMocks, (VOID *)Storage->Mocks, (sizeof (MOCK) * Storage->Count));
    FreePool ((VOID *) Storage->Mocks);
    Storage->Mocks    = NewMocks;
    Storage->Capacity = NewCapacity;
  }
  CopyMem ((VOID *) &(Storage->Mocks[Storage->Count]), (VOID *) Mock, sizeof (MOCK));
  Storage->Count++;
  return EFI_SUCCESS;
}

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
  )
{
  MOCK_STORAGE    *Storage;

  if (MockStorage == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  Storage = (MOCK_STORAGE *)MockStorage;
  if (Storage->Mocks != NULL) {
    if (Storage->Capacity > INITIAL_MOCK_STORAGE_MOCKS_CAPACITY) {
      FreePool ((VOID *) Storage->Mocks);
      Storage->Mocks = NULL;
      Storage->Capacity = 0;
    } else {
      //
      // Prevent excessive heap allocation, it won't reduce memory footprint in this case
      //
      ZeroMem ((VOID *) Storage->Mocks, sizeof (MOCK) * INITIAL_MOCK_STORAGE_MOCKS_CAPACITY);
    }
  }
  Storage->Count    = 0;
  return EFI_SUCCESS;
}
