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

#ifndef _AM_IMPLEMENTATION_CORE_ENGINE_H
#define _AM_IMPLEMENTATION_CORE_ENGINE_H

#include <SparkyStudios/Audio/Amplitude/Core/Engine.h>

#include <Core/EngineInternalState.h>

#include "engine_config_definition_generated.h"

#define amEngine static_cast<EngineImpl*>(SparkyStudios::Audio::Amplitude::Engine::GetInstance())

namespace SparkyStudios::Audio::Amplitude
{
    class EngineImpl final : public Engine
    {
        friend class Engine;

    public:
        /**
         * @brief Construct an uninitialized Engine.
         */
        EngineImpl();

        ~EngineImpl() override;

        /**
         * @brief Initializes the Amplitude engine.
         *
         * @param config A pointer to a loaded EngineConfigDefinition object.
         *
         * @return Whether the engine has been successfully initialized.
         */
        bool Initialize(const EngineConfigDefinition* config);

        [[nodiscard]] const EngineConfigDefinition* GetEngineConfigDefinition() const;

        /**
         * @brief Gets the current state of this Engine.
         *
         * @return The current state of this Engine.
         */
        [[nodiscard]] EngineInternalState* GetState() const;

        [[nodiscard]] const struct AmVersion* Version() const override;
        bool Initialize(const AmOsString& configFile) override;
        bool Deinitialize() override;
        [[nodiscard]] bool IsInitialized() const override;
        void SetFileSystem(FileSystem* fs) override;
        [[nodiscard]] const FileSystem* GetFileSystem() const override;
        void StartOpenFileSystem() override;
        bool TryFinalizeOpenFileSystem() override;
        void StartCloseFileSystem() override;
        bool TryFinalizeCloseFileSystem() override;
        void AdvanceFrame(AmTime delta) const override;
        void OnNextFrame(std::function<void(AmTime delta)> callback) const override;
        void WaitUntilNextFrame() const override;
        void WaitUntilFrames(AmUInt64 frameCount) const override;
        [[nodiscard]] AmTime GetTotalTime() const override;
        bool LoadSoundBank(const AmOsString& filename) override;
        bool LoadSoundBank(const AmOsString& filename, AmBankID& outID) override;
        bool LoadSoundBankFromMemory(const char* fileData) override;
        bool LoadSoundBankFromMemory(const char* fileData, AmBankID& outID) override;
        bool LoadSoundBankFromMemoryView(void* ptr, AmSize size) override;
        bool LoadSoundBankFromMemoryView(void* ptr, AmSize size, AmBankID& outID) override;
        void UnloadSoundBank(const AmOsString& filename) override;
        void UnloadSoundBank(AmBankID id) override;
        void UnloadSoundBanks() override;
        [[nodiscard]] bool HasLoadedSoundBank(const AmOsString& filename) const override;
        [[nodiscard]] bool HasLoadedSoundBank(AmBankID id) const override;
        [[nodiscard]] bool HasLoadedSoundBanks() const override;
        void StartLoadSoundFiles() override;
        bool TryFinalizeLoadSoundFiles() override;
        [[nodiscard]] SwitchContainerHandle GetSwitchContainerHandle(const AmString& name) const override;
        [[nodiscard]] SwitchContainerHandle GetSwitchContainerHandle(AmSwitchContainerID id) const override;
        [[nodiscard]] SwitchContainerHandle GetSwitchContainerHandleFromFile(const AmOsString& filename) const override;
        [[nodiscard]] CollectionHandle GetCollectionHandle(const AmString& name) const override;
        [[nodiscard]] CollectionHandle GetCollectionHandle(AmCollectionID id) const override;
        [[nodiscard]] CollectionHandle GetCollectionHandleFromFile(const AmOsString& filename) const override;
        [[nodiscard]] SoundHandle GetSoundHandle(const AmString& name) const override;
        [[nodiscard]] SoundHandle GetSoundHandle(AmSoundID id) const override;
        [[nodiscard]] SoundHandle GetSoundHandleFromFile(const AmOsString& filename) const override;
        [[nodiscard]] SoundObjectHandle GetSoundObjectHandle(const AmString& name) const override;
        [[nodiscard]] SoundObjectHandle GetSoundObjectHandle(AmSoundID id) const override;
        [[nodiscard]] SoundObjectHandle GetSoundObjectHandleFromFile(const AmOsString& filename) const override;
        [[nodiscard]] EventHandle GetEventHandle(const AmString& name) const override;
        [[nodiscard]] EventHandle GetEventHandle(AmEventID id) const override;
        [[nodiscard]] EventHandle GetEventHandleFromFile(const AmOsString& filename) const override;
        [[nodiscard]] AttenuationHandle GetAttenuationHandle(const AmString& name) const override;
        [[nodiscard]] AttenuationHandle GetAttenuationHandle(AmAttenuationID id) const override;
        [[nodiscard]] AttenuationHandle GetAttenuationHandleFromFile(const AmOsString& filename) const override;
        [[nodiscard]] SwitchHandle GetSwitchHandle(const AmString& name) const override;
        [[nodiscard]] SwitchHandle GetSwitchHandle(AmSwitchID id) const override;
        [[nodiscard]] SwitchHandle GetSwitchHandleFromFile(const AmOsString& filename) const override;
        [[nodiscard]] RtpcHandle GetRtpcHandle(const AmString& name) const override;
        [[nodiscard]] RtpcHandle GetRtpcHandle(AmRtpcID id) const override;
        [[nodiscard]] RtpcHandle GetRtpcHandleFromFile(const AmOsString& filename) const override;
        [[nodiscard]] EffectHandle GetEffectHandle(const AmString& name) const override;
        [[nodiscard]] EffectHandle GetEffectHandle(AmEffectID id) const override;
        [[nodiscard]] EffectHandle GetEffectHandleFromFile(const AmOsString& filename) const override;
        [[nodiscard]] PipelineHandle GetPipelineHandle() const override;
        void SetMasterGain(AmReal32 gain) const override;
        [[nodiscard]] AmReal32 GetMasterGain() const override;
        void SetMute(bool mute) const override;
        [[nodiscard]] bool IsMuted() const override;
        void Pause(bool pause) const override;
        [[nodiscard]] bool IsPaused() const override;
        void SetDefaultListener(const Listener* listener) override;
        void SetDefaultListener(AmListenerID id) override;
        [[nodiscard]] Listener GetDefaultListener() const override;
        [[nodiscard]] Listener AddListener(AmListenerID id) const override;
        [[nodiscard]] Listener GetListener(AmListenerID id) const override;
        void RemoveListener(AmListenerID id) const override;
        void RemoveListener(const Listener* listener) const override;
        [[nodiscard]] Entity AddEntity(AmEntityID id) const override;
        [[nodiscard]] Entity GetEntity(AmEntityID id) const override;
        void RemoveEntity(const Entity* entity) const override;
        void RemoveEntity(AmEntityID id) const override;
        [[nodiscard]] Environment AddEnvironment(AmEnvironmentID id) const override;
        [[nodiscard]] Environment GetEnvironment(AmEnvironmentID id) const override;
        void RemoveEnvironment(const Environment* Environment) const override;
        void RemoveEnvironment(AmEnvironmentID id) const override;
        [[nodiscard]] Room AddRoom(AmRoomID id) const override;
        [[nodiscard]] Room GetRoom(AmRoomID id) const override;
        void RemoveRoom(const Room* room) const override;
        void RemoveRoom(AmRoomID id) const override;
        [[nodiscard]] Bus FindBus(const AmString& name) const override;
        [[nodiscard]] Bus FindBus(AmBusID id) const override;
        [[nodiscard]] Channel Play(SwitchContainerHandle handle) const override;
        [[nodiscard]] Channel Play(SwitchContainerHandle handle, const AmVec3& location) const override;
        [[nodiscard]] Channel Play(SwitchContainerHandle handle, const AmVec3& location, AmReal32 userGain) const override;
        [[nodiscard]] Channel Play(SwitchContainerHandle handle, const Entity& entity) const override;
        [[nodiscard]] Channel Play(SwitchContainerHandle handle, const Entity& entity, AmReal32 userGain) const override;
        [[nodiscard]] Channel Play(CollectionHandle handle) const override;
        [[nodiscard]] Channel Play(CollectionHandle handle, const AmVec3& location) const override;
        [[nodiscard]] Channel Play(CollectionHandle handle, const AmVec3& location, AmReal32 userGain) const override;
        [[nodiscard]] Channel Play(CollectionHandle handle, const Entity& entity) const override;
        [[nodiscard]] Channel Play(CollectionHandle handle, const Entity& entity, AmReal32 userGain) const override;
        [[nodiscard]] Channel Play(SoundHandle handle) const override;
        [[nodiscard]] Channel Play(SoundHandle handle, const AmVec3& location) const override;
        [[nodiscard]] Channel Play(SoundHandle handle, const AmVec3& location, AmReal32 userGain) const override;
        [[nodiscard]] Channel Play(SoundHandle handle, const Entity& entity) const override;
        [[nodiscard]] Channel Play(SoundHandle handle, const Entity& entity, AmReal32 userGain) const override;
        [[nodiscard]] Channel Play(const AmString& name) const override;
        [[nodiscard]] Channel Play(const AmString& name, const AmVec3& location) const override;
        [[nodiscard]] Channel Play(const AmString& name, const AmVec3& location, AmReal32 userGain) const override;
        [[nodiscard]] Channel Play(const AmString& name, const Entity& entity) const override;
        [[nodiscard]] Channel Play(const AmString& name, const Entity& entity, AmReal32 userGain) const override;
        [[nodiscard]] Channel Play(AmObjectID id) const override;
        [[nodiscard]] Channel Play(AmObjectID id, const AmVec3& location) const override;
        [[nodiscard]] Channel Play(AmObjectID id, const AmVec3& location, AmReal32 userGain) const override;
        [[nodiscard]] Channel Play(AmObjectID id, const Entity& entity) const override;
        [[nodiscard]] Channel Play(AmObjectID id, const Entity& entity, AmReal32 userGain) const override;
        void StopAll() const override;
        [[nodiscard]] EventCanceler Trigger(EventHandle handle, const Entity& entity) const override;
        [[nodiscard]] EventCanceler Trigger(const AmString& name, const Entity& entity) const override;
        [[nodiscard]] EventCanceler Trigger(AmEventID id, const Entity& entity) const override;
        void SetSwitchState(SwitchHandle handle, AmObjectID stateId) const override;
        void SetSwitchState(SwitchHandle handle, const AmString& stateName) const override;
        void SetSwitchState(SwitchHandle handle, const SwitchState& state) const override;
        void SetSwitchState(AmSwitchID id, AmObjectID stateId) const override;
        void SetSwitchState(AmSwitchID id, const AmString& stateName) const override;
        void SetSwitchState(AmSwitchID id, const SwitchState& state) const override;
        void SetSwitchState(const AmString& name, AmObjectID stateId) const override;
        void SetSwitchState(const AmString& name, const AmString& stateName) const override;
        void SetSwitchState(const AmString& name, const SwitchState& state) const override;
        void SetRtpcValue(RtpcHandle handle, double value) const override;
        void SetRtpcValue(AmRtpcID id, double value) const override;
        void SetRtpcValue(const AmString& name, double value) const override;
        [[nodiscard]] Driver* GetDriver() const override;
        [[nodiscard]] Amplimix* GetMixer() const override;
        [[nodiscard]] AmReal32 GetSoundSpeed() const override;
        [[nodiscard]] AmReal32 GetDopplerFactor() const override;
        [[nodiscard]] AmUInt32 GetSamplesPerStream() const override;
        [[nodiscard]] bool IsGameTrackingEnvironmentAmounts() const override;
        [[nodiscard]] AmUInt32 GetMaxListenersCount() const override;
        [[nodiscard]] AmUInt32 GetMaxEntitiesCount() const override;
        [[nodiscard]] const Curve& GetOcclusionCoefficientCurve() const override;
        [[nodiscard]] const Curve& GetOcclusionGainCurve() const override;
        [[nodiscard]] const Curve& GetObstructionCoefficientCurve() const override;
        [[nodiscard]] const Curve& GetObstructionGainCurve() const override;
        [[nodiscard]] ePanningMode GetPanningMode() const override;
        [[nodiscard]] eHRIRSphereSamplingMode GetHRIRSphereSamplingMode() const override;
        [[nodiscard]] const HRIRSphere* GetHRIRSphere() const override;

    private:
        Channel PlayScopedSwitchContainer(
            SwitchContainerHandle handle, const Entity& entity, const AmVec3& location, AmReal32 userGain) const;
        Channel PlayScopedCollection(CollectionHandle handle, const Entity& entity, const AmVec3& location, AmReal32 userGain) const;
        Channel PlayScopedSound(SoundHandle handle, const Entity& entity, const AmVec3& location, AmReal32 userGain) const;

        // The lis of paths in which search for plugins.
        static std::set<AmOsString> _pluginSearchPaths;

        AmMutexHandle _frameThreadMutex;
        // The list of pending next frame callbacks.
        mutable std::queue<std::function<void(AmTime)>> _nextFrameCallbacks;

        // Hold the engine config file contents.
        AmString _configSrc;

        // The current state of the engine.
        EngineInternalState* _state;

        // The default audio listener.
        ListenerInternalState* _defaultListener;

        // The file loader implementation.
        FileSystem* _fs;

        // The audio driver used by the engine.
        Driver* _audioDriver;

        // The thread pool used to load audio files.
        AmUniquePtr<eMemoryPoolKind_Engine, Thread::Pool> _soundLoaderThreadPool;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_CORE_ENGINE_H
