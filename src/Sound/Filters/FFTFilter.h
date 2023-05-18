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

#ifndef SS_AMPLITUDE_AUDIO_FFT_FILTER_H
#define SS_AMPLITUDE_AUDIO_FFT_FILTER_H

#include <SparkyStudios/Audio/Amplitude/Sound/Filter.h>

#include <Utils/pffft/pffft_double.h>

namespace SparkyStudios::Audio::Amplitude
{
    class FFTFilterInstance;

    class FFTFilter : public Filter
    {
        friend class FFTFilterInstance;

    public:
        explicit FFTFilter(const std::string& name);
        ~FFTFilter() override = default;

        FilterInstance* CreateInstance() override;
        void DestroyInstance(FilterInstance* instance) override;
    };

    class FFTFilterInstance : public FilterInstance
    {
    public:
        explicit FFTFilterInstance(FFTFilter* parent);
        ~FFTFilterInstance() override;

        void ProcessChannel(
            AmAudioSampleBuffer buffer, AmUInt16 channel, AmUInt64 frames, AmUInt16 channels, AmUInt32 sampleRate, bool isInterleaved)
            override;

        virtual void ProcessFFTChannel(AmReal64Buffer buffer, AmUInt16 channel, AmUInt64 frames, AmUInt16 channels, AmUInt32 sampleRate);

        void Comp2MagPhase(AmReal64Buffer buffer, AmUInt32 samples);
        void MagPhase2MagFreq(AmReal64Buffer buffer, AmUInt32 samples, AmUInt32 sampleRate, AmUInt16 channel);
        void MagFreq2MagPhase(AmReal64Buffer buffer, AmUInt32 samples, AmUInt32 sampleRate, AmUInt16 channel);

        static void MagPhase2Comp(AmReal64Buffer buffer, AmUInt32 samples);

        void InitFFT();

    private:
        PFFFTD_Setup* _pffft_setup = nullptr;

        AmReal64Buffer _temp = nullptr;
        AmReal64Buffer _inputBuffer = nullptr;
        AmReal64Buffer _mixBuffer = nullptr;
        AmReal64Buffer _lastPhase = nullptr;
        AmReal64Buffer _sumPhase = nullptr;

        AmUInt32 _inputOffset[AM_MAX_CHANNELS]{};
        AmUInt32 _mixOffset[AM_MAX_CHANNELS]{};
        AmUInt32 _readOffset[AM_MAX_CHANNELS]{};
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_FFT_FILTER_H
