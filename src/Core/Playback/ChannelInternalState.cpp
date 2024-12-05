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

#ifdef _WIN32
#if !defined(_USE_MATH_DEFINES)
#define _USE_MATH_DEFINES
#endif // !defined(_USE_MATH_DEFINES)
#endif // _WIN32

#include <algorithm>
#include <cmath>
#include <ranges>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Core/Engine.h>
#include <Core/EntityInternalState.h>
#include <Core/Playback/BusInternalState.h>
#include <Core/Playback/ChannelInternalState.h>

#include <Utils/intrusive_list.h>
#include <Utils/Utils.h>

#include "collection_definition_generated.h"
#include "sound_definition_generated.h"
#include "switch_container_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    // Removes this channel state from all lists.
    void ChannelInternalState::Remove()
    {
        free_node.remove();
        priority_node.remove();
        bus_node.remove();
        entity_node.remove();
        room_node.remove();
        listener_node.remove();
    }

    void ChannelInternalState::Reset()
    {
        Fader::Destruct(_faderName, _fader);

        _realChannel._channelLayersId.clear();
        _realChannel._activeSounds.clear();
        _realChannel._playedSounds.clear();
        _realChannel._stream.clear();
        _realChannel._loop.clear();
        _realChannel._gain.clear();

        _dopplerFactors.clear();
        _channelState = eChannelPlaybackState_Stopped;
        _switchContainer = nullptr;
        _collection = nullptr;
        _sound = nullptr;
        _fader = nullptr;
        _faderName = "";
        _targetFadeOutState = eChannelPlaybackState_Stopped;
        _entity = Entity();
        _userGain = 0.0f;
        _gain = 0.0f;
        _location = AM_V3(0, 0, 0);
        _channelStateId = 0;

        for (const auto& sound : _eventsMap | std::views::values)
            ampooldelete(eMemoryPoolKind_Engine, ChannelEventListener, sound);

        _eventsMap.clear();
    }

    void ChannelInternalState::SetSwitchContainer(SwitchContainerImpl* switchContainer)
    {
        if (_switchContainer && _switchContainer->GetBus().Valid())
            bus_node.remove();

        _switchContainer = switchContainer;

        if (_switchContainer && _switchContainer->GetBus().Valid())
            _switchContainer->GetBus().GetState()->GetPlayingSoundList().push_front(*this);
    }

    void ChannelInternalState::SetCollection(CollectionImpl* collection)
    {
        if (_collection && _collection->GetBus().Valid())
            bus_node.remove();

        _collection = collection;

        if (_collection && _collection->GetBus().Valid())
            _collection->GetBus().GetState()->GetPlayingSoundList().push_front(*this);
    }

    void ChannelInternalState::SetSound(SoundImpl* sound)
    {
        if (_sound && _sound->GetBus().Valid())
            bus_node.remove();

        _sound = sound;

        if (_sound && _sound->GetBus().Valid())
            _sound->GetBus().GetState()->GetPlayingSoundList().push_front(*this);
    }

    void ChannelInternalState::SetEntity(const Entity& entity)
    {
        if (entity.GetState() == _entity.GetState())
            return;

        if (_entity.Valid())
            entity_node.remove();

        _entity = entity;

        if (_entity.Valid())
            _entity.GetState()->GetPlayingSoundList().push_front(*this);
    }

    void ChannelInternalState::SetListener(const Listener& listener)
    {
        if (listener.GetState() == _activeListener.GetState())
            return;

        if (_activeListener.Valid())
            listener_node.remove();

        _activeListener = listener;

        if (_activeListener.Valid())
            _activeListener.GetState()->GetPlayingSoundList().push_front(*this);
    }

    void ChannelInternalState::SetRoom(const Room& room)
    {
        if (room.GetState() == _room.GetState())
            return;

        if (_room.Valid())
            room_node.remove();

        _room = room;

        if (_room.Valid())
            _room.GetState()->GetPlayingSoundList().push_front(*this);
    }

    bool ChannelInternalState::Play()
    {
        if (_switchContainer != nullptr)
            return PlaySwitchContainer();

        if (_collection != nullptr)
            return PlayCollection();

        if (_sound != nullptr)
            return PlaySound();

        amLogError("Cannot play a channel. Neither a sound, a collection, nor a switch container was defined.");
        return false;
    }

    bool ChannelInternalState::Playing() const
    {
        return _channelState == eChannelPlaybackState_Playing;
    }

    bool ChannelInternalState::Stopped() const
    {
        return _channelState == eChannelPlaybackState_Stopped;
    }

    bool ChannelInternalState::Paused() const
    {
        return _channelState == eChannelPlaybackState_Paused;
    }

    void ChannelInternalState::Halt()
    {
        if (Stopped())
            return;

        HaltInternal();

        if (_collection == nullptr)
            return;

        if (_entity.Valid())
            _collection->ResetEntityScopeScheduler(_entity);
        else
            _collection->ResetWorldScopeScheduler();
    }

    void ChannelInternalState::Pause()
    {
        if (Paused() || !Valid())
            return;

        if (_realChannel.Pause())
            _channelState = eChannelPlaybackState_Paused;
    }

    void ChannelInternalState::Resume()
    {
        if (Playing() || !Valid())
            return;

        if (_realChannel.Resume())
            _channelState = eChannelPlaybackState_Playing;
    }

    void ChannelInternalState::FadeIn(AmTime duration)
    {
        if (Playing() || !Valid() || _channelState == eChannelPlaybackState_FadingIn)
            return;

        _fader->Set(0.0f, _gain, duration);
        _fader->Start(Engine::GetInstance()->GetTotalTime());

        _realChannel.SetGain(0.0f);

        if (_realChannel.Resume())
            _channelState = eChannelPlaybackState_FadingIn;
        else
            _realChannel.SetGain(_gain);
    }

    void ChannelInternalState::FadeOut(AmTime duration, eChannelPlaybackState targetState)
    {
        if (Stopped() || Paused() || _channelState == eChannelPlaybackState_FadingOut)
            return;

        // If the sound is muted, no need to fade out
        if (_gain == 0.0f)
        {
            if (targetState == eChannelPlaybackState_Stopped)
                return Halt();

            if (targetState == eChannelPlaybackState_Paused)
                return Pause();
        }

        if (!Valid())
            return;

        _realChannel.SetGain(_gain);

        _fader->Set(_gain, 0.0f, duration);
        _fader->Start(Engine::GetInstance()->GetTotalTime());

        _channelState = eChannelPlaybackState_FadingOut;
        _targetFadeOutState = targetState;
    }

    void ChannelInternalState::SetPan(const AmVec2& pan)
    {
        _pan = pan;

        if (!Valid())
            return;

        _realChannel.SetPan(pan);
    }

    void ChannelInternalState::SetGain(const AmReal32 gain)
    {
        if (_channelState == eChannelPlaybackState_FadingOut || _channelState == eChannelPlaybackState_FadingIn ||
            _channelState == eChannelPlaybackState_SwitchingState)
            // Do not update gain when fading...
            return;

        _gain = gain;

        if (!Valid())
            return;

        _realChannel.SetGain(gain);
    }

    void ChannelInternalState::SetPitch(AmReal32 pitch)
    {
        _pitch = pitch;

        if (!Valid())
            return;

        _realChannel.SetPitch(pitch);
    }

    AmReal32 ChannelInternalState::GetPitch() const
    {
        return _pitch;
    }

    void ChannelInternalState::SetDirectivity(AmReal32 directivity, AmReal32 directivitySharpness)
    {
        _directivity = _entity.Valid() ? directivity : 0.0f;
        _directivitySharpness = _entity.Valid() ? directivitySharpness : 1.0f;
    }

    void ChannelInternalState::Devirtualize(ChannelInternalState* other)
    {
        AMPLITUDE_ASSERT(!_realChannel.Valid());
        AMPLITUDE_ASSERT(other->_realChannel.Valid());

        other->_realChannel.Pause();

        // Transfer the real channel id to this channel.
        std::swap(_realChannel._channelId, other->_realChannel._channelId);

        if (Playing())
        {
            // Resume playing the audio.
            if (!_realChannel._channelLayersId.empty())
            {
                Play();
            }
            else
            {
                _realChannel.Resume();
            }
        }
        else if (Paused())
        {
            Resume();
        }
    }

    AmReal32 ChannelInternalState::Priority() const
    {
        if (_switchContainer != nullptr)
            return GetGain() * _switchContainer->GetPriority().GetValue();

        if (_collection != nullptr)
            return GetGain() * _collection->GetPriority().GetValue();

        if (_sound != nullptr)
            return GetGain() * _sound->GetPriority().GetValue();

        AMPLITUDE_ASSERT(false); // Should never fall in this case...
        return 0.0f;
    }

    void ChannelInternalState::AdvanceFrame([[maybe_unused]] AmTime deltaTime)
    {
        // Skip paused and stopped channels
        if (_channelState == eChannelPlaybackState_Paused || _channelState == eChannelPlaybackState_Stopped)
            return;

        // Update Doppler factors
        if (_entity.Valid())
        {
            for (auto&& listener : amEngine->GetState()->listener_list)
            {
                if (listener.GetId() == kAmInvalidObjectId)
                    continue;

                _dopplerFactors[listener.GetId()] = ComputeDopplerFactor(
                    _entity.GetLocation() - listener.GetLocation(), _entity.GetVelocity(), listener.GetVelocity(),
                    amEngine->GetSoundSpeed(), amEngine->GetDopplerFactor());
            }
        }

        // Update Room gains
        if (_room.Valid())
        {
            AmReal32 gain = 0.0f;

            const AmReal32 roomVolume = _room.GetVolume();
            if (roomVolume >= kEpsilon)
            {
                const AmVec3& relativeLocation =
                    GetRelativeDirection(_room.GetLocation(), _room.GetOrientation().GetQuaternion(), GetLocation());
                const AmVec3& closestPoint = _room.GetShape().GetClosestPoint(relativeLocation);

                // Avoid division by zero by shifting the attenuation by 1.0f
                const AmReal32 distance = AM_Len(relativeLocation - closestPoint) + 1.0f;

                gain = 1.0f / (distance * distance);
            }

            _roomGains[_room.GetId()] = _room.GetGain() * gain;
        }

        // Update sounds if playing a switch container
        // TODO: This part should probably be optimized
        if (_switchContainer != nullptr && _channelState != eChannelPlaybackState_FadingIn &&
            _channelState != eChannelPlaybackState_FadingOut)
        {
            const SwitchContainerDefinition* definition = _switchContainer->GetDefinition();
            if (_switch->GetState().m_id != kAmInvalidObjectId && _switch->GetState().m_id != _playingSwitchContainerStateId &&
                definition->update_behavior() == SwitchContainerUpdateBehavior_UpdateOnChange)
            {
                const std::vector<SwitchContainerItem>& previousItems = _switchContainer->GetSoundObjects(_playingSwitchContainerStateId);
                std::vector<SwitchContainerItem> nextItems = _switchContainer->GetSoundObjects(_switch->GetState().m_id);

                for (const auto& item : previousItems)
                {
                    bool shouldSkip = false;

                    for (const auto& next : nextItems)
                    {
                        if (next.m_id != item.m_id)
                            continue;

                        shouldSkip = item.m_continueBetweenStates;
                        break;
                    }

                    if (shouldSkip)
                    {
                        std::erase_if(
                            nextItems,
                            [item](const SwitchContainerItem& nextItem)
                            {
                                return nextItem.m_id == item.m_id;
                            });

                        continue;
                    }

                    FaderInstance* out = _switchContainer->GetFaderOut(item.m_id);
                    out->Set(_gain, 0.0f);
                    out->Start(Engine::GetInstance()->GetTotalTime());
                }

                for (const auto& item : nextItems)
                {
                    FaderInstance* in = _switchContainer->GetFaderIn(item.m_id);
                    in->Set(0.0f, _gain);
                    in->Start(Engine::GetInstance()->GetTotalTime());
                }

                _previousSwitchContainerStateId = _playingSwitchContainerStateId;
                PlaySwitchContainerStateUpdate(previousItems, nextItems);
                _playingSwitchContainerStateId = _switch->GetState().m_id;

                _channelState = eChannelPlaybackState_SwitchingState;
            }

            if (_channelState == eChannelPlaybackState_SwitchingState)
            {
                const std::vector<SwitchContainerItem>& previousItems = _switchContainer->GetSoundObjects(_previousSwitchContainerStateId);
                std::vector<SwitchContainerItem> nextItems = _switchContainer->GetSoundObjects(_playingSwitchContainerStateId);

                bool isAtLeastOneFadeInRunning = false;
                bool isAtLeastOneFadeOutRunning = false;

                for (const auto& item : previousItems)
                {
                    bool shouldSkip = false;

                    for (const auto& next : nextItems)
                    {
                        if (next.m_id != item.m_id)
                            continue;

                        shouldSkip = item.m_continueBetweenStates;
                        break;
                    }

                    if (shouldSkip)
                    {
                        std::erase_if(
                            nextItems,
                            [item](const SwitchContainerItem& nextItem)
                            {
                                return nextItem.m_id == item.m_id;
                            });

                        continue;
                    }

                    AmUInt32 layer = 0;
                    for (auto&& _activeSound : _realChannel._activeSounds)
                    {
                        if (_activeSound.second->GetSettings().m_id != item.m_id)
                            continue;

                        layer = _activeSound.first;
                        break;
                    }

                    if (layer == 0)
                        continue;

                    FaderInstance* out = _switchContainer->GetFaderOut(item.m_id);
                    if (out->GetState() == eFaderState_Stopped)
                        continue;

                    const AmReal32 gain = out->GetFromTime(Engine::GetInstance()->GetTotalTime());
                    isAtLeastOneFadeOutRunning = true;

                    if (IsReal())
                        _realChannel.SetGain(gain, layer);

                    if (gain == 0.0f)
                    {
                        out->SetState(eFaderState_Stopped);
                        // Fading in transition complete. Now we can halt the channel layer
                        _realChannel.Halt(layer);
                    }
                }

                for (const auto& item : nextItems)
                {
                    AmUInt32 layer = 0;
                    for (const auto& _activeSound : std::ranges::reverse_view(_realChannel._activeSounds))
                    {
                        if (_activeSound.second->GetSettings().m_id == item.m_id)
                        {
                            layer = _activeSound.first;
                            break;
                        }
                    }

                    if (layer == 0)
                        continue;

                    FaderInstance* in = _switchContainer->GetFaderIn(item.m_id);
                    if (in->GetState() == eFaderState_Stopped)
                        continue;

                    const AmReal32 gain = in->GetFromTime(Engine::GetInstance()->GetTotalTime());
                    isAtLeastOneFadeInRunning = true;

                    if (IsReal())
                        _realChannel.SetGain(gain, layer);

                    if (_gain - gain <= kEpsilon)
                        in->SetState(eFaderState_Stopped);
                }

                if (!isAtLeastOneFadeInRunning && !isAtLeastOneFadeOutRunning)
                {
                    _channelState = eChannelPlaybackState_Playing;
                    _previousSwitchContainerStateId = _playingSwitchContainerStateId;
                }
            }
        }

        // Update the fading in animation if necessary
        if (_channelState == eChannelPlaybackState_FadingIn)
        {
            if (_fader != nullptr && _fader->GetState() == eFaderState_Active)
            {
                const AmReal32 gain = _fader->GetFromTime(Engine::GetInstance()->GetTotalTime());

                if (IsReal())
                    _realChannel.SetGain(gain);

                if (_gain - gain <= kEpsilon)
                {
                    _fader->SetState(eFaderState_Stopped);

                    // Fading in transition complete. Now we mark the channel as playing.
                    _channelState = eChannelPlaybackState_Playing;
                    _gain = gain;
                }
            }
            else
            {
                // No fader is defined, no fading occurs
                if (IsReal())
                    _realChannel.SetGain(_gain);

                _channelState = eChannelPlaybackState_Playing;
            }
        }

        // Update the fading out animation if necessary
        if (_channelState == eChannelPlaybackState_FadingOut)
        {
            if (_fader != nullptr && _fader->GetState() == eFaderState_Active)
            {
                _gain = _fader->GetFromTime(Engine::GetInstance()->GetTotalTime());

                if (IsReal())
                    _realChannel.SetGain(_gain);

                if (_gain == 0.0f)
                {
                    _fader->SetState(eFaderState_Stopped);

                    // Fading out transition complete. Now we can halt or pause the channel.
                    if (_targetFadeOutState == eChannelPlaybackState_Stopped)
                        Halt();
                    else if (_targetFadeOutState == eChannelPlaybackState_Paused)
                        Pause();
                }
            }
            else
            {
                // No fader is defined, no fading occurs
                if (IsReal())
                    _realChannel.SetGain(0.0f);

                if (_targetFadeOutState == eChannelPlaybackState_Stopped)
                    Halt();
                else if (_targetFadeOutState == eChannelPlaybackState_Paused)
                    Pause();
            }
        }
    }

    AmObjectID ChannelInternalState::GetPlayingObjectId() const
    {
        if (_switchContainer)
            return _switchContainer->GetId();
        if (_collection)
            return _collection->GetId();
        if (_sound)
            return _sound->GetId();

        return kAmInvalidObjectId;
    }

    void ChannelInternalState::SetObstruction(AmReal32 obstruction)
    {
        if (!Valid())
            return;

        _realChannel.SetObstruction(obstruction);
    }

    void ChannelInternalState::SetOcclusion(AmReal32 occlusion)
    {
        if (!Valid())
            return;

        _realChannel.SetOcclusion(occlusion);
    }

    AmReal32 ChannelInternalState::GetDopplerFactor(AmListenerID listener) const
    {
        return _dopplerFactors.contains(listener) ? _dopplerFactors.at(listener) : 1.0f;
    }

    AmReal32 ChannelInternalState::GetRoomGain(AmRoomID room) const
    {
        return _roomGains.contains(room) ? _roomGains.at(room) : 0.0f;
    }

    void ChannelInternalState::On(const ChannelEvent event, ChannelEventCallback callback, void* userData)
    {
        if (!Valid())
            return;

        if (_eventsMap[event] == nullptr)
            _eventsMap[event] = ampoolnew(eMemoryPoolKind_Engine, ChannelEventListener);

        _eventsMap[event]->Add(callback, userData);
    }

    void ChannelInternalState::Trigger(ChannelEvent event)
    {
        if (!Valid())
            return;

        if (_eventsMap[event] == nullptr)
            _eventsMap[event] = ampoolnew(eMemoryPoolKind_Engine, ChannelEventListener);

        _eventsMap[event]->Call(this);
    }

    void ChannelInternalState::UpdateState()
    {
        switch (_channelState)
        {
        case eChannelPlaybackState_SwitchingState:
        case eChannelPlaybackState_Paused:
        case eChannelPlaybackState_Stopped:
            break;
        case eChannelPlaybackState_FadingIn:
        case eChannelPlaybackState_Playing:
        case eChannelPlaybackState_FadingOut:
            if (!Valid() || !_realChannel.Playing())
            {
                _channelState = eChannelPlaybackState_Stopped;
            }
            break;
        default:
            AMPLITUDE_ASSERT(false);
        }
    }

    void ChannelInternalState::HaltInternal()
    {
        if (!Valid())
            return;

        if (_realChannel.Halt())
            _channelState = eChannelPlaybackState_Stopped;
    }

    bool ChannelInternalState::PlaySwitchContainerStateUpdate(
        const std::vector<SwitchContainerItem>& previous, const std::vector<SwitchContainerItem>& next)
    {
        const SwitchContainerDefinition* definition = _switchContainer->GetDefinition();

        std::vector<SoundInstance*> instances;
        for (const auto& item : next)
        {
            bool shouldSkip = false;
            for (const auto& prev : previous)
            {
                if (prev.m_id == item.m_id)
                {
                    shouldSkip = item.m_continueBetweenStates;
                }
            }

            if (shouldSkip)
                continue;

            SoundImpl* sound;

            if (Collection* collection = amEngine->GetCollectionHandle(item.m_id); collection != nullptr)
            {
                sound = _entity.Valid() ? static_cast<SoundImpl*>(collection->SelectFromEntity(_entity, _realChannel._playedSounds))
                                        : static_cast<SoundImpl*>(collection->SelectFromWorld(_realChannel._playedSounds));
            }
            else
            {
                sound = static_cast<SoundImpl*>(amEngine->GetSoundHandle(item.m_id));
            }

            if (!sound)
            {
                amLogError("Unable to find a sound object with id: " AM_ID_CHAR_FMT, item.m_id);
                return false;
            }

            SoundInstanceSettings settings;
            settings.m_id = item.m_id;
            settings.m_kind = SoundKind::Switched;
            settings.m_busID = definition->bus();
            settings.m_attenuationID = definition->attenuation();
            settings.m_spatialization = definition->spatialization();
            settings.m_priority = _switchContainer->GetPriority();
            settings.m_gain = item.m_gain;
            settings.m_nearFieldGain = sound->GetNearFieldGain();
            settings.m_pitch = item.m_pitch;
            settings.m_loop = sound->IsLoop();
            settings.m_loopCount = sound->GetDefinition()->loop()->loop_count();
            settings.m_effectID = definition->effect();

            instances.push_back(ampoolnew(
                eMemoryPoolKind_Amplimix, SoundInstance, sound, settings, static_cast<const EffectImpl*>(_switchContainer->GetEffect())));
        }

        const bool success = _realChannel.Play(instances);
        if (!success)
            for (SoundInstance* instance : instances)
                SoundImpl::DestroyInstance(instance);

        return success;
    }

    bool ChannelInternalState::PlaySwitchContainer()
    {
        AMPLITUDE_ASSERT(_switchContainer != nullptr);

        const SwitchContainerDefinition* definition = _switchContainer->GetDefinition();

        _switch = static_cast<const SwitchImpl*>(_switchContainer->GetSwitch());

        Fader::Destruct(_faderName, _fader);

        _faderName = definition->fader()->str();
        _fader = Fader::Construct(_faderName);

        _channelState = eChannelPlaybackState_Playing;

        if (IsReal())
        {
            const auto& [stateId, stateName] = _switch->GetState();
            _playingSwitchContainerStateId = stateId != kAmInvalidObjectId ? stateId : definition->default_switch_state();
            const std::vector<SwitchContainerItem>& items = _switchContainer->GetSoundObjects(_playingSwitchContainerStateId);

            return PlaySwitchContainerStateUpdate({}, items);
        }

        return true;
    }

    bool ChannelInternalState::PlayCollection()
    {
        AMPLITUDE_ASSERT(_collection != nullptr);

        const CollectionDefinition* definition = _collection->GetDefinition();

        SoundImpl* sound = _entity.Valid() ? static_cast<SoundImpl*>(_collection->SelectFromEntity(_entity, _realChannel._playedSounds))
                                           : static_cast<SoundImpl*>(_collection->SelectFromWorld(_realChannel._playedSounds));

        Fader::Destruct(_faderName, _fader);

        _faderName = definition->fader()->str();
        _fader = Fader::Construct(_faderName);

        if (_channelState != eChannelPlaybackState_FadingIn && _channelState != eChannelPlaybackState_FadingOut)
            _channelState = eChannelPlaybackState_Playing;

        SoundInstance* instance = sound->CreateInstance();

        const bool success = !IsReal() || _realChannel.Play(instance);
        if (!success)
            SoundImpl::DestroyInstance(instance);

        return success;
    }

    bool ChannelInternalState::PlaySound()
    {
        AMPLITUDE_ASSERT(_sound != nullptr);

        const SoundDefinition* definition = _sound->GetDefinition();

        Fader::Destruct(_faderName, _fader);

        _faderName = definition->fader()->str();
        _fader = Fader::Construct(_faderName);

        _channelState = eChannelPlaybackState_Playing;

        SoundInstance* instance = _sound->CreateInstance();

        const bool success = !IsReal() || _realChannel.Play(instance);
        if (!success)
            SoundImpl::DestroyInstance(instance);

        return success;
    }
} // namespace SparkyStudios::Audio::Amplitude
