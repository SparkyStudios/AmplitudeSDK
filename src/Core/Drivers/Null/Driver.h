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

#ifndef _AM_IMPLEMENTATION_CORE_DRIVERS_NULL_DRIVER_H
#define _AM_IMPLEMENTATION_CORE_DRIVERS_NULL_DRIVER_H

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

namespace SparkyStudios::Audio::Amplitude
{
    struct NullDriverDeviceData
    {
        AmUInt32 mOutputBufferSize;
        DeviceDescription mDeviceDescription;
        bool mRunning;
    };

    class NullDriver final : public Driver
    {
    public:
        NullDriver();

        ~NullDriver() override;

        bool Open(const DeviceDescription& device) override;

        bool Close() override;

        bool EnumerateDevices(std::vector<DeviceDescription>& devices) override;

    private:
        bool _initialized;

        AmThreadHandle _thread;

        NullDriverDeviceData _deviceData;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_CORE_DRIVERS_NULL_DRIVER_H
