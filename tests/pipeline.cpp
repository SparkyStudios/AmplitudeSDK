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

#include <Mixer/Amplimix.h>
#include <Mixer/Pipeline.h>

#include <Mixer/Nodes/AmbisonicBinauralDecoderNode.h>
#include <Mixer/Nodes/AmbisonicMixerNode.h>
#include <Mixer/Nodes/AmbisonicPanningNode.h>
#include <Mixer/Nodes/AmbisonicRotatorNode.h>
#include <Mixer/Nodes/AttenuationNode.h>
#include <Mixer/Nodes/ClampNode.h>
#include <Mixer/Nodes/ClipNode.h>
#include <Mixer/Nodes/EnvironmentEffectNode.h>
#include <Mixer/Nodes/InputNode.h>
#include <Mixer/Nodes/NearFieldEffectNode.h>
#include <Mixer/Nodes/ObstructionNode.h>
#include <Mixer/Nodes/OcclusionNode.h>
#include <Mixer/Nodes/OutputNode.h>
#include <Mixer/Nodes/ReflectionsNode.h>
#include <Mixer/Nodes/ReverbNode.h>
#include <Mixer/Nodes/StereoMixerNode.h>
#include <Mixer/Nodes/StereoPanningNode.h>

using namespace SparkyStudios::Audio::Amplitude;

TEST_CASE("Pipeline Tests", "[pipeline][mixer][amplitude]")
{
    DiskFileSystem fileSystem;
    fileSystem.SetBasePath(AM_OS_STRING("./samples/assets"));

    AmplimixLayerImpl layer;

    THEN("can create instances")
    {
        PipelineImpl pipeline;
        pipeline.LoadDefinitionFromFile(
            fileSystem.OpenFile(fileSystem.Join({ AM_OS_STRING("pipelines"), AM_OS_STRING("stereo.ampipeline") }), eFileOpenMode_Read),
            nullptr);

        PipelineInstance* instance = pipeline.CreateInstance(&layer);
        REQUIRE_FALSE(instance == nullptr);

        pipeline.DestroyInstance(instance);
    }

    THEN("can create instances with duplicate producers")
    {
        PipelineImpl pipeline;
        pipeline.LoadDefinitionFromFile(
            fileSystem.OpenFile(
                fileSystem.Join({ AM_OS_STRING("pipelines"), AM_OS_STRING("tests.double_consume.ampipeline") }), eFileOpenMode_Read),
            nullptr);

        PipelineInstance* instance = pipeline.CreateInstance(&layer);
        REQUIRE_FALSE(instance == nullptr);

        pipeline.DestroyInstance(instance);
    }

    THEN("cannot create instances with self consuming node")
    {
        PipelineImpl pipeline;
        pipeline.LoadDefinitionFromFile(
            fileSystem.OpenFile(
                fileSystem.Join({ AM_OS_STRING("pipelines"), AM_OS_STRING("tests.invalid.self_consume.ampipeline") }), eFileOpenMode_Read),
            nullptr);

        PipelineInstance* instance = pipeline.CreateInstance(&layer);
        REQUIRE(instance == nullptr);
    }

    THEN("cannot create instances with missing input node")
    {
        PipelineImpl pipeline;
        pipeline.LoadDefinitionFromFile(
            fileSystem.OpenFile(
                fileSystem.Join({ AM_OS_STRING("pipelines"), AM_OS_STRING("tests.invalid.missing_input.ampipeline") }), eFileOpenMode_Read),
            nullptr);

        PipelineInstance* instance = pipeline.CreateInstance(&layer);
        REQUIRE(instance == nullptr);
    }

    THEN("cannot create instances with missing output node")
    {
        PipelineImpl pipeline;
        pipeline.LoadDefinitionFromFile(
            fileSystem.OpenFile(
                fileSystem.Join({ AM_OS_STRING("pipelines"), AM_OS_STRING("tests.invalid.missing_output.ampipeline") }),
                eFileOpenMode_Read),
            nullptr);

        PipelineInstance* instance = pipeline.CreateInstance(&layer);
        REQUIRE(instance == nullptr);
    }

    THEN("cannot create instances with multiple input nodes")
    {
        PipelineImpl pipeline;
        pipeline.LoadDefinitionFromFile(
            fileSystem.OpenFile(
                fileSystem.Join({ AM_OS_STRING("pipelines"), AM_OS_STRING("tests.invalid.multiple_input.ampipeline") }),
                eFileOpenMode_Read),
            nullptr);

        PipelineInstance* instance = pipeline.CreateInstance(&layer);
        REQUIRE(instance == nullptr);
    }

    THEN("cannot create instances with multiple output nodes")
    {
        PipelineImpl pipeline;
        pipeline.LoadDefinitionFromFile(
            fileSystem.OpenFile(
                fileSystem.Join({ AM_OS_STRING("pipelines"), AM_OS_STRING("tests.invalid.multiple_output.ampipeline") }),
                eFileOpenMode_Read),
            nullptr);

        PipelineInstance* instance = pipeline.CreateInstance(&layer);
        REQUIRE(instance == nullptr);
    }

    THEN("cannot create instances with missing connections")
    {
        PipelineImpl pipeline;
        pipeline.LoadDefinitionFromFile(
            fileSystem.OpenFile(
                fileSystem.Join({ AM_OS_STRING("pipelines"), AM_OS_STRING("tests.invalid.missing_producers.ampipeline") }),
                eFileOpenMode_Read),
            nullptr);

        PipelineInstance* instance = pipeline.CreateInstance(&layer);
        REQUIRE(instance == nullptr);
    }

    THEN("cannot create instances with invalid connections count")
    {
        PipelineImpl pipeline;
        pipeline.LoadDefinitionFromFile(
            fileSystem.OpenFile(
                fileSystem.Join({ AM_OS_STRING("pipelines"), AM_OS_STRING("tests.invalid.invalid_producers_count.ampipeline") }),
                eFileOpenMode_Read),
            nullptr);

        PipelineInstance* instance = pipeline.CreateInstance(&layer);
        REQUIRE(instance == nullptr);
    }

    THEN("cannot create instances with unregistered node")
    {
        PipelineImpl pipeline;
        pipeline.LoadDefinitionFromFile(
            fileSystem.OpenFile(
                fileSystem.Join({ AM_OS_STRING("pipelines"), AM_OS_STRING("tests.invalid.unknown_node.ampipeline") }), eFileOpenMode_Read),
            nullptr);

        PipelineInstance* instance = pipeline.CreateInstance(&layer);
        REQUIRE(instance == nullptr);
    }

    THEN("cannot create instances with invalid consumer node")
    {
        PipelineImpl pipeline;
        pipeline.LoadDefinitionFromFile(
            fileSystem.OpenFile(
                fileSystem.Join({ AM_OS_STRING("pipelines"), AM_OS_STRING("tests.invalid.invalid_consumer_node.ampipeline") }),
                eFileOpenMode_Read),
            nullptr);

        PipelineInstance* instance = pipeline.CreateInstance(&layer);
        REQUIRE(instance == nullptr);
    }
}

TEST_CASE("AmbisonicBinauralDecoderNode Tests", "[ambisonic_binaural_decoder][nodes][mixer][amplitude]")
{
    AmbisonicBinauralDecoderNode node;

    SECTION("test can consume")
    {
        REQUIRE(node.CanConsume());
    }

    SECTION("test can produce")
    {
        REQUIRE(node.CanProduce());
    }

    SECTION("test get max input count")
    {
        REQUIRE(node.GetMaxInputCount() == 1);
    }

    SECTION("test get min input count")
    {
        REQUIRE(node.GetMinInputCount() == 1);
    }
}

TEST_CASE("AmbisonicMixerNode Tests", "[ambisonic_mixer][nodes][mixer][amplitude]")
{
    AmbisonicMixerNode node;

    SECTION("test can consume")
    {
        REQUIRE(node.CanConsume());
    }

    SECTION("test can produce")
    {
        REQUIRE(node.CanProduce());
    }

    SECTION("test get max input count")
    {
        REQUIRE(node.GetMaxInputCount() == static_cast<AmSize>(-1));
    }

    SECTION("test get min input count")
    {
        REQUIRE(node.GetMinInputCount() == 1);
    }
}

TEST_CASE("AmbisonicPanningNode Tests", "[ambisonic_panning][nodes][mixer][amplitude]")
{
    AmbisonicPanningNode node;

    SECTION("test can consume")
    {
        REQUIRE(node.CanConsume());
    }

    SECTION("test can produce")
    {
        REQUIRE(node.CanProduce());
    }

    SECTION("test get max input count")
    {
        REQUIRE(node.GetMaxInputCount() == 1);
    }

    SECTION("test get min input count")
    {
        REQUIRE(node.GetMinInputCount() == 1);
    }
}

TEST_CASE("AmbisonicRotatorNode Tests", "[ambisonic_rotator][nodes][mixer][amplitude]")
{
    AmbisonicRotatorNode node;

    SECTION("test can consume")
    {
        REQUIRE(node.CanConsume());
    }

    SECTION("test can produce")
    {
        REQUIRE(node.CanProduce());
    }

    SECTION("test get max input count")
    {
        REQUIRE(node.GetMaxInputCount() == 1);
    }

    SECTION("test get min input count")
    {
        REQUIRE(node.GetMinInputCount() == 1);
    }
}

TEST_CASE("AttenuationNode Tests", "[attenuation][nodes][mixer][amplitude]")
{
    AttenuationNode node;

    SECTION("test can consume")
    {
        REQUIRE(node.CanConsume());
    }

    SECTION("test can produce")
    {
        REQUIRE(node.CanProduce());
    }

    SECTION("test get max input count")
    {
        REQUIRE(node.GetMaxInputCount() == 1);
    }

    SECTION("test get min input count")
    {
        REQUIRE(node.GetMinInputCount() == 1);
    }
}

TEST_CASE("ClampNode Tests", "[clamp][nodes][mixer][amplitude]")
{
    ClampNode node;

    SECTION("test can consume")
    {
        REQUIRE(node.CanConsume());
    }

    SECTION("test can produce")
    {
        REQUIRE(node.CanProduce());
    }

    SECTION("test get max input count")
    {
        REQUIRE(node.GetMaxInputCount() == 1);
    }

    SECTION("test get min input count")
    {
        REQUIRE(node.GetMinInputCount() == 1);
    }
}

TEST_CASE("ClipNode Tests", "[clip][nodes][mixer][amplitude]")
{
    ClipNode node;

    SECTION("test can consume")
    {
        REQUIRE(node.CanConsume());
    }

    SECTION("test can produce")
    {
        REQUIRE(node.CanProduce());
    }

    SECTION("test get max input count")
    {
        REQUIRE(node.GetMaxInputCount() == 1);
    }

    SECTION("test get min input count")
    {
        REQUIRE(node.GetMinInputCount() == 1);
    }
}

TEST_CASE("EnvironmentEffectNode Tests", "[environment_effect][nodes][mixer][amplitude]")
{
    EnvironmentEffectNode node;

    SECTION("test can consume")
    {
        REQUIRE(node.CanConsume());
    }

    SECTION("test can produce")
    {
        REQUIRE(node.CanProduce());
    }

    SECTION("test get max input count")
    {
        REQUIRE(node.GetMaxInputCount() == 1);
    }

    SECTION("test get min input count")
    {
        REQUIRE(node.GetMinInputCount() == 1);
    }
}

TEST_CASE("InputNode Tests", "[input][nodes][mixer][amplitude]")
{
    InputNode node;

    SECTION("test can consume")
    {
        REQUIRE_FALSE(node.CanConsume());
    }

    SECTION("test can produce")
    {
        REQUIRE(node.CanProduce());
    }

    SECTION("test get max input count")
    {
        REQUIRE(node.GetMaxInputCount() == 0);
    }

    SECTION("test get min input count")
    {
        REQUIRE(node.GetMinInputCount() == 0);
    }
}

TEST_CASE("NearFieldEffectNode Tests", "[near_field_effect][nodes][mixer][amplitude]")
{
    NearFieldEffectNode node;

    SECTION("test can consume")
    {
        REQUIRE(node.CanConsume());
    }

    SECTION("test can produce")
    {
        REQUIRE(node.CanProduce());
    }

    SECTION("test get max input count")
    {
        REQUIRE(node.GetMaxInputCount() == 1);
    }

    SECTION("test get min input count")
    {
        REQUIRE(node.GetMinInputCount() == 1);
    }
}

TEST_CASE("ObstructionNode Tests", "[obstruction][nodes][mixer][amplitude]")
{
    ObstructionNode node;

    SECTION("test can consume")
    {
        REQUIRE(node.CanConsume());
    }

    SECTION("test can produce")
    {
        REQUIRE(node.CanProduce());
    }

    SECTION("test get max input count")
    {
        REQUIRE(node.GetMaxInputCount() == 1);
    }

    SECTION("test get min input count")
    {
        REQUIRE(node.GetMinInputCount() == 1);
    }
}

TEST_CASE("OcclusionNode Tests", "[occlusion][nodes][mixer][amplitude]")
{
    OcclusionNode node;

    SECTION("test can consume")
    {
        REQUIRE(node.CanConsume());
    }

    SECTION("test can produce")
    {
        REQUIRE(node.CanProduce());
    }

    SECTION("test get max input count")
    {
        REQUIRE(node.GetMaxInputCount() == 1);
    }

    SECTION("test get min input count")
    {
        REQUIRE(node.GetMinInputCount() == 1);
    }
}

TEST_CASE("OutputNode Tests", "[output][nodes][mixer][amplitude]")
{
    OutputNode node;

    SECTION("test can consume")
    {
        REQUIRE(node.CanConsume());
    }

    SECTION("test can produce")
    {
        REQUIRE_FALSE(node.CanProduce());
    }

    SECTION("test get max input count")
    {
        REQUIRE(node.GetMaxInputCount() == 1);
    }

    SECTION("test get min input count")
    {
        REQUIRE(node.GetMinInputCount() == 1);
    }
}

TEST_CASE("ReflectionsNode Tests", "[reflections][nodes][mixer][amplitude]")
{
    ReflectionsNode node;

    SECTION("test can consume")
    {
        REQUIRE(node.CanConsume());
    }

    SECTION("test can produce")
    {
        REQUIRE(node.CanProduce());
    }

    SECTION("test get max input count")
    {
        REQUIRE(node.GetMaxInputCount() == 1);
    }

    SECTION("test get min input count")
    {
        REQUIRE(node.GetMinInputCount() == 1);
    }
}

TEST_CASE("ReverbNode Tests", "[reverb][nodes][mixer][amplitude]")
{
    ReverbNode node;

    SECTION("test can consume")
    {
        REQUIRE(node.CanConsume());
    }

    SECTION("test can produce")
    {
        REQUIRE(node.CanProduce());
    }

    SECTION("test get max input count")
    {
        REQUIRE(node.GetMaxInputCount() == 1);
    }

    SECTION("test get min input count")
    {
        REQUIRE(node.GetMinInputCount() == 1);
    }
}

TEST_CASE("StereoMixerNode Tests", "[stereo_mixer][nodes][mixer][amplitude]")
{
    StereoMixerNode node;

    SECTION("test can consume")
    {
        REQUIRE(node.CanConsume());
    }

    SECTION("test can produce")
    {
        REQUIRE(node.CanProduce());
    }

    SECTION("test get max input count")
    {
        REQUIRE(node.GetMaxInputCount() == static_cast<AmSize>(-1));
    }

    SECTION("test get min input count")
    {
        REQUIRE(node.GetMinInputCount() == 1);
    }
}

TEST_CASE("StereoPanningNode Tests", "[stereo_panning][nodes][mixer][amplitude]")
{
    StereoPanningNode node;

    SECTION("test can consume")
    {
        REQUIRE(node.CanConsume());
    }

    SECTION("test can produce")
    {
        REQUIRE(node.CanProduce());
    }

    SECTION("test get max input count")
    {
        REQUIRE(node.GetMaxInputCount() == 1);
    }

    SECTION("test get min input count")
    {
        REQUIRE(node.GetMinInputCount() == 1);
    }
}
