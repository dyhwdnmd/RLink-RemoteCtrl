// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

class LibWebRtcSession::CallbackGate final {
public:
    class Lease final {
    public:
        Lease() = default;
        Lease(CallbackGate* gate, LibWebRtcSession* owner)
            : gate_(gate), owner_(owner)
        {}
        Lease(const Lease&) = delete;
        Lease& operator=(const Lease&) = delete;
        Lease(Lease&& other) noexcept
            : gate_(std::exchange(other.gate_, nullptr)),
              owner_(std::exchange(other.owner_, nullptr))
        {}
        ~Lease()
        {
            if (gate_) {
                gate_->Leave();
            }
        }

        LibWebRtcSession* Owner() const { return owner_; }

    private:
        CallbackGate* gate_ = nullptr;
        LibWebRtcSession* owner_ = nullptr;
    };

    explicit CallbackGate(LibWebRtcSession* owner) : owner_(owner) {}

    Lease Enter()
    {
        std::lock_guard lock(mutex_);
        if (!owner_) {
            return {};
        }
        ++activeCallbacks_;
        return {this, owner_};
    }

    void DetachAndWait()
    {
        std::unique_lock lock(mutex_);
        owner_ = nullptr;
        condition_.wait(lock, [this] { return activeCallbacks_ == 0; });
    }

private:
    void Leave()
    {
        std::lock_guard lock(mutex_);
        --activeCallbacks_;
        if (activeCallbacks_ == 0) {
            condition_.notify_all();
        }
    }

    std::mutex mutex_;
    std::condition_variable condition_;
    LibWebRtcSession* owner_ = nullptr;
    int activeCallbacks_ = 0;
};

class LibWebRtcSession::DataChannelBinding final
    : public webrtc::DataChannelObserver {
public:
    DataChannelBinding(
        LibWebRtcSession* owner,
        webrtc::scoped_refptr<webrtc::DataChannelInterface> channel)
        : owner_(owner), channel_(std::move(channel))
    {
        channel_->RegisterObserver(this);
    }

    ~DataChannelBinding() override { channel_->UnregisterObserver(); }

    webrtc::scoped_refptr<webrtc::DataChannelInterface> Channel() const
    {
        return channel_;
    }

    void Close() { channel_->Close(); }

    void OnStateChange() override
    {
        owner_->HandleDataChannelState(channel_);
    }

    void OnMessage(const webrtc::DataBuffer& buffer) override
    {
        owner_->HandleDataMessage(channel_->label(), buffer);
    }

private:
    LibWebRtcSession* owner_;
    webrtc::scoped_refptr<webrtc::DataChannelInterface> channel_;
};
