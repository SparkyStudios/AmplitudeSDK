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

#ifndef _AM_SOUND_SWITCH_H
#define _AM_SOUND_SWITCH_H

#include <SparkyStudios/Audio/Amplitude/Core/Asset.h>
#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief A switch state.
     *
     * A switch state is a single state that can be applied to a `SwitchContainer` to control which sounds are played.
     * Only one state can be active at a time in the same `Switch` asset.
     *
     * @see [Switch](../../assets/Switch/index.md), [SwitchContainer](../../assets/SwitchContainer/index.md)
     *
     * @ingroup engine
     */
    struct AM_API_PUBLIC SwitchState
    {
        /**
         * @brief The ID of this switch state.
         *
         * This ID is unique only in the parent switch.
         */
        AmObjectID m_id;

        /**
         * @brief The name of this switch state.
         */
        AmString m_name;

        /**
         * @brief Checks whether this switch state is valid.
         *
         * @return `true` if the switch state is valid, `false` otherwise.
         */
        [[nodiscard]] bool Valid() const;

        /**
         * @brief Compares this switch state with another one for equality.
         *
         * @param[im] other The other switch state to compare with.
         *
         * @return `true` if the switch states are equal, `false` otherwise.
         */
        bool operator==(const SwitchState& other) const;

        /**
         * @brief Compares this switch state with another one for inequality.
         *
         * @param[in] other The other switch state to compare with.
         *
         * @return `true` if the switch states are not equal, `false` otherwise.
         */
        bool operator!=(const SwitchState& other) const;
    };

    /**
     * @brief Amplitude Switch Asset.
     *
     * A switch is a collection of states which can change the sounds played from a `SwitchContainer`.
     *
     * For example, you can have a switch named `SurfaceType` which have `wood`, `grass`, `metal` and `water` as states. A
     * `SwitchContainer` using this switch can group sounds per switch states, so when a state is active, all the sounds of
     * that state are played. Changing the state of a `Switch` will update ALL the `SwitchContainer` objects that use this `Switch`.
     *
     * The `Switch` is a shared object between sound sources. They are used only by `SwitchContainer` objects.
     *
     * @see [SwitchState](../../engine/SwitchState/index.md), [SwitchContainer](../../assets/SwitchContainer/index.md)
     *
     * @ingroup assets
     */
    class AM_API_PUBLIC Switch : public Asset<AmSwitchID>
    {
    public:
        /**
         * @brief Gets the current state of the switch.
         *
         * @return The current state of the switch.
         */
        [[nodiscard]] virtual const SwitchState& GetState() const = 0;

        /**
         * @brief Sets the current state of the switch.
         *
         * @note Changing the state of a `Switch` will update ALL the `SwitchContainer` objects that use this `Switch`.
         *
         * @param[in] state The state to apply to the switch.
         */
        virtual void SetState(const SwitchState& state) = 0;

        /**
         * @brief Sets the current state of the switch using the state ID.
         *
         * @note Changing the state of a `Switch` will update ALL the `SwitchContainer` objects that use this `Switch`.
         *
         * @param[in] id The ID of the state to apply. This ID should exist in the list
         * of registered switch states.
         */
        virtual void SetState(AmObjectID id) = 0;

        /**
         * @brief Sets the current state of the switch using the state name.
         *
         * @note Changing the state of a `Switch` will update ALL the `SwitchContainer` objects that use this `Switch`.
         *
         * @param[in] name The name of the state to apply. This name should exist in the
         * list of registered switch states.
         */
        virtual void SetState(const AmString& name) = 0;

        /**
         * @brief Gets the list of available SwitchStates in this Switch.
         *
         * @return The list of available SwitchStates.
         */
        [[nodiscard]] virtual const std::vector<SwitchState>& GetSwitchStates() const = 0;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_SOUND_SWITCH_H
