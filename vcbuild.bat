@echo off

@rem Attribution Notice
@rem ------------------
@rem This file uses parts of Node.js `vcbuild.bat`.
@rem Please refer to https://github.com/joyent/node.

cd %~dp0

if /i "%1"=="help" goto help
if /i "%1"=="--help" goto help
if /i "%1"=="-help" goto help
if /i "%1"=="/help" goto help
if /i "%1"=="?" goto help
if /i "%1"=="-?" goto help
if /i "%1"=="--?" goto help
if /i "%1"=="/?" goto help

@rem Process arguments.
set config=Release
set target=Build
set target_arch=ia32
set noprojgen=
set nobuild=
set test=

:next-arg
if "%1"=="" goto args-done
if /i "%1"=="debug"         set config=Debug&goto arg-ok
if /i "%1"=="release"       set config=Release&goto arg-ok
if /i "%1"=="clean"         set target=Clean&goto arg-ok
if /i "%1"=="ia32"          set target_arch=ia32&goto arg-ok
if /i "%1"=="x86"           set target_arch=ia32&goto arg-ok
if /i "%1"=="x64"           set target_arch=x64&goto arg-ok
if /i "%1"=="noprojgen"     set noprojgen=1&goto arg-ok
if /i "%1"=="nobuild"       set nobuild=1&goto arg-ok
if /i "%1"=="test"          set test=test&goto arg-ok
if /i "%1"=="inttest"       set inttest=1&goto arg-ok
if /i "%1"=="MSVC2012"      set GYP_MSVS_VERSION=2012&goto arg-ok
if /i "%1"=="MSVC2010"      set GYP_MSVS_VERSION=2010&goto arg-ok
if /i "%1"=="MSVC2008"      set GYP_MSVS_VERSION=2008&goto arg-ok

echo Warning: ignoring invalid command line option `%1`.

:arg-ok
shift
goto next-arg

:args-done
if "%config%"=="Debug" set debug_arg=--debug

:project-gen
@rem Skip project generation if requested.
if defined noprojgen goto msbuild
if "%GYP_MSVS_VERSION%"=="" set GYP_MSVS_VERSION=2012

@rem Generate the VS project.
SETLOCAL
  if defined VS100COMNTOOLS call "%VS100COMNTOOLS%\VCVarsQueryRegistry.bat"
  if defined inttest python configure %debug_arg% --dest-cpu=%target_arch% --include-integration-tests
  if not defined inttest python configure %debug_arg% --dest-cpu=%target_arch%
  if errorlevel 1 goto create-msvs-files-failed
  if not exist build/snowcrash.sln goto create-msvs-files-failed
  echo Project files generated.
ENDLOCAL

@rem Skip to the end for now
@rem goto exit

:msbuild
@rem Skip project generation if requested.
if defined nobuild goto exit
if "%GYP_MSVS_VERSION%"=="2010" goto vc-set-2010
if "%GYP_MSVS_VERSION%"=="2008" goto vc-set-2008

@rem Look for Visual Studio 2012
if not defined VS110COMNTOOLS goto vc-set-2010
if not exist "%VS110COMNTOOLS%\..\..\vc\vcvarsall.bat" goto vc-set-2010
call "%VS110COMNTOOLS%\..\..\vc\vcvarsall.bat"
if not defined VCINSTALLDIR goto msbuild-not-found
goto msbuild-found

:vc-set-2010
if not defined VS100COMNTOOLS goto vc-set-2008
if not exist "%VS100COMNTOOLS%\..\..\vc\vcvarsall.bat" goto vc-set-2008
call "%VS100COMNTOOLS%\..\..\vc\vcvarsall.bat"
if not defined VCINSTALLDIR goto msbuild-not-found
goto msbuild-found

:vc-set-2008
if not defined VS90COMNTOOLS goto msbuild-not-found
if not exist "%VS90COMNTOOLS%\..\..\vc\vcvarsall.bat" goto msbuild-not-found
call "%VS90COMNTOOLS%\..\..\vc\vcvarsall.bat"
if not defined VCINSTALLDIR goto msbuild-not-found
goto msbuild-found

:msbuild-not-found
echo Build skipped. To build, this file needs to run from VS cmd prompt.
goto exit

:msbuild-found
@rem Build the sln with msbuild.
@rem Refer to http://msdn.microsoft.com/en-us/library/ms164311.aspx
echo Building Snow Crash...
msbuild build/snowcrash.sln /m /clp:NoSummary;NoItemAndPropertyList;Verbosity=normal /nologo /property:Configuration=%config%
if errorlevel 1 goto exit

:run
@rem Run tests if requested.
if "%test%"=="" goto intigration-test
echo Running tests...
.\build\%config%\test-libsnowcrash.exe

:intigration-test
if defined inttest goto run-integration-test

@rem All Done
goto exit

:run-integration-test
if "%config%"=="Debug" (
SET "Replacement=      ENV['PATH'] = "../../build/Debug""
goto :run-cucumber
)
SET "Replacement=      ENV['PATH'] = "../../build/Release""
goto :run-cucumber

:run-cucumber
SET "file=features\support\env-win.rb"
if exist "%file%" goto env-exist
SETLOCAL ENABLEDELAYEDEXPANSION
SET "line=require 'aruba/cucumber'"
ECHO !line! >"%file%"
SET "line=require 'rbconfig'"
ECHO !line! >>"%file%"
SET "line=Before do"
ECHO !line! >>"%file%"
SET "line=  @dirs << "../../features/fixtures""
ECHO !line! >>"%file%"
SET "line=  case RbConfig::CONFIG['host_os']"
ECHO !line! >>"%file%"
SET "line=    when /mswin|msys|mingw|cygwin|bccwin|wince|emc/"
ECHO !line! >>"%file%"
ECHO !Replacement! >>"%file%"
SET "line=end"
ECHO !line! >>"%file%"
SET "line=end"
ECHO !line! >>"%file%"
ENDLOCAL

bundle exec cucumber
goto exit

:env-exist
SET /a Line#ToSearch=7
(FOR /f "tokens=1*delims=:" %%a IN ('findstr /n "^" "%file%"') DO (
    SET "Line=%%b"
    IF %%a equ %Line#ToSearch% SET "Line=%Replacement%"
    SETLOCAL ENABLEDELAYEDEXPANSION
    ECHO(!Line!)
    ENDLOCAL
)>"%file%.new"
MOVE "%file%.new" "%file%" >nul

bundle exec cucumber
goto exit

:create-msvs-files-failed
echo Failed to create vc project files. 
goto exit

:help
echo vcbuild.bat [debug/release] [test] [clean] [noprojgen] [nobuild] [x86/x64] [inttest] [MSVC2008/MSVC2010/MSVC2012]
echo Examples:
echo   vcbuild.bat                : builds release build
echo   vcbuild.bat nobuild        : generate MSVS project files only
echo   vcbuild.bat debug          : builds debug build
echo   vcbuild.bat test           : builds debug build and runs tests
echo   vcbuild.bat inttest        : include integration tests
echo   vcbuild.bat MSVC2012       : indicate target solution's version, could also define as MSVC2008 , MSVC2010 , MSVC2012
goto exit

:exit
goto :EOF
