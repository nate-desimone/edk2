/** @file
  Mock I/O PEI Data Storage Library implementation.

@copyright
  Copyright (c) 2015, Intel Corporation. All rights reserved
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Base.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PeiServicesLib.h>

#include <Ppi/MemoryDiscovered.h>

#include <Library/MockIoLib.h>
#include <LibraryPrivate/MockIoDataStorageLib.h>

#include "PeiMockIoDataStorageLib.h"

/**
  Notifies the Mock Storage Library that Main Memory has been Installed

  @param[in] PeiServices          General purpose services available to every PEIM.
  @param[in] NotifyDescriptor     The notification structure this PEIM registered on install.
  @param[in] Ppi                  The memory discovered PPI.  Not used.

  @retval EFI_SUCCESS             Succeeds.
**/
EFI_STATUS
EFIAPI
MockStorageOnMemoryDiscovered (
  IN  EFI_PEI_SERVICES             **PeiServices,
  IN  EFI_PEI_NOTIFY_DESCRIPTOR    *NotifyDescriptor,
  IN  VOID                         *Ppi
  );

static EFI_PEI_NOTIFY_DESCRIPTOR  mMockStorageNotifyList[] = {
  {
    EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
    &gEfiPeiMemoryDiscoveredPpiGuid,
    MockStorageOnMemoryDiscovered
  }
};

/**
  Notifies the Mock Storage Library that Main Memory has been Installed

  @param[in] PeiServices          General purpose services available to every PEIM.
  @param[in] NotifyDescriptor     The notification structure this PEIM registered on install.
  @param[in] Ppi                  The memory discovered PPI.  Not used.

  @retval EFI_SUCCESS             Succeeds.
**/
EFI_STATUS
EFIAPI
MockStorageOnMemoryDiscovered (
  IN  EFI_PEI_SERVICES             **PeiServices,
  IN  EFI_PEI_NOTIFY_DESCRIPTOR    *NotifyDescriptor,
  IN  VOID                         *Ppi
  )
{
  EFI_STATUS              Status;
  PEI_MOCK_STORAGE_PPI    *MockStoragePpi;

  Status = PeiServicesLocatePpi (
            &gPeiMockIoDataStoragePpi,
            0,
            NULL,
            (VOID **) &MockStoragePpi
            );
  if (!EFI_ERROR (Status) && (MockStoragePpi != NULL)) {
    if (MockStoragePpi->Head.Next != NULL) {
      //
      // The pointer will be broken, delete everything
      //
      MockStoragePpi->Head.Next         = NULL;
      MockStoragePpi->TotalMockCapacity = PEI_MOCK_STORAGE_MOCKS_PER_NODE;
      ClearMockStorage ((VOID *) MockStoragePpi);
    }
  }
  return EFI_SUCCESS;
}

/**
  Creates the Mock Storage Area.

  @param[out] MockStoragePpiPtr   Returns the Pointer to the Mock Storage Area

  @retval EFI_SUCCESS             Succeeds.
  @retval EFI_OUT_OF_RESOURCES    Failed to Allocate Memory for the Mock storage
**/
EFI_STATUS
EFIAPI
CreateMockStoragePpi (
  OUT    PEI_MOCK_STORAGE_PPI    **MockStoragePpiPtr
  )
{
  EFI_STATUS              Status;
  PEI_MOCK_STORAGE_PPI    *MockStoragePpi;
  EFI_PEI_PPI_DESCRIPTOR  *MockStoragePpiDesc;
  VOID                    *MemoryDiscoveredPpi;

  *MockStoragePpiPtr = NULL;
  //
  // Initialize the PPI
  //
  MockStoragePpi = (PEI_MOCK_STORAGE_PPI *) AllocateZeroPool (sizeof (PEI_MOCK_STORAGE_PPI));
  if (MockStoragePpi == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  MockStoragePpi->TotalMockCapacity = PEI_MOCK_STORAGE_MOCKS_PER_NODE;

  //
  // Initialize the PPI descriptor
  //
  MockStoragePpiDesc = (EFI_PEI_PPI_DESCRIPTOR *) AllocateZeroPool (sizeof (EFI_PEI_PPI_DESCRIPTOR));
  if (MockStoragePpiDesc == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  MockStoragePpiDesc->Flags   = EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST;
  MockStoragePpiDesc->Guid    = &gPeiMockIoDataStoragePpi;
  MockStoragePpiDesc->Ppi     = MockStoragePpi;

  //
  // Install the PPI
  //
  Status = PeiServicesInstallPpi (MockStoragePpiDesc);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  *MockStoragePpiPtr = MockStoragePpi;

  //
  // Check and see if main memory exists yet
  //
  Status = PeiServicesLocatePpi (
            &gEfiPeiMemoryDiscoveredPpiGuid,
            0,
            NULL,
            (VOID **) &MemoryDiscoveredPpi
            );
  if (Status == EFI_NOT_FOUND) {
    //
    // The system is still running in Cache as RAM mode
    //
    // We need to install a callback so that when main memory exists we clear
    // out all existing Mocks; PEI shadowing will break the Linked List
    //
    Status = PeiServicesNotifyPpi (mMockStorageNotifyList);
  }
  return Status;
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
  EFI_STATUS              Status;
  PEI_MOCK_STORAGE_PPI    *MockStoragePpi;

  *MockStorage = NULL;
  Status = PeiServicesLocatePpi (
            &gPeiMockIoDataStoragePpi,
            0,
            NULL,
            (VOID **) &MockStoragePpi
            );
  if (EFI_ERROR (Status) || (MockStoragePpi == NULL)) {
    Status = CreateMockStoragePpi (&MockStoragePpi);
    if (!EFI_ERROR (Status)) {
      *MockStorage = (VOID *) MockStoragePpi;
    }
    return Status;
  } else {
    *MockStorage = (VOID *) MockStoragePpi;
    return EFI_SUCCESS;
  }
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
  PEI_MOCK_STORAGE_PPI    *MockStoragePpi;

  MockStoragePpi  = (PEI_MOCK_STORAGE_PPI *) MockStorage;
  if (MockStoragePpi == NULL) {
    return 0;
  } else {
    return (UINTN) MockStoragePpi->TotalMockCount;
  }
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
  PEI_MOCK_STORAGE_PPI    *MockStoragePpi;
  PEI_MOCK_STORAGE_NODE   *MockNode;
  UINT32                  NodeNumber;
  UINT32                  CurrentNode;
  UINT8                   NodeIndex;

  if (MockStorage == NULL) {
    *Mock = NULL;
    return;
  }
  MockStoragePpi  = (PEI_MOCK_STORAGE_PPI *) MockStorage;
  NodeNumber      = (UINT32) (Index / PEI_MOCK_STORAGE_MOCKS_PER_NODE);
  NodeIndex       = (UINT8) (Index % PEI_MOCK_STORAGE_MOCKS_PER_NODE);
  MockNode        = &(MockStoragePpi->Head);
  if (Index >= MockStoragePpi->TotalMockCount) {
    *Mock = NULL;
    return;
  }
  for (CurrentNode = 0; CurrentNode < NodeNumber; CurrentNode++) {
    if (MockNode == NULL) {
      ASSERT (FALSE);
      *Mock = NULL;
      return;
    }
    MockNode = MockNode->Next;
  }
  if (NodeIndex >= MockNode->MockCount) {
    ASSERT(FALSE);
    *Mock = NULL;
    return;
  }
  *Mock = &(MockNode->Mocks[NodeIndex]);
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
  PEI_MOCK_STORAGE_PPI    *MockStoragePpi;
  PEI_MOCK_STORAGE_NODE   *NodeWithFreeSpace;
  PEI_MOCK_STORAGE_NODE   *TailNode;

  if ((MockStorage == NULL) || (Mock == NULL)) {
    return EFI_INVALID_PARAMETER;
  }
  MockStoragePpi    = (PEI_MOCK_STORAGE_PPI *) MockStorage;
  NodeWithFreeSpace = &(MockStoragePpi->Head);
  while ((NodeWithFreeSpace != NULL) && (NodeWithFreeSpace->MockCount >= PEI_MOCK_STORAGE_MOCKS_PER_NODE)) {
    NodeWithFreeSpace = NodeWithFreeSpace->Next;
  }
  if (NodeWithFreeSpace == NULL) {
    TailNode = &(MockStoragePpi->Head);
    while (TailNode->Next != NULL) {
      TailNode = TailNode->Next;
    }
    NodeWithFreeSpace = (PEI_MOCK_STORAGE_NODE *) AllocateZeroPool (sizeof (PEI_MOCK_STORAGE_NODE));
    if (NodeWithFreeSpace == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    TailNode->Next = NodeWithFreeSpace;
    MockStoragePpi->TotalMockCapacity += PEI_MOCK_STORAGE_MOCKS_PER_NODE;
  }
  CopyMem ((VOID *) &(NodeWithFreeSpace->Mocks[NodeWithFreeSpace->MockCount]), (VOID *) Mock, sizeof (MOCK));
  NodeWithFreeSpace->MockCount++;
  MockStoragePpi->TotalMockCount++;
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
  PEI_MOCK_STORAGE_PPI    *MockStoragePpi;
  PEI_MOCK_STORAGE_NODE   *MockNode;

  if (MockStorage == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  MockStoragePpi                  = (PEI_MOCK_STORAGE_PPI *) MockStorage;
  MockStoragePpi->TotalMockCount  = 0;
  MockNode                        = &(MockStoragePpi->Head);
  while (MockNode != NULL) {
    MockNode->MockCount = 0;
    ZeroMem ((VOID *) &(MockNode->Mocks[0]), sizeof (MockNode->Mocks));
    MockNode = MockNode->Next;
  }
  return EFI_SUCCESS;
}
