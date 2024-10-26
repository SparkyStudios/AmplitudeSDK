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

#include <SparkyStudios/Audio/Amplitude/Core/Log.h>
#include <SparkyStudios/Audio/Amplitude/Sound/SoundBank.h>

#include <Core/Engine.h>

#include "attenuation_definition_generated.h"
#include "collection_definition_generated.h"
#include "effect_definition_generated.h"
#include "event_definition_generated.h"
#include "rtpc_definition_generated.h"
#include "sound_bank_definition_generated.h"
#include "sound_definition_generated.h"
#include "switch_container_definition_generated.h"
#include "switch_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    SoundBank::SoundBank()
        : _refCounter()
        , _soundBankDefSource()
        , _name()
        , _id(kAmInvalidObjectId)
    {}

    SoundBank::SoundBank(const std::string& source)
        : SoundBank()
    {
        _soundBankDefSource = source;

        const SoundBankDefinition* definition = GetSoundBankDefinition();

        _id = definition->id();
        _name = definition->name()->str();
    }

    static bool InitializeSwitchContainer(const AmOsString& filename, const EngineImpl* engine)
    {
        // Find the ID.
        if (SwitchContainerHandle handle = engine->GetSwitchContainerHandleFromFile(filename))
        {
            // We've seen this ID before, update it.
            static_cast<SwitchContainerImpl*>(handle)->GetRefCounter()->Increment();
        }
        else
        {
            const FileSystem* fs = engine->GetFileSystem();
            const AmOsString& filePath = fs->ResolvePath(fs->Join({ AM_OS_STRING("switch_containers"), filename }));

            // This is a new switch container, load it and update it.
            AmUniquePtr<eMemoryPoolKind_Engine, SwitchContainerImpl> switch_container(
                ampoolnew(eMemoryPoolKind_Engine, SwitchContainerImpl));
            if (!switch_container->LoadDefinitionFromPath(filePath, engine->GetState()))
                return false;

            const SwitchContainerDefinition* definition = switch_container->GetDefinition();
            const AmSwitchContainerID id = definition->id();
            if (id == kAmInvalidObjectId)
            {
                amLogError("Cannot load switch container '%s'. Invalid ID.", definition->name()->c_str());
                return false;
            }

            switch_container->AcquireReferences(engine->GetState());
            switch_container->GetRefCounter()->Increment();

            engine->GetState()->switch_container_map[id] = std::move(switch_container);
            engine->GetState()->switch_container_id_map[filename] = id;
        }

        return true;
    }

    static bool InitializeCollection(const AmOsString& filename, const EngineImpl* engine)
    {
        // Find the ID.
        if (CollectionHandle handle = engine->GetCollectionHandleFromFile(filename))
        {
            // We've seen this ID before, update it.
            static_cast<CollectionImpl*>(handle)->GetRefCounter()->Increment();
        }
        else
        {
            const FileSystem* fs = engine->GetFileSystem();
            const AmOsString& filePath = fs->ResolvePath(fs->Join({ AM_OS_STRING("collections"), filename }));

            // This is a new collection, load it and update it.
            AmUniquePtr<eMemoryPoolKind_Engine, CollectionImpl> collection(ampoolnew(eMemoryPoolKind_Engine, CollectionImpl));
            if (!collection->LoadDefinitionFromPath(filePath, engine->GetState()))
                return false;

            const CollectionDefinition* definition = collection->GetDefinition();
            const AmCollectionID id = definition->id();
            if (id == kAmInvalidObjectId)
            {
                amLogError("Cannot load collection '%s'. Invalid ID.", definition->name()->c_str());
                return false;
            }

            collection->AcquireReferences(engine->GetState());
            collection->GetRefCounter()->Increment();

            engine->GetState()->collection_map[id] = std::move(collection);
            engine->GetState()->collection_id_map[filename] = id;
        }

        return true;
    }

    static bool InitializeSound(const AmOsString& filename, const EngineImpl* engine, AmSoundID& outId)
    {
        // Find the ID
        if (SoundHandle handle = engine->GetSoundHandleFromFile(filename))
        {
            // We've seen this id before, update it
            static_cast<SoundImpl*>(handle)->GetRefCounter()->Increment();
        }
        else
        {
            const FileSystem* fs = engine->GetFileSystem();
            const AmOsString& filePath = fs->ResolvePath(fs->Join({ AM_OS_STRING("sounds"), filename }));

            // This is a new sound, load it and update it.
            AmUniquePtr<eMemoryPoolKind_Engine, SoundImpl> sound(ampoolnew(eMemoryPoolKind_Engine, SoundImpl));
            if (!sound->LoadDefinitionFromPath(filePath, engine->GetState()))
                return false;

            const SoundDefinition* definition = sound->GetDefinition();
            const AmSoundID id = definition->id();
            if (id == kAmInvalidObjectId)
            {
                amLogError("Cannot load sound '%s'. Invalid ID.", definition->name()->c_str());
                return false;
            }

            sound->AcquireReferences(engine->GetState());
            sound->GetRefCounter()->Increment();

            engine->GetState()->sound_map[id] = std::move(sound);
            engine->GetState()->sound_id_map[filename] = id;

            outId = id;
        }

        return true;
    }

    static bool InitializeEvent(const AmOsString& filename, const EngineImpl* engine)
    {
        // Find the ID.
        if (EventHandle handle = engine->GetEventHandleFromFile(filename))
        {
            // We've seen this ID before, update it.
            static_cast<EventImpl*>(handle)->GetRefCounter()->Increment();
        }
        else
        {
            const FileSystem* fs = engine->GetFileSystem();
            const AmOsString& filePath = fs->ResolvePath(fs->Join({ AM_OS_STRING("events"), filename }));

            // This is a new event, load it and update it.
            AmUniquePtr<eMemoryPoolKind_Engine, EventImpl> event(ampoolnew(eMemoryPoolKind_Engine, EventImpl));
            if (!event->LoadDefinitionFromPath(filePath, engine->GetState()))
                return false;

            const EventDefinition* definition = event->GetDefinition();
            const AmEventID id = definition->id();
            if (id == kAmInvalidObjectId)
            {
                amLogError("Cannot load event '%s'. Invalid ID.", definition->name()->c_str());
                return false;
            }

            event->AcquireReferences(engine->GetState());
            event->GetRefCounter()->Increment();

            engine->GetState()->event_map[id] = std::move(event);
            engine->GetState()->event_id_map[filename] = id;
        }

        return true;
    }

    static bool InitializeAttenuation(const AmOsString& filename, const EngineImpl* engine)
    {
        // Find the ID.
        if (AttenuationHandle handle = engine->GetAttenuationHandleFromFile(filename))
        {
            // We've seen this ID before, update it.
            static_cast<AttenuationImpl*>(handle)->GetRefCounter()->Increment();
        }
        else
        {
            const FileSystem* fs = engine->GetFileSystem();
            const AmOsString& filePath = fs->ResolvePath(fs->Join({ AM_OS_STRING("attenuators"), filename }));

            // This is a new event, load it and update it.
            AmUniquePtr<eMemoryPoolKind_Engine, AttenuationImpl> attenuation(ampoolnew(eMemoryPoolKind_Engine, AttenuationImpl));
            if (!attenuation->LoadDefinitionFromPath(filePath, engine->GetState()))
                return false;

            const AttenuationDefinition* definition = attenuation->GetDefinition();
            const AmAttenuationID id = definition->id();
            if (id == kAmInvalidObjectId)
            {
                amLogError("Cannot load attenuation '%s'. Invalid ID.", definition->name()->c_str());
                return false;
            }

            attenuation->AcquireReferences(engine->GetState());
            attenuation->GetRefCounter()->Increment();

            engine->GetState()->attenuation_map[id] = std::move(attenuation);
            engine->GetState()->attenuation_id_map[filename] = id;
        }

        return true;
    }

    static bool InitializeSwitch(const AmOsString& filename, const EngineImpl* engine)
    {
        // Find the ID.
        if (SwitchHandle handle = engine->GetSwitchHandleFromFile(filename))
        {
            // We've seen this ID before, update it.
            static_cast<SwitchImpl*>(handle)->GetRefCounter()->Increment();
        }
        else
        {
            const FileSystem* fs = engine->GetFileSystem();
            const AmOsString& filePath = fs->ResolvePath(fs->Join({ AM_OS_STRING("switches"), filename }));

            // This is a new event, load it and update it.
            AmUniquePtr<eMemoryPoolKind_Engine, SwitchImpl> _switch(ampoolnew(eMemoryPoolKind_Engine, SwitchImpl));
            if (!_switch->LoadDefinitionFromPath(filePath, engine->GetState()))
            {
                return false;
            }

            const SwitchDefinition* definition = _switch->GetDefinition();
            const AmSwitchID id = definition->id();
            if (id == kAmInvalidObjectId)
            {
                amLogError("Cannot load switch '%s'. Invalid ID.", definition->name()->c_str());
                return false;
            }

            _switch->AcquireReferences(engine->GetState());
            _switch->GetRefCounter()->Increment();

            engine->GetState()->switch_map[id] = std::move(_switch);
            engine->GetState()->switch_id_map[filename] = id;
        }

        return true;
    }

    static bool InitializeRtpc(const AmOsString& filename, const EngineImpl* engine)
    {
        // Find the ID.
        if (RtpcHandle handle = engine->GetRtpcHandleFromFile(filename))
        {
            // We've seen this ID before, update it.
            static_cast<RtpcImpl*>(handle)->GetRefCounter()->Increment();
        }
        else
        {
            const FileSystem* fs = engine->GetFileSystem();
            const AmOsString& filePath = fs->ResolvePath(fs->Join({ AM_OS_STRING("rtpc"), filename }));

            // This is a new rtpc, load it and update it.
            AmUniquePtr<eMemoryPoolKind_Engine, RtpcImpl> rtpc(ampoolnew(eMemoryPoolKind_Engine, RtpcImpl));
            if (!rtpc->LoadDefinitionFromPath(filePath, engine->GetState()))
            {
                return false;
            }

            const RtpcDefinition* definition = rtpc->GetDefinition();
            const AmRtpcID id = definition->id();
            if (id == kAmInvalidObjectId)
            {
                amLogError("Cannot load RTPC '%s'. Invalid ID.", definition->name()->c_str());
                return false;
            }

            rtpc->AcquireReferences(engine->GetState());
            rtpc->GetRefCounter()->Increment();

            engine->GetState()->rtpc_map[id] = std::move(rtpc);
            engine->GetState()->rtpc_id_map[filename] = id;
        }

        return true;
    }

    static bool InitializeEffect(const AmOsString& filename, const EngineImpl* engine)
    {
        // Find the ID.
        if (EffectHandle handle = engine->GetEffectHandleFromFile(filename))
        {
            // We've seen this ID before, update it.
            static_cast<EffectImpl*>(handle)->GetRefCounter()->Increment();
        }
        else
        {
            const FileSystem* fs = engine->GetFileSystem();
            const AmOsString& filePath = fs->ResolvePath(fs->Join({ AM_OS_STRING("effects"), filename }));

            // This is a new effect, load it and update it.
            AmUniquePtr<eMemoryPoolKind_Engine, EffectImpl> effect(ampoolnew(eMemoryPoolKind_Engine, EffectImpl));
            if (!effect->LoadDefinitionFromPath(filePath, engine->GetState()))
            {
                return false;
            }

            const EffectDefinition* definition = effect->GetDefinition();
            const AmEffectID id = definition->id();
            if (id == kAmInvalidObjectId)
            {
                amLogError("Cannot load effect '%s'. Invalid ID.", definition->name()->c_str());
                return false;
            }

            effect->AcquireReferences(engine->GetState());
            effect->GetRefCounter()->Increment();

            engine->GetState()->effect_map[id] = std::move(effect);
            engine->GetState()->effect_id_map[filename] = id;
        }

        return true;
    }

    bool SoundBank::Initialize(const AmOsString& filename, Engine* engine)
    {
        const FileSystem* fs = engine->GetFileSystem();
        const AmOsString& filePath = fs->ResolvePath(fs->Join({ AM_OS_STRING("soundbanks"), filename }));

        if (!LoadFile(fs->OpenFile(filePath), &_soundBankDefSource))
            return false;

        return InitializeInternal(engine);
    }

    bool SoundBank::InitializeFromMemory(const char* fileData, Engine* engine)
    {
        if (!fileData)
            return false;

        _soundBankDefSource = fileData;

        return InitializeInternal(engine);
    }

    static bool DeinitializeSwitchContainer(const AmOsString& filename, EngineInternalState* state)
    {
        const auto id_iter = state->switch_container_id_map.find(filename);
        if (id_iter == state->switch_container_id_map.end())
            return false;

        const AmSwitchContainerID id = id_iter->second;

        const auto switch_container_iter = state->switch_container_map.find(id);
        if (switch_container_iter == state->switch_container_map.end())
            return false;

        if (switch_container_iter->second->GetRefCounter()->Decrement() == 0)
        {
            switch_container_iter->second->ReleaseReferences(state);
            state->switch_container_map.erase(switch_container_iter);
        }

        return true;
    }

    static bool DeinitializeCollection(const AmOsString& filename, EngineInternalState* state)
    {
        const auto id_iter = state->collection_id_map.find(filename);
        if (id_iter == state->collection_id_map.end())
            return false;

        const AmCollectionID id = id_iter->second;

        const auto collection_iter = state->collection_map.find(id);
        if (collection_iter == state->collection_map.end())
            return false;

        if (collection_iter->second->GetRefCounter()->Decrement() == 0)
        {
            collection_iter->second->ReleaseReferences(state);
            state->collection_map.erase(collection_iter);
        }

        return true;
    }

    static bool DeinitializeSound(const AmOsString& filename, EngineInternalState* state)
    {
        const auto id_iter = state->sound_id_map.find(filename);
        if (id_iter == state->sound_id_map.end())
            return false;

        const AmSoundID id = id_iter->second;

        const auto sound_iter = state->sound_map.find(id);
        if (sound_iter == state->sound_map.end())
            return false;

        if (sound_iter->second->GetRefCounter()->Decrement() == 0)
        {
            sound_iter->second->ReleaseReferences(state);
            state->sound_map.erase(sound_iter);
        }

        return true;
    }

    static bool DeinitializeEvent(const AmOsString& filename, EngineInternalState* state)
    {
        const auto id_iter = state->event_id_map.find(filename);
        if (id_iter == state->event_id_map.end())
            return false;

        const AmEventID id = id_iter->second;

        const auto event_iter = state->event_map.find(id);
        if (event_iter == state->event_map.end())
            return false;

        if (event_iter->second->GetRefCounter()->Decrement() == 0)
        {
            event_iter->second->ReleaseReferences(state);
            state->event_map.erase(event_iter);
        }

        return true;
    }

    static bool DeinitializeAttenuation(const AmOsString& filename, EngineInternalState* state)
    {
        const auto id_iter = state->attenuation_id_map.find(filename);
        if (id_iter == state->attenuation_id_map.end())
            return false;

        const AmAttenuationID id = id_iter->second;

        const auto attenuation_iter = state->attenuation_map.find(id);
        if (attenuation_iter == state->attenuation_map.end())
            return false;

        if (attenuation_iter->second->GetRefCounter()->Decrement() == 0)
        {
            attenuation_iter->second->ReleaseReferences(state);
            state->attenuation_map.erase(attenuation_iter);
        }

        return true;
    }

    static bool DeinitializeSwitch(const AmOsString& filename, EngineInternalState* state)
    {
        const auto id_iter = state->switch_id_map.find(filename);
        if (id_iter == state->switch_id_map.end())
            return false;

        const AmSwitchID id = id_iter->second;

        const auto switch_iter = state->switch_map.find(id);
        if (switch_iter == state->switch_map.end())
            return false;

        if (switch_iter->second->GetRefCounter()->Decrement() == 0)
        {
            switch_iter->second->ReleaseReferences(state);
            state->switch_map.erase(switch_iter);
        }

        return true;
    }

    static bool DeinitializeEffect(const AmOsString& filename, EngineInternalState* state)
    {
        const auto id_iter = state->effect_id_map.find(filename);
        if (id_iter == state->effect_id_map.end())
            return false;

        const AmSwitchID id = id_iter->second;

        const auto effect_iter = state->effect_map.find(id);
        if (effect_iter == state->effect_map.end())
            return false;

        if (effect_iter->second->GetRefCounter()->Decrement() == 0)
        {
            effect_iter->second->ReleaseReferences(state);
            state->effect_map.erase(effect_iter);
        }

        return true;
    }

    static bool DeinitializeRtpc(const AmOsString& filename, EngineInternalState* state)
    {
        const auto id_iter = state->rtpc_id_map.find(filename);
        if (id_iter == state->rtpc_id_map.end())
            return false;

        const AmSwitchID id = id_iter->second;

        const auto rtpc_iter = state->rtpc_map.find(id);
        if (rtpc_iter == state->rtpc_map.end())
            return false;

        if (rtpc_iter->second->GetRefCounter()->Decrement() == 0)
        {
            rtpc_iter->second->ReleaseReferences(state);
            state->rtpc_map.erase(rtpc_iter);
        }

        return true;
    }

    void SoundBank::Deinitialize(Engine* engine)
    {
        auto* engineImpl = static_cast<EngineImpl*>(engine);
        const SoundBankDefinition* definition = GetSoundBankDefinition();

        for (flatbuffers::uoffset_t i = 0; i < definition->switch_containers()->size(); ++i)
        {
            AmString filename = definition->switch_containers()->Get(i)->str();
            if (!DeinitializeSwitchContainer(AM_STRING_TO_OS_STRING(filename), engineImpl->GetState()))
            {
                amLogError("Error while deinitializing switch container %s in sound bank.", filename.c_str());
                AMPLITUDE_ASSERT(false);
            }
        }

        for (flatbuffers::uoffset_t i = 0; i < definition->collections()->size(); ++i)
        {
            AmString filename = definition->collections()->Get(i)->str();
            if (!DeinitializeCollection(AM_STRING_TO_OS_STRING(filename), engineImpl->GetState()))
            {
                amLogError("Error while deinitializing collection %s in sound bank.", filename.c_str());
                AMPLITUDE_ASSERT(false);
            }
        }

        for (flatbuffers::uoffset_t i = 0; i < definition->sounds()->size(); ++i)
        {
            AmString filename = definition->sounds()->Get(i)->str();
            if (!DeinitializeSound(AM_STRING_TO_OS_STRING(filename), engineImpl->GetState()))
            {
                amLogError("Error while deinitializing sound %s in sound bank.", filename.c_str());
                AMPLITUDE_ASSERT(false);
            }
        }

        for (flatbuffers::uoffset_t i = 0; i < definition->events()->size(); ++i)
        {
            AmString filename = definition->events()->Get(i)->str();
            if (!DeinitializeEvent(AM_STRING_TO_OS_STRING(filename), engineImpl->GetState()))
            {
                amLogError("Error while deinitializing event %s in sound bank.", filename.c_str());
                AMPLITUDE_ASSERT(false);
            }
        }

        for (flatbuffers::uoffset_t i = 0; i < definition->attenuators()->size(); ++i)
        {
            AmString filename = definition->attenuators()->Get(i)->str();
            if (!DeinitializeAttenuation(AM_STRING_TO_OS_STRING(filename), engineImpl->GetState()))
            {
                amLogError("Error while deinitializing attenuation %s in sound bank.", filename.c_str());
                AMPLITUDE_ASSERT(false);
            }
        }

        for (flatbuffers::uoffset_t i = 0; i < definition->switches()->size(); ++i)
        {
            AmString filename = definition->switches()->Get(i)->str();
            if (!DeinitializeSwitch(AM_STRING_TO_OS_STRING(filename), engineImpl->GetState()))
            {
                amLogError("Error while deinitializing switch %s in sound bank.", filename.c_str());
                AMPLITUDE_ASSERT(false);
            }
        }

        for (flatbuffers::uoffset_t i = 0; i < definition->effects()->size(); ++i)
        {
            AmString filename = definition->effects()->Get(i)->str();
            if (!DeinitializeEffect(AM_STRING_TO_OS_STRING(filename), engineImpl->GetState()))
            {
                amLogError("Error while deinitializing effect %s in sound bank.", filename.c_str());
                AMPLITUDE_ASSERT(false);
            }
        }

        for (flatbuffers::uoffset_t i = 0; i < definition->rtpc()->size(); ++i)
        {
            AmString filename = definition->rtpc()->Get(i)->str();
            if (!DeinitializeRtpc(AM_STRING_TO_OS_STRING(filename), engineImpl->GetState()))
            {
                amLogError("Error while deinitializing RTPC %s in sound bank.", filename.c_str());
                AMPLITUDE_ASSERT(false);
            }
        }
    }

    AmBankID SoundBank::GetId() const
    {
        return _id;
    }

    const std::string& SoundBank::GetName() const
    {
        return _name;
    }

    const SoundBankDefinition* SoundBank::GetSoundBankDefinition() const
    {
        return Amplitude::GetSoundBankDefinition(_soundBankDefSource.c_str());
    }

    RefCounter* SoundBank::GetRefCounter()
    {
        return &_refCounter;
    }

    void SoundBank::LoadSoundFiles(const Engine* engine)
    {
        const auto* engineImpl = static_cast<const EngineImpl*>(engine);

        while (!_pendingSoundsToLoad.empty())
        {
            const auto id = _pendingSoundsToLoad.front();
            _pendingSoundsToLoad.pop();

            if (!engineImpl->GetState()->sound_map.contains(id))
                continue;

            engineImpl->GetState()->sound_map[id]->Load(engineImpl->GetFileSystem());
        }
    }

    bool SoundBank::InitializeInternal(Engine* engine)
    {
        bool success = true;
        const SoundBankDefinition* definition = GetSoundBankDefinition();

        _id = definition->id();
        _name = definition->name()->str();

        auto* engineImpl = static_cast<EngineImpl*>(engine);

        // Load each Rtpc named in the sound bank.
        for (flatbuffers::uoffset_t i = 0; success && i < definition->rtpc()->size(); ++i)
        {
            AmString filename = definition->rtpc()->Get(i)->str();
            success &= InitializeRtpc(AM_STRING_TO_OS_STRING(filename), engineImpl);
        }

        // Load each effect named in the sound bank.
        for (flatbuffers::uoffset_t i = 0; success && i < definition->effects()->size(); ++i)
        {
            AmString filename = definition->effects()->Get(i)->str();
            success &= InitializeEffect(AM_STRING_TO_OS_STRING(filename), engineImpl);
        }

        // Load each Switch named in the sound bank.
        for (flatbuffers::uoffset_t i = 0; success && i < definition->switches()->size(); ++i)
        {
            AmString switch_filename = definition->switches()->Get(i)->str();
            success &= InitializeSwitch(AM_STRING_TO_OS_STRING(switch_filename), engineImpl);
        }

        // Load each Attenuation named in the sound bank.
        for (flatbuffers::uoffset_t i = 0; success && i < definition->attenuators()->size(); ++i)
        {
            AmString attenuation_filename = definition->attenuators()->Get(i)->str();
            success &= InitializeAttenuation(AM_STRING_TO_OS_STRING(attenuation_filename), engineImpl);
        }

        // Load each Event named in the sound bank.
        for (flatbuffers::uoffset_t i = 0; success && i < definition->events()->size(); ++i)
        {
            AmString event_filename = definition->events()->Get(i)->str();
            success &= InitializeEvent(AM_STRING_TO_OS_STRING(event_filename), engineImpl);
        }

        // Load each Sound named in the sound bank.
        for (flatbuffers::uoffset_t i = 0; success && i < definition->sounds()->size(); ++i)
        {
            AmSoundID id = kAmInvalidObjectId;
            AmString filename = definition->sounds()->Get(i)->str();
            success &= InitializeSound(AM_STRING_TO_OS_STRING(filename), engineImpl, id);
            _pendingSoundsToLoad.push(id);
        }

        // Load each Collection named in the sound bank.
        for (flatbuffers::uoffset_t i = 0; success && i < definition->collections()->size(); ++i)
        {
            AmString sound_filename = definition->collections()->Get(i)->str();
            success &= InitializeCollection(AM_STRING_TO_OS_STRING(sound_filename), engineImpl);
        }

        // Load each SwitchContainer named in the sound bank.
        for (flatbuffers::uoffset_t i = 0; success && i < definition->switch_containers()->size(); ++i)
        {
            AmString filename = definition->switch_containers()->Get(i)->str();
            success &= InitializeSwitchContainer(AM_STRING_TO_OS_STRING(filename), engineImpl);
        }

        return success;
    }
} // namespace SparkyStudios::Audio::Amplitude
