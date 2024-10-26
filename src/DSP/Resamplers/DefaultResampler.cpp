// Copyright 2018 Google Inc. All Rights Reserved.
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

#include <numeric>

#include <SparkyStudios/Audio/Amplitude/Math/Utils.h>

#include <DSP/Resamplers/DefaultResampler.h>
#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    // The value below was chosen empirically as a tradeoff between execution time
    // and filter rolloff wrt. cutoff frequency.
    constexpr AmUInt64 kTransitionBandwidthRatio = 13;

    bool DefaultResamplerInstance::IsConversionSupported(AmUInt64 source, AmUInt64 destination)
    {
        AMPLITUDE_ASSERT(source > 0 && destination > 0);

        // Determines whether sample rates are supported based upon whether our
        // maximum filter length is big enough to hold the corresponding
        // interpolation filter.
        const AmInt64 maxRate = std::max(source, destination) / FindGCD(source, destination);
        AmUInt64 filterLength = maxRate * kTransitionBandwidthRatio;
        filterLength += filterLength % 2;

        return filterLength <= kAmMaxSupportedFrameCount;
    }

    DefaultResamplerInstance::DefaultResamplerInstance()
        : _upRate(0)
        , _downRate(0)
        , _timeModuloUpRate(0)
        , _lastProcessedSample(0)
        , _channelCount(0)
        , _coefficientsPerPhase(0)
        , _transposedFilterCoefficients(kAmMaxSupportedFrameCount, 1)
        , _temporaryFilterCoefficients(kAmMaxSupportedFrameCount, 1)
        , _state(kAmMaxSupportedFrameCount, kAmMaxSupportedChannelCount)
    {
        _state.Clear();
    }

    bool DefaultResamplerInstance::Process(const AudioBuffer& input, AmUInt64& inputFrames, AudioBuffer& output, AmUInt64& outputFrames)
    {
        // See "Digital Signal Processing", 4th Edition, Prolakis and Manolakis,
        // Pearson, Chapter 11 (specifically Figures 11.5.10 and 11.5.13).

        AMPLITUDE_ASSERT(input.GetChannelCount() == _channelCount);
        AMPLITUDE_ASSERT(output.GetChannelCount() == _channelCount);

        // AMPLITUDE_ASSERT(output.GetFrameCount() >= GetExpectedOutputFrames(inputFrames));
        // AMPLITUDE_ASSERT(output.GetFrameCount() <= GetMaxOutputLength(inputFrames));

        output.Clear();

        if (IsIdentity())
        {
            output = input;
            return true;
        }

        AmUInt64 inputSample = _lastProcessedSample;
        AmUInt64 outputSample = 0;

        const auto& filterCoeffs = _transposedFilterCoefficients[0];

        while (inputSample < inputFrames && outputSample < outputFrames)
        {
            AmUInt64 filterIndex = _timeModuloUpRate * _coefficientsPerPhase;
            AmUInt64 offsetInputIndex = inputSample - _coefficientsPerPhase + 1;
            const AmInt64 offset = -static_cast<AmInt64>(offsetInputIndex);

            if (offset > 0)
            {
                // We will need to draw data from the _state buffer.
                const AmInt64 stateFrameCount = static_cast<AmInt64>(_coefficientsPerPhase - 1);
                AmInt64 stateIndex = stateFrameCount - offset;

                while (stateIndex < stateFrameCount)
                {
                    for (AmUInt64 channel = 0; channel < _channelCount; ++channel)
                        output[channel][outputSample] += _state[channel][stateIndex] * filterCoeffs[filterIndex];

                    stateIndex++;
                    filterIndex++;
                }

                // Move along by offset samples up as far as input.
                offsetInputIndex += offset;
            }

            // We now move back to where inputSample "points".
            while (offsetInputIndex <= inputSample)
            {
                for (AmUInt64 channel = 0; channel < _channelCount; ++channel)
                    output[channel][outputSample] += input[channel][offsetInputIndex] * filterCoeffs[filterIndex];

                offsetInputIndex++;
                filterIndex++;
            }

            outputSample++;

            _timeModuloUpRate += _downRate;
            // Advance the input pointer.
            inputSample += _timeModuloUpRate / _upRate;
            // Decide which phase of the polyphase filter to use next.
            _timeModuloUpRate %= _upRate;
        }

        AMPLITUDE_ASSERT(inputSample >= inputFrames || outputSample >= outputFrames);
        _lastProcessedSample = AM_MAX(inputSample, inputFrames) - inputFrames;

        // Take care of the state buffer.
        if (const AmInt64 remainingSamples = static_cast<AmInt64>(_coefficientsPerPhase) - 1 - static_cast<AmInt64>(inputFrames);
            remainingSamples > 0)
        {
            for (AmUInt64 channel = 0; channel < _channelCount; ++channel)
            {
                // Copy end of the state buffer to the beginning.
                auto& stateChannel = _state[channel];
                AMPLITUDE_ASSERT(static_cast<AmInt64>(stateChannel.size()) >= remainingSamples);
                std::copy_n(stateChannel.end() - remainingSamples, remainingSamples, stateChannel.begin());

                // Then copy input to the end of the buffer.
                std::copy_n(input[channel].begin(), inputFrames, stateChannel.end() - inputFrames);
            }
        }
        else
        {
            for (AmUInt64 channel = 0; channel < _channelCount; ++channel)
            {
                AMPLITUDE_ASSERT(_coefficientsPerPhase > 0U);
                AMPLITUDE_ASSERT(input[channel].size() > _coefficientsPerPhase - 1);

                // Copy the last of the input samples into the state buffer.
                std::copy_n(input[channel].end() - (_coefficientsPerPhase - 1), _coefficientsPerPhase - 1, _state[channel].begin());
            }
        }

        inputFrames = inputSample;
        outputFrames = outputSample;

        return true;
    }

    AmUInt64 DefaultResamplerInstance::GetMaxOutputLength(AmUInt64 inputLength) const
    {
        if (IsIdentity())
            return inputLength;

        AMPLITUDE_ASSERT(_downRate > 0 && _upRate > 0);

        // The + 1 takes care of the case where:
        // (_timeModuloUpRate + _upRate * _lastProcessedSample) < ((inputLength * _upRate) % _downRate)
        // The output length will be equal to the return value or the return value -1.
        return (inputLength * _upRate) / _downRate + 1;
    }

    AmUInt64 DefaultResamplerInstance::GetExpectedOutputFrames(AmUInt64 inputLength) const
    {
        if (IsIdentity())
            return inputLength;

        const AmUInt64 maxLength = GetMaxOutputLength(inputLength);
        if ((_timeModuloUpRate + _upRate * _lastProcessedSample) >= ((inputLength * _upRate) % _downRate))
            return maxLength - 1;

        return maxLength;
    }

    AmUInt64 DefaultResamplerInstance::GetRequiredInputFrames(AmUInt64 outputLength) const
    {
        if (IsIdentity())
            return outputLength;

        return (outputLength * _downRate) / _upRate;
    }

    void DefaultResamplerInstance::Initialize(AmUInt16 channelCount, AmUInt32 sampleRateIn, AmUInt32 sampleRateOut)
    {
        // Convert sampling rates to be relatively prime.
        AMPLITUDE_ASSERT(sampleRateIn > 0);
        AMPLITUDE_ASSERT(sampleRateOut > 0);
        AMPLITUDE_ASSERT(channelCount > 0);

        const AmInt64 gcd = FindGCD(sampleRateOut, sampleRateIn);
        const AmUInt64 destination = static_cast<AmUInt64>(sampleRateOut / gcd);
        const AmUInt64 source = static_cast<AmUInt64>(sampleRateIn / gcd);

        // Obtain the size of the _state before _coefficientsPerPhase is updated in GenerateInterpolatingFilter().
        const AmUInt64 oldStateSize = _coefficientsPerPhase > 0 ? _coefficientsPerPhase - 1 : 0;
        if ((destination != _upRate) || (source != _downRate))
        {
            _upRate = destination;
            _downRate = source;

            _sampleRateIn = sampleRateIn;
            _sampleRateOut = sampleRateOut;

            if (IsIdentity())
            {
                _channelCount = channelCount;
                return;
            }

            // Create transposed multirate filters from sincs.
            GenerateInterpolatingFilter(sampleRateIn);

            // Reset the time variable as it may be longer than the new filter length if
            // we switched from upsampling to downsampling.
            _timeModuloUpRate = 0;
        }

        // Update the state buffer.
        if (_channelCount != channelCount)
        {
            _channelCount = channelCount;
            InitializeStateBuffer(oldStateSize);
        }
    }

    void DefaultResamplerInstance::Reset()
    {
        _timeModuloUpRate = 0;
        _lastProcessedSample = 0;
        _state.Clear();
    }

    void DefaultResamplerInstance::SetSampleRate(AmUInt32 sampleRateIn, AmUInt32 sampleRateOut)
    {
        Initialize(_channelCount, sampleRateIn, sampleRateOut);
    }

    void DefaultResamplerInstance::Clear()
    {
        Reset();

        _upRate = 0;
        _downRate = 0;
        _channelCount = 0;
        _coefficientsPerPhase = 0;
        _transposedFilterCoefficients.Clear();
        _temporaryFilterCoefficients.Clear();

        _sampleRateIn = 0;
        _sampleRateOut = 0;
    }

    void DefaultResamplerInstance::InitializeStateBuffer(AmUInt64 oldFrameCount)
    {
        // Update the state buffer if it is null or if the number of coefficients per phase in the polyphase filter has changed.
        if (IsIdentity() || _channelCount == 0)
            return;

        // If the state buffer is to be kept. For example in the case of a change
        // in either source or destination sampling rate, maintaining the old state
        // buffers contents allows a glitch free transition.
        const AmUInt64 newFrameCount = _coefficientsPerPhase > 0 ? _coefficientsPerPhase - 1 : 0;
        if (oldFrameCount != newFrameCount)
        {
            const AmUInt64 minSize = std::min(newFrameCount, oldFrameCount);
            const AmUInt64 maxSize = std::max(newFrameCount, oldFrameCount);

            for (AmUInt64 channel = 0; channel < _channelCount; ++channel)
            {
                auto& state_channel = _state[channel];
                AMPLITUDE_ASSERT(state_channel.begin() + maxSize < state_channel.end());
                std::fill(state_channel.begin() + minSize, state_channel.begin() + maxSize, 0.0f);
            }
        }
    }

    void DefaultResamplerInstance::GenerateInterpolatingFilter(AmUInt64 sampleRate)
    {
        // See "Digital Signal Processing", 4th Edition, Prolakis and Manolakis,
        // Pearson, Chapter 11 (specifically Figures 11.5.10 and 11.5.13).
        const AmUInt64 maxRate = std::max(_upRate, _downRate);
        const AmReal32 cutoffFrequency = static_cast<AmReal32>(sampleRate) / static_cast<AmReal32>(2 * maxRate);

        AmUInt64 filterLength = maxRate * kTransitionBandwidthRatio;
        filterLength += filterLength % 2;

        auto* filterChannel = &_temporaryFilterCoefficients[0];
        filterChannel->clear();

        GenerateSincFilter(cutoffFrequency, static_cast<AmReal32>(sampleRate), filterLength, filterChannel);

        // Pad out the filter length so that it can be arranged in polyphase fashion.
        const AmUInt64 transposedLength = filterLength + maxRate - (filterLength % maxRate);
        _coefficientsPerPhase = transposedLength / maxRate;

        ArrangeFilterAsPolyphase(filterLength, *filterChannel);
    }

    void DefaultResamplerInstance::ArrangeFilterAsPolyphase(AmUInt64 filterLength, const AudioBufferChannel& filter)
    {
        // Coefficients are transposed and flipped.
        // Suppose _upRate is 3, and the input number of coefficients is 10,
        // h[0], ..., h[9].
        // Then the _transposedFilterCoefficients buffer will look like this:
        // h[9], h[6], h[3], h[0],   flipped phase 0 coefs.
        //  0,   h[7], h[4], h[1],   flipped phase 1 coefs (zero-padded).
        //  0,   h[8], h[5], h[2],   flipped phase 2 coefs (zero-padded).
        _transposedFilterCoefficients.Clear();
        auto& transposedCoefficientsChannel = _transposedFilterCoefficients[0];

        for (AmUInt64 i = 0; i < _upRate; ++i)
        {
            for (AmUInt64 j = 0; j < _coefficientsPerPhase; ++j)
            {
                if (j * _upRate + i >= filterLength)
                    continue;

                const AmUInt64 coeffIndex = (_coefficientsPerPhase - 1 - j) + i * _coefficientsPerPhase;
                transposedCoefficientsChannel[coeffIndex] = filter[j * _upRate + i];
            }
        }
    }

    void DefaultResamplerInstance::GenerateSincFilter(
        AmReal32 cutoffFrequency, AmUInt64 sampleRate, AmUInt64 filterLength, AudioBufferChannel* filter)
    {
        AMPLITUDE_ASSERT(sampleRate > 0.0f);
        const AmReal32 angularCutoffFrequency = 2.0f * AM_PI32 * cutoffFrequency / sampleRate;

        const size_t half_filter_length = filterLength / 2;
        GenerateHannWindow(true, filterLength, filter);
        auto* filterChannel = &filter[0];

        for (size_t i = 0; i < filterLength; ++i)
        {
            if (i == half_filter_length)
            {
                (*filterChannel)[half_filter_length] *= angularCutoffFrequency;
            }
            else
            {
                const AmReal32 denominator = static_cast<AmReal32>(i) - (static_cast<AmReal32>(filterLength) / 2.0f);
                AMPLITUDE_ASSERT(std::abs(denominator) > kEpsilon);
                (*filterChannel)[i] *= std::sin(angularCutoffFrequency * denominator) / denominator;
            }
        }

        // Normalize.
        const AmReal32 normalizingFactor =
            static_cast<AmReal32>(_upRate) / std::accumulate(filterChannel->begin(), filterChannel->end(), 0.0f);
        ScalarMultiply(filterChannel->begin(), filterChannel->begin(), normalizingFactor, filterLength);
    }

    ResamplerInstance* DefaultResampler::CreateInstance()
    {
        return ampoolnew(eMemoryPoolKind_Filtering, DefaultResamplerInstance);
    }

    void DefaultResampler::DestroyInstance(ResamplerInstance* instance)
    {
        ampooldelete(eMemoryPoolKind_Filtering, DefaultResamplerInstance, (DefaultResamplerInstance*)instance);
    }
} // namespace SparkyStudios::Audio::Amplitude
