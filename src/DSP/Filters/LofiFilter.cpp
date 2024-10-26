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

#include <DSP/Filters/LofiFilter.h>
#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    LofiFilter::LofiFilter()
        : Filter("Lofi")
        , _sampleRate(4000)
        , _bitDepth(3)
    {}

    AmResult LofiFilter::Init(AmReal32 sampleRate, AmReal32 bitDepth)
    {
        if (sampleRate <= 0 || bitDepth <= 0)
            return eErrorCode_InvalidParameter;

        _sampleRate = sampleRate;
        _bitDepth = bitDepth;

        return eErrorCode_Success;
    }

    AmUInt32 LofiFilter::GetParamCount() const
    {
        return ATTRIBUTE_LAST;
    }

    AmReal32 LofiFilter::GetParamMax(AmUInt32 index) const
    {
        switch (index)
        {
        case ATTRIBUTE_SAMPLERATE:
            return 22000;
        case ATTRIBUTE_BITDEPTH:
            return 16;
        default:
            return 1;
        }
    }

    AmReal32 LofiFilter::GetParamMin(AmUInt32 index) const
    {
        switch (index)
        {
        case ATTRIBUTE_SAMPLERATE:
            return 100;
        case ATTRIBUTE_BITDEPTH:
            return 0.5;
        default:
            return 0;
        }
    }

    AmString LofiFilter::GetParamName(AmUInt32 index) const
    {
        if (index >= ATTRIBUTE_LAST)
            return "";

        static constexpr const char* names[ATTRIBUTE_LAST] = { "Wet", "Samplerate", "BitDepth" };

        return names[index];
    }

    AmUInt32 LofiFilter::GetParamType(AmUInt32 index) const
    {
        return kParameterTypeFloat;
    }

    FilterInstance* LofiFilter::CreateInstance()
    {
        return ampoolnew(eMemoryPoolKind_Filtering, LofiFilterInstance, this);
    }

    void LofiFilter::DestroyInstance(FilterInstance* instance)
    {
        ampooldelete(eMemoryPoolKind_Filtering, LofiFilterInstance, (LofiFilterInstance*)instance);
    }

    LofiFilterInstance::LofiFilterInstance(LofiFilter* parent)
        : FilterInstance(parent)
    {
        Initialize(LofiFilter::ATTRIBUTE_LAST);

        m_parameters[LofiFilter::ATTRIBUTE_SAMPLERATE] = parent->_sampleRate;
        m_parameters[LofiFilter::ATTRIBUTE_BITDEPTH] = parent->_bitDepth;

        for (LofiChannelData& i : _channelData)
        {
            i.m_sample = 0;
            i.m_samplesToSkip = 0;
        }
    }

    AmAudioSample LofiFilterInstance::ProcessSample(AmAudioSample sample, AmUInt16 channel, AmUInt32 sampleRate)
    {
        if (_channelData[channel].m_samplesToSkip <= 0)
        {
            _channelData[channel].m_samplesToSkip += (sampleRate / m_parameters[LofiFilter::ATTRIBUTE_SAMPLERATE]) - 1;
            const AmReal32 q = std::pow(2.0f, m_parameters[LofiFilter::ATTRIBUTE_BITDEPTH]);
            _channelData[channel].m_sample = std::floor(q * sample) / q;
        }
        else
        {
            _channelData[channel].m_samplesToSkip--;
        }

        const AmReal32 y = sample + (_channelData[channel].m_sample - sample) * m_parameters[LofiFilter::ATTRIBUTE_WET];

        return static_cast<AmAudioSample>(y);
    }
} // namespace SparkyStudios::Audio::Amplitude
