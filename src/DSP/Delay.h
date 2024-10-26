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

#ifndef _AM_DSP_DELAY_H
#define _AM_DSP_DELAY_H

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief A delay line.
     *
     * This delay line is a simple implementation of a delay line. It can be used to
     * simulate the delay of a reverb or a delay effect.
     */
    class Delay
    {
    public:
        /**
         * @brief Creates a new delay line.
         *
         * @param maxDelay The maximum delay in milliseconds.
         * @param framesCount The number of frames in each input/output buffer.
         */
        explicit Delay(AmSize maxDelay, AmSize framesCount);

        /**
         * @brief Destroys the delay line.
         */
        ~Delay();

        /**
         * @brief Sets the maximum delay in milliseconds.
         *
         * @param maxDelay The maximum delay in milliseconds.
         */
        void SetMaxDelay(AmSize maxDelay);

        /**
         * @brief Gets the maximum delay in milliseconds.
         *
         * @return The maximum delay in milliseconds.
         */
        [[nodiscard]] AmSize GetMaxDelay() const;

        /**
         * @brief Gets the delay in samples.
         *
         * @return The delay in samples.
         */
        [[nodiscard]] AmSize GetDelayInSamples() const;

        /**
         * @brief Clears the delay line buffer.
         */
        void Clear();

        /**
         * @brief Copies an audio buffer channel to the delay line buffer.
         *
         * @param channel The audio buffer channel to copy.
         */
        void Insert(const AudioBufferChannel& channel);

        /**
         * @brief Fills an audio buffer channel with data delayed by a given
         * amount less or equal to the delay line's maximum length.
         *
         * @param channel The audio buffer channel to fill.
         * @param delay The delay in number of samples. The delay must be less
         * or equal to the delay line's maximum length.
         */
        void Process(AudioBufferChannel& channel, AmSize delay);

    private:
        AmSize _maxDelay;
        AmSize _framesCount;
        AmSize _writePos;
        AmUniquePtr<eMemoryPoolKind_Filtering, AudioBuffer> _buffer;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_DSP_DELAY_H
