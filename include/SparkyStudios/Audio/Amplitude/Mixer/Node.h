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

#ifndef _AM_MIXER_NODE_H
#define _AM_MIXER_NODE_H

#include <SparkyStudios/Audio/Amplitude/Core/AudioBuffer.h>
#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/DSP/Filter.h>

namespace SparkyStudios::Audio::Amplitude
{
    class AmplimixLayer;
    class PipelineInstance;

    /**
     * @brief An instance of an Amplimix pipeline node.
     *
     * This class represents the actual node executed within the
     * Amplimix pipeline. Each node instance has a unique ID assigned
     * to it, that matches the one provided in the pipeline asset.
     *
     * @ingroup mixer
     */
    class AM_API_PUBLIC NodeInstance
    {
        friend class PipelineInstance;

    public:
        /**
         * @brief Initializes the node instance.
         *
         * @param[in] id Unique identifier for the node instance.
         * @param[in] layer The Amplimix layer this node instance is currently associated with.
         * @param[in] pipeline The pipeline this node instance belongs to.
         */
        virtual void Initialize(AmObjectID id, const AmplimixLayer* layer, const PipelineInstance* pipeline);

        /**
         * @brief Default destructor.
         */
        virtual ~NodeInstance() = default;

        /**
         * @brief Gets the unique identifier for the node instance.
         *
         * @return The unique identifier for the node instance.
         */
        [[nodiscard]] AmObjectID GetId() const;

        /**
         * @brief Gets the Amplimix layer this node instance is currently associated with.
         *
         * @return The Amplimix layer this node instance is currently associated with.
         *
         * @see [AmplimixLayer](../AmplimixLayer/index.md)
         */
        [[nodiscard]] const AmplimixLayer* GetLayer() const;

        /**
         * @brief Resets the node instance's internal state.
         *
         * @warning This function should be called automatically by Amplimix, each time the pipeline is
         * about to be executed. Call it manually only if you know what you're doing.
         */
        virtual void Reset() = 0;

    protected:
        AmObjectID m_id; ///< The unique identifier for the node instance in the pipeline.
        const AmplimixLayer* m_layer; ///< The Amplimix layer this node instance is currently associated with.
        const PipelineInstance* m_pipeline; ///< The pipeline this node instance belongs to.
    };

    /**
     * @brief Interface for Amplimix pipeline nodes that can consume audio data from an input buffer.
     *
     * @ingroup mixer
     */
    class AM_API_PUBLIC ConsumerNodeInstance
    {
    public:
        /**
         * @brief Default destructor.
         */
        virtual ~ConsumerNodeInstance() = default;

        /**
         * @brief Consumes audio data from the provider node.
         *
         * The provider node should be  specified with the call of @ref Connect `Connect()`.
         */
        virtual void Consume() = 0;

        /**
         * @brief Sets the input provider node ID for this node.
         *
         * @param[in] provider The provider node for this node.
         */
        virtual void Connect(AmObjectID provider) = 0;
    };

    /**
     * @brief Interface for Amplimix pipeline nodes that can provide audio data to an output buffer.
     *
     * @ingroup mixer
     */
    class AM_API_PUBLIC ProviderNodeInstance
    {
    public:
        /**
         * @brief Default destructor.
         */
        virtual ~ProviderNodeInstance() = default;

        /**
         * @brief Produces audio data ready to be taken as input from a consumer node.
         *
         * @return The output audio data.
         *
         * @see [AudioBuffer](../../core/AudioBuffer/index.md)
         */
        virtual const AudioBuffer* Provide() = 0;
    };

    /**
     * @brief Base class for Amplimix pipeline nodes that can process audio data in-place.
     *
     * @see [NodeInstance](../NodeInstance/index.md), [ConsumerNodeInstance](../ConsumerNodeInstance/index.md),
     * [ProviderNodeInstance](../ProviderNodeInstance/index.md)
     *
     * @ingroup mixer
     */
    class AM_API_PUBLIC ProcessorNodeInstance
        : public NodeInstance
        , public ConsumerNodeInstance
        , public ProviderNodeInstance
    {
    public:
        /**
         * @brief PropertyNodeInstance constructor.
         *
         * @param[in] processOnEmptyBuffer If `true`, the node will execute the @ref Process `Process()` method
         * even if the input buffer is `nullptr`.
         */
        explicit ProcessorNodeInstance(bool processOnEmptyBuffer = false);

        /**
         * @brief Default destructor.
         */
        ~ProcessorNodeInstance() override = default;

        /**
         * @brief Processes input audio data and returns the output audio data.
         *
         * @param[in] input The input audio data to process.
         *
         * @return The output audio data.
         */
        virtual const AudioBuffer* Process(const AudioBuffer* input) = 0;

        /**
         * @inherit
         */
        void Consume() final;

        /**
         * @inherit
         */
        void Connect(AmObjectID provider) final;

        /**
         * @inherit
         */
        const AudioBuffer* Provide() final;

        /**
         * @inherit
         */
        void Reset() override;

    protected:
        AmObjectID m_provider; ///< The ID of the input provider node.

    private:
        const AudioBuffer* _processingBuffer;
        const AudioBuffer* _lastOutputBuffer;
        bool _processOnEmptyInputBuffer;
    };

    /**
     * @brief Base class for Amplimix pipeline nodes that can mix audio data from multiple input buffers.
     *
     * @see [NodeInstance](../NodeInstance/index.md), [ConsumerNodeInstance](../ConsumerNodeInstance/index.md),
     * [ProviderNodeInstance](../ProviderNodeInstance/index.md)
     *
     * @ingroup mixer
     */
    class AM_API_PUBLIC MixerNodeInstance
        : public NodeInstance
        , public ConsumerNodeInstance
        , public ProviderNodeInstance
    {
    public:
        /**
         * @brief MixerNodeInstance constructor.
         */
        MixerNodeInstance();

        /**
         * @brief Default destructor.
         */
        ~MixerNodeInstance() override = default;

        /**
         * @inherit
         */
        void Consume() final;

        /**
         * @inherit
         *
         * @note This method appends the given provider to the list of input
         * provider nodes. If the provider node already exists in the list, it
         * will not be added again.
         */
        void Connect(AmObjectID provider) final;

        /**
         * @brief Sets the input provider nodes for this mixer node.
         *
         * @param[in] providers The provider nodes for this mixer node.
         *
         * @note This method clears the existing input provider nodes,
         * and replaces them with the provided ones.
         */
        void Connect(const std::vector<AmObjectID>& providers);

        /**
         * @inherit
         */
        const AudioBuffer* Provide() final;

        /**
         * @inherit
         */
        void Reset() override;

    protected:
        std::vector<AmObjectID> m_providers; ///< The IDs of the input provider nodes.

    private:
        std::vector<const AudioBuffer*> _processingBuffers;
        AudioBuffer _mixBuffer;
        bool _processed;
    };

    /**
     * @brief Class used to mark the input of the pipeline.
     *
     * @warning This node is automatically added to the pipeline when created. And thus
     * should not be manually added to the pipeline asset.
     *
     * @see [ProviderNodeInstance](../ProviderNodeInstance/index.md)
     *
     * @ingroup mixer
     */
    class AM_API_PUBLIC InputNodeInstance
        : public NodeInstance
        , public ProviderNodeInstance
    {
        friend class PipelineInstance;

    public:
        /**
         * @brief InputNodeInstance constructor.
         */
        InputNodeInstance();

        /**
         * @brief Default destructor.
         */
        ~InputNodeInstance() override = default;

        /**
         * @brief Sets the input of the pipeline.
         *
         * @param[in] buffer The buffer to set as the input.
         */
        void SetInput(AudioBuffer* buffer);

        /**
         * @inherit
         */
        const AudioBuffer* Provide() override;

        /**
         * @inherit
         */
        void Reset() override;

    private:
        AudioBuffer* _buffer;
        FilterInstance* _filter;
    };

    /**
     * @brief Class used to mark the output of the pipeline.
     *
     * @warning This node is automatically added to the pipeline when created. And thus
     * should not be manually added to the pipeline asset.
     *
     * @see [ConsumerNodeInstance](../ConsumerNodeInstance/index.md)
     *
     * @ingroup mixer
     */
    class AM_API_PUBLIC OutputNodeInstance
        : public NodeInstance
        , public ConsumerNodeInstance
    {
        friend class PipelineInstance;

    public:
        /**
         * @brief OutputNodeInstance constructor.
         */
        OutputNodeInstance();

        /**
         * @brief Default destructor.
         */
        ~OutputNodeInstance() override = default;

        /**
         * @brief Sets the output of the pipeline.
         *
         * @param[in] buffer The buffer to set as the output.
         */
        void SetOutput(AudioBuffer* buffer);

        /**
         * @inherit
         */
        void Consume() override;

        /**
         * @inherit
         */
        void Connect(AmObjectID provider) override;

        /**
         * @inherit
         */
        void Reset() override;

    private:
        AmObjectID _provider;
        AudioBuffer* _buffer;
    };

    /**
     * @brief Base class for Amplimix pipeline nodes.
     *
     * This class presents the basic structure to create Amplimix pipeline nodes. Each
     * `Node` of your pipelines must be derived from this class and implement the @ref CreateInstance `CreateInstance()`
     * and @ref DestroyInstance `DestroyInstance()` methods.
     *
     * @see [NodeInstance](../NodeInstance/index.md)
     *
     * @ingroup mixer
     */
    class AM_API_PUBLIC Node
    {
    public:
        /**
         * @brief Node constructor.
         *
         * @param[in] name Name of the node. Should be unique within the pipeline.
         */
        explicit Node(AmString name);

        /**
         * @brief Node destructor.
         */
        virtual ~Node();

        /**
         * @brief Creates a new instance of the node.
         *
         * @return A new instance of the node.
         */
        virtual NodeInstance* CreateInstance() const = 0;

        /**
         * @brief Destroys the specified instance of the node.
         *
         * @param[in] instance Pointer to the instance to be destroyed.
         */
        virtual void DestroyInstance(NodeInstance* instance) const = 0;

        /**
         * @brief Returns the name of the node.
         */
        [[nodiscard]] const AmString& GetName() const;

        /**
         * @brief Registers a new node.
         *
         * @param[in] node The node to add in the registry.
         */
        static void Register(Node* node);

        /**
         * @brief Unregisters a node.
         *
         * @param[in] node The node to remove from the registry.
         */
        static void Unregister(const Node* node);

        /**
         * @brief Creates a new instance of the node with the given name
         * and returns its pointer. The returned pointer should be deleted using Node::Destruct().
         *
         * @param[in] name The name of the node.
         *
         * @return The node with the given name, or `nullptr` if none.
         */
        static NodeInstance* Construct(const AmString& name);

        /**
         * @brief Destroys the given node instance.
         *
         * @param[in] name The name of the node.
         * @param[in] instance The node instance to destroy.
         */
        static void Destruct(const AmString& name, NodeInstance* instance);

        /**
         * @brief Locks the nodes' registry.
         *
         * @warning This function is mainly used for internal purposes. It's
         * called before the `Engine` initialization, to discard the registration
         * of new nodes after the engine is fully loaded.
         */
        static void LockRegistry();

        /**
         * @brief Unlocks the nodes' registry.
         *
         * @warning This function is mainly used for internal purposes. It's
         * called after the `Engine` deinitialization, to allow the registration
         * of new nodes after the engine is fully unloaded.
         */
        static void UnlockRegistry();

        /**
         * @brief Gets the list of registered nodes.
         *
         * @return The registry of nodes.
         */
        static const std::map<AmString, Node*>& GetRegistry();

        /**
         * @brief Look up a node by name.
         *
         * @return The node with the given name, or NULL if none.
         *
         * @internal
         */
        static Node* Find(const AmString& name);

    protected:
        /**
         * @brief The name of this node.
         */
        AmString m_name;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_MIXER_NODE_H