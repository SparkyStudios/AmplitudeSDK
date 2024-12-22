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

#ifndef SS_AMPLITUDE_MINIAUDIO_UTILS_H
#define SS_AMPLITUDE_MINIAUDIO_UTILS_H

#include <SparkyStudios/Audio/Amplitude/Core/Device.h>

#define MA_NO_DECODING
#define MA_NO_ENCODING
#define MA_NO_RESOURCE_MANAGER
#define MA_NO_NODE_GRAPH
#define MA_NO_ENGINE
#define MA_USE_STDINT

#if !defined(NOMINMAX)
#define NOMINMAX
#endif

#if defined(AM_SIMD_ARCH_AVX2)
#define MA_SUPPORT_AVX2
#define MA_PREFER_AVX2
#elif defined(AM_SIMD_ARCH_AVX)
#define MA_SUPPORT_AVX
#define MA_PREFER_AVX
#elif defined(AM_SIMD_ARCH_SSE2)
#define MA_SUPPORT_SSE2
#define MA_PREFER_SSE2
#elif defined(AM_SIMD_ARCH_NEON)
#define MA_SUPPORT_NEON
#define MA_PREFER_NEON
#endif

#include <miniaudio.h>

ma_format ma_format_from_amplitude(SparkyStudios::Audio::Amplitude::PlaybackOutputFormat format);

#endif /* SS_AMPLITUDE_MINIAUDIO_UTILS_H */
