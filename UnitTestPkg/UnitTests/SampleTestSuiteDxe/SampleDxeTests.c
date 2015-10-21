/** @file
  Sample DXE Unit Tests

@copyright
 Copyright (c) 2015 Intel Corporation. All rights reserved
 SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "SampleTestSuiteDxe.h"

UNIT_TEST (Case4_Test1)
{
  Pass ();
}

UNIT_TEST (Case4_Test2)
{
  Pass ();
}

UNIT_TEST (Case4_Test3)
{
  Fail ("Unit Test 3 in Test Case 4 Fails on purpose.");
}

BEGIN_TEST_CASE (Case4)
  &Case4_Test1,
  &Case4_Test2,
  &Case4_Test3
END_TEST_CASE (Case4, NULL)

UNIT_TEST (Case5_Test1)
{
  Pass ();
}

UNIT_TEST (Case5_Test2)
{
  AssertTrue (0 == 1, "Unit Test 2 in Test Case 5 Fails on purpose.");
}

BEGIN_TEST_CASE (Case5)
  &Case5_Test1,
  &Case5_Test2
END_TEST_CASE (Case5, NULL)

UNIT_TEST (Case6_Test1)
{
  Pass();
}

BEGIN_TEST_CASE (Case6)
  &Case6_Test1
END_TEST_CASE (Case6, NULL)

BEGIN_TEST_SUITE (SampleDxeTestSuite)
  &Case4,
  &Case5,
  &Case6
END_TEST_SUITE (SampleDxeTestSuite)

#define NUM_SAMPLE_SUITE_TESTS   (NUM_TESTS (Case4)   + \
                                  NUM_TESTS (Case5)   + \
                                  NUM_TESTS (Case6))

VOID
RunSampleDxeUnitTests (
  VOID
  )
{
  TEST_RESULT   TestResults[NUM_SAMPLE_SUITE_TESTS];

  RunTestSuite (&SampleDxeTestSuite, &TestResults[0]);
}
