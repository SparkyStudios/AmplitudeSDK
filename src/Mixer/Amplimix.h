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

#ifndef _AM_IMPLEMENTATION_MIXER_AMPLIMIX_H
#define _AM_IMPLEMENTATION_MIXER_AMPLIMIX_H

#include <queue>

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/Core/Device.h>
#include <SparkyStudios/Audio/Amplitude/Core/Thread.h>
#include <SparkyStudios/Audio/Amplitude/DSP/AudioConverter.h>
#include <SparkyStudios/Audio/Amplitude/Mixer/Amplimix.h>
#include <SparkyStudios/Audio/Amplitude/Mixer/Pipeline.h>

#include <Mixer/SoundData.h>

#include <Utils/miniaudio/miniaudio_utils.h>
#include <Utils/Utils.h>

#include "engine_config_definition_generated.h"

#define _Atomic(X) std::atomic<X>

namespace SparkyStudios::Audio::Amplitude
{
    static constexpr AmUInt32 kAmplimixLayersBits = 12;
    static constexpr AmUInt32 kAmplimixLayersCount = (1 << kAmplimixLayersBits);
    static constexpr AmUInt32 kAmplimixLayersMask = (kAmplimixLayersCount - 1);

    class AmplimixImpl;

    /**
     * @brief The callback to execute when running a mixer command.
     */
    typedef std::function<bool()> MixerCommandCallback;

    enum PlayStateFlag : AmUInt8
    {
        ePSF_MIN = 0,
        ePSF_STOP = 1,
        ePSF_HALT = 2,
        ePSF_PLAY = 3,
        ePSF_LOOP = 4,
        ePSF_MAX,
    };

    class AmplimixLayerImpl final : public AmplimixLayer
    {
    public:
        AmUInt32 id = kAmInvalidObjectId; // playing id
        _Atomic(PlayStateFlag) flag; // state
        _Atomic(AmUInt64) cursor; // cursor
        _Atomic(AmReal32) gain; // gain
        _Atomic(AmReal32) pan; // pan
        _Atomic(AmReal32) pitch; // pitch
        SoundData* snd = nullptr; // sound data
        AmUInt64 start = 0, end = 0; // start and end frames

        _Atomic(AmReal32) obstruction; // obstruction factor
        _Atomic(AmReal32) occlusion; // occlusion factor

        _Atomic(AmReal32) userPlaySpeed; // user-defined sound playback speed
        _Atomic(AmReal32) playSpeed; // current sound playback speed
        _Atomic(AmReal32) targetPlaySpeed; // computed (real) sound playback speed
        _Atomic(AmReal32) sampleRateRatio; // sample rate ratio
        _Atomic(AmReal32) baseSampleRateRatio; // base sample rate ratio

        AudioConverter* dataConverter = nullptr; // miniaudio resampler & channel converter
        PipelineInstance* pipeline = nullptr; // pipeline for this layer

        AmMutexHandle mutex = nullptr; // mutex for thread-safe access
        std::unordered_map<AmThreadID, bool> mutexLocked; // true if mutex is locked

        ~AmplimixLayerImpl() override;

        /**
         * @brief Resets the layer.
         */
        void Reset();

        void ResetPipeline();

        [[nodiscard]] AmUInt32 GetId() const override;
        [[nodiscard]] AmUInt64 GetStartPosition() const override;
        [[nodiscard]] AmUInt64 GetEndPosition() const override;
        [[nodiscard]] AmUInt64 GetCurrentPosition() const override;
        [[nodiscard]] AmReal32 GetGain() const override;
        [[nodiscard]] AmReal32 GetStereoPan() const override;
        [[nodiscard]] AmReal32 GetPitch() const override;
        [[nodiscard]] AmReal32 GetObstruction() const override;
        [[nodiscard]] AmReal32 GetOcclusion() const override;
        [[nodiscard]] AmReal32 GetPlaySpeed() const override;
        [[nodiscard]] AmVec3 GetLocation() const override;
        [[nodiscard]] Entity GetEntity() const override;
        [[nodiscard]] Listener GetListener() const override;
        [[nodiscard]] Room GetRoom() const override;
        [[nodiscard]] Channel GetChannel() const override;
        [[nodiscard]] Bus GetBus() const override;
        [[nodiscard]] SoundFormat GetSoundFormat() const override;
        [[nodiscard]] eSpatialization GetSpatialization() const override;
        [[nodiscard]] bool IsLoopEnabled() const override;
        [[nodiscard]] bool IsStreamEnabled() const override;
        [[nodiscard]] const Sound* GetSound() const override;
        [[nodiscard]] const EffectInstance* GetEffect() const override;
        [[nodiscard]] const Attenuation* GetAttenuation() const override;
        [[nodiscard]] AmUInt32 GetSampleRate() const override;
    };

    struct MixerCommand
    {
        MixerCommandCallback callback; // command callback
    };

    /**
     * @brief Amplimix - The Amplitude Audio Mixer
     */
    class AmplimixImpl final : public Amplimix
    {
        friend struct AmplimixMutexLocker;

    public:
        explicit AmplimixImpl(AmReal32 masterGain);

        ~AmplimixImpl() override;

        /**
         * @brief Initializes the audio Mixer.
         *
         * @param config The audio engine configuration.
         * @return true on success, false on failure.
         */
        bool Init(const EngineConfigDefinition* config);

        /**
         * @brief Deinitializes the audio mixer.
         */
        void Deinit();

        /**
         * @copydoc Amplimix::UpdateDevice
         */
        void UpdateDevice(
            AmObjectID deviceID,
            AmString deviceName,
            AmUInt32 deviceOutputSampleRate,
            PlaybackOutputChannels deviceOutputChannels,
            PlaybackOutputFormat deviceOutputFormat) override;

        [[nodiscard]] AM_INLINE bool IsInitialized() const override
        {
            return _initialized;
        }

        void SetAfterMixCallback(AfterMixCallback callback) override;

        AmUInt64 Mix(AudioBuffer** outBuffer, AmUInt64 frameCount) override;

        AmUInt32 Play(
            SoundData* sound, PlayStateFlag flag, AmReal32 gain, AmReal32 pan, AmReal32 pitch, AmReal32 speed, AmUInt32 id, AmUInt32 layer);

        AmUInt32 PlayAdvanced(
            SoundData* sound,
            PlayStateFlag flag,
            AmReal32 gain,
            AmReal32 pan,
            AmReal32 pitch,
            AmReal32 speed,
            AmUInt64 startFrame,
            AmUInt64 endFrame,
            AmUInt32 id,
            AmUInt32 layer);

        bool SetObstruction(AmUInt32 id, AmUInt32 layer, AmReal32 obstruction);

        bool SetOcclusion(AmUInt32 id, AmUInt32 layer, AmReal32 occlusion);

        bool SetGainPan(AmUInt32 id, AmUInt32 layer, AmReal32 gain, AmReal32 pan);

        bool SetPitch(AmUInt32 id, AmUInt32 layer, AmReal32 pitch);

        bool SetCursor(AmUInt32 id, AmUInt32 layer, AmUInt64 cursor);

        bool SetPlayState(AmUInt32 id, AmUInt32 layer, PlayStateFlag flag);

        bool SetPlaySpeed(AmUInt32 id, AmUInt32 layer, AmReal32 speed);

        PlayStateFlag GetPlayState(AmUInt32 id, AmUInt32 layer);

        void SetMasterGain(AmReal32 gain);

        void StopAll();

        void HaltAll();

        void PlayAll();

        [[nodiscard]] bool IsInsideThreadMutex() const;

        void PushCommand(const MixerCommand& command);

        [[nodiscard]] const Pipeline* GetPipeline() const;

        [[nodiscard]] Pipeline* GetPipeline();

        [[nodiscard]] AM_INLINE const DeviceDescription& GetDeviceDescription() const override
        {
            return _device;
        }

        static void IncrementSoundLoopCount(SoundInstance* sound);

    private:
        void ExecuteCommands();
        void MixLayer(AmplimixLayerImpl* layer, AudioBuffer* buffer, AmUInt64 frameCount);
        AmplimixLayerImpl* GetLayer(AmUInt32 layer);
        bool ShouldMix(AmplimixLayerImpl* layer);
        void UpdatePitch(AmplimixLayerImpl* layer);
        void LockAudioMutex();
        void UnlockAudioMutex();

        bool _initialized;

        std::queue<MixerCommand> _commandsStack;

        AmMutexHandle _audioThreadMutex;
        std::unordered_map<AmThreadID, bool> _insideAudioThreadMutex;

        AmUInt32 _nextId;
        _Atomic(AmReal32) _masterGain{};
        AmplimixLayerImpl _layers[kAmplimixLayersCount];
        AmUInt64 _remainingFrames;

        Pipeline* _pipeline = nullptr;

        DeviceDescription _device;

        AudioBuffer _scratchBuffer;

        AfterMixCallback _afterMixCallback = nullptr;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_MIXER_AMPLIMIX_H
