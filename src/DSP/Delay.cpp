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

#include <ranges>

#include <DSP/Delay.h>

namespace SparkyStudios::Audio::Amplitude
{
    Delay::Delay(AmSize maxDelay, AmSize framesCount)
        : _maxDelay(maxDelay)
        , _buffer(nullptr)
        , _framesCount(framesCount)
        , _writePos(0)
    {
        AMPLITUDE_ASSERT(_framesCount > 0);
        SetMaxDelay(maxDelay);
    }

    Delay::~Delay()
    {
        _buffer.reset(nullptr);
    }

    void Delay::SetMaxDelay(AmSize maxDelay)
    {
        _maxDelay = maxDelay;
        const AmSize newFramesCount = _framesCount + _maxDelay;

        if (_buffer == nullptr)
        {
            _buffer.reset(ampoolnew(eMemoryPoolKind_Filtering, AudioBuffer, newFramesCount, kAmMonoChannelCount));
            _buffer->Clear();
            return;
        }

        AudioBufferChannel* channel = &_buffer->GetChannel(0);
        const AmSize oldFramesCount = _buffer->GetFrameCount();

        if (_maxDelay > oldFramesCount - _framesCount)
        {
            AmUniquePtr<eMemoryPoolKind_Filtering, AudioBuffer> newBuffer(
                ampoolnew(eMemoryPoolKind_Filtering, AudioBuffer, newFramesCount, kAmMonoChannelCount));
            newBuffer->Clear();

            std::copy(channel->begin() + _writePos, channel->end(), newBuffer->GetChannel(0).begin());

            if (_writePos > 0)
            {
                std::copy_n(channel->begin(), _writePos, newBuffer->GetChannel(0).begin() + oldFramesCount - _writePos);
                _writePos = oldFramesCount;
            }

            _buffer = std::move(newBuffer);
        }
    }

    AmSize Delay::GetMaxDelay() const
    {
        return _maxDelay;
    }

    AmSize Delay::GetDelayInSamples() const
    {
        return _buffer->GetFrameCount();
    }

    void Delay::Clear()
    {
        _buffer->Clear();
    }

    void Delay::Insert(const AudioBufferChannel& channel)
    {
        AMPLITUDE_ASSERT(_buffer != nullptr);
        AMPLITUDE_ASSERT(channel.size() == _framesCount);

        const AmSize delayBufferSize = _buffer->GetFrameCount();

        // Record the remaining space in the _buffer after the write cursor.
        const AmSize remainingSizeWrite = delayBufferSize - _writePos;
        AudioBufferChannel* delayChannel = &_buffer->GetChannel(0);

        // Copy the channel into the delay line.
        if (remainingSizeWrite >= _framesCount)
        {
            AMPLITUDE_ASSERT(delayChannel->begin() + _writePos + _framesCount <= delayChannel->end());
            std::ranges::copy(channel, delayChannel->begin() + _writePos);
        }
        else
        {
            AMPLITUDE_ASSERT(delayChannel->begin() + _writePos + remainingSizeWrite <= delayChannel->end());
            AMPLITUDE_ASSERT(channel.begin() + remainingSizeWrite <= channel.end());
            std::copy_n(channel.begin(), remainingSizeWrite, delayChannel->begin() + _writePos);
            AMPLITUDE_ASSERT(delayChannel->begin() + remainingSizeWrite <= delayChannel->end());
            std::copy(channel.begin() + remainingSizeWrite, channel.end(), delayChannel->begin());
        }

        _writePos = (_writePos + _framesCount) % delayBufferSize;
    }

    void Delay::Process(AudioBufferChannel& channel, AmSize delaySamples)
    {
        AMPLITUDE_ASSERT(_buffer != nullptr);
        AMPLITUDE_ASSERT(delaySamples >= 0U);
        AMPLITUDE_ASSERT(delaySamples <= _maxDelay);

        const AmSize delayBufferSize = _buffer->GetFrameCount();
        // Position in the delay line to begin reading from.
        AMPLITUDE_ASSERT(_writePos + delayBufferSize >= delaySamples + _framesCount);
        const AmSize readCursor = (_writePos + delayBufferSize - delaySamples - _framesCount) % delayBufferSize;
        // Record the remaining space in the _buffer after the read cursor.
        const AmSize remainingSizeRead = delayBufferSize - readCursor;
        AudioBufferChannel* delayChannel = &(*_buffer)[0];

        // Extract a portion of the delay line into the channel.
        if (remainingSizeRead >= _framesCount)
        {
            AMPLITUDE_ASSERT(channel.begin() + _framesCount <= channel.end());
            AMPLITUDE_ASSERT(delayChannel->begin() + readCursor + _framesCount <= delayChannel->end());
            std::copy_n(delayChannel->begin() + readCursor, _framesCount, channel.begin());
        }
        else
        {
            AMPLITUDE_ASSERT(channel.begin() + delayChannel->size() - readCursor <= channel.end());
            std::copy(delayChannel->begin() + readCursor, delayChannel->end(), channel.begin());

            AMPLITUDE_ASSERT(channel.begin() + _framesCount <= channel.end());
            AMPLITUDE_ASSERT(delayChannel->begin() + _framesCount - remainingSizeRead <= delayChannel->end());
            std::copy_n(delayChannel->begin(), _framesCount - remainingSizeRead, channel.begin() + remainingSizeRead);
        }
    }
} // namespace SparkyStudios::Audio::Amplitude
