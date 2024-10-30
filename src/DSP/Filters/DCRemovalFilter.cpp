// Copyright (c) 2021-present Sparky Studios. All rights reserved.
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

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>

#include <DSP/Filters/DCRemovalFilter.h>

namespace SparkyStudios::Audio::Amplitude
{
    DCRemovalFilter::DCRemovalFilter()
        : Filter("DCRemoval")
        , _length(0.1f)
    {}

    AmResult DCRemovalFilter::Initialize(AmReal32 length)
    {
        if (length <= 0)
            return eErrorCode_InvalidParameter;

        _length = length;

        return eErrorCode_Success;
    }

    FilterInstance* DCRemovalFilter::CreateInstance()
    {
        return ampoolnew(eMemoryPoolKind_Filtering, DCRemovalFilterInstance, this);
    }

    void DCRemovalFilter::DestroyInstance(FilterInstance* instance)
    {
        ampooldelete(eMemoryPoolKind_Filtering, DCRemovalFilterInstance, (DCRemovalFilterInstance*)instance);
    }

    DCRemovalFilterInstance::DCRemovalFilterInstance(DCRemovalFilter* parent)
        : FilterInstance(parent)
        , _buffer()
        , _totals()
        , _bufferLength(0)
        , _offset(0)
    {
        Initialize(DCRemovalFilter::ATTRIBUTE_LAST);
        SetParameter(DCRemovalFilter::ATTRIBUTE_LAST, parent->_length);
    }

    DCRemovalFilterInstance::~DCRemovalFilterInstance()
    {
        _buffer.Release();
        _totals.Release();
    }

    void DCRemovalFilterInstance::Process(const AudioBuffer& in, AudioBuffer& out, AmUInt64 frames, AmUInt32 sampleRate)
    {
        const AmUInt16 channels = in.GetChannelCount();

        if (_buffer.GetPointer() == nullptr)
        {
            InitializeBuffer(channels, sampleRate);
        }

        for (AmUInt16 c = 0; c < channels; c++)
        {
            const auto& inChannel = in[c];
            auto& outChannel = out[c];

            _offset = 0;

            for (AmInt64 i = 0; i < frames; i++)
            {
                outChannel[i] = ProcessSample(inChannel[i], c, sampleRate);
                _offset = (_offset + 1) % _bufferLength;
            }
        }
    }

    AmAudioSample DCRemovalFilterInstance::ProcessSample(AmAudioSample sample, AmUInt16 channel, AmUInt32 sampleRate)
    {
        const AmUInt64 o = _offset + channel * _bufferLength;

        const AmReal32 x = sample;
        /* */ AmReal32 y;

        _totals[channel] -= _buffer[o];
        _totals[channel] += x;

        _buffer[o] = x;

        y = x - _totals[channel] / static_cast<AmReal32>(_bufferLength);
        y = x + (y - x) * m_parameters[DCRemovalFilter::ATTRIBUTE_WET];

        return static_cast<AmAudioSample>(y);
    }

    void DCRemovalFilterInstance::InitializeBuffer(AmUInt16 channels, AmUInt32 sampleRate)
    {
        _bufferLength = static_cast<AmUInt64>(std::ceil(m_parameters[DCRemovalFilter::ATTRIBUTE_LENGTH] * sampleRate));

        _buffer.Resize(_bufferLength * channels, true);
        _totals.Resize(channels, true);
    }
} // namespace SparkyStudios::Audio::Amplitude
