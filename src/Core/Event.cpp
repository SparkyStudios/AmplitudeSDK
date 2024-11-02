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

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Core/Engine.h>
#include <Core/EntityInternalState.h>
#include <Core/Event.h>

namespace SparkyStudios::Audio::Amplitude
{
    EventAction::EventAction()
        : EventAction(nullptr)
    {}

    EventAction::EventAction(EventImpl* parent)
        : _active(false)
        , _type(EventActionType_None)
        , _scope(Scope_Entity)
        , _targets()
        , _accumulatedTime(0.0)
        , _parent(parent)
    {}

    void EventAction::Initialize(const EventActionDefinition* definition)
    {
        _active = definition->active();
        _type = definition->type();
        _scope = definition->scope();
        const flatbuffers::uoffset_t targets_count = definition->targets() ? definition->targets()->size() : 0;
        for (flatbuffers::uoffset_t i = 0; i < targets_count; ++i)
            _targets.push_back(definition->targets()->Get(i));
    }

    void EventAction::Run(const Entity& entity)
    {
        if (!_active)
            return;

        switch (_type)
        {
        default:
            [[fallthrough]];
        case EventActionType_None:
            return;

        case EventActionType_Play:
            return ExecutePlay(entity);

        case EventActionType_Pause:
            return ExecutePause(entity);

        case EventActionType_Resume:
            return ExecuteResume(entity);

        case EventActionType_Stop:
            return ExecuteStop(entity);

        case EventActionType_Seek:
            return ExecuteSeek(entity);

        case EventActionType_MuteBus:
            return ExecuteMute(entity, true);

        case EventActionType_UnmuteBus:
            return ExecuteMute(entity, false);

        case EventActionType_Wait:
            return ExecuteWait(entity);
        }
    }

    void EventAction::Run()
    {
        if (_scope == Scope_Entity)
        {
            AMPLITUDE_ASSERT(false);
            amLogWarning("Running an entity scoped event action without an entity.");
        }

        Run(Entity(nullptr));
    }

    void EventAction::AdvanceFrame(AmTime delta_time)
    {
        if (!_active)
            return;

        switch (_type)
        {
        default:
            [[fallthrough]];
        case EventActionType_None:
            return;

        case EventActionType_Wait:
            _accumulatedTime += delta_time;
        }
    }

    void EventAction::ExecutePlay(const Entity& entity)
    {
        if (entity.Valid())
        {
            for (auto&& target : _targets)
            {
                _playingChannels.push_back(amEngine->Play(target, entity));
            }
        }
        else
        {
            for (auto&& target : _targets)
            {
                _playingChannels.push_back(amEngine->Play(target));
            }
        }
    }

    void EventAction::ExecutePause(const Entity& entity)
    {
        if (_scope == Scope_Entity)
        {
            for (auto&& target : _targets)
            {
                for (auto&& item : entity.GetState()->GetPlayingSoundList())
                {
                    if (target == item.GetPlayingObjectId())
                    {
                        item.Pause();
                    }
                }
            }
        }
        else
        {
            for (auto&& target : _targets)
            {
                for (auto&& item : amEngine->GetState()->playing_channel_list)
                {
                    if (target == item.GetPlayingObjectId())
                    {
                        item.Pause();
                    }
                }
            }
        }
    }

    void EventAction::ExecuteResume(const Entity& entity)
    {
        if (_scope == Scope_Entity)
        {
            for (auto&& target : _targets)
            {
                for (auto&& item : entity.GetState()->GetPlayingSoundList())
                {
                    if (target == item.GetPlayingObjectId())
                    {
                        item.Resume();
                    }
                }
            }
        }
        else
        {
            for (auto&& target : _targets)
            {
                for (auto&& item : amEngine->GetState()->playing_channel_list)
                {
                    if (target == item.GetPlayingObjectId())
                    {
                        item.Resume();
                    }
                }
            }
        }
    }

    void EventAction::ExecuteStop(const Entity& entity)
    {
        if (_scope == Scope_Entity)
        {
            for (auto&& target : _targets)
            {
                for (auto&& item : entity.GetState()->GetPlayingSoundList())
                {
                    if (target == item.GetPlayingObjectId())
                    {
                        item.Halt();
                    }
                }
            }
        }
        else
        {
            for (auto&& target : _targets)
            {
                for (auto&& item : amEngine->GetState()->playing_channel_list)
                {
                    if (target == item.GetPlayingObjectId())
                    {
                        item.Halt();
                    }
                }
            }
        }
    }

    void EventAction::ExecuteSeek(const Entity& entity)
    {
        // noop
    }

    void EventAction::ExecuteMute(const Entity& entity, bool mute)
    {
        for (auto&& target : _targets)
        {
            if (Bus bus = amEngine->FindBus(target); bus.Valid())
            {
                bus.SetMute(mute);
            }
        }
    }

    void EventAction::ExecuteWait(const Entity& entity)
    {
        _accumulatedTime = 0.0;
    }

    bool EventAction::IsExecuting() const
    {
        if (!_active)
            return false;

        switch (_type)
        {
        case EventActionType_None:
            return false;

        case EventActionType_Play:
            {
                bool active = false;
                for (auto&& channel : _playingChannels)
                    active |= channel.GetPlaybackState() == eChannelPlaybackState_Playing;

                return active;
            }

        case EventActionType_Pause:
            {
                bool active = false;
                for (auto&& channel : _playingChannels)
                    active |= channel.GetPlaybackState() == eChannelPlaybackState_Paused;

                return active;
            }

        case EventActionType_Resume:
            {
                bool active = false;
                for (auto&& channel : _playingChannels)
                    active |= channel.GetPlaybackState() == eChannelPlaybackState_Playing;

                return active;
            }

        case EventActionType_Stop:
            {
                bool active = false;
                for (auto&& channel : _playingChannels)
                    active |= channel.GetPlaybackState() == eChannelPlaybackState_Stopped;

                return active;
            }

        case EventActionType_Seek:
            return false;

        case EventActionType_MuteBus:
            {
                bool active = false;
                for (auto&& target : _targets)
                    if (Bus bus = amEngine->FindBus(target); bus.Valid())
                        active |= bus.IsMuted();

                return active;
            }

        case EventActionType_UnmuteBus:
            {
                bool active = false;
                for (auto&& target : _targets)
                    if (Bus bus = amEngine->FindBus(target); bus.Valid())
                        active |= !bus.IsMuted();

                return active;
            }

        case EventActionType_Wait:
            return _accumulatedTime < static_cast<AmTime>(_targets[0]);
        }

        return false;
    }

    EventImpl::EventImpl()
        : _runMode(EventActionRunningMode_Parallel)
        , _actions()
    {}

    EventImpl::~EventImpl()
    {
        _actions.clear();
    }

    EventInstanceImpl EventImpl::Trigger(const Entity& entity) const
    {
        amLogDebug("Event '%s' triggered.", m_name.c_str());

        auto event = EventInstanceImpl(this);
        event.Start(entity);

        return event;
    }

    bool EventImpl::LoadDefinition(const EventDefinition* definition, EngineInternalState* state)
    {
        m_id = definition->id();
        m_name = definition->name()->str();

        _runMode = definition->run_mode();

        const flatbuffers::uoffset_t actions_count = definition->actions() ? definition->actions()->size() : 0;
        _actions.resize(actions_count);

        for (flatbuffers::uoffset_t i = 0; i < actions_count; ++i)
        {
            const EventActionDefinition* item = definition->actions()->Get(i);

            EventAction& action = _actions[i];
            action._parent = this;
            action.Initialize(item);
        }

        return true;
    }

    const EventDefinition* EventImpl::GetDefinition() const
    {
        return GetEventDefinition(m_source.c_str());
    }

    EventCanceler::EventCanceler()
        : EventCanceler(nullptr)
    {}

    EventCanceler::EventCanceler(EventInstance* event)
        : _event(event)
    {}

    EventCanceler::~EventCanceler()
    {
        if (_event == nullptr)
            return;

        _event = nullptr;
    }

    bool EventCanceler::Valid() const
    {
        return _event != nullptr;
    }

    void EventCanceler::Cancel() const
    {
        AMPLITUDE_ASSERT(Valid());
        _event->Abort();
    }

    EventInstance* EventCanceler::GetEvent() const
    {
        return _event;
    }

    EventInstanceImpl::EventInstanceImpl()
        : _runMode(EventActionRunningMode_Parallel)
        , _actions()
        , _running(false)
        , _runningActionIndex(0)
        , _entity(nullptr)
    {}

    EventInstanceImpl::EventInstanceImpl(const EventImpl* parent)
        : _runMode(parent->_runMode)
        , _actions(parent->_actions)
        , _running(false)
        , _runningActionIndex(0)
        , _entity(nullptr)
    {}

    void EventInstanceImpl::AdvanceFrame(AmTime deltaTime)
    {
        if (!_running)
            return;

        if (_runMode == EventActionRunningMode_Parallel)
        {
            _running = false;

            for (auto&& action : _actions)
            {
                if (action.IsExecuting())
                {
                    _running = true;
                    action.AdvanceFrame(deltaTime);
                }
            }
        }
        else if (_runMode == EventActionRunningMode_Sequential)
        {
            if (_runningActionIndex < _actions.size())
            {
                if (!_actions[_runningActionIndex].IsExecuting())
                {
                    ++_runningActionIndex;
                    _actions[_runningActionIndex].Run(_entity);
                }
                else
                {
                    _actions[_runningActionIndex].AdvanceFrame(deltaTime);
                }
            }
            else
            {
                _running = false;
            }
        }
    }

    bool EventInstanceImpl::IsRunning() const
    {
        return _running;
    }

    void EventInstanceImpl::Abort()
    {
        _running = false;
    }

    void EventInstanceImpl::Start(const Entity& entity)
    {
        if (_running)
            return;

        _running = true;
        _entity = entity;

        if (_runMode == EventActionRunningMode_Parallel)
        {
            for (auto&& action : _actions)
                action.Run(_entity);
        }
        else if (_runMode == EventActionRunningMode_Sequential)
        {
            _runningActionIndex = 0;
            _actions[_runningActionIndex].Run(_entity);
        }
    }
} // namespace SparkyStudios::Audio::Amplitude
