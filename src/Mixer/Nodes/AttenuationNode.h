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

#pragma once

#ifndef _AM_IMPLEMENTATION_MIXER_NODES_ATTENUATION_NODE_H
#define _AM_IMPLEMENTATION_MIXER_NODES_ATTENUATION_NODE_H

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>
#include <SparkyStudios/Audio/Amplitude/Mixer/Node.h>

#include <DSP/Filters/BiquadResonantFilter.h>

namespace SparkyStudios::Audio::Amplitude
{
    class AmplimixLayer;

    class AirAbsorptionEQFilter final
    {
    public:
        static void Normalize(std::array<AmReal32, kAmAirAbsorptionBandCount>& gains, AmReal32& overallGain);

        AirAbsorptionEQFilter();
        ~AirAbsorptionEQFilter();

        void SetGains(AmReal32 gainLow, AmReal32 gainMid, AmReal32 gainHigh);

        void Process(const AudioBuffer& input, AudioBuffer& output, AmReal32 sampleRate);

    private:
        void EnsureFilters();
        void ApplyFilters(AmUInt32 set, const AudioBuffer& input, AudioBuffer& output, AmReal32 sampleRate);

        BiquadResonantFilter _eqFilterFactory;

        FilterInstance* _lowShelfFilter[2];
        FilterInstance* _peakingFilter[2];
        FilterInstance* _highShelfFilter[2];

        AmUInt32 _currentSet;
        bool _needUpdateGains;
    };

    class AttenuationNodeInstance final : public ProcessorNodeInstance
    {
    public:
        AttenuationNodeInstance();

        const AudioBuffer* Process(const AudioBuffer* input) override;

    private:
        AudioBuffer _output;

        std::array<AmReal32, kAmAirAbsorptionBandCount> _gains;
        AirAbsorptionEQFilter _eqFilter;
    };

    class AttenuationNode final : public Node
    {
    public:
        AttenuationNode();

        [[nodiscard]] AM_INLINE NodeInstance* CreateInstance() const override
        {
            return ampoolnew(eMemoryPoolKind_Amplimix, AttenuationNodeInstance);
        }

        AM_INLINE void DestroyInstance(NodeInstance* instance) const override
        {
            ampooldelete(eMemoryPoolKind_Amplimix, AttenuationNodeInstance, (AttenuationNodeInstance*)instance);
        }

        [[nodiscard]] AM_INLINE bool CanConsume() const override
        {
            return true;
        }

        [[nodiscard]] AM_INLINE bool CanProduce() const override
        {
            return true;
        }

        [[nodiscard]] AM_INLINE AmSize GetMaxInputCount() const override
        {
            return 1;
        }

        [[nodiscard]] AM_INLINE AmSize GetMinInputCount() const override
        {
            return 1;
        }
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_MIXER_NODES_ATTENUATION_NODE_H