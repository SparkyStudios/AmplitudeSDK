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

#ifndef _AM_SOUND_ATTENUATION_H
#define _AM_SOUND_ATTENUATION_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

#include <SparkyStudios/Audio/Amplitude/Core/Asset.h>
#include <SparkyStudios/Audio/Amplitude/Core/Entity.h>
#include <SparkyStudios/Audio/Amplitude/Core/Listener.h>

#include <SparkyStudios/Audio/Amplitude/IO/File.h>

#include <SparkyStudios/Audio/Amplitude/Math/Curve.h>

namespace SparkyStudios::Audio::Amplitude
{
    class Attenuation;

    /**
     * @brief The propagation shape for positional sounds.
     *
     * This allows to increase the attenuation according to the shape of
     * the sound propagation.
     */
    class AM_API_PUBLIC AttenuationZone
    {
    public:
        virtual ~AttenuationZone() = default;

        /**
         * @brief Returns the attenuation factor.
         *
         * This method is used only for position based sound sources.
         *
         * @param attenuation The Attenuator object to use for distance attenuation.
         * @param soundLocation The location of the sound source.
         * @param listener The listener for which compute the attenuation.
         *
         * @return The attenuation factor.
         */
        virtual AmReal32 GetAttenuationFactor(const Attenuation* attenuation, const AmVec3& soundLocation, const Listener& listener) = 0;

        /**
         * @brief Returns the attenuation factor.
         *
         * This method is used by position and orientation based sound sources.
         *
         * @param attenuation The Attenuator object to use for distance attenuation.
         * @param entity The entity which emits the sound.
         * @param listener The listener for which compute the attenuation.
         *
         * @return The attenuation factor.
         */
        virtual AmReal32 GetAttenuationFactor(const Attenuation* attenuation, const Entity& entity, const Listener& listener) = 0;
    };

    /**
     * @brief Amplitude Attenuation.
     *
     * An Attenuation materializes how the sound volume and other distance-based
     * parameters are calculated following the distance of the sound source to the listener.
     *
     * The Attenuation is a shared object between sound sources. They are used only
     * when the sound need to adjust his volume due to the distance of from the listener,
     * and many other parameters.
     */
    class AM_API_PUBLIC Attenuation : public Asset<AmAttenuationID>
    {
    public:
        /**
         * @brief Returns the gain of the sound from the given distance to the listener.
         *
         * @param soundLocation The location of the sound source.
         * @param listener The listener which is hearing the sound.
         *
         * @return The computed gain value fom the curve.
         */
        [[nodiscard]] virtual AmReal32 GetGain(const AmVec3& soundLocation, const Listener& listener) const = 0;

        /**
         * @brief Returns the gain of the sound from the given distance to the listener.
         *
         * @param entity The entity which emits the sound.
         * @param listener The listener which is hearing the sound.
         *
         * @return The computed gain value fom the curve.
         */
        [[nodiscard]] virtual AmReal32 GetGain(const Entity& entity, const Listener& listener) const = 0;

        /**
         * @brief Returns the shape object of this Attenuation.
         *
         * @return The Attenuation shape.
         */
        [[nodiscard]] virtual AttenuationZone* GetShape() const = 0;

        /**
         * @brief Returns the gain curve attached to this Attenuation.
         *
         * @return The attenuation's gain curve.
         */
        [[nodiscard]] virtual const Curve& GetGainCurve() const = 0;

        /**
         * @brief Returns the maximum distance for a fully attenuated sound
         *
         * @return The maximum sound attenuation distance.
         */
        [[nodiscard]] virtual AmReal64 GetMaxDistance() const = 0;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_SOUND_ATTENUATION_H
