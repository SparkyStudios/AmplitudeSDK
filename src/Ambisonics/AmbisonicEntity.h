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

#ifndef _AM_IMPLEMENTATION_AMBISONICS_AMBISONIC_ENTITY_H
#define _AM_IMPLEMENTATION_AMBISONICS_AMBISONIC_ENTITY_H

#include <SparkyStudios/Audio/Amplitude/Math/SphericalPosition.h>

#include <Ambisonics/AmbisonicComponent.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Represents an ambisonic entity.
     *
     * An ambisonic entity is a point in the 3D space, which can encode an audio
     * signal into a 3D sound field (a source), or decode a 3D sound field back into an audio signal (a listener).
     */
    class AmbisonicEntity : public AmbisonicComponent
    {
    public:
        AmbisonicEntity();

        ~AmbisonicEntity() override;

        /**
         * @copydoc AmbisonicComponent::Reset
         */
        void Reset() override;

        /**
         * @copydoc AmbisonicComponent::Refresh
         */
        void Refresh() override;

        /**
         * @copydoc AmbisonicComponent::Configure
         */
        bool Configure(AmUInt32 order, bool is3D) override;

        /**
         * @brief Sets the position of the ambisonic entity in the 3D space.
         *
         * @param position The new position of the entity.
         */
        AM_INLINE void SetPosition(const SphericalPosition& position)
        {
            m_position = position;
        }

        /**
         * @brief Sets the position of the ambisonic entity in the 3D space.
         *
         * @return The current ambisonic entity's position in the 3D space.
         */
        [[nodiscard]] AM_INLINE SphericalPosition GetPosition() const
        {
            return m_position;
        }

        /**
         * @brief Sets the weight for the spherical harmonics of the given order.
         *
         * @param order The order.
         * @param weight The weight.
         */
        AM_INLINE void SetOrderWeight(AmUInt32 order, AmReal32 weight)
        {
            m_orderWeights[order] = weight;
        }

        /**
         * @brief Sets the weights for all spherical harmonics.
         *
         * @param weight The weight.
         */
        AM_INLINE void SetWeight(AmReal32 weight)
        {
            for (AmReal32& orderWeight : m_orderWeights)
                orderWeight = weight;
        }

        /**
         * @brief Gets the weight for the spherical harmonic of the given order.
         *
         * @param order The order.
         *
         * @return The weight for the spherical harmonic of the given order.
         */
        [[nodiscard]] AM_INLINE AmReal32 GetOrderWeight(AmUInt32 order) const
        {
            return m_orderWeights[order];
        }

        /**
         * @brief Sets the spherical harmonic coefficient for the given channel.
         * Can be used for preset decoders to non-regular arrays where a Sampling decoder is sub-optimal.
         *
         * @param channel The channel index.
         * @param coefficient The coefficient for the given channel.
         */
        AM_INLINE void SetCoefficient(AmUInt32 channel, AmReal32 coefficient)
        {
            m_coefficients[channel] = coefficient;
        }

        /**
         * @brief Gets the spherical harmonic coefficient for the given channel.
         *
         * @param channel The channel index.
         * @return The spherical harmonic coefficient for the given channel.
         */
        [[nodiscard]] AM_INLINE AmReal32 GetCoefficient(AmUInt32 channel) const
        {
            return m_coefficients[channel];
        }

        /**
         * @brief Gets the vector of spherical harmonic coefficients.
         *
         * @return The vector of spherical harmonic coefficients.
         */
        [[nodiscard]] AM_INLINE AmAlignedReal32Buffer GetCoefficients() const
        {
            return m_coefficients;
        }

        /**
         * @brief Sets the gain of the ambisonic entity.
         *
         * @param gain The gain.
         */
        AM_INLINE void SetGain(AmReal32 gain)
        {
            m_gain = gain;
        }

        /**
         * @brief Gets the gain of the ambisonic entity.
         *
         * @return The gain of the ambisonic entity.
         */
        [[nodiscard]] AM_INLINE AmReal32 GetGain() const
        {
            return m_gain;
        }

    protected:
        SphericalPosition m_position;
        AmReal32 m_gain;
        AmAlignedReal32Buffer m_coefficients;
        AmAlignedReal32Buffer m_orderWeights;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_AMBISONICS_AMBISONIC_ENTITY_H
