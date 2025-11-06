@echo off
REM Build script for Windows

echo Building Dear ImGui Redis Assistant for Windows...

REM Get version
set VERSION=1.0.0

REM Create build directory
if not exist build\windows mkdir build\windows
cd build\windows

REM Configure with CMake
cmake ..\.. ^
    -G "Visual Studio 17 2022" ^
    -A x64 ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DBUILD_SHARED_LIBS=OFF

REM Build
cmake --build . --config Release

REM Create release directory
set RELEASE_DIR=..\..\releases\v%VERSION%\windows
if not exist %RELEASE_DIR% mkdir %RELEASE_DIR%

REM Copy executable
copy bin\Release\DearImGuiRedisAssistant.exe %RELEASE_DIR%\

REM Copy any required files
echo v%VERSION% > %RELEASE_DIR%\VERSION

echo Build complete! Executable in: %RELEASE_DIR%
dir %RELEASE_DIR%
