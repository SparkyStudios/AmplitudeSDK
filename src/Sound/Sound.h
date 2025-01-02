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

#ifndef _AM_IMPLEMENTATION_SOUND_SOUND_H
#define _AM_IMPLEMENTATION_SOUND_SOUND_H

#include <SparkyStudios/Audio/Amplitude/Core/Codec.h>
#include <SparkyStudios/Audio/Amplitude/Core/Playback/Channel.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Sound.h>

#include <Core/Asset.h>
#include <IO/Resource.h>
#include <Sound/SoundObject.h>

#include "sound_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    class SoundInstance;
    class CollectionImpl;
    class EffectInstance;
    class RealChannel;
    struct SoundChunk;

    /**
     * @brief Describes the place where a Sound belongs to.
     */
    enum class SoundKind
    {
        /**
         * @brief The sound is associated to a @a SwitchContainer.
         */
        Switched,

        /**
         * @brief The sound is associated to a @a Collection.
         */
        Contained,

        /**
         * @brief The sound is not associated to any container, and can be played directly.
         */
        Standalone,
    };

    /**
     * @brief Stores all the information required to play a sound instance.
     */
    struct SoundInstanceSettings
    {
        AmObjectID m_id;
        SoundKind m_kind;
        AmBusID m_busID;
        AmAttenuationID m_attenuationID;
        AmEffectID m_effectID;
        AmUInt8 m_spatialization;
        RtpcValue m_priority;
        RtpcValue m_gain;
        RtpcValue m_nearFieldGain;
        RtpcValue m_pitch;
        bool m_loop;
        AmUInt32 m_loopCount;
    };

    class SoundImpl final
        : public Sound
        , public SoundObjectImpl
        , public ResourceImpl
        , public AssetImpl<AmSoundID, SoundDefinition>
    {
        friend class CollectionImpl;
        friend class SoundInstance;

    public:
        /**
         * @brief Destroys a SoundInstance.
         *
         * @param soundInstance The sound instance to destroy.
         */
        static void DestroyInstance(SoundInstance* soundInstance);

        SoundImpl();

        ~SoundImpl() override;

        /**
         * @brief Create a new SoundInstance from this Sound.
         *
         * The SoundInstance created this way will use settings fom the Sound object.
         *
         * @return SoundInstance* A sound instance which can be played.
         */
        [[nodiscard]] SoundInstance* CreateInstance();

        /**
         * @brief Create a new SoundInstance from this Sound.
         *
         * The SoundInstance created this way will use the settings fom the given
         * Collection. This is mainly used when a Collection play a sound fom his container.
         *
         * @param collection The Collection that the sound is part of.
         *
         * @return SoundInstance* A sound instance which can be played.
         */
        [[nodiscard]] SoundInstance* CreateInstance(const CollectionImpl* collection);

        /**
         * @brief Returns the SoundChunk associated with this Sound
         * and increment its reference counter.
         *
         * If the reference equals 0, the SoundChunk is created.
         *
         * This method is used by the SoundInstance to get the SoundChunk
         * only when the audio file is not streamed.
         *
         * @return The SoundChunk of this sound.
         */
        SoundChunk* AcquireSoundData();

        /**
         * @brief Decrements the SoundChunk's reference counter.
         *
         * If the reference counter reaches 0, the SoundChunk is deleted.
         *
         * This method is used by the SoundInstance to get the SoundChunk
         * only when the audio file is not streamed.
         *
         * @return The SoundChunk of this sound.
         */
        void ReleaseSoundData();

        /**
         * @copydoc Asset::GetId
         */
        [[nodiscard]] AmSoundID GetId() const override;

        /**
         * @copydoc Asset::GetName
         */
        [[nodiscard]] const AmString& GetName() const override;

        /**
         * @copydoc AssetImpl::LoadDefinition
         */
        bool LoadDefinition(const SoundDefinition* definition, EngineInternalState* state) override;

        /**
         * @copydoc AssetImpl::GetDefinition
         */
        [[nodiscard]] const SoundDefinition* GetDefinition() const override;

        /**
         * @copydoc AssetImpl::AcquireReferences
         */
        void AcquireReferences(EngineInternalState* state) override;

        /**
         * @copydoc AssetImpl::ReleaseReferences
         */
        void ReleaseReferences(EngineInternalState* state) override;

        /**
         * @copydoc Resource::GetPath
         */
        [[nodiscard]] const AmOsString& GetPath() const override;

        /**
         * @copydoc Resource::Load
         */
        void Load(const FileSystem* loader) override;

        /**
         * @copydoc SoundObject::GetGain
         */
        [[nodiscard]] const RtpcValue& GetGain() const override;

        /**
         * @brief Gets the near field effect gain.
         */
        [[nodiscard]] const RtpcValue& GetNearFieldGain() const override;

        /**
         * @copydoc SoundObject::GetPitch
         */
        [[nodiscard]] const RtpcValue& GetPitch() const override;

        /**
         * @copydoc SoundObject::GetPriority
         */
        [[nodiscard]] const RtpcValue& GetPriority() const override;

        /**
         * @copydoc SoundObject::GetEffect
         */
        [[nodiscard]] const Effect* GetEffect() const override;

        /**
         * @copydoc SoundObject::GetAttenuation
         */
        [[nodiscard]] const Attenuation* GetAttenuation() const override;

        /**
         * @copydoc SoundObject::GetBus
         */
        [[nodiscard]] Bus GetBus() const override;

        /**
         * @copydoc SoundObject::GetSpatialization
         */
        [[nodiscard]] AM_INLINE eSpatialization GetSpatialization() const override
        {
            return SoundObjectImpl::GetSpatialization();
        }

        /**
         * @copydoc SoundObject::GetScope
         */
        [[nodiscard]] AM_INLINE eScope GetScope() const override
        {
            return SoundObjectImpl::GetScope();
        }

        /**
         * @copydoc Sound::IsStream
         */
        [[nodiscard]] bool IsStream() const override;

        /**
         * @copydoc Sound::IsLoop
         */
        [[nodiscard]] bool IsLoop() const override;

    private:
        Codec* _codec;
        Codec::Decoder* _decoder;

        bool _stream;
        bool _loop;
        AmUInt32 _loopCount;

        SoundChunk* _soundData;
        SoundFormat _format;
        RefCounter _soundDataRefCounter;

        RtpcValue _nearFieldGain;

        SoundInstanceSettings _settings;
    };

    class SoundInstance
    {
        friend class AmplimixImpl;
        friend class SoundImpl;

    public:
        /**
         * @brief Creates a new SoundInstance from the given Sound.
         *
         * @param parent The parent Sound from which to create an instance.
         * @param settings The settings of the Sound instance.
         * @param effect The sound effect to apply on playback.
         */
        SoundInstance(SoundImpl* parent, SoundInstanceSettings settings, const EffectImpl* effect = nullptr);
        ~SoundInstance();

        /**
         * @brief Loads the audio sample data into this SoundInstance.
         */
        void Load();

        /**
         * @brief Get the settings used to create this SoundInstance.
         *
         * @return The SoundInstance settings.
         */
        [[nodiscard]] const SoundInstanceSettings& GetSettings() const;

        /**
         * @brief Returns the user data associated to this Sound.
         * @return The user data.
         */
        [[nodiscard]] AmVoidPtr GetUserData() const;

        /**
         * @brief Sets the user data associated to this Sound.
         * @param userData The user data.
         */
        void SetUserData(AmVoidPtr userData);

        /**
         * @brief Renders audio data.
         *
         * The audio data is read from the audio file loaded by this
         * Sound. This function is mostly for internal uses.
         *
         * @param offset The offset in the audio data to start reading from.
         * @param frames The number of audio frames to read.
         *
         * @return The number of frames read.
         */
        [[nodiscard]] AmUInt64 GetAudio(AmUInt64 offset, AmUInt64 frames) const;

        /**
         * @brief Destroys the audio sample loaded by this Sound and
         * releases all resources.
         */
        void Destroy();

        /**
         * @brief Checks if this SoundInstance has a valid state.
         *
         * @return true when the state of this SoundInstance is valid.
         * @return false when the state of this SoundInstance is not valid.
         */
        [[nodiscard]] bool Valid() const;

        /**
         * @brief Sets the RealChannel in which this SoundInstance is playing.
         *
         * @param channel The RealChannel instance in which this SoundInstance is playing.
         */
        void SetChannel(RealChannel* channel);

        /**
         * @brief Gets the RealChannel in which this SoundInstance is playing.
         *
         * @return The RealChannel instance in which this SoundInstance is playing.
         */
        [[nodiscard]] Channel GetChannel() const;

        /**
         * @brief Gets the RealChannel in which this SoundInstance is playing.
         *
         * @return The RealChannel instance in which this SoundInstance is playing.
         */
        [[nodiscard]] RealChannel* GetRealChannel() const;

        /**
         * @brief Gets the Sound source which have generated this SoundInstance.
         *
         * @return The Sound object which is the source of this instance.
         */
        [[nodiscard]] const Sound* GetSound() const;

        /**
         * @brief Gets the Collection which have generated this SoundInstance.
         *
         * @return The Collection object which have generated this instance, if any.
         */
        [[nodiscard]] const CollectionImpl* GetCollection() const;

        /**
         * @brief Gets the number of times this SoundInstance have looped.
         *
         * @return The number of times this sound instance have looped.
         */
        [[nodiscard]] AmUInt32 GetCurrentLoopCount() const;

        /**
         * @brief Gets the effect applied to this SoundInstance, if any.
         *
         * @return An EffectInstance object if an effect was applied to the sound which have
         * generated this SoundInstance, or nullptr otherwise.
         */
        [[nodiscard]] const EffectInstance* GetEffect() const;

        /**
         * @brief Set the obstruction level of sounds played by this Entity.
         *
         * @param obstruction The obstruction amount. This is provided by the
         * game engine.
         */
        void SetObstruction(AmReal32 obstruction);

        /**
         * @brief Set the occlusion level of sounds played by this Entity.
         *
         * @param occlusion The occlusion amount. This is provided by the
         * game engine.
         */
        void SetOcclusion(AmReal32 occlusion);

        /**
         * @brief Get the generated sound instance ID.
         *
         * This generated ID is ensured to be unique within all the sound instances
         * created within the engine.
         *
         * @return A generated sound instance ID.
         */
        [[nodiscard]] AmObjectID GetId() const;

    private:
        AmVoidPtr _userData;

        RealChannel* _channel;
        SoundImpl* _parent;
        const CollectionImpl* _collection;
        const EffectImpl* _effect;
        EffectInstance* _effectInstance;
        Codec::Decoder* _decoder;

        SoundInstanceSettings _settings;

        AmUInt32 _currentLoopCount;

        AmObjectID _id;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_SOUND_SOUND_H
