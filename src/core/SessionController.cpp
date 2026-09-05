// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "SessionController.h"

#include <algorithm>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <optional>
#include <queue>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace remote {
namespace {

class SerialExecutor final {
public:
    SerialExecutor() : thread_([this] { Run(); }) {}
    ~SerialExecutor() { Stop(); }

    SerialExecutor(const SerialExecutor&) = delete;
    SerialExecutor& operator=(const SerialExecutor&) = delete;

    bool Post(std::function<void()> task)
    {
        return PostAt(std::chrono::steady_clock::now(), std::move(task));
    }

    bool PostAfter(std::chrono::milliseconds delay,
                   std::function<void()> task)
    {
        return PostAt(std::chrono::steady_clock::now() + delay,
                      std::move(task));
    }

    void Invoke(std::function<void()> task)
    {
        if (std::this_thread::get_id() == thread_.get_id()) {
            task();
            return;
        }

        auto done = std::make_shared<std::promise<void>>();
        auto completed = done->get_future();
        if (!Post([task = std::move(task), done] {
                task();
                done->set_value();
            })) {
            return;
        }
        completed.wait();
    }

    void Stop()
    {
        {
            std::lock_guard lock(mutex_);
            if (stopping_) {
                return;
            }
            stopping_ = true;
            while (!tasks_.empty()) {
                tasks_.pop();
            }
        }
        condition_.notify_all();
        if (thread_.joinable() &&
            std::this_thread::get_id() != thread_.get_id()) {
            thread_.join();
        }
    }

private:
    struct WorkItem {
        std::chrono::steady_clock::time_point due;
        std::uint64_t sequence = 0;
        std::function<void()> task;
    };

    struct WorkItemLater {
        bool operator()(const WorkItem& left, const WorkItem& right) const
        {
            if (left.due != right.due) {
                return left.due > right.due;
            }
            return left.sequence > right.sequence;
        }
    };

    bool PostAt(std::chrono::steady_clock::time_point due,
                std::function<void()> task)
    {
        {
            std::lock_guard lock(mutex_);
            if (stopping_) {
                return false;
            }
            tasks_.push({due, nextSequence_++, std::move(task)});
        }
        condition_.notify_all();
        return true;
    }

    void Run()
    {
        std::unique_lock lock(mutex_);
        for (;;) {
            if (stopping_) {
                return;
            }
            if (tasks_.empty()) {
                condition_.wait(lock,
                                [this] { return stopping_ || !tasks_.empty(); });
                continue;
            }

            const auto due = tasks_.top().due;
            if (due > std::chrono::steady_clock::now()) {
                condition_.wait_until(lock, due);
                continue;
            }

            auto task = std::move(tasks_.top().task);
            tasks_.pop();
            lock.unlock();
            task();
            lock.lock();
        }
    }

    std::mutex mutex_;
    std::condition_variable condition_;
    std::priority_queue<WorkItem,
                        std::vector<WorkItem>,
                        WorkItemLater>
        tasks_;
    std::thread thread_;
    std::uint64_t nextSequence_ = 1;
    bool stopping_ = false;
};

std::string CandidateKey(const IceCandidate& candidate)
{
    return candidate.sdpMid + "\n" +
           std::to_string(candidate.sdpMLineIndex) + "\n" +
           candidate.candidate;
}

}  // namespace

class SessionControllerBase::Impl final {
public:
    Impl(SessionControllerBase* owner,
         Role role,
         IWebRtcSession& session,
         ISessionSignalingSender& signaling)
        : owner_(owner),
          role_(role),
          session_(session),
          signaling_(signaling)
    {}

    ~Impl() { Shutdown(); }

    void SetObserver(ISessionControllerObserver* observer)
    {
        std::lock_guard lock(observerMutex_);
        observer_ = observer;
    }

    void Start(SessionControllerConfig config)
    {
        executor_.Post([this, config = std::move(config)]() mutable {
            StartOnExecutor(std::move(config));
        });
    }

    void BeginNegotiation(std::vector<DataChannelSpec> dataChannels)
    {
        executor_.Post(
            [this, dataChannels = std::move(dataChannels)]() mutable {
                BeginNegotiationOnExecutor(std::move(dataChannels));
            });
    }

    void SetAgentAnswerPreparation(
        std::function<std::optional<OperationError>()> preparation)
    {
        executor_.Invoke([this, preparation = std::move(preparation)]() mutable {
            if (role_ != Role::kAgent) {
                return;
            }
            answerPreparation_ = std::move(preparation);
        });
    }

    void HandleRemoteDescription(SessionDescription description)
    {
        executor_.Post([this, description = std::move(description)]() mutable {
            HandleRemoteDescriptionOnExecutor(std::move(description));
        });
    }

    void HandleRemoteIceCandidate(IceCandidate candidate)
    {
        executor_.Post([this, candidate = std::move(candidate)]() mutable {
            HandleRemoteIceCandidateOnExecutor(std::move(candidate));
        });
    }

    void HandleRemoteIceRestartRequest(std::uint64_t observedGeneration,
                                       std::uint64_t requestSequence)
    {
        executor_.Post([this, observedGeneration, requestSequence] {
            // The answerer can only request a restart for the generation it
            // is currently observing.  Accepting an older generation here
            // used to start a second restart after the first one had already
            // recovered, which eventually exhausted the retry budget.
            if (role_ != Role::kController || closing_ ||
                observedGeneration != signalingGeneration_ ||
                requestSequence == 0 ||
                requestSequence <= lastRemoteRestartRequestSequence_ ||
                restartScheduled_) {
                return;
            }
            lastRemoteRestartRequestSequence_ = requestSequence;
            pendingRemoteRestartGeneration_ = observedGeneration;
            pendingRemoteRestartRequestSequence_ = requestSequence;
            const std::uint64_t token = ++remoteRestartRequestToken_;
            executor_.PostAfter(
                config_.remoteIceRestartRequestDebounce,
                [this, observedGeneration, requestSequence, token] {
                    if (closing_ || token != remoteRestartRequestToken_ ||
                        restartScheduled_ ||
                        observedGeneration != signalingGeneration_ ||
                        observedGeneration != pendingRemoteRestartGeneration_ ||
                        requestSequence !=
                            pendingRemoteRestartRequestSequence_) {
                        return;
                    }
                    ClearPendingRemoteIceRestartRequestOnExecutor();
                    BeginIceRestartOnExecutor();
                });
        });
    }

    void HandleRemoteIceRestartCancel(std::uint64_t observedGeneration,
                                      std::uint64_t requestSequence)
    {
        executor_.Post([this, observedGeneration, requestSequence] {
            if (role_ != Role::kController || closing_ ||
                observedGeneration != pendingRemoteRestartGeneration_ ||
                requestSequence != pendingRemoteRestartRequestSequence_) {
                return;
            }
            ClearPendingRemoteIceRestartRequestOnExecutor();
        });
    }

    void SetSignalingAvailable(bool available)
    {
        executor_.Post([this, available] {
            signalingAvailable_ = available;
            if (!available) {
                restartScheduled_ = false;
                if (recovering_) {
                    PublishState(SessionControllerState::kWaitingForSignaling);
                }
                return;
            }
            if (role_ == Role::kAgent &&
                activeRestartRequestSequence_ != 0 &&
                Snapshot().webRtcState == WebRtcSessionState::kConnected) {
                CancelOutstandingIceRestartRequestOnExecutor();
            }
            if (recovering_) {
                BeginOrRequestIceRestartOnExecutor();
            }
        });
    }

    void Close()
    {
        executor_.Post([this] { CloseOnExecutor(); });
    }

    SendResult SendData(std::string channelName,
                        std::vector<std::uint8_t> payload,
                        bool binary)
    {
        SendResult result = SendResult::kSessionNotStarted;
        executor_.Invoke([this, &result,
                          channelName = std::move(channelName),
                          payload = std::move(payload), binary] {
            result = session_.SendData(channelName, payload, binary);
        });
        return result;
    }

    bool QueueData(std::string channelName,
                   std::vector<std::uint8_t> payload,
                   bool binary,
                   std::function<void(SendResult)> completion)
    {
        return executor_.Post(
            [this,
             channelName = std::move(channelName),
             payload = std::move(payload),
             binary,
             completion = std::move(completion)] {
                const SendResult result =
                    session_.SendData(channelName, payload, binary);
                if (completion) {
                    completion(result);
                }
            });
    }

    SessionControllerSnapshot Snapshot() const
    {
        std::lock_guard lock(snapshotMutex_);
        return snapshot_;
    }

    void OnSessionStateChanged(WebRtcSessionState state)
    {
        executor_.Post([this, state] {
            {
                std::lock_guard lock(snapshotMutex_);
                snapshot_.webRtcState = state;
            }
            switch (state) {
            case WebRtcSessionState::kConnected:
                if (role_ == Role::kAgent) {
                    // Cancel immediately instead of waiting for the stability
                    // window, so a queued request cannot restart a connection
                    // that has already resumed media flow.
                    restartScheduled_ = false;
                    CancelOutstandingIceRestartRequestOnExecutor();
                }
                if (recovering_) {
                    const std::uint64_t generation = ++timerGeneration_;
                    executor_.PostAfter(
                        config_.connectedStabilityPeriod,
                        [this, generation] {
                            if (generation != timerGeneration_ || closing_ ||
                                !recovering_ ||
                                Snapshot().webRtcState !=
                                    WebRtcSessionState::kConnected) {
                                return;
                            }
                            CompleteConnectedOnExecutor();
                        });
                } else {
                    CompleteConnectedOnExecutor();
                }
                break;
            case WebRtcSessionState::kDisconnected:
                if (!recovering_) {
                    recovering_ = true;
                    recoveryStarted_ = std::chrono::steady_clock::now();
                    ArmReconnectTimeout();
                }
                PublishState(SessionControllerState::kDisconnected);
                ArmDisconnectedGracePeriod();
                break;
            case WebRtcSessionState::kFailed:
                ++timerGeneration_;
                if (!recovering_) {
                    recovering_ = true;
                    recoveryStarted_ = std::chrono::steady_clock::now();
                    ArmReconnectTimeout();
                }
                BeginOrRequestIceRestartOnExecutor();
                break;
            case WebRtcSessionState::kClosed:
                PublishState(SessionControllerState::kClosed);
                break;
            default:
                break;
            }
        });
    }

    void OnIceGatheringStateChanged(WebRtcIceGatheringState)
    {}

    void OnLocalDescription(SessionDescription description)
    {
        executor_.Post([this, description = std::move(description)] {
            auto outgoing = description;
            outgoing.negotiationGeneration = signalingGeneration_;
            if (!signaling_.SendDescription(outgoing)) {
                if (recovering_) {
                    signalingAvailable_ = false;
                    restartScheduled_ = false;
                    PublishState(SessionControllerState::kWaitingForSignaling);
                    return;
                }
                Fail("signaling_description_send_failed",
                     "Failed to send the local SDP through signaling.");
            }
        });
    }

    void OnLocalIceCandidate(IceCandidate candidate)
    {
        executor_.Post([this, candidate = std::move(candidate)] {
            auto outgoing = candidate;
            outgoing.negotiationGeneration = signalingGeneration_;
            if (!signaling_.SendIceCandidate(outgoing)) {
                if (recovering_) {
                    signalingAvailable_ = false;
                    restartScheduled_ = false;
                    PublishState(SessionControllerState::kWaitingForSignaling);
                    return;
                }
                Fail("signaling_candidate_send_failed",
                     "Failed to send a local ICE candidate through signaling.");
            }
        });
    }

    void OnDataChannelStateChanged(DataChannelInfo channel)
    {
        executor_.Post([this, channel = std::move(channel)] {
            if (auto* observer = Observer()) {
                observer->OnDataChannelStateChanged(channel);
            }
        });
    }

    void OnDataMessage(std::string label,
                       std::vector<std::uint8_t> payload,
                       bool binary)
    {
        executor_.Post([this, label = std::move(label),
                        payload = std::move(payload), binary] {
            if (auto* observer = Observer()) {
                observer->OnDataMessage(label, payload, binary);
            }
        });
    }

    void OnRemoteTrackAdded(RemoteTrackInfo track)
    {
        executor_.Post([this, track = std::move(track)] {
            if (auto* observer = Observer()) {
                observer->OnRemoteTrackAdded(track);
            }
        });
    }

    void OnOperationCompleted(OperationId operationId)
    {
        executor_.Post(
            [this, operationId] { OnOperationCompletedOnExecutor(operationId); });
    }

    void OnWebRtcError(OperationId operationId, OperationError error)
    {
        executor_.Post([this, operationId, error = std::move(error)] {
            if (closing_ || operationId == 0) {
                if (operationId == 0 && !closing_) {
                    Fail(error.code, error.message);
                }
                return;
            }
            const bool candidateOperation =
                remoteCandidateOperationIds_.erase(operationId) != 0;
            if (recovering_) {
                if (Snapshot().webRtcState == WebRtcSessionState::kConnected) {
                    return;
                }
                // A single trickle candidate may legitimately become stale
                // during a network switch. Other candidates and the selected
                // pair can still recover, so it must not consume a complete
                // ICE restart attempt.
                if (candidateOperation) {
                    return;
                }
                const bool currentNegotiationOperation =
                    operationId == createOfferOperation_ ||
                    operationId == createAnswerOperation_ ||
                    operationId == applyRemoteDescriptionOperation_;
                if (!currentNegotiationOperation) {
                    return;
                }
                restartScheduled_ = false;
                if (role_ == Role::kAgent) {
                    CancelOutstandingIceRestartRequestOnExecutor();
                }
                if (restartAttempt_ >= config_.maximumIceRestartAttempts) {
                    Fail("ice_restart_exhausted",
                         error.message.empty()
                             ? "ICE restart failed after the configured number of attempts."
                             : error.message);
                    return;
                }
                executor_.PostAfter(std::chrono::milliseconds(2000), [this] {
                    if (!closing_ && recovering_ &&
                        Snapshot().webRtcState !=
                            WebRtcSessionState::kConnected) {
                        BeginOrRequestIceRestartOnExecutor();
                    }
                });
                return;
            }
            Fail(error.code, error.message);
        });
    }

    void Shutdown()
    {
        if (shutdown_) {
            return;
        }
        shutdown_ = true;
        executor_.Invoke([this] {
            ++timerGeneration_;
            ++recoveryGeneration_;
            session_.Close();
            session_.SetObserver(nullptr);
            std::lock_guard lock(observerMutex_);
            observer_ = nullptr;
        });
        executor_.Stop();
    }

private:
    void StartOnExecutor(SessionControllerConfig config)
    {
        if (Snapshot().state != SessionControllerState::kIdle) {
            Fail("controller_already_started",
                 "A session controller can only be started once.");
            return;
        }
        config_ = std::move(config);
        session_.SetObserver(owner_);
        PublishState(SessionControllerState::kStarting);
        startOperation_ = session_.Start(config_.webRtc);
    }

    void BeginNegotiationOnExecutor(
        std::vector<DataChannelSpec> dataChannels)
    {
        if (role_ != Role::kController) {
            Fail("invalid_offer_role",
                 "Only ControllerSessionController may create the initial offer.");
            return;
        }
        if (Snapshot().state != SessionControllerState::kReady) {
            Fail("controller_not_ready",
                 "The controller must be Ready before negotiation starts.");
            return;
        }
        PublishState(SessionControllerState::kNegotiating);
        ArmNegotiationTimeout();
        createDataChannelsOperation_ =
            session_.CreateDataChannels(dataChannels);
    }

    void HandleRemoteDescriptionOnExecutor(SessionDescription description)
    {
        const SessionDescriptionType expected =
            role_ == Role::kController ? SessionDescriptionType::kAnswer
                                       : SessionDescriptionType::kOffer;
        if (description.type != expected) {
            Fail("unexpected_remote_sdp_type",
                 "The remote SDP type does not match the fixed V1 role.");
            return;
        }

        if (description.negotiationGeneration < signalingGeneration_) {
            return;
        }
        if (description.negotiationGeneration > signalingGeneration_) {
            if (role_ != Role::kAgent ||
                description.type != SessionDescriptionType::kOffer) {
                Fail("unexpected_negotiation_generation",
                     "Only the fixed offerer may start a newer negotiation generation.");
                return;
            }
            signalingGeneration_ = description.negotiationGeneration;
            recovering_ = true;
            restartScheduled_ = true;
            activeRestartRequestSequence_ = 0;
            activeRestartRequestGeneration_ = 0;
            if (restartAttempt_ == 0) {
                restartAttempt_ = 1;
            }
            ResetRemoteNegotiationContext();
            PublishState(SessionControllerState::kRestartingIce);
            UpdateRecoverySnapshot();
        }

        if (remoteDescriptionSdp_ == description.sdp) {
            return;
        }
        if (!remoteDescriptionSdp_.empty() ||
            applyRemoteDescriptionOperation_ != 0) {
            Fail("conflicting_remote_sdp",
                 "A different remote SDP was received for this negotiation.");
            return;
        }

        const auto state = Snapshot().state;
        if (state == SessionControllerState::kIdle ||
            state == SessionControllerState::kStarting) {
            pendingRemoteDescription_ = std::move(description);
            return;
        }
        if (state != SessionControllerState::kReady &&
            state != SessionControllerState::kNegotiating &&
            state != SessionControllerState::kConnected &&
            state != SessionControllerState::kDisconnected &&
            state != SessionControllerState::kWaitingForSignaling &&
            state != SessionControllerState::kRestartingIce) {
            Fail("remote_sdp_in_invalid_state",
                 "Remote SDP arrived outside a negotiable state.");
            return;
        }

        if (role_ == Role::kAgent) {
            if (state == SessionControllerState::kReady) {
                PublishState(SessionControllerState::kNegotiating);
                ArmNegotiationTimeout();
            } else if (recovering_) {
                PublishState(SessionControllerState::kRestartingIce);
                ArmIceRestartTimeout();
            }
        }
        remoteDescriptionSdp_ = description.sdp;
        remoteDescriptionType_ = description.type;
        applyRemoteDescriptionOperation_ =
            session_.ApplyRemoteDescription(description);
    }

    void HandleRemoteIceCandidateOnExecutor(IceCandidate candidate)
    {
        if (candidate.negotiationGeneration < signalingGeneration_) {
            return;
        }
        if (candidate.negotiationGeneration > signalingGeneration_) {
            futureRemoteCandidates_[candidate.negotiationGeneration]
                .push_back(std::move(candidate));
            return;
        }
        if (!remoteCandidateKeys_.insert(CandidateKey(candidate)).second) {
            return;
        }
        if (!remoteDescriptionApplied_) {
            pendingRemoteCandidates_.push_back(std::move(candidate));
            return;
        }
        AddRemoteIceCandidateOnExecutor(candidate);
    }

    void AddRemoteIceCandidateOnExecutor(const IceCandidate& candidate)
    {
        const OperationId operation =
            session_.AddRemoteIceCandidate(candidate);
        if (operation != 0) {
            remoteCandidateOperationIds_.insert(operation);
        }
    }

    void OnOperationCompletedOnExecutor(OperationId operationId)
    {
        if (remoteCandidateOperationIds_.erase(operationId) != 0) {
            return;
        }
        if (operationId == startOperation_) {
            PublishState(SessionControllerState::kReady);
            if (pendingRemoteDescription_) {
                auto description = std::move(*pendingRemoteDescription_);
                pendingRemoteDescription_.reset();
                HandleRemoteDescriptionOnExecutor(std::move(description));
            }
            return;
        }

        if (operationId == createDataChannelsOperation_) {
            createOfferOperation_ = session_.CreateOffer();
            return;
        }

        if (operationId == applyRemoteDescriptionOperation_) {
            remoteDescriptionApplied_ = true;
            for (const auto& candidate : pendingRemoteCandidates_) {
                AddRemoteIceCandidateOnExecutor(candidate);
            }
            pendingRemoteCandidates_.clear();
            if (role_ == Role::kAgent &&
                remoteDescriptionType_ == SessionDescriptionType::kOffer) {
                if (signalingGeneration_ == 1 && answerPreparation_) {
                    if (auto error = answerPreparation_()) {
                        Fail(std::move(error->code),
                             std::move(error->message));
                        return;
                    }
                }
                createAnswerOperation_ = session_.CreateAnswer();
            }
            return;
        }

        // CreateOffer/CreateAnswer and AddRemoteIceCandidate completion need
        // no extra transition; SDP and connection callbacks drive progress.
    }

    void ResetRemoteNegotiationContext()
    {
        applyRemoteDescriptionOperation_ = 0;
        createOfferOperation_ = 0;
        createAnswerOperation_ = 0;
        remoteDescriptionSdp_.clear();
        remoteDescriptionApplied_ = false;
        pendingRemoteDescription_.reset();
        pendingRemoteCandidates_.clear();
        remoteCandidateOperationIds_.clear();
        remoteCandidateKeys_.clear();
        const auto future = futureRemoteCandidates_.find(signalingGeneration_);
        if (future != futureRemoteCandidates_.end()) {
            pendingRemoteCandidates_ = std::move(future->second);
            futureRemoteCandidates_.erase(future);
        }
        std::erase_if(futureRemoteCandidates_, [this](const auto& entry) {
            return entry.first < signalingGeneration_;
        });
    }

    void CompleteConnectedOnExecutor()
    {
        if (role_ == Role::kAgent) {
            CancelOutstandingIceRestartRequestOnExecutor();
        } else {
            ClearPendingRemoteIceRestartRequestOnExecutor();
        }
        ++timerGeneration_;
        ++recoveryGeneration_;
        recovering_ = false;
        restartScheduled_ = false;
        restartAttempt_ = 0;
        recoveryStarted_ = {};
        {
            std::lock_guard lock(snapshotMutex_);
            snapshot_.webRtcState = WebRtcSessionState::kConnected;
        }
        UpdateRecoverySnapshot();
        PublishState(SessionControllerState::kConnected);
    }

    void UpdateRecoverySnapshot()
    {
        SessionControllerSnapshot snapshot;
        {
            std::lock_guard lock(snapshotMutex_);
            snapshot_.negotiationGeneration = signalingGeneration_;
            snapshot_.iceRestartAttempt = restartAttempt_;
            snapshot = snapshot_;
        }
        if (auto* observer = Observer()) {
            observer->OnControllerSnapshot(snapshot);
        }
    }

    void BeginOrRequestIceRestartOnExecutor()
    {
        if (closing_ || restartScheduled_) {
            return;
        }
        if (Snapshot().webRtcState == WebRtcSessionState::kConnected) {
            return;
        }
        recovering_ = true;
        if (recoveryStarted_ == std::chrono::steady_clock::time_point{}) {
            recoveryStarted_ = std::chrono::steady_clock::now();
            ArmReconnectTimeout();
        }
        if (!signalingAvailable_) {
            PublishState(SessionControllerState::kWaitingForSignaling);
            return;
        }
        if (restartAttempt_ >= config_.maximumIceRestartAttempts) {
            Fail("ice_restart_exhausted",
                 "ICE restart failed after the configured number of attempts.");
            return;
        }
        if (role_ == Role::kController) {
            BeginIceRestartOnExecutor();
            return;
        }
        restartScheduled_ = true;
        if (activeRestartRequestSequence_ == 0 ||
            activeRestartRequestGeneration_ != signalingGeneration_) {
            ++restartAttempt_;
            activeRestartRequestGeneration_ = signalingGeneration_;
            activeRestartRequestSequence_ = ++nextRestartRequestSequence_;
        }
        UpdateRecoverySnapshot();
        if (!signaling_.RequestIceRestart(
                activeRestartRequestGeneration_,
                activeRestartRequestSequence_)) {
            signalingAvailable_ = false;
            restartScheduled_ = false;
            PublishState(SessionControllerState::kWaitingForSignaling);
            return;
        }
        PublishState(SessionControllerState::kWaitingForSignaling);
        ArmIceRestartTimeout();
    }

    void BeginIceRestartOnExecutor()
    {
        if (closing_ || restartScheduled_) {
            return;
        }
        if (!recovering_) {
            recovering_ = true;
            recoveryStarted_ = std::chrono::steady_clock::now();
            ArmReconnectTimeout();
        }
        if (!signalingAvailable_) {
            restartScheduled_ = false;
            PublishState(SessionControllerState::kWaitingForSignaling);
            return;
        }
        if (restartAttempt_ >= config_.maximumIceRestartAttempts) {
            Fail("ice_restart_exhausted",
                 "ICE restart failed after the configured number of attempts.");
            return;
        }
        restartScheduled_ = true;
        ClearPendingRemoteIceRestartRequestOnExecutor();
        ++restartAttempt_;
        ++signalingGeneration_;
        ResetRemoteNegotiationContext();
        UpdateRecoverySnapshot();
        PublishState(SessionControllerState::kRestartingIce);
        createOfferOperation_ = session_.CreateIceRestartOffer();
        ArmIceRestartTimeout();
    }

    void ArmDisconnectedGracePeriod()
    {
        const std::uint64_t generation = ++timerGeneration_;
        executor_.PostAfter(config_.disconnectedGracePeriod,
            [this, generation] {
                if (generation != timerGeneration_ || closing_ ||
                    !recovering_ || Snapshot().webRtcState ==
                        WebRtcSessionState::kConnected) {
                    return;
                }
                BeginOrRequestIceRestartOnExecutor();
            });
    }

    void ArmIceRestartTimeout()
    {
        const std::uint64_t generation = signalingGeneration_;
        const std::uint32_t attempt = restartAttempt_;
        executor_.PostAfter(config_.iceRestartTimeout,
            [this, generation, attempt] {
                if (closing_ || !recovering_ ||
                    generation != signalingGeneration_ ||
                    attempt != restartAttempt_) {
                    return;
                }
                if (Snapshot().webRtcState == WebRtcSessionState::kConnected) {
                    return;
                }
                const std::int64_t previousStatsTimestamp =
                    session_.StatsSnapshot().transport.timestampMs;
                session_.RequestStats();
                VerifyTransportBeforeIceRestartTimeoutOnExecutor(
                    generation,
                    attempt,
                    previousStatsTimestamp,
                    std::chrono::steady_clock::now() +
                        config_.transportVerificationTimeout);
            });
    }

    void VerifyTransportBeforeIceRestartTimeoutOnExecutor(
        std::uint64_t generation,
        std::uint32_t attempt,
        std::int64_t previousStatsTimestamp,
        std::chrono::steady_clock::time_point deadline)
    {
        executor_.PostAfter(
            config_.transportVerificationDelay,
            [this, generation, attempt, previousStatsTimestamp, deadline] {
                if (closing_ || !recovering_ ||
                    generation != signalingGeneration_ ||
                    attempt != restartAttempt_ ||
                    Snapshot().webRtcState ==
                        WebRtcSessionState::kConnected) {
                    return;
                }
                const auto stats = session_.StatsSnapshot();
                const auto& transport = stats.transport;
                const bool fresh = transport.collected &&
                    transport.timestampMs > previousStatsTimestamp;
                const bool iceConnected =
                    transport.iceState == "connected" ||
                    transport.iceState == "completed";
                const bool dtlsConnected =
                    transport.dtlsState.empty() ||
                    transport.dtlsState == "connected";
                const bool selectedPairUsable =
                    transport.candidatePairState == "succeeded" &&
                    transport.writable;
                if (fresh && iceConnected && dtlsConnected &&
                    selectedPairUsable) {
                    CompleteConnectedOnExecutor();
                    return;
                }
                if (std::chrono::steady_clock::now() < deadline) {
                    session_.RequestStats();
                    VerifyTransportBeforeIceRestartTimeoutOnExecutor(
                        generation,
                        attempt,
                        previousStatsTimestamp,
                        deadline);
                    return;
                }
                HandleIceRestartTimeoutOnExecutor();
            });
    }

    void HandleIceRestartTimeoutOnExecutor()
    {
        restartScheduled_ = false;
        if (role_ == Role::kAgent) {
            CancelOutstandingIceRestartRequestOnExecutor();
        }
        if (restartAttempt_ >= config_.maximumIceRestartAttempts) {
            Fail("ice_restart_exhausted",
                 "ICE restart failed after the configured number of attempts.");
            return;
        }
        const auto delay = restartAttempt_ <= 1
            ? std::chrono::milliseconds(2000)
            : std::chrono::milliseconds(5000);
        executor_.PostAfter(delay, [this] {
            if (!closing_ && recovering_ &&
                Snapshot().webRtcState != WebRtcSessionState::kConnected) {
                BeginOrRequestIceRestartOnExecutor();
            }
        });
    }

    void CancelOutstandingIceRestartRequestOnExecutor()
    {
        if (role_ != Role::kAgent ||
            activeRestartRequestSequence_ == 0) {
            return;
        }
        if (signalingAvailable_) {
            // Best effort: even when signaling drops between request and
            // cancellation, abandon the local transaction so a later
            // recovery uses a fresh sequence instead of replaying an ID the
            // offerer has already deduplicated.
            signaling_.CancelIceRestart(
                activeRestartRequestGeneration_,
                activeRestartRequestSequence_);
        }
        activeRestartRequestGeneration_ = 0;
        activeRestartRequestSequence_ = 0;
    }

    void ClearPendingRemoteIceRestartRequestOnExecutor()
    {
        ++remoteRestartRequestToken_;
        pendingRemoteRestartGeneration_ = 0;
        pendingRemoteRestartRequestSequence_ = 0;
    }

    void ArmNegotiationTimeout()
    {
        const std::uint64_t generation = ++timerGeneration_;
        executor_.PostAfter(config_.negotiationTimeout, [this, generation] {
            if (generation != timerGeneration_ || closing_) {
                return;
            }
            const auto state = Snapshot().state;
            if (state == SessionControllerState::kNegotiating) {
                Fail("negotiation_timeout",
                     "WebRTC negotiation did not reach Connected before the timeout.");
            }
        });
    }

    void ArmReconnectTimeout()
    {
        const std::uint64_t generation = ++recoveryGeneration_;
        executor_.PostAfter(config_.reconnectTimeout, [this, generation] {
            if (generation != recoveryGeneration_ || closing_ ||
                !recovering_) {
                return;
            }
            if (Snapshot().webRtcState == WebRtcSessionState::kConnected) {
                return;
            }
            Fail("peer_reconnect_timeout",
                 "The established WebRTC connection did not recover "
                 "before the reconnect timeout.");
        });
    }

    void Fail(std::string code, std::string message)
    {
        if (closing_ || Snapshot().state == SessionControllerState::kClosed) {
            return;
        }
        {
            std::lock_guard lock(snapshotMutex_);
            snapshot_.errorCode = std::move(code);
            snapshot_.errorMessage = std::move(message);
        }
        PublishState(SessionControllerState::kFailed);
        CloseOnExecutor();
    }

    void CloseOnExecutor()
    {
        if (closing_ || Snapshot().state == SessionControllerState::kClosed) {
            return;
        }
        closing_ = true;
        ++timerGeneration_;
        ++recoveryGeneration_;
        PublishState(SessionControllerState::kClosing);
        session_.Close();
    }

    void PublishState(SessionControllerState state)
    {
        SessionControllerSnapshot snapshot;
        {
            std::lock_guard lock(snapshotMutex_);
            if (snapshot_.state == state) {
                return;
            }
            snapshot_.state = state;
            snapshot = snapshot_;
        }
        if (auto* observer = Observer()) {
            observer->OnControllerSnapshot(snapshot);
        }
    }

    ISessionControllerObserver* Observer() const
    {
        std::lock_guard lock(observerMutex_);
        return observer_;
    }

    SessionControllerBase* owner_;
    Role role_;
    IWebRtcSession& session_;
    ISessionSignalingSender& signaling_;
    SerialExecutor executor_;
    SessionControllerConfig config_;

    mutable std::mutex snapshotMutex_;
    SessionControllerSnapshot snapshot_;
    mutable std::mutex observerMutex_;
    ISessionControllerObserver* observer_ = nullptr;

    OperationId startOperation_ = 0;
    OperationId createDataChannelsOperation_ = 0;
    OperationId createOfferOperation_ = 0;
    OperationId createAnswerOperation_ = 0;
    OperationId applyRemoteDescriptionOperation_ = 0;
    SessionDescriptionType remoteDescriptionType_ =
        SessionDescriptionType::kOffer;
    std::string remoteDescriptionSdp_;
    std::optional<SessionDescription> pendingRemoteDescription_;
    bool remoteDescriptionApplied_ = false;
    std::vector<IceCandidate> pendingRemoteCandidates_;
    std::unordered_map<std::uint64_t, std::vector<IceCandidate>>
        futureRemoteCandidates_;
    std::unordered_set<std::string> remoteCandidateKeys_;
    std::unordered_set<OperationId> remoteCandidateOperationIds_;
    std::function<std::optional<OperationError>()> answerPreparation_;
    std::uint64_t signalingGeneration_ = 1;
    std::uint64_t timerGeneration_ = 0;
    std::uint64_t recoveryGeneration_ = 0;
    std::uint64_t nextRestartRequestSequence_ = 0;
    std::uint64_t activeRestartRequestGeneration_ = 0;
    std::uint64_t activeRestartRequestSequence_ = 0;
    std::uint64_t lastRemoteRestartRequestSequence_ = 0;
    std::uint64_t pendingRemoteRestartGeneration_ = 0;
    std::uint64_t pendingRemoteRestartRequestSequence_ = 0;
    std::uint64_t remoteRestartRequestToken_ = 0;
    std::uint32_t restartAttempt_ = 0;
    std::chrono::steady_clock::time_point recoveryStarted_{};
    bool signalingAvailable_ = true;
    bool recovering_ = false;
    bool restartScheduled_ = false;
    bool closing_ = false;
    bool shutdown_ = false;
};

SessionControllerBase::SessionControllerBase(
    Role role,
    IWebRtcSession& session,
    ISessionSignalingSender& signaling)
    : impl_(std::make_unique<Impl>(this, role, session, signaling))
{}

SessionControllerBase::~SessionControllerBase() = default;

void SessionControllerBase::SetObserver(
    ISessionControllerObserver* observer)
{
    impl_->SetObserver(observer);
}

void SessionControllerBase::Start(const SessionControllerConfig& config)
{
    impl_->Start(config);
}

void SessionControllerBase::HandleRemoteDescription(
    const SessionDescription& description)
{
    impl_->HandleRemoteDescription(description);
}

void SessionControllerBase::HandleRemoteIceCandidate(
    const IceCandidate& candidate)
{
    impl_->HandleRemoteIceCandidate(candidate);
}

void SessionControllerBase::HandleRemoteIceRestartRequest(
    std::uint64_t observedGeneration,
    std::uint64_t requestSequence)
{
    impl_->HandleRemoteIceRestartRequest(observedGeneration, requestSequence);
}

void SessionControllerBase::HandleRemoteIceRestartCancel(
    std::uint64_t observedGeneration,
    std::uint64_t requestSequence)
{
    impl_->HandleRemoteIceRestartCancel(observedGeneration, requestSequence);
}

void SessionControllerBase::SetSignalingAvailable(bool available)
{
    impl_->SetSignalingAvailable(available);
}

SendResult SessionControllerBase::SendData(
    const std::string& channelName,
    std::span<const std::uint8_t> data,
    bool binary)
{
    return impl_->SendData(
        channelName,
        std::vector<std::uint8_t>(data.begin(), data.end()), binary);
}

bool SessionControllerBase::QueueData(
    const std::string& channelName,
    std::span<const std::uint8_t> data,
    bool binary,
    std::function<void(SendResult)> completion)
{
    return impl_->QueueData(
        channelName,
        std::vector<std::uint8_t>(data.begin(), data.end()),
        binary,
        std::move(completion));
}

void SessionControllerBase::Close()
{
    impl_->Close();
}

SessionControllerSnapshot SessionControllerBase::Snapshot() const
{
    return impl_->Snapshot();
}

void SessionControllerBase::BeginNegotiation(
    const std::vector<DataChannelSpec>& dataChannels)
{
    impl_->BeginNegotiation(dataChannels);
}

void SessionControllerBase::SetAgentAnswerPreparation(
    std::function<std::optional<OperationError>()> preparation)
{
    impl_->SetAgentAnswerPreparation(std::move(preparation));
}

void SessionControllerBase::OnSessionStateChanged(WebRtcSessionState state)
{
    impl_->OnSessionStateChanged(state);
}

void SessionControllerBase::OnIceGatheringStateChanged(
    WebRtcIceGatheringState state)
{
    impl_->OnIceGatheringStateChanged(state);
}

void SessionControllerBase::OnLocalDescription(
    const SessionDescription& description)
{
    impl_->OnLocalDescription(description);
}

void SessionControllerBase::OnLocalIceCandidate(
    const IceCandidate& candidate)
{
    impl_->OnLocalIceCandidate(candidate);
}

void SessionControllerBase::OnDataChannelStateChanged(
    const DataChannelInfo& channel)
{
    impl_->OnDataChannelStateChanged(channel);
}

void SessionControllerBase::OnDataMessage(
    const std::string& label,
    std::span<const std::uint8_t> payload,
    bool binary)
{
    impl_->OnDataMessage(label,
                         std::vector<std::uint8_t>(payload.begin(),
                                                   payload.end()),
                         binary);
}

void SessionControllerBase::OnRemoteTrackAdded(const RemoteTrackInfo& track)
{
    impl_->OnRemoteTrackAdded(track);
}

void SessionControllerBase::OnOperationCompleted(OperationId operationId)
{
    impl_->OnOperationCompleted(operationId);
}

void SessionControllerBase::OnWebRtcError(
    OperationId operationId,
    const OperationError& error)
{
    impl_->OnWebRtcError(operationId, error);
}

ControllerSessionController::ControllerSessionController(
    IWebRtcSession& session,
    ISessionSignalingSender& signaling)
    : SessionControllerBase(Role::kController, session, signaling)
{}

void ControllerSessionController::Connect(
    const std::vector<DataChannelSpec>& dataChannels)
{
    BeginNegotiation(dataChannels);
}

AgentSessionController::AgentSessionController(
    IWebRtcSession& session,
    ISessionSignalingSender& signaling)
    : SessionControllerBase(Role::kAgent, session, signaling)
{}

void AgentSessionController::SetAnswerPreparation(
    std::function<std::optional<OperationError>()> preparation)
{
    SetAgentAnswerPreparation(std::move(preparation));
}

}  // namespace remote
