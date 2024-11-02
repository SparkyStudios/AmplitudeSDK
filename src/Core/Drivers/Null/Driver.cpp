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

#include <Core/Drivers/Null/Driver.h>

#include <Mixer/Amplimix.h>

namespace SparkyStudios::Audio::Amplitude
{
    static void null_mix(void* param)
    {
        const auto* data = static_cast<NullDriverDeviceData*>(param);

        while (data->mRunning)
        {
            Engine::GetInstance()->GetMixer()->Mix(nullptr, data->mOutputBufferSize);
            Thread::Sleep(1);
        }
    }

    NullDriver::NullDriver()
        : Driver("null")
        , _initialized(false)
        , _thread(nullptr)
        , _deviceData()
    {}

    NullDriver::~NullDriver()
    {
        Close();
    }

    bool NullDriver::Open(const DeviceDescription& device)
    {
        if (_initialized)
            return true;

        CallDeviceNotificationCallback(DeviceNotification::Opened, device, this);

        _deviceData.mOutputBufferSize = device.mOutputBufferSize / static_cast<AmUInt32>(device.mRequestedOutputChannels);
        _deviceData.mDeviceDescription = device;
        _deviceData.mRunning = true;

        _thread = Thread::CreateThread(null_mix, &_deviceData);

        _initialized = true;
        CallDeviceNotificationCallback(DeviceNotification::Started, device, this);

        return true;
    }

    bool NullDriver::Close()
    {
        if (_initialized)
        {
            _deviceData.mRunning = false;
            CallDeviceNotificationCallback(DeviceNotification::Stopped, _deviceData.mDeviceDescription, this);

            Thread::Wait(_thread);
            Thread::Release(_thread);
            _thread = nullptr;

            _deviceData.mOutputBufferSize = 0;

            _initialized = false;
            CallDeviceNotificationCallback(DeviceNotification::Closed, _deviceData.mDeviceDescription, this);
        }

        return true;
    }

    bool NullDriver::EnumerateDevices(std::vector<DeviceDescription>& devices)
    {
        return true;
    }
} // namespace SparkyStudios::Audio::Amplitude