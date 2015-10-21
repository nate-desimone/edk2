/** @file
  Mock I/O Library header file

@copyright
  Copyright (c) 2015 Intel Corporation. All rights reserved
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#ifndef _MOCK_IO_LIB_H_
#define _MOCK_IO_LIB_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <Uefi.h>

#define MOCK_DATA_SIZE          8
#define MOCK_SCRATCHPAD_SIZE    16

typedef enum {
  /**
    MMIO Mock
  **/
  MockSpaceMmio = 0,
  /**
    Port I/O Mock
  **/
  MockSpaceIo
} MOCK_SPACE;
//
//@note: It is difficult to support MSR mocking since
//       AsmReadMsr/AsmWriteMsr are defined in BaseLib
//

typedef enum {
  /**
    A Mock that behaves like normal DRAM, accepts reads and writes.
    Reads return the last written value.
  **/
  MockTypeRw = 0,
  /**
    A Read Only Mock.  Writes are ignored.  Reads will always return
    the same value.  The value returned is set when the Mock is created.
  **/
  MockTypeRo,
  /**
    A callback Mock.  Any time the Mock is read or written, callback
    functions are invoked to perform the operation.
  **/
  MockTypeCallback
} MOCK_TYPE;

/**
  Indicates the size of the CR that the Mock emulates.  Also used to indicate
  this size of read/write operations
**/
typedef enum {
  MockSize8 = 0,
  MockSize16,
  MockSize32,
  MockSize64
} MOCK_SIZE;

struct MOCK;

/**
  Read from the Mock

  @param[in]  This                The Mock to be read from
  @param[out] Data                The data to be read.  The size of the
                                  buffer pointed to by Data must be large
                                  enough to contain the data type indicated
                                  by the This->Size parameter
**/
typedef
VOID
(EFIAPI *MOCK_READ_CALLBACK)(
  IN  struct  MOCK      *This,
  OUT         UINT8     *Data
  );

/**
  Write to the Mock

  @param[in]  This                The Mock to be written to
  @param[out] Data                The data to be write.  The size of the
                                  buffer pointed to by Data must be large
                                  enough to contain the data type indicated
                                  by the This->Size parameter
**/
typedef
VOID
(EFIAPI *MOCK_WRITE_CALLBACK)(
  IN  struct  MOCK      *This,
  IN          UINT8     *Data
  );

/**
  For Mocks that are of type MockTypeCallback, the creator of the Mock needs
  to provide this structure
**/
typedef struct {
  /**
    The callback to invoke when the Mock is read from
  **/
  MOCK_READ_CALLBACK    Read;
  /**
    The callback to invoke when the Mock is written to
  **/
  MOCK_WRITE_CALLBACK   Write;
  /**
    16 Bytes of spare space that the Mock callbacks can use for thier own
    purposes.
  **/
  UINT8                 ScratchPad[MOCK_SCRATCHPAD_SIZE];
} CALLBACK_MOCK;

typedef struct MOCK {
  /**
    The Mock Type
  **/
  MOCK_TYPE             Type;
  /**
    The size of the CR that the Mock emulates
  **/
  MOCK_SIZE             Size;
  /**
    The address of the CR that the Mock emulates
  **/
  UINT64                Address;
  /**
    The address space of the CR (MMIO, Port I/O, etc.)
  **/
  MOCK_SPACE            AddressSpace;
  union {
    /**
      If Type is MockTypeCallback, this should be filled in
    **/
    CALLBACK_MOCK       Callback;
    /**
      If Type is not MockTypeCallback, this is the current value
      of the CR that this Mock emulates.  When a new mock is created
      this should be set to the initial value for the CR
    **/
    UINT8               Data[MOCK_DATA_SIZE];
  } TypeData;
} MOCK;

/**
  Read from emulated I/O

  @param[in]  Address             Address to read from
  @param[in]  AddressSpace        Address Space to read from
  @param[in]  Size                Size of the emulated read operation
  @param[out] Data                The data to be read.  The size of the
                                  buffer pointed to by Data must be large
                                  enough to contain the data type indicated
                                  by the Size parameter
**/
VOID
EFIAPI
MockRead (
  IN    UINT64          Address,
  IN    MOCK_SPACE      AddressSpace,
  IN    MOCK_SIZE       Size,
  OUT   UINT8           *Data
  );

/**
  Write to emulated I/O

  @param[in]  Address             Address to write to
  @param[in]  AddressSpace        Address Space to write to
  @param[in]  Size                Size of the emulated write operation
  @param[in]  Data                The data to be written.  The size of the
                                  buffer pointed to by Data must be large
                                  enough to contain the data type indicated
                                  by the Size parameter
**/
VOID
EFIAPI
MockWrite (
  IN    UINT64          Address,
  IN    MOCK_SPACE      AddressSpace,
  IN    MOCK_SIZE       Size,
  IN    UINT8           *Data
  );

/**
  Adds a new I/O Mock

  @param[in]  Mock                The new Mock to be added

  @retval EFI_SUCCESS             Succeeds.
  @retval EFI_INVALID_PARAMETER   Mock is a NULL pointer or Mock's Address
                                  overlaps with an already existing Mock
  @retval EFI_OUT_OF_RESOURCES    Failed to Allocate Memory for the Mock
**/
EFI_STATUS
EFIAPI
AddNewMock (
  IN    MOCK            *NewMock
  );

/**
  Deletes all Mocks.  If possible, reclaims memory.

  @retval EFI_SUCCESS             Succeeds.
  @retval EFI_OUT_OF_RESOURCES    Memory Allocation Failure
**/
EFI_STATUS
EFIAPI
DeleteAllMocks (
  VOID
  );

/**
  Gets the current value for a Mock as a 8 bit integer

  @param[in]  Mock                The Mock to get the value of

  @retval The Mock's current Value
**/
UINT8
EFIAPI
GetMockData8 (
  IN    MOCK            *Mock
  );

/**
  Sets the current value for a Mock to a 8 bit integer

  @param[in]  Mock                The Mock to set the value on
  @param[in]  Value               The value to be copied to the mock
**/
VOID
EFIAPI
SetMockData8 (
  IN    MOCK            *Mock,
  IN    UINT8           Value
  );

/**
  Gets the current value for a Mock as a 8 bit integer

  @param[in]  Mock                The Mock to get the value of

  @retval The Mock's current Value
**/
UINT16
EFIAPI
GetMockData16 (
  IN    MOCK            *Mock
  );

/**
  Sets the current value for a Mock to a 16 bit integer

  @param[in]  Mock                The Mock to set the value on
  @param[in]  Value               The value to be copied to the mock
**/
VOID
EFIAPI
SetMockData16 (
  IN    MOCK            *Mock,
  IN    UINT16          Value
  );

/**
  Gets the current value for a Mock as a 32 bit integer

  @param[in]  Mock                The Mock to get the value of

  @retval The Mock's current Value
**/
UINT32
EFIAPI
GetMockData32 (
  IN    MOCK            *Mock
  );

/**
  Sets the current value for a Mock to a 32 bit integer

  @param[in]  Mock                The Mock to set the value on
  @param[in]  Value               The value to be copied to the mock
**/
VOID
EFIAPI
SetMockData32 (
  IN    MOCK            *Mock,
  IN    UINT32          Value
  );

/**
  Gets the current value for a Mock as a 64 bit integer

  @param[in]  Mock                The Mock to get the value of

  @retval The Mock's current Value
**/
UINT64
EFIAPI
GetMockData64 (
  IN    MOCK            *Mock
  );

/**
  Sets the current value for a Mock to a 64 bit integer

  @param[in]  Mock                The Mock to set the value on
  @param[in]  Value               The value to be copied to the mock
**/
VOID
EFIAPI
SetMockData64 (
  IN    MOCK            *Mock,
  IN    UINT64          Value
  );

#ifdef __cplusplus
}
#endif
#endif
