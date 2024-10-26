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

#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    static void InterleaveStereo(
        AmSize length, const AmReal32* channel0, const AmReal32* channel1, AmUInt64 inOffset, AmReal32* out, AmUInt64 outOffset)
    {
        AmSize remaining = length;

#if defined(AM_SIMD_INTRINSICS)
        const AmSize end = GetNumSimdChunks(length);
        constexpr AmSize blockSize = GetSimdBlockSize();
        remaining = remaining - end;

        for (AmSize i = 0; i < end; i += blockSize)
        {
            auto c0 = xsimd::load_aligned<simd_arch>(&channel0[i + inOffset]);
            auto c1 = xsimd::load_aligned<simd_arch>(&channel1[i + inOffset]);

            const AmSize k = 2 * (i + outOffset);
            xsimd::store_aligned(&out[k], xsimd::zip_lo(c0, c1));
            xsimd::store_aligned(&out[k + blockSize], xsimd::zip_hi(c0, c1));
        }
#endif // AM_SIMD_INTRINSICS

        for (AmSize i = length - remaining; i < length; ++i)
        {
            const AmSize k = 2 * (i + outOffset);
            out[k] = channel0[i + inOffset];
            out[k + 1] = channel1[i + inOffset];
        }
    }

    void Interleave(const AudioBuffer* in, AmUInt64 inOffset, AmReal32* out, AmUInt64 outOffset, AmInt32 numSamples, AmInt32 numChannels)
    {
        if (numChannels == 1)
        {
            std::memcpy(out + outOffset, in->GetChannel(0).begin() + inOffset, numSamples * sizeof(AmReal32));
            return;
        }
        else if (numChannels == 2)
        {
            InterleaveStereo(numSamples, in->GetChannel(0).begin(), in->GetChannel(1).begin(), inOffset, out, outOffset);
            return;
        }

        for (AmInt32 j = 0; j < numChannels; ++j)
        {
            const auto& channel = in->GetChannel(j);
            for (AmInt32 i = 0; i < numSamples; ++i)
                out[(i + outOffset) * numChannels + j] = channel[i + inOffset];
        }
    }

    void Sum(AmAudioSample* AM_RESTRICT result, const AmAudioSample* AM_RESTRICT a, const AmAudioSample* AM_RESTRICT b, const AmSize len)
    {
#if defined(AM_SIMD_INTRINSICS)
        const AmSize end = GetNumSimdChunks(len);
        constexpr AmSize blockSize = GetSimdBlockSize();

        for (AmSize i = 0; i < end; i += blockSize)
        {
            const auto ba = xsimd::load_aligned<simd_arch>(&a[i]);
            const auto bb = xsimd::load_aligned<simd_arch>(&b[i]);

            auto res = xsimd::add(ba, bb);
            res.store_aligned(&result[i]);
        }
#else
        const AmSize end = GetNumChunks(len, 4);

        for (AmSize i = 0; i < end; i += 4)
        {
            result[i + 0] = a[i + 0] + b[i + 0];
            result[i + 1] = a[i + 1] + b[i + 1];
            result[i + 2] = a[i + 2] + b[i + 2];
            result[i + 3] = a[i + 3] + b[i + 3];
        }
#endif

        for (AmSize i = end; i < len; ++i)
            result[i] = a[i] + b[i];
    }

    void ComplexMultiplyAccumulate(
        AmAudioSample* AM_RESTRICT re,
        AmAudioSample* AM_RESTRICT im,
        const AmAudioSample* AM_RESTRICT reA,
        const AmAudioSample* AM_RESTRICT imA,
        const AmAudioSample* AM_RESTRICT reB,
        const AmAudioSample* AM_RESTRICT imB,
        const AmSize len)
    {
#if defined(AM_SIMD_INTRINSICS)
        const AmSize end = GetNumSimdChunks(len);
        constexpr AmSize blockSize = GetSimdBlockSize();

        for (AmSize i = 0; i < end; i += blockSize)
        {
            const auto ra = xsimd::load_aligned<simd_arch>(&reA[i]);
            const auto rb = xsimd::load_aligned<simd_arch>(&reB[i]);
            const auto ia = xsimd::load_aligned<simd_arch>(&imA[i]);
            const auto ib = xsimd::load_aligned<simd_arch>(&imB[i]);

            auto real = xsimd::load_aligned<simd_arch>(&re[i]);
            auto imag = xsimd::load_aligned<simd_arch>(&im[i]);

            real = xsimd::fma(ra, rb, real);
            real = xsimd::sub(real, xsimd::mul(ia, ib));
            real.store_aligned(&re[i]);

            imag = xsimd::fma(ra, ib, imag);
            imag = xsimd::fma(ia, rb, imag);
            imag.store_aligned(&im[i]);
        }
#else
        const AmSize end = GetNumChunks(len, 4);

        for (AmSize i = 0; i < end; i += 4)
        {
            re[i + 0] += reA[i + 0] * reB[i + 0] - imA[i + 0] * imB[i + 0];
            re[i + 1] += reA[i + 1] * reB[i + 1] - imA[i + 1] * imB[i + 1];
            re[i + 2] += reA[i + 2] * reB[i + 2] - imA[i + 2] * imB[i + 2];
            re[i + 3] += reA[i + 3] * reB[i + 3] - imA[i + 3] * imB[i + 3];

            im[i + 0] += reA[i + 0] * imB[i + 0] + imA[i + 0] * reB[i + 0];
            im[i + 1] += reA[i + 1] * imB[i + 1] + imA[i + 1] * reB[i + 1];
            im[i + 2] += reA[i + 2] * imB[i + 2] + imA[i + 2] * reB[i + 2];
            im[i + 3] += reA[i + 3] * imB[i + 3] + imA[i + 3] * reB[i + 3];
        }
#endif

        for (AmSize i = end; i < len; ++i)
        {
            re[i] += reA[i] * reB[i] - imA[i] * imB[i];
            im[i] += reA[i] * imB[i] + imA[i] * reB[i];
        }
    }

    AmReal32 ComputeMonopoleFilterCoefficient(AmReal32 cutoffFrequency, AmUInt32 sampleRate)
    {
        AmReal32 coefficient = 0.0f;

        if (cutoffFrequency > 20.f)
        {
            const AmReal32 inverseTimeConstant = AM_PI32 * 2.0f * cutoffFrequency;
            const AmReal32 fSampleRate = static_cast<AmReal32>(sampleRate);
            coefficient = fSampleRate / (inverseTimeConstant + fSampleRate);
        }

        return coefficient;
    }
} // namespace SparkyStudios::Audio::Amplitude
