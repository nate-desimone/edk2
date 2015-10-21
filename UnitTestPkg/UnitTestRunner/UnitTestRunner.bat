@REM @file
@REM  Unit Test Run Script
@REM
@REM @copyright
@REM  Copyright (c) 2015 Intel Corporation. All rights reserved
@REM  SPDX-License-Identifier: BSD-2-Clause-Patent
@REM
@REM @par Specification Reference:
@REM

@echo off
@if not defined DSC_FILE_PATH (
  set DSC_FILE_PATH=UnitTestPkg\UnitTestPkg.dsc
)
@if not defined BUILD_DIR_NAME (
  set BUILD_DIR_NAME=UnitTestUnitTest
)
@if not defined BUILD_TARGET (
  set BUILD_TARGET=DEBUG
)

@REM
@REM Setup Visual Studio environment. Order of precedence is 2014, 2013, 2012, 2010 and then 2008.
@REM
@REM NOTE: To override precedence set TOOL_CHAIN_TAG before calling prep.bat.
@REM       Example: set TOOL_CHAIN_TAG=VS2008
@REM

@REM Check if tool chain has not been selected and Visual Studio 2014 is installed.
@if not defined TOOL_CHAIN_TAG (
  if defined VS140COMNTOOLS (
    set TOOL_CHAIN_TAG=VS2014
  )
)

@REM If Visual Studio 2014 is selected by priority or by preference, setup the environment variables.
@if /I "%TOOL_CHAIN_TAG%"=="VS2014" (
  echo.
  echo Prebuild:  Set the VS2014 environment.
  echo.
  if not defined VSINSTALLDIR call "%VS140COMNTOOLS%\vsvars32.bat"
  if /I "%VS140COMNTOOLS%" == "C:\Program Files\Microsoft Visual Studio 14.0\Common7\Tools\" (
    set TOOL_CHAIN_TAG=VS2014
  ) else (
    set TOOL_CHAIN_TAG=VS2014x86
  )
)

@REM Check if tool chain has not been selected and Visual Studio 2013 is installed.
@if not defined TOOL_CHAIN_TAG (
  if defined VS120COMNTOOLS (
    set TOOL_CHAIN_TAG=VS2013
  )
)

@REM If Visual Studio 2013 is selected by priority or by preference, setup the environment variables.
@if /I "%TOOL_CHAIN_TAG%"=="VS2013" (
  echo.
  echo Prebuild:  Set the VS2013 environment.
  echo.
  if not defined VSINSTALLDIR call "%VS120COMNTOOLS%\vsvars32.bat"
  if /I "%VS120COMNTOOLS%" == "C:\Program Files\Microsoft Visual Studio 12.0\Common7\Tools\" (
    set TOOL_CHAIN_TAG=VS2013
  ) else (
    set TOOL_CHAIN_TAG=VS2013x86
  )
)

@REM Check if tool chain has not been selected and Visual Studio 2012 is installed.
@if not defined TOOL_CHAIN_TAG (
  if defined VS110COMNTOOLS (
    set TOOL_CHAIN_TAG=VS2012
  )
)

@REM If Visual Studio 2012 is selected by priority or by preference, setup the environment variables.
@if /I "%TOOL_CHAIN_TAG%"=="VS2012" (
  echo.
  echo Prebuild:  Set the VS2012 environment.
  echo.
  if not defined VSINSTALLDIR call "%VS110COMNTOOLS%\vsvars32.bat"
  if /I "%VS110COMNTOOLS%" == "C:\Program Files\Microsoft Visual Studio 11.0\Common7\Tools\" (
    set TOOL_CHAIN_TAG=VS2012
  ) else (
    set TOOL_CHAIN_TAG=VS2012x86
  )
)

@REM Check if tool chain has not been selected and Visual Studio 2010 is installed.
@if not defined TOOL_CHAIN_TAG (
  if defined VS100COMNTOOLS (
    set TOOL_CHAIN_TAG=VS2010
  )
)

@REM If Visual Studio 2010 is selected by priority or by preference, setup the environment variables.
@if /I "%TOOL_CHAIN_TAG%"=="VS2010" (
  echo.
  echo Prebuild:  Set the VS2010 environment.
  echo.
  if not defined VSINSTALLDIR call "%VS100COMNTOOLS%\vsvars32.bat"
  if /I "%VS100COMNTOOLS%" == "C:\Program Files\Microsoft Visual Studio 10.0\Common7\Tools\" (
    set TOOL_CHAIN_TAG=VS2010
  ) else (
    set TOOL_CHAIN_TAG=VS2010x86
  )
)

@REM Check if tool chain has not been selected and Visual Studio 2008 is installed.
@if not defined TOOL_CHAIN_TAG (
  if defined VS90COMNTOOLS (
    set TOOL_CHAIN_TAG=VS2008
  )
)

@REM If Visual Studio 2008 is selected by priority or by preference, setup the environment variables.
@if /I "%TOOL_CHAIN_TAG%"=="VS2008" (
  echo.
  echo Prebuild:  Set the VS2008 environment.
  echo.
  if not defined VSINSTALLDIR call "%VS90COMNTOOLS%\vsvars32.bat"
  if /I "%VS90COMNTOOLS%" == "C:\Program Files\Microsoft Visual Studio 9.0\Common7\Tools\" (
    set TOOL_CHAIN_TAG=VS2008
  ) else (
    set TOOL_CHAIN_TAG=VS2008x86
  )
)

@REM If no supported version of Visual Studio was detected, return an error.
@if not defined TOOL_CHAIN_TAG (
  echo.
  echo !!! ERROR !!! Visual Studio not installed correctly!!!
  echo.
  set SCRIPT_ERROR=1
  goto :EndPreBuild
)

pushd ..\..
call edksetup.bat

build -n %NUMBER_OF_PROCESSORS% -p %DSC_FILE_PATH% -t %TOOL_CHAIN_TAG% -a IA32 -a X64 -b %BUILD_TARGET%
@if %ERRORLEVEL% NEQ 0 (
  popd
  goto END
)

popd
UnitTestRunner.py

:END

pushd ..\..\Build
rmdir /S /Q %BUILD_DIR_NAME%
popd
