// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "BinaryProtocol.h"

#include <algorithm>

namespace remote {

void BinaryProtocolWriter::WriteU8(std::uint8_t value)
{
    if (output_) {
        output_->push_back(value);
    }
}

void BinaryProtocolWriter::WriteU16(std::uint16_t value)
{
    if (!output_) {
        return;
    }
    output_->push_back(static_cast<std::uint8_t>((value >> 8) & 0xff));
    output_->push_back(static_cast<std::uint8_t>(value & 0xff));
}

void BinaryProtocolWriter::WriteU32(std::uint32_t value)
{
    if (!output_) {
        return;
    }
    for (int shift = 24; shift >= 0; shift -= 8) {
        output_->push_back(
            static_cast<std::uint8_t>((value >> shift) & 0xff));
    }
}

void BinaryProtocolWriter::WriteU64(std::uint64_t value)
{
    if (!output_) {
        return;
    }
    for (int shift = 56; shift >= 0; shift -= 8) {
        output_->push_back(
            static_cast<std::uint8_t>((value >> shift) & 0xff));
    }
}

void BinaryProtocolWriter::WriteString(const std::string& value)
{
    if (output_) {
        output_->insert(output_->end(), value.begin(), value.end());
    }
}

void BinaryProtocolWriter::WriteBytes(
    std::span<const std::uint8_t> value)
{
    if (output_) {
        output_->insert(output_->end(), value.begin(), value.end());
    }
}

bool BinaryProtocolReader::ReadU8(std::uint8_t* value)
{
    if (!value || remaining() < 1) {
        return false;
    }
    *value = bytes_[offset_++];
    return true;
}

bool BinaryProtocolReader::ReadU16(std::uint16_t* value)
{
    if (!value || remaining() < 2) {
        return false;
    }
    *value = static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(bytes_[offset_]) << 8) |
        bytes_[offset_ + 1]);
    offset_ += 2;
    return true;
}

bool BinaryProtocolReader::ReadU32(std::uint32_t* value)
{
    if (!value || remaining() < 4) {
        return false;
    }
    *value = 0;
    for (int index = 0; index < 4; ++index) {
        *value = (*value << 8) | bytes_[offset_ + index];
    }
    offset_ += 4;
    return true;
}

bool BinaryProtocolReader::ReadU64(std::uint64_t* value)
{
    if (!value || remaining() < 8) {
        return false;
    }
    *value = 0;
    for (int index = 0; index < 8; ++index) {
        *value = (*value << 8) | bytes_[offset_ + index];
    }
    offset_ += 8;
    return true;
}

bool BinaryProtocolReader::ReadString(
    std::size_t size,
    std::string* value)
{
    if (!value || remaining() < size) {
        return false;
    }
    value->assign(
        reinterpret_cast<const char*>(bytes_.data() + offset_), size);
    offset_ += size;
    return true;
}

bool BinaryProtocolReader::ReadBytes(
    std::size_t size,
    std::vector<std::uint8_t>* value)
{
    if (!value || remaining() < size) {
        return false;
    }
    value->assign(bytes_.begin() + offset_,
                  bytes_.begin() + offset_ + size);
    offset_ += size;
    return true;
}

bool BinaryProtocolReader::ReadBytes(std::span<std::uint8_t> value)
{
    if (remaining() < value.size()) {
        return false;
    }
    std::copy_n(bytes_.begin() + offset_, value.size(), value.begin());
    offset_ += value.size();
    return true;
}

bool BinaryProtocolReader::Skip(std::size_t size)
{
    if (remaining() < size) {
        return false;
    }
    offset_ += size;
    return true;
}

}  // namespace remote
