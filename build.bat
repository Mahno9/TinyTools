@echo off
if exist build rmdir /S /Q build
mkdir build
cd build
cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=C:/Qt/6.3.0/msvc2019_64 ..
if %errorlevel% neq 0 exit /b %errorlevel%
cmake --build . --config Release
if %errorlevel% neq 0 exit /b %errorlevel%
echo Running windeployqt...
if not exist Release\bin mkdir Release\bin
copy Release\TinyTools.exe Release\bin\
C:\Qt\6.3.0\msvc2019_64\bin\windeployqt.exe --release --compiler-runtime --no-translations --dir Release\bin Release\bin\TinyTools.exe
echo Packaging complete.
