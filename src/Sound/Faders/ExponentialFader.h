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

#ifndef _AM_IMPLEMENTATION_SOUND_FADERS_EXPONENTIAL_FADER_H
#define _AM_IMPLEMENTATION_SOUND_FADERS_EXPONENTIAL_FADER_H

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Fader.h>

namespace SparkyStudios::Audio::Amplitude
{
    constexpr BezierCurveControlPoints gExponentialFaderCurveControlPoints = { 1.0f, 0.0f, 1.0f, 1.0f };

    class ExponentialFaderInstance final : public FaderInstance
    {
    public:
        ExponentialFaderInstance()
        {
            m_curve = gExponentialFaderCurveControlPoints;
        }
    };

    class ExponentialFader final : public Fader
    {
    public:
        ExponentialFader()
            : Fader("Exponential")
        {}

        FaderInstance* CreateInstance() override
        {
            return amnew(ExponentialFaderInstance);
        }

        void DestroyInstance(FaderInstance* instance) override
        {
            amdelete(ExponentialFaderInstance, (ExponentialFaderInstance*)instance);
        }

        [[nodiscard]] BezierCurveControlPoints GetControlPoints() const override
        {
            return gExponentialFaderCurveControlPoints;
        }
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_SOUND_FADERS_EXPONENTIAL_FADER_H
