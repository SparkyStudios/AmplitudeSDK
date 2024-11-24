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

#include <SparkyStudios/Audio/Amplitude/Core/Log.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Fader.h>

namespace SparkyStudios::Audio::Amplitude
{
    typedef std::map<AmString, Fader*> FaderRegistry;
    typedef FaderRegistry::value_type FaderImpl;

    static constexpr AmUInt32 kNewtonIterations = 4;
    static constexpr AmReal64 kNewtonMinSlope = 0.001;
    static constexpr AmReal64 kSubdivisionPrecision = 0.0000001;
    static constexpr AmUInt32 kSubdivisionMaxIterations = 10;

    static constexpr AmUInt32 kSplineTableSize = 11;
    static constexpr AmReal64 kSampleStepSize = 1.0 / (kSplineTableSize - 1.0);

    static AmReal64 A(const AmReal64 a1, const AmReal64 a2)
    {
        return 1.0 - 3.0 * a2 + 3.0 * a1;
    }

    static AmReal64 B(const AmReal64 a1, const AmReal64 a2)
    {
        return 3.0 * a2 - 6.0 * a1;
    }

    static AmReal64 C(const AmReal64 a1)
    {
        return 3.0 * a1;
    }

    // Returns x(t) given t, x1, and x2, or y(t) given t, y1, and y2.
    static AmReal64 CalculateBezier(const AmReal64 t, const AmReal64 a1, const AmReal64 a2)
    {
        return ((A(a1, a2) * t + B(a1, a2)) * t + C(a1)) * t;
    }

    // Returns dx/dt given t, x1, and x2, or dy/dt given t, y1, and y2.
    static AmReal64 GetSlope(const AmReal64 t, const AmReal64 a1, const AmReal64 a2)
    {
        return 3.0 * A(a1, a2) * t * t + 2.0 * B(a1, a2) * t + C(a1);
    }

    static AmReal64 BinarySubdivide(const AmReal64 x, AmReal64 a, AmReal64 b, const AmReal64 mX1, const AmReal64 mX2)
    {
        AmReal64 currentX, currentT, i = 0;
        do
        {
            currentT = a + (b - a) / 2.0;
            currentX = CalculateBezier(currentT, mX1, mX2) - x;
            if (currentX > 0.0)
            {
                b = currentT;
            }
            else
            {
                a = currentT;
            }
        } while (std::abs(currentX) > kSubdivisionPrecision && ++i < kSubdivisionMaxIterations);

        return currentT;
    }

    static AmReal64 NewtonRaphsonIterate(const AmReal64 x, AmReal64 aGuessT, const AmReal64 mX1, const AmReal64 mX2)
    {
        for (AmUInt32 i = 0; i < kNewtonIterations; ++i)
        {
            const AmReal64 currentSlope = GetSlope(aGuessT, mX1, mX2);
            if (currentSlope == 0.0)
            {
                return aGuessT;
            }

            const AmReal64 currentX = CalculateBezier(aGuessT, mX1, mX2) - x;
            aGuessT -= currentX / currentSlope;
        }

        return aGuessT;
    }

    FaderInstance::Transition::Transition(const AmReal32 x1, const AmReal32 y1, const AmReal32 x2, const AmReal32 y2)
        : m_controlPoints({ x1, y1, x2, y2 })
        , _samples()
    {
        for (AmUInt32 i = 0; i < kSplineTableSize; ++i)
        {
            _samples[i] = CalculateBezier(i * kSampleStepSize, m_controlPoints.x1, m_controlPoints.x2);
        }
    }

    FaderInstance::Transition::Transition(const BezierCurveControlPoints& controlPoints)
        : Transition(controlPoints.x1, controlPoints.y1, controlPoints.x2, controlPoints.y2)
    {}

    AmTime FaderInstance::Transition::Ease(AmTime t) const
    {
        // No transition (constant until the end)
        if (m_controlPoints.x1 == m_controlPoints.x2 && m_controlPoints.y1 == m_controlPoints.y2)
        {
            t = AM_CLAMP(t, 0.0, 1.0);
            return t == 1.0 ? 1.0 : 0.0;
        }

        // Linear transition
        if (m_controlPoints.x1 == m_controlPoints.y1 && m_controlPoints.x2 == m_controlPoints.y2)
        {
            return t;
        }

        // Don't waste time calculating extreme values.
        if (t <= 0.0 || t >= 1.0)
        {
            return t;
        }

        return CalculateBezier(GetTFromX(t), m_controlPoints.y1, m_controlPoints.y2);
    }

    AmTime FaderInstance::Transition::GetTFromX(AmReal64 x) const
    {
        AmReal64 intervalStart = 0.0;
        AmUInt32 currentSample = 1;

        for (constexpr AmUInt32 lastSample = kSplineTableSize - 1; currentSample != lastSample && _samples[currentSample] <= x;
             ++currentSample)
            intervalStart += kSampleStepSize;

        --currentSample;

        // Interpolate to provide an initial guess for t
        const AmReal64 dist = (x - _samples[currentSample]) / (_samples[currentSample + 1] - _samples[currentSample]);
        const AmReal64 guessForT = intervalStart + dist * kSampleStepSize;

        const AmReal64 initialSlope = GetSlope(guessForT, m_controlPoints.x1, m_controlPoints.x2);
        if (initialSlope >= kNewtonMinSlope)
            return NewtonRaphsonIterate(x, guessForT, m_controlPoints.x1, m_controlPoints.x2);

        if (initialSlope == 0.0)
            return guessForT;

        return BinarySubdivide(x, intervalStart, intervalStart + kSampleStepSize, m_controlPoints.x1, m_controlPoints.x2);
    }

    FaderInstance::FaderInstance()
        : m_curve(0.0f, 0.0f, 0.0f, 0.0f)
    {
        m_from = m_to = m_delta = 0;
        m_time = m_startTime = m_endTime = 0;
        m_state = eFaderState_Disabled;
    }

    void FaderInstance::Set(AmReal64 from, AmReal64 to, AmTime duration)
    {
        SetDuration(duration);
        Set(from, to);
        Start(0.0);
    }

    void FaderInstance::Set(AmReal64 from, AmReal64 to)
    {
        m_from = from;
        m_to = to;
        m_delta = to - from;
    }

    void FaderInstance::SetDuration(AmTime duration)
    {
        m_time = duration;
    }

    AmReal64 FaderInstance::GetFromTime(AmTime time)
    {
        if (m_state != eFaderState_Active)
            return 0.0f;

        if (m_startTime >= time)
            return m_from;

        if (time >= m_endTime)
            return m_to;

        return GetFromPercentage((time - m_startTime) / (m_endTime - m_startTime));
    }

    AmReal64 FaderInstance::GetFromPercentage(AmReal64 percentage)
    {
        percentage = m_curve.Ease(AM_CLAMP(percentage, 0.0, 1.0));
        return (1.0 - percentage) * m_from + percentage * m_to;
    }

    void FaderInstance::Start(AmTime time)
    {
        m_startTime = time;
        m_endTime = m_startTime + m_time;
        m_state = eFaderState_Active;
    }

    static FaderRegistry& faderRegistry()
    {
        static FaderRegistry r;
        return r;
    }

    static bool& lockFaders()
    {
        static bool b = false;
        return b;
    }

    static AmUInt32& fadersCount()
    {
        static AmUInt32 c = 0;
        return c;
    }

    Fader::Fader(AmString name)
        : m_name(std::move(name))
    {
        Register(this);
    }

    Fader::Fader()
        : m_name()
    {}

    Fader::~Fader()
    {
        Unregister(this);
    }

    const AmString& Fader::GetName() const
    {
        return m_name;
    }

    void Fader::Register(Fader* fader)
    {
        if (lockFaders() || fader == nullptr)
            return;

        if (Find(fader->GetName()) != nullptr)
        {
            amLogWarning("Failed to register fader '%s' as it is already registered", fader->GetName().c_str());
            return;
        }

        FaderRegistry& faders = faderRegistry();
        faders.insert(FaderImpl(fader->GetName(), fader));
        fadersCount()++;
    }

    void Fader::Unregister(const Fader* fader)
    {
        if (lockFaders() || fader == nullptr)
            return;

        FaderRegistry& faders = faderRegistry();
        if (const auto& it = faders.find(fader->GetName()); it != faders.end())
        {
            faders.erase(it);
            fadersCount()--;
        }
    }

    Fader* Fader::Find(const AmString& name)
    {
        const FaderRegistry& faders = faderRegistry();
        if (const auto& it = faders.find(name); it != faders.end())
            return it->second;

        return nullptr;
    }

    FaderInstance* Fader::Construct(const AmString& name)
    {
        Fader* fader = Find(name);
        if (fader == nullptr)
            return nullptr;

        return fader->CreateInstance();
    }

    void Fader::Destruct(const AmString& name, FaderInstance* instance)
    {
        if (instance == nullptr)
            return;

        Fader* fader = Find(name);
        if (fader == nullptr)
            return;

        fader->DestroyInstance(instance);
    }

    void Fader::LockRegistry()
    {
        lockFaders() = true;
    }

    void Fader::UnlockRegistry()
    {
        lockFaders() = false;
    }

    const std::map<AmString, Fader*>& Fader::GetRegistry()
    {
        return faderRegistry();
    }
} // namespace SparkyStudios::Audio::Amplitude