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

#include <map>

#include <SparkyStudios/Audio/Amplitude/Core/Log.h>
#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>
#include <SparkyStudios/Audio/Amplitude/DSP/Resampler.h>

namespace SparkyStudios::Audio::Amplitude
{
    typedef std::map<std::string, Resampler*> ResamplerRegistry;
    typedef ResamplerRegistry::value_type ResamplerImpl;

    static ResamplerRegistry& resamplerRegistry()
    {
        static ResamplerRegistry r;
        return r;
    }

    static bool& lockResamplers()
    {
        static bool b = false;
        return b;
    }

    static AmUInt32& resamplersCount()
    {
        static AmUInt32 c = 0;
        return c;
    }

    Resampler::Resampler(std::string name)
        : m_name(std::move(name))
    {
        Register(this);
    }

    Resampler::Resampler()
        : m_name()
    {}

    Resampler::~Resampler()
    {
        Unregister(this);
    }

    const std::string& Resampler::GetName() const
    {
        return m_name;
    }

    void Resampler::Register(Resampler* resampler)
    {
        if (lockResamplers() || resampler == nullptr)
            return;

        if (Find(resampler->GetName()) != nullptr)
        {
            amLogWarning("Failed to register resampler '%s' as it is already registered", resampler->GetName().c_str());
            return;
        }

        ResamplerRegistry& resamplers = resamplerRegistry();
        resamplers.insert(ResamplerImpl(resampler->GetName(), resampler));
        resamplersCount()++;
    }

    void Resampler::Unregister(const Resampler* resampler)
    {
        if (lockResamplers() || resampler == nullptr)
            return;

        ResamplerRegistry& resamplers = resamplerRegistry();
        if (const auto& it = resamplers.find(resampler->GetName()); it != resamplers.end())
        {
            resamplers.erase(it);
            resamplersCount()--;
        }
    }

    Resampler* Resampler::Find(const std::string& name)
    {
        ResamplerRegistry& resamplers = resamplerRegistry();
        if (const auto& it = resamplers.find(name); it != resamplers.end())
            return it->second;

        return nullptr;
    }

    ResamplerInstance* Resampler::Construct(const std::string& name)
    {
        Resampler* resampler = Find(name);
        if (resampler == nullptr)
            return nullptr;

        return resampler->CreateInstance();
    }

    void Resampler::Destruct(const std::string& name, ResamplerInstance* instance)
    {
        if (instance == nullptr)
            return;

        Resampler* resampler = Find(name);
        if (resampler == nullptr)
            return;

        resampler->DestroyInstance(instance);
    }

    void Resampler::LockRegistry()
    {
        lockResamplers() = true;
    }

    void Resampler::UnlockRegistry()
    {
        lockResamplers() = false;
    }
} // namespace SparkyStudios::Audio::Amplitude