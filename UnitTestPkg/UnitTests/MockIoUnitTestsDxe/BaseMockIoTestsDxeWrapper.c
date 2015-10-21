/** @file
  DXE Wrapper for Base Mock I/O Unit Tests

@copyright
  Copyright (c) 2015, Intel Corporation. All rights reserved
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "MockIoUnitTestsDxe.h"

DECLARE_UNIT_TEST (GetMockSizeInBytes8Bit)
DECLARE_UNIT_TEST (GetMockSizeInBytes16Bit)
DECLARE_UNIT_TEST (GetMockSizeInBytes32Bit)
DECLARE_UNIT_TEST (GetMockSizeInBytes64Bit)
DECLARE_UNIT_TEST (GetMockSizeInBytesInvalid)
BEGIN_TEST_CASE (GetMockSizeTestCase)
  &GetMockSizeInBytes8Bit,
  &GetMockSizeInBytes16Bit,
  &GetMockSizeInBytes32Bit,
  &GetMockSizeInBytes64Bit,
  &GetMockSizeInBytesInvalid
END_TEST_CASE (GetMockSizeTestCase, NULL)

DECLARE_UNIT_TEST (ReadFromRwMock)
DECLARE_UNIT_TEST (ReadFromRoMock)
DECLARE_UNIT_TEST (ReadFromCallbackMock)
DECLARE_UNIT_TEST (ReadFromInvalidMockType)
BEGIN_TEST_CASE (ReadFromMockTestCase)
  &ReadFromRwMock,
  &ReadFromRoMock,
  &ReadFromCallbackMock,
  &ReadFromInvalidMockType
END_TEST_CASE (ReadFromMockTestCase, NULL)

DECLARE_UNIT_TEST (WriteToRwMock)
DECLARE_UNIT_TEST (WriteToRoMock)
DECLARE_UNIT_TEST (WriteToCallbackMock)
DECLARE_UNIT_TEST (WriteToInvalidMockType)
BEGIN_TEST_CASE (WriteToMockTestCase)
  &WriteToRwMock,
  &WriteToRoMock,
  &WriteToCallbackMock,
  &WriteToInvalidMockType
END_TEST_CASE (WriteToMockTestCase, NULL)

DECLARE_UNIT_TEST (SimpleIoRead)
DECLARE_UNIT_TEST (TruncatedIoRead)
DECLARE_UNIT_TEST (NoMockIoRead)
BEGIN_TEST_CASE (ReadIoTestCase)
  &SimpleIoRead,
  &TruncatedIoRead,
  &NoMockIoRead
END_TEST_CASE (ReadIoTestCase, NULL)

DECLARE_UNIT_TEST (SimpleMmioRead)
DECLARE_UNIT_TEST (TruncatedMmioRead)
DECLARE_UNIT_TEST (NoMockMmioRead)
DECLARE_UNIT_TEST (Two16Mock32MmioRead)
DECLARE_UNIT_TEST (Partial64Mock16MmioRead)
DECLARE_UNIT_TEST (UpperOverlap32MockMmioRead)
DECLARE_UNIT_TEST (LowerOverlap32MockMmioRead)
DECLARE_UNIT_TEST (ComplexMmioRead)
DECLARE_UNIT_TEST (Eight8BitMockMmioRead)
BEGIN_TEST_CASE (ReadMmioTestCase)
  &SimpleMmioRead,
  &TruncatedMmioRead,
  &NoMockMmioRead,
  &Two16Mock32MmioRead,
  &Partial64Mock16MmioRead,
  &UpperOverlap32MockMmioRead,
  &LowerOverlap32MockMmioRead,
  &ComplexMmioRead,
  &Eight8BitMockMmioRead
END_TEST_CASE (ReadMmioTestCase, NULL)

DECLARE_UNIT_TEST (SimpleIoWrite)
DECLARE_UNIT_TEST (IoWriteToRoMock)
DECLARE_UNIT_TEST (TruncatedIoWrite)
DECLARE_UNIT_TEST (NoMockIoWrite)
BEGIN_TEST_CASE (WriteIoTestCase)
  &SimpleIoWrite,
  &IoWriteToRoMock,
  &TruncatedIoWrite,
  &NoMockIoWrite
END_TEST_CASE (WriteIoTestCase, NULL)

DECLARE_UNIT_TEST (SimpleMmioWrite)
DECLARE_UNIT_TEST (MmioWriteToRoMock)
DECLARE_UNIT_TEST (TruncatedMmioWrite)
DECLARE_UNIT_TEST (NoMockMmioWrite)
DECLARE_UNIT_TEST (Two16Mock32MmioWrite)
DECLARE_UNIT_TEST (Partial64Mock16MmioWrite)
DECLARE_UNIT_TEST (ComplexMmioWrite)
BEGIN_TEST_CASE (WriteMmioTestCase)
  &SimpleMmioWrite,
  &MmioWriteToRoMock,
  &TruncatedMmioWrite,
  &NoMockMmioWrite,
  &Two16Mock32MmioWrite,
  &Partial64Mock16MmioWrite,
  &ComplexMmioWrite
END_TEST_CASE (WriteMmioTestCase, NULL)

DECLARE_UNIT_TEST (TestGetMockData8)
DECLARE_UNIT_TEST (TestGetMockData16)
DECLARE_UNIT_TEST (TestGetMockData32)
DECLARE_UNIT_TEST (TestGetMockData64)
BEGIN_TEST_CASE (TestGetMockData)
  &TestGetMockData8,
  &TestGetMockData16,
  &TestGetMockData32,
  &TestGetMockData64
END_TEST_CASE (TestGetMockData, NULL)

DECLARE_UNIT_TEST (TestSetMockData8)
DECLARE_UNIT_TEST (TestSetMockData16)
DECLARE_UNIT_TEST (TestSetMockData32)
DECLARE_UNIT_TEST (TestSetMockData64)
BEGIN_TEST_CASE (TestSetMockData)
  &TestSetMockData8,
  &TestSetMockData16,
  &TestSetMockData32,
  &TestSetMockData64
END_TEST_CASE (TestSetMockData, NULL)

BEGIN_TEST_SUITE (MockIoDxeUnitTests)
  &GetMockSizeTestCase,
  &ReadFromMockTestCase,
  &WriteToMockTestCase,
  &ReadIoTestCase,
  &ReadMmioTestCase,
  &WriteIoTestCase,
  &WriteMmioTestCase,
  &TestGetMockData,
  &TestSetMockData
END_TEST_SUITE (MockIoDxeUnitTests)

#define NUM_MOCK_IO_SUITE_TESTS   (NUM_TESTS (GetMockSizeTestCase)  + \
                                   NUM_TESTS (ReadFromMockTestCase) + \
                                   NUM_TESTS (WriteToMockTestCase)  + \
                                   NUM_TESTS (ReadIoTestCase)       + \
                                   NUM_TESTS (ReadMmioTestCase)     + \
                                   NUM_TESTS (WriteIoTestCase)      + \
                                   NUM_TESTS (WriteMmioTestCase)    + \
                                   NUM_TESTS (TestGetMockData)      + \
                                   NUM_TESTS (TestSetMockData))

VOID
RunMockIoDxeUnitTests (
  VOID
  )
{
  TEST_RESULT   TestResults[NUM_MOCK_IO_SUITE_TESTS];

  RunTestSuite (&MockIoDxeUnitTests, &TestResults[0]);
}
