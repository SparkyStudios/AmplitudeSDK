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

#include <Core/Drivers/MiniAudio/Driver.h>
#include <Core/Engine.h>

#include <Mixer/Amplimix.h>

namespace SparkyStudios::Audio::Amplitude
{
    static void* ma_malloc(size_t sz, void* pUserData)
    {
        AM_UNUSED(pUserData);
        return ampoolmalloc(eMemoryPoolKind_Amplimix, sz);
    }

    static void* ma_realloc(void* p, size_t sz, void* pUserData)
    {
        AM_UNUSED(pUserData);
        return ampoolrealloc(eMemoryPoolKind_Amplimix, p, sz);
    }

    static void ma_free(void* p, void* pUserData)
    {
        AM_UNUSED(pUserData);
        ampoolfree(eMemoryPoolKind_Amplimix, p);
    }

    static void miniaudio_log(void* pUserData, ma_uint32 level, const char* pMessage)
    {
        AM_UNUSED(pUserData);
        AM_UNUSED(level);

        switch (level)
        {
        case MA_LOG_LEVEL_DEBUG:
            amLogDebug("%s", pMessage);
            break;
        case MA_LOG_LEVEL_INFO:
            amLogInfo("%s", pMessage);
            break;
        case MA_LOG_LEVEL_WARNING:
            amLogWarning("%s", pMessage);
            break;
        case MA_LOG_LEVEL_ERROR:
            amLogError("%s", pMessage);
            break;
        default:
            amLogCritical("%s", pMessage);
            break;
        }
    }

    static void miniaudio_mixer(ma_device* pDevice, AmVoidPtr pOutput, AmConstVoidPtr pInput, ma_uint32 frameCount)
    {
        AM_UNUSED(pInput);

        const auto* driver = static_cast<MiniAudioDriver*>(pDevice->pUserData);

        AudioBuffer* pOutputBuffer = nullptr;
        frameCount = amEngine->GetMixer()->Mix(&pOutputBuffer, frameCount);

        if (pOutputBuffer == nullptr || frameCount == 0)
            return;

        Interleave(
            pOutputBuffer, 0, static_cast<AmReal32*>(pOutput), 0, frameCount,
            static_cast<AmInt32>(driver->GetDeviceDescription().mRequestedOutputChannels));
    }

    void miniaudio_device_notification(const ma_device_notification* pNotification)
    {
        auto* driver = static_cast<MiniAudioDriver*>(pNotification->pDevice->pUserData);

        driver->m_deviceDescription.mDeviceID = 0; // TODO: Compute a proper device ID
        driver->m_deviceDescription.mDeviceName = AmString(pNotification->pDevice->playback.name);
        driver->m_deviceDescription.mDeviceOutputSampleRate = pNotification->pDevice->playback.internalSampleRate;
        driver->m_deviceDescription.mDeviceOutputChannels =
            static_cast<PlaybackOutputChannels>(pNotification->pDevice->playback.internalChannels);
        driver->m_deviceDescription.mDeviceOutputFormat =
            static_cast<PlaybackOutputFormat>(pNotification->pDevice->playback.internalFormat);

        if (auto* mixer = amEngine->GetMixer(); mixer != nullptr)
        {
            mixer->UpdateDevice(
                driver->m_deviceDescription.mDeviceID, driver->m_deviceDescription.mDeviceName,
                driver->m_deviceDescription.mDeviceOutputSampleRate, driver->m_deviceDescription.mDeviceOutputChannels,
                driver->m_deviceDescription.mDeviceOutputFormat);
        }

        switch (pNotification->type)
        {
        case ma_device_notification_type_started:
            driver->m_deviceDescription.mDeviceState = DeviceState::Started;
            CallDeviceNotificationCallback(DeviceNotification::Started, driver->GetDeviceDescription(), driver);
            break;
        case ma_device_notification_type_stopped:
            driver->m_deviceDescription.mDeviceState = DeviceState::Stopped;
            CallDeviceNotificationCallback(DeviceNotification::Stopped, driver->GetDeviceDescription(), driver);
            break;
        case ma_device_notification_type_rerouted:
            CallDeviceNotificationCallback(DeviceNotification::Rerouted, driver->GetDeviceDescription(), driver);
            break;
        case ma_device_notification_type_interruption_began:
            amLogDebug("Device interruption began");
            break;
        case ma_device_notification_type_interruption_ended:
            amLogDebug("Device interruption ended");
            break;
        case ma_device_notification_type_unlocked:
            amLogDebug("Device unlocked");
            break;
        }
    }

    static ma_allocation_callbacks gAllocationCallbacks = { nullptr, ma_malloc, ma_realloc, ma_free };

    MiniAudioDriver::MiniAudioDriver()
        : Driver("miniaudio")
        , _initialized(false)
        , _device()
        , _logCallback()
        , _log()
        , _context()
        , _devices()
    {}

    MiniAudioDriver::~MiniAudioDriver()
    {
        if (_initialized)
            Close();
    }

    bool MiniAudioDriver::Open(const DeviceDescription& device)
    {
        if (!_initialized)
        {
            _logCallback = ma_log_callback_init(miniaudio_log, nullptr);
            if (ma_log_init(nullptr, &_log) != MA_SUCCESS)
            {
                amLogCritical("Failed to initialize miniaudio log.");
                return false;
            }

            ma_log_register_callback(&_log, _logCallback);

            ma_context_config config = ma_context_config_init();
            config.pLog = &_log;
            config.pUserData = this;
            config.threadPriority = ma_thread_priority_realtime;

            if (ma_context_init(nullptr, 0, &config, &_context) != MA_SUCCESS)
            {
                amLogCritical("Failed to initialize miniaudio context");
                return false;
            }

            const auto channelsCount = static_cast<AmInt16>(device.mRequestedOutputChannels);

            ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
            deviceConfig.noClip = true;
            deviceConfig.noPreSilencedOutputBuffer = false;
            deviceConfig.periodSizeInFrames = device.mOutputBufferSize / channelsCount;
            deviceConfig.playback.format = ma_format_from_amplitude(device.mRequestedOutputFormat);
            deviceConfig.playback.channels = channelsCount;
            deviceConfig.playback.pChannelMap =
                static_cast<ma_channel*>(ampoolmalloc(eMemoryPoolKind_Engine, channelsCount * sizeof(ma_channel)));
            deviceConfig.playback.channelMixMode = ma_channel_mix_mode_rectangular;
            deviceConfig.sampleRate = device.mRequestedOutputSampleRate;
            deviceConfig.dataCallback = miniaudio_mixer;
            deviceConfig.notificationCallback = miniaudio_device_notification;
            deviceConfig.pUserData = static_cast<void*>(this);
            deviceConfig.resampling.algorithm = ma_resample_algorithm_linear;

            ma_channel_map_init_standard(ma_standard_channel_map_vorbis, deviceConfig.playback.pChannelMap, channelsCount, channelsCount);

            _initialized = ma_device_init(&_context, &deviceConfig, &_device) == MA_SUCCESS;
            if (!_initialized)
            {
                amLogCritical("The miniaudio driver was not initialized successfully.");

                ampoolfree(eMemoryPoolKind_Engine, deviceConfig.playback.pChannelMap);
                return false;
            }

            m_deviceDescription = device;

            m_deviceDescription.mDeviceID = 0; // TODO: Compute a proper device ID
            m_deviceDescription.mDeviceName = AmString(_device.playback.name);
            m_deviceDescription.mDeviceOutputSampleRate = _device.playback.internalSampleRate;
            m_deviceDescription.mDeviceOutputChannels = static_cast<PlaybackOutputChannels>(_device.playback.internalChannels);
            m_deviceDescription.mDeviceOutputFormat = static_cast<PlaybackOutputFormat>(_device.playback.internalFormat);
            m_deviceDescription.mDeviceState = DeviceState::Opened;

            CallDeviceNotificationCallback(DeviceNotification::Opened, m_deviceDescription, this);

            amEngine->GetMixer()->UpdateDevice(
                m_deviceDescription.mDeviceID, m_deviceDescription.mDeviceName, m_deviceDescription.mDeviceOutputSampleRate,
                m_deviceDescription.mDeviceOutputChannels, m_deviceDescription.mDeviceOutputFormat);

            ampoolfree(eMemoryPoolKind_Engine, deviceConfig.playback.pChannelMap);
        }

        if (ma_device_is_started(&_device) == MA_FALSE && ma_device_start(&_device) != MA_SUCCESS)
        {
            amLogCritical("Unable to open the audio device.");
            return false;
        }

        m_deviceDescription.mDeviceState = DeviceState::Started;
        return true;
    }

    bool MiniAudioDriver::Close()
    {
        if (!_initialized)
        {
            amLogCritical("Cannot close an uninitialized audio device.");
            return false;
        }

        if (ma_device_is_started(&_device) == MA_TRUE && ma_device_stop(&_device) != MA_SUCCESS)
        {
            amLogCritical("Unable to close the audio device.");
            return false;
        }

        m_deviceDescription.mDeviceState = DeviceState::Closed;

        ma_device_uninit(&_device);

        ma_log_unregister_callback(&_log, _logCallback);
        ma_log_uninit(&_log);

        ma_context_uninit(&_context);

        _initialized = false;

        return true;
    }

    bool MiniAudioDriver::EnumerateDevices(std::vector<DeviceDescription>& devices)
    {
        if (_devices.empty())
        {
            ma_device_info* pPlaybackInfos;
            ma_uint32 playbackCount;
            ma_device_info* pCaptureInfos;
            ma_uint32 captureCount;

            if (ma_context_get_devices(&_context, &pPlaybackInfos, &playbackCount, &pCaptureInfos, &captureCount) != MA_SUCCESS)
            {
                amLogError("Unable to enumerate the audio devices.");
                return false;
            }

            _devices.resize(playbackCount);
            for (ma_uint32 iDevice = 0; iDevice < playbackCount; iDevice++)
            {
                _devices[iDevice].mDeviceID = iDevice;
                _devices[iDevice].mDeviceName = AmString(pPlaybackInfos[iDevice].name);
                _devices[iDevice].mDeviceOutputSampleRate = pPlaybackInfos[iDevice].nativeDataFormats[0].sampleRate;
                _devices[iDevice].mDeviceOutputChannels =
                    static_cast<PlaybackOutputChannels>(pPlaybackInfos[iDevice].nativeDataFormats[0].channels);
                _devices[iDevice].mDeviceOutputFormat =
                    static_cast<PlaybackOutputFormat>(pPlaybackInfos[iDevice].nativeDataFormats[0].format);
                _devices[iDevice].mDeviceState = DeviceState::Closed;
            }
        }

        devices = _devices;

        return true;
    }
} // namespace SparkyStudios::Audio::Amplitude
