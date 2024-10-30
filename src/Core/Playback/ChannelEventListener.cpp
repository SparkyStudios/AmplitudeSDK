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

#include <SparkyStudios/Audio/Amplitude/Core/Playback/ChannelEventListener.h>

namespace SparkyStudios::Audio::Amplitude
{
    ChannelEventInfo::ChannelEventInfo(ChannelInternalState* source)
        : m_source(source)
        , m_userData(nullptr)
    {}

    ChannelEventListener::ChannelEventListener()
        : _handlers()
    {}

    ChannelEventListener::~ChannelEventListener()
    {
        _handlers.clear();
    }

    void ChannelEventListener::Add(const ChannelEventCallback& callback, void* userData)
    {
        AddInternal(callback, userData);
    }

    void ChannelEventListener::Call(ChannelInternalState* channel)
    {
        if (_handlers.empty())
            return;

        ChannelEventInfo info(channel);

        for (auto& handler : _handlers)
        {
            info.m_userData = handler.m_userData;

            if (handler.m_callback)
                handler.m_callback(info);
        }
    }

    void ChannelEventListener::AddInternal(const ChannelEventCallback& callback, void* userData)
    {
        const HandlerInstance h = { callback, userData };
        _handlers.push_back(h);
    }

    void ChannelEventListener::CleanLinks()
    {
        _handlers.clear();
    }
} // namespace SparkyStudios::Audio::Amplitude
