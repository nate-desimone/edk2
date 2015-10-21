/** @file
  Sample PEI Unit Test Suite implementation.

@copyright
  Copyright (c) 2015, Intel Corporation. All rights reserved
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "SampleTestSuitePei.h"

UNIT_TEST (Case1_Test1)
{
  Pass ();
}

UNIT_TEST (Case1_Test2)
{
  Pass ();
}

UNIT_TEST (Case1_Test3)
{
  Fail ("Unit Test 3 in Test Case 1 Fails on purpose.");
}

BEGIN_TEST_CASE (Case1)
  &Case1_Test1,
  &Case1_Test2,
  &Case1_Test3
END_TEST_CASE (Case1, NULL)

UNIT_TEST (Case2_Test1)
{
  Pass ();
}

UNIT_TEST (Case2_Test2)
{
  AssertTrue (0 == 1, "Unit Test 2 in Test Case 2 Fails on purpose.");
}

BEGIN_TEST_CASE (Case2)
  &Case2_Test1,
  &Case2_Test2
END_TEST_CASE (Case2, NULL)

UNIT_TEST (Case3_Test1)
{
  Pass();
}

BEGIN_TEST_CASE (Case3)
  &Case3_Test1
END_TEST_CASE (Case3, NULL)

BEGIN_TEST_SUITE (SamplePeiTestSuite)
  &Case1,
  &Case2,
  &Case3
END_TEST_SUITE (SamplePeiTestSuite)

#define NUM_SAMPLE_SUITE_TESTS   (NUM_TESTS (Case1) + \
                                  NUM_TESTS (Case2) + \
                                  NUM_TESTS (Case3))

VOID
RunSamplePeiUnitTests (
  VOID
  )
{
  TEST_RESULT   TestResults[NUM_SAMPLE_SUITE_TESTS];

  RunTestSuite (&SamplePeiTestSuite, &TestResults[0]);
}
