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

#include <utility>

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/Core/Playback/Channel.h>

#include <Core/Playback/ChannelInternalState.h>

namespace SparkyStudios::Audio::Amplitude
{
    static AmUInt64 globalStateId = 0;
    static AmVec3 globalPosition = { 0.0f, 0.0f, 0.0f };

    Channel::Channel()
        : _state(nullptr)
        , _stateId(0)
    {}

    Channel::Channel(ChannelInternalState* state)
        : _state(state)
        , _stateId(0)
    {
        if (state == nullptr)
            return;

        if (state->GetChannelStateId() > 0)
        {
            _stateId = state->GetChannelStateId();
        }
        else
        {
            _stateId = ++globalStateId;
            state->SetChannelStateId(_stateId);
        }
    }

    void Channel::Clear()
    {
        _state = nullptr;
        _stateId = 0;
    }

    bool Channel::Valid() const
    {
        return _state != nullptr && _stateId != 0;
    }

    AmUInt64 Channel::GetId() const
    {
        return _stateId;
    }

    bool Channel::Playing() const
    {
        AMPLITUDE_ASSERT(Valid());
        if (IsValidStateId())
            return _state->Playing();

        return false;
    }

    void Channel::Stop(AmTime duration) const
    {
        AMPLITUDE_ASSERT(Valid());
        if (!IsValidStateId())
            return;

        if (_state->Stopped())
            return;

        if (duration == 0.0)
            _state->Halt();
        else
            _state->FadeOut(duration, eChannelPlaybackState_Stopped);
    }

    void Channel::Pause(AmTime duration) const
    {
        AMPLITUDE_ASSERT(Valid());
        if (!IsValidStateId())
            return;

        if (_state->Paused())
            return;

        if (duration == 0.0)
            _state->Pause();
        else
            _state->FadeOut(duration, eChannelPlaybackState_Paused);
    }

    void Channel::Resume(AmTime duration) const
    {
        AMPLITUDE_ASSERT(Valid());
        if (!IsValidStateId())
            return;

        if (_state->Playing())
            return;

        if (duration == 0.0)
            _state->Resume();
        else
            _state->FadeIn(duration);
    }

    const AmVec3& Channel::GetLocation() const
    {
        AMPLITUDE_ASSERT(Valid());
        if (IsValidStateId())
            return _state->GetLocation();

        return globalPosition;
    }

    void Channel::SetLocation(const AmVec3& location) const
    {
        AMPLITUDE_ASSERT(Valid());
        if (IsValidStateId())
            _state->SetLocation(location);
    }

    void Channel::SetGain(const AmReal32 gain) const
    {
        AMPLITUDE_ASSERT(Valid());
        if (IsValidStateId())
            _state->SetUserGain(gain);
    }

    AmReal32 Channel::GetGain() const
    {
        AMPLITUDE_ASSERT(Valid());
        if (IsValidStateId())
            return _state->GetUserGain();

        return 0.0f;
    }

    eChannelPlaybackState Channel::GetPlaybackState() const
    {
        AMPLITUDE_ASSERT(Valid());
        return _state->GetChannelState();
    }

    Entity Channel::GetEntity() const
    {
        AMPLITUDE_ASSERT(Valid());
        return _state->GetEntity();
    }

    Listener Channel::GetListener() const
    {
        AMPLITUDE_ASSERT(Valid());
        return _state->GetListener();
    }

    Room Channel::GetRoom() const
    {
        AMPLITUDE_ASSERT(Valid());
        return _state->GetRoom();
    }

    ChannelInternalState* Channel::GetState() const
    {
        return _state;
    }

    void Channel::On(const ChannelEvent event, ChannelEventCallback callback, void* userData) const
    {
        AMPLITUDE_ASSERT(Valid());

        if (!IsValidStateId())
            return;

        _state->On(event, std::move(callback), userData);
    }

    Channel::Channel(ChannelInternalState* state, const AmUInt64 id)
        : _state(state)
        , _stateId(id)
    {
        if (_state != nullptr)
            _state->SetChannelStateId(_stateId);
    }

    bool Channel::IsValidStateId() const
    {
        return _state->GetChannelStateId() == _stateId;
    }
} // namespace SparkyStudios::Audio::Amplitude
