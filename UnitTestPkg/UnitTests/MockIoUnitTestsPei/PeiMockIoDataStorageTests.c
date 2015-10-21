/** @file
  Mock I/O Data Storage Library PEI Unit Tests

@copyright
  Copyright (c) 2015, Intel Corporation. All rights reserved
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/UnitTestLib.h>

#include <LibraryPrivate/MockIoDataStorageLib.h>
#include <../LibraryPrivate/PeiMockIoDataStorageLib/PeiMockIoDataStorageLib.h>

#define MOCKSTORAGE_INVALID_COUNT     213654684
#define MOCKSTORAGE_FAKE_COUNT        10
#define MOCKSTORAGE_FAKE_OFFSET       7

///
/// GetMockCount() Unit Tests
///

//
// Test that GetMockCount() returns the TotalMockCount
//
UNIT_TEST (GetMockCountReturnsMockCount)
{
  PEI_MOCK_STORAGE_PPI    MockStoragePpi;

  ZeroMem ((VOID *) &MockStoragePpi, sizeof (PEI_MOCK_STORAGE_PPI));
  MockStoragePpi.TotalMockCapacity  = PEI_MOCK_STORAGE_MOCKS_PER_NODE;
  MockStoragePpi.TotalMockCount     = MOCKSTORAGE_INVALID_COUNT;
  AssertTrue (
    (GetMockCount ((VOID *) &MockStoragePpi) == (UINTN) MOCKSTORAGE_INVALID_COUNT),
    "GetMockCount() doesn't return TotalMockCount"
    );
}

//
// Test that GetMockCount() returns 0 when given a NULL pointer
//
UNIT_TEST (GetMockCountReturnsZeroOnNull)
{
  AssertTrue (
    (GetMockCount (NULL) == (UINTN) 0),
    "GetMockCount() doesn't return zero when given a NULL pointer"
    );
}

///
/// GetMock() Unit Tests
///

//
// Test that GetMock() returns NULL when given a NULL MockStorage pointer
//
UNIT_TEST (GetMockReturnsNullOnNull)
{
  MOCK                    *Mock;

  GetMock (NULL, MOCKSTORAGE_INVALID_COUNT, &Mock);
  AssertTrue (Mock == NULL, "GetMock() didn't return NULL when given a NULL MockStorage");
}

//
// Test that GetMock() return NULL when given an index that is
// larger than the number of mocks currently stored
//
UNIT_TEST (GetMockReturnsNullOnBadIndex)
{
  PEI_MOCK_STORAGE_PPI    MockStoragePpi;
  MOCK                    *Mock;

  ZeroMem ((VOID *) &MockStoragePpi, sizeof (PEI_MOCK_STORAGE_PPI));
  MockStoragePpi.TotalMockCapacity = PEI_MOCK_STORAGE_MOCKS_PER_NODE;
  GetMock ((VOID *) &MockStoragePpi, MOCKSTORAGE_INVALID_COUNT, &Mock);
  AssertTrue (Mock == NULL, "GetMock() didn't return NULL when given an invalid index");
}

//
// Get a Mock within the head Mock storage node
//
UNIT_TEST (SimpleGetMock)
{
  PEI_MOCK_STORAGE_PPI    MockStoragePpi;
  MOCK                    *Mock;

  ZeroMem ((VOID *) &MockStoragePpi, sizeof (PEI_MOCK_STORAGE_PPI));
  MockStoragePpi.TotalMockCapacity  = PEI_MOCK_STORAGE_MOCKS_PER_NODE;
  MockStoragePpi.TotalMockCount     = MOCKSTORAGE_FAKE_COUNT;
  MockStoragePpi.Head.MockCount     = MOCKSTORAGE_FAKE_COUNT;
  MockStoragePpi.Head.Mocks[MOCKSTORAGE_FAKE_OFFSET].Type = MockTypeRo;

  GetMock ((VOID *) &MockStoragePpi, MOCKSTORAGE_FAKE_OFFSET, &Mock);
  AssertTrue (
    Mock == &(MockStoragePpi.Head.Mocks[MOCKSTORAGE_FAKE_OFFSET]),
    "GetMock() didn't return the right Mock"
    );
  AssertTrue (
    Mock->Type == MockTypeRo,
    "GetMock() didn't get the requested Mock"
    );
}

//
// Get a Mock in the middle of a large linked list of Mocks
//
UNIT_TEST (ComplexGetMock)
{
  PEI_MOCK_STORAGE_PPI    MockStoragePpi;
  PEI_MOCK_STORAGE_NODE   MockStorageNode1;
  PEI_MOCK_STORAGE_NODE   MockStorageNode2;
  PEI_MOCK_STORAGE_NODE   MockStorageNode3;
  MOCK                    *Mock;
  UINTN                   Index;

  Index = (PEI_MOCK_STORAGE_MOCKS_PER_NODE * 3) + (PEI_MOCK_STORAGE_MOCKS_PER_NODE / 4);
  ZeroMem ((VOID *) &MockStoragePpi, sizeof (PEI_MOCK_STORAGE_PPI));
  ZeroMem ((VOID *) &MockStorageNode1, sizeof (PEI_MOCK_STORAGE_NODE));
  ZeroMem ((VOID *) &MockStorageNode2, sizeof (PEI_MOCK_STORAGE_NODE));
  ZeroMem ((VOID *) &MockStorageNode3, sizeof (PEI_MOCK_STORAGE_NODE));
  MockStoragePpi.TotalMockCapacity  = PEI_MOCK_STORAGE_MOCKS_PER_NODE * 4;
  MockStoragePpi.TotalMockCount     = PEI_MOCK_STORAGE_MOCKS_PER_NODE * 3 + (PEI_MOCK_STORAGE_MOCKS_PER_NODE / 2);
  MockStoragePpi.Head.MockCount     = PEI_MOCK_STORAGE_MOCKS_PER_NODE;
  MockStorageNode1.MockCount        = PEI_MOCK_STORAGE_MOCKS_PER_NODE;
  MockStorageNode2.MockCount        = PEI_MOCK_STORAGE_MOCKS_PER_NODE;
  MockStorageNode3.MockCount        = PEI_MOCK_STORAGE_MOCKS_PER_NODE / 2;
  MockStoragePpi.Head.Next          = &MockStorageNode1;
  MockStorageNode1.Next             = &MockStorageNode2;
  MockStorageNode2.Next             = &MockStorageNode3;
  MockStorageNode3.Mocks[PEI_MOCK_STORAGE_MOCKS_PER_NODE / 4].Type = MockTypeRo;

  GetMock ((VOID *) &MockStoragePpi, Index, &Mock);
  AssertTrue (
    Mock == &(MockStorageNode3.Mocks[PEI_MOCK_STORAGE_MOCKS_PER_NODE / 4]),
    "GetMock() didn't return the right Mock"
    );
  AssertTrue (
    Mock->Type == MockTypeRo,
    "GetMock() didn't retain the Mock's data"
    );
}

///
/// ClearMockStorage() Unit Tests
///

//
// Test that ClearMockStorage return EFI_INVALID_PARAMETER when given NULL
//
UNIT_TEST (ClearMockStorageReturnsError)
{
  EFI_STATUS              Status;

  Status = ClearMockStorage (NULL);

  AssertTrue (
    Status == EFI_INVALID_PARAMETER,
    "ClearMockStorage() didn't return EFI_INVALID_PARAMETER on NULL"
    );
}

//
// Clear Mock storage when only the head node exists
//
UNIT_TEST (SimpleClearMockStorage)
{
  EFI_STATUS              Status;
  PEI_MOCK_STORAGE_PPI    MockStoragePpi;

  ZeroMem ((VOID *) &MockStoragePpi, sizeof (PEI_MOCK_STORAGE_PPI));
  MockStoragePpi.TotalMockCapacity  = PEI_MOCK_STORAGE_MOCKS_PER_NODE;
  MockStoragePpi.TotalMockCount     = 1;
  MockStoragePpi.Head.MockCount     = 1;
  MockStoragePpi.Head.Mocks[MOCKSTORAGE_FAKE_OFFSET].Type = MockTypeRo;
  Status = ClearMockStorage ((VOID *) &MockStoragePpi);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "ClearMockStorage() Error: %r\n", Status));
    Fail ("ClearMockStorage() returned error");
  }
  AssertTrue (
    MockStoragePpi.TotalMockCapacity == PEI_MOCK_STORAGE_MOCKS_PER_NODE,
    "TotalMockCapacity is incorrect"
    );
  AssertTrue (
    MockStoragePpi.TotalMockCount == 0,
    "TotalMockCount is not zero"
    );
  AssertTrue (
    MockStoragePpi.Head.MockCount == 0,
    "Head.MockCount is not zero"
    );
  AssertTrue (
    ((int)MockStoragePpi.Head.Mocks[MOCKSTORAGE_FAKE_OFFSET].Type) == 0,
    "Mock data was not cleared"
    );
}

//
// Clear Mock storage on a large linked list of Mocks
//
UNIT_TEST (ComplexClearMockStorage)
{
  PEI_MOCK_STORAGE_PPI    MockStoragePpi;
  PEI_MOCK_STORAGE_NODE   MockStorageNode1;
  PEI_MOCK_STORAGE_NODE   MockStorageNode2;
  PEI_MOCK_STORAGE_NODE   MockStorageNode3;
  EFI_STATUS              Status;

  ZeroMem ((VOID *) &MockStoragePpi, sizeof (PEI_MOCK_STORAGE_PPI));
  ZeroMem ((VOID *) &MockStorageNode1, sizeof (PEI_MOCK_STORAGE_NODE));
  ZeroMem ((VOID *) &MockStorageNode2, sizeof (PEI_MOCK_STORAGE_NODE));
  ZeroMem ((VOID *) &MockStorageNode3, sizeof (PEI_MOCK_STORAGE_NODE));
  MockStoragePpi.TotalMockCapacity  = PEI_MOCK_STORAGE_MOCKS_PER_NODE * 4;
  MockStoragePpi.TotalMockCount     = PEI_MOCK_STORAGE_MOCKS_PER_NODE * 3 + (PEI_MOCK_STORAGE_MOCKS_PER_NODE / 2);
  MockStoragePpi.Head.MockCount     = PEI_MOCK_STORAGE_MOCKS_PER_NODE;
  MockStorageNode1.MockCount        = PEI_MOCK_STORAGE_MOCKS_PER_NODE;
  MockStorageNode2.MockCount        = PEI_MOCK_STORAGE_MOCKS_PER_NODE;
  MockStorageNode3.MockCount        = PEI_MOCK_STORAGE_MOCKS_PER_NODE / 2;
  MockStoragePpi.Head.Next          = &MockStorageNode1;
  MockStorageNode1.Next             = &MockStorageNode2;
  MockStorageNode2.Next             = &MockStorageNode3;
  MockStorageNode3.Mocks[PEI_MOCK_STORAGE_MOCKS_PER_NODE / 4].Type = MockTypeRo;

  Status = ClearMockStorage ((VOID *) &MockStoragePpi);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "ClearMockStorage() Error: %r\n", Status));
    Fail ("ClearMockStorage() returned error");
  }
  AssertTrue (
    MockStoragePpi.TotalMockCapacity == PEI_MOCK_STORAGE_MOCKS_PER_NODE * 4,
    "TotalMockCapacity is incorrect"
    );
  AssertTrue (
    MockStoragePpi.TotalMockCount == 0,
    "TotalMockCount is not zero"
    );
  AssertTrue (
    MockStoragePpi.Head.MockCount == 0,
    "Head.MockCount is not zero"
    );
  AssertTrue (
    MockStorageNode1.MockCount == 0,
    "MockStorageNode1.MockCount is not zero"
    );
  AssertTrue (
    MockStorageNode2.MockCount == 0,
    "MockStorageNode2.MockCount is not zero"
    );
  AssertTrue (
    MockStorageNode3.MockCount == 0,
    "MockStorageNode3.MockCount is not zero"
    );
  AssertTrue (
    MockStoragePpi.Head.Next == &MockStorageNode1,
    "MockStoragePpi.Head.Next is incorrect"
    );
  AssertTrue (
    MockStorageNode1.Next == &MockStorageNode2,
    "MockStorageNode1.Next is incorrect"
    );
  AssertTrue (
    MockStorageNode2.Next == &MockStorageNode3,
    "MockStorageNode2.Next is incorrect"
    );
  AssertTrue (
    ((int)MockStorageNode3.Mocks[PEI_MOCK_STORAGE_MOCKS_PER_NODE / 4].Type) == 0,
    "Mock data was not cleared"
    );
}

//
// @note: The tests for GetMockStorage() and AddMock() are operating on a global
// singleton that we cannot instrument since there is no way to uninstall a PPI.
// The side effect of this is that failures in these tests do not nessesarilly
// indicate that the bug is in the function being tested.  This is undesirable
// behavior and one should try to avoid this when writting unit tests when possible.
//

///
/// GetMockStorage() Unit Tests
///

//
// Test Mock Storage Creation
//
UNIT_TEST (TestCreateMockStoragePpi)
{
  EFI_STATUS              Status;
  PEI_MOCK_STORAGE_PPI    *MockStoragePpi;
  VOID                    *MockStorage;

  Status = PeiServicesLocatePpi (
            &gPeiMockIoDataStoragePpi,
            0,
            NULL,
            (VOID **) &MockStoragePpi
            );
  if (EFI_ERROR (Status) || (MockStoragePpi == NULL)) {
    //
    // PPI doesn't exist yet, calling GetMockStorage() should create it
    //
    DEBUG ((DEBUG_INFO, "PEI_MOCK_STORAGE_PPI does not exist yet\n"));
    Status = GetMockStorage (&MockStorage);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_INFO, "GetMockStorage() returned %r\n", Status));
      Fail ("GetMockStorage() returned an error");
    }
    Status = PeiServicesLocatePpi (
            &gPeiMockIoDataStoragePpi,
            0,
            NULL,
            (VOID **) &MockStoragePpi
            );
    if (EFI_ERROR (Status) || (MockStoragePpi == NULL)) {
      Fail ("GetMockStorage() didn't create the MockStoragePpi");
    }
  } else {
    //
    // PPI already exists, calling GetMockStorage() should return the same PPI
    //
    DEBUG ((DEBUG_INFO, "PEI_MOCK_STORAGE_PPI already exists\n"));
    Status = GetMockStorage (&MockStorage);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_INFO, "GetMockStorage() returned %r\n", Status));
      Fail ("GetMockStorage() returned an error");
    }
  }
  AssertTrue (
    (((VOID *)MockStoragePpi) == MockStorage),
    "GetMockStorage() didn't return the MockStoragePpi"
    );
}

//
// Validate that the MockStorage is a singleton
//
UNIT_TEST (MockStoragePpiSingleton)
{
  EFI_STATUS              Status;
  VOID                    *MockStorage1;
  VOID                    *MockStorage2;

  Status = GetMockStorage (&MockStorage1);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "GetMockStorage() returned %r\n", Status));
    Fail ("GetMockStorage() returned an error");
  }
  Status = GetMockStorage (&MockStorage2);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "GetMockStorage() returned %r\n", Status));
    Fail ("GetMockStorage() returned an error");
  }
  AssertTrue (MockStorage1 == MockStorage2, "MockStorage is not a singleton instance");
}

///
/// AddMock() Unit Tests
///

//
// Check that AddMock() returns EFI_INVALID_PARAMETER if a NULL MockStorage is given
//
UNIT_TEST (AddMockReturnsErrorOnNullStore)
{
  MOCK                    Mock;
  EFI_STATUS              Status;

  Status = AddMock (NULL, &Mock);
  AssertTrue (
    Status == EFI_INVALID_PARAMETER,
    "AddMock() didn't return EFI_INVALID_PARAMETER on NULL Store"
    );
}

//
// Check that AddMock() returns EFI_INVALID_PARAMETER if a NULL Mock is given
//
UNIT_TEST (AddMockReturnsErrorOnNullMock)
{
  EFI_STATUS              Status;
  VOID                    *MockStorage;
  UINTN                   MockCount;

  Status = GetMockStorage (&MockStorage);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "GetMockStorage() returned %r\n", Status));
    Fail ("GetMockStorage() returned an error");
  }
  AssertTrue (MockStorage != NULL, "MockStorage is NULL");
  MockCount = GetMockCount (MockStorage);
  Status    = AddMock (MockStorage, NULL);
  AssertTrue (
    Status == EFI_INVALID_PARAMETER,
    "AddMock() didn't return EFI_INVALID_PARAMETER on NULL Mock"
    );
  AssertTrue (
    MockCount == GetMockCount (MockStorage),
    "MockCount is not the same as before calling AddMock()"
    );
}

//
// Test AddMock() in the simple case of an empty Mock Storage
//
UNIT_TEST (SimpleAddMock)
{
  MOCK                    Mock;
  EFI_STATUS              Status;
  VOID                    *MockStorage;
  MOCK                    *pMock;
  UINTN                   MockCount;

  Status = GetMockStorage (&MockStorage);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "GetMockStorage() returned %r\n", Status));
    Fail ("GetMockStorage() returned an error");
  }
  AssertTrue (MockStorage != NULL, "MockStorage is NULL");
  Status = ClearMockStorage (MockStorage);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "ClearMockStorage() returned %r\n", Status));
    Fail ("ClearMockStorage() returned an error");
  }
  MockCount = GetMockCount (MockStorage);
  AssertTrue (
    MockCount == 0,
    "MockCount is non-zero after calling ClearMockStorage()"
    );
  ZeroMem ((VOID *) &Mock, sizeof (MOCK));
  Mock.Type = MockTypeRo;
  Status = AddMock (MockStorage, &Mock);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "AddMock() returned %r\n", Status));
    Fail ("AddMock() returned an error");
  }
  AssertTrue (
    GetMockCount (MockStorage) == (MockCount + 1),
    "AddMock() didn't increment the Mock count"
    );
  GetMock (MockStorage, 0, &pMock);
  AssertTrue (
    pMock != NULL,
    "Can't get Mock after adding it"
    );
  AssertTrue (
    pMock->Type == MockTypeRo,
    "AddMock() didn't copy the mock correctly"
    );
  Status = ClearMockStorage (MockStorage);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "ClearMockStorage() returned %r\n", Status));
    Fail ("ClearMockStorage() returned an error");
  }
}

//
// Test AddMock() with allocation of several extra Mock nodes
//
UNIT_TEST (ComplexAddMock)
{
  MOCK                    Mock;
  EFI_STATUS              Status;
  PEI_MOCK_STORAGE_PPI    *MockStoragePpi;
  VOID                    *MockStorage;
  UINTN                   MockCount;
  UINTN                   Index;

  Status = GetMockStorage (&MockStorage);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "GetMockStorage() returned %r\n", Status));
    Fail ("GetMockStorage() returned an error");
  }
  AssertTrue (MockStorage != NULL, "MockStorage is NULL");
  Status = ClearMockStorage (MockStorage);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "ClearMockStorage() returned %r\n", Status));
    Fail ("ClearMockStorage() returned an error");
  }
  MockCount = GetMockCount (MockStorage);
  AssertTrue (
    MockCount == 0,
    "MockCount is non-zero after calling ClearMockStorage()"
    );
  MockStoragePpi = (PEI_MOCK_STORAGE_PPI *) MockStorage;
  ZeroMem ((VOID *) &Mock, sizeof (MOCK));
  Mock.Type = MockTypeRo;
  for (Index = 0; Index <= (PEI_MOCK_STORAGE_MOCKS_PER_NODE * 3); Index++) {
    Status = AddMock (MockStorage, &Mock);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_INFO, "AddMock() returned %r\n", Status));
      Fail ("AddMock() returned an error");
    }
  }
  AssertTrue (MockStoragePpi->Head.Next != NULL, "Head.Next is NULL");
  AssertTrue (
    MockStoragePpi->Head.Next->Next != NULL,
    "Head.Next->Next is NULL"
    );
  AssertTrue (
    MockStoragePpi->Head.Next->Next->Mocks[0].Type == MockTypeRo,
    "AddMock() didn't copy the mock correctly"
    );
  Status = ClearMockStorage (MockStorage);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "ClearMockStorage() returned %r\n", Status));
    Fail ("ClearMockStorage() returned an error");
  }
}

BEGIN_TEST_CASE (GetMockCountTestCase)
  &GetMockCountReturnsMockCount,
  &GetMockCountReturnsZeroOnNull
END_TEST_CASE (GetMockCountTestCase, NULL)

BEGIN_TEST_CASE (GetMockTestCase)
  &GetMockReturnsNullOnNull,
  &GetMockReturnsNullOnBadIndex,
  &SimpleGetMock,
  &ComplexGetMock
END_TEST_CASE (GetMockTestCase, NULL)

BEGIN_TEST_CASE (ClearMockStorageTestCase)
  &ClearMockStorageReturnsError,
  &SimpleClearMockStorage,
  &ComplexClearMockStorage
END_TEST_CASE (ClearMockStorageTestCase, NULL)

BEGIN_TEST_CASE (GetMockStorageTestCase)
  &TestCreateMockStoragePpi,
  &MockStoragePpiSingleton
END_TEST_CASE (GetMockStorageTestCase, NULL)

BEGIN_TEST_CASE (AddMockTestCase)
  &AddMockReturnsErrorOnNullStore,
  &AddMockReturnsErrorOnNullMock,
  &SimpleAddMock,
  &ComplexAddMock
END_TEST_CASE (AddMockTestCase, NULL)

BEGIN_TEST_SUITE (MockIoDataStoragePeiUnitTests)
  &GetMockCountTestCase,
  &GetMockTestCase,
  &ClearMockStorageTestCase,
  &GetMockStorageTestCase,
  &AddMockTestCase
END_TEST_SUITE (MockIoDataStoragePeiUnitTests)

#define NUM_DATA_STORAGE_SUITE_TESTS   (NUM_TESTS (GetMockCountTestCase)      + \
                                        NUM_TESTS (GetMockTestCase)           + \
                                        NUM_TESTS (ClearMockStorageTestCase)  + \
                                        NUM_TESTS (GetMockStorageTestCase)    + \
                                        NUM_TESTS (AddMockTestCase))

VOID
RunMockIoDataStoragePeiUnitTests (
  VOID
  )
{
  TEST_RESULT   TestResults[NUM_DATA_STORAGE_SUITE_TESTS];

  RunTestSuite (&MockIoDataStoragePeiUnitTests, &TestResults[0]);
}
