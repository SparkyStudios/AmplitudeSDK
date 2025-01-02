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

#include <ranges>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Core/EngineInternalState.h>
#include <Sound/SwitchContainer.h>

namespace SparkyStudios::Audio::Amplitude
{
    SwitchContainerImpl::SwitchContainerImpl()
        : _switch(nullptr)
        , _sounds()
    {}

    SwitchContainerImpl::~SwitchContainerImpl()
    {
        _switch = nullptr;

        for (const auto& instance : _fadersIn | std::views::values)
            std::get<0>(instance)->DestroyInstance(std::get<1>(instance));

        for (const auto& instance : _fadersOut | std::views::values)
            std::get<0>(instance)->DestroyInstance(std::get<1>(instance));

        _sounds.clear();
        _fadersIn.clear();
        _fadersOut.clear();

        m_bus = nullptr;
        m_effect = nullptr;
        m_attenuation = nullptr;

        m_id = kAmInvalidObjectId;
        m_name.clear();

        m_spatialization = eSpatialization_None;
        m_scope = eScope_World;
    }

    const Switch* SwitchContainerImpl::GetSwitch() const
    {
        return _switch;
    }

    FaderInstance* SwitchContainerImpl::GetFaderIn(AmObjectID id) const
    {
        if (_fadersIn.contains(id))
            return std::get<1>(_fadersIn.at(id));

        return nullptr;
    }

    FaderInstance* SwitchContainerImpl::GetFaderOut(AmObjectID id) const
    {
        if (_fadersOut.contains(id))
            return std::get<1>(_fadersOut.at(id));

        return nullptr;
    }

    const std::vector<SwitchContainerItem>& SwitchContainerImpl::GetSoundObjects(AmObjectID stateId) const
    {
        return _sounds.at(stateId);
    }

    bool SwitchContainerImpl::LoadDefinition(const SwitchContainerDefinition* definition, EngineInternalState* state)
    {
        if (definition->id() == kAmInvalidObjectId)
        {
            amLogError("Invalid ID for switch container.");
            return false;
        }

        const uint64_t busID = definition->bus();
        if (busID == kAmInvalidObjectId)
        {
            amLogError("SwitchContainer %s does not specify a bus.", definition->name()->c_str());
            return false;
        }

        const uint64_t switchGroupID = definition->switch_group();
        if (switchGroupID == kAmInvalidObjectId)
        {
            amLogError("SwitchContainer %s does not specify a switch.", definition->name()->c_str());
            return false;
        }

        m_bus = FindBusInternalState(state, busID);
        if (!m_bus)
        {
            amLogError(
                "SwitchContainer %s specifies an unknown bus ID: " AM_ID_CHAR_FMT ".", definition->name()->c_str(), definition->bus());
            return false;
        }

        if (const auto findIt = state->switch_map.find(switchGroupID); findIt != state->switch_map.end())
        {
            _switch = findIt->second.get();
        }

        const uint64_t effectID = definition->effect();
        if (effectID != kAmInvalidObjectId)
        {
            if (const auto findIt = state->effect_map.find(effectID); findIt != state->effect_map.end())
            {
                m_effect = findIt->second.get();
            }
            else
            {
                amLogError("Sound definition is invalid: invalid effect ID '" AM_ID_CHAR_FMT "'", definition->effect());
                return false;
            }
        }

        const uint64_t attenuationID = definition->attenuation();
        if (attenuationID != kAmInvalidObjectId)
        {
            if (const auto findIt = state->attenuation_map.find(attenuationID); findIt != state->attenuation_map.end())
            {
                m_attenuation = findIt->second.get();
            }

            if (!m_attenuation)
            {
                amLogError(
                    "SwitchContainer %s specifies an unknown attenuation ID: " AM_ID_CHAR_FMT ".", definition->name()->c_str(),
                    definition->attenuation());
                return false;
            }
        }

        m_id = definition->id();
        m_name = definition->name()->str();

        RtpcValue::Init(m_gain, definition->gain(), 1);
        RtpcValue::Init(m_pitch, definition->pitch(), 1);
        RtpcValue::Init(m_priority, definition->priority(), 1);

        m_spatialization = static_cast<eSpatialization>(definition->spatialization());
        m_scope = static_cast<eScope>(definition->scope());

        for (const auto& states = _switch->GetSwitchStates(); const auto& switchState : states)
        {
            if (!_sounds.contains(switchState.m_id))
            {
                _sounds.insert({ switchState.m_id, std::vector<SwitchContainerItem>() });
            }
        }

        const flatbuffers::uoffset_t count = definition->entries() ? definition->entries()->size() : 0;
        for (flatbuffers::uoffset_t i = 0; i < count; ++i)
        {
            const SwitchContainerEntry* entry = definition->entries()->Get(i);
            AmObjectID id = entry->object();

            if (id == kAmInvalidObjectId)
            {
                amLogError("SwitchContainer %s specifies an invalid sound object ID: " AM_ID_CHAR_FMT ".", definition->name()->c_str(), id);
                return false;
            }

            if (!state->sound_map.contains(id) && !state->collection_map.contains(id))
            {
                amLogError(
                    "SwitchContainer %s specifies an unknown sound object ID: " AM_ID_CHAR_FMT ". It's neither a Sound nor a Collection.",
                    definition->name()->c_str(), id);
                return false;
            }

            // Setup entry Faders
            Fader* fader = Fader::Find(entry->fade_in()->fader()->str());
            FaderInstance* faderInstance = fader->CreateInstance();
            faderInstance->SetDuration(entry->fade_in()->duration());

            _fadersIn[id] = std::make_tuple(fader, faderInstance);

            fader = Fader::Find(entry->fade_out()->fader()->str());
            faderInstance = fader->CreateInstance();
            faderInstance->SetDuration(entry->fade_out()->duration());

            _fadersOut[id] = std::make_tuple(fader, faderInstance);

            SwitchContainerItem item;
            item.m_id = id;
            item.m_continueBetweenStates = entry->continue_between_states();
            item.m_fadeInDuration = entry->fade_in()->duration();
            item.m_fadeOutDuration = entry->fade_out()->duration();
            item.m_fadeInAlgorithm = entry->fade_in()->fader()->str();
            item.m_fadeOutAlgorithm = entry->fade_out()->fader()->str();
            RtpcValue::Init(item.m_gain, entry->gain(), 1);
            RtpcValue::Init(item.m_pitch, entry->pitch(), 1);

            const flatbuffers::uoffset_t statesCount = entry->switch_states()->size();
            for (flatbuffers::uoffset_t j = 0; j < statesCount; ++j)
            {
                AmObjectID stateId = entry->switch_states()->Get(j);
                _sounds[stateId].push_back(item);
            }
        }

        return true;
    }

    const SwitchContainerDefinition* SwitchContainerImpl::GetDefinition() const
    {
        return GetSwitchContainerDefinition(m_source.c_str());
    }

    void SwitchContainerImpl::AcquireReferences(EngineInternalState* state)
    {
        AMPLITUDE_ASSERT(m_id != kAmInvalidObjectId);

        _switch->GetRefCounter()->Increment();

        if (m_effect)
            m_effect->GetRefCounter()->Increment();

        if (m_attenuation)
            m_attenuation->GetRefCounter()->Increment();

        for (auto&& id : _sounds | std::ranges::views::keys)
        {
            if (auto findIt = state->sound_map.find(id); findIt != state->sound_map.end())
            {
                findIt->second->GetRefCounter()->Increment();
                continue;
            }

            if (auto findIt = state->collection_map.find(id); findIt != state->collection_map.end())
            {
                findIt->second->GetRefCounter()->Increment();
                continue;
            }
        }
    }

    void SwitchContainerImpl::ReleaseReferences(EngineInternalState* state)
    {
        AMPLITUDE_ASSERT(m_id != kAmInvalidObjectId);

        _switch->GetRefCounter()->Decrement();

        if (m_effect)
            m_effect->GetRefCounter()->Decrement();

        if (m_attenuation)
            m_attenuation->GetRefCounter()->Decrement();

        for (auto&& id : _sounds | std::ranges::views::keys)
        {
            if (auto findIt = state->sound_map.find(id); findIt != state->sound_map.end())
            {
                findIt->second->GetRefCounter()->Decrement();
                continue;
            }

            if (auto findIt = state->collection_map.find(id); findIt != state->collection_map.end())
            {
                findIt->second->GetRefCounter()->Decrement();
                continue;
            }
        }
    }
} // namespace SparkyStudios::Audio::Amplitude
