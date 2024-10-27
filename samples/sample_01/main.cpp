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

#include <iostream>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

using namespace SparkyStudios::Audio::Amplitude;

static constexpr AmUInt32 kAppModeMainMenu = 0;
static constexpr AmUInt32 kAppModeCollectionTest = 1;
static constexpr AmUInt32 kAppModeSwitchContainerTest = 2;

struct ExecutionContext
{
    DiskFileSystem fileLoader;

    AmUInt32 appMode = kAppModeMainMenu;

    Channel mainMenuBackgroundChannel;
    Channel collectionSampleChannel;
    Channel switchContainerChannel;

    AmUInt32 currentSwitchState = 3;

#if !defined(AM_NO_MEMORY_STATS)
    bool printMemoryStats = false;
#endif

    bool pause = false;
    bool stop = false;
};

#if !defined(AM_NO_MEMORY_STATS)
static void printMemoryStats()
{
    std::vector<eMemoryPoolKind> pools = {
        eMemoryPoolKind_Amplimix,  eMemoryPoolKind_Codec, eMemoryPoolKind_Engine,  eMemoryPoolKind_Filtering,
        eMemoryPoolKind_SoundData, eMemoryPoolKind_IO,    eMemoryPoolKind_Default,
    };

    for (auto&& kind : pools)
    {
        const auto& stats = amMemory->GetStats(kind);

        std::cout << "Pool Name - " << MemoryManager::GetMemoryPoolName(kind) << std::endl;
        std::cout << "    Allocations Count: " << stats.allocCount << std::endl;
        std::cout << "    Frees Count: " << stats.freeCount << std::endl;
        std::cout << "    Total Memory used: " << stats.maxMemoryUsed << std::endl;
        std::cout << std::endl;
    }
}
#endif

static void run(AmVoidPtr param)
{
    auto* ctx = static_cast<ExecutionContext*>(param);

    auto cleanup = [&ctx]()
    {
        amEngine->Deinitialize();

        // Wait for the file system to complete loading.
        amEngine->StartCloseFileSystem();
        while (!amEngine->TryFinalizeCloseFileSystem())
            Thread::Sleep(1);

        // Unregister all default plugins
        Engine::UnregisterDefaultPlugins();

        amEngine->DestroyInstance();

        ctx->stop = true;
    };

    ctx->fileLoader.SetBasePath(AM_OS_STRING("./assets"));
    amEngine->SetFileSystem(&ctx->fileLoader);

    // Wait for the file system to complete loading.
    amEngine->StartOpenFileSystem();
    while (!amEngine->TryFinalizeOpenFileSystem())
        Thread::Sleep(1);

    const auto sdkPath = std::filesystem::path(std::getenv("AM_SDK_PATH"));

    // Register all the default plugins shipped with the engine
    Engine::RegisterDefaultPlugins();

    Engine::AddPluginSearchPath(AM_OS_STRING("./assets/plugins"));
    Engine::AddPluginSearchPath(sdkPath / AM_OS_STRING("lib/" AM_SDK_PLATFORM "/plugins"));

#if defined(_DEBUG) || defined(DEBUG) || (defined(__GNUC__) && !defined(__OPTIMIZE__))
    Engine::LoadPlugin(AM_OS_STRING("AmplitudeVorbisCodecPlugin_d"));
    Engine::LoadPlugin(AM_OS_STRING("AmplitudeFlacCodecPlugin_d"));
#else
    Engine::LoadPlugin(AM_OS_STRING("AmplitudeVorbisCodecPlugin"));
    Engine::LoadPlugin(AM_OS_STRING("AmplitudeFlacCodecPlugin"));
#endif

    // Initialize Amplitude.
    if (!amEngine->Initialize(AM_OS_STRING("pc.config.amconfig")))
    {
        cleanup();
        return;
    }

    if (!amEngine->LoadSoundBank(AM_OS_STRING("sample_01.ambank")))
    {
        cleanup();
        return;
    }

    // Start loading sound files.
    amEngine->StartLoadSoundFiles();

    // Cache the master bus, so we can adjust the gain.
    auto masterBus = amEngine->FindBus("master");

    // Cache the background sound handle for the main menu.
    auto mainMenuBackgroundHandle = amEngine->GetSoundHandle("symphony");

    // Cache the CollectionHandle for the collection sample.
    auto collectionHandle = amEngine->GetCollectionHandle("throw_collection_1");
    if (collectionHandle == nullptr)
    {
        fprintf(stderr, "Could not find collection handle %s\n", "throw_collection_1");
        cleanup();
        return;
    }

    // Cache the SwitchContainerHandle for the switch sample.
    auto footstepsHandle = amEngine->GetSwitchContainerHandle(200);
    if (footstepsHandle == nullptr)
    {
        fprintf(stderr, "Could not find switch container handle %s\n", "footsteps");
        cleanup();
        return;
    }

    // Cache the EventHandle for the event sample.
    auto playEventHandle = amEngine->GetEventHandle("play_throw");
    if (playEventHandle == nullptr)
    {
        fprintf(stderr, "Could not find event handle %s\n", "play_throw");
        cleanup();
        return;
    }

    auto stopEventHandle = amEngine->GetEventHandle("stop_throw");
    if (stopEventHandle == nullptr)
    {
        fprintf(stderr, "Could not find event handle %s\n", "stop_throw");
        cleanup();
        return;
    }

    AmUInt32 lastSwitch = 0;

    // Setup the default listener
    auto listener = amEngine->AddListener(1);
    listener.SetLocation(AM_V3(0, 0, 0));
    listener.SetOrientation(Orientation::Zero());

    // Setup a virtual player entity, for the footsteps.
    auto player = amEngine->AddEntity(1);
    player.SetLocation(AM_V3(0, 0, 0));
    player.SetOrientation(Orientation::Zero());

    amEngine->SetDefaultListener(&listener);

    // Wait for the sound files to complete loading
    while (!amEngine->TryFinalizeLoadSoundFiles())
        Thread::Sleep(1);

    while (true)
    {
        if (ctx->stop)
            break;

#if !defined(AM_NO_MEMORY_STATS)
        if (ctx->printMemoryStats)
        {
            printMemoryStats();
            ctx->printMemoryStats = false;
        }
#endif

        amEngine->Pause(ctx->pause);

        if (ctx->pause)
            continue;

        if (ctx->appMode == kAppModeMainMenu)
        {
            if (ctx->collectionSampleChannel.Valid() && ctx->collectionSampleChannel.Playing())
                ctx->collectionSampleChannel.Stop(kAmSecond);

            if (ctx->switchContainerChannel.Valid() && ctx->switchContainerChannel.Playing())
                ctx->switchContainerChannel.Stop(kAmSecond);

            if (!ctx->mainMenuBackgroundChannel.Valid() ||
                ctx->mainMenuBackgroundChannel.GetPlaybackState() == eChannelPlaybackState_Stopped)
            {
                ctx->mainMenuBackgroundChannel = amEngine->Play(mainMenuBackgroundHandle);
                ctx->mainMenuBackgroundChannel.On(
                    ChannelEvent::Begin,
                    [ctx](const ChannelEventInfo& info)
                    {
                        amLogInfo("[CALLBACK] Sound started playing");
                    });
                ctx->mainMenuBackgroundChannel.On(
                    ChannelEvent::Pause,
                    [ctx](const ChannelEventInfo& info)
                    {
                        amLogInfo("[CALLBACK] Sound paused");
                    });
                ctx->mainMenuBackgroundChannel.On(
                    ChannelEvent::Resume,
                    [ctx](const ChannelEventInfo& info)
                    {
                        amLogInfo("[CALLBACK] Sound resumed");
                    });
                ctx->mainMenuBackgroundChannel.On(
                    ChannelEvent::End,
                    [ctx](const ChannelEventInfo& info)
                    {
                        amLogInfo("[CALLBACK] Sound finished playing");
                    });
                ctx->mainMenuBackgroundChannel.On(
                    ChannelEvent::Loop,
                    [ctx](const ChannelEventInfo& info)
                    {
                        amLogInfo("[CALLBACK] Sound started playing again (loop)");
                    });
            }
            else if (ctx->mainMenuBackgroundChannel.GetPlaybackState() == eChannelPlaybackState_Paused)
                ctx->mainMenuBackgroundChannel.Resume(kAmSecond);
        }
        else
        {
            if (ctx->mainMenuBackgroundChannel.Valid())
                ctx->mainMenuBackgroundChannel.Pause(kAmSecond);

            if (ctx->appMode == kAppModeCollectionTest)
            {
                if (!ctx->collectionSampleChannel.Valid() || !ctx->collectionSampleChannel.Playing())
                {
                    ctx->collectionSampleChannel = amEngine->Play(collectionHandle);
                    ctx->collectionSampleChannel.On(
                        ChannelEvent::Begin,
                        [ctx](const ChannelEventInfo& info)
                        {
                            amLogInfo("[CALLBACK] Collection started playing");
                        });
                    ctx->collectionSampleChannel.On(
                        ChannelEvent::End,
                        [ctx](const ChannelEventInfo& info)
                        {
                            amLogInfo("[CALLBACK] Collection finished playing");
                        });
                }
            }
            else if (ctx->appMode == kAppModeSwitchContainerTest)
            {
                if (ctx->currentSwitchState == 3 && lastSwitch != ctx->currentSwitchState)
                {
                    amLogInfo("Walking on metal");
                    amEngine->SetSwitchState("surface_type", "metal");
                }

                if (ctx->currentSwitchState == 4 && lastSwitch != ctx->currentSwitchState)
                {
                    amLogInfo("Walking on grass");
                    amEngine->SetSwitchState("surface_type", "grass");
                }

                if (ctx->currentSwitchState == 5 && lastSwitch != ctx->currentSwitchState)
                {
                    amLogInfo("Walking on snow");
                    amEngine->SetSwitchState("surface_type", "snow");
                }

                if (!ctx->switchContainerChannel.Valid() || !ctx->switchContainerChannel.Playing())
                {
                    ctx->switchContainerChannel = amEngine->Play(footstepsHandle, player);
                    ctx->switchContainerChannel.On(
                        ChannelEvent::Begin,
                        [ctx](const ChannelEventInfo& info)
                        {
                            amLogInfo("[CALLBACK] Switch started playing");
                        });
                    ctx->switchContainerChannel.On(
                        ChannelEvent::End,
                        [ctx](const ChannelEventInfo& info)
                        {
                            amLogInfo("[CALLBACK] Switch finished playing");
                        });
                }

                lastSwitch = ctx->currentSwitchState;
            }
        }

        constexpr AmTime delta = kAmSecond / 60.0;

        amEngine->AdvanceFrame(delta);
        Thread::Sleep(static_cast<AmInt32>(delta));
    }

    cleanup();
}

int main(int argc, char* argv[])
{
#if defined(_DEBUG) || defined(DEBUG) || (defined(__GNUC__) && !defined(__OPTIMIZE__))
    ConsoleLogger logger(true);
#else
    ConsoleLogger logger(false);
#endif

    Logger::SetLogger(&logger);

    MemoryManager::Initialize();

    ExecutionContext ctx{};
    auto t = Thread::CreateThread(run, &ctx);

    std::cout << "Amplitude Audio SDK Sample 01." << std::endl;

    while (!ctx.stop)
    {
        if (ctx.appMode == kAppModeMainMenu)
        {
            std::cout << std::endl;
            std::cout << "Select a sample:" << std::endl;
            std::cout << kAppModeCollectionTest << " - Collection Sample" << std::endl;
            std::cout << kAppModeSwitchContainerTest << " - Switch Container Sample" << std::endl;
            std::cout << "Press 0 to quit the program";

#if !defined(AM_NO_MEMORY_STATS)
            std::cout << ", and 9 to print memory stats";
#endif

            std::cout << "." << std::endl << std::endl;
            std::cout << "Enter a value: ";
        }

        if (ctx.appMode == kAppModeCollectionTest)
        {
            std::cout << std::endl;
            std::cout << "Playing sounds from collection \"throw_collection_1\"." << std::endl;
            std::cout << "All sounds are played one by one, and when the end of the collection is reached, all sounds are played in the "
                         "reverse order."
                      << std::endl;
            std::cout << "This behavior has been configured in the collection file. You can play with the settings, recompile the project "
                         "using flatbuffers, and preview it here."
                      << std::endl
                      << std::endl;
            std::cout << "Press 0 to go back to the main menu";

#if !defined(AM_NO_MEMORY_STATS)
            std::cout << ", and 9 to print memory stats";
#endif

            std::cout << ": ";
        }

        if (ctx.appMode == kAppModeSwitchContainerTest)
        {
            std::cout << std::endl;
            std::cout << "Playing sounds from switch container \"footsteps\"." << std::endl;
            std::cout << "The sounds are played using a virtual entity, and changes according to the state of the \"surface_type\" switch."
                      << std::endl;
            std::cout << "To change the current state of the switch, choose between the following values:" << std::endl;
            std::cout << "3 - Metal" << (ctx.currentSwitchState == 3 ? " (active)" : "") << std::endl;
            std::cout << "4 - Grass" << (ctx.currentSwitchState == 4 ? " (active)" : "") << std::endl;
            std::cout << "5 - Snow" << (ctx.currentSwitchState == 5 ? " (active)" : "") << std::endl << std::endl;
            std::cout << "Press 0 to go back to the main menu";

#if !defined(AM_NO_MEMORY_STATS)
            std::cout << ", and 9 to print memory stats";
#endif

            std::cout << ": ";
        }

        int input = 0;
        std::cin >> input;

#if !defined(AM_NO_MEMORY_STATS)
        if (input == 9)
        {
            std::cout << std::endl;
            printMemoryStats();
            continue;
        }
#endif

        if (ctx.appMode == kAppModeMainMenu)
        {
            if (input != 0 && input != kAppModeCollectionTest && input != kAppModeSwitchContainerTest)
            {
                std::cout << "The input is invalid." << std::endl;
                continue;
            }

            if (input == 0)
            {
                ctx.stop = true;
                break;
            }

            ctx.appMode = input;
        }
        else if (ctx.appMode == kAppModeCollectionTest)
        {
            if (input == 0)
            {
                ctx.appMode = kAppModeMainMenu;
                continue;
            }

            std::cout << "The input is invalid." << std::endl;
        }
        else if (ctx.appMode == kAppModeSwitchContainerTest)
        {
            if (input == 0)
            {
                ctx.appMode = kAppModeMainMenu;
                continue;
            }

            if (input == 3 || input == 4 || input == 5)
            {
                ctx.currentSwitchState = input;
                continue;
            }

            std::cout << "The input is invalid." << std::endl;
        }
    }

    Thread::Wait(t);
    Thread::Release(t);

#if !defined(AM_NO_MEMORY_STATS)
    printMemoryStats();

    amLogInfo("%s", amMemory->InspectMemoryLeaks().c_str());
#endif

    MemoryManager::Deinitialize();

    return 0;
}
