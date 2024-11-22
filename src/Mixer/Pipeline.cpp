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
            Node::Destruct(node.second.first, node.second.second);

        _nodeInstances.clear();

        if (_outputNode != nullptr)
        {
            Node::Destruct("Output", _outputNode);
            _outputNode = nullptr;
        }

        if (_inputNode != nullptr)
        {
            Node::Destruct("Input", _inputNode);
            _inputNode = nullptr;
        }
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
            return _nodeInstances.at(id).second;

        if (_inputNode != nullptr && _inputNode->GetId() == id)
            return _inputNode;

        if (_outputNode != nullptr && _outputNode->GetId() == id)
            return _outputNode;

        return nullptr;
    }

    void PipelineInstanceImpl::Reset()
    {
        _inputNode->Reset();

        for (const auto& node : _nodeInstances)
            node.second.second->Reset();

        _outputNode->Reset();
    }

    void PipelineInstanceImpl::AddNode(AmObjectID id, AmString nodeName, NodeInstance* nodeInstance)
    {
        if (_nodeInstances.find(id) != _nodeInstances.end())
            return;

        _nodeInstances[id] = std::make_pair(nodeName, nodeInstance);
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

            Node* node = Node::Find(nodeName);
            NodeInstance* nodeInstance = nullptr;

            if (nodeName == "Input")
            {
                if (instance->_inputNode != nullptr)
                {
                    amLogError("More than one input node was found in the pipeline.");
                    DestroyInstance(instance);
                    return nullptr;
                }

                nodeInstance = node->CreateInstance();
                instance->_inputNode = static_cast<InputNodeInstance*>(nodeInstance);
            }
            else if (nodeName == "Output")
            {
                if (instance->_outputNode != nullptr)
                {
                    amLogError("More than one output node was found in the pipeline.");
                    DestroyInstance(instance);
                    return nullptr;
                }

                nodeInstance = node->CreateInstance();
                instance->_outputNode = static_cast<OutputNodeInstance*>(nodeInstance);
            }
            else
            {
                if (node != nullptr)
                    nodeInstance = node->CreateInstance();

                if (nodeInstance == nullptr)
                {
                    amLogError(
                        "Pipeline node not found: %s. Make sure it is registered. If the node is provided by a plugin, make sure to load "
                        "the "
                        "plugin before Amplitude.",
                        nodeName.c_str());
                    DestroyInstance(instance);
                    return nullptr;
                }

                instance->AddNode(nodeId, nodeName, nodeInstance);
            }

            // Initialize the node with the provided parameters
            nodeInstance->Initialize(nodeId, layer, instance);

            // Connect the node inputs
            if (node->CanConsume())
            {
                auto* consumerNode = dynamic_cast<ConsumerNodeInstance*>(nodeInstance);
                if (consumerNode == nullptr)
                {
                    amLogError(
                        "The node '%s' can consume, but it doesn't inherits ConsumerNodeInstance. This is a programming error.",
                        nodeName.c_str());
                    DestroyInstance(instance);
                    return nullptr;
                }

                if (!AM_BETWEEN(inputs->size(), node->GetMinInputCount(), node->GetMaxInputCount()))
                {
                    amLogError(
                        "The node '%s' requires %zu to %zu input(s), but %d were provided.", nodeName.c_str(), node->GetMinInputCount(),
                        node->GetMaxInputCount(), inputs->size());
                    DestroyInstance(instance);
                    return nullptr;
                }

                std::vector<AmObjectID> connectedNodes;
                for (flatbuffers::uoffset_t j = 0, m = inputs->size(); j < m; ++j)
                {
                    const AmObjectID producerNodeId = inputs->Get(j);

                    if (producerNodeId == nodeId)
                    {
                        amLogError("A node cannot consume itself: %s", nodeName.c_str());
                        DestroyInstance(instance);
                        return nullptr;
                    }

                    if (std::ranges::find(connectedNodes, producerNodeId) != connectedNodes.end())
                    {
                        amLogWarning(
                            "The node with ID '" AM_ID_CHAR_FMT "' is already connected to %s, skipping.", producerNodeId,
                            nodeName.c_str());
                        continue;
                    }

                    consumerNode->Connect(producerNodeId);
                    connectedNodes.push_back(producerNodeId);
                }
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
