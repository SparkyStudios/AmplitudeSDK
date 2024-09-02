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

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>
#include <SparkyStudios/Audio/Amplitude/Mixer/Amplimix.h>

#include <Mixer/Nodes/StereoMixerNode.h>
#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    StereoMixerNodeInstance::StereoMixerNodeInstance(AmObjectID id, const Pipeline* pipeline)
        : MixerNodeInstance(id, pipeline)
    {}

    AudioBuffer StereoMixerNodeInstance::Mix(const std::vector<AudioBuffer>& inputs)
    {
        AudioBuffer output(kAmMaxSupportedFrameCount, 2);

        if (inputs.empty())
            return output;

        output = AudioBuffer(inputs[0].GetFrameCount(), 2);

        constexpr AmReal32 ratio = 1.0f;

        for (AmSize i = 0, l = inputs.size(); i < l; ++i)
        {
            if (inputs[i].IsEmpty())
                continue;

            AMPLITUDE_ASSERT(inputs[i].GetFrameCount() == output.GetFrameCount());
            AMPLITUDE_ASSERT(inputs[i].GetChannelCount() == output.GetChannelCount());

            ScalarMultiplyAccumulate(inputs[i].GetData().GetBuffer(), output.GetData().GetBuffer(), ratio, output.GetData().GetSize());
        }

        return output;
    }

    StereoMixerNode::StereoMixerNode()
        : Node("StereoMixer")
    {}
} // namespace SparkyStudios::Audio::Amplitude
