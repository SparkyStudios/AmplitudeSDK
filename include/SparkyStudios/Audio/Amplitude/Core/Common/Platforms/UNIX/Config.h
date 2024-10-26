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

#ifndef _AM_CORE_COMMON_PLATFORMS_UNIX_CONFIG_H
#define _AM_CORE_COMMON_PLATFORMS_UNIX_CONFIG_H

/// Call policy
#define AM_CALL_POLICY

// Library export policy
#if defined(__GNUC__) && __GNUC__ >= 4
#define AM_LIB_EXPORT __attribute__((visibility("default")))
#define AM_LIB_IMPORT __attribute__((visibility("default")))
#define AM_LIB_PRIVATE __attribute__((visibility("hidden"))) static
#else
#define AM_LIB_EXPORT
#define AM_LIB_IMPORT
#define AM_LIB_PRIVATE static
#endif

// Function inline
#define AM_INLINE inline __attribute__((always_inline))
#define AM_NO_INLINE __attribute__((noinline))

// Restrict keyword
#if defined(__GNUC__)
#define AM_RESTRICT __restrict__
#else
#define AM_RESTRICT __restrict
#endif

// Alignment required for SIMD data processing
#define AM_TYPE_ALIGN(_declaration_, _alignment_) _declaration_ __attribute__((aligned(_alignment_)))
#define AM_TYPE_ALIGN_SIMD(_declaration_) AM_TYPE_ALIGN(_declaration_, AM_SIMD_ALIGNMENT)

#if defined(AM_WCHAR_SUPPORTED)
#include <sstream>

// Defines the format used to print AmOsString text
#define AM_OS_CHAR_FMT "%ls"

// Macro used to convert a string literal to an AmOsString string at compile-time
#define AM_OS_STRING(s) L##s

static AM_INLINE std::wstring am_string_widen(const std::string& str)
{
    std::wostringstream wstm;
    const auto& ctfacet = std::use_facet<std::ctype<wchar_t>>(wstm.getloc());
    for (char i : str)
        wstm << ctfacet.widen(i);
    return wstm.str();
}

static AM_INLINE std::string am_wstring_narrow(const std::wstring& str)
{
    std::ostringstream stm;
    const auto& ctfacet = std::use_facet<std::ctype<wchar_t>>(stm.getloc());
    for (wchar_t i : str)
        stm << ctfacet.narrow(i, 0);
    return stm.str();
}

// Conversion between OS strings and default strings
#define AM_OS_STRING_TO_STRING(s) am_wstring_narrow(s).c_str()
#define AM_STRING_TO_OS_STRING(s) am_string_widen(s).c_str()
#else
/// Defines the format used to print AmOsString text
#define AM_OS_CHAR_FMT "%s"

/// Macro used to convert a string literal to an AmOsString string at compile-time
#define AM_OS_STRING(s) s

/// Conversion between OS strings and default strings
#define AM_OS_STRING_TO_STRING(s) std::string(s).c_str()
#define AM_STRING_TO_OS_STRING(s) std::string(s).c_str()
#endif

// AMPLITUDE_ASSERT Config
#ifdef AM_NO_ASSERTS
#define AMPLITUDE_ASSERT(x)
#else
#include <cassert> // assert
#define AMPLITUDE_ASSERT(x) assert(x)
#endif // AM_NO_ASSERTS

#endif // _AM_CORE_COMMON_PLATFORMS_UNIX_CONFIG_H
