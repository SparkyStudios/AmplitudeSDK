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

#include <HRTF/HRIRSphere.h>

using namespace SparkyStudios::Audio::Amplitude;

TEST_CASE("HRTF Sphere Tests", "[hrtf_sphere][hrtf][amplitude]")
{
    HRIRSphereImpl sphere;
    REQUIRE_FALSE(sphere.IsLoaded());

    DiskFileSystem fs;
    fs.SetBasePath(AM_OS_STRING("./samples/assets"));

    sphere.SetResource(AM_OS_STRING("./data/baker_a_101_lp_512.amir"));
    REQUIRE_FALSE(sphere.IsLoaded());

    sphere.SetResource(AM_OS_STRING("./data/throw_01.ogg"));
    REQUIRE_FALSE(sphere.IsLoaded());

    sphere.SetResource(AM_OS_STRING("./data/sadie_h12.amir"));
    sphere.Load(&fs);

    REQUIRE(sphere.IsLoaded());
    REQUIRE(sphere.GetPath() == AM_OS_STRING("./data/sadie_h12.amir"));

    REQUIRE(sphere.GetVertices().size() == sphere.GetVertexCount());
    REQUIRE(sphere.GetVertexCount() == 2114);

    REQUIRE(sphere.GetFaces().size() == sphere.GetFaceCount());
    REQUIRE(sphere.GetFaceCount() == 4224);

    REQUIRE(sphere.GetIRLength() == 256);
    REQUIRE(sphere.GetSampleRate() == 48000);

    HRIRSphereVertex vertex = sphere.GetVertex(0);

    AmAlignedReal32Buffer l, r;
    l.Init(256);
    r.Init(256);

    AmVec3 direction = AM_V3(0.0f, 0.0f, -1.2f);

    sphere.SetSamplingMode(eHRIRSphereSamplingMode_Bilinear);
    REQUIRE(sphere.GetSamplingMode() == eHRIRSphereSamplingMode_Bilinear);
    sphere.Sample(direction, l.GetBuffer(), r.GetBuffer());
    REQUIRE(std::memcmp(l.GetBuffer(), vertex.m_LeftIR.data(), 256 * sizeof(AmReal32)) == 0);
    REQUIRE(std::memcmp(r.GetBuffer(), vertex.m_RightIR.data(), 256 * sizeof(AmReal32)) == 0);

    l.Clear();
    r.Clear();

    sphere.SetSamplingMode(eHRIRSphereSamplingMode_NearestNeighbor);
    REQUIRE(sphere.GetSamplingMode() == eHRIRSphereSamplingMode_NearestNeighbor);
    sphere.Sample(direction, l.GetBuffer(), r.GetBuffer());
    REQUIRE(std::memcmp(l.GetBuffer(), vertex.m_LeftIR.data(), 256 * sizeof(AmReal32)) == 0);
    REQUIRE(std::memcmp(r.GetBuffer(), vertex.m_RightIR.data(), 256 * sizeof(AmReal32)) == 0);

    direction.Z = -1.0f;

    sphere.Sample(direction, l.GetBuffer(), r.GetBuffer());
    REQUIRE(std::memcmp(l.GetBuffer(), vertex.m_LeftIR.data(), 256 * sizeof(AmReal32)) == 0);
    REQUIRE(std::memcmp(r.GetBuffer(), vertex.m_RightIR.data(), 256 * sizeof(AmReal32)) == 0);

    AmMat4 rotation = AM_Rotate_RH(90.0f * AM_DegToRad, AM_V3(0, 0, 1));
    sphere.Transform(rotation);

    HRIRSphereVertex transformedVertex = sphere.GetVertex(0);
    REQUIRE(transformedVertex.m_Position == AM_Mul(rotation, AM_V4V(vertex.m_Position, 1.0f)).XYZ);
}