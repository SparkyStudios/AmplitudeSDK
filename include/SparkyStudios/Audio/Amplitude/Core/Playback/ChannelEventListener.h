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

#pragma once

#ifndef _AM_CORE_PLAYBACK_CHANNEL_EVENT_LISTENER_H
#define _AM_CORE_PLAYBACK_CHANNEL_EVENT_LISTENER_H

#include <functional>
#include <list>

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

namespace SparkyStudios::Audio::Amplitude
{
    class ChannelInternalState;

    /**
     * @brief The event info passed to the channel event listener.
     *
     * @ingroup engine
     */
    struct AM_API_PUBLIC ChannelEventInfo
    {
        /**
         * @brief Constructor.
         *
         * @param[in] source The source of the event.
         *
         * @warning This constructor is for internal usage only.
         */
        explicit ChannelEventInfo(ChannelInternalState* source);

        /**
         * @brief The event source.
         */
        ChannelInternalState* m_source;

        /**
         * @brief Additional user data passed to the event listener.
         */
        void* m_userData;
    };

    /**
     * @brief A callback function for handling channel events.
     *
     * @ingroup engine
     */
    typedef std::function<void(ChannelEventInfo)> ChannelEventCallback;

    /**
     * @brief Channel Event listener.
     *
     * Event handlers are registered to the event listener through a callback to receive
     * event notifications.
     *
     * @ingroup engine
     */
    class AM_API_PUBLIC ChannelEventListener
    {
    public:
        /**
         * @brief Initializes a new channel event listener.
         */
        ChannelEventListener();

        /**
         * @brief Destroys the channel event listener.
         */
        ~ChannelEventListener();

        /**
         * @brief Registers a custom callback to this event listener.
         *
         * @param[in] callback The event callback.
         * @param[in] userData The additional data to pass to the handler when this listener receive the event.
         */
        void Add(const ChannelEventCallback& callback, void* userData = nullptr);

        /**
         * @brief Executes the event by calling all the registered event handlers.
         *
         * @param[in] channel The channel which have triggered the event.
         *
         * @warning This method is for internal usage only.
         */
        void Call(ChannelInternalState* channel);

    private:
        /**
         * @brief Adds a custom callback to the internal list of handlers.
         *
         * @param[in] callback The event callback.
         * @param[in] userData The additional data to pass to the handler when this listener receive the event.
         *
         * @internal
         */
        void AddInternal(const ChannelEventCallback& callback, void* userData = nullptr);

        /**
         * @brief Removes all the custom callbacks from the internal list of handlers.
         *
         * @internal
         */
        void CleanLinks();

        /**
         * @brief Internal structure to hold a callback and its user data.
         *
         * @internal
         */
        struct HandlerInstance
        {
            ChannelEventCallback m_callback; ///< The callback function
            void* m_userData; ///< The user data
        };

        /**
         * @brief Internal list of registered event handlers.
         *
         * @internal
         */
        std::list<HandlerInstance> _handlers;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_CORE_PLAYBACK_CHANNEL_EVENT_LISTENER_H
