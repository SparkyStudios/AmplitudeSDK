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

#ifndef _AM_CORE_COMMON_TYPES_H
#define _AM_CORE_COMMON_TYPES_H

#include <SparkyStudios/Audio/Amplitude/Core/Common/Config.h>

#if defined(AM_WINDOWS_VERSION)
#include <SparkyStudios/Audio/Amplitude/Core/Common/Platforms/Windows/Types.h>
#elif defined(AM_ANDROID_VERSION)
#include <SparkyStudios/Audio/Amplitude/Core/Common/Platforms/Android/Types.h>
#elif defined(AM_LINUX_VERSION)
#include <SparkyStudios/Audio/Amplitude/Core/Common/Platforms/Linux/Types.h>
#elif defined(AM_OSX_VERSION) || defined(AM_IOS_VERSION)
#include <SparkyStudios/Audio/Amplitude/Core/Common/Platforms/Apple/Types.h>
#endif

namespace SparkyStudios::Audio::Amplitude
{
    // clang-format off

    typedef AmUInt64            AmObjectID;

    typedef AmObjectID          AmBankID;
    typedef AmObjectID          AmBusID;
    typedef AmObjectID          AmSwitchContainerID;
    typedef AmObjectID          AmCollectionID;
    typedef AmObjectID          AmSoundID;
    typedef AmObjectID          AmEntityID;
    typedef AmObjectID          AmListenerID;
    typedef AmObjectID          AmChannelID;
    typedef AmObjectID          AmEventID;
    typedef AmObjectID          AmPipelineID;
    typedef AmObjectID          AmAttenuationID;
    typedef AmObjectID          AmSwitchID;
    typedef AmObjectID          AmRtpcID;
    typedef AmObjectID          AmEffectID;
    typedef AmObjectID          AmEnvironmentID;
    typedef AmObjectID          AmRoomID;

    typedef AmUInt8             AmResult;
    typedef AmUInt64            AmHandle;
    typedef AmReal64            AmTime;

    typedef AmReal32            AmAudioSample;
    typedef AmReal32*           AmAudioSampleBuffer;
    typedef const AmReal32*     AmConstAudioSampleBuffer;

    // clang-format on
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_CORE_COMMON_TYPES_H
