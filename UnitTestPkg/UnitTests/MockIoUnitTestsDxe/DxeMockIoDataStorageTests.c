/** @file
  Mock I/O Data Storage Library DXE Unit Tests

@copyright
  Copyright (c) 2015, Intel Corporation. All rights reserved
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UnitTestLib.h>

#include <LibraryPrivate/MockIoDataStorageLib.h>
#include <../LibraryPrivate/DxeMockIoDataStorageLib/DxeMockIoDataStorageLib.h>

#define MOCKSTORAGE_INVALID_COUNT     213654684
#define MOCKSTORAGE_FAKE_COUNT        10
#define MOCKSTORAGE_FAKE_OFFSET       7

///
/// GetMockCount() Unit Tests
///

//
// Test that GetMockCount() returns the Mock Count
//
UNIT_TEST (GetMockCountReturnsMockCount)
{
  MOCK_STORAGE    MockStorage;

  ZeroMem ((VOID *) &MockStorage, sizeof (MOCK_STORAGE));
  MockStorage.Capacity  = 0;
  MockStorage.Count     = MOCKSTORAGE_INVALID_COUNT;
  AssertTrue (
    (GetMockCount ((VOID *) &MockStorage) == (UINTN) MOCKSTORAGE_INVALID_COUNT),
    "GetMockCount() doesn't return Count"
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
// Test that GetMock() returns NULL when given a MockStorage that does not have
// any Mock Storage actually allocated
//
UNIT_TEST (GetMockReturnsNullOnNullStorage)
{
  MOCK_STORAGE            MockStorage;
  MOCK                    *Mock;

  MockStorage.Mocks     = NULL;
  MockStorage.Count     = 0;
  MockStorage.Capacity  = 0;
  GetMock ((VOID *) &MockStorage, 0, &Mock);
  AssertTrue (Mock == NULL, "GetMock() didn't return NULL when given a MockStorage with NULL for Mocks");
}

//
// Test that GetMock() return NULL when given an index that is
// larger than the number of mocks currently stored
//
UNIT_TEST (GetMockReturnsNullOnBadIndex)
{
  MOCK                    Mocks[INITIAL_MOCK_STORAGE_MOCKS_CAPACITY];
  MOCK_STORAGE            MockStorage;
  MOCK                    *Mock;

  ZeroMem ((VOID *) &Mocks, sizeof (Mocks));
  MockStorage.Mocks     = &(Mocks[0]);
  MockStorage.Count     = 0;
  MockStorage.Capacity  = INITIAL_MOCK_STORAGE_MOCKS_CAPACITY;
  GetMock ((VOID *) &MockStorage, MOCKSTORAGE_INVALID_COUNT, &Mock);
  AssertTrue (Mock == NULL, "GetMock() didn't return NULL when given an invalid index");
}

UNIT_TEST (GetMockReturnsMock)
{
  MOCK                    Mocks[INITIAL_MOCK_STORAGE_MOCKS_CAPACITY];
  MOCK_STORAGE            MockStorage;
  MOCK                    *Mock;

  ZeroMem ((VOID *) &Mocks, sizeof (Mocks));
  MockStorage.Mocks     = &(Mocks[0]);
  MockStorage.Count     = MOCKSTORAGE_FAKE_COUNT;
  MockStorage.Capacity  = INITIAL_MOCK_STORAGE_MOCKS_CAPACITY;
  Mocks[MOCKSTORAGE_FAKE_OFFSET].Type         = MockTypeRo;
  GetMock ((VOID *) &MockStorage, MOCKSTORAGE_FAKE_OFFSET, &Mock);
  AssertTrue (
    Mock->Type == MockTypeRo,
    "GetMock() didn't get the requested Mock"
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
// Test that ClearMockStorage does not allocate memory when the MockStorage
// does not have any memory allocated to begin with
//
UNIT_TEST (ClearMockStorageDoesNotAllocateMemory)
{
  MOCK_STORAGE            MockStorage;
  EFI_STATUS              Status;

  MockStorage.Mocks     = NULL;
  MockStorage.Count     = 0;
  MockStorage.Capacity  = 0;

  Status = ClearMockStorage ((VOID *) &MockStorage);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "ClearMockStorage() returned %r\n", Status));
    Fail ("ClearMockStorage() returned an error");
  }
  AssertTrue (
    MockStorage.Mocks == NULL,
    "ClearMockStorage() allocated memory"
    );
  AssertTrue (
    MockStorage.Count == 0,
    "ClearMockStorage() returns a non-zero Mock count"
    );
  AssertTrue (
    MockStorage.Capacity == 0,
    "ClearMockStorage() returns a non-zero Mock capacity when the input Capacity was zero"
    );
}

//
// Test that ClearMockStorage retains allocated Mock Storage when the Capacity
// hasn't been increased over the original capacity
//
UNIT_TEST (ClearMockStorageInitialSize)
{
  MOCK_STORAGE            MockStorage;
  EFI_STATUS              Status;
  MOCK                    *Mocks;

  Mocks = (MOCK *) AllocateZeroPool (sizeof (MOCK) * INITIAL_MOCK_STORAGE_MOCKS_CAPACITY);
  if (Mocks == NULL) {
    Fail ("Memory allocation failed");
  }
  MockStorage.Count     = MOCKSTORAGE_FAKE_COUNT;
  MockStorage.Capacity  = INITIAL_MOCK_STORAGE_MOCKS_CAPACITY;
  MockStorage.Mocks     = Mocks;
  Mocks[MOCKSTORAGE_FAKE_OFFSET].Type         = MockTypeRo;

  Status = ClearMockStorage ((VOID *) &MockStorage);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "ClearMockStorage() returned %r\n", Status));
    Fail ("ClearMockStorage() returned an error");
  }
  AssertTrue (
    MockStorage.Mocks == Mocks,
    "ClearMockStorage() undesirably freed memory"
    );
  AssertTrue (
    MockStorage.Count == 0,
    "ClearMockStorage() returned a non-zero Mock count"
    );
  AssertTrue (
    MockStorage.Capacity == INITIAL_MOCK_STORAGE_MOCKS_CAPACITY,
    "ClearMockStorage() changed the Mock Capacity"
    );
  AssertTrue (
    ((int)Mocks[MOCKSTORAGE_FAKE_OFFSET].Type) == 0,
    "ClearMockStorage() didn't clear out old data"
    );
  FreePool ((VOID *) Mocks);
}

//
// Test that ClearMockStorage de-allocated memory when the capacity is greater
// than the initial capacity
//
UNIT_TEST (ClearMockStorageExpandedSize)
{
  MOCK_STORAGE            MockStorage;
  EFI_STATUS              Status;
  MOCK                    *Mocks;
  UINTN                   Capacity;

  Capacity  = (INITIAL_MOCK_STORAGE_MOCKS_CAPACITY * MOCK_STORAGE_CAPACITY_INCREASE_MULTIPLIER /
              MOCK_STORAGE_CAPACITY_INCREASE_DIVISOR);
  Mocks     = (MOCK *) AllocateZeroPool (sizeof (MOCK) * Capacity);
  if (Mocks == NULL) {
    Fail ("Memory allocation failed");
  }
  MockStorage.Count     = MOCKSTORAGE_FAKE_COUNT;
  MockStorage.Capacity  = Capacity;
  MockStorage.Mocks     = Mocks;

  Status = ClearMockStorage ((VOID *) &MockStorage);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "ClearMockStorage() returned %r\n", Status));
    Fail ("ClearMockStorage() returned an error");
  }
  AssertTrue (
    MockStorage.Mocks == NULL,
    "ClearMockStorage() did not free memory"
    );
  AssertTrue (
    MockStorage.Count == 0,
    "ClearMockStorage() returned a non-zero Mock count"
    );
  AssertTrue (
    MockStorage.Capacity == 0,
    "ClearMockStorage() returned a non-zero Mock Capacity"
    );
}

///
/// GetMockStorage() Unit Tests
///

//
// Test that GetMockStorage() returns the Mock Storage global variable
//
UNIT_TEST (GetMockStorageReturnsMockStorage)
{
  EFI_STATUS              Status;
  VOID                    *MockStorage;

  Status = GetMockStorage (&MockStorage);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "GetMockStorage() returned %r\n", Status));
    Fail ("GetMockStorage() returned an error");
  }
  AssertTrue (MockStorage == &gMockStorage, "GetMockStorage() didn't return the MockStorage global");
}

//
// Test that the MockStorage is a singleton
//
UNIT_TEST (MockStorageSingleton)
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
// Test that AddMock() returns EFI_INVALID_PARAMETER if a NULL MockStorage is given
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
// Test that AddMock() returns EFI_INVALID_PARAMETER if a NULL Mock is given
//
UNIT_TEST (AddMockReturnsErrorOnNullMock)
{
  MOCK_STORAGE            MockStorage;
  EFI_STATUS              Status;

  MockStorage.Mocks     = NULL;
  MockStorage.Count     = 0;
  MockStorage.Capacity  = 0;
  Status    = AddMock ((VOID *) &MockStorage, NULL);
  AssertTrue (
    Status == EFI_INVALID_PARAMETER,
    "AddMock() didn't return EFI_INVALID_PARAMETER on NULL Mock"
    );
  AssertTrue (
    MockStorage.Count == 0,
    "MockCount is not the same as before calling AddMock()"
    );
}

//
// Test that AddMock() adds a Mock and doesn't change Mock Storage if there
// is sufficient space for the new mock
//
UNIT_TEST (AddMockSimple)
{
  MOCK_STORAGE            MockStorage;
  MOCK                    Mock;
  MOCK                    *Mocks;
  EFI_STATUS              Status;

  Mocks = (MOCK *) AllocateZeroPool (sizeof (MOCK) * INITIAL_MOCK_STORAGE_MOCKS_CAPACITY);
  if (Mocks == NULL) {
    Fail ("Memory allocation failed");
  }
  MockStorage.Count     = 0;
  MockStorage.Capacity  = INITIAL_MOCK_STORAGE_MOCKS_CAPACITY;
  MockStorage.Mocks     = Mocks;
  ZeroMem ((VOID *) &Mock, sizeof (MOCK));
  Mock.Type             = MockTypeRo;
  Status                = AddMock ((VOID *) &MockStorage, &Mock);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "AddMock() returned %r\n", Status));
    Fail ("AddMock() returned an error");
  }
  AssertTrue (
    MockStorage.Mocks == Mocks,
    "AddMock() changed the Mock Storage pointer"
    );
  AssertTrue (
    MockStorage.Capacity == INITIAL_MOCK_STORAGE_MOCKS_CAPACITY,
    "AddMock() didn't set Capacity to initial capacity"
    );
  AssertTrue (
    MockStorage.Count == 1,
    "AddMock() didn't increment the Mock count"
    );
  AssertTrue (
    MockStorage.Mocks[0].Type == MockTypeRo,
    "AddMock() didn't copy over the Mock data"
    );
  FreePool ((VOID *) MockStorage.Mocks);
}

//
// Test that AddMock() allocates initial Mock Storage if it does not exist yet
//
UNIT_TEST (AddMockInitialAllocate)
{
  MOCK_STORAGE            MockStorage;
  MOCK                    Mock;
  EFI_STATUS              Status;

  MockStorage.Mocks     = NULL;
  MockStorage.Count     = 0;
  MockStorage.Capacity  = 0;
  ZeroMem ((VOID *) &Mock, sizeof (MOCK));
  Mock.Type             = MockTypeRo;
  Status                = AddMock ((VOID *) &MockStorage, &Mock);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "AddMock() returned %r\n", Status));
    Fail ("AddMock() returned an error");
  }
  AssertTrue (
    MockStorage.Mocks != NULL,
    "AddMock() didn't allocate initial Mock Storage"
    );
  AssertTrue (
    MockStorage.Capacity == INITIAL_MOCK_STORAGE_MOCKS_CAPACITY,
    "AddMock() didn't set Capacity to initial capacity"
    );
  AssertTrue (
    MockStorage.Count == 1,
    "AddMock() didn't increment the Mock count"
    );
  AssertTrue (
    MockStorage.Mocks[0].Type == MockTypeRo,
    "AddMock() didn't copy over the Mock data"
    );
  FreePool ((VOID *) MockStorage.Mocks);
}

//
// Test that AddMock() expands Mock Storage if there is insufficient space
//
UNIT_TEST (AddMockExpandSize)
{
  MOCK_STORAGE            MockStorage;
  MOCK                    Mock;
  MOCK                    *Mocks;
  EFI_STATUS              Status;
  UINTN                   Index;

  Mocks = (MOCK *) AllocateZeroPool (sizeof (MOCK) * INITIAL_MOCK_STORAGE_MOCKS_CAPACITY);
  if (Mocks == NULL) {
    Fail ("Memory allocation failed");
  }
  MockStorage.Mocks     = Mocks;
  MockStorage.Count     = 0;
  MockStorage.Capacity  = INITIAL_MOCK_STORAGE_MOCKS_CAPACITY;
  ZeroMem ((VOID *) &Mock, sizeof (MOCK));
  Mock.Type             = MockTypeRo;
  for (Index = 0; Index <= INITIAL_MOCK_STORAGE_MOCKS_CAPACITY; Index++) {
    Status = AddMock ((VOID *) &MockStorage, &Mock);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_INFO, "AddMock() returned %r\n", Status));
      Fail ("AddMock() returned an error");
    }
  }
  AssertTrue (
    MockStorage.Mocks != Mocks,
    "AddMock() didn't allocate expanded Mock Storage"
    );
  AssertTrue (
    MockStorage.Capacity > INITIAL_MOCK_STORAGE_MOCKS_CAPACITY,
    "AddMock() didn't set Capacity to expanded capacity"
    );
  AssertTrue (
    MockStorage.Count == INITIAL_MOCK_STORAGE_MOCKS_CAPACITY + 1,
    "AddMock() didn't increment the Mock count"
    );
  AssertTrue (
    MockStorage.Mocks[0].Type == MockTypeRo,
    "AddMock() didn't copy over the Mock[0] data"
    );
  AssertTrue (
    MockStorage.Mocks[INITIAL_MOCK_STORAGE_MOCKS_CAPACITY].Type == MockTypeRo,
    "AddMock() didn't copy over the Mock[n] data"
    );
  FreePool ((VOID *) MockStorage.Mocks);
}

BEGIN_TEST_CASE (GetMockCountTestCase)
  &GetMockCountReturnsMockCount,
  &GetMockCountReturnsZeroOnNull
END_TEST_CASE (GetMockCountTestCase, NULL)

BEGIN_TEST_CASE (GetMockTestCase)
  &GetMockReturnsNullOnNull,
  &GetMockReturnsNullOnNullStorage,
  &GetMockReturnsNullOnBadIndex,
  &GetMockReturnsMock
END_TEST_CASE (GetMockTestCase, NULL)

BEGIN_TEST_CASE (ClearMockStorageTestCase)
  &ClearMockStorageReturnsError,
  &ClearMockStorageDoesNotAllocateMemory,
  &ClearMockStorageInitialSize,
  &ClearMockStorageExpandedSize
END_TEST_CASE (ClearMockStorageTestCase, NULL)

BEGIN_TEST_CASE (GetMockStorageTestCase)
  &GetMockStorageReturnsMockStorage,
  &MockStorageSingleton
END_TEST_CASE (GetMockStorageTestCase, NULL)

BEGIN_TEST_CASE (AddMockTestCase)
  &AddMockReturnsErrorOnNullStore,
  &AddMockReturnsErrorOnNullMock,
  &AddMockSimple,
  &AddMockInitialAllocate,
  &AddMockExpandSize
END_TEST_CASE (AddMockTestCase, NULL)

BEGIN_TEST_SUITE (MockIoDxeDataStorageUnitTests)
  &GetMockCountTestCase,
  &GetMockTestCase,
  &ClearMockStorageTestCase,
  &GetMockStorageTestCase,
  &AddMockTestCase
END_TEST_SUITE (MockIoDxeDataStorageUnitTests)

#define NUM_SUITE_TESTS   (NUM_TESTS (GetMockCountTestCase) + \
                           NUM_TESTS (GetMockTestCase) + \
                           NUM_TESTS (ClearMockStorageTestCase) + \
                           NUM_TESTS (GetMockStorageTestCase) + \
                           NUM_TESTS (AddMockTestCase))

VOID
RunMockIoDxeDataStorageUnitTests (
  VOID
  )
{
  TEST_RESULT   TestResults[NUM_SUITE_TESTS];

  RunTestSuite (&MockIoDxeDataStorageUnitTests, &TestResults[0]);
}
