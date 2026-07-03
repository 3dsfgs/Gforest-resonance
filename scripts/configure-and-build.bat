@echo off
setlocal EnableExtensions

set "VS_ROOT=D:\vs2022_Professional"
set "PROJECT_ROOT=%~dp0.."
set "CMAKE_BIN=%VS_ROOT%\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin"
set "NINJA_BIN=%VS_ROOT%\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja"

call "%VS_ROOT%\Common7\Tools\VsDevCmd.bat" -arch=amd64 -host_arch=amd64
if %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%

set "PATH=%CMAKE_BIN%;%NINJA_BIN%;%PATH%"
cd /d "%PROJECT_ROOT%"

echo Using compiler:
where cl

if "%1"=="clean" rmdir /s /q ".out" 2>nul

echo === CMake Configure (msvc-debug) ===
cmake --preset msvc-debug
if %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%

echo === CMake Build (msvc-debug) ===
cmake --build .out/build/msvc-debug
if %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%

if exist "project\bin\roguelite.windows.64.debug.dll" (
    echo SUCCESS: project\bin\roguelite.windows.64.debug.dll
) else (
    echo WARNING: Expected DLL not found
    dir /b project\bin\*.dll 2>nul
)

endlocal
