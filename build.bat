@echo off
setlocal

:: -- Command line build -------------------------------------------------------
:: Generates the solution for whichever Visual Studio is installed, then builds it.
:: Also what the VS Code "Build" task runs.
::
:: Usage:
::   build.bat               -> Release
::   build.bat Debug         -> Debug
::
:: The exe ends up in build\bin\<config>\ImageGeometrise\ with everything it needs next to it.

set CONFIG=%1
if "%CONFIG%"=="" set CONFIG=Release

set VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe
if not exist "%VSWHERE%" (
    echo [error] Visual Studio not found. Install Visual Studio 2022 or later with "Desktop development with C++".
    exit /b 1
)

:: Newest Visual Studio with MSBuild and the C++ tools
set VSVER=
for /f "usebackq tokens=1 delims=." %%v in (`"%VSWHERE%" -latest -prerelease -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationVersion`) do set VSVER=%%v
set MSBUILD=
for /f "usebackq tokens=*" %%m in (`"%VSWHERE%" -latest -prerelease -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -find MSBuild\**\Bin\MSBuild.exe`) do set MSBUILD=%%m
if not defined MSBUILD (
    echo [error] MSBuild with the C++ tools not found. Add "Desktop development with C++" in the Visual Studio Installer.
    exit /b 1
)

set ACTION=vs2026
if "%VSVER%"=="17" set ACTION=vs2022

pushd "%~dp0"
call "%~dp0createproject.bat" %ACTION%
if %ERRORLEVEL% neq 0 (
    popd
    exit /b 1
)

set SLN=
for %%s in (build\*.slnx build\*.sln) do if not defined SLN set SLN=%%s

echo [build] Building %CONFIG%...
"%MSBUILD%" "%SLN%" -p:Configuration=%CONFIG% -p:Platform=x64 -m -nologo -v:minimal
if %ERRORLEVEL% neq 0 (
    echo [error] Build failed.
    popd
    exit /b 1
)

popd
echo [build] Done: build\bin\%CONFIG%\ImageGeometrise\ImageGeometrise.exe
exit /b 0
