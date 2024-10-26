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

#ifndef _AM_IMPLEMENTATION_AMBISONICS_AMBISONIC_SOURCE_H
#define _AM_IMPLEMENTATION_AMBISONICS_AMBISONIC_SOURCE_H

#include <Ambisonics/AmbisonicEntity.h>
#include <Ambisonics/BFormat.h>

namespace SparkyStudios::Audio::Amplitude
{
    class AmbisonicSource : public AmbisonicEntity
    {
    public:
        AmbisonicSource();

        ~AmbisonicSource() override;

        /**
         * @copydoc AmbisonicEntity::Configure
         */
        bool Configure(AmUInt32 order, bool is3D) override;

        /**
         * @copydoc AmbisonicEntity::Refresh
         */
        void Refresh() override;

        /**
         * @brief Sets the ambisonic entity's position with the possibility to interpolate the position over a specified duration.
         *
         * @param position The ambisonic entity's position.
         * @param duration Interpolation duration. The value is in the range [0.0, 1.0] where 1.0f interpolates over a full frame.
         */
        void SetPosition(const SphericalPosition& position, AmTime duration = 0.0f);

        /**
         * @brief Encodes the input audio samples into the BFormat object.
         *
         * @param input The input audio samples. Should be a mono channel buffer.
         * @param samples The number of audio samples to process.
         * @param output The BFormat object to store the encoded audio samples.
         */
        void Process(const AudioBufferChannel& input, AmUInt32 samples, BFormat* output);

        /**
         * @brief Encodes the input audio samples into the BFormat object, accumulating the encoded samples.
         *
         * @param input The input audio samples. Should be a mono channel buffer.
         * @param samples The number of audio samples to process.
         * @param output The BFormat object to store the encoded audio samples.
         * @param offset An optional offset in the output buffer.
         * @param gain An optional gain factor applied to the encoded audio samples.
         */
        void ProcessAccumulate(
            const AudioBufferChannel& input, AmUInt32 samples, BFormat* output, AmUInt32 offset = 0, AmReal32 gain = 1.0f);

    private:
        AmAlignedReal32Buffer _oldCoefficients; // The previous HOA coefficients
        AmTime _interpolationDuration; // The interpolation duration
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_AMBISONICS_AMBISONIC_SOURCE_H
