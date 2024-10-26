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

#include <SparkyStudios/Audio/Amplitude/Math/Curve.h>

#include "common_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    bool CurvePoint::operator==(const CurvePoint& rhs) const
    {
        return x == rhs.x && y == rhs.y;
    }

    bool CurvePoint::operator!=(const CurvePoint& rhs) const
    {
        return !(rhs == *this);
    }

    CurvePart::CurvePart()
        : _start()
        , _end()
        , _faderFactory(nullptr)
        , _fader(nullptr)
    {}

    CurvePart::CurvePart(const CurvePart& other)
        : CurvePart()
    {
        SetStart(other._start);
        SetEnd(other._end);

        if (other._fader != nullptr)
            SetFader(other._faderFactory->GetName());
    }

    CurvePart::~CurvePart()
    {
        if (_fader != nullptr)
            _faderFactory->DestroyInstance(_fader);

        _fader = nullptr;
        _faderFactory = nullptr;
    }

    void CurvePart::Initialize(const CurvePartDefinition* definition)
    {
        if (definition == nullptr)
            return;

        _start = { definition->start()->x(), definition->start()->y() };
        _end = { definition->end()->x(), definition->end()->y() };

        SetFader(definition->fader()->str());
    }

    const CurvePoint& CurvePart::GetStart() const
    {
        return _start;
    }

    void CurvePart::SetStart(const CurvePoint& start)
    {
        _start = start;

        if (_fader != nullptr)
            _fader->Set(_start.y, _end.y, 0.0);
    }

    const CurvePoint& CurvePart::GetEnd() const
    {
        return _end;
    }

    void CurvePart::SetEnd(const CurvePoint& end)
    {
        _end = end;

        if (_fader != nullptr)
            _fader->Set(_start.y, _end.y, 0.0);
    }

    FaderInstance* CurvePart::GetFader() const
    {
        return _fader;
    }

    void CurvePart::SetFader(const AmString& fader)
    {
        if (_fader != nullptr)
        {
            _faderFactory->DestroyInstance(_fader);
            _fader = nullptr;
        }

        _faderFactory = Fader::Find(fader);
        if (_faderFactory == nullptr)
            return;

        _fader = _faderFactory->CreateInstance();
        _fader->Set(_start.y, _end.y, 0.0);
    }

    AmReal32 CurvePart::Get(AmReal64 x) const
    {
        const AmReal64 percentage = (x - _start.x) / (_end.x - _start.x);
        return _fader->GetFromPercentage(percentage);
    }

    CurvePart& CurvePart::operator=(const CurvePart& other)
    {
        if (this == &other)
            return *this;

        SetStart(other._start);
        SetEnd(other._end);

        SetFader(other._faderFactory->GetName());

        return *this;
    }

    Curve::Curve()
        : _parts()
    {}

    void Curve::Initialize(const CurveDefinition* definition)
    {
        if (definition == nullptr)
            return;

        const flatbuffers::uoffset_t size = definition->parts() ? definition->parts()->size() : 0;
        _parts.resize(size);

        for (flatbuffers::uoffset_t i = 0; i < size; ++i)
            _parts[i].Initialize(definition->parts()->Get(i));
    }

    void Curve::Initialize(const std::vector<CurvePart>& parts)
    {
        _parts.resize(parts.size());

        for (size_t i = 0; i < parts.size(); ++i)
            _parts[i] = parts[i];
    }

    AmReal32 Curve::Get(AmReal64 x) const
    {
        if (_parts.empty())
            return 0.0f;

        // Search the curve range corresponding to the x value.
        const CurvePart* part = _findCurvePart(x);

        // If no curve part found
        if (part == nullptr)
            return 0.0f;

        return part->Get(x);
    }

    const CurvePart* Curve::_findCurvePart(AmReal64 x) const
    {
        for (const auto& item : _parts)
            if (item.GetStart().x <= x && item.GetEnd().x >= x)
                return &item;

        return nullptr;
    }
} // namespace SparkyStudios::Audio::Amplitude
