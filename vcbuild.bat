@echo off

cd %~dp0

@rem Generate the VS project.
SETLOCAL
	if defined VS100COMNTOOLS call "%VS100COMNTOOLS%\VCVarsQueryRegistry.bat"
  python configure
  if errorlevel 1 goto create-msvs-files-failed
  if not exist build/snowcrash.sln goto create-msvs-files-failed
  echo Project files generated.
ENDLOCAL

@rem Skip to the end for now
@rem goto exit

:msbuild
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
msbuild build/snowcrash.sln /m /clp:NoSummary;NoItemAndPropertyList;Verbosity=minimal /nologo
if errorlevel 1 goto exit

@rem All Done
goto exit

:create-msvs-files-failed
echo Failed to create vc project files. 
goto exit

:exit
goto :EOF
