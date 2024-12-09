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

#ifndef _AM_CORE_DEVICE_H
#define _AM_CORE_DEVICE_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

namespace SparkyStudios::Audio::Amplitude
{
    class Driver;

    /**
     * @brief The playback output format of the device.
     *
     * @ingroup engine
     */
    enum class PlaybackOutputFormat : AmUInt16
    {
        /**
         * @brief The default output format of the device.
         *
         * This instruct to use the output format provided by the device.
         */
        Default = 0,

        /**
         * @brief Send audio samples as unsigned 8-bit integers to the device.
         */
        UInt8 = 1,

        /**
         * @brief Send audio samples as signed 16-bit integers to the device.
         */
        Int16 = 2,

        /**
         * @brief Send audio samples as signed 24-bit integers to the device.
         */
        Int24 = 3,

        /**
         * @brief Send audio samples as signed 32-bit integers to the device.
         */
        Int32 = 4,

        /**
         * @brief Send audio samples as 32-bit floating point values to the device.
         */
        Float32 = 5
    };

    /**
     * @brief The playback output channel layout of the device.
     *
     * @ingroup engine
     */
    enum class PlaybackOutputChannels : AmUInt16
    {
        /**
         * @brief The default output channel layout of the device.
         *
         * This instruct to use the output channel layout provided by the device.
         */
        Default = 0,

        /**
         * @brief Encodes audio stream as a 1D mono stream.
         */
        Mono = 1,

        /**
         * @brief Encodes audio stream as a 2D stereo stream.
         */
        Stereo = 2,

        /**
         * @brief Encodes audio stream as a 2D quadratic stream.
         */
        Quad = 4,

        /**
         * @brief Encodes audio stream as a 2D stream following 5.1 surround specifications.
         */
        Surround_5_1 = 6,

        /**
         * @brief Encodes audio stream as a 2D stream following 7.1 surround specifications.
         */
        Surround_7_1 = 8
    };

    /**
     * @brief The device state.
     *
     * @ingroup engine
     */
    enum class DeviceState : AmUInt8
    {
        /**
         * @brief The device is opened, but not yet started.
         */
        Opened,

        /**
         * @brief The device is started and running.
         */
        Started,

        /**
         * @brief The device is paused.
         */
        Paused,

        /**
         * @brief The device is stopped.
         */
        Stopped,

        /**
         * @brief The device is closed.
         */
        Closed
    };

    /**
     * @brief The possible device notification types.
     *
     * @ingroup engine
     */
    enum class DeviceNotification : AmUInt8
    {
        /**
         * @brief The device is opened, but not yet started.
         */
        Opened,

        /**
         * @brief The device is started.
         */
        Started,

        /**
         * @brief The device has changed.
         */
        Rerouted,

        /**
         * @brief The device is stopped.
         */
        Stopped,

        /**
         * @brief The device is closed.
         */
        Closed
    };

    /**
     * @brief The device description.
     *
     * This stores the settings requested from the engine configuration
     * and the actual settings provided by the device.
     *
     * The device settings are filled after the Amplimix initialization,
     * and are provided by the selected Driver.
     *
     * @ingroup engine
     */
    struct DeviceDescription
    {
        /**
         * @brief The device name.
         */
        AmString mDeviceName;

        /**
         * @brief The device ID.
         */
        AmObjectID mDeviceID;

        /**
         * @brief The requested device output format.
         */
        PlaybackOutputFormat mRequestedOutputFormat;

        /**
         * @brief The actual device format.
         */
        PlaybackOutputFormat mDeviceOutputFormat;

        /**
         * @brief The requested device output channel layout.
         */
        PlaybackOutputChannels mRequestedOutputChannels;

        /**
         * @brief The actual device channel layout.
         */
        PlaybackOutputChannels mDeviceOutputChannels;

        /**
         * @brief The requested device sample rate.
         */
        AmUInt32 mRequestedOutputSampleRate;

        /**
         * @brief The actual device sample rate.
         */
        AmUInt32 mDeviceOutputSampleRate;

        /**
         * @brief The device output buffer size.
         */
        AmUInt32 mOutputBufferSize;

        /**
         * @brief The device state.
         */
        DeviceState mDeviceState;
    };

    /**
     * @brief The device notification callback.
     *
     * @param[in] notification The notification type.
     * @param[in] device The device description.
     * @param[in] driver The driver which triggered the device notification.
     *
     * @ingroup engine
     */
    AM_CALLBACK(void, DeviceNotificationCallback)(DeviceNotification notification, const DeviceDescription& device, Driver* driver);

    /**
     * @brief Registers a callback to listen to device state changes.
     *
     * @param[in] callback The callback to register.
     *
     * @ingroup engine
     */
    AM_API_PUBLIC void RegisterDeviceNotificationCallback(DeviceNotificationCallback callback);

    /**
     * @brief Unregisters the registered device notification callback.
     *
     * @ingroup engine
     */
    AM_API_PUBLIC void UnregisterDeviceNotificationCallback();

    /**
     * @brief Calls the registered device notification callback.
     *
     * @param[in] notification The notification type.
     * @param[in] device The device description.
     * @param[in] driver The driver which triggered the device notification.
     *
     * @ingroup engine
     */
    AM_API_PUBLIC void CallDeviceNotificationCallback(DeviceNotification notification, const DeviceDescription& device, Driver* driver);
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_CORE_DEVICE_H
