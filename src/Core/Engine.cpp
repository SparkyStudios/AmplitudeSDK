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
#include <cmath>
#include <fstream>
#include <memory>
#include <ranges>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Core/Engine.h>
#include <Core/EngineInternalState.h>
#include <Core/Event.h>
#include <Core/Playback/BusInternalState.h>
#include <Sound/Attenuation.h>
#include <Sound/Collection.h>
#include <Sound/Effect.h>
#include <Sound/Rtpc.h>
#include <Sound/Sound.h>
#include <Sound/Switch.h>
#include <Sound/SwitchContainer.h>

#include "buses_definition_generated.h"
#include "collection_definition_generated.h"
#include "engine_config_definition_generated.h"
#include "sound_definition_generated.h"
#include "switch_container_definition_generated.h"

#include <Core/DefaultPlugins.h>

#include <dylib.hpp>

#if defined(AM_WINDOWS_VERSION)
#undef CreateMutex
#endif

namespace SparkyStudios::Audio::Amplitude
{
    typedef flatbuffers::Vector<uint64_t> BusIdList;
    typedef flatbuffers::Vector<flatbuffers::Offset<DuckBusDefinition>> DuckBusDefinitionList;

    // The list of loaded plugins.
    static std::vector<dylib*> gLoadedPlugins = {};

    // Default Plugins instances
    static AmUniquePtr<eMemoryPoolKind_Engine, DefaultResampler> sDefaultResamplerPlugin = nullptr;
    static AmUniquePtr<eMemoryPoolKind_Engine, R8BrainResampler> sR8BrainResamplerPlugin = nullptr;
    // ---
    static AmUniquePtr<eMemoryPoolKind_Engine, ConstantFader> sConstantFaderPlugin = nullptr;
    static AmUniquePtr<eMemoryPoolKind_Engine, EaseFader> sEaseFaderPlugin = nullptr;
    static AmUniquePtr<eMemoryPoolKind_Engine, EaseInFader> sEaseInFaderPlugin = nullptr;
    static AmUniquePtr<eMemoryPoolKind_Engine, EaseInOutFader> sEaseInOutFaderPlugin = nullptr;
    static AmUniquePtr<eMemoryPoolKind_Engine, EaseOutFader> sEaseOutFaderPlugin = nullptr;
    static AmUniquePtr<eMemoryPoolKind_Engine, ExponentialFader> sExponentialFaderPlugin = nullptr;
    static AmUniquePtr<eMemoryPoolKind_Engine, LinearFader> sLinearFaderPlugin = nullptr;
    static AmUniquePtr<eMemoryPoolKind_Engine, SCurveSmoothFader> sCurveSmoothFaderPlugin = nullptr;
    static AmUniquePtr<eMemoryPoolKind_Engine, SCurveSharpFader> sCurveSharpFaderPlugin = nullptr;
    // ---
    static AmUniquePtr<eMemoryPoolKind_Engine, AMSCodec> sAMSCodecPlugin = nullptr;
    static AmUniquePtr<eMemoryPoolKind_Engine, MP3Codec> sMP3CodecPlugin = nullptr;
    static AmUniquePtr<eMemoryPoolKind_Engine, WAVCodec> sWAVCodecPlugin = nullptr;
    // ---
    static AmUniquePtr<eMemoryPoolKind_Engine, MiniAudioDriver> sMiniAudioDriverPlugin = nullptr;
    static AmUniquePtr<eMemoryPoolKind_Engine, NullDriver> sNullDriverPlugin = nullptr;
    // ---
    static AmUniquePtr<eMemoryPoolKind_Engine, BassBoostFilter> sBassBoostFilterPlugin = nullptr;
    static AmUniquePtr<eMemoryPoolKind_Engine, BiquadResonantFilter> sBiquadResonantFilterPlugin = nullptr;
    static AmUniquePtr<eMemoryPoolKind_Engine, DCRemovalFilter> sDCRemovalFilterPlugin = nullptr;
    static AmUniquePtr<eMemoryPoolKind_Engine, DelayFilter> sDelayFilterPlugin = nullptr;
    static AmUniquePtr<eMemoryPoolKind_Engine, EqualizerFilter> sEqualizerFilterPlugin = nullptr;
    static AmUniquePtr<eMemoryPoolKind_Engine, FlangerFilter> sFlangerFilterPlugin = nullptr;
    static AmUniquePtr<eMemoryPoolKind_Engine, LofiFilter> sLofiFilterPlugin = nullptr;
    static AmUniquePtr<eMemoryPoolKind_Engine, MonoPoleFilter> sMonoPoleFilterPlugin = nullptr;
    static AmUniquePtr<eMemoryPoolKind_Engine, RobotizeFilter> sRobotizeFilterPlugin = nullptr;
    static AmUniquePtr<eMemoryPoolKind_Engine, WaveShaperFilter> sWaveShaperFilterPlugin = nullptr;
    // ---
    static AmUniquePtr<eMemoryPoolKind_Engine, AmbisonicBinauralDecoderNode> sAmbisonicBinauralDecoderNodePlugin = nullptr;
    static AmUniquePtr<eMemoryPoolKind_Engine, AmbisonicMixerNode> sAmbisonicMixerNodePlugin = nullptr;
    static AmUniquePtr<eMemoryPoolKind_Engine, AmbisonicPanningNode> sAmbisonicPanningNodePlugin = nullptr;
    static AmUniquePtr<eMemoryPoolKind_Engine, AmbisonicRotatorNode> sAmbisonicRotatorNodePlugin = nullptr;
    static AmUniquePtr<eMemoryPoolKind_Engine, AttenuationNode> sAttenuationNodePlugin = nullptr;
    static AmUniquePtr<eMemoryPoolKind_Engine, ClampNode> sClampNodePlugin = nullptr;
    static AmUniquePtr<eMemoryPoolKind_Engine, ClipNode> sClipNodePlugin = nullptr;
    static AmUniquePtr<eMemoryPoolKind_Engine, EnvironmentEffectNode> sEnvironmentEffectNodePlugin = nullptr;
    static AmUniquePtr<eMemoryPoolKind_Engine, InputNode> sInputNodePlugin = nullptr;
    static AmUniquePtr<eMemoryPoolKind_Engine, NearFieldEffectNode> sNearFieldEffectNodePlugin = nullptr;
    static AmUniquePtr<eMemoryPoolKind_Engine, ObstructionNode> sObstructionNodePlugin = nullptr;
    static AmUniquePtr<eMemoryPoolKind_Engine, OcclusionNode> sOcclusionNodePlugin = nullptr;
    static AmUniquePtr<eMemoryPoolKind_Engine, OutputNode> sOutputNodePlugin = nullptr;
    static AmUniquePtr<eMemoryPoolKind_Engine, ReflectionsNode> sReflectionsNodePlugin = nullptr;
    static AmUniquePtr<eMemoryPoolKind_Engine, ReverbNode> sReverbNodePlugin = nullptr;
    static AmUniquePtr<eMemoryPoolKind_Engine, StereoMixerNode> sStereoMixerNodePlugin = nullptr;
    static AmUniquePtr<eMemoryPoolKind_Engine, StereoPanningNode> sStereoPanningNodePlugin = nullptr;

    static AmUniquePtr<eMemoryPoolKind_Engine, EngineImpl> gAmplitude = nullptr;

    std::set<AmOsString> EngineImpl::_pluginSearchPaths = {};

    class LoadSoundBankTask final : public Thread::PoolTask
    {
    public:
        explicit LoadSoundBankTask(SoundBank* soundBank)
            : PoolTask()
            , _soundBank(soundBank)
        {}

        void Work() override
        {
            _soundBank->LoadSoundFiles(amEngine);
        }

        bool Ready() override
        {
            return _soundBank != nullptr;
        }

    private:
        SoundBank* _soundBank = nullptr;
    };

    bool LoadFile(const std::shared_ptr<File>& file, AmString* dest)
    {
        if (!file->IsValid())
        {
            amLogError("The file is not valid.");
            return false;
        }

        const AmSize bytes = file->Length();

        // Get the file's size:
        dest->assign(bytes + 1, 0);

        // Read the file into the buffer
        file->Seek(0, eFileSeekOrigin_Start);
        const AmUInt32 len = file->Read(reinterpret_cast<AmUInt8Buffer>(&(*dest)[0]), bytes);

        return len == bytes && len > 0;
    }

    AmUInt32 GetMaxNumberOfChannels(const EngineConfigDefinition* config)
    {
        return config->mixer()->virtual_channels() + config->mixer()->active_channels();
    }

    // Returns this channel to the appropriate free list based on whether it's
    // backed by a real channel or not.
    void InsertIntoFreeList(EngineInternalState* state, ChannelInternalState* channel)
    {
        channel->Remove();
        channel->Reset();
        FreeList* list = channel->IsReal() ? &state->real_channel_free_list : &state->virtual_channel_free_list;
        list->push_front(*channel);
    }

    void AssignBestRoom(ChannelInternalState* newChannel, const AmVec3& location, EngineInternalState* state)
    {
        RoomInternalState* bestRoom = nullptr;
        AmReal32 minDistanceSquared = std::numeric_limits<AmReal32>::max();

        for (auto&& room : state->room_list)
        {
            auto& shape = room.GetShape();

            if (shape.Contains(location))
            {
                bestRoom = &room;
                break;
            }
            else if (const AmReal32 distance = shape.GetShortestDistanceToEdge(location); distance < minDistanceSquared)
            {
                minDistanceSquared = distance;
                bestRoom = &room;
            }
        }

        newChannel->SetRoom(Room(bestRoom));
    }

    EngineImpl::EngineImpl()
        : _frameThreadMutex(nullptr)
        , _configSrc()
        , _state(nullptr)
        , _defaultListener(nullptr)
        , _fs()
        , _audioDriver(nullptr)
    {}

    EngineImpl::~EngineImpl()
    {
        Deinitialize();

        _configSrc.clear();

        _audioDriver = nullptr;

        for (const auto& plugin : gLoadedPlugins)
        {
            if (const auto unregisterFunc = plugin->get_function<bool()>("UnregisterPlugin"); !unregisterFunc())
                amLogError("An error occurred while unloading the plugin '%s'", plugin->get_function<const char*()>("PluginName")());

            ampooldelete(eMemoryPoolKind_Engine, dylib, plugin);
        }

        gLoadedPlugins.clear();
    }

    AmVoidPtr Engine::LoadPlugin(const AmOsString& pluginLibraryName)
    {
        if (pluginLibraryName.empty())
        {
            amLogError("The plugin library path is empty");
            return nullptr;
        }

        AmOsString pluginsDirectoryPath = std::filesystem::current_path().native();
        const auto& finalName = AM_STRING_TO_OS_STRING(dylib::filename_components::prefix) + pluginLibraryName +
            AM_STRING_TO_OS_STRING(dylib::filename_components::suffix);

        bool foundPath = false;

        {
            DiskFileSystem fs;
            fs.SetBasePath(pluginsDirectoryPath);

            // Search for the library in the current directory
            if (const auto realPath = fs.ResolvePath(pluginsDirectoryPath); fs.Exists(fs.Join({ realPath, finalName })))
            {
                pluginsDirectoryPath = realPath;
                foundPath = true;
            }

            if (!foundPath)
            {
                // Search for the library in the search paths
                for (const auto& path : EngineImpl::_pluginSearchPaths)
                {
                    if (const auto realPath = fs.ResolvePath(path); fs.Exists(fs.Join({ realPath, finalName })))
                    {
                        pluginsDirectoryPath = realPath;
                        foundPath = true;
                        break;
                    }
                }
            }
        }

        if (!foundPath)
        {
            amLogError("The plugin '" AM_OS_CHAR_FMT "' cannot be found in any of the search paths.", pluginLibraryName.c_str());
            return nullptr;
        }

        auto* plugin = ampoolnew(
            eMemoryPoolKind_Engine, dylib, AM_OS_STRING_TO_STRING(pluginsDirectoryPath), AM_OS_STRING_TO_STRING(finalName),
            dylib::no_filename_decorations);

        if (!plugin->has_symbol("RegisterPlugin"))
        {
            amLogError(
                "Failed to load plugin '" AM_OS_CHAR_FMT "'. The library doesn't export a RegisterPlugin symbol.",
                pluginLibraryName.c_str());
            return nullptr;
        }

        if (!plugin->has_symbol("PluginName"))
        {
            amLogError(
                "Failed to load plugin '" AM_OS_CHAR_FMT "'. The library doesn't export a PluginName symbol.", pluginLibraryName.c_str());
            return nullptr;
        }

        if (!plugin->has_symbol("PluginVersion"))
        {
            amLogError(
                "Failed to load plugin '" AM_OS_CHAR_FMT "'. The library doesn't export a PluginVersion symbol.",
                pluginLibraryName.c_str());
            return nullptr;
        }

        if (const auto registerFunc = plugin->get_function<bool(Engine*, MemoryManager*)>("RegisterPlugin");
            !registerFunc(amEngine, amMemory))
        {
            amLogError("Failed to load plugin '" AM_OS_CHAR_FMT "'. The plugin registration has failed.", pluginLibraryName.c_str());
            return nullptr;
        }

        {
            const auto GetPluginName = plugin->get_function<const char*()>("PluginName");
            const auto GetPluginVersion = plugin->get_function<const char*()>("PluginVersion");
            amLogInfo("Loaded Plugin '%s' Version: %s", GetPluginName(), GetPluginVersion());
        }

        void* handle = plugin->native_handle();
        gLoadedPlugins.push_back(plugin);

        return handle;
    }

    void Engine::AddPluginSearchPath(const AmOsString& path)
    {
        if (path.empty())
        {
            amLogError("The plugin search path is empty.");
            return;
        }

        if (EngineImpl::_pluginSearchPaths.contains(path))
            return;

        EngineImpl::_pluginSearchPaths.emplace(path);
    }

    void Engine::RemovePluginSearchPath(const AmOsString& path)
    {
        if (path.empty())
        {
            amLogError("The plugin search path is empty.");
            return;
        }

        const auto it = EngineImpl::_pluginSearchPaths.find(path);
        if (it == EngineImpl::_pluginSearchPaths.cend())
            return;

        EngineImpl::_pluginSearchPaths.erase(it);
    }

    bool Engine::RegisterDefaultPlugins()
    {
        if (gAmplitude != nullptr && gAmplitude->_state != nullptr)
            return false; // Cannot register the default plugins when the engine is already initialized.

        // Ensure to clean up registries
        UnregisterDefaultPlugins();

        sDefaultResamplerPlugin.reset(ampoolnew(eMemoryPoolKind_Engine, DefaultResampler));
        sR8BrainResamplerPlugin.reset(ampoolnew(eMemoryPoolKind_Engine, R8BrainResampler));
        // ---
        sConstantFaderPlugin.reset(ampoolnew(eMemoryPoolKind_Engine, ConstantFader));
        sEaseFaderPlugin.reset(ampoolnew(eMemoryPoolKind_Engine, EaseFader));
        sEaseInFaderPlugin.reset(ampoolnew(eMemoryPoolKind_Engine, EaseInFader));
        sEaseInOutFaderPlugin.reset(ampoolnew(eMemoryPoolKind_Engine, EaseInOutFader));
        sEaseOutFaderPlugin.reset(ampoolnew(eMemoryPoolKind_Engine, EaseOutFader));
        sExponentialFaderPlugin.reset(ampoolnew(eMemoryPoolKind_Engine, ExponentialFader));
        sLinearFaderPlugin.reset(ampoolnew(eMemoryPoolKind_Engine, LinearFader));
        sCurveSmoothFaderPlugin.reset(ampoolnew(eMemoryPoolKind_Engine, SCurveSmoothFader));
        sCurveSharpFaderPlugin.reset(ampoolnew(eMemoryPoolKind_Engine, SCurveSharpFader));
        // ---
        sAMSCodecPlugin.reset(ampoolnew(eMemoryPoolKind_Engine, AMSCodec));
        sMP3CodecPlugin.reset(ampoolnew(eMemoryPoolKind_Engine, MP3Codec));
        sWAVCodecPlugin.reset(ampoolnew(eMemoryPoolKind_Engine, WAVCodec));
        // ---
        sMiniAudioDriverPlugin.reset(ampoolnew(eMemoryPoolKind_Engine, MiniAudioDriver));
        sNullDriverPlugin.reset(ampoolnew(eMemoryPoolKind_Engine, NullDriver));
        // ---
        sBassBoostFilterPlugin.reset(ampoolnew(eMemoryPoolKind_Engine, BassBoostFilter));
        sBiquadResonantFilterPlugin.reset(ampoolnew(eMemoryPoolKind_Engine, BiquadResonantFilter));
        sDCRemovalFilterPlugin.reset(ampoolnew(eMemoryPoolKind_Engine, DCRemovalFilter));
        sDelayFilterPlugin.reset(ampoolnew(eMemoryPoolKind_Engine, DelayFilter));
        sEqualizerFilterPlugin.reset(ampoolnew(eMemoryPoolKind_Engine, EqualizerFilter));
        sFlangerFilterPlugin.reset(ampoolnew(eMemoryPoolKind_Engine, FlangerFilter));
        sLofiFilterPlugin.reset(ampoolnew(eMemoryPoolKind_Engine, LofiFilter));
        sMonoPoleFilterPlugin.reset(ampoolnew(eMemoryPoolKind_Engine, MonoPoleFilter));
        sRobotizeFilterPlugin.reset(ampoolnew(eMemoryPoolKind_Engine, RobotizeFilter));
        sWaveShaperFilterPlugin.reset(ampoolnew(eMemoryPoolKind_Engine, WaveShaperFilter));
        // ---
        sAmbisonicBinauralDecoderNodePlugin.reset(ampoolnew(eMemoryPoolKind_Engine, AmbisonicBinauralDecoderNode));
        sAmbisonicMixerNodePlugin.reset(ampoolnew(eMemoryPoolKind_Engine, AmbisonicMixerNode));
        sAmbisonicPanningNodePlugin.reset(ampoolnew(eMemoryPoolKind_Engine, AmbisonicPanningNode));
        sAmbisonicRotatorNodePlugin.reset(ampoolnew(eMemoryPoolKind_Engine, AmbisonicRotatorNode));
        sAttenuationNodePlugin.reset(ampoolnew(eMemoryPoolKind_Engine, AttenuationNode));
        sClampNodePlugin.reset(ampoolnew(eMemoryPoolKind_Engine, ClampNode));
        sClipNodePlugin.reset(ampoolnew(eMemoryPoolKind_Engine, ClipNode));
        sEnvironmentEffectNodePlugin.reset(ampoolnew(eMemoryPoolKind_Engine, EnvironmentEffectNode));
        sInputNodePlugin.reset(ampoolnew(eMemoryPoolKind_Engine, InputNode));
        sNearFieldEffectNodePlugin.reset(ampoolnew(eMemoryPoolKind_Engine, NearFieldEffectNode));
        sObstructionNodePlugin.reset(ampoolnew(eMemoryPoolKind_Engine, ObstructionNode));
        sOcclusionNodePlugin.reset(ampoolnew(eMemoryPoolKind_Engine, OcclusionNode));
        sOutputNodePlugin.reset(ampoolnew(eMemoryPoolKind_Engine, OutputNode));
        sReflectionsNodePlugin.reset(ampoolnew(eMemoryPoolKind_Engine, ReflectionsNode));
        sReverbNodePlugin.reset(ampoolnew(eMemoryPoolKind_Engine, ReverbNode));
        sStereoMixerNodePlugin.reset(ampoolnew(eMemoryPoolKind_Engine, StereoMixerNode));
        sStereoPanningNodePlugin.reset(ampoolnew(eMemoryPoolKind_Engine, StereoPanningNode));

        return true;
    }

    bool Engine::UnregisterDefaultPlugins()
    {
        if (gAmplitude != nullptr && gAmplitude->_state != nullptr)
            return false; // Cannot unregister the default plugins when the engine is already initialized.

        sDefaultResamplerPlugin.reset(nullptr);
        sR8BrainResamplerPlugin.reset(nullptr);
        // ---
        sConstantFaderPlugin.reset(nullptr);
        sEaseFaderPlugin.reset(nullptr);
        sEaseInFaderPlugin.reset(nullptr);
        sEaseInOutFaderPlugin.reset(nullptr);
        sEaseOutFaderPlugin.reset(nullptr);
        sExponentialFaderPlugin.reset(nullptr);
        sLinearFaderPlugin.reset(nullptr);
        sCurveSmoothFaderPlugin.reset(nullptr);
        sCurveSharpFaderPlugin.reset(nullptr);
        // ---
        sAMSCodecPlugin.reset(nullptr);
        sMP3CodecPlugin.reset(nullptr);
        sWAVCodecPlugin.reset(nullptr);
        // ---
        sMiniAudioDriverPlugin.reset(nullptr);
        sNullDriverPlugin.reset(nullptr);
        // ---
        sBassBoostFilterPlugin.reset(nullptr);
        sBiquadResonantFilterPlugin.reset(nullptr);
        sDCRemovalFilterPlugin.reset(nullptr);
        sDelayFilterPlugin.reset(nullptr);
        sEqualizerFilterPlugin.reset(nullptr);
        sFlangerFilterPlugin.reset(nullptr);
        sMonoPoleFilterPlugin.reset(nullptr);
        sLofiFilterPlugin.reset(nullptr);
        sRobotizeFilterPlugin.reset(nullptr);
        sWaveShaperFilterPlugin.reset(nullptr);
        //
        sAmbisonicBinauralDecoderNodePlugin.reset(nullptr);
        sAmbisonicMixerNodePlugin.reset(nullptr);
        sAmbisonicPanningNodePlugin.reset(nullptr);
        sAmbisonicRotatorNodePlugin.reset(nullptr);
        sAttenuationNodePlugin.reset(nullptr);
        sClampNodePlugin.reset(nullptr);
        sClipNodePlugin.reset(nullptr);
        sEnvironmentEffectNodePlugin.reset(nullptr);
        sInputNodePlugin.reset(nullptr);
        sNearFieldEffectNodePlugin.reset(nullptr);
        sObstructionNodePlugin.reset(nullptr);
        sOcclusionNodePlugin.reset(nullptr);
        sOutputNodePlugin.reset(nullptr);
        sReflectionsNodePlugin.reset(nullptr);
        sReverbNodePlugin.reset(nullptr);
        sStereoMixerNodePlugin.reset(nullptr);
        sStereoPanningNodePlugin.reset(nullptr);

        return true;
    }

    Engine* Engine::GetInstance()
    {
        // Amplitude Engine unique instance.
        if (gAmplitude == nullptr)
            gAmplitude.reset(ampoolnew(eMemoryPoolKind_Engine, EngineImpl));

        return gAmplitude.get();
    }

    void Engine::DestroyInstance()
    {
        gAmplitude.reset();
    }

    BusInternalState* FindBusInternalState(EngineInternalState* state, AmBusID id)
    {
        if (const auto it = std::ranges::find_if(
                state->buses,
                [&id](const BusInternalState& bus)
                {
                    return bus.GetId() == id;
                });
            it != state->buses.end())
        {
            return &*it;
        }

        return nullptr;
    }

    BusInternalState* FindBusInternalState(EngineInternalState* state, const AmString& name)
    {
        if (const auto it = std::ranges::find_if(
                state->buses,
                [&name](const BusInternalState& bus)
                {
                    return bus.GetName() == name;
                });
            it != state->buses.end())
        {
            return &*it;
        }

        return nullptr;
    }

    static bool PopulateChildBuses(EngineInternalState* state, BusInternalState* parent, const BusIdList* childIdList)
    {
        std::vector<BusInternalState*>* output = &parent->GetChildBuses();

        for (flatbuffers::uoffset_t i = 0; childIdList && i < childIdList->size(); ++i)
        {
            const AmBusID busId = childIdList->Get(i);

            if (BusInternalState* bus = FindBusInternalState(state, busId))
            {
                output->push_back(bus);
            }
            else
            {
                amLogError("Unknown bus with ID '" AM_ID_CHAR_FMT "' listed in child buses.", busId);
                return false;
            }
        }

        return true;
    }

    static bool PopulateDuckBuses(EngineInternalState* state, BusInternalState* parent, const DuckBusDefinitionList* duckBusDefinitionList)
    {
        DuckBusList* output = &parent->GetDuckBuses();

        for (flatbuffers::uoffset_t i = 0; duckBusDefinitionList && i < duckBusDefinitionList->size(); ++i)
        {
            const DuckBusDefinition* duck = duckBusDefinitionList->Get(i);

            if (auto* bus = ampoolnew(eMemoryPoolKind_Engine, DuckBusInternalState, parent); bus->Initialize(duck))
            {
                output->emplace_back(bus);
            }
            else
            {
                ampooldelete(eMemoryPoolKind_Engine, DuckBusInternalState, bus);
                amLogError("Unknown bus with ID '" AM_ID_CHAR_FMT "' listed in duck buses.", duck->id());
                return false;
            }
        }

        return true;
    }

    // The InternalChannelStates have three lists they are a part of: The engine's
    // priority list, the bus's playing sound list, and which free list they are in.
    // Initially, all nodes are in a free list because nothing is playing. Separate
    // free lists are kept for real channels and virtual channels (where 'real'
    // channels are channels that have a channel_id
    static void InitializeChannelFreeLists(
        FreeList* realChannelFreeList,
        FreeList* virtualChannelFreeList,
        std::vector<ChannelInternalState>* channels,
        const AmUInt32 virtualChannels,
        const AmUInt32 realChannels)
    {
        // We do our own tracking of audio channels so that when a new sound is
        // played we can determine if one of the currently playing channels is lower
        // priority so that we can drop it.
        const AmUInt32 totalChannels = realChannels + virtualChannels;
        channels->resize(totalChannels);

        for (AmSize i = 0; i < totalChannels; ++i)
        {
            ChannelInternalState& channel = (*channels)[i];

            // Track real channels separately from virtual channels.
            if (i < realChannels)
            {
                channel.GetRealChannel().Initialize(static_cast<int>(i + 1));
                realChannelFreeList->push_front(channel);
            }
            else
            {
                channel.GetRealChannel().Initialize(kAmInvalidObjectId);
                virtualChannelFreeList->push_front(channel);
            }
        }
    }

    static void InitializeListenerFreeList(
        std::vector<ListenerInternalState*>* listenerStateFreeList, ListenerStateVector* listenerList, const AmUInt32 listSize)
    {
        listenerList->resize(listSize);
        listenerStateFreeList->reserve(listSize);
        for (AmSize i = 0; i < listSize; ++i)
        {
            ListenerInternalState& listener = (*listenerList)[i];
            listenerStateFreeList->push_back(&listener);
        }
    }

    static void InitializeEntityFreeList(
        std::vector<EntityInternalState*>* entityStateFreeList, EntityStateVector* entityList, const AmUInt32 listSize)
    {
        entityList->resize(listSize);
        entityStateFreeList->reserve(listSize);
        for (AmSize i = 0; i < listSize; ++i)
        {
            EntityInternalState& entity = (*entityList)[i];
            entityStateFreeList->push_back(&entity);
        }
    }

    static void InitializeEnvironmentFreeList(
        std::vector<EnvironmentInternalState*>* environmentStateFreeList, EnvironmentStateVector* environmentList, const AmUInt32 listSize)
    {
        environmentList->resize(listSize);
        environmentStateFreeList->reserve(listSize);
        for (AmSize i = 0; i < listSize; ++i)
        {
            EnvironmentInternalState& environment = (*environmentList)[i];
            environmentStateFreeList->push_back(&environment);
        }
    }

    static void InitializeRoomFreeList(
        std::vector<RoomInternalState*>* roomStateFreeList, RoomStateVector* roomList, const AmUInt32 listSize)
    {
        roomList->resize(listSize);
        roomStateFreeList->reserve(listSize);
        for (AmSize i = 0; i < listSize; ++i)
        {
            RoomInternalState& room = (*roomList)[i];
            roomStateFreeList->push_back(&room);
        }
    }

    bool EngineImpl::Initialize(const AmOsString& configFile)
    {
        if (const AmOsString& configFilePath = _fs->ResolvePath(configFile); !LoadFile(_fs->OpenFile(configFilePath), &_configSrc))
        {
            amLogError("Could not load audio config file at path '" AM_OS_CHAR_FMT "'.", configFile.c_str());
            return false;
        }

        return Initialize(GetEngineConfigDefinition());
    }

    bool EngineImpl::Initialize(const EngineConfigDefinition* config)
    {
        if (IsInitialized())
            return true;

        // Lock plugins registries
        Driver::LockRegistry();
        Codec::LockRegistry();
        Resampler::LockRegistry();
        Filter::LockRegistry();
        Fader::LockRegistry();
        Node::LockRegistry();

        _frameThreadMutex = Thread::CreateMutex(500);

        // Create the internal engine state
        _state = ampoolnew(eMemoryPoolKind_Engine, EngineInternalState);
        _state->version = &Amplitude::GetVersion();

        // Load the audio driver
        if (config->driver())
        {
            if (_audioDriver = Driver::Find(config->driver()->str()); _audioDriver == nullptr)
            {
                amLogWarning("Could not load the audio driver '%s'. Loading the default driver.", config->driver()->c_str());
                _audioDriver = Driver::Default();
            }
        }
        else
        {
            _audioDriver = Driver::Default();
        }

        if (_audioDriver == nullptr)
        {
            amLogError("Could not load the audio driver. Loading the null driver as fallback.");
            _audioDriver = sNullDriverPlugin.get();
        }

        if (_audioDriver == nullptr)
        {
            amLogCritical("Failed to load the specified driver, the default driver, and the null driver. Please check your engine "
                          "configuration, and ensure that all the needed plugins are loaded.");
            Deinitialize();
            return false;
        }

        // Load the pipeline from the specified file
        if (const AmOsString& pipelineFilePath =
                _fs->ResolvePath(_fs->Join({ AM_OS_STRING("pipelines"), AM_STRING_TO_OS_STRING(config->mixer()->pipeline()->c_str()) }));
            !_state->pipeline.LoadDefinitionFromPath(pipelineFilePath, _state))
        {
            amLogCritical("Could not load the pipeline asset.");
            Deinitialize();
            return false;
        }

        // Store the panning mode
        _state->panning_mode = static_cast<ePanningMode>(config->mixer()->panning_mode());

        const auto* hrtfConfig = config->hrtf();

        if (hrtfConfig != nullptr)
        {
            // Store the HRIR sampling mode
            _state->hrir_sampling_mode = static_cast<eHRIRSphereSamplingMode>(config->hrtf()->hrir_sampling());

            // Load the HRIR sphere
            _state->hrir_sphere = ampoolnew(eMemoryPoolKind_Engine, HRIRSphereImpl);
            _state->hrir_sphere->SetResource(AM_STRING_TO_OS_STRING(config->hrtf()->amir_file()->c_str()));
            _state->hrir_sphere->SetSamplingMode(_state->hrir_sampling_mode);
            _state->hrir_sphere->Load(GetFileSystem());
        }
        else if (_state->panning_mode != ePanningMode_Stereo)
        {
            amLogCritical("The HRTF configuration is missing, but the panning mode is not stereo. Please provide an HRTF configuration, or "
                          "set the panning mode to Stereo.");
            Deinitialize();
            return false;
        }

        // Initialize audio mixer
        if (!_state->mixer.Init(config))
        {
            amLogCritical("Could not initialize the audio mixer.");
            Deinitialize();
            return false;
        }

        // Initialize the channel internal data.
        InitializeChannelFreeLists(
            &_state->real_channel_free_list, &_state->virtual_channel_free_list, &_state->channel_state_memory,
            config->mixer()->virtual_channels(), config->mixer()->active_channels());

        // Initialize the listener internal data.
        InitializeListenerFreeList(&_state->listener_state_free_list, &_state->listener_state_memory, config->game()->listeners());

        // Initialize the entity internal data.
        InitializeEntityFreeList(&_state->entity_state_free_list, &_state->entity_state_memory, config->game()->entities());

        // Initialize the environment internal data.
        InitializeEnvironmentFreeList(
            &_state->environment_state_free_list, &_state->environment_state_memory, config->game()->environments());

        // Initialize the room internal data.
        InitializeRoomFreeList(&_state->room_state_free_list, &_state->room_state_memory, config->game()->rooms());

        // Load the audio buses.
        if (const AmOsString& busesFilePath = _fs->ResolvePath(AM_STRING_TO_OS_STRING(config->buses_file()->c_str()));
            !LoadFile(_fs->OpenFile(busesFilePath), &_state->buses_source))
        {
            amLogCritical("Could not load audio bus file.");
            Deinitialize();
            return false;
        }
        const BusDefinitionList* busDefList = Amplitude::GetBusDefinitionList(_state->buses_source.c_str());
        const auto busCount = busDefList->buses()->size();
        _state->buses.resize(busCount);
        for (flatbuffers::uoffset_t i = 0; i < busCount; ++i)
        {
            _state->buses[i].Initialize(busDefList->buses()->Get(i));
        }

        // Set up the children and ducking pointers.
        for (auto& bus : _state->buses)
        {
            const BusDefinition* def = bus.GetBusDefinition();
            if (!PopulateChildBuses(_state, &bus, def->child_buses()))
            {
                Deinitialize();
                return false;
            }
            if (!PopulateDuckBuses(_state, &bus, def->duck_buses()))
            {
                Deinitialize();
                return false;
            }
        }

        // Fetch the master bus by ID
        _state->master_bus = FindBusInternalState(_state, kAmMasterBusId);
        if (!_state->master_bus)
        {
            // Fetch the master bus by name
            _state->master_bus = FindBusInternalState(_state, "master");
            if (!_state->master_bus)
            {
                amLogCritical("Unable to find a master bus.");
                Deinitialize();
                return false;
            }
        }

        // Set the listener fetch mode
        _state->listener_fetch_mode = config->game()->listener_fetch_mode();

        // Doppler effect settings
        _state->sound_speed = config->game()->sound_speed();
        _state->doppler_factor = config->game()->doppler_factor();

        // Samples per streams
        _state->samples_per_stream = config->output()->buffer_size() / 2;

        // Save obstruction/occlusion configurations
        _state->obstruction_config.Init(config->game()->obstruction());
        _state->occlusion_config.Init(config->game()->occlusion());

        // Environment Amounts
        _state->track_environments = config->game()->track_environments();

        // Engine state
        _state->paused = false;
        _state->mute = false;
        _state->master_gain = 1.0f;

        // Open the audio device through the driver
        if (!_audioDriver->Open(_state->mixer.GetDeviceDescription()))
        {
            amLogError(
                "Could not open the audio device using the '%s' driver. Loading the null driver as fallback.",
                _audioDriver->GetName().c_str());
            _audioDriver = sNullDriverPlugin.get();

            if (_audioDriver == nullptr || !_audioDriver->Open(_state->mixer.GetDeviceDescription()))
            {
                amLogCritical("Could not open the audio device.");
                Deinitialize();
                return false;
            }
        }

        amLogDebug("Amplitude Engine initialized successfully.");
        return true;
    }

    bool EngineImpl::Deinitialize()
    {
        if (_state == nullptr)
            return true;

        _state->stopping = true;

        // Stop all sounds
        StopAll();

        // Process one last frame to update stopped channel states
        AdvanceFrame(0);

        // Pause the engine so no further updates are made
        _state->paused = true;

        // Release channels
        EraseFinishedSounds(_state);

        // Close the audio device through the driver
        if (_audioDriver != nullptr)
            _audioDriver->Close();

        if (_state->mixer.IsInitialized())
            _state->mixer.Deinit();

        // Unload sound banks
        while (HasLoadedSoundBanks())
            UnloadSoundBanks();

        // Release HRIR sphere
        if (_state->hrir_sphere != nullptr)
        {
            ampooldelete(eMemoryPoolKind_Engine, HRIRSphereImpl, _state->hrir_sphere);
            _state->hrir_sphere = nullptr;
        }

        ampooldelete(eMemoryPoolKind_Engine, EngineInternalState, _state);
        _state = nullptr;

        Thread::DestroyMutex(_frameThreadMutex);

        // Unlock registries
        Driver::UnlockRegistry();
        Codec::UnlockRegistry();
        Resampler::UnlockRegistry();
        Filter::UnlockRegistry();
        Fader::UnlockRegistry();
        Node::UnlockRegistry();

        return true;
    }

    bool EngineImpl::IsInitialized() const
    {
        // An initialized engine have a running state
        return _state != nullptr && !_state->stopping;
    }

    void EngineImpl::SetFileSystem(FileSystem* fs)
    {
        _fs = fs;
    }

    const FileSystem* EngineImpl::GetFileSystem() const
    {
        return _fs;
    }

    bool EngineImpl::LoadSoundBank(const AmOsString& filename)
    {
        AmBankID outID = kAmInvalidObjectId;
        return LoadSoundBank(filename, outID);
    }

    bool EngineImpl::LoadSoundBank(const AmOsString& filename, AmBankID& outID)
    {
        outID = kAmInvalidObjectId;
        bool success = true;

        if (const auto findIt = _state->sound_bank_id_map.find(filename); findIt == _state->sound_bank_id_map.end() ||
            (findIt != _state->sound_bank_id_map.end() && !_state->sound_bank_map.contains(findIt->second)))
        {
            AmUniquePtr<eMemoryPoolKind_Engine, SoundBank> soundBank(ampoolnew(eMemoryPoolKind_Engine, SoundBank));
            success = soundBank->Initialize(filename, this);

            if (success)
            {
                soundBank->GetRefCounter()->Increment();

                const AmBankID id = soundBank->GetId();
                _state->sound_bank_id_map[filename] = id;
                _state->sound_bank_map[id] = std::move(soundBank);
                outID = id;
            }
            else
            {
                _state->sound_bank_id_map.erase(findIt);
            }
        }
        else
        {
            _state->sound_bank_map[findIt->second]->GetRefCounter()->Increment();
        }

        return success;
    }

    bool EngineImpl::LoadSoundBankFromMemory(const char* fileData)
    {
        AmBankID outID = kAmInvalidObjectId;
        return LoadSoundBankFromMemory(fileData, outID);
    }

    bool EngineImpl::LoadSoundBankFromMemory(const char* fileData, AmBankID& outID)
    {
        outID = kAmInvalidObjectId;
        bool success = true;

        AmUniquePtr<eMemoryPoolKind_Engine, SoundBank> soundBank(ampoolnew(eMemoryPoolKind_Engine, SoundBank));
        const AmOsString filename = AM_STRING_TO_OS_STRING(soundBank->GetName());
        if (const auto findIt = _state->sound_bank_id_map.find(filename); findIt == _state->sound_bank_id_map.end() ||
            (findIt != _state->sound_bank_id_map.end() && !_state->sound_bank_map.contains(findIt->second)))
        {
            success = soundBank->InitializeFromMemory(fileData, this);

            if (success)
            {
                soundBank->GetRefCounter()->Increment();

                const AmBankID id = soundBank->GetId();
                _state->sound_bank_id_map[filename] = id;
                _state->sound_bank_map[id] = std::move(soundBank);
                outID = id;
            }
            else
            {
                _state->sound_bank_id_map.erase(findIt);
            }
        }
        else
        {
            _state->sound_bank_map[findIt->second]->GetRefCounter()->Increment();
        }

        return success;
    }

    bool EngineImpl::LoadSoundBankFromMemoryView(void* ptr, AmSize size)
    {
        AmBankID outID = kAmInvalidObjectId;
        return LoadSoundBankFromMemoryView(ptr, size, outID);
    }

    bool EngineImpl::LoadSoundBankFromMemoryView(void* ptr, AmSize size, AmBankID& outID)
    {
        outID = kAmInvalidObjectId;

        MemoryFile mf;
        AmString dst;

        mf.OpenMem(static_cast<AmConstUInt8Buffer>(ptr), size, false, false);
        dst.assign(size + 1, 0);

        if (const AmUInt32 len = mf.Read(reinterpret_cast<AmUInt8Buffer>(&dst[0]), mf.Length()); len != size)
            return false;

        return LoadSoundBankFromMemory(dst.c_str(), outID);
    }

    void EngineImpl::UnloadSoundBank(const AmOsString& filename)
    {
        if (const auto findIt = _state->sound_bank_id_map.find(filename); findIt == _state->sound_bank_id_map.end())
        {
            amLogWarning("Cannot unload Soundbank '" AM_OS_CHAR_FMT "'. Soundbank not loaded.", filename.c_str());
            AMPLITUDE_ASSERT(false);
        }
        else
        {
            UnloadSoundBank(findIt->second);
        }
    }

    void EngineImpl::UnloadSoundBank(AmBankID id)
    {
        if (const auto findIt = _state->sound_bank_map.find(id); findIt == _state->sound_bank_map.end())
        {
            amLogWarning("Cannot unload Soundbank with ID '" AM_ID_CHAR_FMT "'. Soundbank not loaded.", id);
            AMPLITUDE_ASSERT(false);
        }
        else if (findIt->second->GetRefCounter()->Decrement() == 0)
        {
            Thread::LockMutex(_frameThreadMutex);

            findIt->second->Deinitialize(this);
            _state->sound_bank_map.erase(id);

            Thread::UnlockMutex(_frameThreadMutex);
        }
    }

    void EngineImpl::UnloadSoundBanks()
    {
        Thread::LockMutex(_frameThreadMutex);

        std::vector<AmBankID> idsToDelete;
        idsToDelete.reserve(_state->sound_bank_map.size());

        for (const auto& item : _state->sound_bank_map | std::ranges::views::values)
        {
            if (RefCounter* ref = item->GetRefCounter(); ref->GetCount() > 0 && ref->Decrement() == 0)
            {
                item->Deinitialize(this);
                idsToDelete.push_back(item->GetId());
            }
        }

        for (const auto id : idsToDelete)
            _state->sound_bank_map.erase(id);

        Thread::UnlockMutex(_frameThreadMutex);
    }

    bool EngineImpl::HasLoadedSoundBank(const AmOsString& filename) const
    {
        if (const auto findIt = _state->sound_bank_id_map.find(filename); findIt != _state->sound_bank_id_map.end())
            return _state->sound_bank_map.contains(findIt->second);

        return false;
    }

    bool EngineImpl::HasLoadedSoundBank(AmBankID id) const
    {
        return _state->sound_bank_map.contains(id);
    }

    bool EngineImpl::HasLoadedSoundBanks() const
    {
        return !_state->sound_bank_map.empty();
    }

    void EngineImpl::StartOpenFileSystem()
    {
        _fs->StartOpenFileSystem();
    }

    bool EngineImpl::TryFinalizeOpenFileSystem()
    {
        return _fs->TryFinalizeOpenFileSystem();
    }

    void EngineImpl::StartCloseFileSystem()
    {
        _fs->StartCloseFileSystem();
    }

    bool EngineImpl::TryFinalizeCloseFileSystem()
    {
        return _fs->TryFinalizeCloseFileSystem();
    }

    void EngineImpl::StartLoadSoundFiles()
    {
        if (_soundLoaderThreadPool == nullptr)
            _soundLoaderThreadPool.reset(ampoolnew(eMemoryPoolKind_Engine, Thread::Pool));

        _soundLoaderThreadPool->Init(8);

        for (const auto& item : _state->sound_bank_map)
        {
            auto task = std::shared_ptr<LoadSoundBankTask>(
                ampoolnew(eMemoryPoolKind_Engine, LoadSoundBankTask, item.second.get()),
                am_delete<eMemoryPoolKind_Engine, LoadSoundBankTask>{});

            _soundLoaderThreadPool->AddTask(task);
        }
    }

    bool EngineImpl::TryFinalizeLoadSoundFiles()
    {
        if (_soundLoaderThreadPool == nullptr)
            return true;

        if (_soundLoaderThreadPool->HasTasks())
            return false;

        _soundLoaderThreadPool.reset(nullptr);
        return true;
    }

    ListenerInternalState* FindBestListener(ListenerList& listeners, const AmVec3& location, eListenerFetchMode fetchMode)
    {
        if (listeners.empty())
            return nullptr;

        ListenerList::iterator bestListener;
        const AmVec4 location4 = AM_V4V(location, 1.0f);

        switch (fetchMode)
        {
        case eListenerFetchMode_None:
            return nullptr;

        case eListenerFetchMode_Nearest:
            [[fallthrough]];
        case eListenerFetchMode_Farthest:
            {
                auto listener = listeners.begin();
                auto listenerSpaceLocation = AM_Mul(listener->GetInverseMatrix(), location4).XYZ;
                AmReal32 distanceSquared = AM_LenSqr(listenerSpaceLocation);
                bestListener = listener;

                for (++listener; listener != listeners.end(); ++listener)
                {
                    const AmVec3 transformedLocation = AM_Mul(listener->GetInverseMatrix(), location4).XYZ;
                    if (const AmReal32 magnitudeSquared = AM_LenSqr(transformedLocation);
                        fetchMode == eListenerFetchMode_Nearest ? magnitudeSquared < distanceSquared : magnitudeSquared > distanceSquared)
                    {
                        bestListener = listener;
                        distanceSquared = magnitudeSquared;
                    }
                }
            }
            break;

        case eListenerFetchMode_First:
            {
                bestListener = listeners.begin();
            }
            break;

        case eListenerFetchMode_Last:
            {
                bestListener = listeners.end();
            }
            break;

        case eListenerFetchMode_Default:
            {
                ListenerInternalState* state = amEngine->GetDefaultListener().GetState();
                if (state == nullptr)
                    return nullptr;

                for (auto listener = listeners.begin(); listener != listeners.end(); ++listener)
                    if (listener->GetId() == state->GetId())
                        return &*listener;

                return nullptr;
            }
        }

        return &*bestListener;
    }

    AmVec2 CalculatePan(const AmVec3& listenerSpaceLocation)
    {
        if (AM_LenSqr(listenerSpaceLocation) <= kEpsilon)
            return AM_V2(0.0f, 0.0f);

        const AmVec3 direction = AM_Norm(listenerSpaceLocation);
        return AM_V2(AM_Dot(AM_V3(1, 0, 0), direction), AM_Dot(AM_V3(0, 1, 0), direction));
    }

    static void CalculateGainPanPitch(
        AmReal32* gain,
        AmVec2* pan,
        AmReal32* pitch,
        const ListenerInternalState* listener,
        const ChannelInternalState* channel,
        const AmReal32 soundGain,
        const AmReal32 soundPitch,
        const BusInternalState* bus,
        const Spatialization spatialization,
        const AmReal32 userGain)
    {
        *gain = soundGain * bus->GetGain() * userGain;
        *pitch = soundPitch;
        *pan = AM_V2(0, 0); // TODO: This may be removed in the future, since panning is handled automatically in pipeline nodes..

        if (spatialization != Spatialization_None && listener != nullptr && channel != nullptr)
            *pitch *= channel->GetDopplerFactor(listener->GetId());
    }

    // Given the priority of a node, and the list of ChannelInternalStates sorted by
    // priority, find the location in the list where the node would be inserted.
    // Note that the node should be inserted using InsertAfter. If the node you want
    // to insert turns out to be the highest priority node, this will return the
    // list terminator (and inserting after the terminator will put it at the front
    // of the list).
    PriorityList::iterator FindInsertionPoint(PriorityList* list, const AmReal32 priority)
    {
        PriorityList::reverse_iterator it;

        for (it = list->rbegin(); it != list->rend(); ++it)
            if (const AmReal32 p = it->Priority(); p > priority)
                break;

        return it.base();
    }

    // Given a location to insert a node, take an ChannelInternalState from the
    // appropriate list and insert it there. Return the new ChannelInternalState.
    //
    // There are three places an ChannelInternalState may be taken from. First, if
    // there are any real channels available in the real channel free list, use one
    // of those so that your channel can play.
    //
    // If there are no real channels, then use a free virtual channel instead so
    // that your channel can at least be tracked.
    //
    // If there are no real or virtual channels, use the node in the priority list,
    // remove it from the list, and insert it in the new insertion point. This
    // causes the lowest priority sound to stop being tracked.
    //
    // If the node you are trying to insert is the lowest priority, do nothing and
    // return a nullptr.
    //
    // This function could use some unit tests b/20752976
    static ChannelInternalState* FindFreeChannelInternalState(
        PriorityList::iterator insertionPoint,
        PriorityList* list,
        FreeList* realChannelFreeList,
        FreeList* virtualChannelFreeList,
        const bool paused)
    {
        ChannelInternalState* newChannel = nullptr;
        // Grab a free ChannelInternalState if there is one and the engine is not
        // paused. The engine is paused, grab a virtual channel for now, and it will
        // fix itself when the engine is not paused.
        if (!paused && !realChannelFreeList->empty())
        {
            newChannel = &realChannelFreeList->front();
            realChannelFreeList->pop_front();
            PriorityList::insert_before(*insertionPoint, *newChannel, &ChannelInternalState::priority_node);
        }
        else if (!virtualChannelFreeList->empty())
        {
            newChannel = &virtualChannelFreeList->front();
            virtualChannelFreeList->pop_front();
            PriorityList::insert_before(*insertionPoint, *newChannel, &ChannelInternalState::priority_node);
        }
        else if (&*insertionPoint != &list->back())
        {
            // If there are no free sounds, and the new sound is not the lowest priority
            // sound, evict the lowest priority sound.
            newChannel = &list->back();
            newChannel->Halt();

            // Move it to a new spot in the list if it needs to be moved.
            if (&*insertionPoint != newChannel)
            {
                list->pop_back();
                list->insert(insertionPoint, *newChannel);
            }
        }

        return newChannel;
    }

    Channel EngineImpl::Play(SwitchContainerHandle handle) const
    {
        return Play(handle, AM_V3(0, 0, 0), 1.0f);
    }

    Channel EngineImpl::Play(SwitchContainerHandle handle, const AmVec3& location) const
    {
        return Play(handle, location, 1.0f);
    }

    Channel EngineImpl::Play(SwitchContainerHandle handle, const AmVec3& location, const AmReal32 userGain) const
    {
        return PlayScopedSwitchContainer(handle, Entity(nullptr), location, userGain);
    }

    Channel EngineImpl::Play(SwitchContainerHandle handle, const Entity& entity) const
    {
        return Play(handle, entity, 1.0f);
    }

    Channel EngineImpl::Play(SwitchContainerHandle handle, const Entity& entity, const AmReal32 userGain) const
    {
        return PlayScopedSwitchContainer(handle, entity, entity.GetLocation(), userGain);
    }

    Channel EngineImpl::Play(CollectionHandle handle) const
    {
        return Play(handle, AM_V3(0, 0, 0), 1.0f);
    }

    Channel EngineImpl::Play(CollectionHandle handle, const AmVec3& location) const
    {
        return Play(handle, location, 1.0f);
    }

    Channel EngineImpl::Play(CollectionHandle handle, const AmVec3& location, const AmReal32 userGain) const
    {
        return PlayScopedCollection(handle, Entity(nullptr), location, userGain);
    }

    Channel EngineImpl::Play(CollectionHandle handle, const Entity& entity) const
    {
        return Play(handle, entity, 1.0f);
    }

    Channel EngineImpl::Play(CollectionHandle handle, const Entity& entity, const AmReal32 userGain) const
    {
        return PlayScopedCollection(handle, entity, entity.GetLocation(), userGain);
    }

    Channel EngineImpl::Play(SoundHandle handle) const
    {
        return Play(handle, AM_V3(0, 0, 0), 1.0f);
    }

    Channel EngineImpl::Play(SoundHandle handle, const AmVec3& location) const
    {
        return Play(handle, location, 1.0f);
    }

    Channel EngineImpl::Play(SoundHandle handle, const AmVec3& location, AmReal32 userGain) const
    {
        return PlayScopedSound(handle, Entity(nullptr), location, userGain);
    }

    Channel EngineImpl::Play(SoundHandle handle, const Entity& entity) const
    {
        return Play(handle, entity, 1.0f);
    }

    Channel EngineImpl::Play(SoundHandle handle, const Entity& entity, AmReal32 userGain) const
    {
        return PlayScopedSound(handle, entity, entity.GetLocation(), userGain);
    }

    Channel EngineImpl::Play(const AmString& name) const
    {
        return Play(name, AM_V3(0, 0, 0), 1.0f);
    }

    Channel EngineImpl::Play(const AmString& name, const AmVec3& location) const
    {
        return Play(name, location, 1.0f);
    }

    Channel EngineImpl::Play(const AmString& name, const AmVec3& location, const AmReal32 userGain) const
    {
        if (SoundHandle handle = GetSoundHandle(name))
            return Play(handle, location, userGain);

        if (CollectionHandle handle = GetCollectionHandle(name))
            return Play(handle, location, userGain);

        if (SwitchContainerHandle handle = GetSwitchContainerHandle(name))
            return Play(handle, location, userGain);

        amLogError("Cannot play object: invalid name (%s).", name.c_str());
        return Channel(nullptr);
    }

    Channel EngineImpl::Play(const AmString& name, const Entity& entity) const
    {
        return Play(name, entity, 1.0f);
    }

    Channel EngineImpl::Play(const AmString& name, const Entity& entity, const AmReal32 userGain) const
    {
        if (SoundHandle handle = GetSoundHandle(name))
            return Play(handle, entity, userGain);

        if (CollectionHandle handle = GetCollectionHandle(name))
            return Play(handle, entity, userGain);

        if (SwitchContainerHandle handle = GetSwitchContainerHandle(name))
            return Play(handle, entity, userGain);

        amLogError("Cannot play sound: invalid name (%s).", name.c_str());
        return Channel(nullptr);
    }

    Channel EngineImpl::Play(AmObjectID id) const
    {
        return Play(id, AM_V3(0, 0, 0), 1.0f);
    }

    Channel EngineImpl::Play(AmObjectID id, const AmVec3& location) const
    {
        return Play(id, location, 1.0f);
    }

    Channel EngineImpl::Play(AmObjectID id, const AmVec3& location, const AmReal32 userGain) const
    {
        if (SoundHandle handle = GetSoundHandle(id))
            return Play(handle, location, userGain);

        if (CollectionHandle handle = GetCollectionHandle(id))
            return Play(handle, location, userGain);

        if (SwitchContainerHandle handle = GetSwitchContainerHandle(id))
            return Play(handle, location, userGain);

        amLogError("Cannot play sound: invalid ID (" AM_ID_CHAR_FMT ").", id);
        return Channel(nullptr);
    }

    Channel EngineImpl::Play(AmObjectID id, const Entity& entity) const
    {
        return Play(id, entity, 1.0f);
    }

    Channel EngineImpl::Play(AmObjectID id, const Entity& entity, const AmReal32 userGain) const
    {
        if (SoundHandle handle = GetSoundHandle(id))
            return Play(handle, entity, userGain);

        if (CollectionHandle handle = GetCollectionHandle(id))
            return Play(handle, entity, userGain);

        if (SwitchContainerHandle handle = GetSwitchContainerHandle(id))
            return Play(handle, entity, userGain);

        amLogError("Cannot play sound: invalid ID (" AM_ID_CHAR_FMT ").", id);
        return Channel(nullptr);
    }

    void EngineImpl::StopAll() const
    {
        for (auto&& channel : _state->channel_state_memory)
            if (channel.Valid() && channel.Playing())
                channel.Halt();
    }

    EventCanceler EngineImpl::Trigger(EventHandle handle, const Entity& entity) const
    {
        if (handle == nullptr)
        {
            amLogError("Cannot trigger event: Invalid event handle.");
            return EventCanceler(nullptr);
        }

        EventInstanceImpl instance = static_cast<EventImpl*>(handle)->Trigger(entity);
        _state->running_events.push_back(std::move(instance));

        return EventCanceler(&_state->running_events.back());
    }

    EventCanceler EngineImpl::Trigger(const AmString& name, const Entity& entity) const
    {
        if (EventHandle handle = GetEventHandle(name))
            return Trigger(handle, entity);

        amLogError("Cannot trigger event: invalid name (%s).", name.c_str());
        return EventCanceler(nullptr);
    }

    EventCanceler EngineImpl::Trigger(AmEventID id, const Entity& entity) const
    {
        if (EventHandle handle = GetEventHandle(id))
            return Trigger(handle, entity);

        amLogError("Cannot trigger event: invalid ID (" AM_ID_CHAR_FMT ").", id);
        return EventCanceler(nullptr);
    }

    void EngineImpl::SetSwitchState(SwitchHandle handle, AmObjectID stateId) const
    {
        if (handle == nullptr)
        {
            amLogError("Cannot update switch state: Invalid switch handle.");
            return;
        }

        handle->SetState(stateId);
    }

    void EngineImpl::SetSwitchState(SwitchHandle handle, const AmString& stateName) const
    {
        if (handle == nullptr)
        {
            amLogError("Cannot update switch state: Invalid switch handle.");
            return;
        }

        handle->SetState(stateName);
    }

    void EngineImpl::SetSwitchState(SwitchHandle handle, const SwitchState& state) const
    {
        if (handle == nullptr)
        {
            amLogError("Cannot update switch state: Invalid switch handle.");
            return;
        }

        handle->SetState(state);
    }

    void EngineImpl::SetSwitchState(AmSwitchID id, AmObjectID stateId) const
    {
        if (SwitchHandle handle = GetSwitchHandle(id))
            return SetSwitchState(handle, stateId);

        amLogError("Cannot update switch: Invalid ID (" AM_ID_CHAR_FMT ").", id);
    }

    void EngineImpl::SetSwitchState(AmSwitchID id, const AmString& stateName) const
    {
        if (SwitchHandle handle = GetSwitchHandle(id))
            return SetSwitchState(handle, stateName);

        amLogError("Cannot update switch: Invalid ID (" AM_ID_CHAR_FMT ").", id);
    }

    void EngineImpl::SetSwitchState(AmSwitchID id, const SwitchState& state) const
    {
        if (SwitchHandle handle = GetSwitchHandle(id))
            return SetSwitchState(handle, state);

        amLogError("Cannot update switch: Invalid ID (" AM_ID_CHAR_FMT ").", id);
    }

    void EngineImpl::SetSwitchState(const AmString& name, AmObjectID stateId) const
    {
        if (SwitchHandle handle = GetSwitchHandle(name))
            return SetSwitchState(handle, stateId);

        amLogError("Cannot update switch: Invalid name (%s).", name.c_str());
    }

    void EngineImpl::SetSwitchState(const AmString& name, const AmString& stateName) const
    {
        if (SwitchHandle handle = GetSwitchHandle(name))
            return SetSwitchState(handle, stateName);

        amLogError("Cannot update switch: Invalid name (%s).", name.c_str());
    }

    void EngineImpl::SetSwitchState(const AmString& name, const SwitchState& state) const
    {
        if (SwitchHandle handle = GetSwitchHandle(name))
            return SetSwitchState(handle, state);

        amLogError("Cannot update switch: Invalid name (%s).", name.c_str());
    }

    void EngineImpl::SetRtpcValue(RtpcHandle handle, double value) const
    {
        if (handle == nullptr)
        {
            amLogError("Cannot update RTPC value: Invalid RTPC handle.");
            return;
        }

        handle->SetValue(value);
    }

    void EngineImpl::SetRtpcValue(AmRtpcID id, double value) const
    {
        if (RtpcHandle handle = GetRtpcHandle(id))
            return SetRtpcValue(handle, value);

        amLogError("Cannot update RTPC value: Invalid RTPC ID (" AM_ID_CHAR_FMT ").", id);
    }

    void EngineImpl::SetRtpcValue(const AmString& name, double value) const
    {
        if (RtpcHandle handle = GetRtpcHandle(name))
            return SetRtpcValue(handle, value);

        amLogError("Cannot update RTPC value: Invalid RTPC name (%s).", name.c_str());
    }

    SwitchContainerHandle EngineImpl::GetSwitchContainerHandle(const AmString& name) const
    {
        const auto pair = std::ranges::find_if(
            _state->switch_container_map,
            [&name](const auto& item)
            {
                return item.second->GetName() == name;
            });

        return pair == _state->switch_container_map.end() ? nullptr : pair->second.get();
    }

    SwitchContainerHandle EngineImpl::GetSwitchContainerHandle(AmSwitchContainerID id) const
    {
        const auto pair = _state->switch_container_map.find(id);
        return pair == _state->switch_container_map.end() ? nullptr : pair->second.get();
    }

    SwitchContainerHandle EngineImpl::GetSwitchContainerHandleFromFile(const AmOsString& filename) const
    {
        const auto pair = _state->switch_container_id_map.find(filename);
        return pair == _state->switch_container_id_map.end() ? nullptr : GetSwitchContainerHandle(pair->second);
    }

    CollectionHandle EngineImpl::GetCollectionHandle(const AmString& name) const
    {
        const auto pair = std::ranges::find_if(
            _state->collection_map,
            [&name](const auto& item)
            {
                return item.second->GetName() == name;
            });

        return pair == _state->collection_map.end() ? nullptr : pair->second.get();
    }

    CollectionHandle EngineImpl::GetCollectionHandle(AmCollectionID id) const
    {
        const auto pair = _state->collection_map.find(id);
        return pair == _state->collection_map.end() ? nullptr : pair->second.get();
    }

    CollectionHandle EngineImpl::GetCollectionHandleFromFile(const AmOsString& filename) const
    {
        const auto pair = _state->collection_id_map.find(filename);
        return pair == _state->collection_id_map.end() ? nullptr : GetCollectionHandle(pair->second);
    }

    SoundHandle EngineImpl::GetSoundHandle(const AmString& name) const
    {
        const auto pair = std::ranges::find_if(
            _state->sound_map,
            [&name](const auto& item)
            {
                return item.second->GetName() == name;
            });

        return pair == _state->sound_map.end() ? nullptr : pair->second.get();
    }

    SoundHandle EngineImpl::GetSoundHandle(AmSoundID id) const
    {
        const auto pair = _state->sound_map.find(id);
        return pair == _state->sound_map.end() ? nullptr : pair->second.get();
    }

    SoundHandle EngineImpl::GetSoundHandleFromFile(const AmOsString& filename) const
    {
        const auto pair = _state->sound_id_map.find(filename);
        return pair == _state->sound_id_map.end() ? nullptr : GetSoundHandle(pair->second);
    }

    SoundObjectHandle EngineImpl::GetSoundObjectHandle(const AmString& name) const
    {
        if (const SoundHandle handle = GetSoundHandle(name))
            return handle;

        if (const CollectionHandle handle = GetCollectionHandle(name))
            return handle;

        if (const SwitchContainerHandle handle = GetSwitchContainerHandle(name))
            return handle;

        return nullptr;
    }

    SoundObjectHandle EngineImpl::GetSoundObjectHandle(AmSoundID id) const
    {
        if (const SoundHandle handle = GetSoundHandle(id))
            return handle;

        if (const CollectionHandle handle = GetCollectionHandle(id))
            return handle;

        if (const SwitchContainerHandle handle = GetSwitchContainerHandle(id))
            return handle;

        return nullptr;
    }

    SoundObjectHandle EngineImpl::GetSoundObjectHandleFromFile(const AmOsString& filename) const
    {
        if (const SoundHandle handle = GetSoundHandleFromFile(filename))
            return handle;

        if (const CollectionHandle handle = GetCollectionHandleFromFile(filename))
            return handle;

        if (const SwitchContainerHandle handle = GetSwitchContainerHandleFromFile(filename))
            return handle;

        return nullptr;
    }

    EventHandle EngineImpl::GetEventHandle(const AmString& name) const
    {
        const auto pair = std::ranges::find_if(
            _state->event_map,
            [&name](const auto& item)
            {
                return item.second->GetName() == name;
            });

        return pair == _state->event_map.end() ? nullptr : pair->second.get();
    }

    EventHandle EngineImpl::GetEventHandle(AmEventID id) const
    {
        const auto pair = _state->event_map.find(id);
        return pair == _state->event_map.end() ? nullptr : pair->second.get();
    }

    EventHandle EngineImpl::GetEventHandleFromFile(const AmOsString& filename) const
    {
        const auto pair = _state->event_id_map.find(filename);
        return pair == _state->event_id_map.end() ? nullptr : GetEventHandle(pair->second);
    }

    AttenuationHandle EngineImpl::GetAttenuationHandle(const AmString& name) const
    {
        const auto pair = std::ranges::find_if(
            _state->attenuation_map,
            [&name](const auto& item)
            {
                return item.second->GetName() == name;
            });

        return pair == _state->attenuation_map.end() ? nullptr : pair->second.get();
    }

    AttenuationHandle EngineImpl::GetAttenuationHandle(AmAttenuationID id) const
    {
        const auto pair = _state->attenuation_map.find(id);
        return pair == _state->attenuation_map.end() ? nullptr : pair->second.get();
    }

    AttenuationHandle EngineImpl::GetAttenuationHandleFromFile(const AmOsString& filename) const
    {
        const auto pair = _state->attenuation_id_map.find(filename);
        return pair == _state->attenuation_id_map.end() ? nullptr : GetAttenuationHandle(pair->second);
    }

    SwitchHandle EngineImpl::GetSwitchHandle(const AmString& name) const
    {
        const auto pair = std::ranges::find_if(
            _state->switch_map,
            [&name](const auto& item)
            {
                return item.second->GetName() == name;
            });

        return pair == _state->switch_map.end() ? nullptr : pair->second.get();
    }

    SwitchHandle EngineImpl::GetSwitchHandle(AmSwitchID id) const
    {
        const auto pair = _state->switch_map.find(id);
        return pair == _state->switch_map.end() ? nullptr : pair->second.get();
    }

    SwitchHandle EngineImpl::GetSwitchHandleFromFile(const AmOsString& filename) const
    {
        const auto pair = _state->switch_id_map.find(filename);
        return pair == _state->switch_id_map.end() ? nullptr : GetSwitchHandle(pair->second);
    }

    RtpcHandle EngineImpl::GetRtpcHandle(const AmString& name) const
    {
        const auto pair = std::ranges::find_if(
            _state->rtpc_map,
            [&name](const auto& item)
            {
                return item.second->GetName() == name;
            });

        return pair == _state->rtpc_map.end() ? nullptr : pair->second.get();
    }

    RtpcHandle EngineImpl::GetRtpcHandle(AmRtpcID id) const
    {
        const auto pair = _state->rtpc_map.find(id);
        return pair == _state->rtpc_map.end() ? nullptr : pair->second.get();
    }

    RtpcHandle EngineImpl::GetRtpcHandleFromFile(const AmOsString& filename) const
    {
        const auto pair = _state->rtpc_id_map.find(filename);
        return pair == _state->rtpc_id_map.end() ? nullptr : GetRtpcHandle(pair->second);
    }

    EffectHandle EngineImpl::GetEffectHandle(const AmString& name) const
    {
        const auto pair = std::ranges::find_if(
            _state->effect_map,
            [&name](const auto& item)
            {
                return item.second->GetName() == name;
            });

        return pair == _state->effect_map.end() ? nullptr : pair->second.get();
    }

    EffectHandle EngineImpl::GetEffectHandle(AmEffectID id) const
    {
        const auto pair = _state->effect_map.find(id);
        return pair == _state->effect_map.end() ? nullptr : pair->second.get();
    }

    EffectHandle EngineImpl::GetEffectHandleFromFile(const AmOsString& filename) const
    {
        const auto pair = _state->effect_id_map.find(filename);
        return pair == _state->effect_id_map.end() ? nullptr : GetEffectHandle(pair->second);
    }

    PipelineHandle EngineImpl::GetPipelineHandle() const
    {
        return &_state->pipeline;
    }

    void EngineImpl::SetMasterGain(const AmReal32 gain) const
    {
        _state->master_gain = gain;
        _state->mixer.SetMasterGain(gain);
    }

    AmReal32 EngineImpl::GetMasterGain() const
    {
        return _state->master_gain;
    }

    void EngineImpl::SetMute(const bool mute) const
    {
        _state->mute = mute;
    }

    bool EngineImpl::IsMuted() const
    {
        return _state->mute;
    }

    void EngineImpl::SetDefaultListener(const Listener* listener)
    {
        if (listener == nullptr)
            _defaultListener = nullptr;

        else if (listener->Valid())
            _defaultListener = listener->GetState();
    }

    void EngineImpl::SetDefaultListener(AmListenerID id)
    {
        if (id == kAmInvalidObjectId)
            return;

        if (const auto findIt = std::ranges::find_if(
                _state->listener_state_memory,
                [&id](const ListenerInternalState& state)
                {
                    return state.GetId() == id;
                });
            findIt != _state->listener_state_memory.end())
        {
            _defaultListener = (&*findIt);
        }
    }

    Listener EngineImpl::GetDefaultListener() const
    {
        return Listener(_defaultListener);
    }

    Listener EngineImpl::AddListener(AmListenerID id) const
    {
        if (_state->listener_state_free_list.empty())
            return Listener(nullptr);

        if (const Listener item = GetListener(id); item.Valid())
            return item;

        ListenerInternalState* listener = _state->listener_state_free_list.back();
        listener->SetId(id);
        _state->listener_state_free_list.pop_back();
        _state->listener_list.push_back(*listener);

        return Listener(listener);
    }

    Listener EngineImpl::GetListener(AmListenerID id) const
    {
        const auto findIt = std::ranges::find_if(
            _state->listener_state_memory,
            [&id](const ListenerInternalState& state)
            {
                return state.GetId() == id;
            });

        return Listener(findIt != _state->listener_state_memory.end() ? &*findIt : nullptr);
    }

    void EngineImpl::RemoveListener(AmListenerID id) const
    {
        if (const auto findIt = std::ranges::find_if(
                _state->listener_state_memory,
                [&id](const ListenerInternalState& state)
                {
                    return state.GetId() == id;
                });
            findIt != _state->listener_state_memory.end())
        {
            findIt->SetId(kAmInvalidObjectId);
            findIt->node.remove();
            _state->listener_state_free_list.push_back(&*findIt);
        }
    }

    void EngineImpl::RemoveListener(const Listener* listener) const
    {
        if (!listener->Valid())
            return;

        listener->GetState()->SetId(kAmInvalidObjectId);
        listener->GetState()->node.remove();
        _state->listener_state_free_list.push_back(listener->GetState());
    }

    Entity EngineImpl::AddEntity(AmEntityID id) const
    {
        if (_state->entity_state_free_list.empty())
            return Entity(nullptr);

        if (const Entity item = GetEntity(id); item.Valid())
            return item;

        EntityInternalState* entity = _state->entity_state_free_list.back();
        entity->SetId(id);
        _state->entity_state_free_list.pop_back();
        _state->entity_list.push_back(*entity);

        return Entity(entity);
    }

    Entity EngineImpl::GetEntity(AmEntityID id) const
    {
        const auto findIt = std::ranges::find_if(
            _state->entity_state_memory,
            [&id](const EntityInternalState& state)
            {
                return state.GetId() == id;
            });

        return Entity(findIt != _state->entity_state_memory.end() ? &*findIt : nullptr);
    }

    void EngineImpl::RemoveEntity(const Entity* entity) const
    {
        if (!entity->Valid())
            return;

        entity->GetState()->SetId(kAmInvalidObjectId);
        entity->GetState()->node.remove();
        _state->entity_state_free_list.push_back(entity->GetState());
    }

    void EngineImpl::RemoveEntity(AmEntityID id) const
    {
        if (const auto findIt = std::ranges::find_if(
                _state->entity_state_memory,
                [&id](const EntityInternalState& state)
                {
                    return state.GetId() == id;
                });
            findIt != _state->entity_state_memory.end())
        {
            findIt->SetId(kAmInvalidObjectId);
            findIt->node.remove();
            _state->entity_state_free_list.push_back(&*findIt);
        }
    }

    Environment EngineImpl::AddEnvironment(AmEnvironmentID id) const
    {
        if (_state->environment_state_free_list.empty())
            return Environment(nullptr);

        if (const Environment item = GetEnvironment(id); item.Valid())
            return item;

        EnvironmentInternalState* environment = _state->environment_state_free_list.back();
        environment->SetId(id);
        _state->environment_state_free_list.pop_back();
        _state->environment_list.push_back(*environment);

        return Environment(environment);
    }

    Environment EngineImpl::GetEnvironment(AmEnvironmentID id) const
    {
        const auto findIt = std::ranges::find_if(
            _state->environment_state_memory,
            [&id](const EnvironmentInternalState& state)
            {
                return state.GetId() == id;
            });

        return Environment(findIt != _state->environment_state_memory.end() ? &*findIt : nullptr);
    }

    void EngineImpl::RemoveEnvironment(const Environment* environment) const
    {
        if (!environment->Valid())
            return;

        environment->GetState()->SetId(kAmInvalidObjectId);
        environment->GetState()->node.remove();
        _state->environment_state_free_list.push_back(environment->GetState());
    }

    void EngineImpl::RemoveEnvironment(AmEnvironmentID id) const
    {
        if (const auto findIt = std::ranges::find_if(
                _state->environment_state_memory,
                [&id](const EnvironmentInternalState& state)
                {
                    return state.GetId() == id;
                });
            findIt != _state->environment_state_memory.end())
        {
            findIt->SetId(kAmInvalidObjectId);
            findIt->node.remove();
            _state->environment_state_free_list.push_back(&*findIt);
        }
    }

    Room EngineImpl::AddRoom(AmRoomID id) const
    {
        if (_state->room_state_free_list.empty())
            return Room(nullptr);

        if (const Room item = GetRoom(id); item.Valid())
            return item;

        RoomInternalState* room = _state->room_state_free_list.back();
        room->SetId(id);
        _state->room_state_free_list.pop_back();
        _state->room_list.push_back(*room);

        return Room(room);
    }

    Room EngineImpl::GetRoom(AmRoomID id) const
    {
        const auto findIt = std::ranges::find_if(
            _state->room_state_memory,
            [&id](const RoomInternalState& state)
            {
                return state.GetId() == id;
            });

        return Room(findIt != _state->room_state_memory.end() ? &*findIt : nullptr);
    }

    void EngineImpl::RemoveRoom(const Room* room) const
    {
        if (!room->Valid())
            return;

        room->GetState()->SetId(kAmInvalidObjectId);
        room->GetState()->node.remove();
        _state->room_state_free_list.push_back(room->GetState());
    }

    void EngineImpl::RemoveRoom(AmRoomID id) const
    {
        if (const auto findIt = std::ranges::find_if(
                _state->room_state_memory,
                [&id](const RoomInternalState& state)
                {
                    return state.GetId() == id;
                });
            findIt != _state->room_state_memory.end())
        {
            findIt->SetId(kAmInvalidObjectId);
            findIt->node.remove();
            _state->room_state_free_list.push_back(&*findIt);
        }
    }

    Bus EngineImpl::FindBus(const AmString& name) const
    {
        return Bus(FindBusInternalState(_state, name));
    }

    Bus EngineImpl::FindBus(AmBusID id) const
    {
        return Bus(FindBusInternalState(_state, id));
    }

    void EngineImpl::Pause(bool pause) const
    {
        if (_state->paused == pause)
            return;

        _state->paused = pause;

        for (PriorityList& list = _state->playing_channel_list; auto&& state : list)
        {
            if (!state.Paused() && state.IsReal())
            {
                if (pause)
                {
                    // Pause the real channel underlying this virtual channel. This freezes
                    // playback of the channel without marking it as paused from the audio
                    // engine's point of view, so that we know to restart it when the audio
                    // engine is not paused.
                    state.GetRealChannel().Pause();
                }
                else
                {
                    // Resumed all channels that were not explicitly paused.
                    state.GetRealChannel().Resume();
                }
            }
        }
    }

    bool EngineImpl::IsPaused() const
    {
        return _state->paused;
    }

    void EraseFinishedSounds(EngineInternalState* state)
    {
        PriorityList& list = state->playing_channel_list;
        for (auto channelInternalState = list.begin(); channelInternalState != list.end();)
        {
            auto current = channelInternalState++;
            current->UpdateState();
            if (current->Stopped())
            {
                InsertIntoFreeList(state, &*current);
            }
        }
    }

    static void UpdateChannel(ChannelInternalState* channel, EngineInternalState* state)
    {
        if (channel->Stopped())
            return;

        AmReal32 gain;
        AmVec2 pan;
        AmReal32 pitch;

        // Find the best listener for this channel.
        ListenerInternalState* listener = FindBestListener(state->listener_list, channel->GetLocation(), state->listener_fetch_mode);

        if (const SwitchContainer* switchContainer = channel->GetSwitchContainer(); switchContainer != nullptr)
        {
            const SwitchContainerDefinition* definition = static_cast<const SwitchContainerImpl*>(switchContainer)->GetDefinition();

            CalculateGainPanPitch(
                &gain, &pan, &pitch, listener, nullptr, switchContainer->GetGain().GetValue(), switchContainer->GetPitch().GetValue(),
                switchContainer->GetBus().GetState(), definition->spatialization(), channel->GetUserGain());
        }
        else if (const Collection* collection = channel->GetCollection(); collection != nullptr)
        {
            const CollectionDefinition* definition = static_cast<const CollectionImpl*>(collection)->GetDefinition();

            CalculateGainPanPitch(
                &gain, &pan, &pitch, listener, nullptr, collection->GetGain().GetValue(), collection->GetPitch().GetValue(),
                collection->GetBus().GetState(), definition->spatialization(), channel->GetUserGain());
        }
        else if (const Sound* sound = channel->GetSound(); sound != nullptr)
        {
            const SoundDefinition* definition = static_cast<const SoundImpl*>(sound)->GetDefinition();

            CalculateGainPanPitch(
                &gain, &pan, &pitch, listener, nullptr, sound->GetGain().GetValue(), sound->GetPitch().GetValue(),
                sound->GetBus().GetState(), definition->spatialization(), channel->GetUserGain());
        }
        else
        {
            AMPLITUDE_ASSERT(false);
        }

        AssignBestRoom(channel, channel->GetLocation(), state);

        channel->SetGain(gain);
        channel->SetPan(pan);
        channel->SetPitch(pitch);
        channel->SetListener(Listener(listener));
    }

    // If there are any free real channels, assign those to virtual channels that
    // need them. If the priority list has gaps (i.e. if there are real channels
    // that are lower priority than virtual channels) then move the lower priority
    // real channels to the higher priority virtual channels.
    static void UpdateRealChannels(PriorityList* priorityList, FreeList* realFreeList, FreeList* virtualFreeList)
    {
        auto reverseIterator = priorityList->rbegin();
        for (auto state = priorityList->begin(); state != priorityList->end(); ++state)
        {
            if (!state->IsReal())
            {
                // First check if there are any free real channels.
                if (!realFreeList->empty())
                {
                    // We have a free real channel. Assign this channel id to the channel
                    // that is trying to resume, clear the free channel, and push it into
                    // the virtual free list.
                    ChannelInternalState* freeChannel = &realFreeList->front();
                    state->Devirtualize(freeChannel);

                    freeChannel->Remove();
                    virtualFreeList->push_front(*freeChannel);

                    state->Resume();
                }
                else if (&*reverseIterator != &*state)
                {
                    // If there aren't any free channels, then scan from the back of the
                    // list for low priority real channels.
                    reverseIterator = std::find_if(
                        reverseIterator, PriorityList::reverse_iterator(state),
                        [](const ChannelInternalState& channel)
                        {
                            return channel.GetRealChannel().Valid();
                        });
                    if (reverseIterator == priorityList->rend())
                    {
                        // There is no more swapping that can be done. Return.
                        return;
                    }
                    // Found a real channel that we can give to the higher priority
                    // channel.
                    state->Devirtualize(&*reverseIterator);
                }
            }
        }
    }

    void EngineImpl::AdvanceFrame(AmTime delta) const
    {
        if (_state == nullptr)
            return;

        if (_state->paused)
            return;

        if (!_state->stopping)
        {
            // Execute pending frame callbacks.
            Thread::LockMutex(_frameThreadMutex);
            {
                while (!_nextFrameCallbacks.empty())
                {
                    const auto& callback = _nextFrameCallbacks.front();
                    callback(delta);

                    _nextFrameCallbacks.pop();
                }
            }
            Thread::UnlockMutex(_frameThreadMutex);
        }

        EraseFinishedSounds(_state);

        _state->room_list.sort(
            [](const RoomInternalState& a, const RoomInternalState& b) -> bool
            {
                return a.GetVolume() > b.GetVolume();
            });

        for (const auto& rtpc : _state->rtpc_map | std::views::values)
            rtpc->Update(delta);

        for (const auto& effect : _state->effect_map | std::views::values)
            effect->Update();

        for (auto&& state : _state->listener_list)
            state.Update();

        for (auto&& state : _state->environment_list)
            state.Update();

        for (auto&& state : _state->room_list)
            state.Update();

        for (auto&& state : _state->entity_list)
        {
            state.Update();

            if (!_state->track_environments)
                for (auto&& env : _state->environment_list)
                    state.SetEnvironmentFactor(env.GetId(), env.GetFactor(Entity(&state)));
        }

        for (auto&& bus : _state->buses)
            bus.ResetDuckGain();

        for (auto&& bus : _state->buses)
            bus.UpdateDuckGain(delta);

        if (_state->master_bus)
        {
            const AmReal32 masterGain = _state->mute ? 0.0f : _state->master_gain;
            _state->master_bus->AdvanceFrame(delta, masterGain);
        }

        for (auto&& state : _state->playing_channel_list)
            UpdateChannel(&state, _state);

        _state->playing_channel_list.sort(
            [](const ChannelInternalState& a, const ChannelInternalState& b) -> bool
            {
                return a.Priority() < b.Priority();
            });

        UpdateRealChannels(&_state->playing_channel_list, &_state->real_channel_free_list, &_state->virtual_channel_free_list);

        for (AmSize i = 0; i < _state->running_events.size(); ++i)
        {
            EventInstance* event = &_state->running_events[i];

            if (!event->IsRunning())
            {
                _state->running_events.erase(_state->running_events.begin() + i);
                --i;
                continue;
            }

            event->AdvanceFrame(delta);
        }

        ++_state->current_frame;
        _state->total_time += delta;
    }

    void EngineImpl::OnNextFrame(std::function<void(AmTime delta)> callback) const
    {
        Thread::LockMutex(_frameThreadMutex);
        {
            _nextFrameCallbacks.push(std::move(callback));
        }
        Thread::UnlockMutex(_frameThreadMutex);
    }

    void EngineImpl::WaitUntilNextFrame() const
    {
        const AmUInt64 nextFrame = _state->current_frame + 1;
        while (_state->current_frame < nextFrame)
            Thread::Sleep(1);
    }

    void EngineImpl::WaitUntilFrames(AmUInt64 frameCount) const
    {
        const AmUInt64 nextFrame = _state->current_frame + frameCount;
        while (_state->current_frame < nextFrame)
            Thread::Sleep(1);
    }

    AmTime EngineImpl::GetTotalTime() const
    {
        return _state->total_time;
    }

    const struct AmVersion* EngineImpl::Version() const
    {
        return _state->version;
    }

    const EngineConfigDefinition* EngineImpl::GetEngineConfigDefinition() const
    {
        return Amplitude::GetEngineConfigDefinition(_configSrc.c_str());
    }

    Driver* EngineImpl::GetDriver() const
    {
        return _audioDriver;
    }

#pragma region Amplimix

    Amplimix* EngineImpl::GetMixer() const
    {
        return &_state->mixer;
    }

#pragma endregion

#pragma region Engine State

    EngineInternalState* EngineImpl::GetState() const
    {
        return _state;
    }

    AmReal32 EngineImpl::GetSoundSpeed() const
    {
        return _state->sound_speed;
    }

    AmReal32 EngineImpl::GetDopplerFactor() const
    {
        return _state->doppler_factor;
    }

    AmUInt32 EngineImpl::GetSamplesPerStream() const
    {
        return _state->samples_per_stream;
    }

    bool EngineImpl::IsGameTrackingEnvironmentAmounts() const
    {
        return _state->track_environments;
    }

    AmUInt32 EngineImpl::GetMaxListenersCount() const
    {
        return GetEngineConfigDefinition()->game()->listeners();
    }

    AmUInt32 EngineImpl::GetMaxEntitiesCount() const
    {
        return GetEngineConfigDefinition()->game()->entities();
    }

    const Curve& EngineImpl::GetOcclusionCoefficientCurve() const
    {
        return _state->occlusion_config.lpf;
    }

    const Curve& EngineImpl::GetOcclusionGainCurve() const
    {
        return _state->occlusion_config.gain;
    }

    const Curve& EngineImpl::GetObstructionCoefficientCurve() const
    {
        return _state->obstruction_config.lpf;
    }

    const Curve& EngineImpl::GetObstructionGainCurve() const
    {
        return _state->obstruction_config.gain;
    }

    ePanningMode EngineImpl::GetPanningMode() const
    {
        return _state->panning_mode;
    }

    eHRIRSphereSamplingMode EngineImpl::GetHRIRSphereSamplingMode() const
    {
        return _state->hrir_sampling_mode;
    }

    const HRIRSphere* EngineImpl::GetHRIRSphere() const
    {
        return _state->hrir_sphere;
    }

#pragma endregion

    Channel EngineImpl::PlayScopedSwitchContainer(
        SwitchContainerHandle handle, const Entity& entity, const AmVec3& location, const AmReal32 userGain) const
    {
        if (handle == nullptr)
        {
            amLogError("Cannot play switch container: Invalid switch container handle.");
            return Channel(nullptr);
        }

        const SwitchContainerDefinition* definition = static_cast<SwitchContainerImpl*>(handle)->GetDefinition();

        if (definition->scope() == Scope_Entity && !entity.Valid())
        {
            amLogError("Cannot play a switch container in Entity scope. No entity defined.");
            return Channel(nullptr);
        }

        if (entity.Valid())
        {
            // Process the first entity update
            entity.GetState()->Update();
        }

        // Find the best listener for this channel.
        ListenerInternalState* listener = FindBestListener(_state->listener_list, location, _state->listener_fetch_mode);

        // Find where it belongs in the list.
        AmReal32 gain;
        AmVec2 pan;
        AmReal32 pitch;
        CalculateGainPanPitch(
            &gain, &pan, &pitch, listener, nullptr, handle->GetGain().GetValue(), handle->GetPitch().GetValue(),
            handle->GetBus().GetState(), definition->spatialization(), userGain);
        const AmReal32 priority = gain * handle->GetPriority().GetValue();
        const auto insertionPoint = FindInsertionPoint(&_state->playing_channel_list, priority);

        // Decide which ChannelInternalState object to use.
        ChannelInternalState* newChannel = FindFreeChannelInternalState(
            insertionPoint, &_state->playing_channel_list, &_state->real_channel_free_list, &_state->virtual_channel_free_list,
            _state->paused);

        // The channel could not be added to the list; not high enough priority.
        if (newChannel == nullptr)
        {
            amLogDebug("Cannot play switch container: Not high enough priority.");
            return Channel(nullptr);
        }

        // Now that we have our new channel, set the data on it and update the next pointers.
        newChannel->SetEntity(entity);
        newChannel->SetSwitchContainer(static_cast<SwitchContainerImpl*>(handle));
        newChannel->SetUserGain(userGain);

        // Attempt to play the channel, if the engine is paused, the channel will be played later.
        OnNextFrame(
            [this, newChannel, handle](AmTime delta)
            {
                if (!newChannel->Play())
                {
                    amLogError("Failed to play switch container: %s.", handle->GetName().c_str());

                    // Error playing the sound, put it back in the free list.
                    InsertIntoFreeList(_state, newChannel);
                }
            });

        AssignBestRoom(newChannel, location, _state);

        newChannel->SetGain(gain);
        newChannel->SetPan(pan);
        newChannel->SetPitch(pitch);
        newChannel->SetLocation(location);
        newChannel->SetListener(Listener(listener));

        return Channel(newChannel);
    }

    Channel EngineImpl::PlayScopedCollection(
        CollectionHandle handle, const Entity& entity, const AmVec3& location, const AmReal32 userGain) const
    {
        if (handle == nullptr)
        {
            amLogError("Cannot play collection: Invalid collection handle.");
            return Channel(nullptr);
        }

        const CollectionDefinition* definition = static_cast<CollectionImpl*>(handle)->GetDefinition();

        if (definition->scope() == Scope_Entity && !entity.Valid())
        {
            amLogError("Cannot play a collection in Entity scope. No entity defined.");
            return Channel(nullptr);
        }

        if (entity.Valid())
        {
            // Process the first entity update
            entity.GetState()->Update();
        }

        // Find the best listener for this channel.
        ListenerInternalState* listener = FindBestListener(_state->listener_list, location, _state->listener_fetch_mode);

        // Find where it belongs in the list.
        AmReal32 gain;
        AmVec2 pan;
        AmReal32 pitch;
        CalculateGainPanPitch(
            &gain, &pan, &pitch, listener, nullptr, handle->GetGain().GetValue(), handle->GetPitch().GetValue(),
            handle->GetBus().GetState(), definition->spatialization(), userGain);
        const AmReal32 priority = gain * handle->GetPriority().GetValue();
        const auto insertionPoint = FindInsertionPoint(&_state->playing_channel_list, priority);

        // Decide which ChannelInternalState object to use.
        ChannelInternalState* newChannel = FindFreeChannelInternalState(
            insertionPoint, &_state->playing_channel_list, &_state->real_channel_free_list, &_state->virtual_channel_free_list,
            _state->paused);

        // The channel could not be added to the list; not high enough priority.
        if (newChannel == nullptr)
        {
            amLogDebug("Cannot play collection: Not high enough priority.");
            return Channel(nullptr);
        }

        // Now that we have our new channel, set the data on it and update the next pointers.
        newChannel->SetEntity(entity);
        newChannel->SetCollection(static_cast<CollectionImpl*>(handle));
        newChannel->SetUserGain(userGain);

        // Attempt to play the channel, if the engine is paused, the channel will be played later.
        OnNextFrame(
            [this, newChannel, handle](AmTime delta)
            {
                if (!newChannel->Play())
                {
                    amLogError("Failed to play collection: %s.", handle->GetName().c_str());

                    // Error playing the sound, put it back in the free list.
                    InsertIntoFreeList(_state, newChannel);
                }
            });

        AssignBestRoom(newChannel, location, _state);

        newChannel->SetGain(gain);
        newChannel->SetPan(pan);
        newChannel->SetPitch(pitch);
        newChannel->SetLocation(location);
        newChannel->SetListener(Listener(listener));

        return Channel(newChannel);
    }

    Channel EngineImpl::PlayScopedSound(SoundHandle handle, const Entity& entity, const AmVec3& location, AmReal32 userGain) const
    {
        if (handle == nullptr)
        {
            amLogError("Cannot play sound: Invalid sound handle.");
            return Channel(nullptr);
        }

        const SoundDefinition* definition = static_cast<SoundImpl*>(handle)->GetDefinition();

        if (definition->scope() == Scope_Entity && !entity.Valid())
        {
            amLogError("Cannot play a sound in Entity scope. No entity defined.");
            return Channel(nullptr);
        }

        if (entity.Valid())
        {
            // Process the first entity update
            entity.GetState()->Update();
        }

        // Find the best listener for this channel.
        ListenerInternalState* listener = FindBestListener(_state->listener_list, location, _state->listener_fetch_mode);

        // Find where it belongs in the list.
        AmReal32 gain;
        AmVec2 pan;
        AmReal32 pitch;
        CalculateGainPanPitch(
            &gain, &pan, &pitch, listener, nullptr, handle->GetGain().GetValue(), handle->GetPitch().GetValue(),
            handle->GetBus().GetState(), definition->spatialization(), userGain);
        const AmReal32 priority = gain * handle->GetPriority().GetValue();
        const auto insertionPoint = FindInsertionPoint(&_state->playing_channel_list, priority);

        // Decide which ChannelInternalState object to use.
        ChannelInternalState* newChannel = FindFreeChannelInternalState(
            insertionPoint, &_state->playing_channel_list, &_state->real_channel_free_list, &_state->virtual_channel_free_list,
            _state->paused);

        // The sound could not be added to the list; not high enough priority.
        if (newChannel == nullptr)
        {
            amLogDebug("Cannot play sound: Not high enough priority.");
            return Channel(nullptr);
        }

        // Now that we have our new channel, set the data on it and update the next pointers.
        newChannel->SetEntity(entity);
        newChannel->SetSound(static_cast<SoundImpl*>(handle));
        newChannel->SetUserGain(userGain);

        // Attempt to play the channel, if the engine is paused, the channel will be played later.
        OnNextFrame(
            [this, newChannel, handle](AmTime delta)
            {
                if (!newChannel->Play())
                {
                    amLogError("Failed to play sound: %s.", handle->GetName().c_str());

                    // Error playing the sound, put it back in the free list.
                    InsertIntoFreeList(_state, newChannel);
                }
            });

        AssignBestRoom(newChannel, location, _state);

        newChannel->SetGain(gain);
        newChannel->SetPan(pan);
        newChannel->SetPitch(pitch);
        newChannel->SetLocation(location);
        newChannel->SetListener(Listener(listener));

        return Channel(newChannel);
    }
} // namespace SparkyStudios::Audio::Amplitude
