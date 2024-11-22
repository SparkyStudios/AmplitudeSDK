// Copyright (c) 2024-present Sparky Studios. All rights reserved.
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

#ifndef _AM_IMPLEMENTATION_MIXER_PIPELINE_H
#define _AM_IMPLEMENTATION_MIXER_PIPELINE_H

#include <SparkyStudios/Audio/Amplitude/Mixer/Pipeline.h>

#include <Core/Asset.h>
#include <Mixer/Amplimix.h>

#include "pipeline_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    class PipelineInstanceImpl final : public PipelineInstance
    {
        friend class PipelineImpl;

    public:
        PipelineInstanceImpl(const Pipeline* parent, const AmplimixLayerImpl* layer);

        ~PipelineInstanceImpl() override;

        void Execute(const AudioBuffer& inputBuffer, AudioBuffer& outputBuffer) override;

        NodeInstance* GetNode(AmObjectID id) const override;

        void Reset() override;

        void AddNode(AmObjectID id, AmString nodeName, NodeInstance* nodeInstance);

    private:
        std::unordered_map<AmObjectID, std::pair<AmString, NodeInstance*>> _nodeInstances;

        InputNodeInstance* _inputNode;
        OutputNodeInstance* _outputNode;

        const AmplimixLayerImpl* _layer;
        AudioBuffer _inputBuffer;
    };

    class PipelineImpl final
        : public Pipeline
        , public AssetImpl<AmPipelineID, PipelineDefinition>
    {
    public:
        ~PipelineImpl() override;

        PipelineInstance* CreateInstance(const AmplimixLayer* layer) const override;

        void DestroyInstance(PipelineInstance* instance) const override;

        /**
         * @copydoc Asset::GetId
         */
        [[nodiscard]] AM_INLINE AmEventID GetId() const override
        {
            return AssetImpl::GetId();
        }

        /**
         * @copydoc Asset::GetName
         */
        [[nodiscard]] AM_INLINE const AmString& GetName() const override
        {
            return AssetImpl::GetName();
        }

        /**
         * @copydoc AssetImpl::LoadDefinition
         */
        bool LoadDefinition(const PipelineDefinition* definition, EngineInternalState* state) override;

        /**
         * @copydoc AssetImpl::GetDefinition
         */
        [[nodiscard]] const PipelineDefinition* GetDefinition() const override;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_MIXER_PIPELINE_H
