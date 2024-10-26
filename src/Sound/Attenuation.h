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

#ifndef _AM_IMPLEMENTATION_SOUND_ATTENUATION_H
#define _AM_IMPLEMENTATION_SOUND_ATTENUATION_H

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Attenuation.h>

#include <Core/Asset.h>

#include "attenuation_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    class AttenuationZoneImpl : public AttenuationZone
    {
    public:
        ~AttenuationZoneImpl() override = default;

        /**
         * @brief Creates an AttenuationZone object from the definition.
         *
         * @param definition The attenuation shape definition.
         *
         * @return An AttenuationZone object.
         */
        static AttenuationZone* Create(const AttenuationShapeDefinition* definition);

        AmReal32 GetAttenuationFactor(const Attenuation* attenuation, const AmVec3& soundLocation, const Listener& listener) override;
        AmReal32 GetAttenuationFactor(const Attenuation* attenuation, const Entity& entity, const Listener& listener) override;

    protected:
        AttenuationZoneImpl();

        /**
         * @brief The maximum attenuation factor to apply to the sound gain.
         */
        AmReal32 m_maxAttenuationFactor;
    };

    class AttenuationImpl final
        : public Attenuation
        , public AssetImpl<AmAttenuationID, AttenuationDefinition>
    {
    public:
        /**
         * @brief Creates an uninitialized Attenuation.
         *
         * An uninitialized Attenuation instance cannot compute gain nor provide
         * attenuation configuration data.
         */
        AttenuationImpl();

        /**
         * @brief Destroys the Attenuation asset and releases all associated resources.
         */
        ~AttenuationImpl() override;

        /**
         * @copydoc Asset::GetId
         */
        [[nodiscard]] AM_INLINE AmAttenuationID GetId() const override
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
         * @copydoc Attenuation::GetGain
         */
        [[nodiscard]] AmReal32 GetGain(const AmVec3& soundLocation, const Listener& listener) const override;

        /**
         * @copydoc Attenuation::GetGain
         */
        [[nodiscard]] AmReal32 GetGain(const Entity& entity, const Listener& listener) const override;

        /**
         * @copydoc Attenuation::GetShape
         */
        [[nodiscard]] AttenuationZone* GetShape() const override;

        /**
         * @copydoc Attenuation::GetGainCurve
         */
        [[nodiscard]] AM_INLINE const Curve& GetGainCurve() const override
        {
            return _gainCurve;
        }

        /**
         * @copydoc Attenuation::GetMaxDistance
         */
        [[nodiscard]] AM_INLINE AmReal64 GetMaxDistance() const override
        {
            return _maxDistance;
        }

        /**
         * @copydoc Attenuation::IsAirAbsorptionEnabled
         */
        [[nodiscard]] AM_INLINE bool IsAirAbsorptionEnabled() const override
        {
            return _airAbsorptionEnabled;
        }

        /**
         * @copydoc Attenuation::EvaluateAirAbsorption
         */
        [[nodiscard]] AmReal32 EvaluateAirAbsorption(
            const AmVec3& soundLocation, const AmVec3& listenerLocation, AmUInt32 band) const override;

        /**
         * @copydoc AssetImpl::LoadDefinition
         */
        bool LoadDefinition(const AttenuationDefinition* definition, EngineInternalState* state) override;

        /**
         * @copydoc AssetImpl::GetDefinition
         */
        [[nodiscard]] const AttenuationDefinition* GetDefinition() const override;

    private:
        AmReal64 _maxDistance;

        AmUniquePtr<eMemoryPoolKind_Engine, AttenuationZone> _shape;

        Curve _gainCurve;

        bool _airAbsorptionEnabled;
        std::array<AmReal32, kAmAirAbsorptionBandCount> _airAbsorptionCoefficients;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_SOUND_ATTENUATION_H
