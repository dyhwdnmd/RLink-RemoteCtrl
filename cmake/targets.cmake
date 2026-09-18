# Target definitions for the RemoteC components and applications.
#
# Windows-only for now; platform-specific sources are grouped so a future
# cross-platform port can add e.g. src/platform/posix alongside src/platform/win.

set(_src "${CMAKE_SOURCE_DIR}/src")

# ---------------------------------------------------------------------------
# Static libraries
# ---------------------------------------------------------------------------

# RemoteCore: protocols + core session policy (no Qt, uses WebRTC headers).
add_library(rlink_core STATIC
  "${_src}/core/ScreenNetworkPolicy.cpp"
  "${_src}/core/ScreenStreamPolicy.cpp"
  "${_src}/core/SessionController.cpp"
  "${_src}/protocol/BinaryProtocol.cpp"
  "${_src}/protocol/ClipboardProtocol.cpp"
  "${_src}/protocol/DataChannelCatalog.cpp"
  "${_src}/protocol/FileTransferProtocol.cpp"
  "${_src}/protocol/RemoteInputProtocol.cpp"
  "${_src}/protocol/RemoteCursorProtocol.cpp"
  "${_src}/protocol/RoomMemberControlProtocol.cpp"
  "${_src}/protocol/ScreenShareControlProtocol.cpp")
rlink_apply_common(rlink_core)
target_link_libraries(rlink_core PUBLIC rlink_webrtc)

# RemoteAuth: OIDC/OAuth via Qt NetworkAuthorization.
add_library(rlink_auth STATIC
  "${_src}/auth/AuthConfig.cpp"
  "${_src}/auth/AuthManager.cpp"
  "${_src}/auth/DpapiTokenStore.cpp"
  "${_src}/auth/OidcDiscovery.cpp")
rlink_apply_common(rlink_auth)
set_target_properties(rlink_auth PROPERTIES AUTOMOC ON AUTOUIC ON AUTORCC ON)
target_link_libraries(rlink_auth PUBLIC
  Qt6::Core Qt6::Gui Qt6::Network Qt6::NetworkAuth)

# SignalingTransport: Qt WebSocket signaling client.
add_library(rlink_signaling STATIC
  "${_src}/signaling/SignalingJsonCodec.cpp"
  "${_src}/signaling/QtWebSocketSignalingClient.cpp")
rlink_apply_common(rlink_signaling)
set_target_properties(rlink_signaling PROPERTIES AUTOMOC ON AUTOUIC ON AUTORCC ON)
target_link_libraries(rlink_signaling PUBLIC
  Qt6::Core Qt6::Network Qt6::WebSockets)

# WebRtcTransport: libwebrtc session + Windows capture/encode backends.
add_library(rlink_webrtc_transport STATIC
  "${_src}/platform/win/D3D11DesktopFrameBuffer.cpp"
  "${_src}/platform/win/DesktopBgraFrameBuffer.cpp"
  "${_src}/platform/win/D3D11NativeFrameBuffer.cpp"
  "${_src}/platform/win/DxgiNativeDesktopCapturer.cpp"
  "${_src}/platform/win/WindowsDesktopCaptureSource.cpp"
  "${_src}/platform/win/WindowsDisplayTopology.cpp"
  "${_src}/platform/win/WindowsHardwareFingerprint.cpp"
  "${_src}/platform/win/WindowsCameraCaptureSource.cpp"
  "${_src}/platform/win/MfH264EncoderCapabilityProbe.cpp"
  "${_src}/platform/win/MfD3D11H264Encoder.cpp"
  "${_src}/platform/win/MfD3D11H264EncoderFactory.cpp"
  "${_src}/platform/win/MfH264EncoderSelfTest.cpp"
  "${_src}/platform/win/MfD3D11H264Decoder.cpp"
  "${_src}/platform/win/FfmpegD3D11H264Decoder.cpp"
  "${_src}/platform/win/FfmpegHardwareH264Encoder.cpp"
  "${_src}/platform/win/FfmpegHardwareH264EncoderFactory.cpp"
  "${_src}/platform/win/FfmpegX264H264Encoder.cpp"
  "${_src}/platform/win/FfmpegX264H264EncoderFactory.cpp"
  "${_src}/platform/win/QualityOpenH264Encoder.cpp"
  "${_src}/platform/win/H264EncoderBenchmark.cpp"
  "${_src}/platform/win/MfD3D11H264DecoderBenchmark.cpp"
  "${_src}/platform/win/MfD3D11H264DecoderFactory.cpp"
  "${_src}/webrtc/LibWebRtcSession.cpp"
  "${_src}/webrtc/PeerConnectionStatsCollector.cpp"
  "${_src}/webrtc/WebRtcRuntime.cpp"
  "${_src}/webrtc/WindowsPreferredVideoDecoderFactory.cpp"
  "${_src}/webrtc/WindowsPreferredVideoEncoderFactory.cpp")
rlink_apply_common(rlink_webrtc_transport)
target_link_libraries(rlink_webrtc_transport PUBLIC rlink_webrtc rlink_ffmpeg)

# RemoteSessionEngine: in-process session engine + Windows platform services.
add_library(rlink_session_engine STATIC
  "${_src}/apps/remote/FileTransferController.cpp"
  "${_src}/apps/remote/ClipboardCacheManager.cpp"
  "${_src}/apps/remote/ClipboardController.cpp"
  "${_src}/apps/remote/FileTransferStorage.cpp"
  "${_src}/apps/remote/InProcessSessionEngine.cpp"
  "${_src}/platform/win/WindowsFileTransferService.cpp"
  "${_src}/platform/win/WindowsClipboardService.cpp"
  "${_src}/platform/win/WindowsCursorMonitor.cpp")
rlink_apply_common(rlink_session_engine)
target_link_libraries(rlink_session_engine PUBLIC
  rlink_core rlink_webrtc_transport rlink_signaling)

# ---------------------------------------------------------------------------
# Applications
# ---------------------------------------------------------------------------

# RLinkAPP: Qt Widgets controller/agent client.
add_executable(RLinkAPP WIN32
  "${_src}/apps/remote/RemoteCProductMain.cpp"
  "${_src}/apps/remote/RemoteCApplicationCoordinator.cpp"
  "${_src}/apps/update/SoftwareUpdateController.cpp"
  "${_src}/apps/controller/LoginWindow.cpp"
  "${_src}/apps/controller/ui/RemoteCTheme.cpp"
  "${_src}/apps/controller/ui/morph/MorphIconCore.cpp"
  "${_src}/apps/controller/ui/morph/MorphIconButtonBinding.cpp"
  "${_src}/apps/controller/CameraWindow.cpp"
  "${_src}/apps/controller/ControllerMainWindow.cpp"
  "${_src}/apps/controller/ControllerMainWindow.Ui.cpp"
  "${_src}/apps/controller/ControllerMainWindow.Media.cpp"
  "${_src}/apps/controller/ControllerMainWindow.Diagnostics.cpp"
  "${_src}/apps/controller/ControllerMainWindow.Room.cpp"
  "${_src}/apps/controller/FileTransferWindow.cpp"
  "${_src}/apps/controller/FramelessWindow.cpp"
  "${_src}/apps/controller/RemoteCDialog.cpp"
  "${_src}/apps/controller/RemoteCComboBox.cpp"
  "${_src}/apps/controller/RemoteCToast.cpp"
  "${_src}/apps/controller/RoundedPopupMenu.cpp"
  "${_src}/apps/controller/RemoteSessionWindow.cpp"
  "${_src}/apps/controller/RoomCameraWindow.cpp"
  "${_src}/platform/win/WindowsInputExecutor.cpp"
  "${_src}/apps/controller/RemoteCResources.qrc"
  "${CMAKE_SOURCE_DIR}/assets/branding/RemoteCApp.rc"
  "${_src}/apps/remote/RemoteCApp.manifest")
rlink_apply_common(RLinkAPP)
set_target_properties(RLinkAPP PROPERTIES
  AUTOMOC ON AUTOUIC ON AUTORCC ON
  INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
target_link_libraries(RLinkAPP PRIVATE
  rlink_core rlink_webrtc_transport rlink_signaling rlink_session_engine rlink_auth
  rlink_webrtc rlink_ffmpeg
  Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network Qt6::WebSockets Qt6::NetworkAuth
  d3dcompiler)
target_link_options(RLinkAPP PRIVATE
  "/ENTRY:mainCRTStartup"
  "/MAP:$<TARGET_FILE_DIR:RLinkAPP>/RLinkAPP.map")
rlink_deploy_qt(RLinkAPP)
rlink_copy_ffmpeg_runtime(RLinkAPP)
rlink_copy_licenses(RLinkAPP)

# RemoteCSignalServer: Qt HTTP/WebSocket signaling server (console).
add_executable(RemoteCSignalServer
  "${_src}/server/auth/LogtoUserInfoClient.cpp"
  "${_src}/server/auth/LogtoManagementClient.cpp"
  "${_src}/server/auth/LogtoWebhookServer.cpp"
  "${_src}/server/persistence/IdentityStore.cpp"
  "${_src}/server/signaling/AccessTokenService.cpp"
  "${_src}/server/signaling/SignalServer.cpp"
  "${_src}/server/signaling/SignalServerSupport.cpp"
  "${_src}/server/signaling/SignalServerMain.cpp")
rlink_apply_common(RemoteCSignalServer)
set_target_properties(RemoteCSignalServer PROPERTIES
  AUTOMOC ON AUTOUIC ON AUTORCC ON
  INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
target_link_libraries(RemoteCSignalServer PRIVATE
  Qt6::Core Qt6::Network Qt6::WebSockets Qt6::HttpServer Qt6::Sql)
rlink_deploy_qt(RemoteCSignalServer)

# RLinkUpdater: standalone self-contained updater (no Qt, static CRT /MT).
add_executable(RLinkUpdater WIN32
  "${_src}/apps/update/RLinkUpdaterMain.cpp"
  "${_src}/apps/update/RLinkUpdater.manifest")
set_target_properties(RLinkUpdater PROPERTIES
  MSVC_RUNTIME_LIBRARY "MultiThreaded"
  INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
if(MSVC)
  target_compile_options(RLinkUpdater PRIVATE /utf-8 /Zc:__cplusplus /W4 /permissive- /MP)
  target_compile_definitions(RLinkUpdater PRIVATE
    WIN32 _WINDOWS UNICODE _UNICODE NOMINMAX)
endif()
target_link_libraries(RLinkUpdater PRIVATE bcrypt dwmapi winhttp)
