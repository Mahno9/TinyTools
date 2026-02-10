# escape=`

# Use Windows Server Core 2022 as the base image
FROM mcr.microsoft.com/windows/servercore:ltsc2022

# Restore the default Windows shell for correct batch processing.
SHELL ["cmd", "/S", "/C"]

# Download the Visual Studio 2022 Build Tools bootstrapper.
RUN mkdir C:\TEMP && `
    powershell -Command "Invoke-WebRequest -Uri https://aka.ms/vs/17/release/vs_buildtools.exe -OutFile C:\TEMP\vs_buildtools.exe"

# Install Visual Studio 2022 Build Tools with the C++ workload and necessary components.
# We include the VC++ 2019 toolset (Microsoft.VisualStudio.Component.VC.14.29.16.11.x86.x64) for compatibility if needed,
# or just use the latest 2022 toolset which is binary compatible.
# Note: This step takes a long time and requires a lot of space.
RUN C:\TEMP\vs_buildtools.exe --quiet --wait --norestart --nocache `
    --installPath C:\BuildTools `
    --add Microsoft.VisualStudio.Workload.VCTools `
    --add Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
    --add Microsoft.VisualStudio.Component.Windows10SDK.19041 `
    --add Microsoft.VisualStudio.Component.VC.CMake.Project `
    || IF "%ERRORLEVEL%"=="3010" EXIT 0

# Install Chocolatey for package management
# Pin to 1.4.0 as it is the last version supporting .NET 4.5 by default
# (Windows Server Core 2022 comes with .NET 4.8)
ENV chocolateyVersion=1.4.0
RUN powershell -NoProfile -ExecutionPolicy Bypass -Command `
    "iex ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))"

# Install tools dependencies: Git, Python, Ninja
RUN choco install -y git python ninja

# Install pip and aqtinstall (for installing Qt)
RUN python -m pip install --upgrade pip && `
    python -m pip install aqtinstall

# Install Qt 6.3.0 for MSVC 2019 64-bit
# We install it to C:\Qt
RUN mkdir C:\Qt && `
    python -m aqt install-qt windows desktop 6.3.0 win64_msvc2019_64 --outputdir C:\Qt --modules qtwebengine qtwebchannel qtpositioning

# Add Qt, CMake, and Ninja to PATH
# Note: Visual Studio tools are best accessed via the developer command prompt, so we set that as entrypoint.
ENV PATH="C:\Qt\6.3.0\msvc2019_64\bin;C:\Program Files\Git\cmd;C:\Python312;C:\Python312\Scripts;C:\Windows\system32;C:\Windows"

# Define the working directory
WORKDIR C:/app

# Copy the entrypoint script
COPY entrypoint.cmd C:/entrypoint.cmd
COPY build.bat C:/build.bat

# Use the entrypoint script to set up the environment
ENTRYPOINT ["C:\\entrypoint.cmd"]
