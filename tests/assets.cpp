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

#include <catch2/catch_test_macros.hpp>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Sound/Attenuation.h>
#include <Sound/AttenuationShapes.h>

using namespace SparkyStudios::Audio::Amplitude;

TEST_CASE("Attenuation Asset Tests", "[attenuation][assets][amplitude]")
{
    DiskFileSystem fs;
    fs.SetBasePath(AM_OS_STRING("./samples/assets"));

    AttenuationImpl attenuation;
    attenuation.LoadDefinitionFromFile(fs.OpenFile(AM_OS_STRING("attenuators/impact.amattenuation"), eFileOpenMode_Read), nullptr);

    REQUIRE(attenuation.GetDefinition() != nullptr);
    REQUIRE(attenuation.GetId() == 1);
    REQUIRE(attenuation.GetName() == "impact");
    REQUIRE(attenuation.GetMaxDistance() == 1280);
    REQUIRE_FALSE(attenuation.IsAirAbsorptionEnabled());
    REQUIRE(dynamic_cast<SphereAttenuationZone*>(attenuation.GetShape()) != nullptr);
}