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

#include <Core/Engine.h>
#include <Mixer/Nodes/ReflectionsNode.h>

namespace SparkyStudios::Audio::Amplitude
{
    ReflectionsNodeInstance::ReflectionsNodeInstance()
        : ProcessorNodeInstance(true)
        , _reflectionsProcessor(nullptr)
        , _numFramesProcessedOnEmptyInput(0)
    {}

    ReflectionsNodeInstance::~ReflectionsNodeInstance()
    {
        ampooldelete(eMemoryPoolKind_Amplimix, ReflectionsProcessor, _reflectionsProcessor);
        _reflectionsProcessor = nullptr;
    }

    void ReflectionsNodeInstance::Initialize(AmObjectID id, const AmplimixLayer* layer, const PipelineInstance* node)
    {
        ProcessorNodeInstance::Initialize(id, layer, node);

        const auto& deviceConfig = amEngine->GetMixer()->GetDeviceDescription();

        _orientationProcessor.Configure(1, true);
        _reflectionsProcessor = ampoolnew(
            eMemoryPoolKind_Amplimix, ReflectionsProcessor, deviceConfig.mDeviceOutputSampleRate, amEngine->GetSamplesPerStream());

        _output.Configure(1, true, amEngine->GetSamplesPerStream());
        _silenceBuffer = AudioBuffer(amEngine->GetSamplesPerStream(), kAmMonoChannelCount);

        Reset();
    }

    const AudioBuffer* ReflectionsNodeInstance::Process(const AudioBuffer* input)
    {
        if (input == nullptr)
        {
            if (_numFramesProcessedOnEmptyInput < _reflectionsProcessor->GetNumFramesToProcessOnEmptyInput())
            {
                _numFramesProcessedOnEmptyInput += _silenceBuffer.GetFrameCount();
                input = &_silenceBuffer;
            }
            else
                return nullptr;
        }
        else
        {
            _numFramesProcessedOnEmptyInput = 0;
            AMPLITUDE_ASSERT(input->GetChannelCount() == kAmMonoChannelCount);
        }

        const auto* layer = GetLayer();
        const auto& room = layer->GetRoom();
        const AmReal32 roomGain = layer->GetChannel().GetState()->GetRoomGain(room.GetId());

        if (roomGain < kEpsilon)
            return nullptr;

        const auto& listener = layer->GetListener();
        if (!listener.Valid())
            return nullptr;

        _output.Reset();

        {
            // Apply reflections gain
            AudioBuffer temp(input->GetFrameCount(), kAmMonoChannelCount);
            Gain::ApplyReplaceConstantGain(roomGain, input->GetChannel(0), 0, temp[0], 0, _output.GetSampleCount());

            // Process reflections
            _reflectionsProcessor->Process(temp, &_output);
        }

        // Rotate the reflections to match the listener's orientation
        _orientationProcessor.SetOrientation(Orientation(AM_InvQ(listener.GetOrientation().GetQuaternion())));
        _orientationProcessor.Process(&_output, _output.GetSampleCount());

        return _output.GetBuffer();
    }

    void ReflectionsNodeInstance::Reset()
    {
        ProcessorNodeInstance::Reset();

        const auto* layer = GetLayer();
        if (layer == nullptr)
            return;

        const auto& listener = layer->GetListener();
        if (!listener.Valid())
            return;

        const auto& room = layer->GetRoom();
        _reflectionsProcessor->Update(room.GetState(), listener.GetLocation(), amEngine->GetSoundSpeed());
    }

    ReflectionsNode::ReflectionsNode()
        : Node("Reflections")
    {}
} // namespace SparkyStudios::Audio::Amplitude
