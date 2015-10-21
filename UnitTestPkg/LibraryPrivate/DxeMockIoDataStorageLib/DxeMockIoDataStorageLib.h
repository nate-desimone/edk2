/** @file
  Mock I/O DXE Data Storage Library header file

@copyright
  Copyright (c) 2015 Intel Corporation. All rights reserved
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#ifndef _DXE_MOCK_IO_DATA_STORAGE_LIB_H_
#define _DXE_MOCK_IO_DATA_STORAGE_LIB_H_

#include <Base.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>

#include <Library/MockIoLib.h>
#include <LibraryPrivate/MockIoDataStorageLib.h>

#define INITIAL_MOCK_STORAGE_MOCKS_CAPACITY         64

//
// @note: 1.5 is slightly less than the golden ratio, this means that the array
// will grow at a slow enough rate such that it will eventually fit within the
// memory space it used before expansion
//
#define MOCK_STORAGE_CAPACITY_INCREASE_MULTIPLIER   15
#define MOCK_STORAGE_CAPACITY_INCREASE_DIVISOR      10


typedef struct {
  MOCK      *Mocks;
  UINTN     Count;
  UINTN     Capacity;
} MOCK_STORAGE;

extern MOCK_STORAGE    gMockStorage;

#endif
