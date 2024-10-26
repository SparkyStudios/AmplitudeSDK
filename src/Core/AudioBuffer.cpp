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

#include <SparkyStudios/Audio/Amplitude/Core/AudioBuffer.h>

#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    AmSize AudioBufferChannel::size() const
    {
        return _frameCount;
    }

    AmReal32* AudioBufferChannel::begin()
    {
        AMPLITUDE_ASSERT(_isEnabled);
        return _begin;
    }

    const AmReal32* AudioBufferChannel::begin() const
    {
        AMPLITUDE_ASSERT(_isEnabled);
        return _begin;
    }

    AmReal32* AudioBufferChannel::end()
    {
        AMPLITUDE_ASSERT(_isEnabled);
        return _begin + _frameCount;
    }

    const AmReal32* AudioBufferChannel::end() const
    {
        AMPLITUDE_ASSERT(_isEnabled);
        return _begin + _frameCount;
    }

    void AudioBufferChannel::clear()
    {
        AMPLITUDE_ASSERT(_isEnabled);
        std::memset(begin(), 0, sizeof(AmReal32) * _frameCount);
    }

    bool AudioBufferChannel::enabled() const
    {
        return _isEnabled;
    }

    AmReal32& AudioBufferChannel::operator[](const AmSize index)
    {
        AMPLITUDE_ASSERT(_isEnabled);
        AMPLITUDE_ASSERT(index < _frameCount);
        return _begin[index];
    }

    const AmReal32& AudioBufferChannel::operator[](const AmSize index) const
    {
        AMPLITUDE_ASSERT(_isEnabled);
        AMPLITUDE_ASSERT(index < _frameCount);
        return _begin[index];
    }

    AudioBufferChannel& AudioBufferChannel::operator=(const std::vector<AmReal32>& data)
    {
        AMPLITUDE_ASSERT(_isEnabled);
        AMPLITUDE_ASSERT(_frameCount <= data.size());
        std::memcpy(_begin, data.data(), sizeof(AmReal32) * _frameCount);
        return *this;
    }

    AudioBufferChannel& AudioBufferChannel::operator=(const AudioBufferChannel& channel)
    {
        if (this != &channel)
        {
            AMPLITUDE_ASSERT(_isEnabled);
            AMPLITUDE_ASSERT(channel._isEnabled);
            AMPLITUDE_ASSERT(_frameCount <= channel._frameCount);
            std::memcpy(_begin, channel._begin, sizeof(AmReal32) * _frameCount);
        }

        return *this;
    }

    AudioBufferChannel& AudioBufferChannel::operator+=(const AudioBufferChannel& channel)
    {
        AMPLITUDE_ASSERT(_isEnabled);
        AMPLITUDE_ASSERT(channel._isEnabled);
        AMPLITUDE_ASSERT(_frameCount <= channel._frameCount);

        AmSize remaining = _frameCount;

#if defined(AM_SIMD_INTRINSICS)
        const AmSize end = GetNumSimdChunks(_frameCount);
        constexpr AmSize blockSize = GetSimdBlockSize();
        remaining = remaining - end;

        for (AmSize i = 0; i < end; i += blockSize)
        {
            const auto ba = xsimd::load_aligned<simd_arch>(_begin + i);
            const auto bb = xsimd::load_aligned<simd_arch>(channel._begin + i);

            auto res = xsimd::add(ba, bb);
            res.store_aligned(_begin + i);
        }
#endif

        for (AmSize i = _frameCount - remaining; i < _frameCount; i++)
            _begin[i] += channel._begin[i];

        return *this;
    }

    AudioBufferChannel& AudioBufferChannel::operator-=(const AudioBufferChannel& channel)
    {
        AMPLITUDE_ASSERT(_isEnabled);
        AMPLITUDE_ASSERT(channel._isEnabled);
        AMPLITUDE_ASSERT(_frameCount <= channel._frameCount);

        AmSize remaining = _frameCount;

#if defined(AM_SIMD_INTRINSICS)
        const AmSize end = GetNumSimdChunks(_frameCount);
        constexpr AmSize blockSize = GetSimdBlockSize();
        remaining = remaining - end;

        for (AmSize i = 0; i < end; i += blockSize)
        {
            const auto ba = xsimd::load_aligned<simd_arch>(_begin + i);
            const auto bb = xsimd::load_aligned<simd_arch>(channel._begin + i);

            auto res = xsimd::sub(ba, bb);
            res.store_aligned(_begin + i);
        }
#endif

        for (AmSize i = _frameCount - remaining; i < _frameCount; i++)
            _begin[i] -= channel._begin[i];

        return *this;
    }

    AudioBufferChannel& AudioBufferChannel::operator*=(const AudioBufferChannel& channel)
    {
        AMPLITUDE_ASSERT(_isEnabled);
        AMPLITUDE_ASSERT(channel._isEnabled);
        AMPLITUDE_ASSERT(_frameCount <= channel._frameCount);

        PointWiseMultiply(_begin, channel._begin, _begin, _frameCount);

        return *this;
    }

    AudioBufferChannel& AudioBufferChannel::operator*=(AmReal32 scalar)
    {
        AMPLITUDE_ASSERT(_isEnabled);

        ScalarMultiply(_begin, _begin, scalar, _frameCount);

        return *this;
    }

    AudioBufferChannel::AudioBufferChannel(AmReal32* begin, const AmSize numFrames)
        : _begin(begin)
        , _frameCount(numFrames)
        , _isEnabled(true)
    {}

    void AudioBuffer::Copy(
        const AudioBuffer& source, AmSize sourceOffset, AudioBuffer& destination, AmSize destinationOffset, AmSize numFrames)
    {
        AMPLITUDE_ASSERT(destination.GetChannelCount() >= source.GetChannelCount());
        AMPLITUDE_ASSERT(sourceOffset + numFrames <= source.GetFrameCount());
        AMPLITUDE_ASSERT(destinationOffset + numFrames <= destination.GetFrameCount());

        for (AmSize i = 0, l = source.GetChannelCount(); i < l; ++i)
        {
            const auto& srcChannel = source[i];
            auto& dstChannel = destination[i];

            std::memcpy(dstChannel.begin() + destinationOffset, srcChannel.begin() + sourceOffset, sizeof(AmReal32) * numFrames);
        }
    }

    AudioBuffer::AudioBuffer()
        : _frameCount(0)
    {}

    AudioBuffer::AudioBuffer(const AmSize numFrames, AmSize numChannels)
        : _frameCount(numFrames)
    {
        Initialize(numChannels);
    }

    AudioBuffer::AudioBuffer(AudioBuffer&& buffer) noexcept
    {
        _frameCount = buffer._frameCount;
        _channels = std::move(buffer._channels);
        AmAlignedReal32Buffer::Swap(_data, buffer._data);

        buffer._frameCount = 0;
        buffer._channels.clear();
        buffer._data.Clear();
    }

    AudioBuffer::~AudioBuffer()
    {
        _data.Release();
    }

    bool AudioBuffer::IsEmpty() const
    {
        return _frameCount == 0 || _channels.empty();
    }

    AmSize AudioBuffer::GetFrameCount() const
    {
        return _frameCount;
    }

    AmSize AudioBuffer::GetChannelCount() const
    {
        return _channels.size();
    }

    void AudioBuffer::Clear()
    {
        for (auto& channel : _channels)
            channel.clear();
    }

    const AmAlignedReal32Buffer& AudioBuffer::GetData() const
    {
        return _data;
    }

    AudioBufferChannel& AudioBuffer::GetChannel(AmSize index)
    {
        AMPLITUDE_ASSERT(index < _channels.size());
        return _channels[index];
    }

    const AudioBufferChannel& AudioBuffer::GetChannel(AmSize index) const
    {
        AMPLITUDE_ASSERT(index < _channels.size());
        return _channels[index];
    }

    AudioBuffer AudioBuffer::Clone() const
    {
        AudioBuffer result(_frameCount, GetChannelCount());
        Copy(*this, 0, result, 0, _frameCount);
        return result;
    }

    AudioBufferChannel& AudioBuffer::operator[](const AmSize index)
    {
        return GetChannel(index);
    }

    const AudioBufferChannel& AudioBuffer::operator[](const AmSize index) const
    {
        return GetChannel(index);
    }

    AudioBuffer& AudioBuffer::operator=(const AudioBuffer& buffer)
    {
        if (this != &buffer)
        {
            if (_frameCount != buffer._frameCount || GetChannelCount() != buffer.GetChannelCount())
            {
                _frameCount = buffer._frameCount;
                Initialize(buffer.GetChannelCount());
            }

            for (AmSize i = 0; i < GetChannelCount(); ++i)
                _channels[i] = buffer._channels[i];
        }

        return *this;
    }

    AudioBuffer& AudioBuffer::operator+=(const AudioBuffer& buffer)
    {
        AMPLITUDE_ASSERT(_frameCount == buffer._frameCount);

        for (AmSize i = 0; i < GetChannelCount(); ++i)
            _channels[i] += buffer._channels[i];

        return *this;
    }

    AudioBuffer& AudioBuffer::operator-=(const AudioBuffer& buffer)
    {
        AMPLITUDE_ASSERT(_frameCount == buffer._frameCount);

        for (AmSize i = 0; i < GetChannelCount(); ++i)
            _channels[i] -= buffer._channels[i];

        return *this;
    }

    AudioBuffer& AudioBuffer::operator*=(const AudioBuffer& buffer)
    {
        AMPLITUDE_ASSERT(_frameCount == buffer._frameCount);

        for (AmSize i = 0; i < GetChannelCount(); ++i)
            _channels[i] *= buffer._channels[i];

        return *this;
    }

    AudioBuffer& AudioBuffer::operator*=(AmReal32 scalar)
    {
        for (AmSize i = 0; i < GetChannelCount(); ++i)
            _channels[i] *= scalar;

        return *this;
    }

    void AudioBuffer::Initialize(const AmSize channelCount)
    {
        const AmSize alignedSize = FindNextAlignedArrayIndex<AmReal32>(_frameCount, AM_SIMD_ALIGNMENT);

        _data.Resize(alignedSize * channelCount, true);

        _channels.clear();
        _channels.reserve(channelCount);

        AmReal32* itr = _data.GetBuffer();

        for (AmSize i = 0; i < channelCount; ++i)
        {
            AudioBufferChannel channelBuffer(itr, _frameCount);
            _channels.push_back(std::move(channelBuffer));
            itr += alignedSize;
        }
    }
} // namespace SparkyStudios::Audio::Amplitude