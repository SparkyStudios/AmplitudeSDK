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

#ifndef _AM_IMPLEMENTATION_CORE_PLAYBACK_CHANNEL_INTERNAL_STATE_H
#define _AM_IMPLEMENTATION_CORE_PLAYBACK_CHANNEL_INTERNAL_STATE_H

#include <utility>

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

#include <SparkyStudios/Audio/Amplitude/Core/Entity.h>
#include <SparkyStudios/Audio/Amplitude/Core/Playback/Channel.h>
#include <SparkyStudios/Audio/Amplitude/Core/Playback/ChannelEventListener.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Collection.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Sound.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Switch.h>
#include <SparkyStudios/Audio/Amplitude/Sound/SwitchContainer.h>

#include <Mixer/RealChannel.h>
#include <Utils/intrusive_list.h>

namespace SparkyStudios::Audio::Amplitude
{
    class SwitchImpl;
    class SoundImpl;
    class CollectionImpl;
    class SwitchContainerImpl;

    typedef fplutil::intrusive_list<class ChannelInternalState> ChannelList;

    // Represents a sample that is playing on a channel.
    class ChannelInternalState
    {
    public:
        ChannelInternalState()
            : _realChannel(this)
            , _channelState(eChannelPlaybackState_Stopped)
            , _switchContainer(nullptr)
            , _collection(nullptr)
            , _sound(nullptr)
            , _switch(nullptr)
            , _playingSwitchContainerStateId(kAmInvalidObjectId)
            , _previousSwitchContainerStateId(kAmInvalidObjectId)
            , _fader(nullptr)
            , _targetFadeOutState(eChannelPlaybackState_Stopped)
            , _entity()
            , _userGain(1.0f)
            , _gain(1.0f)
            , _pan()
            , _pitch(1.0f)
            , _location()
            , _directivity(0.0f)
            , _directivitySharpness(1.0f)
            , _channelStateId(kAmInvalidObjectId)
            , _dopplerFactors()
        {}

        // Updates the state enum based on whether this channel is stopped, playing,
        // etc.
        void UpdateState();

        // Remove this channel from all lists that it is a part of.
        void Remove();

        /**
         * @brief Resets this channel to its initial state.
         */
        void Reset();

        // Set the switch container playing on this channel. Note that when you set
        // the switch container, you also add this channel to the bus list that
        // corresponds to that switch container.
        void SetSwitchContainer(SwitchContainerImpl* switchContainer);

        // Get or set the collection playing on this channel. Note that when you set
        // the collection, you also add this channel to the bus list that
        // corresponds to that collection.
        void SetCollection(CollectionImpl* collection);

        // Get or set the sound playing on this channel. Note that when you set
        // the sound, you also add this channel to the bus list that
        // corresponds to that sound.
        void SetSound(SoundImpl* sound);

        [[nodiscard]] AM_INLINE SwitchContainerImpl* GetSwitchContainer() const
        {
            return _switchContainer;
        }

        [[nodiscard]] AM_INLINE CollectionImpl* GetCollection() const
        {
            return _collection;
        }

        [[nodiscard]] AM_INLINE SoundImpl* GetSound() const
        {
            return _sound;
        }

        void SetEntity(const Entity& entity);

        AM_INLINE Entity& GetEntity()
        {
            return _entity;
        }

        [[nodiscard]] AM_INLINE const Entity& GetEntity() const
        {
            return _entity;
        }

        void SetListener(const Listener& listener);

        AM_INLINE Listener& GetListener()
        {
            return _activeListener;
        }

        [[nodiscard]] AM_INLINE const Listener& GetListener() const
        {
            return _activeListener;
        }

        void SetRoom(const Room& room);

        AM_INLINE Room& GetRoom()
        {
            return _room;
        }

        [[nodiscard]] AM_INLINE const Room& GetRoom() const
        {
            return _room;
        }

        // Get the current state of this channel (playing, stopped, paused, etc.). This
        // is tracked manually because not all ChannelInternalStates are backed by
        // real channels.
        [[nodiscard]] AM_INLINE eChannelPlaybackState GetChannelState() const
        {
            return _channelState;
        }

        // Get or set the location of this channel
        AM_INLINE void SetLocation(const AmVec3& location)
        {
            // Entity scoped channel
            if (_entity.Valid())
                return;

            // World scoped channel
            _location = location;
        }

        [[nodiscard]] AM_INLINE const AmVec3& GetLocation() const
        {
            // Entity scoped channel
            if (_entity.Valid())
                return _entity.GetLocation();

            // World scoped channel
            return _location;
        }

        // Play the sound associated to this channel.
        bool Play();

        // Check if this channel is currently playing on a real or virtual channel.
        [[nodiscard]] bool Playing() const;

        // Check if this channel is currently stopped on a real or virtual channel.
        [[nodiscard]] bool Stopped() const;

        // Check if this channel is currently paused on a real or virtual channel.
        [[nodiscard]] bool Paused() const;

        // Set and query the user gain of this channel.
        AM_INLINE void SetUserGain(const AmReal32 user_gain)
        {
            _userGain = user_gain;
        }

        [[nodiscard]] AM_INLINE AmReal32 GetUserGain() const
        {
            return _userGain;
        }

        // Set and query the current gain of this channel.
        void SetGain(AmReal32 gain);

        [[nodiscard]] AM_INLINE AmReal32 GetGain() const
        {
            return _gain;
        }

        void SetPitch(AmReal32 pitch);

        [[nodiscard]] AmReal32 GetPitch() const;

        /**
         * @brief Sets the directivity of souund.
         *
         * @param directivity The directivity of the sound.
         * @param directivitySharpness The sharpness of the directivity.
         */
        void SetDirectivity(AmReal32 directivity, AmReal32 directivitySharpness);

        /**
         * @brief Get the directivity of the sound source.
         *
         * @return The directivity.
         */
        [[nodiscard]] AM_INLINE AmReal32 GetDirectivity() const
        {
            return _directivity;
        }

        /**
         * @brief Get the sharpness of the sound source directivity.
         *
         * @return The directivity sharpness.
         */
        [[nodiscard]] AM_INLINE AmReal32 GetDirectivitySharpness() const
        {
            return _directivitySharpness;
        }

        // Immediately stop the audio. May cause clicking.
        void Halt();

        // Pauses this channel.
        void Pause();

        // Resumes this channel if it is paused.
        void Resume();

        // Fade in over the specified number of milliseconds.
        void FadeIn(AmTime duration);

        // Fade out over the specified number of milliseconds.
        void FadeOut(AmTime duration, eChannelPlaybackState targetState = eChannelPlaybackState_Stopped);

        // Sets the pan based on a position in a unit circle.
        void SetPan(const AmVec2& pan);

        // Returns the pan of this channel.
        [[nodiscard]] AM_INLINE const AmVec2& GetPan() const
        {
            return _pan;
        }

        // Devirtualizes a virtual channel. This transfers ownership of the given
        // channel's channel_id to this channel.
        void Devirtualize(ChannelInternalState* other);

        // Returns the priority of this channel based on its gain and priority
        // multiplier on the sound collection definition.
        [[nodiscard]] AmReal32 Priority() const;

        /**
         * @brief Update this channel data per frames.
         *
         * @param deltaTime The time elapsed since the last frame.
         */
        void AdvanceFrame(AmTime deltaTime);

        // Returns the real channel.
        AM_INLINE RealChannel& GetRealChannel()
        {
            return _realChannel;
        }

        [[nodiscard]] AM_INLINE const RealChannel& GetRealChannel() const
        {
            return _realChannel;
        }

        [[nodiscard]] AM_INLINE bool Valid() const
        {
            return IsAlive() && IsReal();
        }

        [[nodiscard]] AM_INLINE bool IsAlive() const
        {
            return _sound != nullptr || _collection != nullptr || _switchContainer != nullptr;
        }

        // Returns true if the real channel is valid.
        [[nodiscard]] AM_INLINE bool IsReal() const
        {
            return _realChannel.Valid();
        }

        [[nodiscard]] AmObjectID GetPlayingObjectId() const;

        [[nodiscard]] AM_INLINE AmUInt32 GetChannelStateId() const
        {
            return _channelStateId;
        }

        AM_INLINE void SetChannelStateId(AmUInt64 id)
        {
            _channelStateId = id;
        }

        /**
         * @brief Set the obstruction level of sounds played by this Channel.
         *
         * @param obstruction The obstruction amount. This is provided by the
         * game engine.
         */
        void SetObstruction(AmReal32 obstruction);

        /**
         * @brief Set the occlusion level of sounds played by this Channel.
         *
         * @param occlusion The occlusion amount. This is provided by the
         * game engine.
         */
        void SetOcclusion(AmReal32 occlusion);

        /**
         * @brief Get the Doppler factor of this sound for the given Listener.
         *
         * @param listener The listener to get the Doppler factor for.
         *
         * @return A Doppler factor value for the given Listener.
         */
        [[nodiscard]] AmReal32 GetDopplerFactor(AmListenerID listener) const;

        [[nodiscard]] AmReal32 GetRoomGain(AmRoomID room) const;

        void HaltInternal();

        /**
         * @brief Registers a callback for a channel event.
         *
         * @param event The channel event.
         * @param callback The callback function.
         * @param userData The user data to pass to the callback.
         */
        void On(ChannelEvent event, ChannelEventCallback callback, void* userData = nullptr);

        void Trigger(ChannelEvent event);

        // The node that tracks the location in the priority list.
        fplutil::intrusive_list_node priority_node;

        // The node that tracks the location in the free list.
        fplutil::intrusive_list_node free_node;

        // The node that tracks the list of sounds playing on a given bus.
        fplutil::intrusive_list_node bus_node;

        // The node that tracks the list of sounds playing on a given entity.
        fplutil::intrusive_list_node entity_node;

        // The node that tracks the list of sounds rendered by a given listener.
        fplutil::intrusive_list_node listener_node;

        // The node that tracks the list of sounds playing in a given room.
        fplutil::intrusive_list_node room_node;

    private:
        bool PlaySwitchContainerStateUpdate(const std::vector<SwitchContainerItem>& previous, const std::vector<SwitchContainerItem>& next);
        bool PlaySwitchContainer();
        bool PlayCollection();
        bool PlaySound();

        // The real channel feeding the mixer with audio data.
        RealChannel _realChannel;

        // Whether this channel is currently playing, stopped, fading out, etc.
        eChannelPlaybackState _channelState;

        // The collection of the sound being played on this channel.
        SwitchContainerImpl* _switchContainer;

        // The collection of the sound being played on this channel.
        CollectionImpl* _collection;

        // The sound source that was chosen from the sound collection.
        SoundImpl* _sound;

        const SwitchImpl* _switch;
        AmObjectID _playingSwitchContainerStateId;
        AmObjectID _previousSwitchContainerStateId;

        // The sound fader of this channel.
        FaderInstance* _fader;
        AmString _faderName;

        // The target state of the fade out transition. Must be either Paused or Stopped.
        eChannelPlaybackState _targetFadeOutState;

        // The entity which is playing the sound of this channel.
        Entity _entity;

        // The listener currently rendering this channel.
        Listener _activeListener;

        // The room in which this channel is playing.
        Room _room;

        // The gain set by the user.
        AmReal32 _userGain;

        // The gain of this channel.
        AmReal32 _gain;

        // The pan of this channel for 2D rendering.
        AmVec2 _pan;

        // The pitch of this channel.
        AmReal32 _pitch;

        // The location of this channel's sound.
        AmVec3 _location;

        AmReal32 _directivity;
        AmReal32 _directivitySharpness;

        AmUInt64 _channelStateId;

        std::map<AmListenerID, AmReal32> _dopplerFactors;
        std::map<AmRoomID, AmReal32> _roomGains;

        std::map<ChannelEvent, ChannelEventListener*> _eventsMap;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_CORE_PLAYBACK_CHANNEL_INTERNAL_STATE_H
