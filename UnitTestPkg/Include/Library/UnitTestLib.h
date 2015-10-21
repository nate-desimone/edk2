/** @file
  Unit Test Library header file

@copyright
  Copyright (c) 2015 Intel Corporation. All rights reserved
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#ifndef _UNIT_TEST_LIB_H_
#define _UNIT_TEST_LIB_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <Base.h>

/**
  Stores the results from running a unit test
**/
typedef struct {
  /**
    TRUE is the Unit Test Failed, FALSE otherwise
  **/
  BOOLEAN   Failure;
  /**
    If the test failed, the line number where the failure occurred.
  **/
  UINT32    Line;
  /**
    If the test failed, the source file name where the failure occurred.
  **/
  CHAR8     *File;
  /**
    If the test failed, the error message.
  **/
  CHAR8     *Message;
} TEST_RESULT;

/**
  A Unit Test
**/
typedef struct {
  /**
    The name of the Unit Test
  **/
  CHAR8   *Name;
  /**
    The function pointer to invoke to run the Unit Test
  **/
  VOID
  (*Method) (
    IN    VOID          *TestFixtureState,
    OUT   TEST_RESULT   *__result
    );
} UNIT_TEST;

/**
  Macro used to define a Unit Test

  @param[in] NAME          - The Name of the Unit Test

  Usage:
  UNIT_TEST(test_name) {
    ... test body ...
  }
**/
#define UNIT_TEST(NAME) \
  VOID \
  NAME##_method ( \
    VOID*, \
    TEST_RESULT* \
    ); \
  UNIT_TEST  NAME = {#NAME, NAME##_method}; \
  VOID \
  NAME##_method ( \
    VOID        *TestFixtureState, \
    TEST_RESULT *__result \
    )

/**
  Used within a unit test's code to denote that the unit test has failed

  @param[in] MESSAGE          - The failure message
**/
#define Fail(MESSAGE) \
  __result->Failure = TRUE; \
  __result->Line    = __LINE__; \
  __result->File    = __FILE__; \
  __result->Message = MESSAGE; \
  return;

/**
  Used within a unit test's code to exit immediately with a passing result.
**/
#define Pass() \
  return;

/**
  Used within a unit test's code to check a condition and exit the test with
  a failure if the condition is false.

  @param[in] CONDITION        - If zero, then the test will end with a failure.
                                Otherwise, the test will continue.
  @param[in] MESSAGE          - The failure message to display if the condition is zero
**/
#define AssertTrue(CONDITION, MESSAGE) \
  if (! (CONDITION)) { \
    __result->Failure = !(CONDITION); \
    __result->Line    = __LINE__; \
    __result->File    = __FILE__; \
    __result->Message = MESSAGE; \
    return; \
  }

/**
  A Test Case
**/
typedef struct TEST_CASE {
  /**
    The name of the Test Case
  **/
  CHAR8       *Name;
  /**
    Array of pointers to Unit Tests that are included in this Test Case
  **/
  UNIT_TEST   **UnitTests;
  /**
    Length of the UnitTests array
  **/
  UINT32      UnitTestsLength;
  /**
    An optional method to Set Up any Test Fixture that the Unit Tests may need
    After setting up the Test Fixture, SetUpMethod calls DispatchTestCase()
  **/
  OPTIONAL VOID
  (*SetUpMethod) (
    /**
      Set Up method should pass this to DispatchTestCase() unmodified
    **/
    IN          TEST_RESULT   *TestResults,
    /**
      Set Up method should pass this to DispatchTestCase() unmodified
    **/
    IN          UINT32        TestResultsIndex,
    /**
      The Test Case to initialize the Test Fixture for
    **/
    IN  struct  TEST_CASE     *TestCase
    );
} TEST_CASE;

/**
  Macro used to declare the existance of a unit test.
  Allows the unit test to exist in a seperate .c file
  from the test case

  @param[in] NAME          - The Name of the Unit Test

  Usage:
  UNIT_TEST(test_name) {
    ... test body ...
  }
**/
#define DECLARE_UNIT_TEST(NAME) \
  extern UNIT_TEST  NAME;

/**
  Macro used to define the start of a Test Case

  @param[in] NAME          - The Name of the Test Case

  Usage:
  BEGIN_TEST_CASE(case_name)
    unit_test_name1,
    unit_test_name2,
    ...
  END_TEST_CASE(case_name, SetUpMethod or NULL)
**/
#define BEGIN_TEST_CASE(NAME) \
  static  UNIT_TEST   *NAME##_unittests[] = {

/**
  Macro used to determine the number of unit tests in a test case

  @param[in] NAME          - The Name of the Test Case

  @retval The number unit tests in the test case
**/
#define NUM_TESTS(NAME) (sizeof(NAME##_unittests) / sizeof(NAME##_unittests[0]))

/**
  Macro used to define the end of a Test Case

  @param[in] NAME          - The Name of the Test Case
  @param[in] SET_UP_METHOD - The Set Up Method for the Test Case, or NULL

  Usage:
  BEGIN_TEST_CASE(case_name)
    unit_test_name1,
    unit_test_name2,
    ...
  END_TEST_CASE(case_name, SetUpMethod or NULL)
**/
#define END_TEST_CASE(NAME, SET_UP_METHOD) \
  }; \
  static TEST_CASE  NAME = {#NAME, &NAME##_unittests[0], \
                    NUM_TESTS(NAME), \
                    SET_UP_METHOD }; \


/**
  A Test Suite
**/
typedef struct {
  /**
    The name of the Test Suite
  **/
  CHAR8       *Name;
  /**
    Array of pointers to Test Cases that are included in this Test Suite
  **/
  TEST_CASE   **TestCases;
  /**
    Length of the TestCases array
  **/
  UINT32      TestCasesLength;
} TEST_SUITE;

/**
  Macro used to define the start of a Test Suite

  @param[in] NAME          - The Name of the Test Suite

  Usage:
  BEGIN_TEST_SUITE(suite_name)
    test_case_name1,
    test_case_name2,
    ...
  END_TEST_SUITE(suite_name)
**/
#define BEGIN_TEST_SUITE(NAME) \
  static TEST_CASE *NAME##_testcases[] = {

/**
  Macro used to define the end of a Test Suite

  @param[in] NAME          - The Name of the Test Suite

  Usage:
  BEGIN_TEST_SUITE(suite_name)
    test_case_name1,
    test_case_name2,
    ...
  END_TEST_SUITE(suite_name)
**/
#define END_TEST_SUITE(NAME) \
  }; \
  static TEST_SUITE NAME = {#NAME, &NAME##_testcases[0], \
                    (sizeof(NAME##_testcases) / sizeof(NAME##_testcases[0]))}; \

/**
  Run all unit tests contained in the given Test Suite

  @param[in]  TestSuite           The test suite to run
  @param[in]  TestResults         A pre-allocated buffer of test results.  This
                                  is passed in from the caller so that it is
                                  possible for the Test Results to be generated
                                  using the stack.  The size of this array must
                                  be large enough to contain results from all
                                  unit tests contained in the test suite.  The
                                  caller does not need to process the results
                                  in this array, it is done automatically.
**/
VOID
RunTestSuite (
  IN    TEST_SUITE    *TestSuite,
  IN    TEST_RESULT   *TestResults
  );

/**
  If a test case contains a Set Up Method, after creating the Test Fixture the
  Set Up method should call this function to execute all of the unit tests.
  Generally this method should not be called by anything except the Set Up
  Method

  @param[in]  TestCase            The test case to run
  @param[in]  TestResults         A pre-allocated buffer of test results.  This
                                  is passed in from the caller.  The Set Up
                                  Method should pass this in unmodified.
  @param[in]  TestResultsIndex    Index of the Test Results array.  This is
                                  passed in from the caller.  The Set Up Method
                                  should pass this in unmodified.
  @param[in]  TestFixtureState    The Test Fixture that the Set Up Method
                                  created.  Can be used by Unit Tests later.
**/
VOID
DispatchTestCase (
  IN    TEST_CASE     *TestCase,
  IN    TEST_RESULT   *TestResults,
  IN    UINT32        TestResultsIndex,
  IN    VOID          *TestFixtureState
  );

#ifdef __cplusplus
}
#endif
#endif
