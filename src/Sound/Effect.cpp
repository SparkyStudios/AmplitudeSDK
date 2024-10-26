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

#include <Core/EngineInternalState.h>
#include <Sound/Effect.h>

#include "effect_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    EffectImpl::EffectImpl()
        : _instances()
        , _parameters()
        , _filter(nullptr)
    {}

    EffectImpl::~EffectImpl()
    {
        for (auto&& instance : _instances)
            DestroyInstance(instance);

        _filter = nullptr;

        _parameters.clear();
        _instances.clear();
    }

    EffectInstance* EffectImpl::CreateInstance() const
    {
        auto* effect = ampoolnew(eMemoryPoolKind_Engine, EffectInstanceImpl, this);
        _instances.push_back(effect);
        return effect;
    }

    void EffectImpl::DestroyInstance(EffectInstance* instance) const
    {
        if (instance == nullptr)
            return;

        _instances.erase(std::ranges::find(_instances, instance));

        ampooldelete(eMemoryPoolKind_Engine, EffectInstance, instance);
    }

    void EffectImpl::Update()
    {
        // Update effect parameters
        for (auto&& instance : _instances)
            for (AmSize i = 0, l = _parameters.size(); i < l; ++i)
                instance->GetFilter()->SetParameter(i, _parameters[i].GetValue());
    }

    bool EffectImpl::LoadDefinition(const EffectDefinition* definition, EngineInternalState* state)
    {
        m_id = definition->id();
        m_name = definition->name()->str();

        _filter = Filter::Find(definition->effect()->str());

        if (_filter == nullptr)
        {
            amLogError("Effect asset %s specifies an invalid effect type: %s.", definition->name()->c_str(), definition->effect()->c_str());
            return false;
        }

        const flatbuffers::uoffset_t paramCount = definition->parameters() ? definition->parameters()->size() : 0;
        _parameters.resize(paramCount);

        for (flatbuffers::uoffset_t i = 0; i < paramCount; ++i)
            _parameters[i].Init(definition->parameters()->Get(i));

        return true;
    }

    const EffectDefinition* EffectImpl::GetDefinition() const
    {
        return GetEffectDefinition(m_source.c_str());
    }

    EffectInstanceImpl::EffectInstanceImpl(const EffectImpl* parent)
        : _parent(parent)
    {
        _filterInstance = _parent->_filter->CreateInstance();

        // First initialization of filter parameters
        for (AmSize i = 0, l = _parent->_parameters.size(); i < l; ++i)
            _filterInstance->SetParameter(i, _parent->_parameters[i].GetValue());
    }

    EffectInstanceImpl::~EffectInstanceImpl()
    {
        _parent->_filter->DestroyInstance(_filterInstance);
        _filterInstance = nullptr;

        _parent = nullptr;
    }

    FilterInstance* EffectInstanceImpl::GetFilter() const
    {
        return _filterInstance;
    }
} // namespace SparkyStudios::Audio::Amplitude
