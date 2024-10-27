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

#ifndef _AM_IMPLEMENTATION_MIXER_SOUND_DATA_H
#define _AM_IMPLEMENTATION_MIXER_SOUND_DATA_H

#include <SparkyStudios/Audio/Amplitude/Core/AudioBuffer.h>
#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>

#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    class SoundInstance;

    struct SoundChunk
    {
        AmUInt64 length;
        AmUInt64 frames;
        AmSize size;

        AudioBuffer* buffer;

        eMemoryPoolKind memoryPool;

        static SoundChunk* CreateChunk(AmUInt64 frames, AmUInt16 channels, eMemoryPoolKind pool = eMemoryPoolKind_SoundData);
        static void DestroyChunk(SoundChunk* chunk);

        ~SoundChunk();
    };

    struct SoundData
    {
        SoundData();

        SoundChunk* chunk;
        AmUInt64 length;
        AmUniquePtr<eMemoryPoolKind_Amplimix, SoundInstance> sound;
        SoundFormat format;
        bool stream;

        static SoundData* CreateMusic(const SoundFormat& format, SoundChunk* chunk, AmUInt64 frames, SoundInstance* soundInstance);
        static SoundData* CreateSound(const SoundFormat& format, SoundChunk* chunk, AmUInt64 frames, SoundInstance* soundInstance);

        static void Destroy(SoundData* soundData, bool destroyChunk = true);
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_MIXER_SOUND_DATA_H
