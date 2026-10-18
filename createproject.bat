@echo off
setlocal

:: -- Solution generator -------------------------------------------------------
:: Generic: nothing in here is repo specific. Copy it and premake5.lua into any
:: repo root, then write a workspace.lua. Packages are downloaded by premake5.lua.
::
:: Usage:
::   createproject.bat               -> Visual Studio 2026 (default)
::   createproject.bat vs2022        -> Visual Studio 2022
::   createproject.bat open          -> generate, then open the solution
::
:: Re-run it after adding or removing source files.

set ACTION=vs2026
set OPEN=0
for %%a in (%*) do (
    if /i "%%a"=="open" (set OPEN=1) else (set ACTION=%%a)
)

:: premake5 on PATH, or premake5.exe dropped next to this file
set PREMAKE=premake5
where premake5 >nul 2>&1
if %ERRORLEVEL% neq 0 (
    if exist "%~dp0premake5.exe" (
        set PREMAKE="%~dp0premake5.exe"
    ) else (
        echo [error] premake5 not found. Get premake5.exe from https://premake.github.io/download
        echo         and put it on PATH or next to createproject.bat.
        exit /b 1
    )
)

pushd "%~dp0"

:: Pull submodules (sub/*)
if exist ".gitmodules" git submodule update --init --recursive

echo [build] Generating %ACTION% solution...
%PREMAKE% %ACTION%
if %ERRORLEVEL% neq 0 (
    echo [error] premake failed.
    popd
    exit /b 1
)

if "%OPEN%"=="1" (
    for %%s in (build\*.slnx build\*.sln) do (
        start "" "%%s"
        goto :opened
    )
)
:opened

popd
echo [build] Done. Solution is in build\
exit /b 0
