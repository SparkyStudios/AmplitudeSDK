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

#include <cassert>
#include <cmath>

#include <SparkyStudios/Audio/Amplitude/Core/Log.h>
#include <SparkyStudios/Audio/Amplitude/Core/Playback/Channel.h>

#include <SparkyStudios/Audio/Amplitude/Sound/Collection.h>

#include <Core/Engine.h>
#include <Core/Playback/ChannelInternalState.h>
#include <Sound/Sound.h>

#include <Mixer/RealChannel.h>

#include "collection_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    RealChannel::RealChannel()
        : RealChannel(nullptr)
    {}

    RealChannel::RealChannel(ChannelInternalState* parent)
        : _channelId(kAmInvalidObjectId)
        , _channelLayersId()
        , _stream()
        , _loop()
        , _pan()
        , _gain()
        , _pitch(1.0f)
        , _playSpeed(1.0f)
        , _mixer(nullptr)
        , _activeSounds()
        , _parentChannelState(parent)
        , _playedSounds()
    {}

    void RealChannel::Initialize(int i)
    {
        _channelId = i;
        _mixer = &amEngine->GetState()->mixer;
    }

    void RealChannel::MarkAsPlayed(const Sound* sound)
    {
        _playedSounds.push_back(sound->GetId());
    }

    bool RealChannel::AllSoundsHasPlayed() const
    {
        if (_parentChannelState->GetCollection() == nullptr)
            return false;

        bool result = true;
        for (auto&& sound : _parentChannelState->GetCollection()->GetSounds())
        {
            if (auto foundIt = std::ranges::find(_playedSounds, sound); foundIt != _playedSounds.end())
                continue;

            result = false;
            break;
        }
        return result;
    }

    void RealChannel::ClearPlayedSounds()
    {
        _playedSounds.clear();
    }

    ChannelInternalState* RealChannel::GetParentChannelState() const
    {
        return _parentChannelState;
    }

    AmChannelID RealChannel::GetID() const
    {
        return _channelId;
    }

    bool RealChannel::Valid() const
    {
        return _channelId != kAmInvalidObjectId && _mixer != nullptr && _parentChannelState != nullptr;
    }

    bool RealChannel::Play(const std::vector<SoundInstance*>& instances)
    {
        if (instances.empty())
            return false;

        bool success = true;
        AmUInt32 layer = FindFreeLayer(_channelLayersId.empty() ? 1 : _channelLayersId.rbegin()->first);
        std::vector<AmUInt32> layers;

        for (auto& instance : instances)
        {
            success &= Play(instance, layer);
            layers.push_back(layer);

            if (!success)
            {
                for (auto&& l : layers)
                    Destroy(l);

                return false;
            }

            layer = FindFreeLayer(layer);
        }

        return success;
    }

    bool RealChannel::Play(SoundInstance* sound, AmUInt32 layer)
    {
        AMPLITUDE_ASSERT(sound != nullptr);

        _activeSounds[layer] = sound;
        _activeSounds[layer]->SetChannel(this);
        _activeSounds[layer]->Load();

        if (sound->GetUserData() == nullptr)
        {
            _channelLayersId[layer] = kAmInvalidObjectId;
            amLogError("The sound was not loaded successfully.");
            return false;
        }

        _loop[layer] = sound->GetSound()->IsLoop();
        _stream[layer] = sound->GetSound()->IsStream();

        const PlayStateFlag loops = _loop[layer] ? ePSF_LOOP : ePSF_PLAY;

        _channelLayersId[layer] =
            _mixer->Play(static_cast<SoundData*>(sound->GetUserData()), loops, _gain[layer], _pan, _pitch, _playSpeed, _channelId, 0);

        // Check if playing the sound was successful, and display the error if it was not.
        const bool success = _channelLayersId[layer] != kAmInvalidObjectId;
        if (!success)
        {
            _channelLayersId[layer] = kAmInvalidObjectId;
            amLogError("Could not play sound '" AM_OS_CHAR_FMT "'.", _activeSounds[layer]->GetSound()->GetPath().c_str());
        }

        return success;
    }

    void RealChannel::Destroy(AmUInt32 layer)
    {
        AMPLITUDE_ASSERT(Valid() && _channelLayersId[layer] != kAmInvalidObjectId);

        const MixerCommandCallback callback = [&, layer]() -> bool
        {
            _mixer->SetPlayState(_channelId, _channelLayersId[layer], ePSF_MIN);

            _channelLayersId.erase(layer);

            ampooldelete(eMemoryPoolKind_Engine, SoundInstance, _activeSounds[layer]);
            _activeSounds.erase(layer);

            return true;
        };

        if (_mixer->IsInsideThreadMutex())
        {
            _mixer->PushCommand({ callback });
            return;
        }

        AM_UNUSED(callback());
    }

    bool RealChannel::Playing() const
    {
        AMPLITUDE_ASSERT(Valid());

        bool playing = true;
        for (auto&& layer : _channelLayersId)
        {
            if (layer.second == 0)
                continue;

            playing &= Playing(layer.first);
        }

        return playing;
    }

    bool RealChannel::Playing(AmUInt32 layer) const
    {
        AMPLITUDE_ASSERT(Valid());

        const AmUInt32 state = _mixer->GetPlayState(_channelId, _channelLayersId.at(layer));
        if (state < ePSF_PLAY)
            return false;

        if (const auto* collection = _parentChannelState->GetCollection(); collection == nullptr)
        {
            return !_loop.at(layer) && state == ePSF_PLAY || _loop.at(layer) && state == ePSF_LOOP;
        }
        else
        {
            const CollectionPlayMode mode = static_cast<const CollectionImpl*>(collection)->GetDefinition()->play_mode();

            return mode == CollectionPlayMode_PlayOne && !_loop.at(layer) ? state == ePSF_PLAY
                : mode == CollectionPlayMode_PlayOne && _loop.at(layer)   ? state == ePSF_LOOP
                                                                          : _channelId != kAmInvalidObjectId;
        }
    }

    bool RealChannel::Paused() const
    {
        AMPLITUDE_ASSERT(Valid());

        bool paused = true;
        for (auto&& layer : _channelLayersId)
        {
            if (layer.second == 0)
                continue;

            paused &= Paused(layer.first);
        }

        return paused;
    }

    bool RealChannel::Paused(AmUInt32 layer) const
    {
        AMPLITUDE_ASSERT(Valid());
        return _mixer->GetPlayState(_channelId, _channelLayersId.at(layer)) == ePSF_HALT;
    }

    void RealChannel::SetGain(const AmReal32 gain)
    {
        AMPLITUDE_ASSERT(Valid());

        for (auto&& layer : _channelLayersId)
        {
            if (layer.second == 0)
                continue;

            SetGain(gain, layer.first);
        }
    }

    void RealChannel::SetGain(AmReal32 gain, AmUInt32 layer)
    {
        SetGainPan(gain, _pan, layer);
    }

    AmReal32 RealChannel::GetGain(AmUInt32 layer) const
    {
        AMPLITUDE_ASSERT(Valid());
        return _gain.contains(layer) ? _gain.at(layer) : 0.0f;
    }

    bool RealChannel::Halt(AmUInt32 layer)
    {
        AMPLITUDE_ASSERT(Valid());
        return _mixer->SetPlayState(_channelId, _channelLayersId[layer], ePSF_STOP);
    }

    bool RealChannel::Halt()
    {
        AMPLITUDE_ASSERT(Valid());

        bool success = true;
        for (const auto& layer : _channelLayersId | std::views::keys)
            success &= Halt(layer);

        return success;
    }

    bool RealChannel::Pause(AmUInt32 layer)
    {
        AMPLITUDE_ASSERT(Valid());
        return _mixer->SetPlayState(_channelId, _channelLayersId[layer], ePSF_HALT);
    }

    bool RealChannel::Pause()
    {
        AMPLITUDE_ASSERT(Valid());

        bool success = true;
        for (const auto& layer : _channelLayersId | std::views::keys)
            success &= Pause(layer);

        return success;
    }

    bool RealChannel::Resume(AmUInt32 layer)
    {
        AMPLITUDE_ASSERT(Valid());
        return _mixer->SetPlayState(_channelId, _channelLayersId[layer], _loop[layer] ? ePSF_LOOP : ePSF_PLAY);
    }

    bool RealChannel::Resume()
    {
        AMPLITUDE_ASSERT(Valid());

        bool success = true;
        for (const auto& layer : _channelLayersId | std::views::keys)
            success &= Resume(layer);

        return success;
    }

    void RealChannel::SetPan(const AmVec2& pan)
    {
        AMPLITUDE_ASSERT(Valid());

        for (auto&& layer : _channelLayersId)
        {
            if (layer.second == 0)
                continue;

            SetGainPan(_gain[layer.first], pan.X, layer.first);
        }

        _pan = pan.X;
    }

    void RealChannel::SetPitch(AmReal32 pitch)
    {
        AMPLITUDE_ASSERT(Valid());

        for (auto&& layer : _channelLayersId)
        {
            if (layer.second == 0)
                continue;

            AmReal32 finalPitch = pitch;
            if (_activeSounds[layer.first]->GetSettings().m_kind != SoundKind::Standalone)
                finalPitch = pitch * _activeSounds[layer.first]->GetSettings().m_pitch.GetValue();

            _mixer->SetPitch(_channelId, layer.second, finalPitch);
        }

        _pitch = pitch;
    }

    void RealChannel::SetSpeed(AmReal32 speed)
    {
        AMPLITUDE_ASSERT(Valid());

        for (auto&& layer : _channelLayersId)
        {
            if (layer.second == 0)
                continue;

            _mixer->SetPlaySpeed(_channelId, layer.second, speed);
        }

        _playSpeed = speed;
    }

    void RealChannel::SetObstruction(AmReal32 obstruction)
    {
        AMPLITUDE_ASSERT(Valid());

        for (auto&& layer : _channelLayersId)
        {
            if (layer.second == 0)
                continue;

            _mixer->SetObstruction(_channelId, layer.second, obstruction);
        }
    }

    void RealChannel::SetOcclusion(AmReal32 occlusion)
    {
        AMPLITUDE_ASSERT(Valid());

        for (auto&& layer : _channelLayersId)
        {
            if (layer.second == 0)
                continue;

            _mixer->SetOcclusion(_channelId, layer.second, occlusion);
        }
    }

    void RealChannel::SetGainPan(AmReal32 gain, AmReal32 pan, AmUInt32 layer)
    {
        AmReal32 finalGain = gain;
        if (_activeSounds[layer]->GetSettings().m_kind != SoundKind::Standalone)
            finalGain = gain * _activeSounds[layer]->GetSettings().m_gain.GetValue();

        _mixer->SetGainPan(_channelId, _channelLayersId[layer], finalGain, pan);

        _gain[layer] = gain;
        _pan = pan;
    }

    AmUInt32 RealChannel::FindFreeLayer(AmUInt32 layerIndex) const
    {
        while (_channelLayersId.contains(layerIndex))
            layerIndex++;

        return layerIndex;
    }
} // namespace SparkyStudios::Audio::Amplitude
