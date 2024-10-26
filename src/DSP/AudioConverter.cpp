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

#include <SparkyStudios/Audio/Amplitude/DSP/AudioConverter.h>

#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    AudioConverter::AudioConverter()
        : _resampler(nullptr)
        , _channelConversionMode(kChannelConversionModeDisabled)
        , _needResampling(false)
        , _srcInitialized(false)
    {
        _resampler = Resampler::Construct("default");
        Reset();
    }

    AudioConverter::~AudioConverter()
    {
        Resampler::Destruct("default", _resampler);
    }

    bool AudioConverter::Configure(const Settings& settings)
    {
        if (settings.m_sourceChannelCount == settings.m_targetChannelCount)
            _channelConversionMode = kChannelConversionModeDisabled;
        else if (settings.m_sourceChannelCount == 1 && settings.m_targetChannelCount == 2)
            _channelConversionMode = kChannelConversionModeMonoToStereo;
        else if (settings.m_sourceChannelCount == 2 && settings.m_targetChannelCount == 1)
            _channelConversionMode = kChannelConversionModeStereoToMono;
        else
            return false; // Unsupported channel conversion mode

        _needResampling = settings.m_sourceSampleRate != settings.m_targetSampleRate;

        if (_needResampling)
        {
            _resampler->Initialize(settings.m_targetChannelCount, settings.m_sourceSampleRate, settings.m_targetSampleRate);
            _srcInitialized = true;
        }

        _settings = settings;

        return true;
    }

    void AudioConverter::Process(const AudioBuffer& input, AmUInt64& inputFrames, AudioBuffer& output, AmUInt64& outputFrames)
    {
        AudioBuffer temp;
        if (_channelConversionMode == kChannelConversionModeDisabled)
        {
            // No channel conversion required, just copy the input to the output.
            temp = input;
        }
        else if (_channelConversionMode == kChannelConversionModeMonoToStereo)
        {
            temp = AudioBuffer(input.GetFrameCount(), 2);
            ConvertStereoFromMono(input, temp);
        }
        else if (_channelConversionMode == kChannelConversionModeStereoToMono)
        {
            temp = AudioBuffer(input.GetFrameCount(), 1);
            ConvertMonoFromStereo(input, temp);
        }

        AMPLITUDE_ASSERT(temp.GetChannelCount() == output.GetChannelCount());

        if (_needResampling)
            _resampler->Process(temp, inputFrames, output, outputFrames);
        else
            AudioBuffer::Copy(temp, 0, output, 0, outputFrames);
    }

    void AudioConverter::SetSampleRate(AmUInt64 sourceSampleRate, AmUInt64 targetSampleRate)
    {
        _needResampling = sourceSampleRate != targetSampleRate;

        _settings.m_sourceSampleRate = sourceSampleRate;
        _settings.m_targetSampleRate = targetSampleRate;

        if (!_needResampling)
            return;

        if (_srcInitialized)
            _resampler->SetSampleRate(sourceSampleRate, targetSampleRate);
        else
        {
            _resampler->Initialize(_settings.m_targetChannelCount, sourceSampleRate, targetSampleRate);
            _srcInitialized = true;
        }
    }

    AmUInt64 AudioConverter::GetRequiredInputFrameCount(AmUInt64 outputFrameCount) const
    {
        return _resampler->GetRequiredInputFrames(outputFrameCount);
    }

    AmUInt64 AudioConverter::GetExpectedOutputFrameCount(AmUInt64 inputFrameCount) const
    {
        return _resampler->GetExpectedOutputFrames(inputFrameCount);
    }

    AmUInt64 AudioConverter::GetInputLatency() const
    {
        return _resampler->GetInputLatency();
    }

    AmUInt64 AudioConverter::GetOutputLatency() const
    {
        return _resampler->GetOutputLatency();
    }

    void AudioConverter::Reset()
    {
        _channelConversionMode = kChannelConversionModeDisabled;
        _resampler->Reset();
    }

    void AudioConverter::ConvertStereoFromMono(const AudioBuffer& input, AudioBuffer& output)
    {
        AMPLITUDE_ASSERT(input.GetChannelCount() == 1);
        AMPLITUDE_ASSERT(output.GetChannelCount() == 2);
        AMPLITUDE_ASSERT(input.GetFrameCount() == output.GetFrameCount());

        auto& left = output[0];
        auto& right = output[1];
        const auto& mono = input[0];

        ScalarMultiply(mono.begin(), left.begin(), AM_InvSqrtF(2), input.GetFrameCount());
        std::copy_n(left.begin(), input.GetFrameCount(), right.begin());
    }

    void AudioConverter::ConvertMonoFromStereo(const AudioBuffer& input, AudioBuffer& output)
    {
        AMPLITUDE_ASSERT(input.GetChannelCount() == 2);
        AMPLITUDE_ASSERT(output.GetChannelCount() == 1);
        AMPLITUDE_ASSERT(input.GetFrameCount() == output.GetFrameCount());

        const AmSize length = input.GetFrameCount();
        AmSize remaining = input.GetFrameCount();
        const AmReal32 invSqrt2 = AM_InvSqrtF(2);

        const auto& left = input[0];
        const auto& right = input[1];
        auto& mono = output[0];

#if defined(AM_SIMD_INTRINSICS)
        const auto invSqrt2Vec = simd_batch(invSqrt2);
        const AmSize end = GetNumSimdChunks(length);
        constexpr AmSize blockSize = GetSimdBlockSize();

        remaining = remaining - end;

        for (AmSize i = 0; i < end; i += blockSize)
        {
            const auto ba = xsimd::load_aligned<simd_arch>(&left[i]);
            const auto bb = xsimd::load_aligned<simd_arch>(&right[i]);

            auto res = invSqrt2Vec * (ba + bb);
            res.store_aligned(&mono[i]);
        }
#endif

        for (AmSize i = length - remaining; i < length; i++)
            mono[i] = invSqrt2 * (left[i] + right[i]);
    }
} // namespace SparkyStudios::Audio::Amplitude
