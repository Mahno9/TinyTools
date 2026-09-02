@echo off
setlocal enabledelayedexpansion

rem Auto-init the MSVC dev environment if this shell doesn't already have one
rem active (INCLUDE unset). Needed because rc.exe has no SDK auto-detection
rem of its own and fails on windows.h without it, even though clang/cl do.
if "%INCLUDE%"=="" (
    for /f "usebackq delims=" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -prerelease -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath 2^>nul`) do set "VSPATH=%%i"
    if not defined VSPATH (
        echo Error: MSVC build tools not found. Install "Desktop development with C++" in Visual Studio, or run this from an "x64 Native Tools Command Prompt for VS".
        exit /b 1
    )
    call "!VSPATH!\VC\Auxiliary\Build\vcvars64.bat"
)

rem Qt location: env QTDIR wins, otherwise use the pinned version.
if "%QTDIR%"=="" set "QTDIR=C:\Qt\6.10.3\msvc2022_64"
if not exist "%QTDIR%" (
    echo Error: Qt not found at %QTDIR%. Set QTDIR to your Qt installation.
    exit /b 1
)
set "QTDIR_FWD=%QTDIR:\=/%"

if exist build rmdir /S /Q build
mkdir build
cd build
cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=%QTDIR_FWD% ..
if %errorlevel% neq 0 exit /b %errorlevel%
cmake --build . --config Release
if %errorlevel% neq 0 exit /b %errorlevel%

echo Running windeployqt...
if not exist Release\bin mkdir Release\bin
if exist TinyTools.exe (
    copy /y TinyTools.exe Release\bin\
) else if exist Release\TinyTools.exe (
    copy /y Release\TinyTools.exe Release\bin\
) else (
    echo Error: TinyTools.exe not found!
    exit /b 1
)
if %errorlevel% neq 0 (
    echo Error: could not copy TinyTools.exe to Release\bin - is a running instance holding it locked? Close TinyTools and rebuild.
    exit /b 1
)
"%QTDIR%\bin\windeployqt.exe" --release --compiler-runtime --no-translations --dir Release\bin Release\bin\TinyTools.exe
echo Packaging complete.
