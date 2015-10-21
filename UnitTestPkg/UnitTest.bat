@REM @file
@REM  Kaby Lake Unit Test Run Script
@REM
@REM @copyright
@REM  Copyright (c) 2015 Intel Corporation. All rights reserved
@REM  SPDX-License-Identifier: BSD-2-Clause-Patent
@REM

@echo off
set DSC_FILE_PATH=UnitTestPkg\UnitTestPkg.dsc
set BUILD_DIR_NAME=UnitTestUnitTest

pushd UnitTestRunner
call UnitTestRunner.bat
popd

@if exist UnitTestRunner\TestResults32.xml (
  move /Y UnitTestRunner\TestResults32.xml TestResults32.xml > NUL
)

@if exist UnitTestRunner\TestResults64.xml (
  move /Y UnitTestRunner\TestResults64.xml TestResults64.xml > NUL
)
