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

#ifndef _AM_CORE_ENTITY_H
#define _AM_CORE_ENTITY_H

#include <SparkyStudios/Audio/Amplitude/Math/Orientation.h>

namespace SparkyStudios::Audio::Amplitude
{
    class EntityInternalState;

    /**
     * @brief An Entity represent an object in the game.
     *
     * Amplitude use entities to link sound to an object in the game. Each sounds
     * played from an entity get the location and orientation data fom that entity.
     *
     * The Entity class is a lightweight reference to a EntityInternalState object
     * which is managed by the Engine.
     */
    class AM_API_PUBLIC Entity
    {
    public:
        /**
         * @brief Creates an uninitialized Entity.
         *
         * An uninitialized Entity cannot provide location and orientation
         * information, and therefore cannot play sounds.
         */
        Entity();

        explicit Entity(EntityInternalState* state);

        /**
         * @brief Uninitializes this Entity.
         *
         * Note that this does not destroy the internal state it references,
         * it just removes this reference to it.
         */
        void Clear();

        /**
         * @brief Checks whether this Entity has been initialized.
         *
         * @return boolean true if this Entity has been initialized.
         */
        [[nodiscard]] bool Valid() const;

        /**
         * @brief Gets the ID of this Entity in game.
         *
         * @return The game Entity ID.
         */
        [[nodiscard]] AmEntityID GetId() const;

        /**
         * @brief Gets the velocity of the Entity.
         *
         * @return The Entity's velocity.
         */
        [[nodiscard]] const AmVec3& GetVelocity() const;

        /**
         * @brief Sets the location of this Entity.
         *
         * @param location The new location.
         */
        void SetLocation(const AmVec3& location) const;

        /**
         * @brief Gets the current location of this Entity.
         *
         * @return The current location of this Entity.
         */
        [[nodiscard]] const AmVec3& GetLocation() const;

        /**
         * @brief Sets the orientation of this Entity.
         *
         * @param orientation The new orientation.
         */
        void SetOrientation(const Orientation& orientation) const;

        /**
         * @brief Get the direction vector of the Entity.
         *
         * @return The direction vector.
         */
        [[nodiscard]] AmVec3 GetDirection() const;

        /**
         * @brief Get the up vector of the Entity.
         *
         * @return The up vector.
         */
        [[nodiscard]] AmVec3 GetUp() const;

        /**
         * @brief Get the orientation of the Entity.
         *
         * @return The entity's orientation.
         */
        [[nodiscard]] const Orientation& GetOrientation() const;

        /**
         * @brief Update the state of this Entity.
         *
         * This method is called automatically by the Engine
         * on each frames.
         */
        void Update() const;

        /**
         * @brief Set the obstruction level of sounds played by this Entity.
         *
         * @param obstruction The obstruction amount. This is provided by the
         * game engine.
         */
        void SetObstruction(AmReal32 obstruction) const;

        /**
         * @brief Set the occlusion level of sounds played by this Entity.
         *
         * @param occlusion The occlusion amount. This is provided by the
         * game engine.
         */
        void SetOcclusion(AmReal32 occlusion) const;

        /**
         * @brief Sets the directivity and sharpness of sounds played by this Entity.
         *
         * @param directivity The directivity of the sound source, in the range [0, 1].
         * @param sharpness The directivity sharpness of the sound source, in the range [1, +INF].
         * Increasing this value increases the directivity towards the front of the source.
         */
        void SetDirectivity(AmReal32 directivity, AmReal32 sharpness) const;

        /**
         * @brief Get the obstruction level of sounds played by this Entity.
         *
         * @return The obstruction amount.
         */
        [[nodiscard]] AmReal32 GetObstruction() const;

        /**
         * @brief Get the occlusion level of sounds played by this Entity.
         *
         * @return The occlusion amount.
         */
        [[nodiscard]] AmReal32 GetOcclusion() const;

        /**
         * @brief Gets the directivity of sounds played by this Entity.
         *
         * @return The directivity of sound sources.
         */
        [[nodiscard]] AmReal32 GetDirectivity() const;

        /**
         * @brief Gets the directivity sharpness of sounds played by this Entity.
         *
         * @return The directivity sharpness of sounds played by this Entity.
         */
        [[nodiscard]] AmReal32 GetDirectivitySharpness() const;

        /**
         * @brief Sets the environment factor for this Entity in the given environment.
         *
         * @param environment The environment ID.
         * @param factor The environment factor.
         */
        void SetEnvironmentFactor(AmEnvironmentID environment, AmReal32 factor) const;

        /**
         * @brief Gets the environment factor of this Entity for the given environment.
         *
         * @param environment The environment ID.
         *
         * @return The environment factor.
         */
        [[nodiscard]] AmReal32 GetEnvironmentFactor(AmEnvironmentID environment) const;

        /**
         * @brief Get the list of environments where this Entity belongs or has visited.
         *
         * @return The list of environments where this Entity belongs or has visited.
         */
        [[nodiscard]] const std::map<AmEnvironmentID, AmReal32>& GetEnvironments() const;

        /**
         * @brief Returns the internal state of this Entity.
         *
         * @return The Entity internal state.
         */
        [[nodiscard]] EntityInternalState* GetState() const;

    private:
        EntityInternalState* _state;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_CORE_ENTITY_H
