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

#ifndef _AM_IMPLEMENTATION_SOUND_FADERS_S_CURVE_FADER_H
#define _AM_IMPLEMENTATION_SOUND_FADERS_S_CURVE_FADER_H

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Fader.h>

namespace SparkyStudios::Audio::Amplitude
{
    constexpr BezierCurveControlPoints gSCurveSmoothFaderCurveControlPoints = { 0.64f, 0.0f, 0.36f, 1.0f };
    constexpr BezierCurveControlPoints gSCurveSharpFaderCurveControlPoints = { 0.9f, 0.0f, 0.1f, 1.0f };

    class SCurveFaderInstance final : public FaderInstance
    {
    public:
        explicit SCurveFaderInstance(const BezierCurveControlPoints& curveControlPoints)
        {
            m_curve = curveControlPoints;
        }
    };

    class SCurveSmoothFader final : public Fader
    {
    public:
        SCurveSmoothFader()
            : Fader("SCurveSmooth")
        {}

        FaderInstance* CreateInstance() override
        {
            return amnew(SCurveFaderInstance, gSCurveSmoothFaderCurveControlPoints);
        }

        void DestroyInstance(FaderInstance* instance) override
        {
            amdelete(SCurveFaderInstance, (SCurveFaderInstance*)instance);
        }

        [[nodiscard]] BezierCurveControlPoints GetControlPoints() const override
        {
            return gSCurveSmoothFaderCurveControlPoints;
        }
    };

    class SCurveSharpFader final : public Fader
    {
    public:
        SCurveSharpFader()
            : Fader("SCurveSharp")
        {}

        FaderInstance* CreateInstance() override
        {
            return amnew(SCurveFaderInstance, gSCurveSharpFaderCurveControlPoints);
        }

        void DestroyInstance(FaderInstance* instance) override
        {
            amdelete(SCurveFaderInstance, (SCurveFaderInstance*)instance);
        }

        [[nodiscard]] BezierCurveControlPoints GetControlPoints() const override
        {
            return gSCurveSharpFaderCurveControlPoints;
        }
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_SOUND_FADERS_S_CURVE_FADER_H
