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

#include <catch2/catch_session.hpp>
#include <catch2/reporters/catch_reporter_event_listener.hpp>
#include <catch2/reporters/catch_reporter_registrars.hpp>
#include <iostream>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

using namespace SparkyStudios::Audio::Amplitude;

struct AmTestListener : Catch::EventListenerBase
{
    using EventListenerBase::EventListenerBase; // inherit constructor

    // Get rid of weak-tables
    ~AmTestListener() override = default;

    static void run(AmVoidPtr listener)
    {
        const auto* self = static_cast<AmTestListener*>(listener);

        while (self->running)
        {
            constexpr AmTime delta = kAmSecond / 60.0;

            amEngine->AdvanceFrame(delta);
            Thread::Sleep(static_cast<AmInt32>(delta));
        }

        amLogDebug("Test run ended");
    }

    // The whole test run starting
    void testRunStarting(Catch::TestRunInfo const& testRunInfo) override
    {
        fileSystem.SetBasePath(AM_OS_STRING("./samples/assets"));

        amEngine->SetFileSystem(&fileSystem);

        // Wait for the file system to complete loading.
        amEngine->StartOpenFileSystem();
        while (!amEngine->TryFinalizeOpenFileSystem())
            Thread::Sleep(1);

        // Register all the default plugins shipped with the engine
        Engine::RegisterDefaultPlugins();
        Driver::Unregister(Driver::Find("miniaudio"));

        const auto sdkPath = std::filesystem::path(std::getenv("AM_SDK_PATH"));

        Engine::AddPluginSearchPath(AM_OS_STRING("./assets/plugins"));
        Engine::AddPluginSearchPath(sdkPath / AM_OS_STRING("lib/" AM_SDK_PLATFORM "/plugins"));

        // Engine::LoadPlugin(AM_OS_STRING("AmplitudeVorbisCodecPlugin_d"));
        // Engine::LoadPlugin(AM_OS_STRING("AmplitudeFlacCodecPlugin_d"));

        running = true;

        threadHandle = Thread::CreateThread(run, this);
    }

    // The whole test run ending
    void testRunEnded(Catch::TestRunStats const& testRunStats) override
    {
        running = false;

        Thread::Wait(threadHandle);
        Thread::Release(threadHandle);

        if (amEngine->IsInitialized())
        {
            while (amEngine->HasLoadedSoundBanks())
                amEngine->UnloadSoundBanks();

            amEngine->Deinitialize();

            // Wait for the file system to complete loading.
            amEngine->StartCloseFileSystem();
            while (!amEngine->TryFinalizeCloseFileSystem())
                Thread::Sleep(1);
        }

        // Unregister all default plugins
        Engine::UnregisterDefaultPlugins();

        amEngine->DestroyInstance();
    }

    AmThreadHandle threadHandle = nullptr;
    DiskFileSystem fileSystem;
    bool running = false;
};

CATCH_REGISTER_LISTENER(AmTestListener)

void deviceCallback(DeviceNotification notification, const DeviceDescription& device, Driver* driver)
{
    amLogDebug(
        "Device notification: %d, device: %s, driver: %s", static_cast<int>(notification), device.mDeviceName.c_str(),
        driver->GetName().c_str());
}

int main(int argc, char* argv[])
{
    ConsoleLogger logger;
    Logger::SetLogger(&logger);

    RegisterDeviceNotificationCallback(deviceCallback);

    MemoryManager::Initialize();

    const auto res = Catch::Session().run(argc, argv);

    MemoryManager::Deinitialize();

    return res;
}