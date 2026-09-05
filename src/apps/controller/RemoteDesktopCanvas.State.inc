// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

            QImage frameImage_;
            std::uint64_t frameImageSequence_ = 0;
            std::uint64_t lastPresentedCpuSequence_ = 0;
            InputSender inputSender_;
            PasteSender pasteSender_;
            FirstPresentationCallback firstPresentationCallback_;
            std::atomic<std::uint64_t> presentationGeneration_{ 0 };
            std::atomic<bool> firstPresentationNotified_{ false };
            std::uint64_t presentationTelemetryId_ = 0;
            bool controlEnabled_ = false;
            std::atomic_bool actualPixelDisplayMode_{ false };
            QPoint actualPixelPanOffset_;
            QPointF actualPixelPanStartPosition_;
            QPoint actualPixelPanStartOffset_;
            bool actualPixelPanning_ = false;
            bool suppressPasteKeyRelease_ = false;
            QString connectionStage_ =
                QStringLiteral("正在建立远程会话");
            std::unique_ptr<HighResolutionPointerMoveScheduler>
                pointerMoveScheduler_;
            std::uint32_t pointerMoveRateLimitHz_ = 120;
            std::uint32_t dragPointerSampleRateHz_ = 240;
            HWND pointerSamplingWindow_ = nullptr;
            std::atomic<int> pointerSourceWidth_{0};
            std::atomic<int> pointerSourceHeight_{0};
            std::atomic_bool pointerStretchToCanvas_{false};
            std::uint32_t remoteDisplayId_ = 0;
            std::uint64_t remoteDisplayLayoutVersion_ = 0;
            bool nativeRenderingEnabled_ = true;
            D3D11VideoSurface* nativeSurface_ = nullptr;
            std::unique_ptr<RemoteCursorRenderState> cursorRenderState_;
            QSize nativeFrameSize_;
            bool nativeFrameActive_ = false;
            std::atomic<std::uint64_t> receivedFrameSequence_{ 0 };
            std::mutex cpuFrameMutex_;
            std::condition_variable cpuFrameCondition_;
            webrtc::scoped_refptr<webrtc::VideoFrameBuffer> pendingCpuFrame_;
            std::uint64_t pendingCpuSequence_ = 0;
            std::atomic<std::uint32_t> cpuConversionIntervalUs_{ 16'667 };
            std::atomic<int> cpuCanvasWidth_{ 0 };
            std::atomic<int> cpuCanvasHeight_{ 0 };
            // QWidget geometry uses device-independent pixels. The CPU
            // renderer needs the screen DPR because it pre-scales frames
            // before handing them back to the UI thread.
            std::atomic<int> cpuCanvasDevicePixelRatioMilli_{ 1000 };
            std::jthread cpuConversionThread_;
            std::mutex cpuImageMutex_;
            QImage pendingCpuImage_;
            std::uint64_t pendingCpuImageSequence_ = 0;
            QImage recycledCpuImage_;
            bool cpuImageDispatchPending_ = false;
            std::mutex nativeFrameMutex_;
            std::condition_variable nativeFrameCondition_;
            webrtc::scoped_refptr<webrtc::VideoFrameBuffer> pendingNativeFrame_;
            std::uint64_t pendingNativeSequence_ = 0;
            QImage pendingD3D11CpuImage_;
            std::uint64_t pendingD3D11CpuSequence_ = 0;
            bool nativeRedrawRequested_ = false;
            bool nativeResetRequested_ = false;
            std::atomic<std::uint32_t> nativePresentationIntervalUs_{ 16'667 };
            std::jthread nativePresentationThread_;
            std::atomic<int> nativeFrameWidth_{ 0 };
            std::atomic<int> nativeFrameHeight_{ 0 };
            std::atomic<bool> nativeUiActiveRequested_{ false };
            std::atomic<bool> nativeSurfaceReady_{ false };
            std::atomic<bool> nativeUiDispatchPending_{ false };
            std::atomic<std::uint64_t> nativeUiRevision_{ 0 };
            std::atomic<bool> cpuD3D11PresentationFailed_{ false };
            std::atomic<bool> cpuNv12D3D11PresentationFailed_{ false };
            std::atomic<bool> cpuI420D3D11PresentationFailed_{ false };
        };
