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

#include <catch2/catch_test_macros.hpp>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

using namespace SparkyStudios::Audio::Amplitude;

TEST_CASE("Constant Fader Tests", "[constant_fader][faders][sound][amplitude]")
{
    Fader* fader = Fader::Find("Constant");
    constexpr BezierCurveControlPoints cp1 = { 0.0f, 0.0f, 0.0f, 0.0f };
    const BezierCurveControlPoints cp2 = fader->GetControlPoints();
    REQUIRE((cp1.x1 == cp2.x1 && cp1.y1 == cp2.y1 && cp1.x2 == cp2.x2 && cp1.y2 == cp2.y2));

    FaderInstance* instance = Fader::Construct("Constant");

    instance->Set(0.0, 1.0, kAmSecond);

    REQUIRE(instance->GetFromPercentage(0.00) == 0.0);
    REQUIRE(instance->GetFromPercentage(0.25) == 0.0);
    REQUIRE(instance->GetFromPercentage(0.50) == 0.0);
    REQUIRE(instance->GetFromPercentage(0.75) == 0.0);
    REQUIRE(instance->GetFromPercentage(1.00) == 1.0);

    Fader::Destruct("Constant", instance);
}

TEST_CASE("Ease Fader Tests", "[ease_fader][faders][sound][amplitude]")
{
    Fader* fader = Fader::Find("Ease");
    constexpr BezierCurveControlPoints cp1 = { 0.25f, 0.1f, 0.25f, 1.0f };
    const BezierCurveControlPoints cp2 = fader->GetControlPoints();
    REQUIRE((cp1.x1 == cp2.x1 && cp1.y1 == cp2.y1 && cp1.x2 == cp2.x2 && cp1.y2 == cp2.y2));

    FaderInstance* instance = Fader::Construct("Ease");

    instance->Set(0.0, 1.0, kAmSecond);

    REQUIRE(instance->GetFromPercentage(0.00) == 0.0);
    REQUIRE(instance->GetFromPercentage(0.25) - 0.40851059199373591 < kEpsilon);
    REQUIRE(instance->GetFromPercentage(0.50) - 0.80240338786711973 < kEpsilon);
    REQUIRE(instance->GetFromPercentage(0.75) - 0.96045897841111938 < kEpsilon);
    REQUIRE(instance->GetFromPercentage(1.00) == 1.0);

    Fader::Destruct("Ease", instance);
}

TEST_CASE("EaseIn Fader Tests", "[easein_fader][faders][sound][amplitude]")
{
    Fader* fader = Fader::Find("EaseIn");
    constexpr BezierCurveControlPoints cp1 = { 0.42f, 0.0f, 1.0f, 1.0f };
    const BezierCurveControlPoints cp2 = fader->GetControlPoints();
    REQUIRE((cp1.x1 == cp2.x1 && cp1.y1 == cp2.y1 && cp1.x2 == cp2.x2 && cp1.y2 == cp2.y2));

    FaderInstance* instance = Fader::Construct("EaseIn");

    instance->Set(0.0, 1.0, kAmSecond);

    REQUIRE(instance->GetFromPercentage(0.00) == 0.0);
    REQUIRE(instance->GetFromPercentage(0.25) - 0.09346465401576336 < kEpsilon);
    REQUIRE(instance->GetFromPercentage(0.50) - 0.31535681876384836 < kEpsilon);
    REQUIRE(instance->GetFromPercentage(0.75) - 0.62186187464895193 < kEpsilon);
    REQUIRE(instance->GetFromPercentage(1.00) == 1.0);

    Fader::Destruct("EaseIn", instance);
}

TEST_CASE("EaseInOut Fader Tests", "[easeinout_fader][faders][sound][amplitude]")
{
    Fader* fader = Fader::Find("EaseInOut");
    constexpr BezierCurveControlPoints cp1 = { 0.42f, 0.0f, 0.58f, 1.0f };
    const BezierCurveControlPoints cp2 = fader->GetControlPoints();
    REQUIRE((cp1.x1 == cp2.x1 && cp1.y1 == cp2.y1 && cp1.x2 == cp2.x2 && cp1.y2 == cp2.y2));

    FaderInstance* instance = Fader::Construct("EaseInOut");

    instance->Set(0.0, 1.0, kAmSecond);

    REQUIRE(instance->GetFromPercentage(0.00) == 0.0);
    REQUIRE(instance->GetFromPercentage(0.25) - 0.12916193876118984 < kEpsilon);
    REQUIRE(instance->GetFromPercentage(0.50) - 0.50000001926874293 < kEpsilon);
    REQUIRE(instance->GetFromPercentage(0.75) - 0.87083807775645594 < kEpsilon);
    REQUIRE(instance->GetFromPercentage(1.00) == 1.0);

    Fader::Destruct("EaseInOut", instance);
}

TEST_CASE("EaseOut Fader Tests", "[easeout_fader][faders][sound][amplitude]")
{
    Fader* fader = Fader::Find("EaseOut");
    constexpr BezierCurveControlPoints cp1 = { 0.0f, 0.0f, 0.58f, 1.0f };
    const BezierCurveControlPoints cp2 = fader->GetControlPoints();
    REQUIRE((cp1.x1 == cp2.x1 && cp1.y1 == cp2.y1 && cp1.x2 == cp2.x2 && cp1.y2 == cp2.y2));

    FaderInstance* instance = Fader::Construct("EaseOut");

    instance->Set(0.0, 1.0, kAmSecond);

    REQUIRE(instance->GetFromPercentage(0.00) == 0.0);
    REQUIRE(instance->GetFromPercentage(0.25) - 0.37813813779209771 < kEpsilon);
    REQUIRE(instance->GetFromPercentage(0.50) - 0.68464319530730855 < kEpsilon);
    REQUIRE(instance->GetFromPercentage(0.75) - 0.90653535347727843 < kEpsilon);
    REQUIRE(instance->GetFromPercentage(1.00) == 1.0);

    Fader::Destruct("EaseOut", instance);
}

TEST_CASE("Exponential Fader Tests", "[exponential_fader][faders][sound][amplitude]")
{
    Fader* fader = Fader::Find("Exponential");
    constexpr BezierCurveControlPoints cp1 = { 1.0f, 0.0f, 1.0f, 1.0f };
    const BezierCurveControlPoints cp2 = fader->GetControlPoints();
    REQUIRE((cp1.x1 == cp2.x1 && cp1.y1 == cp2.y1 && cp1.x2 == cp2.x2 && cp1.y2 == cp2.y2));

    FaderInstance* instance = Fader::Construct("Exponential");

    instance->Set(0.0, 1.0, kAmSecond);

    REQUIRE(instance->GetFromPercentage(0.00) == 0.0);
    REQUIRE(instance->GetFromPercentage(0.25) - 0.37813813779209771 < kEpsilon);
    REQUIRE(instance->GetFromPercentage(0.50) - 0.68464319530730855 < kEpsilon);
    REQUIRE(instance->GetFromPercentage(0.75) - 0.90653535347727843 < kEpsilon);
    REQUIRE(instance->GetFromPercentage(1.00) == 1.0);

    Fader::Destruct("Exponential", instance);
}

TEST_CASE("Linear Fader Tests", "[linear_fader][faders][sound][amplitude]")
{
    Fader* fader = Fader::Find("Linear");
    constexpr BezierCurveControlPoints cp1 = { 0.0f, 0.0f, 1.0f, 1.0f };
    const BezierCurveControlPoints cp2 = fader->GetControlPoints();
    REQUIRE((cp1.x1 == cp2.x1 && cp1.y1 == cp2.y1 && cp1.x2 == cp2.x2 && cp1.y2 == cp2.y2));

    FaderInstance* instance = Fader::Construct("Linear");

    instance->Set(0.0, 1.0, kAmSecond);

    REQUIRE(instance->GetFromPercentage(0.00) == 0.0);
    REQUIRE(instance->GetFromPercentage(0.25) == 0.25);
    REQUIRE(instance->GetFromPercentage(0.50) == 0.50);
    REQUIRE(instance->GetFromPercentage(0.75) == 0.75);
    REQUIRE(instance->GetFromPercentage(1.00) == 1.0);

    Fader::Destruct("Linear", instance);
}

TEST_CASE("SCurveSharp Fader Tests", "[scurvesharp_fader][faders][sound][amplitude]")
{
    Fader* fader = Fader::Find("SCurveSharp");
    constexpr BezierCurveControlPoints cp1 = { 0.9f, 0.0f, 0.1f, 1.0f };
    const BezierCurveControlPoints cp2 = fader->GetControlPoints();
    REQUIRE((cp1.x1 == cp2.x1 && cp1.y1 == cp2.y1 && cp1.x2 == cp2.x2 && cp1.y2 == cp2.y2));

    FaderInstance* instance = Fader::Construct("SCurveSharp");

    instance->Set(0.0, 1.0, kAmSecond);

    REQUIRE(instance->GetFromPercentage(0.00) == 0.0);
    REQUIRE(instance->GetFromPercentage(0.25) - 0.03729032857535650 < kEpsilon);
    REQUIRE(instance->GetFromPercentage(0.50) - 0.50000008381902017 < kEpsilon);
    REQUIRE(instance->GetFromPercentage(0.75) - 0.96270967398599439 < kEpsilon);
    REQUIRE(instance->GetFromPercentage(1.00) == 1.0);

    Fader::Destruct("SCurveSharp", instance);
}

TEST_CASE("SCurveSmooth Fader Tests", "[scurvesmooth_fader][faders][sound][amplitude]")
{
    Fader* fader = Fader::Find("SCurveSmooth");
    constexpr BezierCurveControlPoints cp1 = { 0.64f, 0.0f, 0.36f, 1.0f };
    const BezierCurveControlPoints cp2 = fader->GetControlPoints();
    REQUIRE((cp1.x1 == cp2.x1 && cp1.y1 == cp2.y1 && cp1.x2 == cp2.x2 && cp1.y2 == cp2.y2));

    FaderInstance* instance = Fader::Construct("SCurveSmooth");

    instance->Set(0.0, 1.0, kAmSecond);

    REQUIRE(instance->GetFromPercentage(0.00) == 0.0);
    REQUIRE(instance->GetFromPercentage(0.25) - 0.07274458735701822 < kEpsilon);
    REQUIRE(instance->GetFromPercentage(0.50) - 0.50000008381902017 < kEpsilon);
    REQUIRE(instance->GetFromPercentage(0.75) - 0.92725541264298184 < kEpsilon);
    REQUIRE(instance->GetFromPercentage(1.00) == 1.0);

    Fader::Destruct("SCurveSmooth", instance);
}
