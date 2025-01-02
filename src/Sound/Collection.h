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

#ifndef _AM_IMPLEMENTATION_SOUND_COLLECTION_H
#define _AM_IMPLEMENTATION_SOUND_COLLECTION_H

#include <SparkyStudios/Audio/Amplitude/Sound/Collection.h>

#include <Core/Asset.h>
#include <Sound/Scheduler.h>
#include <Sound/Sound.h>
#include <Sound/SoundObject.h>

#include "collection_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    class CollectionImpl final
        : public Collection
        , public SoundObjectImpl
        , public AssetImpl<AmSoundID, CollectionDefinition>
    {
        friend class SoundImpl;

    public:
        /**
         * @brief Creates an uninitialized collection.
         */
        CollectionImpl();

        /**
         * @brief Destroys the collection asset and all related resources.
         */
        ~CollectionImpl() override;

        /**
         * @copydoc Asset::GetId
         */
        [[nodiscard]] AM_INLINE AmCollectionID GetId() const override
        {
            return AssetImpl::GetId();
        }

        /**
         * @copydoc Asset::GetName
         */
        [[nodiscard]] AM_INLINE const AmString& GetName() const override
        {
            return AssetImpl::GetName();
        }

        /**
         * @copydoc SoundObject::GetGain
         */
        [[nodiscard]] AM_INLINE const RtpcValue& GetGain() const override
        {
            return SoundObjectImpl::GetGain();
        }

        /**
         * @copydoc SoundObject::GetPitch
         */
        [[nodiscard]] AM_INLINE const RtpcValue& GetPitch() const override
        {
            return SoundObjectImpl::GetPitch();
        }

        /**
         * @copydoc SoundObject::GetPriority
         */
        [[nodiscard]] AM_INLINE const RtpcValue& GetPriority() const override
        {
            return SoundObjectImpl::GetPriority();
        }

        /**
         * @copydoc SoundObject::GetEffect
         */
        [[nodiscard]] AM_INLINE const Effect* GetEffect() const override
        {
            return SoundObjectImpl::GetEffect();
        }

        /**
         * @copydoc SoundObject::GetAttenuation
         */
        [[nodiscard]] AM_INLINE const Attenuation* GetAttenuation() const override
        {
            return SoundObjectImpl::GetAttenuation();
        }

        /**
         * @copydoc SoundObject::GetBus
         */
        [[nodiscard]] AM_INLINE Bus GetBus() const override
        {
            return SoundObjectImpl::GetBus();
        }

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
         * @copydoc Collection::SelectFromWorld
         */
        [[nodiscard]] Sound* SelectFromWorld(const std::vector<AmSoundID>& toSkip) const override;

        /**
         * @copydoc Collection::SelectFromEntity
         */
        Sound* SelectFromEntity(const Entity& entity, const std::vector<AmSoundID>& toSkip) override;

        /**
         * @copydoc Collection::ResetEntityScopeScheduler
         */
        void ResetEntityScopeScheduler(const Entity& entity) override;

        /**
         * @copydoc Collection::ResetWorldScopeScheduler
         */
        void ResetWorldScopeScheduler() override;

        /**
         * @copydoc Collection::GetSounds
         */
        [[nodiscard]] const std::vector<AmSoundID>& GetSounds() const override;

        /**
         * @copydoc AssetImpl::LoadDefinition
         */
        bool LoadDefinition(const CollectionDefinition* definition, EngineInternalState* state) override;

        /**
         * @copydoc AssetImpl::GetDefinition
         */
        [[nodiscard]] const CollectionDefinition* GetDefinition() const override;

        /**
         * @copydoc AssetImpl::AcquireReferences
         */
        void AcquireReferences(EngineInternalState* state) override;

        /**
         * @copydoc AssetImpl::ReleaseReferences
         */
        void ReleaseReferences(EngineInternalState* state) override;

    private:
        static Scheduler* CreateScheduler(const CollectionDefinition* definition);

        // The World scope sound scheduler
        Scheduler* _worldScopeScheduler;

        // Entity scope sound schedulers
        std::map<AmUInt64, Scheduler*> _entityScopeSchedulers;

        std::vector<AmSoundID> _sounds;
        std::map<AmSoundID, SoundInstanceSettings> _soundSettings;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_SOUND_COLLECTION_H
