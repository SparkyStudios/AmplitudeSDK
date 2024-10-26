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

#ifndef _AM_CORE_PLAYBACK_CHANNEL_H
#define _AM_CORE_PLAYBACK_CHANNEL_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/Core/Entity.h>
#include <SparkyStudios/Audio/Amplitude/Core/Listener.h>
#include <SparkyStudios/Audio/Amplitude/Core/Playback/ChannelEventListener.h>
#include <SparkyStudios/Audio/Amplitude/Core/Room.h>

namespace SparkyStudios::Audio::Amplitude
{
    class ChannelInternalState;

    /**
     * @brief Enumerates the playback states for a `Channel`.
     *
     * @ingroup engine
     */
    enum eChannelPlaybackState : AmUInt8
    {
        /**
         * @brief The channel is stopped and not rendering audio.
         */
        eChannelPlaybackState_Stopped = 0,

        /**
         * @brief The channel is playing audio.
         */
        eChannelPlaybackState_Playing = 1,

        /**
         * @brief The channel has just been played or resumed and is fading in to the `Playing` state.
         */
        eChannelPlaybackState_FadingIn = 2,

        /**
         * @brief The channel has just been stopped or paused and is fading out to the `Stopped` or `Paused` state.
         */
        eChannelPlaybackState_FadingOut = 3,

        /**
         * @brief The channel is updating the value of his linked switch state.
         */
        eChannelPlaybackState_SwitchingState = 4,

        /**
         * @brief The channel is playing audio but in a paused state.
         */
        eChannelPlaybackState_Paused = 5,
    };

    /**
     * @brief Enumerates the events triggered by a `Channel` during playback.
     *
     * @ingroup engine
     */
    enum class ChannelEvent : AmUInt8
    {
        Begin = 0, ///< The playback of the channel has started.
        End = 1, ///< The playback of the channel has ended.
        Resume = 2, ///< The playback of the channel has resumed after being paused.
        Pause = 3, ///< The playback of the channel has been paused.
        Stop = 4, ///< The playback of the channel has been stopped.
        Loop = 5 ///< The playback of the channel has looped.
    };

    /**
     * @brief An object that represents a single channel of audio.
     *
     * The `Channel` class is a lightweight reference to a `ChannelInternalState` object
     * which is managed by the Engine. Multiple channels may point to the same
     * underlying data.
     *
     * @ingroup engine
     */
    class AM_API_PUBLIC Channel
    {
    public:
        /**
         * @brief Construct an uninitialized `Channel`.
         *
         * An uninitialized `Channel` cannot have its data set or queried.
         */
        Channel();

        /**
         * @brief Creates a wrapper instance over the provided state.
         *
         * @param[in] state The internal state to wrap.
         *
         * @warning This constructor is for internal usage only.
         */
        explicit Channel(ChannelInternalState* state);

        /**
         * @brief Uninitializes this `Channel`.
         *
         * Note that this does not stop the audio or destroy the internal
         * state it references, it just removes this reference to it.
         */
        void Clear();

        /**
         * @brief Checks whether this `Channel` has been initialized.
         *
         * @return `true` if this `Channel` has been initialized.
         */
        [[nodiscard]] bool Valid() const;

        /**
         * @brief Gets the ID of this `Channel`.
         *
         * @return The ID of this `Channel`.
         */
        [[nodiscard]] AmUInt64 GetId() const;

        /**
         * @brief Checks if the sound associated to this `Channel` is playing.
         *
         * @return Whether the channel is currently playing.
         */
        [[nodiscard]] bool Playing() const;

        /**
         * @brief Stops the `Channel`.
         *
         * A sound will stop on its own if its not set to loop. Looped audio must be explicitly stopped.
         *
         * @param[in] duration The fade out duration before to stop the channel.
         */
        void Stop(AmTime duration = kMinFadeDuration) const;

        /**
         * @brief Pauses the `Channel`.
         *
         * A paused channel may be resumed where it left off.
         *
         * @param[in] duration The fade out duration before to pause the channel.
         */
        void Pause(AmTime duration = kMinFadeDuration) const;

        /**
         * @brief Resumes the `Channel`.
         *
         * If this channel was paused it will continue where it left off.
         *
         * @param[in] duration The fade in duration after resuming the channel.
         */
        void Resume(AmTime duration = kMinFadeDuration) const;

        /**
         * @brief Gets the location of this `Channel`.
         *
         * If the audio on this channel is not set to be Positional, this method will
         * return an invalid location.
         *
         * @return The location of this `Channel`.
         */
        [[nodiscard]] const AmVec3& GetLocation() const;

        /**
         * @brief Sets the location of this `Channel`.
         *
         * If the audio on this channel is not set to be Positional, this method
         * does nothing.
         *
         * @param[in] location The new location of the `Channel`.
         */
        void SetLocation(const AmVec3& location) const;

        /**
         * @brief Sets the gain on this `Channel`.
         *
         * @param[in] gain The new gain value.
         */
        void SetGain(AmReal32 gain) const;

        /**
         * @brief Returns the gain on this `Channel`.
         *
         * @return The channel's gain.
         */
        [[nodiscard]] AmReal32 GetGain() const;

        /**
         * @brief Returns the playback state of this `Channel`.
         *
         * @return A `ChannelPlaybackState` enumeration value representing the current state of the `Channel`.
         */
        [[nodiscard]] eChannelPlaybackState GetPlaybackState() const;

        /**
         * @brief Returns the `Entity` associated with this `Channel`.
         *
         * @note If no `Entity` is associated with this `Channel`, this method will return an
         * uninitialized `Entity` object. You should check if the entity is valid before using it.
         *
         * @return The entity associated with this `Channel`.
         *
         * @see Entity
         */
        [[nodiscard]] Entity GetEntity() const;

        /**
         * @brief Returns the `Listener` associated with this `Channel`.
         *
         * @note If no `Listener` is associated with this `Channel`, this method will return an
         * uninitialized `Listener` object. You should check if the listener is valid before using it.
         *
         * @return The listener associated with this `Channel`.
         *
         * @see Listener
         */
        [[nodiscard]] Listener GetListener() const;

        /**
         * @brief Returns the `Room` associated with this `Channel`.
         *
         * @note If no `Room` is associated with this `Channel`, this method will return an
         * uninitialized `Room` object. You should check if the room is valid before using it.
         *
         * @return The room associated with this Channel.
         */
        [[nodiscard]] Room GetRoom() const;

        /**
         * @brief Returns the internal state of this Channel.
         *
         * @return The internal state of this Channel.
         *
         * @warning This method is for internal usage only.
         */
        [[nodiscard]] ChannelInternalState* GetState() const;

        /**
         * @brief Registers a callback for a channel event.
         *
         * @param[in] event The channel event.
         * @param[in] callback The callback function.
         * @param[in] userData The user data to pass to the callback.
         *
         * @see ChannelEvent
         * @see ChannelEventCallback
         */
        void On(ChannelEvent event, ChannelEventCallback callback, void* userData = nullptr) const;

    private:
        /**
         * @brief Private constructor to wrap an existing state.
         *
         * @internal
         */
        Channel(ChannelInternalState* state, AmUInt64 id);

        /**
         * @brief Checks if the internal state ID is valid.
         *
         * @return `true` if the internal state ID is valid.
         *
         * @internal
         */
        [[nodiscard]] bool IsValidStateId() const;

        /**
         * @brief The internal state of the channel.
         *
         * @internal
         */
        ChannelInternalState* _state;

        /**
         * @brief The ID of the channel.
         *
         * @internal
         */
        AmUInt64 _stateId;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_CORE_PLAYBACK_CHANNEL_H
