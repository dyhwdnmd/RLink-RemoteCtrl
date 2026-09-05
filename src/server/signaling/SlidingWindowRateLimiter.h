// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <algorithm>
#include <limits>

#include <QHash>
#include <QQueue>
#include <QString>

namespace remote::signaling_server {

struct SlidingWindowRateLimitPolicy {
    int maximumCost = 1;
    qint64 windowMs = 60000;
    int maximumKeys = 4096;
};

// In-memory, single-threaded rate limiter for the signaling event loop. Each
// key keeps only the accepted events inside the active window, while the
// number of keys is capped to prevent the limiter itself becoming a memory
// exhaustion target.
class SlidingWindowRateLimiter final {
public:
    explicit SlidingWindowRateLimiter(
        SlidingWindowRateLimitPolicy policy)
        : policy_(policy)
    {}

    bool CanAcquire(const QString& key,
                    int cost,
                    qint64 nowMs,
                    qint64* retryAfterMs = nullptr)
    {
        SetRetryAfter(retryAfterMs, 0);
        if (key.isEmpty() || cost <= 0 || cost > policy_.maximumCost ||
            policy_.maximumCost <= 0 || policy_.windowMs <= 0 ||
            policy_.maximumKeys <= 0) {
            SetRetryAfter(retryAfterMs, policy_.windowMs);
            return false;
        }

        auto bucketIt = buckets_.find(key);
        if (bucketIt == buckets_.end()) {
            return true;
        }
        Bucket& bucket = bucketIt.value();
        const qint64 effectiveNow = std::max(nowMs, bucket.lastSeenMs);
        Expire(bucket, effectiveNow);
        bucket.lastSeenMs = effectiveNow;
        if (bucket.events.size() + cost <= policy_.maximumCost) {
            return true;
        }

        if (!bucket.events.isEmpty()) {
            SetRetryAfter(
                retryAfterMs,
                std::max<qint64>(
                    1, bucket.events.head() + policy_.windowMs - effectiveNow));
        }
        return false;
    }

    void Record(const QString& key, int cost, qint64 nowMs)
    {
        if (key.isEmpty() || cost <= 0 || cost > policy_.maximumCost ||
            policy_.maximumKeys <= 0) {
            return;
        }
        EnsureCapacityFor(key, nowMs);
        Bucket& bucket = buckets_[key];
        const qint64 effectiveNow = std::max(nowMs, bucket.lastSeenMs);
        Expire(bucket, effectiveNow);
        for (int index = 0; index < cost; ++index) {
            bucket.events.enqueue(effectiveNow);
        }
        bucket.lastSeenMs = effectiveNow;
    }

    bool TryAcquire(const QString& key,
                    int cost,
                    qint64 nowMs,
                    qint64* retryAfterMs = nullptr)
    {
        if (!CanAcquire(key, cost, nowMs, retryAfterMs)) {
            return false;
        }
        Record(key, cost, nowMs);
        return true;
    }

    void Prune(qint64 nowMs)
    {
        for (auto it = buckets_.begin(); it != buckets_.end();) {
            Bucket& bucket = it.value();
            const qint64 effectiveNow = std::max(nowMs, bucket.lastSeenMs);
            Expire(bucket, effectiveNow);
            if (bucket.events.isEmpty() &&
                effectiveNow - bucket.lastSeenMs >= policy_.windowMs) {
                it = buckets_.erase(it);
            } else {
                ++it;
            }
        }
    }

    void Clear()
    {
        buckets_.clear();
    }

    qsizetype keyCount() const
    {
        return buckets_.size();
    }

private:
    struct Bucket {
        QQueue<qint64> events;
        qint64 lastSeenMs = 0;
    };

    static void SetRetryAfter(qint64* destination, qint64 value)
    {
        if (destination) {
            *destination = value;
        }
    }

    void Expire(Bucket& bucket, qint64 nowMs) const
    {
        const qint64 cutoff = nowMs - policy_.windowMs;
        while (!bucket.events.isEmpty() &&
               bucket.events.head() <= cutoff) {
            bucket.events.dequeue();
        }
    }

    void EnsureCapacityFor(const QString& key, qint64 nowMs)
    {
        if (buckets_.contains(key)) {
            return;
        }
        if (buckets_.size() < policy_.maximumKeys) {
            return;
        }

        // The periodic server sweep already removes expired buckets. Avoid
        // scanning every existing key whenever a previously unseen device,
        // user, or IP address first records an event. An eager prune is only
        // needed when the bounded table has actually reached capacity.
        Prune(nowMs);
        if (buckets_.size() < policy_.maximumKeys) {
            return;
        }

        auto oldest = buckets_.end();
        qint64 oldestSeen = std::numeric_limits<qint64>::max();
        for (auto it = buckets_.begin(); it != buckets_.end(); ++it) {
            if (it->lastSeenMs < oldestSeen) {
                oldestSeen = it->lastSeenMs;
                oldest = it;
            }
        }
        if (oldest != buckets_.end()) {
            buckets_.erase(oldest);
        }
    }

    SlidingWindowRateLimitPolicy policy_;
    QHash<QString, Bucket> buckets_;
};

}  // namespace remote::signaling_server
