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

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <SDL2/SDL.h>
#include <string>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

using namespace SparkyStudios::Audio::Amplitude;

static void device_notification(DeviceNotification notification, const DeviceDescription& device, Driver* driver)
{
    switch (notification)
    {
    case DeviceNotification::Opened:
        amLogInfo("Device opened: %s", device.mDeviceName.c_str());
        break;
    case DeviceNotification::Started:
        amLogInfo("Device started: %s", device.mDeviceName.c_str());
        break;
    case DeviceNotification::Stopped:
        amLogInfo("Device stopped: %s", device.mDeviceName.c_str());
        break;
    case DeviceNotification::Rerouted:
        amLogInfo("Device rerouted: %s", device.mDeviceName.c_str());
        break;
    case DeviceNotification::Closed:
        amLogInfo("Device closed: %s", device.mDeviceName.c_str());
        break;
    }
}

constexpr AmInt32 kScreenWidth = 640;
constexpr AmInt32 kScreenHeight = 480;
constexpr AmTime kFramesPerSecond = 60.0;
constexpr AmUInt32 kDelayMilliseconds = static_cast<AmUInt32>(kAmSecond * 1.0 / kFramesPerSecond);

constexpr char kWindowTitle[] = "Amplitude Audio SDK Sample";
const AmOsString kAudioConfig = AM_OS_STRING("pc.config.amconfig");
const AmOsString kSoundBank = AM_OS_STRING("sample_02.ambank");
constexpr char kInstructionsTexture[] = "./assets/textures/instructions.bmp";
constexpr char kChannelTexture[] = "./assets/textures/channel.bmp";
constexpr char kListenerTexture[] = "./assets/textures/listener.bmp";
constexpr char kSoundHandleName[] = "throw_collection_1";

AmInt32 gListenerIdCounter = 0;

struct IconState
{
    IconState()
        : location(AM_V2(0, 0))
        , velocity(AM_V2(0, 0))
    {}

    AmVec2 location;
    AmVec2 velocity;
};

struct ListenerIcon : public IconState
{
    ListenerIcon()
        : listener(nullptr)
    {}

    Listener listener;
};

struct ChannelIcon : public IconState
{
    ChannelIcon()
        : channel(nullptr)
        , entity(nullptr)
    {}

    Channel channel;
    Entity entity;
};

class SampleState
{
public:
    SampleState()
        : quit_(false)
        , audio_config_source_()
        , master_bus_()
        , window_(nullptr)
        , renderer_(nullptr)
        , sound_handle_(nullptr)
        , instructions_texture_(nullptr)
        , channel_icons_()
        , channel_texture_(nullptr)
        , new_channel_location_()
        , listener_icons_()
        , listener_texture_(nullptr)
        , new_listener_location_()
        , _loader()
    {}

    ~SampleState();

    // Initialize the sample.
    bool Initialize();

    // Run the main loop.
    void Run();

private:
    SDL_Texture* LoadTexture(const char* texture_path) const;
    void AdvanceFrame(AmReal32 delta_time);
    void HandleInput();
    void UpdateIconState(IconState* icon_state, AmReal32 delta_time);
    void UpdateIcons(AmReal32 delta_time);
    void RemoveInvalidSounds();
    void DrawInstructions() const;
    void DrawIcon(const IconState& icon_state, SDL_Texture* texture) const;
    void DrawIcons() const;

    bool quit_;
    AmString audio_config_source_;
    Bus master_bus_;
    SDL_Window* window_;
    SDL_Renderer* renderer_;
    CollectionHandle sound_handle_;
    SDL_Texture* instructions_texture_;

    std::vector<ChannelIcon> channel_icons_;
    SDL_Texture* channel_texture_;
    AmVec2 new_channel_location_;

    std::vector<ListenerIcon> listener_icons_;
    SDL_Texture* listener_texture_;
    AmVec2 new_listener_location_;

    DiskFileSystem _loader;
};

SampleState::~SampleState()
{
    SDL_DestroyTexture(instructions_texture_);
    SDL_DestroyTexture(channel_texture_);
    SDL_DestroyTexture(listener_texture_);
    SDL_DestroyRenderer(renderer_);
    SDL_DestroyWindow(window_);
    SDL_Quit();
}

SDL_Texture* SampleState::LoadTexture(const char* texture_path) const
{
    SDL_Surface* surface = SDL_LoadBMP(texture_path);
    if (surface == nullptr)
    {
        fprintf(stderr, "Could not load `%s`: %s\n", texture_path, SDL_GetError());
        return nullptr;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
    if (texture == nullptr)
    {
        fprintf(stderr, "Could not load `%s`: %s\n", texture_path, SDL_GetError());
        SDL_DestroyTexture(texture);
        return nullptr;
    }
    SDL_FreeSurface(surface);
    return texture;
}

bool SampleState::Initialize()
{
    // Initialize SDL.
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        fprintf(stderr, "Could not initialize SDL: %s\n", SDL_GetError());
        return false;
    }

    // Initialize the window.
    window_ =
        SDL_CreateWindow(kWindowTitle, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, kScreenWidth, kScreenHeight, SDL_WINDOW_SHOWN);
    if (window_ == nullptr)
    {
        fprintf(stderr, "Could not create window: %s\n", SDL_GetError());
        return false;
    }

    // Initialize the renderer.
    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
    if (renderer_ == nullptr)
    {
        fprintf(stderr, "Could not create renderer: %s\n", SDL_GetError());
        return false;
    }
    SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 255);

    // Initialize images.
    if ((channel_texture_ = LoadTexture(kChannelTexture)) == nullptr || (listener_texture_ = LoadTexture(kListenerTexture)) == nullptr ||
        (instructions_texture_ = LoadTexture(kInstructionsTexture)) == nullptr)
        return false;

    RegisterDeviceNotificationCallback(device_notification);

    _loader.SetBasePath(AM_OS_STRING("./assets"));
    amEngine->SetFileSystem(&_loader);

    // Wait for the sound files to complete loading.
    amEngine->StartOpenFileSystem();
    while (!amEngine->TryFinalizeOpenFileSystem())
        SDL_Delay(1);

    // Register all the default plugins shipped with the engine
    Engine::RegisterDefaultPlugins();

    const auto sdkPath = std::filesystem::path(std::getenv("AM_SDK_PATH"));

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
    if (!amEngine->Initialize(kAudioConfig) || !amEngine->LoadSoundBank(kSoundBank))
        return false;

    // Cache the master bus so we can demonstrate adjusting the gain.
    master_bus_ = amEngine->FindBus(kAmMasterBusId);

    // Cache the SoundHandle to the sound we want to play.
    sound_handle_ = amEngine->GetCollectionHandle(kSoundHandleName);
    if (sound_handle_ == nullptr)
    {
        fprintf(stderr, "Could not find sound handle %s\n", kSoundHandleName);
        return false;
    }

    // Start loading sound files.
    amEngine->StartLoadSoundFiles();

    // Success!
    return true;
}

void SampleState::UpdateIconState(IconState* icon_state, AmReal32 delta_time)
{
    icon_state->location += icon_state->velocity * delta_time;
    if (icon_state->location.X < 0)
    {
        icon_state->location.X *= -1;
        icon_state->velocity.X *= -1;
    }
    else if (icon_state->location.X > kScreenWidth)
    {
        icon_state->location.X -= icon_state->location.X - kScreenWidth;
        icon_state->velocity.X *= -1;
    }
    if (icon_state->location.Y < 0)
    {
        icon_state->location.Y *= -1;
        icon_state->velocity.Y *= -1;
    }
    else if (icon_state->location.Y > kScreenHeight)
    {
        icon_state->location.Y -= icon_state->location.Y - kScreenHeight;
        icon_state->velocity.Y *= -1;
    }
}

void SampleState::UpdateIcons(AmReal32 delta_time)
{
    for (auto& icon : channel_icons_)
    {
        UpdateIconState(&icon, delta_time);
        icon.channel.SetLocation(AM_V3(icon.location.X, 0.0f, icon.location.Y));
        icon.entity.SetLocation(AM_V3(icon.location.X, 0.0f, icon.location.Y));
        icon.entity.SetOrientation(Orientation::Zero());
    }

    for (auto& icon : listener_icons_)
    {
        UpdateIconState(&icon, delta_time);
        AmVec3 location = AM_V3(icon.location.X, 0.0f, icon.location.Y);
        icon.listener.SetLocation(location);
        icon.listener.SetOrientation(Orientation::Zero());
    }
}

void TextureRect(SDL_Rect* rect, const AmVec2& location, SDL_Texture* texture)
{
    SDL_QueryTexture(texture, nullptr, nullptr, &rect->w, &rect->h);
    rect->x = static_cast<AmInt32>(location.X - static_cast<AmReal32>(rect->w) / 2);
    rect->y = static_cast<AmInt32>(location.Y - static_cast<AmReal32>(rect->h) / 2);
}

void SampleState::DrawIcon(const IconState& icon_state, SDL_Texture* texture) const
{
    SDL_Rect rect = { 0, 0, 0, 0 };
    TextureRect(&rect, icon_state.location, texture);
    SDL_RenderCopy(renderer_, texture, nullptr, &rect);
}

void SampleState::RemoveInvalidSounds()
{
    std::erase_if(
        channel_icons_,
        [](const ChannelIcon& icon)
        {
            return !icon.channel.Valid();
        });
}

void SampleState::DrawInstructions() const
{
    SDL_Rect rect = { 0, 0, 0, 0 };
    SDL_QueryTexture(instructions_texture_, nullptr, nullptr, &rect.w, &rect.h);
    SDL_RenderCopy(renderer_, instructions_texture_, nullptr, &rect);
}

void SampleState::DrawIcons() const
{
    for (const auto& channel_icon : channel_icons_)
    {
        DrawIcon(channel_icon, channel_texture_);
    }

    for (const auto& listener_icon : listener_icons_)
    {
        DrawIcon(listener_icon, listener_texture_);
    }
}

bool RectContains(const SDL_Rect& rect, const AmVec2& point)
{
    return point.X >= static_cast<AmReal32>(rect.x) && point.X < static_cast<AmReal32>(rect.x + rect.w) &&
        point.Y >= static_cast<AmReal32>(rect.y) && point.Y < static_cast<AmReal32>(rect.y + rect.h);
}

void SampleState::HandleInput()
{
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0)
    {
        switch (event.type)
        {
        case SDL_QUIT:
            {
                quit_ = true;
                amEngine->Pause(true);
                break;
            }
        case SDL_MOUSEBUTTONDOWN:
            {
                AmVec2 mouse_location(AM_V2(event.button.x, event.button.y));
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    new_channel_location_ = mouse_location;
                }
                else
                {
                    new_listener_location_ = mouse_location;
                }
                break;
            }
        case SDL_MOUSEBUTTONUP:
            {
                AmVec2 mouse_location(AM_V2(event.button.x, event.button.y));

                SDL_Texture* texture = channel_texture_;
                auto channel_iter = std::ranges::find_if(
                    channel_icons_,
                    [mouse_location, texture](const ChannelIcon& icon) -> bool
                    {
                        SDL_Rect rect;
                        TextureRect(&rect, icon.location, texture);
                        return RectContains(rect, mouse_location);
                    });
                if (channel_iter != channel_icons_.end())
                {
                    channel_iter->channel.Stop();
                    channel_icons_.erase(channel_iter);
                    break;
                }

                texture = listener_texture_;
                auto listener_iter = std::ranges::find_if(
                    listener_icons_,
                    [mouse_location, texture](const ListenerIcon& icon) -> bool
                    {
                        SDL_Rect rect;
                        TextureRect(&rect, icon.location, texture);
                        return RectContains(rect, mouse_location);
                    });
                if (listener_iter != listener_icons_.end())
                {
                    amEngine->RemoveListener(&listener_iter->listener);
                    listener_icons_.erase(listener_iter);
                    break;
                }

                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    Entity entity = amEngine->AddEntity(++gListenerIdCounter);
                    entity.SetLocation(AM_V3(new_channel_location_.X, 0.0f, new_channel_location_.Y));
                    Channel channel = amEngine->Play(101, entity);
                    if (channel.Valid())
                    {
                        channel_icons_.emplace_back();
                        ChannelIcon& icon = channel_icons_.back();
                        icon.location = new_channel_location_;
                        icon.velocity = (mouse_location - new_channel_location_) * 0.001f;
                        icon.channel = channel;
                        icon.entity = entity;
                    }
                    break;
                }

                if (event.button.button == SDL_BUTTON_RIGHT)
                {
                    Listener listener = amEngine->AddListener(++gListenerIdCounter);
                    if (listener.Valid())
                    {
                        listener_icons_.emplace_back();
                        ListenerIcon& icon = listener_icons_.back();
                        icon.location = new_listener_location_;
                        icon.velocity = mouse_location - new_listener_location_;
                        icon.listener = listener;
                    }
                    break;
                }
                break;
            }
        case SDL_MOUSEMOTION:
            {
                // Set the master gain to be based on a x position of the mouse such
                // that at x = 0 the gain is 0% and at x = kScreenWidth the master gain
                // is at 100%.
                AmReal32 percentage = static_cast<AmReal32>(event.motion.x) / kScreenWidth;
                master_bus_.SetGain(percentage);
            }
        default:; // Do nothing.
        }
    }
}

void SampleState::AdvanceFrame(AmReal32 delta_time)
{
    HandleInput();
    UpdateIcons(delta_time);
    amEngine->AdvanceFrame(delta_time);
    RemoveInvalidSounds();
    SDL_RenderClear(renderer_);
    DrawInstructions();
    DrawIcons();
    SDL_RenderPresent(renderer_);
    SDL_Delay(kDelayMilliseconds);
}

void SampleState::Run()
{
    // Wait for the sound files to complete loading
    while (!amEngine->TryFinalizeLoadSoundFiles())
        SDL_Delay(1);

    AmUInt32 previous_time = 0;
    AmUInt32 time = 0;
    while (!quit_)
    {
        previous_time = time;
        time = SDL_GetTicks();
        const AmTime delta_time = time - previous_time;
        AdvanceFrame(static_cast<AmReal32>(delta_time));
    }
}

AmInt32 main(AmInt32 argc, char* argv[])
{
    (void)argc;
    (void)argv;

    ConsoleLogger logger;
    Logger::SetLogger(&logger);

    MemoryManager::Initialize();

    if (SampleState sample; !sample.Initialize())
        fprintf(stderr, "Failed to initialize!\n");
    else
        sample.Run();

    amEngine->Deinitialize();

    // Wait for the file system to complete loading.
    amEngine->StartCloseFileSystem();
    while (!amEngine->TryFinalizeCloseFileSystem())
        Thread::Sleep(1);

    // Unregister all default plugins
    Engine::UnregisterDefaultPlugins();

    amEngine->DestroyInstance();

#if !defined(AM_NO_MEMORY_STATS)
    amLogInfo("%s", amMemory->InspectMemoryLeaks().c_str());
#endif

    MemoryManager::Deinitialize();

    return 0;
}
