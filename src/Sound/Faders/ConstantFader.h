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

#pragma once

#ifndef _AM_IMPLEMENTATION_SOUND_FADERS_CONSTANT_FADER_H
#define _AM_IMPLEMENTATION_SOUND_FADERS_CONSTANT_FADER_H

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Fader.h>

namespace SparkyStudios::Audio::Amplitude
{
    constexpr BezierCurveControlPoints gConstantFaderCurveControlPoints = { 0.0f, 0.0f, 0.0f, 0.0f };

    class ConstantFaderInstance final : public FaderInstance
    {
    public:
        ConstantFaderInstance()
        {
            m_curve = gConstantFaderCurveControlPoints;
        }
    };

    class ConstantFader final : public Fader
    {
    public:
        ConstantFader()
            : Fader("Constant")
        {}

        FaderInstance* CreateInstance() override
        {
            return amnew(ConstantFaderInstance);
        }

        void DestroyInstance(FaderInstance* instance) override
        {
            amdelete(ConstantFaderInstance, (ConstantFaderInstance*)instance);
        }

        [[nodiscard]] BezierCurveControlPoints GetControlPoints() const override
        {
            return gConstantFaderCurveControlPoints;
        }
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_SOUND_FADERS_CONSTANT_FADER_H
