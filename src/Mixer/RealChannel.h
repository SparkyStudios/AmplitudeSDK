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

#ifndef _AM_IMPLEMENTATION_MIXER_REAL_CHANNEL_H
#define _AM_IMPLEMENTATION_MIXER_REAL_CHANNEL_H

#include <vector>

#include <SparkyStudios/Audio/Amplitude/Core/Playback/Channel.h>

#include <SparkyStudios/Audio/Amplitude/Math/HandmadeMath.h>

#include <SparkyStudios/Audio/Amplitude/Sound/Collection.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Fader.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Sound.h>

namespace SparkyStudios::Audio::Amplitude
{
    class SoundInstance;

    struct EngineInternalState;
    class ChannelInternalState;

    class AmplimixImpl;

    /**
     * @brief A RealChannel represents a channel of audio on the mixer.
     *
     * Not all channels are backed by RealChannels. If there are more
     * channels of audio being played simultaneously than the mixer can handle,
     * the lowest priority channels will be virtualized. That is, they will no
     * longer have their audio mixed, but their GetGain value and position (and a few
     * other properties) will continue to be tracked.
     *
     * This class represents the real channel interface to the underlying audio
     * mixer backend being used.
     */
    class RealChannel
    {
        friend class ChannelInternalState;
        friend class AmplimixImpl;

    public:
        RealChannel();
        explicit RealChannel(ChannelInternalState* parent);

        /**
         * @brief Initialize this channel.
         */
        void Initialize(int index);

        /**
         * @brief Play all the sound instances on the real channel.
         */
        bool Play(const std::vector<SoundInstance*>& instances);

        /**
         * @brief Play the audio on the real channel.
         */
        bool Play(SoundInstance* sound, AmUInt32 layer = kAmInvalidObjectId);

        /**
         * @brief Halt the real channel so it may be re-used. However, this virtual channel may still be considered playing.
         */
        bool Halt(AmUInt32 layer);
        bool Halt();

        /**
         * @brief Pause the real channel.
         */
        bool Pause(AmUInt32 layer);
        bool Pause();

        /**
         * @brief Resume the paused real channel.
         */
        bool Resume(AmUInt32 layer);
        bool Resume();

        void Destroy(AmUInt32 layer = kAmInvalidObjectId);

        /**
         * @brief Check if this channel is currently playing on a real channel.
         */
        [[nodiscard]] bool Playing(AmUInt32 layer) const;
        [[nodiscard]] bool Playing() const;

        /**
         * @brief Check if this channel is currently paused on a real channel.
         */
        [[nodiscard]] bool Paused(AmUInt32 layer) const;
        [[nodiscard]] bool Paused() const;

        /**
         * @brief Set the current GetGain of the real channel.
         */
        void SetGain(AmReal32 gain);
        void SetGain(AmReal32 gain, AmUInt32 layer);

        /**
         * @brief Get the current GetGain of the real channel.
         */
        [[nodiscard]] AmReal32 GetGain(AmUInt32 layer = kAmInvalidObjectId) const;

        /**
         * @brief Set the pan for the sound. This should be a unit vector.
         */
        void SetPan(const AmVec2& pan);

        /**
         * @brief Set the pitch of the sound.
         *
         * @param pitch The sound's pitch.
         */
        void SetPitch(AmReal32 pitch);

        /**
         * @brief Set the playback speed of the sound.
         *
         * @param speed The playback speed. Set to 1 for normal speed.
         */
        void SetSpeed(AmReal32 speed);

        /**
         * @brief Return true if this is a valid real channel.
         */
        [[nodiscard]] bool Valid() const;

        /**
         * @brief Marks a sound as played.
         *
         * This method is mainly for internal purposes. You can use this only
         * if you know what you are doing.
         *
         * This method is used on sound collections with play mode set to
         * PlayAll or LoopAll to cache the list of played sounds so the scheduler
         * will be able to skip them on subsequent play requests.
         *
         * The played sounds cache is reset when a stop request is received.
         *
         * @param sound The sound to cache in the played sounds list.
         */
        void MarkAsPlayed(const Sound* sound);

        /**
         * @brief Checks if all sounds of this collection have played.
         *
         * This method is used on sound collections with play mode set to
         * PlayAll or LoopAll to check if all sounds have been played and
         * then decide if to play them again (LoopAll) or not (PlayAll).
         *
         * @return true if all sounds have been played.
         */
        [[nodiscard]] bool AllSoundsHasPlayed() const;

        /**
         * @brief Clears the played sounds cache of this collection.
         *
         * This method is mainly for internal purposes. You can use this only
         * if you know what you are doing.
         *
         * This method is used on sound collections with play mode set to
         * PlayAll or LoopAll to clear the play sounds cache when needed.
         */
        void ClearPlayedSounds();

        /**
         * @brief Gets the parent Channel object which created this RealChannel.
         *
         * @return Channel*
         */
        [[nodiscard]] ChannelInternalState* GetParentChannelState() const;

        /**
         * @brief Gets the ID of this channel.
         *
         * @return AmChannelID
         */
        [[nodiscard]] AmChannelID GetID() const;

        /**
         * @brief Set the obstruction level of sounds played by this RealChannel.
         *
         * @param obstruction The obstruction amount. This is provided by the
         * game engine.
         */
        void SetObstruction(AmReal32 obstruction);

        /**
         * @brief Set the occlusion level of sounds played by this RealChannel.
         *
         * @param occlusion The occlusion amount. This is provided by the
         * game engine.
         */
        void SetOcclusion(AmReal32 occlusion);

    private:
        void SetGainPan(AmReal32 gain, AmReal32 pan, AmUInt32 layer);
        [[nodiscard]] AmUInt32 FindFreeLayer(AmUInt32 layerIndex = 0) const;

        AmChannelID _channelId;
        std::map<AmUInt32, AmUInt32> _channelLayersId;

        std::map<AmUInt32, bool> _stream;
        std::map<AmUInt32, bool> _loop;

        AmReal32 _pan;
        std::map<AmUInt32, AmReal32> _gain;
        AmReal32 _pitch;
        AmReal32 _playSpeed;

        AmplimixImpl* _mixer;
        std::map<AmUInt32, SoundInstance*> _activeSounds;

        ChannelInternalState* _parentChannelState;

        std::vector<AmSoundID> _playedSounds;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_MIXER_REAL_CHANNEL_H
