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

#pragma once

#ifndef SS_AMPLITUDE_AUDIO_R8BRAIN_RESAMPLER_H
#define SS_AMPLITUDE_AUDIO_R8BRAIN_RESAMPLER_H

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Utils/Audio/Resampling/CDSPResampler.h>

namespace SparkyStudios::Audio::Amplitude
{
    class R8BrainResamplerInstance final : public ResamplerInstance
    {
        void Initialize(AmUInt16 channelCount, AmUInt32 sampleRateIn, AmUInt32 sampleRateOut) override
        {
            _numChannels = channelCount;

            _resamplers.resize(channelCount);

            SetSampleRate(sampleRateIn, sampleRateOut);
        }

        bool Process(const AudioBuffer& input, AmUInt64& inputFrames, AudioBuffer& output, AmUInt64& outputFrames) override
        {
            AMPLITUDE_ASSERT(input.GetChannelCount() == _numChannels);
            AMPLITUDE_ASSERT(output.GetChannelCount() == _numChannels);

            const AmUniquePtr<eMemoryPoolKind_SoundData, AmReal64> input64(
                static_cast<AmReal64Buffer>(ampoolmalloc(eMemoryPoolKind_SoundData, inputFrames * sizeof(AmReal64))));

            for (AmUInt16 c = 0; c < _numChannels; c++)
            {
                const auto& inChannel = input[c];
                auto& outChannel = output[c];

                r8b::CDSPResampler24* resampler = _resamplers[c].get();

                for (AmUInt64 i = 0; i < inputFrames; i++)
                    input64.get()[i] = static_cast<AmReal64>(inChannel[i]);

                AmReal64Buffer output64 = nullptr;
                const AmUInt64 processedFrames = resampler->process(input64.get(), static_cast<AmInt32>(inputFrames), output64);

                outputFrames = AM_MIN(outputFrames, processedFrames);

                if (output64 == nullptr)
                    return false;

                for (AmUInt64 i = 0; i < outputFrames; i++)
                    outChannel[i] = static_cast<AmReal32>(output64[i]);
            }

            return true;
        }

        void SetSampleRate(AmUInt32 sampleRateIn, AmUInt32 sampleRateOut) override
        {
            _sampleRateIn = sampleRateIn;
            _sampleRateOut = sampleRateOut;
            _sampleRatio = static_cast<AmReal64>(sampleRateOut) / static_cast<AmReal64>(sampleRateIn);

            for (AmUInt16 c = 0; c < _numChannels; c++)
            {
                if (_resamplers[c] != nullptr)
                    _resamplers[c].reset();

                _resamplers[c] = std::make_unique<r8b::CDSPResampler24>(sampleRateIn, sampleRateOut, kAmMaxSupportedFrameCount);
            }
        }

        [[nodiscard]] AmUInt32 GetSampleRateIn() const override
        {
            return _sampleRateIn;
        }

        [[nodiscard]] AmUInt32 GetSampleRateOut() const override
        {
            return _sampleRateOut;
        }

        [[nodiscard]] AmUInt16 GetChannelCount() const override
        {
            return _numChannels;
        }

        [[nodiscard]] AmUInt64 GetRequiredInputFrames(AmUInt64 outputFrameCount) const override
        {
            if (_resamplers.empty())
                return 0;

            return _resamplers[0]->getInputRequiredForOutput(outputFrameCount);
        }

        [[nodiscard]] AmUInt64 GetExpectedOutputFrames(AmUInt64 inputFrameCount) const override
        {
            return std::ceil(_sampleRatio * static_cast<AmReal32>(inputFrameCount));
        }

        [[nodiscard]] AmUInt64 GetInputLatency() const override
        {
            if (_resamplers.empty())
                return 0;

            return _resamplers[0]->getInLenBeforeOutPos(0);
        }

        [[nodiscard]] AmUInt64 GetOutputLatency() const override
        {
            if (_resamplers.empty())
                return 0;

            return _resamplers[0]->getLatency();
        }

        void Reset() override
        {
            if (_resamplers.empty())
                return;

            for (AmUInt16 c = 0; c < _numChannels; c++)
                _resamplers[c]->clear();
        }

        void Clear() override
        {
            if (_resamplers.empty())
                return;

            for (AmUInt16 c = 0; c < _numChannels; c++)
                _resamplers[c].reset();
        }

    private:
        AmUInt16 _numChannels = 0;

        AmUInt32 _sampleRateIn = 0;
        AmUInt32 _sampleRateOut = 0;
        AmReal64 _sampleRatio = 0.0;

        std::vector<std::unique_ptr<r8b::CDSPResampler24>> _resamplers;
    };

    class R8BrainResampler final : public Resampler
    {
    public:
        R8BrainResampler()
            : Resampler("R8Brain")
        {}

        ResamplerInstance* CreateInstance() override
        {
            return ampoolnew(eMemoryPoolKind_Filtering, R8BrainResamplerInstance);
        }

        void DestroyInstance(ResamplerInstance* instance) override
        {
            ampooldelete(eMemoryPoolKind_Filtering, R8BrainResamplerInstance, (R8BrainResamplerInstance*)instance);
        }
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_R8BRAIN_RESAMPLER_H
