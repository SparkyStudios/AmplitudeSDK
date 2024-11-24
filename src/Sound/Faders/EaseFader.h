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

#ifndef _AM_IMPLEMENTATION_SOUND_FADERS_EASE_FADER_H
#define _AM_IMPLEMENTATION_SOUND_FADERS_EASE_FADER_H

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Fader.h>

namespace SparkyStudios::Audio::Amplitude
{
    constexpr BezierCurveControlPoints gEaseFaderCurveControlPoints = { 0.25f, 0.1f, 0.25f, 1.0f };

    class EaseFaderInstance final : public FaderInstance
    {
    public:
        EaseFaderInstance()
        {
            m_curve = gEaseFaderCurveControlPoints;
        }
    };

    class EaseFader final : public Fader
    {
    public:
        EaseFader()
            : Fader("Ease")
        {}

        FaderInstance* CreateInstance() override
        {
            return amnew(EaseFaderInstance);
        }

        void DestroyInstance(FaderInstance* instance) override
        {
            amdelete(EaseFaderInstance, (EaseFaderInstance*)instance);
        }

        [[nodiscard]] BezierCurveControlPoints GetControlPoints() const override
        {
            return gEaseFaderCurveControlPoints;
        }
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_SOUND_FADERS_EASE_FADER_H
