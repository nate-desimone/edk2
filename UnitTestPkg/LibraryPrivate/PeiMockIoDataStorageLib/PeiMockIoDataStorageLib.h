/** @file
  Mock I/O PEI Data Storage Library header file

@copyright
  Copyright (c) 2015 Intel Corporation. All rights reserved
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#ifndef _PEI_MOCK_IO_DATA_STORAGE_LIB_H_
#define _PEI_MOCK_IO_DATA_STORAGE_LIB_H_

#include <Base.h>

#include <Library/MockIoLib.h>
#include <LibraryPrivate/MockIoDataStorageLib.h>

#define PEI_MOCK_STORAGE_MOCKS_PER_NODE   64

extern EFI_GUID   gPeiMockIoDataStoragePpi;

typedef struct PEI_MOCK_STORAGE_NODE {
  struct PEI_MOCK_STORAGE_NODE    *Next;
  UINT8                           MockCount;
  UINT8                           Reserved[3];
  MOCK                            Mocks[PEI_MOCK_STORAGE_MOCKS_PER_NODE];
} PEI_MOCK_STORAGE_NODE;

typedef struct {
  UINT32                          TotalMockCount;
  UINT32                          TotalMockCapacity;
  PEI_MOCK_STORAGE_NODE           Head;
} PEI_MOCK_STORAGE_PPI;

#endif
