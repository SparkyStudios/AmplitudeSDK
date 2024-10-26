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

#include <DSP/Filters/RobotizeFilter.h>
#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    RobotizeFilter::RobotizeFilter()
        : Filter("Robotize")
        , m_frequency(30)
        , m_waveform(0)
    {}

    AmResult RobotizeFilter::Initialize(AmReal32 frequency, AmInt32 waveform)
    {
        m_frequency = frequency;
        m_waveform = waveform;

        return eErrorCode_Success;
    }

    AmUInt32 RobotizeFilter::GetParamCount() const
    {
        return ATTRIBUTE_LAST;
    }

    AmString RobotizeFilter::GetParamName(AmUInt32 index) const
    {
        if (index >= ATTRIBUTE_LAST)
            return "";

        AmString names[3] = { "Wet", "Frequency", "Waveform" };
        return names[index];
    }

    AmUInt32 RobotizeFilter::GetParamType(AmUInt32 index) const
    {
        if (index == ATTRIBUTE_WAVEFORM)
            return kParameterTypeInt;

        return kParameterTypeFloat;
    }

    AmReal32 RobotizeFilter::GetParamMax(AmUInt32 index) const
    {
        if (index == ATTRIBUTE_WAVEFORM)
            return WAVE_LAST - 1;
        if (index == ATTRIBUTE_FREQUENCY)
            return 100.0f;

        return 1.0f;
    }

    AmReal32 RobotizeFilter::GetParamMin(AmUInt32 index) const
    {
        if (index == ATTRIBUTE_FREQUENCY)
            return 0.1f;

        return 0.0f;
    }

    FilterInstance* RobotizeFilter::CreateInstance()
    {
        return ampoolnew(eMemoryPoolKind_Filtering, RobotizeFilterInstance, this);
    }

    void RobotizeFilter::DestroyInstance(FilterInstance* instance)
    {
        ampooldelete(eMemoryPoolKind_Filtering, RobotizeFilterInstance, (RobotizeFilterInstance*)instance);
    }

    RobotizeFilterInstance::RobotizeFilterInstance(RobotizeFilter* parent)
        : FilterInstance(parent)
    {
        Initialize(RobotizeFilter::ATTRIBUTE_LAST);

        _duration = 0.0;
        m_parameters[RobotizeFilter::ATTRIBUTE_FREQUENCY] = parent->m_frequency;
        m_parameters[RobotizeFilter::ATTRIBUTE_WAVEFORM] = static_cast<AmReal32>(parent->m_waveform);
    }

    void RobotizeFilterInstance::AdvanceFrame(AmTime deltaTime)
    {
        _duration += deltaTime;
        FilterInstance::AdvanceFrame(deltaTime);
    }

    void RobotizeFilterInstance::ProcessChannel(
        const AudioBuffer& in, AudioBuffer& out, AmUInt16 channel, AmUInt64 frames, AmUInt32 sampleRate)
    {
        const auto period = static_cast<AmInt32>(static_cast<AmReal32>(sampleRate) / m_parameters[RobotizeFilter::ATTRIBUTE_FREQUENCY]);
        const auto start = static_cast<AmInt32>(_duration * sampleRate) % period;

        const auto& inChannel = in.GetChannel(channel);
        auto& outChannel = out.GetChannel(channel);

        for (AmUInt64 f = 0; f < frames; f++)
        {
            const AmReal32 x = inChannel[f];
            /* */ AmReal32 y;

            const AmReal32 wPos = static_cast<AmReal32>((start + f) % period) / static_cast<AmReal32>(period);

            y = x * (GenerateWaveform(static_cast<AmInt32>(m_parameters[RobotizeFilter::ATTRIBUTE_WAVEFORM]), wPos) + 0.5f);

            y = x + (y - x) * m_parameters[RobotizeFilter::ATTRIBUTE_WET];

            outChannel[f] = static_cast<AmAudioSample>(y);
        }
    }

    AmReal32 RobotizeFilterInstance::GenerateWaveform(AmInt32 waveform, AmReal32 p)
    {
        switch (waveform)
        {
        default:
        case RobotizeFilter::WAVE_SQUARE:
            return p > 0.5f ? 0.5f : -0.5f;
        case RobotizeFilter::WAVE_SAW:
            return p - 0.5f;
        case RobotizeFilter::WAVE_SIN:
            return std::sin(p * M_PI * 2.0f) * 0.5f;
        case RobotizeFilter::WAVE_TRIANGLE:
            return (p > 0.5f ? (1.0f - (p - 0.5f) * 2) : p * 2.0f) - 0.5f;
        case RobotizeFilter::WAVE_BOUNCE:
            return (p < 0.5f ? std::sin(p * M_PI * 2.0f) * 0.5f : -std::sin(p * M_PI * 2.0f) * 0.5f) - 0.5f;
        case RobotizeFilter::WAVE_JAWS:
            return (p < 0.25f ? std::sin(p * M_PI * 2.0f) * 0.5f : 0) - 0.5f;
        case RobotizeFilter::WAVE_HUMPS:
            return (p < 0.5f ? std::sin(p * M_PI * 2.0f) * 0.5f : 0) - 0.5f;
        case RobotizeFilter::WAVE_FSQUARE:
            {
                AmReal32 f = 0;
                for (AmInt32 i = 1; i < 22; i += 2)
                {
                    f += 4.0f / (M_PI * i) * std::sin(2.0f * M_PI * i * p);
                }
                return f * 0.5f;
            }
        case RobotizeFilter::WAVE_FSAW:
            {
                AmReal32 f = 0;
                for (AmInt32 i = 1; i < 15; i++)
                {
                    if (i & 1)
                        f += 1.0f / (M_PI * i) * std::sin(p * 2.0f * M_PI * i);
                    else
                        f -= 1.0f / (M_PI * i) * std::sin(p * 2.0f * M_PI * i);
                }
                return f;
            }
        }
    }
} // namespace SparkyStudios::Audio::Amplitude
