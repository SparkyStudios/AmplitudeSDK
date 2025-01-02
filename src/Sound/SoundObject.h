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

#ifndef _AM_IMPLEMENTATION_SOUND_SOUND_OBJECT_H
#define _AM_IMPLEMENTATION_SOUND_SOUND_OBJECT_H

#include <SparkyStudios/Audio/Amplitude/Sound/SoundObject.h>

namespace SparkyStudios::Audio::Amplitude
{
    class EffectImpl;
    class AttenuationImpl;

    class SoundObjectImpl : public SoundObject
    {
    public:
        SoundObjectImpl()
            : m_bus(nullptr)
            , m_gain()
            , m_priority()
            , m_effect(nullptr)
            , m_attenuation(nullptr)
            , m_spatialization(eSpatialization_None)
            , m_scope(eScope_World)
        {}

        ~SoundObjectImpl() override = default;

        /**
         * @copydoc SoundObject::GetGain
         */
        [[nodiscard]] AM_INLINE const RtpcValue& GetGain() const override
        {
            return m_gain;
        }

        /**
         * @copydoc SoundObject::GetPitch
         */
        [[nodiscard]] AM_INLINE const RtpcValue& GetPitch() const override
        {
            return m_pitch;
        }

        /**
         * @copydoc SoundObject::GetPriority
         */
        [[nodiscard]] AM_INLINE const RtpcValue& GetPriority() const override
        {
            return m_priority;
        }

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
        [[nodiscard]] AM_INLINE Bus GetBus() const override
        {
            return Bus(m_bus);
        }

        /**
         * @copydoc SoundObject::GetSpatialization
         */
        [[nodiscard]] AM_INLINE eSpatialization GetSpatialization() const override
        {
            return m_spatialization;
        }

        /**
         * @copydoc SoundObject::GetScope
         */
        [[nodiscard]] AM_INLINE eScope GetScope() const override
        {
            return m_scope;
        }

    protected:
        // The bus this sound object will play on.
        BusInternalState* m_bus;

        RtpcValue m_gain;
        RtpcValue m_pitch;
        RtpcValue m_priority;

        EffectImpl* m_effect;
        AttenuationImpl* m_attenuation;

        eSpatialization m_spatialization;
        eScope m_scope;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_SOUND_SOUND_OBJECT_H
