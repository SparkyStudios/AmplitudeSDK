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

#ifndef _AM_IMPLEMENTATION_MIXER_NODES_STEREO_PANNING_NODE_H
#define _AM_IMPLEMENTATION_MIXER_NODES_STEREO_PANNING_NODE_H

#include <SparkyStudios/Audio/Amplitude/Mixer/Node.h>

namespace SparkyStudios::Audio::Amplitude
{
    class StereoPanningNodeInstance final : public ProcessorNodeInstance
    {
    public:
        StereoPanningNodeInstance(AmObjectID id, const Pipeline* pipeline);

        AudioBuffer Process(const AudioBuffer& input) override;
    };

    class StereoPanningNode final : public Node
    {
    public:
        StereoPanningNode();

        [[nodiscard]] AM_INLINE NodeInstance* CreateInstance(AmObjectID id, const Pipeline* pipeline) const override
        {
            return ampoolnew(MemoryPoolKind::Amplimix, StereoPanningNodeInstance, id, pipeline);
        }

        AM_INLINE void DestroyInstance(NodeInstance* instance) const override
        {
            ampooldelete(MemoryPoolKind::Amplimix, StereoPanningNodeInstance, (StereoPanningNodeInstance*)instance);
        }
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_MIXER_NODES_STEREO_PANNING_NODE_H