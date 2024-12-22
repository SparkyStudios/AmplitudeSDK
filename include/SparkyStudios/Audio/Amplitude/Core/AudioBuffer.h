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

#ifndef _AM_CORE_AUDIO_BUFFER_H
#define _AM_CORE_AUDIO_BUFFER_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Represents a view to a single channel in an `AudioBuffer`.
     *
     * @see AudioBuffer
     *
     * @ingroup core
     */
    class AM_API_PUBLIC AudioBufferChannel
    {
    public:
        /**
         * @brief Gets the size of the buffer.
         *
         * @return The size of the buffer.
         */
        [[nodiscard]] AmSize size() const;

        /**
         * @brief Returns a float pointer to the beginning of the channel data.
         *
         * @return A float pointer to the beginning of the channel data.
         */
        AmReal32* begin();

        /**
         * @brief Returns a const float pointer to the beginning of the channel data.
         *
         * @return A const float pointer to the beginning of the channel data.
         */
        [[nodiscard]] const AmReal32* begin() const;

        /**
         * @brief Returns a float pointer to the end of the channel data.
         *
         * @return A float pointer to the end of the channel data.
         */
        AmReal32* end();

        /**
         * @brief Returns a const float pointer to the end of the channel data.
         *
         * @return A const float pointer to the end of the channel data.
         */
        [[nodiscard]] const AmReal32* end() const;

        /**
         * @brief Clears the channel data with zero.
         */
        void clear();

        /**
         * @brief Checks if the channel is enabled.
         *
         * @return `true` if the channel is enabled, `false` otherwise.
         */
        [[nodiscard]] bool enabled() const;

        /**
         * @brief Gets the audio sample at the specified index.
         *
         * @param[in] index The audio sample index.
         *
         * @return The audio sample at the specified index.
         */
        AmReal32& operator[](AmSize index);

        /**
         * @brief Gets the audio sample at the specified index.
         *
         * @param[in] index The audio sample index.
         *
         * @return The audio sample at the specified index.
         */
        [[nodiscard]] const AmReal32& operator[](AmSize index) const;

        /**
         * @brief Sets the entire channel data from the provided vector.
         *
         * @param[in] data The vector containing the new channel data.
         *
         * @return A reference to the modified channel.
         */
        AudioBufferChannel& operator=(const std::vector<AmReal32>& data);

        /**
         * @brief Sets the entire channel data from the provided `AudioBufferChannel`.
         *
         * @param[in] channel The `AudioBufferChannel` to copy the data from.
         *
         * @return A reference to the modified channel.
         */
        AudioBufferChannel& operator=(const AudioBufferChannel& channel);

        /**
         * @brief Performs point-wise addition of this channel with the provided `AudioBufferChannel`.
         *
         * @param[in] channel The `AudioBufferChannel` to add.
         *
         * @return A reference to the modified channel.
         */
        AudioBufferChannel& operator+=(const AudioBufferChannel& channel);

        /**
         * @brief Performs point-wise subtraction of this channel with the provided `AudioBufferChannel`.
         *
         * @param[in] channel The `AudioBufferChannel` to subtract.
         *
         * @return A reference to the modified channel.
         */
        AudioBufferChannel& operator-=(const AudioBufferChannel& channel);

        /**
         * @brief Point-wise multiplies this channel with the provided `AudioBufferChannel`.
         *
         * @param[in] channel The `AudioBufferChannel` to multiply with.
         *
         * @return A reference to the modified channel.
         */
        AudioBufferChannel& operator*=(const AudioBufferChannel& channel);

        /**
         * @brief Point-wise multiplies this channel with the provided scalar.
         *
         * @param[in] scalar The scalar to multiply with.
         *
         * @return A reference to the modified channel.
         */
        AudioBufferChannel& operator*=(AmReal32 scalar);

    private:
        friend class AudioBuffer;

        /**
         * @brief Creates a new audio buffer channel from the provided memory range.
         *
         * @param[in] begin The beginning of the memory range storing the channel data.
         * @param[in] numFrames The number of audio samples in the memory range.
         *
         * @internal
         */
        AudioBufferChannel(AmReal32* begin, AmSize numFrames);

        /**
         * @brief The beginning of the memory range storing the channel data.
         *
         * @internal
         */
        AmReal32* _begin;

        /**
         * @brief The number of audio samples in the memory range.
         *
         * @internal
         */
        AmSize _frameCount;

        /**
         * @brief Indicates whether the channel is enabled.
         *
         * @internal
         */
        bool _isEnabled;
    };

    /**
     * @brief Represents an audio buffer containing multiple channels.
     *
     * An `AudioBuffer` is a de-interleaved memory storage used to store and manipulate audio data,
     * such as audio samples or Ambisonics sound fields, in a flexible and efficient manner. Accessing
     * a channel data will return an `AudioBufferChannel` object, which is a view to the memory range storing that channel.
     *
     * @see AudioBufferChannel
     *
     * @ingroup core
     */
    class AM_API_PUBLIC AudioBuffer
    {
    public:
        /**
         * @brief Copies the given number of frames from the source buffer to the destination buffer.
         *
         * @param[in] source The source buffer to copy.
         * @param[in] sourceOffset The offset in the source buffer.
         * @param[out] destination The destination buffer to copy to.
         * @param[in] destinationOffset The offset in the destination buffer.
         * @param[in] numFrames The number of frames to copy.
         */
        static void Copy(
            const AudioBuffer& source, AmSize sourceOffset, AudioBuffer& destination, AmSize destinationOffset, AmSize numFrames);

        /**
         * @brief Creates an empty audio buffer.
         */
        AudioBuffer();

        /**
         * @brief Creates an audio buffer with the specified number of frames and channels.
         *
         * @param[in] numFrames The number of frames in the buffer.
         * @param[in] numChannels The number of channels in the buffer.
         */
        AudioBuffer(AmSize numFrames, AmSize numChannels);

        /**
         * @brief Explicitly deletes the audio buffer copy to avoid unintended usage.
         *
         * @param[in] buffer The other audio buffer to copy.
         *
         * @note Use the assignment operator to copy the audio buffer.
         */
        AudioBuffer(const AudioBuffer& buffer) = delete;

        /**
         * @brief Moves the given audio buffer data in this one.
         *
         * @param[in] buffer The other audio buffer to move.
         */
        AudioBuffer(AudioBuffer&& buffer) noexcept;

        /**
         * @brief Destroys the audio buffer data and release allocated memory.
         */
        ~AudioBuffer();

        /**
         * @brief Check if the audio buffer is empty.
         *
         * @return `true` if the audio buffer is empty, `false` otherwise.
         */
        [[nodiscard]] bool IsEmpty() const;

        /**
         * @brief Gets the number of frames in the buffer.
         *
         * @return The number of frames in the buffer.
         */
        [[nodiscard]] AmSize GetFrameCount() const;

        /**
         * @brief Gets the number of channels in the buffer.
         *
         * @return The number of channels in the buffer.
         */
        [[nodiscard]] AmSize GetChannelCount() const;

        /**
         * @brief Sets the entire audio buffer data to zero.
         */
        void Clear();

        /**
         * @brief Gets the wrapped audio buffer data.
         *
         * @return The wrapped audio buffer data.
         */
        [[nodiscard]] const AmAlignedReal32Buffer& GetData() const;

        /**
         * @brief Gets the `AudioBufferChannel` at the specified index.
         *
         * @param[in] index The channel index.
         *
         * @return The `AudioBufferChannel` at the specified index.
         */
        AudioBufferChannel& GetChannel(AmSize index);

        /**
         * @brief Gets the `AudioBufferChannel` at the specified index.
         *
         * @param[in] index The channel index.
         *
         * @return The `AudioBufferChannel` at the specified index.
         */
        [[nodiscard]] const AudioBufferChannel& GetChannel(AmSize index) const;

        /**
         * @brief Clones the audio buffer and returns a new instance.
         *
         * @return A new instance with a cloned copy of the audio buffer data.
         */
        AudioBuffer Clone() const;

        /**
         * @brief Gets the `AudioBufferChannel` at the specified index.
         *
         * @param[in] index The channel index.
         *
         * @return The `AudioBufferChannel` at the specified index.
         */
        AudioBufferChannel& operator[](AmSize index);

        /**
         * @brief Gets the `AudioBufferChannel` at the specified index.
         *
         * @param[in] index The channel index.
         *
         * @return The `AudioBufferChannel` at the specified index.
         */
        [[nodiscard]] const AudioBufferChannel& operator[](AmSize index) const;

        /**
         * @brief Copies the audio buffer data from the provided `AudioBuffer`.
         *
         * @param[in] buffer The other audio buffer to copy.
         *
         * @return This instance with the copied audio buffer data.
         */
        AudioBuffer& operator=(const AudioBuffer& buffer);

        /**
         * @brief Accumulates the audio buffer data from the provided `AudioBuffer`.
         *
         * @param[in] buffer The buffer to add in this one.
         *
         * @return This instance with the added audio buffer data.
         */
        AudioBuffer& operator+=(const AudioBuffer& buffer);

        /**
         * @brief Subtracts the audio buffer data from the provided `AudioBuffer`.
         *
         * @param[in] buffer The buffer to subtract from this one.
         *
         * @return This instance with the subtracted audio buffer data.
         */
        AudioBuffer& operator-=(const AudioBuffer& buffer);

        /**
         * @brief Point-wise multiplies the audio buffer data with the provided `AudioBuffer`.
         *
         * @param[in] buffer The buffer to multiply with this one.
         *
         * @return This instance with the multiplied audio buffer data.
         */
        AudioBuffer& operator*=(const AudioBuffer& buffer);

        /**
         * @brief Point-wise multiplies this channel with the provided scalar.
         *
         * @param[in] scalar The scalar to multiply with.
         *
         * @return A reference to the modified channel.
         */
        AudioBuffer& operator*=(AmReal32 scalar);

    private:
        /**
         * @brief Initializes the audio buffer with the specified number of channels.
         *
         * @param[in] channelCount The number of channels in the buffer.
         *
         * @internal
         */
        void Initialize(AmSize channelCount);

        /**
         * @brief The number of audio samples in each channel.
         *
         * @internal
         */
        AmSize _frameCount;

        /**
         * @brief The vector of available channels.
         *
         * @internal
         */
        std::vector<AudioBufferChannel> _channels;

        /**
         * @brief The wrapped audio buffer data.
         *
         * @internal
         */
        AmAlignedReal32Buffer _data;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_CORE_AUDIO_BUFFER_H
