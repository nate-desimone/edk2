/** @file
  Mock I/O Library implementation.

@copyright
  Copyright (c) 2015, Intel Corporation. All rights reserved
  SPDX-License-Identifier: BSD-2-Clause-Patent

@par Specification Reference:

**/

#include <Base.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>

#include <Library/MockIoLib.h>
#include <LibraryPrivate/MockIoDataStorageLib.h>

/**
  Convert a MOCK_SIZE to a number of bytes

  @param[in]  Size                The Mock Size

  @retval The number of bytes for the given Mock Size
**/
UINTN
GetMockSizeInBytes (
  IN    MOCK_SIZE       Size
  )
{
  switch (Size) {
  case MockSize64:
    return 8;
  case MockSize32:
    return 4;
  case MockSize16:
    return 2;
  case MockSize8:
  default:
    return 1;
  }
}

/**
  Read from the given Mock

  @param[in]  Mock                The Mock to be read from
  @param[out] Data                The data to be read.  The size of the
                                  buffer pointed to by Data must be large
                                  enough to contain the data type indicated
                                  by the Mock->Size parameter
**/
VOID
EFIAPI
ReadFromMock (
  IN    MOCK            *Mock,
  OUT   UINT8           *Data
  )
{
  UINT8   Empty[8] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
  UINTN   Size;

  Size = GetMockSizeInBytes (Mock->Size);
  switch (Mock->Type) {
  case MockTypeRw:
  case MockTypeRo:
    CopyMem ((VOID *) Data, (VOID *) &(Mock->TypeData.Data[0]), Size);
    break;
  case MockTypeCallback:
    Mock->TypeData.Callback.Read (Mock, Data);
    break;
  default:
    CopyMem ((VOID *) Data, (VOID *) &(Empty[0]), Size);
    break;
  }
}

/**
  Write to the given Mock

  @param[in]  Mock                The Mock to be written to
  @param[out] Data                The data to be write.  The size of the
                                  buffer pointed to by Data must be large
                                  enough to contain the data type indicated
                                  by the Mock->Size parameter
**/
VOID
EFIAPI
WriteToMock (
  IN    MOCK            *Mock,
  IN    UINT8           *Data
  )
{
  UINTN   Size;

  Size = GetMockSizeInBytes (Mock->Size);
  switch (Mock->Type) {
  case MockTypeRw:
    CopyMem ((VOID *) &(Mock->TypeData.Data[0]), (VOID *) Data, Size);
    break;
  case MockTypeCallback:
    Mock->TypeData.Callback.Write (Mock, Data);
    break;
  case MockTypeRo:
  default:
    break;
  }
}

/**
  Read from emulated I/O using a method that supports an unaligned read that
  spans across multiple Mocks

  @param[in]  Address             Address to read from
  @param[in]  AddressSpace        Address Space to read from
  @param[in]  Size                Size of the emulated read operation
  @param[in]  MockStorage         Pointer to the Mock Storage Area
  @param[out] Data                The data to be read.  The size of the
                                  buffer pointed to by Data must be large
                                  enough to contain the data type indicated
                                  by the Size parameter
**/
VOID
EFIAPI
CompositeMockRead (
  IN    UINT64          Address,
  IN    MOCK_SPACE      AddressSpace,
  IN    MOCK_SIZE       Size,
  IN    VOID            *MockStorage,
  OUT   UINT8           *Data
  )
{
  UINT8         Buffer[8];
  UINTN         CopySize;
  UINTN         Index;
  UINTN         MockCount;
  UINTN         MockByteSize;
  MOCK          *Mock;
  UINT8         ByteMask;
  UINT8         DataIndex;
  INT32         MockSize;

  switch (Size) {
  case MockSize64:
    CopySize = 8;
    ByteMask = 0xFF;
    break;
  case MockSize32:
    CopySize = 4;
    ByteMask = 0xF;
    break;
  case MockSize16:
    CopySize = 2;
    ByteMask = 0x3;
    break;
  case MockSize8:
  default:
    CopySize = 1;
    ByteMask = 0x1;
    break;
  }
  MockCount = GetMockCount (MockStorage);

  for (MockSize = (INT32) MockSize64; MockSize >= 0; MockSize--) {
    MockByteSize = GetMockSizeInBytes ((MOCK_SIZE)MockSize);
    for (Index = 0; Index < MockCount; Index++) {
      GetMock (MockStorage, Index, &Mock);
      if ((Mock->Size == (MOCK_SIZE) MockSize) &&
          (Mock->AddressSpace == AddressSpace)) {
        //
        // If the current mock can provide data for part of the read operation
        //
        if ((Mock->Address < (Address + CopySize)) &&
            (Address < (Mock->Address + MockByteSize))) {
          ReadFromMock (Mock, &(Buffer[0]));
          //
          // Copy the data
          //
          for (DataIndex = 0; DataIndex < CopySize; DataIndex++) {
            if (((Address + DataIndex) >= Mock->Address) &&
               ((Address + DataIndex) < (Mock->Address + MockByteSize))) {
              Data[DataIndex] = Buffer[(UINT32) (Address - Mock->Address + DataIndex)];
              ByteMask       &= (UINT8) ~(1 << DataIndex);
            }
          }
          if (ByteMask == 0) {
            return;
          }
        }
      }
    }
  }
  if (ByteMask != 0) {
    for (DataIndex = 0; DataIndex < 8; DataIndex++) {
      if ((ByteMask & (1 << DataIndex)) != 0) {
        Data[DataIndex] = 0xFF;
      }
    }
  }
}

/**
  Write to emulated I/O using a method that supports an unaligned write that
  spans across multiple Mocks

  @param[in]  Address             Address to write to
  @param[in]  AddressSpace        Address Space to write to
  @param[in]  Size                Size of the emulated write operation
  @param[in]  MockStorage         Pointer to the Mock Storage Area
  @param[in]  Data                The data to be written.  The size of the
                                  buffer pointed to by Data must be large
                                  enough to contain the data type indicated
                                  by the Size parameter
**/
VOID
EFIAPI
CompositeMockWrite (
  IN    UINT64          Address,
  IN    MOCK_SPACE      AddressSpace,
  IN    MOCK_SIZE       Size,
  IN    VOID            *MockStorage,
  IN    UINT8           *Data
  )
{
  MOCK          NewMock;
  UINT8         Buffer[8];
  EFI_STATUS    Status;
  UINTN         CopySize;
  UINTN         Index;
  UINTN         MockCount;
  UINTN         MockByteSize;
  MOCK          *Mock;
  UINT8         ByteMask;
  UINT8         DataIndex;
  INT32         MockSize;

  switch (Size) {
  case MockSize64:
    CopySize = 8;
    ByteMask = 0xFF;
    break;
  case MockSize32:
    CopySize = 4;
    ByteMask = 0xF;
    break;
  case MockSize16:
    CopySize = 2;
    ByteMask = 0x3;
    break;
  case MockSize8:
  default:
    CopySize = 1;
    ByteMask = 0x1;
    break;
  }
  MockCount = GetMockCount (MockStorage);

  for (MockSize = (INT32) MockSize64; MockSize >= 0; MockSize--) {
    MockByteSize = GetMockSizeInBytes ((MOCK_SIZE) MockSize);
    for (Index = 0; Index < MockCount; Index++) {
      GetMock (MockStorage, Index, &Mock);
      if ((Mock->Size == (MOCK_SIZE) MockSize) &&
          (Mock->AddressSpace == AddressSpace)) {
        //
        // If the current mock can provide data for part of the write operation
        //
        if ((Mock->Address < (Address + CopySize)) &&
            (Address < (Mock->Address + MockByteSize))) {
          //
          // Read from the mock
          //
          ReadFromMock (Mock, &(Buffer[0]));
          //
          // Modify the appropriate bytes in the mock
          //
          for (DataIndex = 0; DataIndex < CopySize; DataIndex++) {
            if (((Address + DataIndex) >= Mock->Address) &&
               ((Address + DataIndex)  < (Mock->Address + MockByteSize))) {
              Buffer[(UINT32) (Address - Mock->Address + DataIndex)] = Data[DataIndex];
              ByteMask       &= (UINT8) ~(1 << DataIndex);
            }
          }
          //
          // Write back to the mock
          //
          WriteToMock (Mock, &(Buffer[0]));
          if (ByteMask == 0) {
            return;
          }
        }
      }
    }
  }
  if (ByteMask != 0) {
    NewMock.Type          = MockTypeRw;
    NewMock.Size          = MockSize8;
    NewMock.AddressSpace  = AddressSpace;
    ZeroMem ((VOID *) &(NewMock.TypeData.Data[0]), sizeof (NewMock.TypeData.Data));
    for (DataIndex = 0; DataIndex < 8; DataIndex++) {
      if ((ByteMask & (1 << DataIndex)) != 0) {
        NewMock.Address           = Address + DataIndex;
        NewMock.TypeData.Data[0]  = Data[DataIndex];
        Status = AddNewMock (&NewMock);
        ASSERT_EFI_ERROR (Status);
      }
    }
  }
}

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
  )
{
  UINT8         Empty[8] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
  UINT8         Buffer[8];
  VOID          *MockStorage;
  MOCK          *Mock;
  UINTN         CopySize;
  UINTN         Index;
  UINTN         MockCount;
  EFI_STATUS    Status;

  Status = GetMockStorage (&MockStorage);
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return;
  }
  MockCount = GetMockCount (MockStorage);
  CopySize  = GetMockSizeInBytes (Size);

  for (Index = 0; Index < MockCount; Index++) {
    GetMock (MockStorage, Index, &Mock);
    if ((Mock->AddressSpace == AddressSpace) && (Mock->Address == Address) && (Mock->Size >= Size)) {
      if (Mock->Size > Size) {
        ReadFromMock (Mock, Buffer);
        CopyMem ((VOID *) Data, (VOID *) &(Buffer[0]), CopySize);
      } else {
        ReadFromMock (Mock, Data);
      }
      return;
    }
  }
  //
  // We didn't find an exact match mock
  // Check if we need to attempt a composite mock read
  //
  // @note: ((MmioRead16 (0x80) >> 8) && 0xFF) == MmioRead8 (0x81)
  //          ((IoRead16 (0x80) >> 8) && 0xFF) != IoRead8 (0x81)
  //
  if (AddressSpace == MockSpaceMmio) {
    CompositeMockRead (Address, AddressSpace, Size, MockStorage, Data);
  } else {
    CopyMem ((VOID *) Data, (VOID *) &(Empty[0]), CopySize);
  }
}

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
  )
{
  MOCK          NewMock;
  UINT8         Buffer[8];
  VOID          *MockStorage;
  MOCK          *Mock;
  UINTN         Index;
  UINTN         MockCount;
  UINTN         CopySize;
  EFI_STATUS    Status;

  Status = GetMockStorage (&MockStorage);
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return;
  }
  MockCount = GetMockCount (MockStorage);
  CopySize  = GetMockSizeInBytes (Size);

  for (Index = 0; Index < MockCount; Index++) {
    GetMock (MockStorage, Index, &Mock);
    if ((Mock->AddressSpace == AddressSpace) && (Mock->Address == Address) && (Mock->Size >= Size)) {
      if (Mock->Size > Size) {
        //
        // If size of the Mock > size of the write we need to do a read/modify/write
        //
        ReadFromMock (Mock, &(Buffer[0]));
        CopyMem ((VOID *) &(Buffer[0]), (VOID *) Data, CopySize);
        WriteToMock (Mock, &(Buffer[0]));
      } else {
        WriteToMock (Mock, Data);
      }
      return;
    }
  }
  //
  // We didn't find an exact match mock
  // Check if we need to attempt a composite mock write, otherwise
  // we need to create new a mock to emulates the CR write
  //
  // @note: ((MmioWrite16 (0x80) >> 8) && 0xFF) == MmioWrite8 (0x81)
  //          ((IoWrite16 (0x80) >> 8) && 0xFF) != IoWrite8 (0x81)
  //
  if (AddressSpace == MockSpaceMmio) {
    CompositeMockWrite (Address, AddressSpace, Size, MockStorage, Data);
  } else {
    NewMock.Type          = MockTypeRw;
    NewMock.Size          = Size;
    NewMock.Address       = Address;
    NewMock.AddressSpace  = AddressSpace;
    ZeroMem ((VOID *) &(NewMock.TypeData.Data[0]), sizeof (NewMock.TypeData.Data));
    CopyMem ((VOID *) &(NewMock.TypeData.Data[0]), (VOID *) Data, CopySize);
    Status = AddNewMock (&NewMock);
    ASSERT_EFI_ERROR (Status);
  }
}

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
  )
{
  VOID          *MockStorage;
  MOCK          *Mock;
  UINTN         Index;
  UINTN         MockCount;
  UINTN         NewMockSize;
  UINTN         MockByteSize;
  EFI_STATUS    Status;

  if (NewMock == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  Status = GetMockStorage (&MockStorage);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  MockCount   = GetMockCount (MockStorage);

  //
  // If we are adding an MMIO Mock, we need to check for address overlap with existing Mocks
  //
  if (NewMock->AddressSpace == MockSpaceMmio) {
    NewMockSize = GetMockSizeInBytes (NewMock->Size);
    for (Index = 0; Index < MockCount; Index++) {
      GetMock (MockStorage, Index, &Mock);
      if (Mock->AddressSpace == NewMock->AddressSpace) {
        MockByteSize = GetMockSizeInBytes (Mock->Size);
        if ( ( (Mock->Address                     >= NewMock->Address)                 &&
               (Mock->Address                     < (NewMock->Address + NewMockSize))) ||
             (((Mock->Address + MockByteSize)     >  NewMock->Address)                 &&
              ((Mock->Address + MockByteSize - 1) < (NewMock->Address + NewMockSize)))) {
          //
          // New Mock overlaps with an existing Mock, return Invalid Parameter
          //
          return EFI_INVALID_PARAMETER;
        }
      }
    }
  } else {
    //
    // Other types of Mocks we just need to check for overlap on the starting address
    //
    for (Index = 0; Index < MockCount; Index++) {
      GetMock (MockStorage, Index, &Mock);
      if ((Mock->AddressSpace == NewMock->AddressSpace) &&
          (Mock->Address      == NewMock->Address)) {
        //
        // New Mock overlaps with an existing Mock, return Invalid Parameter
        //
        return EFI_INVALID_PARAMETER;
      }
    }
  }
  return AddMock (MockStorage, NewMock);
}

/**
  Deletes all Mocks.  If possible, reclaims memory.

  @retval EFI_SUCCESS             Succeeds.
  @retval EFI_OUT_OF_RESOURCES    Memory Allocation Failure
**/
EFI_STATUS
EFIAPI
DeleteAllMocks (
  VOID
  )
{
  VOID          *MockStorage;
  EFI_STATUS    Status;

  Status = GetMockStorage (&MockStorage);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Status = ClearMockStorage (MockStorage);
  return Status;
}

/**
  Gets the current value for a Mock as a 8 bit integer

  @param[in]  Mock                The Mock to get the value of

  @retval The Mock's current Value
**/
UINT8
EFIAPI
GetMockData8 (
  IN    MOCK            *Mock
  )
{
  ASSERT (Mock != NULL);
  if (Mock == NULL) {
    return 0;
  }
  return Mock->TypeData.Data[0];
}

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
  )
{
  ASSERT (Mock != NULL);
  if (Mock == NULL) {
    return;
  }
  Mock->TypeData.Data[0] = Value;
}

/**
  Gets the current value for a Mock as a 8 bit integer

  @param[in]  Mock                The Mock to get the value of

  @retval The Mock's current Value
**/
UINT16
EFIAPI
GetMockData16 (
  IN    MOCK            *Mock
  )
{
  ASSERT (Mock != NULL);
  if (Mock == NULL) {
    return 0;
  }
  return *((UINT16 *) &(Mock->TypeData.Data[0]));
}

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
  )
{
  ASSERT (Mock != NULL);
  if (Mock == NULL) {
    return;
  }
  *((UINT16 *) &(Mock->TypeData.Data[0])) = Value;
}

/**
  Gets the current value for a Mock as a 32 bit integer

  @param[in]  Mock                The Mock to get the value of

  @retval The Mock's current Value
**/
UINT32
EFIAPI
GetMockData32 (
  IN    MOCK            *Mock
  )
{
  ASSERT (Mock != NULL);
  if (Mock == NULL) {
    return 0;
  }
  return *((UINT32 *) &(Mock->TypeData.Data[0]));
}

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
  )
{
  ASSERT (Mock != NULL);
  if (Mock == NULL) {
    return;
  }
  *((UINT32 *) &(Mock->TypeData.Data[0])) = Value;
}

/**
  Gets the current value for a Mock as a 64 bit integer

  @param[in]  Mock                The Mock to get the value of

  @retval The Mock's current Value
**/
UINT64
EFIAPI
GetMockData64 (
  IN    MOCK            *Mock
  )
{
  UINT64        Buffer;

  ASSERT (Mock != NULL);
  if (Mock == NULL) {
    return 0;
  }
  CopyMem (
    (VOID *) &Buffer,
    (VOID *) &(Mock->TypeData.Data[0]),
    sizeof (UINT64)
    );
  return Buffer;
}

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
  )
{
  ASSERT (Mock != NULL);
  if (Mock == NULL) {
    return;
  }
  CopyMem (
    (VOID *) &(Mock->TypeData.Data[0]),
    (VOID *) &Value,
    sizeof (UINT64)
    );
}
