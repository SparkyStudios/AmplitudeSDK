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

#ifndef _AM_DSP_RESAMPLER_H
#define _AM_DSP_RESAMPLER_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Sound.h>

namespace SparkyStudios::Audio::Amplitude
{
    class Resampler;

    /**
     * @brief A Resampler instance.
     *
     * An object of this class will be created each time a `Resampler` is requested.
     *
     * @ingroup dsp
     */
    class AM_API_PUBLIC ResamplerInstance
    {
    public:
        /**
         * @brief Constructs a new `ResamplerInstance` object.
         *
         * This will initialize the resampler instance state to default values.
         */
        ResamplerInstance() = default;

        /**
         * @brief Default destructor.
         */
        virtual ~ResamplerInstance() = default;

        /**
         * @brief Initializes a new instance of the resampler.
         *
         * @param[in] channelCount The number of channels in the audio data.
         * @param[in] sampleRateIn The input sample rate.
         * @param[in] sampleRateOut The output sample rate.
         */
        virtual void Initialize(AmUInt16 channelCount, AmUInt32 sampleRateIn, AmUInt32 sampleRateOut) = 0;

        /**
         * @brief Processes the audio data.
         *
         * @param[in] input The input audio data.
         * @param[in,out] inputFrames The number of frames in the input buffer.
         * @param[out] output The output audio data.
         * @param[in,out] outputFrames The number of frames in the output buffer.
         *
         * @return `true` if the resampling was successful, `false` otherwise.
         */
        virtual bool Process(const AudioBuffer& input, AmUInt64& inputFrames, AudioBuffer& output, AmUInt64& outputFrames) = 0;

        /**
         * @brief Changes the input and output sample rate.
         *
         * @param[in] sampleRateIn The new input sample rate.
         * @param[in] sampleRateOut The new output sample rate.
         */
        virtual void SetSampleRate(AmUInt32 sampleRateIn, AmUInt32 sampleRateOut) = 0;

        /**
         * @brief Gets the current input sample rate.
         *
         * @return The current input sample rate.
         */
        [[nodiscard]] virtual AmUInt32 GetSampleRateIn() const = 0;

        /**
         * @brief Gets the current output sample rate.
         *
         * @return The current output sample rate.
         */
        [[nodiscard]] virtual AmUInt32 GetSampleRateOut() const = 0;

        /**
         * @brief Gets the current channels count.
         *
         * @return The current channels count.
         */
        [[nodiscard]] virtual AmUInt16 GetChannelCount() const = 0;

        /**
         * @brief Returns the required number of frames to have as input for the given amount of output frames.
         *
         * @param[in] outputFrameCount The number of output frames.
         *
         * @return The input frame count needed to produce the given output frame count.
         */
        [[nodiscard]] virtual AmUInt64 GetRequiredInputFrames(AmUInt64 outputFrameCount) const = 0;

        /**
         * @brief Returns the expected number of frames to have as output for the given amount of input frames.
         *
         * @param[in] inputFrameCount The number of input frames.
         *
         * @return The expected number of output frames for the given input frame count.
         */
        [[nodiscard]] virtual AmUInt64 GetExpectedOutputFrames(AmUInt64 inputFrameCount) const = 0;

        /**
         * @brief Returns the current input latency in frames.
         *
         * @return The resampler's current input latency in frames.
         */
        [[nodiscard]] virtual AmUInt64 GetInputLatency() const = 0;

        /**
         * @brief Returns the current output latency in frames.
         *
         * @return The resampler's current output latency in frames.
         */
        [[nodiscard]] virtual AmUInt64 GetOutputLatency() const = 0;

        /**
         * @brief Resets the internal resampler state.
         */
        virtual void Reset() = 0;

        /**
         * @brief Cleans up the internal resampler state and allocated data.
         *
         * @note This method is called when the resampler is about to be destroyed.
         */
        virtual void Clear() = 0;
    };

    /**
     * @brief Base class to manage resamplers.
     *
     * A resampler is used to change the sample rate of an audio buffer. The `Resampler` class implements
     * factory methods to create instances of `ResamplerInstance` objects, which are where the the resampling is done.
     *
     * The `Resampler` class follows the [plugins architecture](/plugins/anatomy.md), and thus, you are able to create your own resamplers
     * and register them to the `Engine` by inheriting from this class, and by implementing the necessary dependencies.
     *
     * @ingroup dsp
     */
    class AM_API_PUBLIC Resampler
    {
    public:
        /**
         * @brief Create a new Resampler instance.
         *
         * @param[in] name The resampler name. e.g. "MiniAudioLinear".
         */
        explicit Resampler(AmString name);

        /**
         * @brief Default Resampler constructor.
         *
         * This will not automatically register the resampler. It's meant for internal resamplers only.
         */
        Resampler();

        /**
         * @brief Default destructor.
         */
        virtual ~Resampler();

        /**
         * @brief Creates a new instance of the resampler.
         *
         * @return A new instance of the resampler.
         */
        virtual ResamplerInstance* CreateInstance() = 0;

        /**
         * @brief Destroys an instance of the resampler.
         *
         * @warning The instance should have been created with @ref CreateInstance `CreateInstance()`
         * before being destroyed with this method.
         *
         * @param[in] instance The resampler instance to be destroyed.
         */
        virtual void DestroyInstance(ResamplerInstance* instance) = 0;

        /**
         * @brief Gets the name of this resampler.
         *
         * @return The name of this resampler.
         */
        [[nodiscard]] const AmString& GetName() const;

        /**
         * @brief Registers a new resampler.
         *
         * @param[in] resampler The resampler to add in the registry.
         */
        static void Register(Resampler* resampler);

        /**
         * @brief Unregisters a resampler.
         *
         * @param[in] resampler The resampler to remove from the registry.
         */
        static void Unregister(const Resampler* resampler);

        /**
         * @brief Creates a new instance of the resampler with the given name and returns its pointer.
         *
         * @note The returned pointer should be deleted using @ref Destruct `Destruct()`.
         *
         * @param[in] name The name of the resampler.
         *
         * @return The resampler with the given name, or `nullptr` if none.
         */
        static ResamplerInstance* Construct(const AmString& name);

        /**
         * @brief Destroys the given resampler instance.
         *
         * @param[in] name The name of the resampler.
         * @param[in] instance The resampler instance to destroy.
         */
        static void Destruct(const AmString& name, ResamplerInstance* instance);

        /**
         * @brief Locks the resamplers registry.
         *
         * @warning This function is mainly used for internal purposes. It's
         * called before the `Engine` initialization, to discard the registration
         * of new resamplers after the engine is fully loaded.
         */
        static void LockRegistry();

        /**
         * @brief Unlocks the resamplers registry.
         *
         * @warning This function is mainly used for internal purposes. It's
         * called after the `Engine` deinitialization, to allow the registration
         * of new resamplers after the engine is fully unloaded.
         */
        static void UnlockRegistry();

    protected:
        /**
         * @brief The name of this resampler.
         */
        AmString m_name;

    private:
        /**
         * @brief Look up a resampler by name.
         *
         * @return The resampler with the given name, or `nullptr` if none.
         *
         * @internal
         */
        static Resampler* Find(const AmString& name);
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_DSP_RESAMPLER_H
