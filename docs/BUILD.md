# Build Instructions

This document provides detailed instructions for building the TinyTools application from source.

## Prerequisites

### Required Software

1. **Qt 6.x** (6.2 or later recommended)
   - Download from: https://www.qt.io/download
   - Install: Qt 6.x with the following components:
     - Qt 6.x (MSVC 2019/2022 64-bit)
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
git clone https://github.com/yourusername/TinyTools.git
cd TinyTools
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

1. Open `build/TinyTools.sln`
2. Select "Release" configuration
3. Build → Build Solution (Ctrl+Shift+B)

### Step 5: Run the Application

```bash
cd Release
TinyTools.exe
```

Or from Visual Studio: Press F5 to run with debugger.

## Deployment

### Step 1: Deploy Qt Dependencies

Use `windeployqt` to bundle all required Qt libraries:

```bash
cd build/Release
windeployqt --release --no-translations --no-system-d3d-compiler --no-opengl-sw TinyTools.exe
```

### Step 2: Test the Deployment

Run the executable to ensure all dependencies are included:

```bash
TinyTools.exe
```

### Step 3: Create Distribution Package

Create a ZIP archive of the Release directory:

```powershell
Compress-Archive -Path Release\* -DestinationPath TinyTools-Windows-x64.zip
```

### Step 4: Create Installer (Optional)

#### Using Inno Setup:

1. Install Inno Setup: https://jrsoftware.org/isdl.php
2. Create `installer.iss`:

```iss
[Setup]
AppName=TinyTools
AppVersion=1.0.0
DefaultDirName={pf}\TinyTools
DefaultGroupName=TinyTools
OutputBaseFilename=TinyTools-Setup
Compression=lzma
SolidCompression=yes

[Files]
Source: "build\Release\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\TinyTools"; Filename: "{app}\TinyTools.exe"
Name: "{commondesktop}\TinyTools"; Filename: "{app}\TinyTools.exe"

[Run]
Filename: "{app}\TinyTools.exe"; Description: "Launch TinyTools"; Flags: nowait postinstall skipifsilent
```

3. Compile the installer:

```bash
iscc installer.iss
```

#### Using NSIS:

1. Install NSIS: https://nsis.sourceforge.io/Download
2. Create `installer.nsi`:

```nsis
!define APP_NAME "TinyTools"
!define APP_VERSION "1.0.0"
!define APP_EXE "TinyTools.exe"

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

## Support

If you encounter build issues not covered here:

1. Check the [Troubleshooting section](../README.md#troubleshooting)
2. Search existing Issues
