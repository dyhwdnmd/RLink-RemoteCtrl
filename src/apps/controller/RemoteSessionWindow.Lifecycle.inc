// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

    RemoteSessionWindow::RemoteSessionWindow(RemoteSessionBinding binding,
        app::InProcessSessionEngine* sessionEngine,
        QWidget* parent)
        : FramelessMainWindow(parent),
        remoteInputDispatcher_(
            std::make_shared<RemoteInputDispatcher>()),
        binding_(std::move(binding))
    {
        const QSettings settings;
        selectedFrameRate_ = kDefaultScreenFrameRate;
        const std::uint32_t configuredDragSampleRate =
            settings.value(
                QString::fromLatin1(
                    kDragPointerSampleRateSetting),
                QVariant::fromValue(240u)).toUInt();
        if (std::find(
                kSupportedDragPointerSampleRates.begin(),
                kSupportedDragPointerSampleRates.end(),
                configuredDragSampleRate) !=
            kSupportedDragPointerSampleRates.end()) {
            dragPointerSampleRateHz_ =
                configuredDragSampleRate;
        }
        const int savedQuality = settings.value(
            QString::fromLatin1(kRemoteScreenQualitySetting),
            static_cast<int>(ScreenQualityTier::kOriginal)).toInt();
        if (savedQuality >=
                static_cast<int>(ScreenQualityTier::kAutomatic) &&
            savedQuality <=
                static_cast<int>(ScreenQualityTier::kOriginal)) {
            selectedQuality_ =
                static_cast<ScreenQualityTier>(savedQuality);
        }
        BuildUi();
        BindSessionVideo(sessionEngine, binding_);
        sessionElapsed_.start();
        durationTimer_->start(1000);
    }

    RemoteSessionWindow::~RemoteSessionWindow()
    {
        if (releaseFileTransferHostHandler_) {
            releaseFileTransferHostHandler_();
        }
        if (remotePasteAnimation_) {
            remotePasteAnimation_->stop();
        }
        delete remotePasteAnimationOverlay_;
        remotePasteAnimationOverlay_ = nullptr;
        if (desktopCanvas_) {
            static_cast<RemoteDesktopCanvas*>(desktopCanvas_)
                ->ShutdownInputScheduler();
        }
        ReleaseRemoteInputs();
        remoteInputDispatcher_->Clear();
        BindSessionVideo(nullptr, binding_);
    }

    void RemoteSessionWindow::SetDisconnectHandler(
        std::function<void()> handler)
    {
        disconnectHandler_ = std::move(handler);
    }

    void RemoteSessionWindow::SetRemotePasteHandler(
        std::function<bool(const QStringList& localFiles,
                           bool keyboardPaste)> handler)
    {
        remotePasteHandler_ = std::move(handler);
    }

    void RemoteSessionWindow::SetRemotePasteCancelHandler(
        std::function<void()> handler)
    {
        remotePasteCancelHandler_ = std::move(handler);
    }

    void RemoteSessionWindow::SetFileTransferHandlers(
        std::function<void()> openHandler,
        std::function<void()> releaseHostHandler)
    {
        fileTransferHandler_ = std::move(openHandler);
        releaseFileTransferHostHandler_ =
            std::move(releaseHostHandler);
    }

    void RemoteSessionWindow::SetDragPointerSampleRate(
        std::uint32_t hertz)
    {
        if (std::find(
                kSupportedDragPointerSampleRates.begin(),
                kSupportedDragPointerSampleRates.end(),
                hertz) ==
            kSupportedDragPointerSampleRates.end()) {
            hertz = 240;
        }
        dragPointerSampleRateHz_ = hertz;
        if (desktopCanvas_) {
            static_cast<RemoteDesktopCanvas*>(desktopCanvas_)
                ->SetDragPointerSampleRate(hertz);
        }
    }

    void RemoteSessionWindow::SetRoomOnlineMemberCount(
        std::size_t onlineMemberCount)
    {
        const std::uint32_t maximumFrameRate =
            onlineMemberCount > kHighOccupancyRoomMemberThreshold
            ? kMultiMemberMaximumScreenFrameRate
            : kMaximumScreenFrameRate;
        if (roomMaximumFrameRate_ == maximumFrameRate) {
            return;
        }
        roomMaximumFrameRate_ = maximumFrameRate;
        const std::uint32_t previousSelection = selectedFrameRate_;
        RebuildFrameRateMenu();
        if (selectedFrameRate_ != previousSelection && sessionEngine_ &&
            binding_.IsRoom()) {
            (void)RequestStreamPreference(false);
        }
    }

    void RemoteSessionWindow::BindSessionVideo(
        app::InProcessSessionEngine* engine,
        RemoteSessionBinding binding)
    {
        if (sessionEngine_ == engine && binding_.SameTransport(binding) &&
            sessionVideoSinkBound_) {
            binding_ = std::move(binding);
            if (sessionSourceLabel_) {
                sessionSourceLabel_->setText(
                    QStringLiteral("%1 · %2")
                        .arg(binding_.peerDeviceId,
                             binding_.SourceText()));
            }
            return;
        }
        const bool bindingChanged =
            sessionEngine_ != engine || !binding_.SameTransport(binding);
        if (bindingChanged && sessionEngine_) {
            sessionEngine_->SetRemoteCursorCallback({});
        }
        if (bindingChanged) {
            pairWasActive_ = false;
            if (desktopCanvas_) {
                static_cast<RemoteDesktopCanvas*>(desktopCanvas_)
                    ->SetControlEnabled(false);
            }
            remoteInputDispatcher_->SetEnabled(false);
        }
        if (bindingChanged && sessionVideoSinkBound_ && sessionEngine_ &&
            desktopCanvas_) {
            if (binding_.IsDirect()) {
                (void)sessionEngine_->SetDirectRemoteVideoSink(nullptr);
            } else {
                (void)sessionEngine_->SetRoomRemoteVideoSink(
                    binding_.roomPairId.toStdString(),
                    kScreenMainVideoSlot, nullptr);
            }
        }

        sessionEngine_ = engine;
        binding_ = std::move(binding);
        if (sessionEngine_) {
            const QPointer<RemoteSessionWindow> self(this);
            sessionEngine_->SetRemoteCursorCallback(
                [self](const std::string& pairId,
                       const RemoteCursorEnvelope& envelope) {
                    if (!self) return;
                    QMetaObject::invokeMethod(
                        self,
                        [self, pairId, envelope] {
                            if (self) {
                                self->HandleRemoteCursorMessage(
                                    pairId, envelope);
                            }
                        },
                        Qt::QueuedConnection);
                });
        }
        if (sessionSourceLabel_) {
            sessionSourceLabel_->setText(
                QStringLiteral("%1 · %2")
                    .arg(binding_.peerDeviceId, binding_.SourceText()));
        }
        remoteInputDispatcher_->SetEngine(engine, binding_.IsDirect());
        if (bindingChanged) {
            sessionVideoSinkBound_ = false;
            sessionVideoSinkRetryScheduled_ = false;
            roomScreenPreferenceRetryScheduled_ = false;
            preferenceRequestedScreenShareEpoch_ = 0;
            preferenceSentScreenShareEpoch_ = 0;
            screenStartupGeneration_ = 0;
            screenStartupRefreshAttempts_ = 0;
            screenFirstFramePresented_ = false;
            remoteSourceWidth_ = 0;
            remoteSourceHeight_ = 0;
            RebuildQualityMenu();
            if (desktopCanvas_) {
                static_cast<RemoteDesktopCanvas*>(desktopCanvas_)
                    ->BeginPresentationGeneration(0);
                static_cast<RemoteDesktopCanvas*>(desktopCanvas_)
                    ->ResetRemoteCursor();
            }
            reportedRemoteMaximumFrameRate_ = kMaximumScreenFrameRate;
            RebuildFrameRateMenu();
        }
        if (!sessionEngine_ || !desktopCanvas_) {
            sessionVideoSinkBound_ = false;
            if (previewBadge_) {
                previewBadge_->setText(QStringLiteral("等待画面"));
            }
            RefreshControlState();
            return;
        }

        auto* sink = static_cast<RemoteDesktopCanvas*>(desktopCanvas_);
        if (binding_.IsDirect()) {
            const auto result =
                sessionEngine_->SetDirectRemoteVideoSink(sink);
            sessionVideoSinkBound_ = result.accepted;
            if (previewBadge_) {
                previewBadge_->setText(
                    result.accepted ? QStringLiteral("实时画面")
                                    : QStringLiteral("等待视频"));
            }
            if (!result.accepted && !sessionVideoSinkRetryScheduled_) {
                sessionVideoSinkRetryScheduled_ = true;
                const auto* expectedEngine = sessionEngine_;
                QTimer::singleShot(120, this,
                    [this, expectedEngine] {
                        sessionVideoSinkRetryScheduled_ = false;
                        if (sessionEngine_ == expectedEngine &&
                            binding_.IsDirect() &&
                            !sessionVideoSinkBound_) {
                            BindSessionVideo(sessionEngine_, binding_);
                        }
                    });
            }
            RefreshControlState();
            return;
        }

        const auto bindingSnapshot = sessionEngine_->Snapshot();
        if (bindingSnapshot.room.screenShareState ==
                RoomScreenShareState::kActive &&
            bindingSnapshot.room.screenShareEpoch != 0 &&
            bindingSnapshot.room.screenSharerDeviceId ==
                binding_.peerDeviceId.toStdString() &&
            screenStartupGeneration_ !=
                bindingSnapshot.room.screenShareEpoch) {
            BeginScreenStartup(
                bindingSnapshot.room.screenShareEpoch);
        }
        const auto result = sessionEngine_->SetRoomRemoteVideoSink(
            binding_.roomPairId.toStdString(), kScreenMainVideoSlot, sink);
        sessionVideoSinkBound_ = result.accepted;
        if (previewBadge_) {
            previewBadge_->setText(
                result.accepted ? QStringLiteral("实时画面")
                : QStringLiteral("等待视频"));
        }
        if (!result.accepted && !sessionVideoSinkRetryScheduled_) {
            sessionVideoSinkRetryScheduled_ = true;
            const auto* expectedEngine = sessionEngine_;
            const RemoteSessionBinding expectedBinding = binding_;
            QTimer::singleShot(120, this,
                [this, expectedEngine, expectedBinding] {
                    sessionVideoSinkRetryScheduled_ = false;
                    if (sessionEngine_ == expectedEngine &&
                        binding_.SameTransport(expectedBinding) &&
                        !sessionVideoSinkBound_) {
                        BindSessionVideo(sessionEngine_, binding_);
                    }
                });
        }
        RefreshControlState();
    }

    void RemoteSessionWindow::HandleRemoteCursorMessage(
        const std::string& pairId,
        const RemoteCursorEnvelope& envelope)
    {
        if (!desktopCanvas_ ||
            envelope.senderDeviceId !=
                binding_.peerDeviceId.toStdString()) {
            return;
        }
        if (binding_.IsDirect()) {
            if (!pairId.empty()) return;
        } else if (pairId != binding_.roomPairId.toStdString()) {
            return;
        }
        static_cast<RemoteDesktopCanvas*>(desktopCanvas_)
            ->ApplyRemoteCursor(envelope);
    }

    void RemoteSessionWindow::BeginScreenStartup(
        std::uint64_t screenShareGeneration)
    {
        if (screenShareGeneration == 0 || !desktopCanvas_) {
            return;
        }
        screenStartupGeneration_ = screenShareGeneration;
        // Frame-rate preference is scoped to one share generation. Start
        // every new share from the product default instead of carrying the
        // previous sharer's last acknowledged selection into this stream.
        selectedFrameRate_ = kDefaultScreenFrameRate;
        reportedRemoteMaximumFrameRate_ = kMaximumScreenFrameRate;
        RebuildFrameRateMenu();
        screenStartupRefreshAttempts_ = 0;
        screenFirstFramePresented_ = false;
        screenStartupElapsed_.restart();
        static_cast<RemoteDesktopCanvas*>(desktopCanvas_)
            ->BeginPresentationGeneration(screenShareGeneration);

        // Fast machines normally present before the first retry. Keep a few
        // bounded late retries for old hardware encoders whose first IDR or
        // driver initialization can take several seconds. Each retry is
        // generation-bound and stops as soon as a frame is presented.
        constexpr std::array<std::pair<int, std::uint32_t>, 5>
            kStartupRetries = {{{300, 1u}, {800, 2u}, {1500, 3u},
                                {3000, 4u}, {6000, 5u}}};
        for (const auto [delayMs, stage] : kStartupRetries) {
            QTimer::singleShot(delayMs, this,
                [this, screenShareGeneration, stage] {
                    RequestScreenStartupRefresh(
                        screenShareGeneration, stage);
                });
        }
    }

    void RemoteSessionWindow::RequestScreenStartupRefresh(
        std::uint64_t screenShareGeneration,
        std::uint32_t retryStage)
    {
        if (screenFirstFramePresented_ ||
            screenStartupGeneration_ != screenShareGeneration ||
            retryStage <= screenStartupRefreshAttempts_ ||
            !sessionEngine_ || !binding_.IsRoom() || !desktopCanvas_) {
            return;
        }
        const auto snapshot = sessionEngine_->Snapshot();
        if (snapshot.room.screenShareState !=
                RoomScreenShareState::kActive ||
            snapshot.room.screenShareEpoch != screenShareGeneration ||
            snapshot.room.screenSharerDeviceId !=
                binding_.peerDeviceId.toStdString()) {
            return;
        }

        screenStartupRefreshAttempts_ = retryStage;
        auto* sink = static_cast<RemoteDesktopCanvas*>(desktopCanvas_);
        // A bounded remove/add rebind makes libwebrtc issue a fresh PLI and
        // also sends the generation-bound type-10 full-frame request. It does
        // not rebuild ICE or the PeerConnection.
        (void)sessionEngine_->SetRoomRemoteVideoSink(
            binding_.roomPairId.toStdString(), kScreenMainVideoSlot, nullptr);
        const auto result = sessionEngine_->SetRoomRemoteVideoSink(
            binding_.roomPairId.toStdString(), kScreenMainVideoSlot, sink);
        sessionVideoSinkBound_ = result.accepted;
    }

    void RemoteSessionWindow::HandleFirstScreenPresentation(
        std::uint64_t screenShareGeneration)
    {
        if (screenFirstFramePresented_ ||
            screenStartupGeneration_ != screenShareGeneration ||
            !sessionEngine_ || !binding_.IsRoom()) {
            return;
        }
        screenFirstFramePresented_ = true;
        const qint64 elapsed = screenStartupElapsed_.isValid()
            ? screenStartupElapsed_.elapsed() : 0;
        const auto elapsedMs = static_cast<std::uint32_t>(
            std::clamp<qint64>(
                elapsed, 0,
                static_cast<qint64>(
                    std::numeric_limits<std::uint32_t>::max())));
        (void)sessionEngine_->NotifyRoomScreenFirstFramePresented(
            binding_.roomPairId.toStdString(),
            screenShareGeneration,
            elapsedMs);
    }
