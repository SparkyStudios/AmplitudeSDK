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

#include <Core/AudioBufferCrossFader.h>
#include <Utils/Utils.h>

using namespace SparkyStudios::Audio::Amplitude;

TEST_CASE("AudioBuffer Tests", "[audio_buffer][core][amplitude]")
{
    SECTION("can be created")
    {
        AudioBuffer buffer1;
        REQUIRE(buffer1.IsEmpty());

        AudioBuffer buffer2(12345, 3);
        REQUIRE_FALSE(buffer2.IsEmpty());
        REQUIRE(buffer2.GetFrameCount() == 12345);
        REQUIRE(buffer2.GetChannelCount() == 3);

        for (AmSize i = 0, l = buffer2.GetChannelCount(); i < l; ++i)
            REQUIRE(buffer2[i].enabled());

        AudioBuffer buffer3(std::move(buffer2));
        REQUIRE_FALSE(buffer3.IsEmpty());
        REQUIRE(buffer3.GetFrameCount() == 12345);
        REQUIRE(buffer3.GetChannelCount() == 3);
        REQUIRE(buffer2.IsEmpty());

        for (AmSize i = 0, l = buffer3.GetChannelCount(); i < l; ++i)
            REQUIRE(buffer3[i].enabled());

        AudioBuffer buffer4;
        buffer4 = buffer3;
        REQUIRE_FALSE(buffer4.IsEmpty());
        REQUIRE(buffer4.GetFrameCount() == buffer3.GetFrameCount());
        REQUIRE(buffer4.GetChannelCount() == buffer3.GetChannelCount());

        for (AmSize i = 0, l = buffer4.GetChannelCount(); i < l; ++i)
            REQUIRE(buffer4[i].enabled());
    }

    SECTION("can make operations on channels")
    {
        AudioBuffer buffer1(123, 2);
        const AmSize alignedSize = FindNextAlignedArrayIndex<AmReal32>(123, AM_SIMD_ALIGNMENT);
        for (AmSize i = 0; i < 2; ++i)
            for (AmSize j = 0; j < 123; j++)
                buffer1.GetData().GetBuffer()[alignedSize * i + j] = (123.0f * i) + j;

        AudioBuffer buffer2(123, 2);
        for (AmSize i = 0; i < 2; ++i)
            for (AmSize j = 0; j < 123; ++j)
                buffer2[i][j] = (123.0f * i) + j;

        AudioBuffer buffer3(123, 2);
        for (AmSize i = 0; i < 2; ++i)
            for (AmSize j = 0; j < 123; ++j)
                REQUIRE(buffer3[i][j] == 0);

        buffer1 += buffer2;
        for (AmSize i = 0; i < 2; ++i)
            for (AmSize j = 0; j < 123; ++j)
                REQUIRE(buffer1[i][j] == ((123.0f * i) + j) * 2.0f);

        buffer2 -= buffer1;
        for (AmSize i = 0; i < 2; ++i)
            for (AmSize j = 0; j < 123; ++j)
                REQUIRE(buffer2[i][j] == ((123.0f * i) + j) * -1.0f);

        buffer1 *= buffer3;
        for (AmSize i = 0; i < 2; ++i)
            for (AmSize j = 0; j < 123; ++j)
                REQUIRE(buffer1[i][j] == 0);

        buffer2 *= -1;
        for (AmSize i = 0; i < 2; ++i)
            for (AmSize j = 0; j < 123; ++j)
                REQUIRE(buffer2[i][j] == (123.0f * i) + j);
    }

    SECTION("can be cloned and copied")
    {
        AudioBuffer buffer1(123, 1);
        for (AmSize i = 0; i < 123; ++i)
            buffer1[0][i] = i;

        AudioBuffer buffer2;
        buffer2 = buffer1.Clone();
        for (AmSize i = 0; i < 123; ++i)
            REQUIRE(buffer2[0][i] == buffer1[0][i]);

        AudioBuffer buffer3(23, 1);
        AudioBuffer::Copy(buffer2, 100, buffer3, 0, 23);
        for (AmSize i = 0; i < 23; ++i)
            REQUIRE(buffer3[0][i] == buffer2[0][100 + i]);

        AudioBuffer buffer4(123, 1);
        const std::vector<AmReal32> data(123, 1.0f);
        buffer4[0] = data;
        for (AmSize i = 0; i < 123; ++i)
            REQUIRE(buffer4[0][i] == 1.0f);
    }
}

TEST_CASE("AudioBufferCrossFader Tests", "[audio_buffer_cross_fader][core][amplitude]")
{
    AudioBuffer in(10, 1);
    AudioBuffer out(10, 1);

    for (size_t i = 0; i < 10; ++i)
    {
        in[0][i] = 1.0f;
        out[0][i] = 1.0f;
    }

    AudioBuffer fade(10, 1);

    AudioBufferCrossFader crossfader(10);
    crossfader.CrossFade(in, out, fade);

    for (size_t i = 0; i < 10; ++i)
        REQUIRE(std::abs(1.0f - fade[0][i]) < kEpsilon);
}