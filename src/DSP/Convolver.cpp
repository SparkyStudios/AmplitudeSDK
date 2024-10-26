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
//
// Based on the code from https://github.com/HiFi-LoFi/FFTConvolver
// Copyright (c) 2017 HiFi-LoFi, MIT License

#include <SparkyStudios/Audio/Amplitude/DSP/Convolver.h>

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>

#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    Convolver::Convolver()
        : _blockSize(0)
        , _segSize(0)
        , _segCount(0)
        , _fftComplexSize(0)
        , _segments()
        , _segmentsIR()
        , _fftBuffer()
        , _fft()
        , _preMultiplied()
        , _conv()
        , _overlap()
        , _current(0)
        , _inputBuffer()
        , _inputBufferFill(0)
    {}

    Convolver::~Convolver()
    {
        Reset();
    }

    void Convolver::Reset()
    {
        for (AmSize i = 0; i < _segCount; ++i)
        {
            ampooldelete(eMemoryPoolKind_Filtering, SplitComplex, _segments[i]);
            ampooldelete(eMemoryPoolKind_Filtering, SplitComplex, _segmentsIR[i]);
        }

        _blockSize = 0;
        _segSize = 0;
        _segCount = 0;
        _fftComplexSize = 0;
        _segments.clear();
        _segmentsIR.clear();
        _fftBuffer.Release();
        _fft.Initialize(0);
        _preMultiplied.Release();
        _conv.Release();
        _overlap.Release();
        _current = 0;
        _inputBuffer.Release();
        _inputBufferFill = 0;
    }

    AmSize Convolver::GetSegmentSize() const
    {
        return _segSize;
    }

    AmSize Convolver::GetSegmentCount() const
    {
        return _segCount;
    }

    bool Convolver::Init(AmSize blockSize, const AmAudioSample* ir, AmSize irLen)
    {
        Reset();

        if (blockSize == 0)
            return false;

        // Ignore zeros at the end of the impulse response because they only waste computation time
        while (irLen > 0 && std::fabs(ir[irLen - 1]) < 0.000001f)
            --irLen;

        if (irLen == 0)
            return true;

        _blockSize = NextPowerOf2(blockSize);
        _segSize = 2 * _blockSize;
        _segCount = static_cast<AmSize>(std::ceil(static_cast<AmReal32>(irLen) / static_cast<AmReal32>(_blockSize)));
        _fftComplexSize = FFT::GetOutputSize(_segSize);

        // FFT
        _fft.Initialize(_segSize);
        _fftBuffer.Resize(_segSize);

        // Prepare segments
        for (AmSize i = 0; i < _segCount; ++i)
            _segments.push_back(ampoolnew(eMemoryPoolKind_Filtering, SplitComplex, _fftComplexSize));

        // Prepare IR
        for (AmSize i = 0; i < _segCount; ++i)
        {
            auto* segment = ampoolnew(eMemoryPoolKind_Filtering, SplitComplex, _fftComplexSize);
            const AmSize remaining = irLen - (i * _blockSize);
            const AmSize sizeCopy = (remaining >= _blockSize) ? _blockSize : remaining;
            CopyAndPad(_fftBuffer, &ir[i * _blockSize], sizeCopy);
            _fft.Forward(_fftBuffer.GetBuffer(), *segment);
            _segmentsIR.push_back(segment);
        }

        // Prepare convolution buffers
        _preMultiplied.Resize(_fftComplexSize, true);
        _conv.Resize(_fftComplexSize, true);
        _overlap.Resize(_blockSize);

        // Prepare input buffer
        _inputBuffer.Resize(_blockSize);
        _inputBufferFill = 0;

        // Reset current position
        _current = 0;

        return true;
    }

    void Convolver::Process(const AmAudioSample* input, AmAudioSample* output, AmSize len)
    {
        if (_segCount == 0)
        {
            std::memset(output, 0, len * sizeof(AmAudioSample));
            return;
        }

        AmSize processed = 0;
        while (processed < len)
        {
            const bool inputBufferWasEmpty = (_inputBufferFill == 0);
            const AmSize processing = std::min(len - processed, _blockSize - _inputBufferFill);
            const AmSize inputBufferPos = _inputBufferFill;
            std::memcpy(_inputBuffer.GetBuffer() + inputBufferPos, input + processed, processing * sizeof(AmAudioSample));

            // Forward FFT
            CopyAndPad(_fftBuffer, _inputBuffer.GetBuffer(), processing);
            _fft.Forward(_fftBuffer.GetBuffer(), *_segments[_current]);

            // Complex multiplication
            if (inputBufferWasEmpty)
            {
                _preMultiplied.Clear();
                for (AmSize i = 1; i < _segCount; ++i)
                {
                    const AmSize indexIr = i;
                    const AmSize indexAudio = (_current + i) % _segCount;
                    ComplexMultiplyAccumulate(_preMultiplied, *_segmentsIR[indexIr], *_segments[indexAudio]);
                }
            }
            _conv.CopyFrom(_preMultiplied);
            ComplexMultiplyAccumulate(_conv, *_segmentsIR[0], *_segments[_current]);

            // Backward FFT
            _fft.Backward(_fftBuffer.GetBuffer(), _conv);

            // Add overlap
            Sum(output + processed, _fftBuffer.GetBuffer() + inputBufferPos, _overlap.GetBuffer() + inputBufferPos, processing);

            // Input buffer full => Next block
            _inputBufferFill += processing;
            if (_inputBufferFill == _blockSize)
            {
                // Input buffer is empty again now
                _inputBuffer.Clear();
                _inputBufferFill = 0;

                // Save the overlap
                std::memcpy(_overlap.GetBuffer(), _fftBuffer.GetBuffer() + _blockSize, _blockSize * sizeof(AmAudioSample));

                // Update current segments
                _current = (_current > 0) ? (_current - 1) : (_segCount - 1);
            }

            processed += processing;
        }
    }
} // namespace SparkyStudios::Audio::Amplitude