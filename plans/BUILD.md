# Build Instructions

This document provides detailed instructions for building the Yandex Translator Desktop application from source.

## Prerequisites

### Required Software

1. **Qt 6.x** (6.2 or later recommended)
   - Download from: https://www.qt.io/download
   - Install: Qt 6.x with the following components:
     - Qt 6.x (MSVC 2019 64-bit)
     - Qt WebEngine
     - Qt WebChannel
     - Additional Libraries: Qt Network, Qt Core, Qt Gui, Qt Widgets

2. **CMake 3.16+**
   - Download from: https://cmake.org/download/
   - Add to PATH during installation

3. **C++ Compiler**
   - **Windows**: Visual Studio 2019 or 2022 (with C++ desktop development)
     - Download: https://visualstudio.microsoft.com/downloads/
     - Required workload: "Desktop development with C++"
   - **Alternative**: MinGW-w64 (not officially tested)

4. **Git** (for cloning repository)
   - Download: https://git-scm.com/downloads

### Optional Software

- **Inno Setup** or **NSIS** (for creating installers)
- **WinRAR** or **7-Zip** (for packaging)

## Environment Setup

### Windows (Visual Studio)

1. **Set up Qt Environment Variables**

   Add the following to your system environment variables:

   ```
   CMAKE_PREFIX_PATH=C:\Qt\6.3.0\msvc2019_64
   ```

2. **Verify Qt Installation**

   Open Developer Command Prompt for VS and run:

   ```bash
   qmake --version
   ```

   You should see Qt version information.

## Building the Project

### Step 1: Clone the Repository

```bash
git clone https://github.com/yourusername/YandexTranslator.git
cd YandexTranslator
```

### Step 2: Create Build Directory

```bash
mkdir build
cd build
```

### Step 3: Configure with CMake

#### For Visual Studio 2019 (MSVC):

```bash
cmake -G "Visual Studio 16 2019" -A x64 ^
  -DCMAKE_PREFIX_PATH="C:\Qt\6.3.0\msvc2019_64" ^
  -DCMAKE_BUILD_TYPE=Release ^
  ..
```

#### For Visual Studio 2022 (MSVC):

```bash
cmake -G "Visual Studio 17 2022" -A x64 ^
  -DCMAKE_PREFIX_PATH="C:\Qt\6.3.0\msvc2019_64" ^
  -DCMAKE_BUILD_TYPE=Release ^
  ..
```

#### For MinGW:

```bash
cmake -G "MinGW Makefiles" ^
  -DCMAKE_PREFIX_PATH="C:\Qt\6.3.0\mingw_64" ^
  -DCMAKE_BUILD_TYPE=Release ^
  ..
```

### Step 4: Build

#### Using CMake:

```bash
cmake --build . --config Release
```

#### Using Visual Studio IDE:

1. Open `build/YandexTranslator.sln`
2. Select "Release" configuration
3. Build → Build Solution (Ctrl+Shift+B)

### Step 5: Run the Application

```bash
cd Release
YandexTranslator.exe
```

Or from Visual Studio: Press F5 to run with debugger.

## Deployment

### Step 1: Deploy Qt Dependencies

Use `windeployqt` to bundle all required Qt libraries:

```bash
cd build/Release
windeployqt --release --no-translations --no-system-d3d-compiler --no-opengl-sw YandexTranslator.exe
```

### Step 2: Test the Deployment

Run the executable to ensure all dependencies are included:

```bash
YandexTranslator.exe
```

### Step 3: Create Distribution Package

Create a ZIP archive of the Release directory:

```powershell
Compress-Archive -Path Release\* -DestinationPath YandexTranslator-Windows-x64.zip
```

### Step 4: Create Installer (Optional)

#### Using Inno Setup:

1. Install Inno Setup: https://jrsoftware.org/isdl.php
2. Create `installer.iss`:

```iss
[Setup]
AppName=Yandex Translator
AppVersion=1.0.0
DefaultDirName={pf}\YandexTranslator
DefaultGroupName=Yandex Translator
OutputBaseFilename=YandexTranslator-Setup
Compression=lzma
SolidCompression=yes

[Files]
Source: "build\Release\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\Yandex Translator"; Filename: "{app}\YandexTranslator.exe"
Name: "{commondesktop}\Yandex Translator"; Filename: "{app}\YandexTranslator.exe"

[Run]
Filename: "{app}\YandexTranslator.exe"; Description: "Launch Yandex Translator"; Flags: nowait postinstall skipifsilent
```

3. Compile the installer:

```bash
iscc installer.iss
```

#### Using NSIS:

1. Install NSIS: https://nsis.sourceforge.io/Download
2. Create `installer.nsi`:

```nsis
!define APP_NAME "Yandex Translator"
!define APP_VERSION "1.0.0"
!define APP_EXE "YandexTranslator.exe"

Name "${APP_NAME}"
OutFile "${APP_NAME}-${APP_VERSION}-Setup.exe"
InstallDir "$PROGRAMFILES\${APP_NAME}"
InstallDirRegKey HKLM "Software\${APP_NAME}" "InstallLocation"
RequestExecutionLevel admin

Page directory
Page instfiles

Section "Main Files"
  SetOutPath $INSTDIR
  File /r "build\Release\*"
  
  WriteRegStr HKLM "Software\${APP_NAME}" "InstallLocation" $INSTDIR
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "DisplayName" "${APP_NAME}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "DisplayVersion" "${APP_VERSION}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "UninstallString" "$INSTDIR\uninstall.exe"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "NoRepair" 1
  
  CreateDirectory "$SMPROGRAMS\${APP_NAME}"
  CreateShortCut "$SMPROGRAMS\${APP_NAME}\${APP_NAME}.lnk" "$INSTDIR\${APP_EXE}"
  CreateShortCut "$DESKTOP\${APP_NAME}.lnk" "$INSTDIR\${APP_EXE}"
  
  WriteUninstaller "$INSTDIR\uninstall.exe"
SectionEnd

Section "Uninstall"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}"
  DeleteRegKey HKLM "Software\${APP_NAME}"
  
  RMDir /r "$SMPROGRAMS\${APP_NAME}"
  Delete "$DESKTOP\${APP_NAME}.lnk"
  
  RMDir /r "$INSTDIR"
SectionEnd
```

3. Compile the installer:

```bash
makensis installer.nsi
```

## Building with Different Configurations

### Debug Build

```bash
cmake --build . --config Debug
```

### Release Build with Debug Info

```bash
cmake -G "Visual Studio 16 2019" -A x64 -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
cmake --build . --config RelWithDebInfo
```

### Minimal Build (Smaller Size)

Modify `CMakeLists.txt`:

```cmake
# Add before project()
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT /O2 /GL")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG")
```

## Common Build Issues

### Issue: Qt Not Found

**Error:**
```
Could not find Qt6
```

**Solution:**
```bash
set CMAKE_PREFIX_PATH=C:\Qt\6.3.0\msvc2019_64
cmake -G "Visual Studio 16 2019" -A x64 -DCMAKE_PREFIX_PATH="%CMAKE_PREFIX_PATH%" ..
```

### Issue: CMake Version Too Old

**Error:**
```
CMake 3.16 or higher is required
```

**Solution:**
Install CMake 3.16+ from https://cmake.org/download/

### Issue: Visual Studio Not Found

**Error:**
```
Could not find Visual Studio installation
```

**Solution:**
- Install Visual Studio 2019 or 2022
- Ensure "Desktop development with C++" workload is installed
- Open "Developer Command Prompt for VS" before running CMake

### Issue: WebEngine Not Available

**Error:**
```
Qt6WebEngineWidgets not found
```

**Solution:**
1. Ensure Qt WebEngine is installed in your Qt installation
2. Check Qt Maintenance Tool and add "Qt WebEngine" component
3. Rebuild from clean:

```bash
cd build
cmake --build . --target clean
cmake -G "Visual Studio 16 2019" -A x64 ..
cmake --build . --config Release
```

## Code Signing (Optional)

To sign your application with a code signing certificate:

### Using signtool (Windows SDK):

```bash
signtool sign /f certificate.pfx /p password /t http://timestamp.digicert.com build\Release\YandexTranslator.exe
```

### Using osslsigncode (cross-platform):

```bash
osslsigncode sign -certs certificate.pem -key key.pem -n "Yandex Translator" \
  -i https://github.com/yourusername/YandexTranslator \
  -t http://timestamp.digicert.com \
  -in build/Release/YandexTranslator.exe -out build/Release/YandexTranslator-signed.exe
```

## Continuous Integration

### GitHub Actions Example

Create `.github/workflows/build.yml`:

```yaml
name: Build Windows

on:
  push:
    branches: [ main ]
  pull_request:
    branches: [ main ]

jobs:
  build:
    runs-on: windows-latest
    
    steps:
    - uses: actions/checkout@v3
    
    - name: Install Qt
      uses: jurplel/install-qt-action@v3
      with:
        version: '6.3.0'
        host: 'windows'
        target: 'desktop'
        arch: 'win64_msvc2019_64'
        
    - name: Configure CMake
      run: |
        cmake -B build -G "Visual Studio 16 2019" -A x64 -DCMAKE_PREFIX_PATH="${{ steps.install-qt.outputs.qtDir }}"
        
    - name: Build
      run: cmake --build build --config Release
      
    - name: Deploy Qt
      run: windeployqt --release build/Release/YandexTranslator.exe
      
    - name: Create Archive
      run: Compress-Archive -Path build/Release/* -DestinationPath YandexTranslator-Windows-x64.zip
      
    - name: Upload Artifact
      uses: actions/upload-artifact@v3
      with:
        name: YandexTranslator-Windows-x64
        path: YandexTranslator-Windows-x64.zip
```

## Testing

### Run Unit Tests

```bash
cd build/tests
ctest --config Release --output-on-failure
```

### Run Specific Test

```bash
cd build/tests
./unit/test_clipboard.exe
```

## Performance Profiling

### Using Visual Studio Profiler

1. Open project in Visual Studio
2. Analyze → Performance Profiler
3. Select "CPU Usage" or "Memory Usage"
4. Run the application and perform actions
5. Analyze the results

### Using QElapsedTimer

```cpp
#include <QElapsedTimer>

QElapsedTimer timer;
timer.start();

// Your code here

qDebug() << "Operation took:" << timer.elapsed() << "ms";
```

## Additional Resources

- [Qt Documentation](https://doc.qt.io/)
- [CMake Documentation](https://cmake.org/documentation/)
- [Qt WebEngine Documentation](https://doc.qt.io/qt-6/qtwebengine-index.html)
- [windeployqt Documentation](https://doc.qt.io/qt-6/deploy-windows.html)

## Support

If you encounter build issues not covered here:

1. Check the [Troubleshooting section](README.md#troubleshooting)
2. Search existing [GitHub Issues](https://github.com/yourusername/YandexTranslator/issues)
3. Create a new issue with:
   - Your OS version
   - Qt version
   - CMake version
   - Compiler version
   - Full error message
   - Steps to reproduce

---

Happy building! 🚀
