@echo off
setlocal EnableExtensions

rem ===========================================================================
rem  RemoteC CMake configure helper (Windows x64, VS2022 v143, Qt 6.11+).
rem
rem  Dependency locations are taken from the environment:
rem    RLINK_QT_DIR            Qt 6.11+ msvc2022_64 kit
rem    RLINK_WEBRTC_SRC        WebRTC source checkout (built separately with GN)
rem    RLINK_WEBRTC_OUT        WebRTC GN out dir (ReleaseMD)
rem    RLINK_WEBRTC_OUT_DEBUG  WebRTC GN out dir (DebugMD); optional, defaults to
rem                            <RLINK_WEBRTC_SRC>\out\DebugMD
rem
rem  If a Git-ignored cmake\local.bat exists (see cmake\local.bat.example) it is
rem  loaded first, so a machine can keep its paths in one small local file.
rem
rem  This configures only; build with:
rem    cmake --build --preset windows-msvc-x64-v143-release -j
rem    cmake --build --preset windows-msvc-x64-v143-debug   -j
rem ===========================================================================

rem --- load per-machine paths (optional) ------------------------------------
if exist "%~dp0cmake\local.bat" call "%~dp0cmake\local.bat"

rem --- require the environment contract -------------------------------------
set "_missing="
if not defined RLINK_QT_DIR     set "_missing=%_missing% RLINK_QT_DIR"
if not defined RLINK_WEBRTC_SRC set "_missing=%_missing% RLINK_WEBRTC_SRC"
if not defined RLINK_WEBRTC_OUT set "_missing=%_missing% RLINK_WEBRTC_OUT"
if defined _missing (
  echo [ERROR] Missing dependency locations:%_missing%
  echo.
  echo   Set these environment variables, or copy
  echo     cmake\local.bat.example  to  cmake\local.bat
  echo   and edit the paths for this machine, then re-run this script.
  exit /b 1
)

rem --- locate cmake ----------------------------------------------------------
where cmake >nul 2>nul
if errorlevel 1 (
  echo [ERROR] cmake was not found on PATH.
  exit /b 1
)

echo [configure] Qt          = %RLINK_QT_DIR%
echo [configure] WebRTC src  = %RLINK_WEBRTC_SRC%
echo [configure] WebRTC out  = %RLINK_WEBRTC_OUT%
echo.

cd /d "%~dp0"
cmake --preset windows-msvc-x64-v143
if errorlevel 1 (
  echo.
  echo [ERROR] CMake configure failed.
  exit /b 1
)

echo.
echo [configure] done. Build with:
echo   cmake --build --preset windows-msvc-x64-v143-release -j   ^(x64\Release^)
echo   cmake --build --preset windows-msvc-x64-v143-debug   -j   ^(x64\Debug^)
exit /b 0
