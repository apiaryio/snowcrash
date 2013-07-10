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

echo Warning: ignoring invalid command line option `%1`.

:arg-ok
shift
goto next-arg

:args-done
if "%config%"=="Debug" set debug_arg=--debug

:project-gen
@rem Skip project generation if requested.
if defined noprojgen goto msbuild

@rem Generate the VS project.
SETLOCAL
	if defined VS100COMNTOOLS call "%VS100COMNTOOLS%\VCVarsQueryRegistry.bat"
  python configure %debug_arg% --dest-cpu=%target_arch%
  if errorlevel 1 goto create-msvs-files-failed
  if not exist build/snowcrash.sln goto create-msvs-files-failed
  echo Project files generated.
ENDLOCAL

@rem Skip to the end for now
@rem goto exit

:msbuild
@rem Skip project generation if requested.
if defined nobuild goto exit

@rem Look for Visual Studio 2012
if not defined VS110COMNTOOLS goto vc-set-2010
if not exist "%VS110COMNTOOLS%\..\..\vc\vcvarsall.bat" goto vc-set-2010
call "%VS110COMNTOOLS%\..\..\vc\vcvarsall.bat"
if not defined VCINSTALLDIR goto msbuild-not-found
set GYP_MSVS_VERSION=2012
goto msbuild-found

:vc-set-2010
if not defined VS100COMNTOOLS goto msbuild-not-found
if not exist "%VS100COMNTOOLS%\..\..\vc\vcvarsall.bat" goto msbuild-not-found
call "%VS100COMNTOOLS%\..\..\vc\vcvarsall.bat"
if not defined VCINSTALLDIR goto msbuild-not-found
goto msbuild-found

:msbuild-not-found
echo Build skipped. To build, this file needs to run from VS cmd prompt.
goto exit

:msbuild-found
@rem Build the sln with msbuild.
echo Building Snow Crash...
msbuild build/snowcrash.sln /m /clp:NoSummary;NoItemAndPropertyList;Verbosity=minimal /nologo
if errorlevel 1 goto exit

:run
@rem Run tests if requested.
if "%test%"=="" goto exit
echo Running tests...
.\build\Debug\test-snowcrash.exe

@rem All Done
goto exit

:create-msvs-files-failed
echo Failed to create vc project files. 
goto exit

:help
echo vcbuild.bat [debug/release] [test] [clean] [noprojgen] [nobuild] [x86/x64]
echo Examples:
echo   vcbuild.bat                : builds release build
echo   vcbuild.bat nobuild        : generate MSVS project files only
echo   vcbuild.bat debug          : builds debug build
echo   vcbuild.bat test           : builds debug build and runs tests
goto exit

:exit
goto :EOF
