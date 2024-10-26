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
#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>

#include <Mixer/Pipeline.h>

namespace SparkyStudios::Audio::Amplitude
{
    PipelineInstanceImpl::PipelineInstanceImpl(const Pipeline* parent, const AmplimixLayerImpl* layer)
        : _nodeInstances()
        , _layer(layer)
        , _inputNode(nullptr)
        , _outputNode(nullptr)
    {}

    PipelineInstanceImpl::~PipelineInstanceImpl()
    {
        for (const auto& node : _nodeInstances)
            ampooldelete(eMemoryPoolKind_Amplimix, NodeInstance, node.second);

        _inputNode = nullptr;
        _outputNode = nullptr;
    }

    void PipelineInstanceImpl::Execute(const AudioBuffer& in, AudioBuffer& out)
    {
        // Copy the input buffer content
        _inputBuffer = in;

        // Set the input and output buffers for the pipeline
        _inputNode->SetInput(&_inputBuffer);
        _outputNode->SetOutput(&out);

        // Consume data from the output node.
        // This will propagate the data from the input node to the output node,
        // executing all nodes in between.
        _outputNode->Consume();
    }

    NodeInstance* PipelineInstanceImpl::GetNode(AmObjectID id) const
    {
        if (_nodeInstances.find(id) != _nodeInstances.end())
            return _nodeInstances.at(id);

        return nullptr;
    }

    void PipelineInstanceImpl::Reset()
    {
        for (const auto& node : _nodeInstances)
            node.second->Reset();

        _inputNode->Reset();
        _outputNode->Reset();
    }

    void PipelineInstanceImpl::AddNode(AmObjectID id, NodeInstance* nodeInstance)
    {
        if (_nodeInstances.find(id) != _nodeInstances.end())
            return;

        _nodeInstances[id] = nodeInstance;
    }

    PipelineImpl::~PipelineImpl()
    {}

    PipelineInstance* PipelineImpl::CreateInstance(const AmplimixLayer* layer) const
    {
        auto* instance = ampoolnew(eMemoryPoolKind_Amplimix, PipelineInstanceImpl, this, static_cast<const AmplimixLayerImpl*>(layer));

        const auto* definition = GetDefinition();
        const auto* nodes = definition->nodes();

        // Create node instances based on the pipeline definition
        for (flatbuffers::uoffset_t i = 0, l = nodes->size(); i < l; ++i)
        {
            const auto* nodeDef = nodes->Get(i);
            const auto& nodeName = nodeDef->name()->str();
            const auto& nodeId = nodeDef->id();
            const auto* inputs = nodeDef->consume();

            NodeInstance* node = nullptr;
            if (nodeName.compare("Input") == 0)
            {
                node = ampoolnew(eMemoryPoolKind_Amplimix, InputNodeInstance);
                instance->_inputNode = static_cast<InputNodeInstance*>(node);
            }
            else if (nodeName.compare("Output") == 0)
            {
                node = ampoolnew(eMemoryPoolKind_Amplimix, OutputNodeInstance);
                instance->_outputNode = static_cast<OutputNodeInstance*>(node);
            }
            else
            {
                node = Node::Construct(nodeName);
            }

            if (node == nullptr)
            {
                amLogError(
                    "Pipeline node not found: %s. Make sure it is registered. If the node is provided by a plugin, make sure to load the "
                    "plugin before Amplitude.",
                    nodeName.c_str());
                DestroyInstance(instance);
                return nullptr;
            }

            // Initialize the node with the provided parameters
            node->Initialize(nodeId, layer, instance);

            instance->AddNode(nodeId, node);

            // Connect the node inputs
            auto* consumerNode = dynamic_cast<ConsumerNodeInstance*>(node);
            if (consumerNode == nullptr)
                continue;

            for (flatbuffers::uoffset_t j = 0, m = inputs->size(); j < m; ++j)
            {
                if (inputs->Get(j) == nodeId)
                {
                    amLogError("A node cannot consume itself: %s", nodeName.c_str());
                    continue;
                }

                consumerNode->Connect(inputs->Get(j));
            }
        }

        if (instance->_inputNode == nullptr || instance->_outputNode == nullptr)
        {
            amLogError("The pipeline must have an input and an output node.");
            DestroyInstance(instance);
            return nullptr;
        }

        return instance;
    }

    void PipelineImpl::DestroyInstance(PipelineInstance* instance) const
    {
        ampooldelete(eMemoryPoolKind_Amplimix, PipelineInstanceImpl, (PipelineInstanceImpl*)instance);
    }

    bool PipelineImpl::LoadDefinition(const PipelineDefinition* definition, EngineInternalState* state)
    {
        m_id = definition->id();
        m_name = definition->name()->str();

        return true;
    }

    const PipelineDefinition* PipelineImpl::GetDefinition() const
    {
        return GetPipelineDefinition(m_source.c_str());
    }
} // namespace SparkyStudios::Audio::Amplitude
