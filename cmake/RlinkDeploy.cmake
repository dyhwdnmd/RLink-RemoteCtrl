# Runtime deployment helpers (Windows).
#
# - Qt runtime + plugins via windeployqt.
# - FFmpeg runtime DLLs (replaces the ControllerApp PostBuildEvent copy).

function(rlink_deploy_qt target)
  if(NOT WIN32)
    return()
  endif()
  set(_windeployqt "${RLINK_QT_DIR}/bin/windeployqt.exe")
  if(NOT EXISTS "${_windeployqt}")
    message(WARNING "windeployqt not found at ${_windeployqt}; Qt runtime will not be copied for ${target}")
    return()
  endif()
  add_custom_command(TARGET ${target} POST_BUILD
    COMMAND "${_windeployqt}"
            "$<IF:$<CONFIG:Debug>,--debug,--release>"
            --no-translations
            --no-system-d3d-compiler
            --no-opengl-sw
            --no-compiler-runtime
            "$<TARGET_FILE:${target}>"
    COMMENT "Deploying Qt runtime for ${target}"
    VERBATIM)
endfunction()

function(rlink_copy_ffmpeg_runtime target)
  if(NOT WIN32)
    return()
  endif()
  set(_bin "${RLINK_FFMPEG_PREFIX}/bin")
  foreach(_dll avutil-60.dll avcodec-62.dll libwinpthread-1.dll libx264-165.dll libvpl.dll)
    add_custom_command(TARGET ${target} POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E copy_if_different
              "${_bin}/${_dll}" "$<TARGET_FILE_DIR:${target}>/${_dll}"
      COMMENT "Copying ${_dll}"
      VERBATIM)
  endforeach()
endfunction()

# License material bundled next to the app.
function(rlink_copy_licenses target)
  if(NOT WIN32)
    return()
  endif()
  add_custom_command(TARGET ${target} POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E make_directory "$<TARGET_FILE_DIR:${target}>/licenses/fonts"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "$<TARGET_FILE_DIR:${target}>/licenses/icons"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${CMAKE_SOURCE_DIR}/assets/fonts/licenses/Inter-OFL.txt"
            "$<TARGET_FILE_DIR:${target}>/licenses/fonts/Inter-OFL.txt"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${CMAKE_SOURCE_DIR}/assets/ui/icons/licenses/Lucide-LICENSE.txt"
            "$<TARGET_FILE_DIR:${target}>/licenses/icons/Lucide-LICENSE.txt"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${CMAKE_SOURCE_DIR}/assets/ui/icons/licenses/Morphicons-LICENSE.txt"
            "$<TARGET_FILE_DIR:${target}>/licenses/icons/Morphicons-LICENSE.txt"
    COMMENT "Copying license files for ${target}"
    VERBATIM)
endfunction()
