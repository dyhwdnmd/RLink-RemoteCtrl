# Prebuilt Google WebRTC (built with GN). CMake consumes the artifacts; GN stays
# the build system for WebRTC itself.
#
# Release and Debug each need their own GN output tree (ReleaseMD uses /MD, DebugMD
# uses /MDd). The static libraries must have been produced with a toolchain whose
# STL/CRT matches the consuming configuration (v143 / MSVC 14.44.35207).

if(NOT EXISTS "${RLINK_WEBRTC_OUT}/obj/webrtc.lib")
  message(FATAL_ERROR
    "WebRTC Release static libraries were not found at:\n"
    "  ${RLINK_WEBRTC_OUT}/obj/webrtc.lib\n"
    "Build WebRTC first (see BUILDING.md) or point RLINK_WEBRTC_OUT at an existing "
    "GN output directory.")
endif()

if(NOT EXISTS "${RLINK_WEBRTC_OUT_DEBUG}/obj/webrtc.lib")
  message(WARNING
    "WebRTC Debug static libraries were not found at:\n"
    "  ${RLINK_WEBRTC_OUT_DEBUG}/obj/webrtc.lib\n"
    "Release builds are unaffected, but Debug builds will fail to link. Build "
    "WebRTC with is_debug = true or set RLINK_WEBRTC_OUT_DEBUG.")
endif()

# Select the GN output tree for the configuration being built.
set(_rtc_out "$<IF:$<CONFIG:Debug>,${RLINK_WEBRTC_OUT_DEBUG},${RLINK_WEBRTC_OUT}>")

add_library(rlink_webrtc INTERFACE)

target_include_directories(rlink_webrtc INTERFACE
  "${RLINK_WEBRTC_SRC}"
  "${RLINK_WEBRTC_SRC}/third_party/abseil-cpp"
  "${_rtc_out}/gen"
  "${RLINK_WEBRTC_SRC}/third_party/libyuv/include")

target_compile_definitions(rlink_webrtc INTERFACE
  WEBRTC_WIN RTC_ENABLE_WIN_WGC NOMINMAX WIN32_LEAN_AND_MEAN)

# Extra optimization switches used by the WebRTC-consuming projects.
# /wd4068 4146 4996 suppress warnings triggered by WebRTC headers (/sdl would
# otherwise promote them to errors).
if(MSVC)
  target_compile_options(rlink_webrtc INTERFACE /Zo /Gy /Oi /wd4068 /wd4146 /wd4996)
endif()

target_link_libraries(rlink_webrtc INTERFACE
  "${_rtc_out}/obj/webrtc.lib"
  "${_rtc_out}/obj/api/video/adapted_video_track_source.lib"
  "${_rtc_out}/obj/api/video_codecs/builtin_video_decoder_factory.lib"
  "${_rtc_out}/obj/api/video_codecs/builtin_video_encoder_factory.lib"
  "${_rtc_out}/obj/api/video_codecs/rtc_software_fallback_wrappers.lib"
  "${_rtc_out}/obj/media/rtc_internal_video_codecs.lib"
  "${_rtc_out}/obj/media/rtc_simulcast_encoder_adapter.lib"
  d3d11 d3d10 dxgi dwmapi shcore
  mf mfplat mfuuid
  ole32 ws2_32 winmm secur32 crypt32 bcrypt iphlpapi
  dmoguids msdmo wmcodecdspuuid strmiids)
