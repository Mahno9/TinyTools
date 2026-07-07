@echo off
setlocal

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
    copy TinyTools.exe Release\bin\
) else if exist Release\TinyTools.exe (
    copy Release\TinyTools.exe Release\bin\
) else (
    echo Error: TinyTools.exe not found!
    exit /b 1
)
"%QTDIR%\bin\windeployqt.exe" --release --compiler-runtime --no-translations --dir Release\bin Release\bin\TinyTools.exe
echo Packaging complete.
