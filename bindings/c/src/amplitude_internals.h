// Copyright (c) 2023-present Sparky Studios. All rights reserved.
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

#ifndef SS_AMPLITUDE_INTERNALS_H
#define SS_AMPLITUDE_INTERNALS_H

#define BOOL_TO_AM_BOOL(x) ((x) ? AM_TRUE : AM_FALSE)
#define AM_BOOL_TO_BOOL(x) ((x) == AM_TRUE)

using namespace SparkyStudios::Audio::Amplitude;

inline const char* am_allocate_string(const char* str)
{
    const size_t len = strlen(str) + 1;
    auto* result = static_cast<char*>(ammalloc(len));
    std::memcpy(result, str, len);
    return result;
}

inline void am_free_string(const char* str)
{
    amfree(const_cast<char*>(str));
}

inline const am_oschar* am_allocate_osstring(const am_oschar* str)
{
    const size_t len = AmOsString(str).size() + 1;
    auto* result = static_cast<am_oschar*>(ammalloc(len));
    std::memcpy(result, str, len);
    return result;
}

inline void am_free_osstring(const am_oschar* str)
{
    amfree(const_cast<am_oschar*>(str));
}

#endif // SS_AMPLITUDE_INTERNALS_H
