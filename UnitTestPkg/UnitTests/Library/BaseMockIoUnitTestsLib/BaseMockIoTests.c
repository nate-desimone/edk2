/** @file
  Mock I/O Data Storage Library DXE Unit Tests

@copyright
  Copyright (c) 2015, Intel Corporation. All rights reserved
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/IoLib.h>
#include <Library/UnitTestLib.h>

#include <Library/MockIoLib.h>
#include <LibraryPrivate/MockIoDataStorageLib.h>

//
// This makes sure that the scratchpad size will always be big enough to hold a
// pointer by trigging a divide by zero error during compile time if it is not
//
GLOBAL_REMOVE_IF_UNREFERENCED enum { SizeTest = 1/(!!(sizeof (VOID *) <= MOCK_SCRATCHPAD_SIZE)) };

#define DelMocks   Status = DeleteAllMocks (); \
                  if (EFI_ERROR (Status)) { \
                    DEBUG ((DEBUG_INFO, "DeleteAllMocks() returned %r\n", Status)); \
                    Fail ("DeleteAllMocks() returned an error"); \
                  }

///
/// GetMockSizeInBytes() Unit Tests
///

#define MOCK_SIZE8                              1
#define MOCK_SIZE16                             2
#define MOCK_SIZE32                             4
#define MOCK_SIZE64                             8
#define RANDOM_INVALID_MOCK_OFFSET              973
#define MOCK_FAKE_ADDRESS80                     0x80
#define MOCK_FAKE_ADDRESS81                     0x81
#define MOCK_FAKE_ADDRESS82                     0x82
#define MOCK_FAKE_ADDRESS83                     0x83
#define MOCK_FAKE_ADDRESS84                     0x84
#define MOCK_FAKE_ADDRESS85                     0x85
#define MOCK_FAKE_ADDRESS86                     0x86
#define MOCK_FAKE_ADDRESS87                     0x87
#define MOCK_FAKE_DATA11                        0x11
#define MOCK_FAKE_DATA22                        0x22
#define MOCK_FAKE_DATA33                        0x33
#define MOCK_FAKE_DATA34                        0x34
#define MOCK_FAKE_DATA44                        0x44
#define MOCK_FAKE_DATA55                        0x55
#define MOCK_FAKE_DATA66                        0x66
#define MOCK_FAKE_DATA67                        0x67
#define MOCK_FAKE_DATA77                        0x77
#define MOCK_FAKE_DATA88                        0x88
#define MOCK_FAKE_DATACD                        0xCD
#define MOCK_FAKE_DATA1277                      0x1277
#define MOCK_FAKE_DATA1234                      0x1234
#define MOCK_FAKE_DATA3456                      0x3456
#define MOCK_FAKE_DATA3344                      0x3344
#define MOCK_FAKE_DATA5544                      0x5544
#define MOCK_FAKE_DATA7777                      0x7777
#define MOCK_FAKE_DATA8888                      0x8888
#define MOCK_FAKE_DATAAAAA                      0xAAAA
#define MOCK_FAKE_DATABBBB                      0xBBBB
#define MOCK_FAKE_DATACCCC                      0xCCCC
#define MOCK_FAKE_DATA77777777                  0x77777777
#define MOCK_FAKE_DATA88119922                  0x88119922
#define MOCK_FAKE_DATA88887777                  0x88887777
#define MOCK_FAKE_DATA99999999                  0x99999999
#define MOCK_FAKE_DATA9999FFFF                  0x9999FFFF
#define MOCK_FAKE_DATAFFFF9999                  0xFFFF9999
#define MOCK_FAKE_DATAAAAAAAAA                  0xAAAAAAAA
#define MOCK_FAKE_DATAAAAABBBB                  0xAAAABBBB
#define MOCK_FAKE_DATACDCDCDCD                  0xCDCDCDCD
#define MOCK_FAKE_DATA1122334455667788          0x1122334455667788
#define MOCK_FAKE_DATA1122AAAA55667788          0x1122AAAA55667788
#define MOCK_FAKE_DATA5544663388119922          0x5544663388119922
#define MOCK_FAKE_DATA5544773388119922          0x5544773388119922
#define MOCK_FAKE_DATA7777777788888888          0x7777777788888888
#define MOCK_FAKE_DATA8877665544332211          0x8877665544332211
#define MOCK_FAKE_DATACCCC77FFAAAAAAAA          0xCCCC77FFAAAAAAAA
#define MOCK_EMPTY_CR8                          0xFF
#define MOCK_EMPTY_CR32                         0xFFFFFFFF

/**
  Convert a MOCK_SIZE to a number of bytes

  @param[in]  Size                The Mock Size

  @retval The number of bytes for the given Mock Size
**/
UINTN
GetMockSizeInBytes (
  IN    MOCK_SIZE       Size
  );

//
// Test that GetMockSizeInBytes() return 1 for 8 bit
//
UNIT_TEST (GetMockSizeInBytes8Bit)
{
  AssertTrue (
    GetMockSizeInBytes (MockSize8) == MOCK_SIZE8,
    "GetMockSizeInBytes() didn't return 1 for a 8 bit mock size"
    );
}

//
// Test that GetMockSizeInBytes() return 2 for 16 bit
//
UNIT_TEST (GetMockSizeInBytes16Bit)
{
  AssertTrue (
    GetMockSizeInBytes (MockSize16) == MOCK_SIZE16,
    "GetMockSizeInBytes() didn't return 2 for a 16 bit mock size"
    );
}

//
// Test that GetMockSizeInBytes() return 4 for 32 bit
//
UNIT_TEST (GetMockSizeInBytes32Bit)
{
  AssertTrue (
    GetMockSizeInBytes (MockSize32) == MOCK_SIZE32,
    "GetMockSizeInBytes() didn't return 4 for a 32 bit mock size"
    );
}

//
// Test that GetMockSizeInBytes() return 8 for 64 bit
//
UNIT_TEST (GetMockSizeInBytes64Bit)
{
  AssertTrue (
    GetMockSizeInBytes (MockSize64) == MOCK_SIZE64,
    "GetMockSizeInBytes() didn't return 8 for a 64 bit mock size"
    );
}

//
// Test that when given an invalid Mock Size GetMockSizeInBytes() returns 1
//
UNIT_TEST (GetMockSizeInBytesInvalid)
{
  AssertTrue (
    GetMockSizeInBytes ((MOCK_SIZE) (((int)MockSize64) + RANDOM_INVALID_MOCK_OFFSET)) == MOCK_SIZE8,
    "GetMockSizeInBytes() didn't return 1 for an invalid mock size"
    );
}

///
/// ReadFromMock() Unit Tests
///

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
  );

//
// Test ReadFromMock() with a Read/Write Mock
//
UNIT_TEST (ReadFromRwMock)
{
  MOCK                    Mock;
  UINT8                   Data[MOCK_DATA_SIZE];

  ZeroMem ((VOID *) &Mock, sizeof (MOCK));
  ZeroMem ((VOID *) &Mock, sizeof (Data));
  Mock.Type             = MockTypeRw;
  Mock.Size             = MockSize8;
  Mock.Address          = MOCK_FAKE_ADDRESS80;
  Mock.AddressSpace     = MockSpaceMmio;
  Mock.TypeData.Data[0] = MOCK_FAKE_DATA77;

  ReadFromMock (&Mock, &(Data[0]));
  AssertTrue (Data[0] == MOCK_FAKE_DATA77, "Read from Mock didn't return the correct value");
}

//
// Test ReadFromMock() with a Read Only Mock
//
UNIT_TEST (ReadFromRoMock)
{
  MOCK                    Mock;
  UINT8                   Data[MOCK_DATA_SIZE];

  ZeroMem ((VOID *) &Mock, sizeof (MOCK));
  ZeroMem ((VOID *) &Mock, sizeof (Data));
  Mock.Type             = MockTypeRo;
  Mock.Size             = MockSize8;
  Mock.Address          = MOCK_FAKE_ADDRESS80;
  Mock.AddressSpace     = MockSpaceMmio;
  Mock.TypeData.Data[0] = MOCK_FAKE_DATA77;

  ReadFromMock (&Mock, &(Data[0]));
  AssertTrue (Data[0] == MOCK_FAKE_DATA77, "ReadFromMock() returned an incorrect value");
}

//
// Test ReadFromMock() with a callback Mock
//
VOID
EFIAPI
ReadFromCallbackMockTestCallback (
  IN    MOCK            *Mock,
  OUT   UINT8           *Data
  )
{
  //
  // Set flag that indicates the callback was called
  //
   **((BOOLEAN **) &(Mock->TypeData.Callback.ScratchPad[0])) = TRUE;
   Data[0] = MOCK_FAKE_DATA67;
}

UNIT_TEST (ReadFromCallbackMock)
{
  MOCK                    Mock;
  UINT8                   Data[MOCK_DATA_SIZE];
  BOOLEAN                 CallbackRun;
  VOID                    *CallbackRunPtr;

  ZeroMem ((VOID *) &Mock, sizeof (MOCK));
  ZeroMem ((VOID *) &Mock, sizeof (Data));
  CallbackRun                 = FALSE;
  Mock.Type                   = MockTypeCallback;
  Mock.Size                   = MockSize8;
  Mock.Address                = MOCK_FAKE_ADDRESS80;
  Mock.AddressSpace           = MockSpaceMmio;
  Mock.TypeData.Callback.Read = ReadFromCallbackMockTestCallback;
  CallbackRunPtr              = (VOID *) &CallbackRun;
  CopyMem (
    (VOID *) &(Mock.TypeData.Callback.ScratchPad[0]),
    (VOID *) &CallbackRunPtr,
    sizeof (BOOLEAN *)
    );

  ReadFromMock (&Mock, &(Data[0]));
  AssertTrue (CallbackRun, "ReadFromMock() didn't run the callback");
  AssertTrue (Data[0] == MOCK_FAKE_DATA67, "ReadFromMock() returned an incorrect value");
}

//
// Test ReadFromMock() with an invalid Mock Type
//
UNIT_TEST (ReadFromInvalidMockType)
{
  MOCK                    Mock;
  UINT8                   Data[MOCK_DATA_SIZE];

  ZeroMem ((VOID *) &Mock, sizeof (MOCK));
  ZeroMem ((VOID *) &Mock, sizeof (Data));
  Mock.Type                   = (MOCK_TYPE) (((int) MockTypeCallback) + RANDOM_INVALID_MOCK_OFFSET);
  Mock.Size                   = MockSize8;
  Mock.Address                = MOCK_FAKE_ADDRESS80;
  Mock.AddressSpace           = MockSpaceMmio;
  ReadFromMock (&Mock, &(Data[0]));
  AssertTrue (Data[0] == MOCK_EMPTY_CR8, "ReadFromMock() returned an incorrect value");
}

///
/// WriteToMock() Unit Tests
///

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
  );

//
// Test WriteToMock() with a Read/Write Mock
//
UNIT_TEST (WriteToRwMock)
{
  MOCK                    Mock;
  UINT8                   Data[MOCK_DATA_SIZE];

  ZeroMem ((VOID *) &Mock, sizeof (MOCK));
  ZeroMem ((VOID *) &Mock, sizeof (Data));
  Mock.Type             = MockTypeRw;
  Mock.Size             = MockSize8;
  Mock.Address          = MOCK_FAKE_ADDRESS80;
  Mock.AddressSpace     = MockSpaceMmio;
  Data[0]               = MOCK_FAKE_DATA77;

  WriteToMock (&Mock, &(Data[0]));
  AssertTrue (Mock.TypeData.Data[0] == MOCK_FAKE_DATA77, "WriteToMock() didn't write the correct value");
}

//
// Test WriteToMock() with a Read Only Mock
//
UNIT_TEST (WriteToRoMock)
{
  MOCK                    Mock;
  UINT8                   Data[MOCK_DATA_SIZE];

  ZeroMem ((VOID *) &Mock, sizeof (MOCK));
  ZeroMem ((VOID *) &Mock, sizeof (Data));
  Mock.Type             = MockTypeRo;
  Mock.Size             = MockSize8;
  Mock.Address          = MOCK_FAKE_ADDRESS80;
  Mock.AddressSpace     = MockSpaceMmio;
  Data[0]               = MOCK_FAKE_DATA77;

  WriteToMock (&Mock, &(Data[0]));
  AssertTrue (Mock.TypeData.Data[0] == 0x0, "WriteToMock() wrote data when it should not have");
}

//
// Test WriteToMock() with a callback Mock
//
VOID
EFIAPI
WriteToCallbackMockTestCallback (
  IN    MOCK            *Mock,
  IN    UINT8           *Data
  )
{
  //
  // Set flag that indicates the callback was called
  //
   **((BOOLEAN **) &(Mock->TypeData.Callback.ScratchPad[0])) = TRUE;
}

UNIT_TEST (WriteToCallbackMock)
{
  MOCK                    Mock;
  UINT8                   Data[MOCK_DATA_SIZE];
  BOOLEAN                 CallbackRun;
  VOID                    *CallbackRunPtr;

  ZeroMem ((VOID *) &Mock, sizeof (MOCK));
  ZeroMem ((VOID *) &Mock, sizeof (Data));
  CallbackRun                   = FALSE;
  Mock.Type                     = MockTypeCallback;
  Mock.Size                     = MockSize8;
  Mock.Address                  = MOCK_FAKE_ADDRESS80;
  Mock.AddressSpace             = MockSpaceMmio;
  Mock.TypeData.Callback.Write  = WriteToCallbackMockTestCallback;
  CallbackRunPtr                = (VOID *) &CallbackRun;
  CopyMem (
    (VOID *) &(Mock.TypeData.Callback.ScratchPad[0]),
    (VOID *) &CallbackRunPtr,
    sizeof (BOOLEAN *)
    );

  WriteToMock (&Mock, &(Data[0]));
  AssertTrue (CallbackRun, "WriteToMock() didn't run the callback");
}

//
// Test WriteToMock() with an invalid Mock Type
//
UNIT_TEST (WriteToInvalidMockType)
{
  MOCK                    Mock;
  UINT8                   Data[MOCK_DATA_SIZE];

  ZeroMem ((VOID *) &Mock, sizeof (MOCK));
  ZeroMem ((VOID *) &Mock, sizeof (Data));
  Mock.Type                   = (MOCK_TYPE) (((int) MockTypeCallback) + RANDOM_INVALID_MOCK_OFFSET);
  Mock.Size                   = MockSize8;
  Mock.Address                = MOCK_FAKE_ADDRESS80;
  Mock.AddressSpace           = MockSpaceMmio;
  Data[0]                     = MOCK_FAKE_DATA77;
  WriteToMock (&Mock, &(Data[0]));
  AssertTrue (Mock.TypeData.Data[0] == 0x0, "WriteToMock() wrote data when it should not have");
}

///
/// MockRead() Unit Tests
///

//
// Simple Port I/O Read Test
//
UNIT_TEST (SimpleIoRead)
{
  MOCK                    Mock;
  EFI_STATUS              Status;
  UINT16                  Buffer;

  Mock.Type         = MockTypeRw;
  Mock.Size         = MockSize16;
  Mock.Address      = MOCK_FAKE_ADDRESS80;
  Mock.AddressSpace = MockSpaceIo;
  Buffer            = MOCK_FAKE_DATA3456;
  CopyMem ((VOID *) &(Mock.TypeData.Data[0]), (VOID *) &Buffer, sizeof (UINT16));
  Status = AddNewMock (&Mock);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "AddNewMock() returned %r\n", Status));
    Fail ("AddNewMock() returned an error");
  }
  Buffer = IoRead16 (MOCK_FAKE_ADDRESS80);
  DelMocks
  if (Buffer != MOCK_FAKE_DATA3456) {
    DEBUG ((DEBUG_INFO, "Mock I/O Read return bad value: %d\n", (UINTN)Buffer));
  }
  AssertTrue (Buffer == MOCK_FAKE_DATA3456, "Mock I/O read returned bad data");
}

//
// Truncated I/O Port Read Test
//
UNIT_TEST (TruncatedIoRead)
{
  MOCK                    Mock;
  EFI_STATUS              Status;
  UINT16                  Buffer16;
  UINT8                   Buffer;

  Mock.Type         = MockTypeRo;
  Mock.Size         = MockSize16;
  Mock.Address      = MOCK_FAKE_ADDRESS80;
  Mock.AddressSpace = MockSpaceIo;
  Buffer16          = MOCK_FAKE_DATA1234;
  CopyMem ((VOID *) &(Mock.TypeData.Data[0]), (VOID *) &Buffer16, sizeof (UINT16));
  Status = AddNewMock (&Mock);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "AddNewMock() returned %r\n", Status));
    Fail ("AddNewMock() returned an error");
  }
  Buffer = IoRead8 (MOCK_FAKE_ADDRESS80);
  DelMocks
  if (Buffer != MOCK_FAKE_DATA34) {
    DEBUG ((DEBUG_INFO, "Mock I/O Read return bad value: %d\n", (UINTN)Buffer));
  }
  AssertTrue (Buffer == MOCK_FAKE_DATA34, "Mock I/O read returned bad data");
}

//
// Test I/O Port Read when there is no Mock
//
UNIT_TEST (NoMockIoRead)
{
  EFI_STATUS              Status;
  UINT32                  Buffer;

  DelMocks
  Buffer = IoRead32 (MOCK_FAKE_ADDRESS80);
  AssertTrue (Buffer == MOCK_EMPTY_CR32, "Mock I/O read returned bad data");
}

//
// Simple MMIO Read Test
//
UNIT_TEST (SimpleMmioRead)
{
  MOCK                    Mock;
  EFI_STATUS              Status;
  UINT16                  Buffer;

  Mock.Type         = MockTypeRw;
  Mock.Size         = MockSize16;
  Mock.Address      = MOCK_FAKE_ADDRESS80;
  Mock.AddressSpace = MockSpaceMmio;
  Buffer            = MOCK_FAKE_DATA3456;
  CopyMem ((VOID *) &(Mock.TypeData.Data[0]), (VOID *) &Buffer, sizeof (UINT16));
  Status = AddNewMock (&Mock);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "AddNewMock() returned %r\n", Status));
    Fail ("AddNewMock() returned an error");
  }
  Buffer = MmioRead16 (MOCK_FAKE_ADDRESS80);
  DelMocks
  if (Buffer != MOCK_FAKE_DATA3456) {
    DEBUG ((DEBUG_INFO, "Mock I/O Read return bad value: %d\n", (UINTN)Buffer));
  }
  AssertTrue (Buffer == MOCK_FAKE_DATA3456, "Mock I/O read returned bad data");
}

//
// Truncated MMIO Read Test
//
UNIT_TEST (TruncatedMmioRead)
{
  MOCK                    Mock;
  EFI_STATUS              Status;
  UINT16                  Buffer16;
  UINT8                   Buffer;

  Mock.Type         = MockTypeRo;
  Mock.Size         = MockSize16;
  Mock.Address      = MOCK_FAKE_ADDRESS80;
  Mock.AddressSpace = MockSpaceMmio;
  Buffer16          = MOCK_FAKE_DATA1234;
  CopyMem ((VOID *) &(Mock.TypeData.Data[0]), (VOID *) &Buffer16, sizeof (UINT16));
  Status = AddNewMock (&Mock);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "AddNewMock() returned %r\n", Status));
    Fail ("AddNewMock() returned an error");
  }
  Buffer = MmioRead8 (MOCK_FAKE_ADDRESS80);
  DelMocks
  if (Buffer != MOCK_FAKE_DATA34) {
    DEBUG ((DEBUG_INFO, "Mock I/O Read return bad value: %d\n", (UINTN)Buffer));
  }
  AssertTrue (Buffer == MOCK_FAKE_DATA34, "Mock I/O read returned bad data");
}

//
// Test MMIO Read when there is no Mock
//
UNIT_TEST (NoMockMmioRead)
{
  EFI_STATUS              Status;
  UINT32                  Buffer;

  DelMocks
  Buffer = MmioRead32 (MOCK_FAKE_ADDRESS80);
  AssertTrue (Buffer == MOCK_EMPTY_CR32, "Mock I/O read returned bad data");
}

//
// Test a 32 bit composite read across two 16 bit CR Mocks
//
UNIT_TEST (Two16Mock32MmioRead)
{
  MOCK                    Mock;
  EFI_STATUS              Status;
  UINT32                  Buffer32;
  UINT16                  Buffer16;

  Mock.Type         = MockTypeRw;
  Mock.Size         = MockSize16;
  Mock.Address      = MOCK_FAKE_ADDRESS80;
  Mock.AddressSpace = MockSpaceMmio;
  Buffer16          = MOCK_FAKE_DATA7777;
  CopyMem ((VOID *) &(Mock.TypeData.Data[0]), (VOID *) &Buffer16, sizeof (UINT16));
  Status = AddNewMock (&Mock);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "AddNewMock() returned %r\n", Status));
    Fail ("AddNewMock() returned an error");
  }
  Mock.Type     = MockTypeRo;
  Mock.Address  = MOCK_FAKE_ADDRESS82;
  Buffer16      = MOCK_FAKE_DATA8888;
  CopyMem ((VOID *) &(Mock.TypeData.Data[0]), (VOID *) &Buffer16, sizeof (UINT16));
  Status = AddNewMock (&Mock);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "AddNewMock() returned %r\n", Status));
    Fail ("AddNewMock() returned an error");
  }
  Buffer32  = MmioRead32 (MOCK_FAKE_ADDRESS80);
  DelMocks
  AssertTrue (Buffer32 == MOCK_FAKE_DATA88887777, "Mock I/O read returned bad data");
}

//
// Test reading 2 bytes in the middle of a 64 bit Mock
//
UNIT_TEST (Partial64Mock16MmioRead)
{
  MOCK                    Mock;
  UINT64                  Buffer64;
  EFI_STATUS              Status;
  UINT16                  Buffer16;

  Mock.Type         = MockTypeRw;
  Mock.Size         = MockSize64;
  Mock.Address      = MOCK_FAKE_ADDRESS80;
  Mock.AddressSpace = MockSpaceMmio;
  Buffer64 = MOCK_FAKE_DATA1122334455667788;
  CopyMem ((VOID *) &(Mock.TypeData.Data[0]), (VOID *) &Buffer64, sizeof (UINT64));
  Status = AddNewMock (&Mock);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "AddNewMock() returned %r\n", Status));
    Fail ("AddNewMock() returned an error");
  }
  Buffer16 = MmioRead16 (MOCK_FAKE_ADDRESS84);
  DelMocks
  if (Buffer16 != MOCK_FAKE_DATA3344) {
    DEBUG ((DEBUG_INFO, "Mock I/O Read return bad value: 0x%x\n", (UINTN)Buffer16));
  }
  AssertTrue (Buffer16 == MOCK_FAKE_DATA3344, "Mock I/O read returned bad data");
}

//
// Test reading 2 bytes at the start of a 32 bit mock
//
UNIT_TEST (UpperOverlap32MockMmioRead)
{
  MOCK                    Mock;
  EFI_STATUS              Status;
  UINT32                  Buffer;

  Mock.Type         = MockTypeRw;
  Mock.Size         = MockSize32;
  Mock.Address      = MOCK_FAKE_ADDRESS82;
  Mock.AddressSpace = MockSpaceMmio;
  Buffer            = MOCK_FAKE_DATA99999999;
  CopyMem ((VOID *) &(Mock.TypeData.Data[0]), (VOID *) &Buffer, sizeof (UINT32));
  Status = AddNewMock (&Mock);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "AddNewMock() returned %r\n", Status));
    Fail ("AddNewMock() returned an error");
  }
  Buffer = MmioRead32 (MOCK_FAKE_ADDRESS80);
  DelMocks
  if (Buffer != MOCK_FAKE_DATA9999FFFF) {
    DEBUG ((DEBUG_INFO, "Mock I/O Read return bad value: 0x%x\n", (UINTN)Buffer));
  }
  AssertTrue (Buffer == MOCK_FAKE_DATA9999FFFF, "Mock I/O read returned bad data");
}

//
// Test reading 2 bytes at the start of a 32 bit mock
//
UNIT_TEST (LowerOverlap32MockMmioRead)
{
  MOCK                    Mock;
  EFI_STATUS              Status;
  UINT32                  Buffer;

  Mock.Type         = MockTypeRw;
  Mock.Size         = MockSize32;
  Mock.Address      = MOCK_FAKE_ADDRESS82;
  Mock.AddressSpace = MockSpaceMmio;
  Buffer            = MOCK_FAKE_DATA99999999;
  CopyMem ((VOID *) &(Mock.TypeData.Data[0]), (VOID *) &Buffer, sizeof (UINT32));
  Status = AddNewMock (&Mock);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "AddNewMock() returned %r\n", Status));
    Fail ("AddNewMock() returned an error");
  }
  Buffer = MmioRead32 (MOCK_FAKE_ADDRESS84);
  DelMocks
  if (Buffer != MOCK_FAKE_DATAFFFF9999) {
    DEBUG ((DEBUG_INFO, "Mock I/O Read return bad value: 0x%x\n", (UINTN)Buffer));
  }
  AssertTrue (Buffer == MOCK_FAKE_DATAFFFF9999, "Mock I/O read returned bad data");
}

//
// Complex MMIO Read
//
UNIT_TEST (ComplexMmioRead)
{
  MOCK                    Mock;
  UINT64                  Buffer64;
  EFI_STATUS              Status;
  UINT32                  Buffer32;
  UINT16                  Buffer16;
  UINT8                   Buffer8;

  Mock.Type         = MockTypeRw;
  Mock.Size         = MockSize32;
  Mock.Address      = MOCK_FAKE_ADDRESS80;
  Mock.AddressSpace = MockSpaceMmio;
  Buffer32          = MOCK_FAKE_DATAAAAAAAAA;
  CopyMem ((VOID *) &(Mock.TypeData.Data[0]), (VOID *) &Buffer32, sizeof (UINT32));
  Status = AddNewMock (&Mock);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "AddNewMock() returned %r\n", Status));
    Fail ("AddNewMock() returned an error");
  }
  Mock.Type     = MockTypeRo;
  Mock.Size     = MockSize8;
  Mock.Address  = MOCK_FAKE_ADDRESS85;
  Buffer8       = MOCK_FAKE_DATA77;
  CopyMem ((VOID *) &(Mock.TypeData.Data[0]), (VOID *) &Buffer8, sizeof (UINT8));
  Status = AddNewMock (&Mock);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "AddNewMock() returned %r\n", Status));
    Fail ("AddNewMock() returned an error");
  }
  Mock.Type     = MockTypeRw;
  Mock.Size     = MockSize16;
  Mock.Address  = MOCK_FAKE_ADDRESS86;
  Buffer16      = MOCK_FAKE_DATACCCC;
  CopyMem ((VOID *) &(Mock.TypeData.Data[0]), (VOID *) &Buffer16, sizeof (UINT16));
  Status = AddNewMock (&Mock);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "AddNewMock() returned %r\n", Status));
    Fail ("AddNewMock() returned an error");
  }
  Buffer64 = MmioRead64 (MOCK_FAKE_ADDRESS80);
  DelMocks
  AssertTrue (Buffer64 == MOCK_FAKE_DATACCCC77FFAAAAAAAA, "Mock I/O read returned bad data");
}

//
// Eight 8 bit mocks MMIO Read
//
UNIT_TEST (Eight8BitMockMmioRead)
{
  MOCK                    Mock;
  UINT64                  Buffer64;
  EFI_STATUS              Status;

  Mock.Type         = MockTypeRw;
  Mock.Size         = MockSize8;
  Mock.Address      = MOCK_FAKE_ADDRESS80;
  Mock.AddressSpace = MockSpaceMmio;
  ZeroMem ((VOID *) &(Mock.TypeData.Data[0]), sizeof (Mock.TypeData.Data));
  Mock.TypeData.Data[0] = MOCK_FAKE_DATA11;
  Status = AddNewMock (&Mock);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "AddNewMock() returned %r\n", Status));
    Fail ("AddNewMock() returned an error");
  }
  Mock.Type             = MockTypeRo;
  Mock.Address          = MOCK_FAKE_ADDRESS81;
  Mock.TypeData.Data[0] = MOCK_FAKE_DATA22;
  Status = AddNewMock (&Mock);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "AddNewMock() returned %r\n", Status));
    Fail ("AddNewMock() returned an error");
  }
  Mock.Type             = MockTypeRw;
  Mock.Address          = MOCK_FAKE_ADDRESS82;
  Mock.TypeData.Data[0] = MOCK_FAKE_DATA33;
  Status = AddNewMock (&Mock);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "AddNewMock() returned %r\n", Status));
    Fail ("AddNewMock() returned an error");
  }
  Mock.Type             = MockTypeRo;
  Mock.Address          = MOCK_FAKE_ADDRESS83;
  Mock.TypeData.Data[0] = MOCK_FAKE_DATA44;
  Status = AddNewMock (&Mock);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "AddNewMock() returned %r\n", Status));
    Fail ("AddNewMock() returned an error");
  }
  Mock.Type             = MockTypeRw;
  Mock.Address          = MOCK_FAKE_ADDRESS84;
  Mock.TypeData.Data[0] = MOCK_FAKE_DATA55;
  Status = AddNewMock (&Mock);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "AddNewMock() returned %r\n", Status));
    Fail ("AddNewMock() returned an error");
  }
  Mock.Type             = MockTypeRo;
  Mock.Address          = MOCK_FAKE_ADDRESS85;
  Mock.TypeData.Data[0] = MOCK_FAKE_DATA66;
  Status = AddNewMock (&Mock);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "AddNewMock() returned %r\n", Status));
    Fail ("AddNewMock() returned an error");
  }
  Mock.Type             = MockTypeRw;
  Mock.Address          = MOCK_FAKE_ADDRESS86;
  Mock.TypeData.Data[0] = MOCK_FAKE_DATA77;
  Status = AddNewMock (&Mock);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "AddNewMock() returned %r\n", Status));
    Fail ("AddNewMock() returned an error");
  }
  Mock.Type             = MockTypeRo;
  Mock.Address          = MOCK_FAKE_ADDRESS87;
  Mock.TypeData.Data[0] = MOCK_FAKE_DATA88;
  Status = AddNewMock (&Mock);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "AddNewMock() returned %r\n", Status));
    Fail ("AddNewMock() returned an error");
  }
  Buffer64 = MmioRead64 (MOCK_FAKE_ADDRESS80);
  DelMocks
  AssertTrue (Buffer64 == MOCK_FAKE_DATA8877665544332211, "Mock I/O read returned bad data");
}

///
/// MockWrite() Unit Tests
///

//
// Simple Port I/O Write Test
//
UNIT_TEST (SimpleIoWrite)
{
  MOCK                    Mock;
  MOCK                    *MockPtr;
  VOID                    *MockStorage;
  EFI_STATUS              Status;
  UINT16                  Buffer;
  BOOLEAN                 TestPass;

  Mock.Type         = MockTypeRw;
  Mock.Size         = MockSize16;
  Mock.Address      = MOCK_FAKE_ADDRESS80;
  Mock.AddressSpace = MockSpaceIo;
  Buffer            = MOCK_FAKE_DATA3456;
  CopyMem ((VOID *) &(Mock.TypeData.Data[0]), (VOID *) &Buffer, sizeof (UINT16));
  Status = AddNewMock (&Mock);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "AddNewMock() returned %r\n", Status));
    Fail ("AddNewMock() returned an error");
  }
  Status = GetMockStorage (&MockStorage);
  if (EFI_ERROR (Status) || MockStorage == NULL) {
    DEBUG ((DEBUG_INFO, "GetMockStorage() returned %r\n", Status));
    Fail ("GetMockStorage() returned an error");
  }
  GetMock (MockStorage, 0, &MockPtr);
  if (MockPtr == NULL) {
    Fail ("GetMock() returned an error");
  }
  Buffer = IoWrite16 (MOCK_FAKE_ADDRESS80, MOCK_FAKE_DATA7777);
  if ( *((UINT16 *) &(MockPtr->TypeData.Data[0])) == MOCK_FAKE_DATA7777) {
    TestPass = TRUE;
  } else {
    TestPass = FALSE;
  }
  DelMocks
  AssertTrue (TestPass, "Mock I/O read returned bad data");
}

//
// Test writting to a Read Only Port I/O Mock
//
UNIT_TEST (IoWriteToRoMock)
{
  MOCK                    Mock;
  MOCK                    *MockPtr;
  VOID                    *MockStorage;
  EFI_STATUS              Status;
  UINT16                  Buffer;
  BOOLEAN                 TestPass;

  Mock.Type         = MockTypeRo;
  Mock.Size         = MockSize16;
  Mock.Address      = MOCK_FAKE_ADDRESS80;
  Mock.AddressSpace = MockSpaceIo;
  Buffer            = MOCK_FAKE_DATA3456;
  CopyMem ((VOID *) &(Mock.TypeData.Data[0]), (VOID *) &Buffer, sizeof (UINT16));
  Status = AddNewMock (&Mock);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "AddNewMock() returned %r\n", Status));
    Fail ("AddNewMock() returned an error");
  }
  Status = GetMockStorage (&MockStorage);
  if (EFI_ERROR (Status) || MockStorage == NULL) {
    DEBUG ((DEBUG_INFO, "GetMockStorage() returned %r\n", Status));
    Fail ("GetMockStorage() returned an error");
  }
  GetMock (MockStorage, 0, &MockPtr);
  if (MockPtr == NULL) {
    Fail ("GetMock() returned an error");
  }
  Buffer = IoWrite16 (MOCK_FAKE_ADDRESS80, MOCK_FAKE_DATA7777);
  if ( *((UINT16 *) &(MockPtr->TypeData.Data[0])) == MOCK_FAKE_DATA3456) {
    TestPass = TRUE;
  } else {
    TestPass = FALSE;
  }
  DelMocks
  AssertTrue (TestPass, "Mock I/O read returned bad data");
}

//
// Truncated I/O Port Write Test
//
UNIT_TEST (TruncatedIoWrite)
{
  MOCK                    Mock;
  MOCK                    *MockPtr;
  VOID                    *MockStorage;
  EFI_STATUS              Status;
  UINT16                  Buffer16;
  UINT8                   Buffer;
  BOOLEAN                 TestPass;

  Mock.Type         = MockTypeRw;
  Mock.Size         = MockSize16;
  Mock.Address      = MOCK_FAKE_ADDRESS80;
  Mock.AddressSpace = MockSpaceIo;
  Buffer16          = MOCK_FAKE_DATA1234;
  CopyMem ((VOID *) &(Mock.TypeData.Data[0]), (VOID *) &Buffer16, sizeof (UINT16));
  Status = AddNewMock (&Mock);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "AddNewMock() returned %r\n", Status));
    Fail ("AddNewMock() returned an error");
  }
  Status = GetMockStorage (&MockStorage);
  if (EFI_ERROR (Status) || MockStorage == NULL) {
    DEBUG ((DEBUG_INFO, "GetMockStorage() returned %r\n", Status));
    Fail ("GetMockStorage() returned an error");
  }
  GetMock (MockStorage, 0, &MockPtr);
  if (MockPtr == NULL) {
    Fail ("GetMock() returned an error");
  }
  Buffer = IoWrite8 (MOCK_FAKE_ADDRESS80, MOCK_FAKE_DATA77);
  if ( *((UINT16 *) &(MockPtr->TypeData.Data[0])) == MOCK_FAKE_DATA1277) {
    TestPass = TRUE;
  } else {
    TestPass = FALSE;
  }
  DelMocks
  AssertTrue (TestPass, "Mock I/O read returned bad data");
}

//
// Test I/O Port Write when there is no Mock
//
UNIT_TEST (NoMockIoWrite)
{
  MOCK                    Mock;
  MOCK                    *MockPtr;
  VOID                    *MockStorage;
  EFI_STATUS              Status;
  UINTN                   MockCount;

  DelMocks
  Status = GetMockStorage (&MockStorage);
  if (EFI_ERROR (Status) || MockStorage == NULL) {
    DEBUG ((DEBUG_INFO, "GetMockStorage() returned %r\n", Status));
    Fail ("GetMockStorage() returned an error");
  }
  IoWrite32 (MOCK_FAKE_ADDRESS80, MOCK_FAKE_DATACDCDCDCD);
  MockCount = GetMockCount (MockStorage);
  if (MockCount > 0) {
    GetMock (MockStorage, 0, &MockPtr);
    if (MockPtr == NULL) {
      DelMocks
      Fail ("GetMock() returned an error");
    }
    CopyMem ((VOID *) &Mock, (VOID *) MockPtr, sizeof (MOCK));
  }
  DelMocks
  AssertTrue (MockCount == 1, "Mock I/O write to non-existant address didn't create a new Mock");
  AssertTrue (Mock.Type == MockTypeRw, "New Mock is not Read/Write");
  AssertTrue (Mock.Size == MockSize32, "New Mock is not 32 bit");
  AssertTrue (Mock.Address == MOCK_FAKE_ADDRESS80, "New Mock is not at address 0x80");
  AssertTrue (Mock.AddressSpace == MockSpaceIo, "New Mock is not an I/O Port Mock");
  AssertTrue (*((UINT32 *) &(Mock.TypeData.Data[0])) == MOCK_FAKE_DATACDCDCDCD, "New Mock does not have the right value");
}

//
// Simple MMIO Write Test
//
UNIT_TEST (SimpleMmioWrite)
{
  MOCK                    Mock;
  MOCK                    *MockPtr;
  VOID                    *MockStorage;
  EFI_STATUS              Status;
  UINT16                  Buffer;
  BOOLEAN                 TestPass;

  Mock.Type         = MockTypeRw;
  Mock.Size         = MockSize16;
  Mock.Address      = MOCK_FAKE_ADDRESS80;
  Mock.AddressSpace = MockSpaceMmio;
  Buffer            = MOCK_FAKE_DATA3456;
  CopyMem ((VOID *) &(Mock.TypeData.Data[0]), (VOID *) &Buffer, sizeof (UINT16));
  Status = AddNewMock (&Mock);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "AddNewMock() returned %r\n", Status));
    Fail ("AddNewMock() returned an error");
  }
  Status = GetMockStorage (&MockStorage);
  if (EFI_ERROR (Status) || MockStorage == NULL) {
    DEBUG ((DEBUG_INFO, "GetMockStorage() returned %r\n", Status));
    Fail ("GetMockStorage() returned an error");
  }
  GetMock (MockStorage, 0, &MockPtr);
  if (MockPtr == NULL) {
    Fail ("GetMock() returned an error");
  }
  Buffer = MmioWrite16 (MOCK_FAKE_ADDRESS80, MOCK_FAKE_DATA7777);
  if ( *((UINT16 *) &(MockPtr->TypeData.Data[0])) == MOCK_FAKE_DATA7777) {
    TestPass = TRUE;
  } else {
    TestPass = FALSE;
  }
  DelMocks
  AssertTrue (TestPass, "Mock I/O read returned bad data");
}

//
// Test writting to a Read Only MMIO Mock
//
UNIT_TEST (MmioWriteToRoMock)
{
  MOCK                    Mock;
  MOCK                    *MockPtr;
  VOID                    *MockStorage;
  EFI_STATUS              Status;
  UINT16                  Buffer;
  BOOLEAN                 TestPass;

  Mock.Type         = MockTypeRo;
  Mock.Size         = MockSize16;
  Mock.Address      = MOCK_FAKE_ADDRESS80;
  Mock.AddressSpace = MockSpaceMmio;
  Buffer            = MOCK_FAKE_DATA3456;
  CopyMem ((VOID *) &(Mock.TypeData.Data[0]), (VOID *) &Buffer, sizeof (UINT16));
  Status = AddNewMock (&Mock);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "AddNewMock() returned %r\n", Status));
    Fail ("AddNewMock() returned an error");
  }
  Status = GetMockStorage (&MockStorage);
  if (EFI_ERROR (Status) || MockStorage == NULL) {
    DEBUG ((DEBUG_INFO, "GetMockStorage() returned %r\n", Status));
    Fail ("GetMockStorage() returned an error");
  }
  GetMock (MockStorage, 0, &MockPtr);
  if (MockPtr == NULL) {
    Fail ("GetMock() returned an error");
  }
  Buffer = MmioWrite16 (MOCK_FAKE_ADDRESS80, MOCK_FAKE_DATA7777);
  if ( *((UINT16 *) &(MockPtr->TypeData.Data[0])) == MOCK_FAKE_DATA3456) {
    TestPass = TRUE;
  } else {
    TestPass = FALSE;
  }
  DelMocks
  AssertTrue (TestPass, "Mock I/O read returned bad data");
}

//
// Truncated MMIO Write Test
//
UNIT_TEST (TruncatedMmioWrite)
{
  MOCK                    Mock;
  MOCK                    *MockPtr;
  VOID                    *MockStorage;
  EFI_STATUS              Status;
  UINT16                  Buffer16;
  UINT8                   Buffer;
  BOOLEAN                 TestPass;

  Mock.Type         = MockTypeRw;
  Mock.Size         = MockSize16;
  Mock.Address      = MOCK_FAKE_ADDRESS80;
  Mock.AddressSpace = MockSpaceMmio;
  Buffer16          = MOCK_FAKE_DATA1234;
  CopyMem ((VOID *) &(Mock.TypeData.Data[0]), (VOID *) &Buffer16, sizeof (UINT16));
  Status = AddNewMock (&Mock);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "AddNewMock() returned %r\n", Status));
    Fail ("AddNewMock() returned an error");
  }
  Status = GetMockStorage (&MockStorage);
  if (EFI_ERROR (Status) || MockStorage == NULL) {
    DEBUG ((DEBUG_INFO, "GetMockStorage() returned %r\n", Status));
    Fail ("GetMockStorage() returned an error");
  }
  GetMock (MockStorage, 0, &MockPtr);
  if (MockPtr == NULL) {
    Fail ("GetMock() returned an error");
  }
  Buffer = MmioWrite8 (MOCK_FAKE_ADDRESS80, MOCK_FAKE_DATA77);
  if ( *((UINT16 *) &(MockPtr->TypeData.Data[0])) == MOCK_FAKE_DATA1277) {
    TestPass = TRUE;
  } else {
    TestPass = FALSE;
  }
  DelMocks
  AssertTrue (TestPass, "Mock I/O wrote bad data");
}

//
// Test MMIO Write when there is no Mock
//
UNIT_TEST (NoMockMmioWrite)
{
  MOCK                    *Mock;
  VOID                    *MockStorage;
  EFI_STATUS              Status;
  UINTN                   MockCount;
  UINTN                   Index;

  DelMocks
  Status = GetMockStorage (&MockStorage);
  if (EFI_ERROR (Status) || MockStorage == NULL) {
    DEBUG ((DEBUG_INFO, "GetMockStorage() returned %r\n", Status));
    Fail ("GetMockStorage() returned an error");
  }
  MmioWrite32 (MOCK_FAKE_ADDRESS80, MOCK_FAKE_DATACDCDCDCD);
  MockCount = GetMockCount (MockStorage);
  for (Index = 0; Index < MockCount; Index++) {
    GetMock (MockStorage, Index, &Mock);
    if (Mock == NULL) {
      DelMocks
      Fail ("GetMock() returned an error");
    }
    if (Mock->Type != MockTypeRw) {
      DelMocks
      Fail ("New Mock is not Read/Write");
    }
    if (Mock->Size != MockSize8) {
      DelMocks
      Fail ("New Mock is not 8 bit");
    }
    if (Mock->Address != MOCK_FAKE_ADDRESS80 + Index) {
      DelMocks
      Fail ("New Mock is not at the correct address");
    }
    if (Mock->AddressSpace != MockSpaceMmio) {
      DelMocks
      Fail ("New Mock is not a MMIO Mock");
    }
    if (Mock->TypeData.Data[0] != MOCK_FAKE_DATACD) {
      DelMocks
      Fail ("New Mock does not have the right value");
    }
  }
  DelMocks
  AssertTrue (MockCount == 4, "Mock I/O write to non-existant address didn't create a new Mocks");
}

//
// Test a 32 bit composite write across two 16 bit CR Mocks
//
UNIT_TEST (Two16Mock32MmioWrite)
{
  MOCK                    Mock;
  MOCK                    *Mock0;
  MOCK                    *Mock1;
  VOID                    *MockStorage;
  UINTN                   MockCount;
  EFI_STATUS              Status;
  UINT16                  Buffer16;
  BOOLEAN                 TestPass;

  Mock.Type         = MockTypeRw;
  Mock.Size         = MockSize16;
  Mock.Address      = MOCK_FAKE_ADDRESS80;
  Mock.AddressSpace = MockSpaceMmio;
  Status            = GetMockStorage (&MockStorage);
  if (EFI_ERROR (Status) || MockStorage == NULL) {
    DEBUG ((DEBUG_INFO, "GetMockStorage() returned %r\n", Status));
    Fail ("GetMockStorage() returned an error");
  }
  Buffer16 = MOCK_FAKE_DATA7777;
  CopyMem ((VOID *) &(Mock.TypeData.Data[0]), (VOID *) &Buffer16, sizeof (UINT16));
  Status = AddNewMock (&Mock);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "AddNewMock() returned %r\n", Status));
    Fail ("AddNewMock() returned an error");
  }
  Mock.Type     = MockTypeRw;
  Mock.Address  = MOCK_FAKE_ADDRESS82;
  Buffer16      = MOCK_FAKE_DATA8888;
  CopyMem ((VOID *) &(Mock.TypeData.Data[0]), (VOID *) &Buffer16, sizeof (UINT16));
  Status = AddNewMock (&Mock);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "AddNewMock() returned %r\n", Status));
    Fail ("AddNewMock() returned an error");
  }
  GetMock (MockStorage, 0, &Mock0);
  GetMock (MockStorage, 1, &Mock1);
  if (Mock0 == NULL || Mock1 == NULL) {
    DelMocks
    Fail ("GetMock() returned an error");
  }
  MmioWrite32 (MOCK_FAKE_ADDRESS80, MOCK_FAKE_DATAAAAABBBB);
  MockCount = GetMockCount (MockStorage);
  if (MockCount != 2) {
    DelMocks
    Fail ("Mock count is incorrect");
  }
  TestPass = FALSE;
  if ( *((UINT16 *) &(Mock0->TypeData.Data[0])) == MOCK_FAKE_DATABBBB) {
    if ( *((UINT16 *) &(Mock1->TypeData.Data[0])) == MOCK_FAKE_DATAAAAA) {
      TestPass = TRUE;
    }
  }
  DelMocks
}

//
// Test writting 2 bytes in the middle of a 64 bit Mock
//
UNIT_TEST (Partial64Mock16MmioWrite)
{
  MOCK                    Mock;
  UINT64                  Buffer64;
  MOCK                    *Mock0;
  VOID                    *MockStorage;
  UINTN                   MockCount;
  EFI_STATUS              Status;

  Mock.Type         = MockTypeRw;
  Mock.Size         = MockSize64;
  Mock.Address      = MOCK_FAKE_ADDRESS80;
  Mock.AddressSpace = MockSpaceMmio;
  Status            = GetMockStorage (&MockStorage);
  if (EFI_ERROR (Status) || MockStorage == NULL) {
    DEBUG ((DEBUG_INFO, "GetMockStorage() returned %r\n", Status));
    Fail ("GetMockStorage() returned an error");
  }
  Buffer64 = MOCK_FAKE_DATA1122334455667788;
  CopyMem ((VOID *) &(Mock.TypeData.Data[0]), (VOID *) &Buffer64, sizeof (UINT64));
  Status = AddNewMock (&Mock);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "AddNewMock() returned %r\n", Status));
    Fail ("AddNewMock() returned an error");
  }
  GetMock (MockStorage, 0, &Mock0);
  if (Mock0 == NULL) {
    DelMocks
    Fail ("GetMock() returned an error");
  }
  MmioWrite16 (MOCK_FAKE_ADDRESS84, MOCK_FAKE_DATAAAAA);
  MockCount = GetMockCount (MockStorage);
  if (MockCount != 1) {
    DelMocks
    Fail ("Mock count is incorrect");
  }
  CopyMem ((VOID *) &Buffer64, (VOID *) &(Mock0->TypeData.Data[0]), sizeof (UINT64));
  DelMocks
  if (Buffer64 != MOCK_FAKE_DATA1122AAAA55667788) {
    DEBUG ((DEBUG_INFO, "Mock I/O wrote bad value: 0x%x\n", (UINTN)Buffer64));
  }
  AssertTrue ((Buffer64 == MOCK_FAKE_DATA1122AAAA55667788), "Mock I/O wrote bad data");
}

//
// Complex MMIO Write
//
UNIT_TEST (ComplexMmioWrite)
{
  MOCK                    Mock;
  UINT64                  Buffer64;
  EFI_STATUS              Status;
  MOCK                    *Mock0;
  MOCK                    *Mock1;
  MOCK                    *Mock2;
  MOCK                    *Mock3;
  VOID                    *MockStorage;
  UINTN                   MockCount;
  UINT32                  Buffer32;
  UINT16                  Buffer16;
  UINT8                   Buffer8;

  Mock.Type         = MockTypeRw;
  Mock.Size         = MockSize32;
  Mock.Address      = MOCK_FAKE_ADDRESS80;
  Mock.AddressSpace = MockSpaceMmio;
  Status            = GetMockStorage (&MockStorage);
  if (EFI_ERROR (Status) || MockStorage == NULL) {
    DEBUG ((DEBUG_INFO, "GetMockStorage() returned %r\n", Status));
    Fail ("GetMockStorage() returned an error");
  }
  Buffer32 = MOCK_FAKE_DATAAAAAAAAA;
  CopyMem ((VOID *) &(Mock.TypeData.Data[0]), (VOID *) &Buffer32, sizeof (UINT32));
  Status = AddNewMock (&Mock);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "AddNewMock() returned %r\n", Status));
    Fail ("AddNewMock() returned an error");
  }
  Mock.Type     = MockTypeRo;
  Mock.Size     = MockSize8;
  Mock.Address  = MOCK_FAKE_ADDRESS85;
  Buffer8       = MOCK_FAKE_DATA77;
  CopyMem ((VOID *) &(Mock.TypeData.Data[0]), (VOID *) &Buffer8, sizeof (UINT8));
  Status = AddNewMock (&Mock);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "AddNewMock() returned %r\n", Status));
    Fail ("AddNewMock() returned an error");
  }
  Mock.Type     = MockTypeRw;
  Mock.Size     = MockSize16;
  Mock.Address  = MOCK_FAKE_ADDRESS86;
  Buffer16      = MOCK_FAKE_DATACCCC;
  CopyMem ((VOID *) &(Mock.TypeData.Data[0]), (VOID *) &Buffer16, sizeof (UINT16));
  Status = AddNewMock (&Mock);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "AddNewMock() returned %r\n", Status));
    Fail ("AddNewMock() returned an error");
  }
  MmioWrite64 (MOCK_FAKE_ADDRESS80, MOCK_FAKE_DATA5544663388119922);
  MockCount = GetMockCount (MockStorage);
  if (MockCount != 4) {
    DelMocks
    Fail ("Mock count is incorrect");
  }
  GetMock (MockStorage, 0, &Mock0);
  GetMock (MockStorage, 1, &Mock1);
  GetMock (MockStorage, 2, &Mock2);
  GetMock (MockStorage, 3, &Mock3);
  if (Mock0 == NULL || Mock1 == NULL || Mock2 == NULL || Mock3 == NULL) {
    DelMocks
    Fail ("GetMock() returned an error");
  }
  if (*((UINT32 *) &(Mock0->TypeData.Data[0])) != MOCK_FAKE_DATA88119922) {
    DEBUG ((DEBUG_INFO, "Mock I/O wrote bad value: 0x%x\n", (UINTN) *((UINT32 *) &(Mock0->TypeData.Data[0]))));
    DelMocks
    Fail ("Mock I/O wrote bad data");
  }
  if (Mock1->TypeData.Data[0] != MOCK_FAKE_DATA77) {
    DEBUG ((DEBUG_INFO, "Mock I/O wrote bad value: 0x%x\n", (UINTN) Mock1->TypeData.Data[0]));
    DelMocks
    Fail ("Mock I/O wrote bad data");
  }
  if (*((UINT16 *) &(Mock2->TypeData.Data[0])) != MOCK_FAKE_DATA5544) {
    DEBUG ((DEBUG_INFO, "Mock I/O wrote bad value: 0x%x\n", (UINTN) *((UINT16 *) &(Mock2->TypeData.Data[0]))));
    DelMocks
    Fail ("Mock I/O wrote bad data");
  }
  if (Mock3->Type != MockTypeRw) {
    DelMocks
    Fail ("New Mock is not Read/Write");
  }
  if (Mock3->Size != MockSize8) {
    DelMocks
    Fail ("New Mock is not 8 bit");
  }
  if (Mock3->Address != MOCK_FAKE_ADDRESS84) {
    DelMocks
    Fail ("New Mock is not at the correct address");
  }
  if (Mock3->AddressSpace != MockSpaceMmio) {
    DelMocks
    Fail ("New Mock is not a MMIO Mock");
  }
  if (Mock3->TypeData.Data[0] != MOCK_FAKE_DATA33) {
    DelMocks
    Fail ("New Mock does not have the right value");
  }
  Buffer64 = MmioRead64 (MOCK_FAKE_ADDRESS80);
  if (Buffer64 != MOCK_FAKE_DATA5544773388119922) {
    Buffer32 = (UINT32) ((Buffer64 >> 32) & MOCK_EMPTY_CR32);
    DEBUG ((
      DEBUG_INFO,
      "Mock I/O wrote bad value: 0x%x%x\n",
      (UINTN) Buffer32,
      (UINTN) (Buffer64 & MOCK_EMPTY_CR32)
      ));
  }
  DelMocks
  AssertTrue (Buffer64 == MOCK_FAKE_DATA5544773388119922, "Mock I/O read returned bad data");
}

//
// Test get 8 bit data helper function
//
UNIT_TEST (TestGetMockData8)
{
  MOCK                    Mock;

  Mock.Type             = MockTypeRw;
  Mock.Size             = MockSize8;
  Mock.Address          = MOCK_FAKE_ADDRESS80;
  Mock.AddressSpace     = MockSpaceMmio;
  Mock.TypeData.Data[0] = MOCK_FAKE_DATA77;
  AssertTrue (GetMockData8 (&Mock) == MOCK_FAKE_DATA77, "GetMockData8() returned bad data");
}

//
// Test get 16 bit data helper function
//
UNIT_TEST (TestGetMockData16)
{
  MOCK                    Mock;

  Mock.Type                               = MockTypeRw;
  Mock.Size                               = MockSize16;
  Mock.Address                            = MOCK_FAKE_ADDRESS80;
  Mock.AddressSpace                       = MockSpaceMmio;
  *((UINT16 *) &(Mock.TypeData.Data[0]))  = MOCK_FAKE_DATA7777;
  AssertTrue (GetMockData16 (&Mock) == MOCK_FAKE_DATA7777, "GetMockData16() returned bad data");
}

//
// Test get 32 bit data helper function
//
UNIT_TEST (TestGetMockData32)
{
  MOCK                    Mock;

  Mock.Type                               = MockTypeRw;
  Mock.Size                               = MockSize32;
  Mock.Address                            = MOCK_FAKE_ADDRESS80;
  Mock.AddressSpace                       = MockSpaceMmio;
  *((UINT32 *) &(Mock.TypeData.Data[0]))  = MOCK_FAKE_DATA77777777;
  AssertTrue (GetMockData32 (&Mock) == MOCK_FAKE_DATA77777777, "GetMockData32() returned bad data");
}

//
// Test get 64 bit data helper function
//
UNIT_TEST (TestGetMockData64)
{
  MOCK                    Mock;

  Mock.Type                               = MockTypeRw;
  Mock.Size                               = MockSize64;
  Mock.Address                            = MOCK_FAKE_ADDRESS80;
  Mock.AddressSpace                       = MockSpaceMmio;
  *((UINT64 *) &(Mock.TypeData.Data[0]))  = MOCK_FAKE_DATA7777777788888888;
  AssertTrue (GetMockData64 (&Mock) == MOCK_FAKE_DATA7777777788888888, "GetMockData64() returned bad data");
}

//
// Test set 8 bit data helper function
//
UNIT_TEST (TestSetMockData8)
{
  MOCK                    Mock;

  Mock.Type         = MockTypeRw;
  Mock.Size         = MockSize8;
  Mock.Address      = MOCK_FAKE_ADDRESS80;
  Mock.AddressSpace = MockSpaceMmio;
  SetMockData8 (&Mock, MOCK_FAKE_DATA77);
  AssertTrue (Mock.TypeData.Data[0] == MOCK_FAKE_DATA77, "GetMockData8() returned bad data");
}

//
// Test set 16 bit data helper function
//
UNIT_TEST (TestSetMockData16)
{
  MOCK                    Mock;

  Mock.Type         = MockTypeRw;
  Mock.Size         = MockSize16;
  Mock.Address      = MOCK_FAKE_ADDRESS80;
  Mock.AddressSpace = MockSpaceMmio;
  SetMockData16 (&Mock, MOCK_FAKE_DATA7777);
  AssertTrue (*((UINT16 *) &(Mock.TypeData.Data[0])) == MOCK_FAKE_DATA7777, "GetMockData16() returned bad data");
}

//
// Test set 32 bit data helper function
//
UNIT_TEST (TestSetMockData32)
{
  MOCK                    Mock;

  Mock.Type         = MockTypeRw;
  Mock.Size         = MockSize32;
  Mock.Address      = MOCK_FAKE_ADDRESS80;
  Mock.AddressSpace = MockSpaceMmio;
  SetMockData32 (&Mock, MOCK_FAKE_DATA77777777);
  AssertTrue (*((UINT32 *) &(Mock.TypeData.Data[0])) == MOCK_FAKE_DATA77777777, "GetMockData32() returned bad data");
}

//
// Test set 64 bit data helper function
//
UNIT_TEST (TestSetMockData64)
{
  MOCK                    Mock;

  Mock.Type         = MockTypeRw;
  Mock.Size         = MockSize64;
  Mock.Address      = MOCK_FAKE_ADDRESS80;
  Mock.AddressSpace = MockSpaceMmio;
  SetMockData64 (&Mock, MOCK_FAKE_DATA7777777788888888);
  AssertTrue (*((UINT64 *) &(Mock.TypeData.Data[0])) == MOCK_FAKE_DATA7777777788888888, "GetMockData64() returned bad data");
}

#undef DelMocks
