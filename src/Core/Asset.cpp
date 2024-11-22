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

#include <SparkyStudios/Audio/Amplitude/Core/Asset.h>

#include <Core/Asset.h>
#include <Core/EngineInternalState.h>

namespace SparkyStudios::Audio::Amplitude
{
    template<typename Id, typename Definition>
    AssetImpl<Id, Definition>::~AssetImpl()
    {
        AMPLITUDE_ASSERT(m_refCounter.GetCount() == 0);

        m_id = kAmInvalidObjectId;
        m_source.clear();
    }

    template<typename Id, typename Definition>
    bool AssetImpl<Id, Definition>::LoadDefinitionFromFile(std::shared_ptr<File> file, EngineInternalState* state)
    {
        // Ensure we do not load the asset more than once
        AMPLITUDE_ASSERT(m_id == kAmInvalidObjectId);

        AmString source;
        if (!LoadFile(file, &source))
            return false;

        m_source = source;

        return LoadDefinition(GetDefinition(), state);
    }

    template<typename Id, typename Definition>
    bool AssetImpl<Id, Definition>::LoadDefinitionFromPath(const AmOsString& path, EngineInternalState* state)
    {
        const FileSystem* fs = Engine::GetInstance()->GetFileSystem();
        const AmOsString& rp = fs->ResolvePath(path);

        return LoadDefinitionFromFile(fs->OpenFile(rp), state);
    }

    template<typename Id, typename Definition>
    void AssetImpl<Id, Definition>::AcquireReferences(EngineInternalState* state)
    {}

    template<typename Id, typename Definition>
    void AssetImpl<Id, Definition>::ReleaseReferences(EngineInternalState* state)
    {}

    template class AssetImpl<AmAttenuationID, AttenuationDefinition>;
    template class AssetImpl<AmCollectionID, CollectionDefinition>;
    template class AssetImpl<AmEffectID, EffectDefinition>;
    template class AssetImpl<AmEventID, EventDefinition>;
    template class AssetImpl<AmPipelineID, PipelineDefinition>;
    template class AssetImpl<AmRtpcID, RtpcDefinition>;
    template class AssetImpl<AmSoundID, SoundDefinition>;
    template class AssetImpl<AmSwitchContainerID, SwitchContainerDefinition>;
    template class AssetImpl<AmSwitchID, SwitchDefinition>;
} // namespace SparkyStudios::Audio::Amplitude
