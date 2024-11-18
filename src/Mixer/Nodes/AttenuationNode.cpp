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

#include <SparkyStudios/Audio/Amplitude/Mixer/Amplimix.h>

#include <Core/EngineInternalState.h>
#include <DSP/Gain.h>
#include <Mixer/Nodes/AttenuationNode.h>

#include "engine_config_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    constexpr AmReal32 kQ = 0.707107f; // sqrt(0.5)

    void AirAbsorptionEQFilter::Normalize(std::array<AmReal32, kAmAirAbsorptionBandCount>& gains, AmReal32& overallGain)
    {
        if (const auto maxGain = std::max({ gains[0], gains[1], gains[2] }); maxGain < kEpsilon)
        {
            overallGain = 0.0f;
            for (auto i = 0; i < kAmAirAbsorptionBandCount; ++i)
                gains[i] = 1.0f;
        }
        else
        {
            for (auto i = 0; i < kAmAirAbsorptionBandCount; ++i)
            {
                constexpr AmReal32 kMaxEQGain = 0.0625f;

                gains[i] /= maxGain;
                gains[i] = std::max(gains[i], kMaxEQGain);
            }

            overallGain *= maxGain;
        }
    }

    AirAbsorptionEQFilter::AirAbsorptionEQFilter()
        : _eqFilterFactory()
        , _lowShelfFilter{ nullptr, nullptr }
        , _peakingFilter{ nullptr, nullptr }
        , _highShelfFilter{ nullptr, nullptr }
        , _currentSet(0)
        , _needUpdateGains(false)
    {
        EnsureFilters();
    }

    AirAbsorptionEQFilter::~AirAbsorptionEQFilter()
    {
        for (AmUInt32 i = 0; i < 2; ++i)
        {
            if (_lowShelfFilter[i] != nullptr)
            {
                _eqFilterFactory.DestroyInstance(_lowShelfFilter[i]);
                _lowShelfFilter[i] = nullptr;
            }

            if (_peakingFilter[i] != nullptr)
            {
                _eqFilterFactory.DestroyInstance(_peakingFilter[i]);
                _peakingFilter[i] = nullptr;
            }

            if (_highShelfFilter[i] != nullptr)
            {
                _eqFilterFactory.DestroyInstance(_highShelfFilter[i]);
                _highShelfFilter[i] = nullptr;
            }
        }
    }

    void AirAbsorptionEQFilter::SetGains(AmReal32 gainLow, AmReal32 gainMid, AmReal32 gainHigh)
    {
        const AmReal32 oldGainLow = _lowShelfFilter[_currentSet]->GetParameter(BiquadResonantFilter::ATTRIBUTE_GAIN);
        const AmReal32 oldGainMid = _peakingFilter[_currentSet]->GetParameter(BiquadResonantFilter::ATTRIBUTE_GAIN);
        const AmReal32 oldGainHigh = _highShelfFilter[_currentSet]->GetParameter(BiquadResonantFilter::ATTRIBUTE_GAIN);

        if (std::abs(gainLow - oldGainLow) > kEpsilon)
        {
            _lowShelfFilter[_currentSet]->SetParameter(BiquadResonantFilter::ATTRIBUTE_GAIN, gainLow);
            _needUpdateGains = true;
        }

        if (std::abs(gainMid - oldGainMid) > kEpsilon)
        {
            _peakingFilter[_currentSet]->SetParameter(BiquadResonantFilter::ATTRIBUTE_GAIN, gainMid);
            _needUpdateGains = true;
        }

        if (std::abs(gainHigh - oldGainHigh) > kEpsilon)
        {
            _highShelfFilter[_currentSet]->SetParameter(BiquadResonantFilter::ATTRIBUTE_GAIN, gainHigh);
            _needUpdateGains = true;
        }
    }

    void AirAbsorptionEQFilter::Process(const AudioBuffer& input, AudioBuffer& output, AmReal32 sampleRate)
    {
        if (_needUpdateGains)
        {
            const AmUInt32 previousSet = _currentSet;
            _currentSet = 1 - _currentSet;

            _lowShelfFilter[_currentSet]->SetParameter(
                BiquadResonantFilter::ATTRIBUTE_FREQUENCY,
                _lowShelfFilter[previousSet]->GetParameter(BiquadResonantFilter::ATTRIBUTE_FREQUENCY));
            _lowShelfFilter[_currentSet]->SetParameter(
                BiquadResonantFilter::ATTRIBUTE_RESONANCE,
                _lowShelfFilter[previousSet]->GetParameter(BiquadResonantFilter::ATTRIBUTE_RESONANCE));
            _lowShelfFilter[_currentSet]->SetParameter(
                BiquadResonantFilter::ATTRIBUTE_GAIN, _lowShelfFilter[previousSet]->GetParameter(BiquadResonantFilter::ATTRIBUTE_GAIN));

            _peakingFilter[_currentSet]->SetParameter(
                BiquadResonantFilter::ATTRIBUTE_FREQUENCY,
                _peakingFilter[previousSet]->GetParameter(BiquadResonantFilter::ATTRIBUTE_FREQUENCY));
            _peakingFilter[_currentSet]->SetParameter(
                BiquadResonantFilter::ATTRIBUTE_RESONANCE,
                _peakingFilter[previousSet]->GetParameter(BiquadResonantFilter::ATTRIBUTE_RESONANCE));
            _peakingFilter[_currentSet]->SetParameter(
                BiquadResonantFilter::ATTRIBUTE_GAIN, _peakingFilter[previousSet]->GetParameter(BiquadResonantFilter::ATTRIBUTE_GAIN));

            _highShelfFilter[_currentSet]->SetParameter(
                BiquadResonantFilter::ATTRIBUTE_FREQUENCY,
                _highShelfFilter[previousSet]->GetParameter(BiquadResonantFilter::ATTRIBUTE_FREQUENCY));
            _highShelfFilter[_currentSet]->SetParameter(
                BiquadResonantFilter::ATTRIBUTE_RESONANCE,
                _highShelfFilter[previousSet]->GetParameter(BiquadResonantFilter::ATTRIBUTE_RESONANCE));
            _highShelfFilter[_currentSet]->SetParameter(
                BiquadResonantFilter::ATTRIBUTE_GAIN, _highShelfFilter[previousSet]->GetParameter(BiquadResonantFilter::ATTRIBUTE_GAIN));

            AudioBuffer temp(input.GetFrameCount(), input.GetChannelCount());

            ApplyFilters(previousSet, input, temp, sampleRate);
            ApplyFilters(_currentSet, input, output, sampleRate);

            auto& o = output[0];
            auto& t = temp[0];

            for (AmUInt32 i = 0, l = input.GetFrameCount(); i < l; ++i)
            {
                AmReal32 weight = static_cast<AmReal32>(i) / static_cast<AmReal32>(l);
                o[i] = weight * o[i] + (1.0f - weight) * t[i];
            }

            _needUpdateGains = false;
        }
        else
        {
            ApplyFilters(_currentSet, input, output, sampleRate);
        }
    }

    void AirAbsorptionEQFilter::EnsureFilters()
    {
        for (AmUInt32 i = 0; i < 2; ++i)
        {
            if (_lowShelfFilter[i] == nullptr)
            {
                _eqFilterFactory.InitializeLowShelf(kHighCutoffFrequencies[0], kQ, 0.0f);
                _lowShelfFilter[i] = _eqFilterFactory.CreateInstance();
            }

            if (_peakingFilter[i] == nullptr)
            {
                const AmReal32 cutoffFrequency = AM_SqrtF(kLowCutoffFrequencies[1] * kHighCutoffFrequencies[1]);
                _eqFilterFactory.InitializePeaking(
                    cutoffFrequency, cutoffFrequency / (kHighCutoffFrequencies[1] - kLowCutoffFrequencies[1]), 0.0f);
                _peakingFilter[i] = _eqFilterFactory.CreateInstance();
            }

            if (_highShelfFilter[i] == nullptr)
            {
                _eqFilterFactory.InitializeHighShelf(kLowCutoffFrequencies[2], kQ, 0.0f);
                _highShelfFilter[i] = _eqFilterFactory.CreateInstance();
            }
        }
    }

    void AirAbsorptionEQFilter::ApplyFilters(AmUInt32 set, const AudioBuffer& input, AudioBuffer& output, AmReal32 sampleRate)
    {
        _lowShelfFilter[set]->Process(input, output, input.GetFrameCount(), sampleRate);
        _peakingFilter[set]->Process(output, output, input.GetFrameCount(), sampleRate);
        _highShelfFilter[set]->Process(output, output, input.GetFrameCount(), sampleRate);
    }

    AttenuationNodeInstance::AttenuationNodeInstance()
        : ProcessorNodeInstance(false)
        , _output()
        , _gains{ 1.0f, 1.0f, 1.0f }
        , _eqFilter()
    {}

    const AudioBuffer* AttenuationNodeInstance::Process(const AudioBuffer* input)
    {
        const auto* layer = GetLayer();

        const Attenuation* attenuation = layer->GetAttenuation();
        if (attenuation == nullptr)
            return input;

        const Listener& listener = layer->GetListener();

        AmReal32 targetGain = 1.0f;

        // Compute attenuated gain based on spatialization
        {
            const eSpatialization spatialization = layer->GetSpatialization();

            if (listener.Valid())
            {
                const Entity& entity = layer->GetEntity();

                if (spatialization == eSpatialization_PositionOrientation)
                {
                    AMPLITUDE_ASSERT(entity.Valid());
                    targetGain *= attenuation->GetGain(entity, listener);
                }
                else if (spatialization == eSpatialization_HRTF && entity.Valid())
                {
                    targetGain *= attenuation->GetGain(entity, listener);
                }
                else if (spatialization == eSpatialization_Position)
                {
                    const AmVec3& location = layer->GetLocation();

                    // Position-based spatialization, or HRTF-based spatialization without entity
                    targetGain *= attenuation->GetGain(location, listener);
                }
            }
            else
            {
                // No sound without listener on an attenuated source
                targetGain = 0.0f;
            }
        }

        if (Gain::IsZero(targetGain))
            return nullptr;

        // Set and normalize gains
        if (attenuation->IsAirAbsorptionEnabled() && listener.Valid())
        {
            const AmVec3& soundLocation = layer->GetLocation();
            const AmVec3& listenerLocation = listener.GetLocation();

            for (AmUInt32 i = 0; i < kAmAirAbsorptionBandCount; ++i)
                _gains[i] = attenuation->EvaluateAirAbsorption(soundLocation, listenerLocation, i);

            AirAbsorptionEQFilter::Normalize(_gains, targetGain);
            _eqFilter.SetGains(_gains[0], _gains[1], _gains[2]);
        }

        _output = *input;

        // Apply gain attenuation
        if (!Gain::IsOne(targetGain))
            for (AmSize c = 0; c < _output.GetChannelCount(); ++c)
                Gain::ApplyReplaceConstantGain(targetGain, input->GetChannel(c), 0, _output[c], 0, input->GetFrameCount());

        // Apply air absorption EQ filter
        if (attenuation->IsAirAbsorptionEnabled() && listener.Valid())
            _eqFilter.Process(_output, _output, layer->GetSampleRate());

        return &_output;
    }

    AttenuationNode::AttenuationNode()
        : Node("Attenuation")
    {}
} // namespace SparkyStudios::Audio::Amplitude
