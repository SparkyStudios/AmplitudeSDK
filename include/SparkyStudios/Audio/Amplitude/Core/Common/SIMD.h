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

#ifndef _AM_CORE_COMMON_SIMD_H
#define _AM_CORE_COMMON_SIMD_H

#if defined(AM_SIMD_INTRINSICS)

#if defined(__AVX2__)
#if !defined(AM_BUILDSYSTEM_ARCH_X86_AVX2)
#define AM_BUILDSYSTEM_ARCH_X86_AVX2
#endif
#define AM_SIMD_ARCH_AVX2
#elif defined(__AVX__)
#if !defined(AM_BUILDSYSTEM_ARCH_X86_AVX)
#define AM_BUILDSYSTEM_ARCH_X86_AVX
#endif
#define AM_SIMD_ARCH_AVX
#endif

#if defined(__SSE4_2__)
#if !defined(AM_BUILDSYSTEM_ARCH_X86_SSE4_2)
#define AM_BUILDSYSTEM_ARCH_X86_SSE4_2
#endif
#define AM_SIMD_ARCH_SSE4_2
#define AM_SIMD_ARCH_SSE4_1
#define AM_SIMD_ARCH_SSSE3
#define AM_SIMD_ARCH_SSE3
#define AM_SIMD_ARCH_SSE2
#elif defined(__SSE4_1__)
#if !defined(AM_BUILDSYSTEM_ARCH_X86_SSE4_1)
#define AM_BUILDSYSTEM_ARCH_X86_SSE4_1
#endif
#define AM_SIMD_ARCH_SSE4_1
#define AM_SIMD_ARCH_SSSE3
#define AM_SIMD_ARCH_SSE3
#define AM_SIMD_ARCH_SSE2
#elif defined(__SSSE3__)
#if !defined(AM_BUILDSYSTEM_ARCH_X86_SSSE3)
#define AM_BUILDSYSTEM_ARCH_X86_SSSE3
#endif
#define AM_SIMD_ARCH_SSSE3
#define AM_SIMD_ARCH_SSE3
#define AM_SIMD_ARCH_SSE2
#elif defined(__SSE3__)
#if !defined(AM_BUILDSYSTEM_ARCH_X86_SSE3)
#define AM_BUILDSYSTEM_ARCH_X86_SSE3
#endif
#define AM_SIMD_ARCH_SSE3
#define AM_SIMD_ARCH_SSE2
#elif defined(__SSE2__)
#if !defined(AM_BUILDSYSTEM_ARCH_X86_SSE2)
#define AM_BUILDSYSTEM_ARCH_X86_SSE2
#endif
#define AM_SIMD_ARCH_SSE2
#endif

#if defined(__FMA__)
#if !defined(AM_BUILDSYSTEM_ARCH_X86_FMA3)
#define AM_BUILDSYSTEM_ARCH_X86_FMA3
#endif
#define AM_SIMD_ARCH_FMA3
#endif

#if defined(AM_CPU_ARM_NEON)
#if !defined(AM_BUILDSYSTEM_ARCH_ARM_NEON)
#define AM_BUILDSYSTEM_ARCH_ARM_NEON
#endif
#define AM_SIMD_ARCH_NEON
#endif

#if defined(AM_SIMD_ARCH_AVX2) || defined(AM_SIMD_ARCH_AVX)
#define AM_SIMD_ALIGNMENT 32
#elif defined(AM_SIMD_ARCH_SSE4_1) || defined(AM_SIMD_ARCH_SSSE3) || defined(AM_SIMD_ARCH_SSE3) || defined(AM_SIMD_ARCH_SSE2) || defined(AM_SIMD_ARCH_SSE1)
#define AM_SIMD_ALIGNMENT 16
#elif defined(AM_SIMD_ARCH_NEON)
#define AM_SIMD_ALIGNMENT 16
#endif

#if defined(AM_SIMD_ARCH_AVX2)
#define MA_SUPPORT_AVX2
#define MA_PREFER_AVX2
#elif defined(AM_SIMD_ARCH_AVX)
#define MA_SUPPORT_AVX
#define MA_PREFER_AVX
#define AM_SIMD_ALIGNMENT 32
#elif defined(AM_SIMD_ARCH_SSE2)
#define MA_SUPPORT_SSE2
#define MA_PREFER_SSE2
#elif defined(AM_SIMD_ARCH_NEON)
#define MA_SUPPORT_NEON
#define MA_PREFER_NEON
#define PFFFT_ENABLE_NEON
#endif

#endif

#endif // _AM_CORE_COMMON_SIMD_H
