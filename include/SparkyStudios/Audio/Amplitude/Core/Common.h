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

#ifndef _AM_CORE_COMMON_H
#define _AM_CORE_COMMON_H

// Standard Library
// --------------------------------------------------------------

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <map>
#include <memory>
#include <numeric>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

// Amplitude
// --------------------------------------------------------------

#include <SparkyStudios/Audio/Amplitude/Core/Common/Config.h>
#include <SparkyStudios/Audio/Amplitude/Core/Common/Types.h>

#include <SparkyStudios/Audio/Amplitude/Core/Common/Constants.h>

#include <SparkyStudios/Audio/Amplitude/Math/HandmadeMath.h>

// Common defines
// --------------------------------------------------------------

/**
 * @brief Define an invalid object handle.
 *
 * @ingroup engine
 */
#define AM_INVALID_HANDLE nullptr

/**
 * @brief Checks if a handle is valid
 *
 * @param handle The handle to check
 *
 * @return @c true if the handle is valid, @c false otherwise.
 *
 * @ingroup engine
 */
#define AM_IS_VALID_HANDLE(handle) ((handle) != AM_INVALID_HANDLE)

/**
 * @brief Clamps a value between a and b.
 *
 * @param v The value to clamp
 * @param a The minimum value
 * @param b The maximum value
 *
 * @return The clamped value
 *
 * @ingroup math
 */
#define AM_CLAMP(v, a, b) (((v) < (a)) ? (a) : ((v) > (b)) ? (b) : (v))

/**
 * @brief Checks if a value is between a and b.
 *
 * @param v The value to check
 * @param a The minimum value
 * @param b The maximum value
 *
 * @return @c true if the value is between a and b, @c false otherwise.
 *
 * @ingroup math
 */
#define AM_BETWEEN(v, a, b) ((v) >= AM_MIN(a, b) && (v) <= AM_MAX(a, b))

/**
 * @brief Declare a callback function type
 *
 * @param _type_ Return type of the function
 * @param _name_ Name of the function
 *
 * @note This must be followed by the parentheses containing the function arguments declaration
 *
 * @ingroup core
 */
#define AM_CALLBACK(_type_, _name_) typedef _type_(AM_CALL_POLICY* _name_)

/**
 * @brief The minimum value for an audio sample.
 *
 * @ingroup engine
 */
#define AM_AUDIO_SAMPLE_MIN (-1.0f)

/**
 * @brief The maximum value for an audio sample.
 *
 * @ingroup engine
 */
#define AM_AUDIO_SAMPLE_MAX (+1.0f)

/**
 * @brief Helps to avoid compiler warnings about unused values.
 *
 * @param x The statement where the return value is not used.
 *
 * @ingroup core
 */
#define AM_UNUSED(x) ((void)(x))

/**
 * @brief Turn X into a string literal.
 *
 * @param x The value to transform into a string literal
 */
#define AM_STRING_EXPAND(X) #X
#define AM_TO_STRING(X) AM_STRING_EXPAND(X)

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Class that handles aligned allocations to support vectorized operations.
     *
     * @ingroup core
     */
    class AM_API_PUBLIC AmAlignedReal32Buffer
    {
    public:
        /**
         * @brief Constructs an empty buffer.
         */
        AmAlignedReal32Buffer();

        /**
         * @brief Destructs the buffer and deallocates the memory.
         */
        ~AmAlignedReal32Buffer();

        /**
         * @brief Allocates and align buffer.
         *
         * @param[in] size The buffer size.
         * @param[in] clear Whether to clear the buffer.
         *
         * @returns An `AM_ERROR` value indicating if the allocation was successful or not.
         */
        AmResult Init(AmUInt32 size, bool clear = true);

        /**
         * @brief Clears all data.
         */
        void Clear() const;

        /**
         * @brief Releases the allocated buffer.
         */
        void Release();

        /**
         * @brief Gets the size of the buffer.
         *
         * @return The number of float values stored in the buffer.
         */
        [[nodiscard]] AM_INLINE AmUInt32 GetSize() const
        {
            return m_floats;
        }

        /**
         * @brief Gets the current aligned pointer.
         *
         * @return The pointer the float buffer
         */
        [[nodiscard]] AM_INLINE AmReal32* GetBuffer() const
        {
            return m_data;
        }

        /**
         * @brief Gets the raw allocated pointer.
         *
         * @return The pointer to the raw allocated memory.
         */
        [[nodiscard]] AM_INLINE AmUInt8Buffer GetPointer() const
        {
            return m_basePtr;
        }

        /**
         * @brief Copies data from another buffer.
         *
         * @param[in] other The other buffer to copy data from.
         */
        void CopyFrom(const AmAlignedReal32Buffer& other) const;

        /**
         * @brief Resizes the buffer to the specified size.
         *
         * @param[in] size The new size of the buffer.
         * @param[in] clear Whether to clear the buffer after resize. If `true`, the buffer will be cleared
         * even if the new size equals the old size.
         */
        void Resize(AmUInt32 size, bool clear = true);

        /**
         * @brief Swaps two buffers.
         *
         * @param[in,out] a The first buffer.
         * @param[in,out] b The second buffer.
         */
        static void Swap(AmAlignedReal32Buffer& a, AmAlignedReal32Buffer& b);

        /**
         * @brief Returns an iterator to the beginning of the buffer.
         *
         * @return An iterator to the beginning of the buffer.
         */
        [[nodiscard]] AM_INLINE const AmReal32* begin() const
        {
            return m_data;
        }

        /**
         * @brief Returns an iterator to the end of the buffer.
         *
         * @return An iterator to the end of the buffer.
         */
        [[nodiscard]] AM_INLINE const AmReal32* end() const
        {
            return m_data + m_floats;
        }

        /**
         * @brief Returns an iterator to the beginning of the buffer.
         *
         * @return An iterator to the beginning of the buffer.
         */
        [[nodiscard]] AM_INLINE AmReal32* begin()
        {
            return m_data;
        }

        /**
         * @brief Returns an iterator to the end of the buffer.
         *
         * @return An iterator to the end of the buffer.
         */
        [[nodiscard]] AM_INLINE AmReal32* end()
        {
            return m_data + m_floats;
        }

        /**
         * @brief Returns a reference to the float at the specified index.
         *
         * @param[in] index The index of the float to retrieve.
         *
         * @return The reference to the float at the specified index.
         */
        AmReal32& operator[](AmSize index)
        {
            AMPLITUDE_ASSERT(m_data != nullptr && index < m_floats);
            return m_data[index];
        }

        /**
         * @brief Returns a const reference to the float at the specified index.
         *
         * @param[in] index The index of the float to retrieve.
         *
         * @return The const reference to the float at the specified index.
         */
        const AmReal32& operator[](AmSize index) const
        {
            AMPLITUDE_ASSERT(m_data != nullptr && index < m_floats);
            return m_data[index];
        }

    private:
        AmReal32* m_data; // aligned pointer
        AmUInt8Buffer m_basePtr; // raw allocated pointer (for delete)
        AmUInt32 m_floats; // size of buffer (w/out padding)
    };

    /**
     * @brief Enumerates the list of possible errors encountered by the library.
     *
     * @ingroup core
     */
    enum eErrorCode : AmUInt8
    {
        eErrorCode_Success = 0, ///< No error
        eErrorCode_InvalidParameter = 1, ///< Some parameter is invalid
        eErrorCode_FileNotFound = 2, ///< File not found
        eErrorCode_FileLoadFailed = 3, ///< File found, but could not be loaded
        eErrorCode_DllNotFound = 4, ///< DLL not found, or wrong DLL
        eErrorCode_OutOfMemory = 5, ///< Out of memory
        eErrorCode_NotImplemented = 6, ///< Feature not implemented
        eErrorCode_Unknown = 7 ///< Unknown error
    };

    /**
     * @brief Enumerates the list of possible sample formats handled by Amplitude.
     *
     * @ingroup core
     */
    enum eAudioSampleFormat : AmUInt8
    {
        eAudioSampleFormat_Float32, ///< 32-bit floating-point sample
        eAudioSampleFormat_Int16, //< 16-bit signed integer sample
        eAudioSampleFormat_Unknown, ///< An unknown or unsupported format
    };

    /**
     * @brief Enumerates the list of available spatialization modes.
     *
     * @ingroup core
     */
    enum eSpatialization : AmUInt8
    {
        /**
         * @brief Disables spatialization.
         */
        eSpatialization_None,

        /**
         * @brief Enables 2D (left-right) spatialization based on sound position.
         *
         * @note This mode is available for every panning mode.
         */
        eSpatialization_Position,

        /**
         * @brief Enables 2D (left-right) spatialization based on sound position and orientation.
         *
         * @note The sound instance using this spatialization mode needs to be attached to an `Entity`.
         *
         * @note This mode is available for every panning mode.
         */
        eSpatialization_PositionOrientation,

        /**
         * @brief Enables 3D spatialization using Head Related Transfer Functions.
         *
         * @note This mode is only available for binaural panning modes.
         */
        eSpatialization_HRTF
    };

    /**
     * @brief Enumerates the list of available scopes for sound objects.
     *
     * @ingroup core
     */
    enum eScope : AmUInt8
    {
        /**
         * @brief The sound object is within the game world. Instances of collections played in this scope
         * will be treated as one object across all entities.
         */
        eScope_World,

        /**
         * @brief The sound object is within a specific entity. Instances of collections played in this scope
         * will be treated as separate objects, and no data will be shared across entities.
         *
         * @note Sound objects using this scope are required to be attached to an [`Entity`](./Entity.md).
         */
        eScope_Entity
    };

    /**
     * @brief Enumerates the list of available panning modes.
     *
     * @ingroup core
     */
    enum ePanningMode : AmUInt8
    {
        /**
         * @brief 2D stereo panning. This panning mode won't provide HRTF-related features.
         *
         * @note The Ambisonic decoder will use a virtual array of 2 loudspeakers
         * evenly arranged in front of the listener.
         */
        ePanningMode_Stereo = 0,

        /**
         * @brief 3D binaural panning using first-order HRTF.
         *
         * @note The Ambisonic decoder will use a virtual array of 8 loudspeakers
         * arranged in a cube configuration around the listener.
         */
        ePanningMode_BinauralLowQuality = 1,

        /**
         * @brief 3D binaural panning using second-order HRTF.
         *
         * @note The Ambisonic decoder will use a virtual array of 12 loudspeakers
         * arranged in a dodecahedral configuration (using faces of the dodecahedron).
         */
        ePanningMode_BinauralMediumQuality = 2,

        /**
         * @brief 3D binaural panning using third-order HRTF.
         *
         * @note The Ambisonic decoder will use a virtual array of 26 loudspeakers
         * arranged in a Lebedev grid. See: https://people.sc.fsu.edu/~jburkardt/m_src/sphere_lebedev_rule/sphere_lebedev_rule.html
         */
        ePanningMode_BinauralHighQuality = 3,
    };

    /**
     * @brief Defines how the HRIR sphere is sampled when doing Ambisonics binauralization.
     *
     * @ingroup core
     */
    enum eHRIRSphereSamplingMode : AmUInt8
    {
        /**
         * @brief Provides the most accurate binauralization, as the HRIR data are smoothly transitioned between sphere points.
         *
         * See more info about bilinear sampling [here](http://www02.smt.ufrj.br/~diniz/conf/confi117.pdf).
         */
        eHRIRSphereSamplingMode_Bilinear = 0,

        /**
         * @brief Provides a more efficient binauralization, as the HRIR data are interpolated using only the nearest neighbors.
         */
        eHRIRSphereSamplingMode_NearestNeighbor = 1,
    };

    /**
     * @brief Describe the format of an audio sample.
     *
     * This data structure is mainly filled by a `Codec` during the initialization time.
     *
     * @ingroup core
     */
    struct AM_API_PUBLIC SoundFormat
    {
    public:
        /**
         * @brief Sets all the properties of the sound format.
         *
         * @param[in] sampleRate The sample rate of the audio.
         * @param[in] numChannels The number of audio channels.
         * @param[in] bitsPerSample The number of bits per sample.
         * @param[in] framesCount The total number of audio frames.
         * @param[in] frameSize The size of each audio frame in bytes.
         * @param[in] sampleType The type of audio sample.
         */
        void SetAll(
            AmUInt32 sampleRate,
            AmUInt16 numChannels,
            AmUInt32 bitsPerSample,
            AmUInt64 framesCount,
            AmUInt32 frameSize,
            eAudioSampleFormat sampleType);

        /**
         * @brief Get the sample rate.
         *
         * @return The sample rate of the audio.
         */
        [[nodiscard]] AM_INLINE AmUInt32 GetSampleRate() const
        {
            return _sampleRate;
        }

        /**
         * @brief Get the number of channels.
         *
         * @return The number of audio channels.
         */
        [[nodiscard]] AM_INLINE AmUInt16 GetNumChannels() const
        {
            return _numChannels;
        }

        /**
         * @brief Get the bits per sample.
         *
         * @return The number of bits per sample.
         */
        [[nodiscard]] AM_INLINE AmUInt32 GetBitsPerSample() const
        {
            return _bitsPerSample;
        }

        /**
         * @brief Get the number of frames.
         *
         * @return The total number of audio frames.
         */
        [[nodiscard]] AM_INLINE AmUInt64 GetFramesCount() const
        {
            return _framesCount;
        }

        /**
         * @brief Get the frame size.
         *
         * @return The size of each audio frame in bytes.
         */
        [[nodiscard]] AM_INLINE AmUInt32 GetFrameSize() const
        {
            return _frameSize;
        }

        /**
         * @brief Get the sample type.
         *
         * @return The type of audio sample.
         */
        [[nodiscard]] AM_INLINE eAudioSampleFormat GetSampleType() const
        {
            return _sampleType;
        }

    private:
        AmUInt32 _sampleRate = 0;
        AmUInt16 _numChannels = 0;
        AmUInt32 _bitsPerSample = 0;
        AmUInt64 _framesCount = 0;
        AmUInt32 _frameSize = 0;
        eAudioSampleFormat _sampleType = eAudioSampleFormat_Float32;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_CORE_COMMON_H
