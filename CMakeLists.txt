cmake_minimum_required(VERSION 3.24)

project(RLinkRemoteCtrl
  VERSION 0.1.0
  DESCRIPTION "RemoteC remote control suite"
  LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# ---------------------------------------------------------------------------
# Per-machine dependency locations.
#
# These come from the environment so that no machine path is committed. On
# Windows, cmake\local.bat (Git-ignored, see cmake\local.bat.example) can set
# them; CI sets them directly. Command-line -D overrides still win.
# ---------------------------------------------------------------------------
set(RLINK_QT_DIR "$ENV{RLINK_QT_DIR}" CACHE PATH
    "Qt 6.11+ msvc2022_64 kit directory (env RLINK_QT_DIR)")
set(RLINK_WEBRTC_SRC "$ENV{RLINK_WEBRTC_SRC}" CACHE PATH
    "WebRTC source checkout (env RLINK_WEBRTC_SRC)")
set(RLINK_WEBRTC_OUT "$ENV{RLINK_WEBRTC_OUT}" CACHE PATH
    "WebRTC GN out directory for Release builds (env RLINK_WEBRTC_OUT)")
set(RLINK_WEBRTC_OUT_DEBUG "$ENV{RLINK_WEBRTC_OUT_DEBUG}" CACHE PATH
    "WebRTC GN out directory for Debug builds (env RLINK_WEBRTC_OUT_DEBUG)")
set(RLINK_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/x64" CACHE PATH
    "Root directory for build outputs")

if(NOT RLINK_QT_DIR OR NOT RLINK_WEBRTC_SRC OR NOT RLINK_WEBRTC_OUT)
  message(FATAL_ERROR
    "Missing dependency locations.\n"
    "Set the environment variables RLINK_QT_DIR, RLINK_WEBRTC_SRC and "
    "RLINK_WEBRTC_OUT (see cmake/local.bat.example), then re-run configure. "
    "Alternatively pass -DRLINK_QT_DIR=... -DRLINK_WEBRTC_SRC=... "
    "-DRLINK_WEBRTC_OUT=... .")
endif()

# Debug WebRTC output defaults to <webrtc-src>/out/DebugMD when not provided.
if(NOT RLINK_WEBRTC_OUT_DEBUG)
  set(RLINK_WEBRTC_OUT_DEBUG "${RLINK_WEBRTC_SRC}/out/DebugMD" CACHE PATH
      "WebRTC GN out directory for Debug builds" FORCE)
endif()

# ---------------------------------------------------------------------------
# Language / runtime settings (C++20, /MD, UTF-8, SDL checks)
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
set(CMAKE_POSITION_INDEPENDENT_CODE OFF)

if(NOT WIN32)
  message(FATAL_ERROR
    "The CMake build currently targets Windows only. The source tree is being "
    "restructured for cross-platform support; non-Windows builds are not ready yet.")
endif()

# ---------------------------------------------------------------------------
# Qt 6.11+ (6.8 is too old for the NetworkAuth APIs used by RemoteAuth)
# ---------------------------------------------------------------------------
list(APPEND CMAKE_PREFIX_PATH "${RLINK_QT_DIR}")
find_package(Qt6 6.11 REQUIRED COMPONENTS
  Core Gui Widgets Network WebSockets NetworkAuth HttpServer Sql)

# Enables AUTOMOC / AUTOUIC / AUTORCC for every target (per project policy:
# any target touching Qt must have auto moc/uic/rcc enabled).
qt_standard_project_setup()

# ---------------------------------------------------------------------------
# Output layout: x64/<Config>/ so binaries, Qt runtime and FFmpeg DLLs sit in
# a predictable, Git-ignored location.
# ---------------------------------------------------------------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${RLINK_OUTPUT_DIR}/$<CONFIG>")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${RLINK_OUTPUT_DIR}/$<CONFIG>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${RLINK_OUTPUT_DIR}/$<CONFIG>")

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/RlinkCommon.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/RlinkWebRtc.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/RlinkFfmpeg.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/RlinkDeploy.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/targets.cmake")
