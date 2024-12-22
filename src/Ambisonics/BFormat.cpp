// Copyright (c) 2024-present Sparky Studios. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <Ambisonics/BFormat.h>
#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    BFormat::BFormat()
        : _buffer(nullptr)
    {}

    BFormat::~BFormat()
    {
        if (_buffer)
            ampooldelete(eMemoryPoolKind_SoundData, AudioBuffer, _buffer);

        _buffer = nullptr;
    }

    void BFormat::Reset()
    {
        if (_buffer == nullptr)
            return;

        _buffer->Clear();
    }

    void BFormat::Refresh()
    {
        // noop
    }

    bool BFormat::Configure(AmUInt32 order, bool is3D, AmUInt32 sampleCount)
    {
        if (!AmbisonicComponent::Configure(order, is3D))
            return false;

        if (_buffer)
            ampooldelete(eMemoryPoolKind_SoundData, AudioBuffer, _buffer);

        _buffer = ampoolnew(eMemoryPoolKind_SoundData, AudioBuffer, sampleCount, GetChannelCount());

        return true;
    }

    void BFormat::CopyStream(const AudioBufferChannel& buffer, AmUInt32 channel, AmUInt32 sampleCount) const
    {
        if (channel >= GetChannelCount())
            return;

        auto& channelBuffer = _buffer->GetChannel(channel);
        channelBuffer = buffer;
    }

    void BFormat::AddStream(const AudioBufferChannel& buffer, AmUInt32 channel, AmUInt32 sampleCount, AmUInt32 offset) const
    {
        if (channel >= GetChannelCount())
            return;

        auto& channelBuffer = _buffer->GetChannel(channel);
        channelBuffer += buffer;
    }

    void BFormat::GetStream(AudioBufferChannel& buffer, AmUInt32 channel, AmUInt32 sampleCount) const
    {
        if (channel >= GetChannelCount())
            return;

        const auto& channelBuffer = _buffer->GetChannel(channel);
        buffer = channelBuffer;
    }

    BFormat& BFormat::operator=(const BFormat& other)
    {
        if (this != &other)
            AudioBuffer::Copy(*other._buffer, 0, *_buffer, 0, _buffer->GetFrameCount());

        return *this;
    }

    bool BFormat::operator==(const BFormat& other) const
    {
        return m_is3D == other.m_is3D && m_order == other.m_order && _buffer->GetFrameCount() == other._buffer->GetFrameCount();
    }

    bool BFormat::operator!=(const BFormat& other) const
    {
        return !(*this == other);
    }

    BFormat& BFormat::operator+=(const BFormat& other)
    {
        *_buffer += *other._buffer;
        return *this;
    }

    BFormat& BFormat::operator-=(const BFormat& other)
    {
        *_buffer -= *other._buffer;
        return *this;
    }

    BFormat& BFormat::operator*=(const BFormat& other)
    {
        *_buffer *= *other._buffer;
        return *this;
    }

    BFormat& BFormat::operator+=(const AmReal32& value)
    {
        for (AmUInt32 c = 0; c < GetChannelCount(); ++c)
        {
            auto& src = _buffer->GetChannel(c);
            for (AmUInt32 i = 0; i < GetSampleCount(); ++i)
                src[i] += value;
        }

        return *this;
    }

    BFormat& BFormat::operator-=(const AmReal32& value)
    {
        for (AmUInt32 c = 0; c < GetChannelCount(); ++c)
        {
            auto& src = _buffer->GetChannel(c);
            for (AmUInt32 i = 0; i < GetSampleCount(); ++i)
                src[i] -= value;
        }

        return *this;
    }

    BFormat& BFormat::operator*=(const AmReal32& value)
    {
        ScalarMultiply(_buffer->GetData().GetBuffer(), _buffer->GetData().GetBuffer(), value, GetSampleCount() * GetChannelCount());
        return *this;
    }
} // namespace SparkyStudios::Audio::Amplitude
