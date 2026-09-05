// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace remote {

class BinaryProtocolWriter final {
public:
    explicit BinaryProtocolWriter(std::vector<std::uint8_t>* output)
        : output_(output)
    {}

    bool valid() const { return output_ != nullptr; }

    void WriteU8(std::uint8_t value);
    void WriteU16(std::uint16_t value);
    void WriteU32(std::uint32_t value);
    void WriteU64(std::uint64_t value);
    void WriteString(const std::string& value);
    void WriteBytes(std::span<const std::uint8_t> value);

private:
    std::vector<std::uint8_t>* output_ = nullptr;
};

class BinaryProtocolReader final {
public:
    explicit BinaryProtocolReader(std::span<const std::uint8_t> bytes)
        : bytes_(bytes)
    {}

    bool ReadU8(std::uint8_t* value);
    bool ReadU16(std::uint16_t* value);
    bool ReadU32(std::uint32_t* value);
    bool ReadU64(std::uint64_t* value);
    bool ReadString(std::size_t size, std::string* value);
    bool ReadBytes(std::size_t size, std::vector<std::uint8_t>* value);
    bool ReadBytes(std::span<std::uint8_t> value);
    template <std::size_t Size>
    bool ReadBytes(std::array<std::uint8_t, Size>* value)
    {
        return value && ReadBytes(std::span<std::uint8_t>(*value));
    }
    bool Skip(std::size_t size);

    std::size_t remaining() const { return bytes_.size() - offset_; }
    std::size_t Remaining() const { return remaining(); }
    std::size_t offset() const { return offset_; }

private:
    std::span<const std::uint8_t> bytes_;
    std::size_t offset_ = 0;
};

}  // namespace remote
