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

#ifndef _AM_IMPLEMENTATION_MIXER_NODES_CLAMP_NODE_H
#define _AM_IMPLEMENTATION_MIXER_NODES_CLAMP_NODE_H

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>
#include <SparkyStudios/Audio/Amplitude/Mixer/Node.h>

namespace SparkyStudios::Audio::Amplitude
{
    class ClampNodeInstance final : public ProcessorNodeInstance
    {
    public:
        ClampNodeInstance();

        const AudioBuffer* Process(const AudioBuffer* input) override;

    private:
        AudioBuffer _output;
    };

    class ClampNode final : public Node
    {
    public:
        ClampNode();

        [[nodiscard]] AM_INLINE NodeInstance* CreateInstance() const override
        {
            return ampoolnew(eMemoryPoolKind_Amplimix, ClampNodeInstance);
        }

        AM_INLINE void DestroyInstance(NodeInstance* instance) const override
        {
            ampooldelete(eMemoryPoolKind_Amplimix, ClampNodeInstance, (ClampNodeInstance*)instance);
        }

        [[nodiscard]] AM_INLINE bool CanConsume() const override
        {
            return true;
        }

        [[nodiscard]] AM_INLINE bool CanProduce() const override
        {
            return true;
        }

        [[nodiscard]] AM_INLINE AmSize GetMaxInputCount() const override
        {
            return 1;
        }

        [[nodiscard]] AM_INLINE AmSize GetMinInputCount() const override
        {
            return 1;
        }
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_MIXER_NODES_CLAMP_NODE_H