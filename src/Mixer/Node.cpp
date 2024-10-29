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

#include <SparkyStudios/Audio/Amplitude/Core/Log.h>
#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>
#include <SparkyStudios/Audio/Amplitude/Mixer/Node.h>
#include <SparkyStudios/Audio/Amplitude/Mixer/Pipeline.h>

#include <Sound/Effect.h>
#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    typedef std::map<std::string, Node*> NodeRegistry;
    typedef NodeRegistry::value_type NodeImpl;

    static NodeRegistry& nodeRegistry()
    {
        static NodeRegistry r;
        return r;
    }

    static bool& lockNodes()
    {
        static bool b = false;
        return b;
    }

    static AmUInt32& nodesCount()
    {
        static AmUInt32 c = 0;
        return c;
    }

    void NodeInstance::Initialize(AmObjectID id, const AmplimixLayer* layer, const PipelineInstance* node)
    {
        AMPLITUDE_ASSERT(layer != nullptr);

        m_id = id;
        m_layer = layer;
        m_pipeline = node;
    }

    AmObjectID NodeInstance::GetId() const
    {
        return m_id;
    }

    const AmplimixLayer* NodeInstance::GetLayer() const
    {
        return m_layer;
    }

    ProcessorNodeInstance::ProcessorNodeInstance(bool processOnEmptyInputBuffer)
        : m_provider(0)
        , _processingBuffer(nullptr)
        , _lastOuputBuffer(nullptr)
        , _processOnEmptyInputBuffer(processOnEmptyInputBuffer)
    {}

    void ProcessorNodeInstance::Consume()
    {
        if (m_layer == nullptr)
            return;

        AMPLITUDE_ASSERT(m_provider != kAmInvalidObjectId);

        NodeInstance* node = m_pipeline->GetNode(m_provider);
        AMPLITUDE_ASSERT(node != nullptr);

        ProviderNodeInstance* provider = dynamic_cast<ProviderNodeInstance*>(node);
        AMPLITUDE_ASSERT(provider != nullptr);

        _processingBuffer = provider->Provide();
    }

    void ProcessorNodeInstance::Connect(AmObjectID provider)
    {
        m_provider = provider;
    }

    const AudioBuffer* ProcessorNodeInstance::Provide()
    {
        if (m_layer == nullptr)
            return nullptr;

        if (_lastOuputBuffer != nullptr)
            return _lastOuputBuffer;

        if (_processingBuffer == nullptr)
            Consume();

        if (_processingBuffer == nullptr && !_processOnEmptyInputBuffer)
            return nullptr;

        return _lastOuputBuffer = Process(_processingBuffer);
    }

    void ProcessorNodeInstance::Reset()
    {
        _processingBuffer = nullptr;
        _lastOuputBuffer = nullptr;
    }

    MixerNodeInstance::MixerNodeInstance()
        : _processingBuffers()
        , _mixBuffer()
        , _processed(false)
    {}

    void MixerNodeInstance::Consume()
    {
        if (m_layer == nullptr)
            return;

        if (m_providers.empty())
            return;

        _processingBuffers.clear();
        for (const auto& providerId : m_providers)
        {
            AMPLITUDE_ASSERT(providerId != kAmInvalidObjectId);

            NodeInstance* node = m_pipeline->GetNode(providerId);
            AMPLITUDE_ASSERT(node != nullptr);

            ProviderNodeInstance* provider = dynamic_cast<ProviderNodeInstance*>(node);
            AMPLITUDE_ASSERT(provider != nullptr);

            _processingBuffers.push_back(provider->Provide());
        }
    }

    void MixerNodeInstance::Connect(AmObjectID provider)
    {
        m_providers.push_back(provider);
    }

    void MixerNodeInstance::Connect(const std::vector<AmObjectID>& providers)
    {
        m_providers = providers;
    }

    const AudioBuffer* MixerNodeInstance::Provide()
    {
        if (m_layer == nullptr)
            return nullptr;

        if (_processed)
            return &_mixBuffer;

        if (_processingBuffers.empty())
            Consume();

        if (_processingBuffers.empty())
            return nullptr;

        auto buffers = _processingBuffers |
            std::ranges::views::filter(
                           [](const AudioBuffer* buffer)
                           {
                               return buffer != nullptr && !buffer->IsEmpty();
                           });

        if (buffers.empty())
            return nullptr;

        const auto& first = buffers.begin();
        _mixBuffer = AudioBuffer((*first)->GetFrameCount(), (*first)->GetChannelCount());

        for (const auto& input : buffers)
        {
            AMPLITUDE_ASSERT(input->GetFrameCount() == _mixBuffer.GetFrameCount());
            AMPLITUDE_ASSERT(input->GetChannelCount() == _mixBuffer.GetChannelCount());

            _mixBuffer += *input;
        }

        _processed = true;
        return &_mixBuffer;
    }

    void MixerNodeInstance::Reset()
    {
        _processingBuffers.clear();
        _mixBuffer.Clear();

        _processed = false;
    }

    InputNodeInstance::InputNodeInstance()
        : _buffer(nullptr)
        , _filter(nullptr)
    {}

    void InputNodeInstance::SetInput(AudioBuffer* buffer)
    {
        _buffer = buffer;

        const auto* effect = static_cast<const EffectInstanceImpl*>(GetLayer()->GetEffect());
        if (effect != nullptr)
            _filter = effect->GetFilter();
    }

    const AudioBuffer* InputNodeInstance::Provide()
    {
        if (_buffer == nullptr)
            return nullptr;

        if (_filter != nullptr)
            _filter->Process(*_buffer, *_buffer, _buffer->GetFrameCount(), GetLayer()->GetSampleRate());

        return _buffer;
    }

    void InputNodeInstance::Reset()
    {
        _buffer = nullptr;
    }

    OutputNodeInstance::OutputNodeInstance()
        : _provider(0)
        , _buffer(nullptr)
    {}

    void OutputNodeInstance::SetOutput(AudioBuffer* buffer)
    {
        _buffer = buffer;
    }

    void OutputNodeInstance::Connect(AmObjectID provider)
    {
        _provider = provider;
    }

    void OutputNodeInstance::Consume()
    {
        if (m_layer == nullptr)
            return;

        if (_buffer == nullptr)
            return;

        AMPLITUDE_ASSERT(_provider != kAmInvalidObjectId);

        NodeInstance* node = m_pipeline->GetNode(_provider);
        AMPLITUDE_ASSERT(node != nullptr);

        ProviderNodeInstance* provider = dynamic_cast<ProviderNodeInstance*>(node);
        AMPLITUDE_ASSERT(provider != nullptr);

        const auto* output = provider->Provide();
        if (output == nullptr)
            return;

        *_buffer = *output;
    }

    void OutputNodeInstance::Reset()
    {
        _buffer = nullptr;
    }

    Node::Node(AmString name)
        : m_name(std::move(name))
    {
        Register(this);
    }

    Node::~Node()
    {
        Unregister(this);
    }

    const AmString& Node::GetName() const
    {
        return m_name;
    }

    void Node::Register(Node* node)
    {
        if (lockNodes() || node == nullptr)
            return;

        if (Find(node->GetName()) != nullptr)
        {
            amLogWarning("Failed to register node '%s' as it is already registered", node->GetName().c_str());
            return;
        }

        NodeRegistry& nodes = nodeRegistry();
        nodes.insert(NodeImpl(node->GetName(), node));
        nodesCount()++;
    }

    void Node::Unregister(const Node* node)
    {
        if (lockNodes() || node == nullptr)
            return;

        NodeRegistry& nodes = nodeRegistry();
        if (const auto& it = nodes.find(node->GetName()); it != nodes.end())
        {
            nodes.erase(it);
            nodesCount()--;
        }
    }

    Node* Node::Find(const std::string& name)
    {
        NodeRegistry& nodes = nodeRegistry();
        if (const auto& it = nodes.find(name); it != nodes.end())
            return it->second;

        return nullptr;
    }

    NodeInstance* Node::Construct(const std::string& name)
    {
        Node* node = Find(name);
        if (node == nullptr)
            return nullptr;

        return node->CreateInstance();
    }

    void Node::Destruct(const std::string& name, NodeInstance* instance)
    {
        if (instance == nullptr)
            return;

        Node* node = Find(name);
        if (node == nullptr)
            return;

        node->DestroyInstance(instance);
    }

    void Node::LockRegistry()
    {
        lockNodes() = true;
    }

    void Node::UnlockRegistry()
    {
        lockNodes() = false;
    }
} // namespace SparkyStudios::Audio::Amplitude
