# FFmpeg (D3D11VA, shared) bundled under third_party. Only headers are used at
# compile time: ffmpeg is loaded dynamically at runtime, so there is no import
# library to link. The runtime DLLs are copied next to the binaries by
# rlink_copy_ffmpeg_runtime().

set(RLINK_FFMPEG_PREFIX
    "${CMAKE_SOURCE_DIR}/third_party/ffmpeg_d3d11va/prefix"
    CACHE PATH "FFmpeg d3d11va prefix (include/ + bin/)")

add_library(rlink_ffmpeg INTERFACE)
target_include_directories(rlink_ffmpeg INTERFACE "${RLINK_FFMPEG_PREFIX}/include")
