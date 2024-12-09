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

#include <SparkyStudios/Audio/Amplitude/Core/Device.h>

namespace SparkyStudios::Audio::Amplitude
{
    static DeviceNotificationCallback gDeviceNotificationCallback = nullptr;

    void RegisterDeviceNotificationCallback(DeviceNotificationCallback callback)
    {
        gDeviceNotificationCallback = callback;
    }

    void UnregisterDeviceNotificationCallback()
    {
        gDeviceNotificationCallback = nullptr;
    }

    void CallDeviceNotificationCallback(DeviceNotification notification, const DeviceDescription& device, Driver* driver)
    {
        if (gDeviceNotificationCallback != nullptr)
        {
            gDeviceNotificationCallback(notification, device, driver);
        }
    }
} // namespace SparkyStudios::Audio::Amplitude