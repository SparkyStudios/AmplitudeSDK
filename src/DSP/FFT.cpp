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
#include <SparkyStudios/Audio/Amplitude/DSP/FFT.h>

#include <Utils/Audio/FFT/AudioFFT.h>

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude
{
    AmUInt64 FFT::GetOutputSize(const AmUInt64 inputSize)
    {
        return AudioFFT::ComplexSize(inputSize);
    }

    FFT::FFT()
    {
        _implementation = ampoolnew(eMemoryPoolKind_Filtering, AudioFFT);
    }

    FFT::~FFT()
    {
        ampooldelete(eMemoryPoolKind_Filtering, AudioFFT, (AudioFFT*)_implementation);
    }

    void FFT::Initialize(const AmSize size) const
    {
        static_cast<AudioFFT*>(_implementation)->init(size);
    }

    void FFT::Forward(AmConstAudioSampleBuffer input, SplitComplex& splitComplex) const
    {
        splitComplex.Resize(GetOutputSize(static_cast<AudioFFT*>(_implementation)->size()));
        static_cast<AudioFFT*>(_implementation)->fft(input, splitComplex.re(), splitComplex.im());
    }

    void FFT::Backward(AmAudioSampleBuffer output, SplitComplex& splitComplex) const
    {
        splitComplex.Resize(GetOutputSize(static_cast<AudioFFT*>(_implementation)->size()));
        static_cast<AudioFFT*>(_implementation)->ifft(output, splitComplex.re(), splitComplex.im());
    }
} // namespace SparkyStudios::Audio::Amplitude
