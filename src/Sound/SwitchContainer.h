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

#ifndef _AM_IMPLEMENTATION_SOUND_SWITCH_CONTAINER_H
#define _AM_IMPLEMENTATION_SOUND_SWITCH_CONTAINER_H

#include <SparkyStudios/Audio/Amplitude/Sound/SwitchContainer.h>

#include <Sound/SoundObject.h>

#include "switch_container_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    class SwitchContainerImpl final
        : public SwitchContainer
        , public SoundObjectImpl
        , public AssetImpl<AmSwitchContainerID, SwitchContainerDefinition>
    {
    public:
        /**
         * @brief Creates an uninitialized switch container.
         */
        SwitchContainerImpl();

        /**
         * @brief Destroys the switch container asset and all related resources.
         */
        ~SwitchContainerImpl() override;

        /**
         * @copydoc Asset::GetId
         */
        [[nodiscard]] AM_INLINE AmSwitchContainerID GetId() const override
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
         * @copydoc SwitchContainer::GetSwitch
         */
        [[nodiscard]] const Switch* GetSwitch() const override;

        /**
         * @copydoc SwitchContainer::GetFaderIn
         */
        [[nodiscard]] FaderInstance* GetFaderIn(AmObjectID id) const override;

        /**
         * @copydoc SwitchContainer::GetFaderOut
         */
        [[nodiscard]] FaderInstance* GetFaderOut(AmObjectID id) const override;

        /**
         * @copydoc SwitchContainer::GetSoundObjects
         */
        [[nodiscard]] const std::vector<SwitchContainerItem>& GetSoundObjects(AmObjectID stateId) const override;

        /**
         * @copydoc AssetImpl::LoadDefinition
         */
        bool LoadDefinition(const SwitchContainerDefinition* definition, EngineInternalState* state) override;

        /**
         * @copydoc AssetImpl::GetDefinition
         */
        [[nodiscard]] const SwitchContainerDefinition* GetDefinition() const override;

        /**
         * @copydoc AssetImpl::AcquireReferences
         */
        void AcquireReferences(EngineInternalState* state) override;

        /**
         * @copydoc AssetImpl::ReleaseReferences
         */
        void ReleaseReferences(EngineInternalState* state) override;

    private:
        SwitchImpl* _switch;

        std::map<AmObjectID, std::vector<SwitchContainerItem>> _sounds;
        std::map<AmObjectID, std::tuple<Fader*, FaderInstance*>> _fadersIn;
        std::map<AmObjectID, std::tuple<Fader*, FaderInstance*>> _fadersOut;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif //_AM_IMPLEMENTATION_SOUND_SWITCH_CONTAINER_H
