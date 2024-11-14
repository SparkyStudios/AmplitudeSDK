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

#ifndef _AM_CORE_COMMON_PLATFORMS_WINDOWS_CONFIG_H
#define _AM_CORE_COMMON_PLATFORMS_WINDOWS_CONFIG_H

#include <sstream>

// Enable Windows Compilation
#define AM_WINDOWS_VERSION

// Detect the platform CPU type
#if defined(_M_IX86)
#define AM_CPU_X86
#elif defined(_M_AMD64) || defined(_M_X64)
#define AM_CPU_X86_64
#elif defined(_M_ARM)
#define AM_CPU_ARM
#define AM_CPU_ARM_NEON
#elif defined(_M_ARM64)
#define AM_CPU_ARM_64
#define AM_CPU_ARM_NEON
#endif

// Call policy
#define AM_CALL_POLICY __cdecl

// Library export policy
#define AM_LIB_EXPORT __declspec(dllexport)
#define AM_LIB_IMPORT __declspec(dllimport)
#define AM_LIB_PRIVATE static

// Function inline
#define AM_INLINE __forceinline
#define AM_NO_INLINE __declspec(noinline)

// Restrict keyword
#define AM_RESTRICT __restrict

// Alignment required for SIMD data processing
#define AM_TYPE_ALIGN(_declaration_, _alignment_) __declspec(align(_alignment_)) _declaration_
#define AM_TYPE_ALIGN_SIMD(_declaration_) AM_TYPE_ALIGN(_declaration_, AM_SIMD_ALIGNMENT)

// Windows platforms supports wchar_t
#define AM_WCHAR_SUPPORTED

// Defines the format used to print AmOsString text
#define AM_OS_CHAR_FMT "%ls"

// Defines the format used to print AmObjectId value
#define AM_ID_CHAR_FMT "%llu"

// Macro used to convert a string literal to an AmOsString string at compile-time
#define AM_OS_STRING(s) L##s

static AM_INLINE std::wstring am_string_widen(const std::string& str)
{
    std::wostringstream s;
    const auto& f = std::use_facet<std::ctype<wchar_t>>(s.getloc());
    for (const char& i : str)
        s << f.widen(i);
    return s.str();
}

static AM_INLINE std::string am_wstring_narrow(const std::wstring& str)
{
    std::ostringstream s;
    const auto& f = std::use_facet<std::ctype<wchar_t>>(s.getloc());
    for (const wchar_t& i : str)
        s << f.narrow(i, 0);
    return s.str();
}

// Conversion between OS strings and default strings
#define AM_OS_STRING_TO_STRING(s) am_wstring_narrow(s).c_str()
#define AM_STRING_TO_OS_STRING(s) am_string_widen(s).c_str()

// AMPLITUDE_ASSERT Config
#ifdef AM_NO_ASSERTS
#define AMPLITUDE_ASSERT(x)
#else
#ifdef _MSC_VER
#include <cstdio> // for sprintf in asserts
#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif // VC_EXTRALEAN
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX // No need of min and max macros
#endif // NOMINMAX
#include <Windows.h> // only needed for OutputDebugStringA, should be solved somehow.
#define AMPLITUDE_ASSERT(x)                                                                                                                \
    if (!(x))                                                                                                                              \
    {                                                                                                                                      \
        char temp[200];                                                                                                                    \
        sprintf(temp, "%s(%d): assert(%s) failed.\n", __FILE__, __LINE__, #x);                                                             \
        OutputDebugStringA(temp);                                                                                                          \
        __debugbreak();                                                                                                                    \
    }                                                                                                                                      \
    (void)0
#else
#include <cassert> // assert
#define AMPLITUDE_ASSERT(x) assert(x)
#endif // _MSC_VER
#endif // AM_NO_ASSERTS

#endif // _AM_CORE_COMMON_PLATFORMS_WINDOWS_CONFIG_H
