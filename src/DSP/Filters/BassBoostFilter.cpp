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

#include <DSP/Filters/BassBoostFilter.h>

namespace SparkyStudios::Audio::Amplitude
{
    BassBoostFilter::BassBoostFilter()
        : FFTFilter("BassBoost")
        , m_boost(2.0f)
    {}

    AmResult BassBoostFilter::Initialize(AmReal32 aBoost)
    {
        if (aBoost < 0)
            return eErrorCode_InvalidParameter;

        m_boost = aBoost;
        return eErrorCode_Success;
    }

    AmUInt32 BassBoostFilter::GetParamCount() const
    {
        return ATTRIBUTE_LAST;
    }

    AmString BassBoostFilter::GetParamName(AmUInt32 index) const
    {
        if (index == ATTRIBUTE_BOOST)
            return "Boost";

        return "Wet";
    }

    AmUInt32 BassBoostFilter::GetParamType(AmUInt32 index) const
    {
        return kParameterTypeFloat;
    }

    AmReal32 BassBoostFilter::GetParamMax(AmUInt32 index) const
    {
        if (index == ATTRIBUTE_BOOST)
            return 10.0f;

        return 1.0f;
    }

    AmReal32 BassBoostFilter::GetParamMin(AmUInt32 index) const
    {
        return 0.0f;
    }

    FilterInstance* BassBoostFilter::CreateInstance()
    {
        return ampoolnew(eMemoryPoolKind_Filtering, BassBoostFilterInstance, this);
    }

    void BassBoostFilter::DestroyInstance(FilterInstance* instance)
    {
        ampooldelete(eMemoryPoolKind_Filtering, BassBoostFilterInstance, (BassBoostFilterInstance*)instance);
    }

    BassBoostFilterInstance::BassBoostFilterInstance(BassBoostFilter* parent)
        : FFTFilterInstance(parent)
    {
        Initialize(BassBoostFilter::ATTRIBUTE_LAST);
        m_parameters[BassBoostFilter::ATTRIBUTE_BOOST] = parent->m_boost;
    }

    void BassBoostFilterInstance::ProcessFFTChannel(
        SplitComplex& fft, AmUInt16 channel, AmUInt64 frames, AmUInt16 channels, AmUInt32 sampleRate)
    {
        frames = 2;

        Comp2MagPhase(fft, frames);

        for (AmUInt32 i = 0; i < frames; i++)
            fft.re()[i] *= m_parameters[BassBoostFilter::ATTRIBUTE_BOOST];

        MagPhase2Comp(fft, frames);
    }
} // namespace SparkyStudios::Audio::Amplitude
