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

#include <DSP/Resamplers/LibsamplerateResampler.h>

namespace SparkyStudios::Audio::Amplitude
{
    void LibsamplerateResamplerInstance::Initialize(AmUInt16 channelCount, AmUInt32 sampleRateIn, AmUInt32 sampleRateOut)
    {
        _resampler.resize(channelCount);
        for (AmUInt16 c = 0; c < channelCount; c++)
            _resampler[c] = src_new(SRC_SINC_BEST_QUALITY, 1, nullptr);

        _numChannels = channelCount;

        _sampleRateIn = sampleRateIn;
        _sampleRateOut = sampleRateOut;
        _sampleRatio = static_cast<AmReal64>(sampleRateOut) / static_cast<AmReal64>(sampleRateIn);
    }

    bool LibsamplerateResamplerInstance::Process(
        const AudioBuffer& input, AmUInt64& inputFrames, AudioBuffer& output, AmUInt64& outputFrames)
    {
        AMPLITUDE_ASSERT(input.GetChannelCount() == _numChannels);
        AMPLITUDE_ASSERT(output.GetChannelCount() == _numChannels);

        SRC_DATA data;
        for (AmUInt16 c = 0; c < _numChannels; c++)
        {
            data.data_in = input[c].begin();
            data.data_out = output[c].begin();
            data.end_of_input = 0;
            data.input_frames = inputFrames;
            data.output_frames = outputFrames;
            data.src_ratio = _sampleRatio;

            int err = src_process(_resampler[c], &data);

            if (err != 0)
                return false;
        }

        inputFrames = data.input_frames_used;
        outputFrames = data.output_frames_gen;

        return true;
    }

    void LibsamplerateResamplerInstance::SetSampleRate(AmUInt32 sampleRateIn, AmUInt32 sampleRateOut)
    {
        _sampleRateIn = sampleRateIn;
        _sampleRateOut = sampleRateOut;

        _sampleRatio = static_cast<AmReal64>(sampleRateOut) / static_cast<AmReal64>(sampleRateIn);

        for (AmUInt16 c = 0; c < _numChannels; c++)
            src_set_ratio(_resampler[c], _sampleRatio);
    }

    void LibsamplerateResamplerInstance::Reset()
    {
        for (AmUInt16 c = 0; c < _numChannels; c++)
            src_reset(_resampler[c]);
    }

    void LibsamplerateResamplerInstance::Clear()
    {
        if (_resampler.empty())
            return;

        for (AmUInt16 c = 0; c < _numChannels; c++)
            src_delete(_resampler[c]);

        _resampler.clear();
    }

    ResamplerInstance* LibsamplerateResampler::CreateInstance()
    {
        return ampoolnew(eMemoryPoolKind_Filtering, LibsamplerateResamplerInstance);
    }

    void LibsamplerateResampler::DestroyInstance(ResamplerInstance* instance)
    {
        ampooldelete(eMemoryPoolKind_Filtering, LibsamplerateResamplerInstance, (LibsamplerateResamplerInstance*)instance);
    }
} // namespace SparkyStudios::Audio::Amplitude