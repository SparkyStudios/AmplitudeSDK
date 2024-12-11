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

#ifndef _AM_CORE_ENGINE_H
#define _AM_CORE_ENGINE_H

#include <queue>

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

#include <SparkyStudios/Audio/Amplitude/Core/Device.h>
#include <SparkyStudios/Audio/Amplitude/Core/Entity.h>
#include <SparkyStudios/Audio/Amplitude/Core/Environment.h>
#include <SparkyStudios/Audio/Amplitude/Core/Event.h>
#include <SparkyStudios/Audio/Amplitude/Core/Listener.h>
#include <SparkyStudios/Audio/Amplitude/Core/Playback/Bus.h>
#include <SparkyStudios/Audio/Amplitude/Core/Playback/Channel.h>
#include <SparkyStudios/Audio/Amplitude/Core/Room.h>
#include <SparkyStudios/Audio/Amplitude/Core/Version.h>

#include <SparkyStudios/Audio/Amplitude/HRTF/HRIRSphere.h>

#include <SparkyStudios/Audio/Amplitude/Mixer/Amplimix.h>
#include <SparkyStudios/Audio/Amplitude/Mixer/Pipeline.h>

#include <SparkyStudios/Audio/Amplitude/Sound/Attenuation.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Collection.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Effect.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Rtpc.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Sound.h>
#include <SparkyStudios/Audio/Amplitude/Sound/SoundObject.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Switch.h>
#include <SparkyStudios/Audio/Amplitude/Sound/SwitchContainer.h>

#if !defined(AM_BUILDSYSTEM_BUILDING_PLUGIN) && !defined(AM_BUILDSYSTEM_BUILDING_AMPLITUDE)

/**
 * @brief Macro to get the current Amplitude engine instance.
 *
 * @ingroup engine
 */
#define amEngine SparkyStudios::Audio::Amplitude::Engine::GetInstance()

#endif // AM_BUILDSYSTEM_BUILDING_PLUGIN

namespace SparkyStudios::Audio::Amplitude
{
    typedef SwitchContainer* SwitchContainerHandle;
    typedef Collection* CollectionHandle;
    typedef Sound* SoundHandle;
    typedef SoundObject* SoundObjectHandle;
    typedef Event* EventHandle;
    typedef Pipeline* PipelineHandle;
    typedef Attenuation* AttenuationHandle;
    typedef Switch* SwitchHandle;
    typedef Rtpc* RtpcHandle;
    typedef Effect* EffectHandle;

    /**
     * @brief The Amplitude Engine.
     *
     * This is the main class of the library that manages all the entities
     * and provides methods to create, destroy, and manipulate them. You can also
     * access to the internal state of the engine through the public API.
     *
     * The `Engine` is a singleton class, and you can access it using the `amEngine` macro. Before
     * using most of the methods of the engine, you need to [initialize the
     * engine](../../../integration/initializing-the-engine.md) first, for example:
     * ```cpp
     * amEngine->Initialize("config.amconfig");
     * //...
     * amEngine->Deinitialize();
     * ```
     *
     * @ingroup engine
     */
    class AM_API_PUBLIC Engine
    {
    public:
        virtual ~Engine() = default;

#pragma region Miscellaneous

        /**
         * @brief Gets the version structure.
         *
         * @return The version string structure
         */
        [[nodiscard]] virtual const AmVersion* Version() const = 0;

#pragma endregion

#pragma region Initialization

        /**
         * @brief Initializes the Amplitude engine.
         *
         * @param[in] configFile The path to the configuration file.
         *
         * @return `true` when the engine has been successfully initialized, `false` otherwise.
         */
        virtual bool Initialize(const AmOsString& configFile) = 0;

        /**
         * @brief Deinitializes the Amplitude engine.
         *
         * @return `true` when the engine has been successfully deinitialized, `false` otherwise.
         */
        virtual bool Deinitialize() = 0;

        /**
         * @brief Checks if the Amplitude engine has been successfully initialized.
         *
         * @return `true` if the engine has been successfully initialized, `false` otherwise.
         */
        [[nodiscard]] virtual bool IsInitialized() const = 0;

#pragma endregion

#pragma region FileSystem Management

        /**
         * @brief Sets a file system implementation to be used by the engine.
         *
         * @param[in] fs The file system implementation.
         */
        virtual void SetFileSystem(FileSystem* fs) = 0;

        /**
         * @brief Gets the file system implementation used by the engine.
         *
         * @return The current file system implementation used by the engine,
         * or `nullptr` if no file system has been set.
         */
        [[nodiscard]] virtual const FileSystem* GetFileSystem() const = 0;

        /**
         * @brief Opens the file system, usually in a separate thread.
         */
        virtual void StartOpenFileSystem() = 0;

        /**
         * @brief Checks if the file system has been fully loaded.
         *
         * This method is helpful when the file system implementation is loaded asynchronously. You
         * can use this method to wait until the file system is fully loaded before using it.
         *
         * @example
         * ```cpp
         * // Open the file system
         * amEngine->StartOpenFileSystem();
         * while (!amEngine->TryFinalizeOpenFileSystem()) {
         *     // Wait until the file system is fully loaded
         *     Thread::Sleep(100);
         * }
         * // Use the file system now
         * //...
         * ```
         *
         * @return `true` if the file system has been fully loaded, `false` otherwise.
         */
        virtual bool TryFinalizeOpenFileSystem() = 0;

        /**
         * @brief Closes the file system, usually in a separate thread.
         */
        virtual void StartCloseFileSystem() = 0;

        /**
         * @brief Checks if the file system has been fully closed.
         *
         * This method is helpful when the file system implementation is closed asynchronously. You
         * can use this method to wait until the file system is fully closed.
         *
         * @example
         * ```cpp
         * // Close the file system
         * amEngine->StartCloseFileSystem();
         * while (!amEngine->TryFinalizeCloseFileSystem()) {
         *     // Wait until the file system is fully closed
         *     Thread::Sleep(100);
         * }
         * // The file system is now closed
         * //...
         * ```
         *
         * @return `true` if the file system has been fully closed, `false` otherwise.
         */
        virtual bool TryFinalizeCloseFileSystem() = 0;

#pragma endregion

#pragma region Runtime Loop

        /**
         * @brief Updates the engine state for the given number of milliseconds.
         *
         * @param[in] delta The number of milliseconds since the last frame.
         */
        virtual void AdvanceFrame(AmTime delta) const = 0;

        /**
         * @brief Executes the given callback on the next frame.
         *
         * @note The given callback will be executed at the *beginning* of the next frame,
         * before doing the actual frame update.
         *
         * @param[in] callback The callback to be called when the next frame is ready.
         */
        virtual void OnNextFrame(std::function<void(AmTime delta)> callback) const = 0;

        /**
         * @brief Waits until the next frame is ready.
         *
         * This method blocks the current thread until the next frame is ready.
         */
        virtual void WaitUntilNextFrame() const = 0;

        /**
         * @brief Waits until the specified number of frames are ready.
         *
         * This method blocks the current thread until the specified number of frames are ready.
         *
         * @param[in] frameCount The number of frames to wait until.
         */
        virtual void WaitUntilFrames(AmUInt64 frameCount) const = 0;

        /**
         * @brief Gets the total elapsed time in milliseconds since the start of the engine.
         *
         * @return The total elapsed time in milliseconds since the start of the engine.
         */
        [[nodiscard]] virtual AmTime GetTotalTime() const = 0;

#pragma endregion

#pragma region Sound Bank Management

        /**
         * @brief Loads a sound bank from a binary asset file (`.ambank`).
         *
         * This method queues the sound files in that sound bank for loading. Call
         * @ref StartLoadSoundFiles `StartLoadSoundFiles()` to trigger the loading
         * of sound files on a separate thread.
         *
         * @param[in] filename The path to the sound bank asset file.
         *
         * @return `true` when the sound bank is successfully loaded, `false` otherwise.
         */
        virtual bool LoadSoundBank(const AmOsString& filename) = 0;

        /**
         * @brief Loads a sound bank from a binary asset file (`.ambank`).
         *
         * This method queues the sound files in that sound bank for loading. Call
         * @ref StartLoadSoundFiles `StartLoadSoundFiles()` to trigger the loading
         * of sound files on a separate thread.
         *
         * @param[in] filename The path to the sound bank asset file.
         * @param[out] outID The ID of the loaded sound bank.
         *
         * @return `true` when the sound bank is successfully loaded, `false` otherwise.
         */
        virtual bool LoadSoundBank(const AmOsString& filename, AmBankID& outID) = 0;

        /**
         * @brief Loads a sound bank from memory.
         *
         * This method queues the sound files in that sound bank for loading. Call
         * @ref StartLoadSoundFiles `StartLoadSoundFiles()` to trigger the loading
         * of sound files on a separate thread.
         *
         * @param[in] fileData The sound bank data to be loaded.
         *
         * @note The `fileData` pointer should be null terminated.
         *
         * @warning The `fileData` pointer should remain valid until the sound bank is unloaded.
         *
         * @return `true` when the sound bank is successfully loaded, `false` otherwise.
         */
        virtual bool LoadSoundBankFromMemory(const char* fileData) = 0;

        /**
         * @brief Loads a sound bank from memory.
         *
         * This method queues the sound files in that sound bank for loading. Call
         * @ref StartLoadSoundFiles `StartLoadSoundFiles()` to trigger the loading
         * of sound files on a separate thread.
         *
         * @param[in] fileData The sound bank data to be loaded.
         * @param[out] outID The ID of the loaded sound bank.
         *
         * @note The `fileData` pointer should be null terminated.
         *
         * @warning The `fileData` pointer should remain valid until the sound bank is unloaded.
         *
         * @return `true` when the sound bank is successfully loaded, `false` otherwise.
         */
        virtual bool LoadSoundBankFromMemory(const char* fileData, AmBankID& outID) = 0;

        /**
         * @brief Loads a sound bank from memory.
         *
         * This method queues the sound files in that sound bank for loading. Call
         * @ref StartLoadSoundFiles `StartLoadSoundFiles()` to trigger the loading
         * of sound files on a separate thread.
         *
         * @param[in] ptr The pointer to the sound bank data to be loaded.
         * @param[in] size The size of the memory to read.
         *
         * @note The `fileData` pointer should be null terminated.
         *
         * @warning The `fileData` pointer should remain valid until the sound bank is unloaded.
         *
         * @return `true` when the sound bank is successfully loaded, `false` otherwise.
         */
        virtual bool LoadSoundBankFromMemoryView(AmVoidPtr ptr, AmSize size) = 0;

        /**
         * @brief Loads a sound bank from memory.
         *
         * This method queues the sound files in that sound bank for loading. Call
         * @ref StartLoadSoundFiles `StartLoadSoundFiles()` to trigger the loading
         * of sound files on a separate thread.
         *
         * @param[in] ptr The pointer to the sound bank data to be loaded.
         * @param[in] size The size of the memory to read.
         * @param[out] outID The ID of the loaded sound bank.
         *
         * @note The `ptr` pointer should be null terminated.
         *
         * @warning The `ptr` pointer should remain valid until the sound bank is unloaded.
         *
         * @return `true` when the sound bank is successfully loaded, `false` otherwise.
         */
        virtual bool LoadSoundBankFromMemoryView(AmVoidPtr ptr, AmSize size, AmBankID& outID) = 0;

        /**
         * @brief Unloads a sound bank given its filename.
         *
         * @param[in] filename The file to unload.
         */
        virtual void UnloadSoundBank(const AmOsString& filename) = 0;

        /**
         * @brief Unloads a sound bank given its ID.
         *
         * @param[in] id The sound bank id to unload.
         */
        virtual void UnloadSoundBank(AmBankID id) = 0;

        /**
         * @brief Unloads all the loaded sound banks.
         */
        virtual void UnloadSoundBanks() = 0;

        /**
         * @brief Checks if a sound bank with the given filename has been loaded.
         *
         * @param[in] filename The file to check.
         *
         * @return `true` if the sound bank has been loaded, `false` otherwise.
         */
        [[nodiscard]] virtual bool HasLoadedSoundBank(const AmOsString& filename) const = 0;

        /**
         * @brief Checks if a sound bank with the given ID has been loaded.
         *
         * @param[in] id The sound bank id to check.
         *
         * @return `true` if the sound bank has been loaded, `false` otherwise.
         */
        [[nodiscard]] virtual bool HasLoadedSoundBank(AmBankID id) const = 0;

        /**
         * @brief Checks if any sound banks have been loaded.
         *
         * @return `true` if any sound banks have been loaded, `false` otherwise.
         */
        [[nodiscard]] virtual bool HasLoadedSoundBanks() const = 0;

        /**
         * @brief Starts the loading of sound files referenced in loaded sound banks.
         *
         * This process will run in another thread. You must call @ref TryFinalizeLoadSoundFiles `TryFinalizeLoadSoundFiles()` to
         * know when the loading has completed, and to automatically release used resources.
         */
        virtual void StartLoadSoundFiles() = 0;

        /**
         * @brief Checks if the loading of sound files has been completed, and releases used resources.
         *
         * @note This method should be called after calling @ref StartLoadSoundFiles `StartLoadSoundFiles()`.
         *
         * @example
         * ```cpp
         * // Start loading sound files
         * amEngine->StartLoadSoundFiles();
         * while (!amEngine->TryFinalizeLoadSoundFiles()) {
         *     // Wait for loading to complete
         *     Thread::Sleep(100);
         * }
         * // Sound files have been loaded, and used resources has been released
         * ```
         *
         * @return `true` when the sound files have been successfully loaded, `false` otherwise.
         */
        virtual bool TryFinalizeLoadSoundFiles() = 0;

#pragma endregion

#pragma region Handles

        /**
         * @brief Gets a `SwitchContainerHandle` given its name as defined in its asset file (`.amswitchcontainer`).
         *
         * @param[in] name The unique name as defined in the asset file.
         *
         * @return The `SwitchContainerHandle` for the given name, or an invalid handle if no switch container
         * with that name was found in any loaded sound bank.
         */
        [[nodiscard]] virtual SwitchContainerHandle GetSwitchContainerHandle(const AmString& name) const = 0;

        /**
         * @brief Gets a `SwitchContainerHandle` given its ID as defined in its asset file (`.amswitchcontainer`).
         *
         * @param[in] id The unique ID as defined in the asset file.
         *
         * @return The `SwitchContainerHandle` for the given ID, or an invalid handle if no switch container
         * with that ID was found in any loaded sound bank.
         */
        [[nodiscard]] virtual SwitchContainerHandle GetSwitchContainerHandle(AmSwitchContainerID id) const = 0;

        /**
         * @brief Gets a `SwitchContainerHandle` given its asset's filename.
         *
         * @param[in] filename The asset's filename.
         *
         * @note The asset's filename should be relative path from the `switch_containers` directory of your Amplitude
         * project, not an absolute path from the filesystem base path.
         *
         * @example
         * ```cpp
         * // Assuming the asset file is located in "switch_containers/footsteps.amswitchcontainer"
         * SwitchContainerHandle handle = amEngine->GetSwitchContainerHandleFromFile("footsteps.amswitchcontainer");
         * ```
         *
         * @return The `SwitchContainerHandle` for the given asset's filename, or an invalid handle if no switch container
         * with that filename was found in any loaded sound bank.
         */
        [[nodiscard]] virtual SwitchContainerHandle GetSwitchContainerHandleFromFile(const AmOsString& filename) const = 0;

        /**
         * @brief Gets a `CollectionHandle` given its name as defined in its asset file (`.amcollection`).
         *
         * @param[in] name The unique name as defined in the asset file.
         *
         * @return The `CollectionHandle` for the given name, or an invalid handle if no collection
         * with that name was found in any loaded sound bank.
         */
        [[nodiscard]] virtual CollectionHandle GetCollectionHandle(const AmString& name) const = 0;

        /**
         * @brief Gets a `CollectionHandle` given its ID as defined in its asset file (`.amcollection`).
         *
         * @param[in] id The unique ID as defined in the asset file.
         *
         * @return The `CollectionHandle` for the given ID, or an invalid handle if no collection
         * with that ID was found in any loaded sound bank.
         */
        [[nodiscard]] virtual CollectionHandle GetCollectionHandle(AmCollectionID id) const = 0;

        /**
         * @brief Gets a `CollectionHandle` given its asset's filename.
         *
         * @param[in] filename The asset's filename.
         *
         * @note The asset's filename should be relative path from the `collections` directory of your Amplitude
         * project, not an absolute path from the filesystem base path.
         *
         * @example
         * ```cpp
         * // Assuming the asset file is located in "collections/weapons/ak47_gunfires.amcollection"
         * CollectionHandle handle = amEngine->GetCollectionHandleFromFile("weapons/ak47_gunfires.amcollection");
         * ```
         *
         * @return The `CollectionHandle` for the given asset's filename, or an invalid handle if no collection
         * with that filename was found in any loaded sound bank.
         */
        [[nodiscard]] virtual CollectionHandle GetCollectionHandleFromFile(const AmOsString& filename) const = 0;

        /**
         * @brief Gets a `SoundHandle` given its name as defined in its asset file (`.amsound`).
         *
         * @param[in] name The unique name as defined in the asset file.
         *
         * @return The `SoundHandle` for the given name, or an invalid handle if no sound with that name
         * was found in any loaded sound bank.
         */
        [[nodiscard]] virtual SoundHandle GetSoundHandle(const AmString& name) const = 0;

        /**
         * @brief Gets a `SoundHandle` given its ID as defined in its asset file (`.amsound`).
         *
         * @param[in] id The unique ID as defined in the asset file.
         *
         * @return The `SoundHandle` for the given ID, or an invalid handle if no sound with that ID
         * was found in any loaded sound bank.
         */
        [[nodiscard]] virtual SoundHandle GetSoundHandle(AmSoundID id) const = 0;

        /**
         * @brief Gets a `SoundHandle` given its asset's filename.
         *
         * @param[in] filename The asset's filename.
         *
         * @note The asset's filename should be relative path from the `sounds` directory of your Amplitude
         * project, not an absolute path from the filesystem base path.
         *
         * @example
         * ```cpp
         * // Assuming the asset file is located in "sounds/env/forest/calm_lake_bg.amsound"
         * SoundHandle handle = amEngine->GetSoundHandleFromFile("env/forest/calm_lake_bg.amsound");
         * ```
         *
         * @return The `SoundHandle` for the given asset's filename, or an invalid handle if no sound
         * with that filename was found in any loaded sound bank.
         */
        [[nodiscard]] virtual SoundHandle GetSoundHandleFromFile(const AmOsString& filename) const = 0;

        /**
         * @brief Gets a `SoundObjectHandle` given its name as defined in its asset file.
         *
         * @param[in] name The unique name as defined in the asset file.
         *
         * @return The `SoundObjectHandle` for the given name, or an invalid handle if no sound object
         * with that name was found in any loaded sound bank.
         *
         * @note The return value can be a `SwitchContainerHandle`, a `CollectionHandle`, or a `SoundHandle`.
         */
        [[nodiscard]] virtual SoundObjectHandle GetSoundObjectHandle(const AmString& name) const = 0;

        /**
         * @brief Gets a `SoundObjectHandle` given its ID as defined in its asset file.
         *
         * @param[in] id The unique ID as defined in the asset file.
         *
         * @return The `SoundObjectHandle` for the given ID, or an invalid handle if no sound object
         * with that ID was found in any loaded sound bank.
         *
         * @note The return value can be a `SwitchContainerHandle`, a `CollectionHandle`, or a `SoundHandle`.
         */
        [[nodiscard]] virtual SoundObjectHandle GetSoundObjectHandle(AmSoundID id) const = 0;

        /**
         * @brief Gets a `SoundObjectHandle` given its asset's filename.
         *
         * @param[in] filename The asset's filename.
         *
         * @note The asset's filename should be relative path from the `sounds`, `collections`,
         * or `switch_containers` directories of your Amplitude project, not an absolute path
         * from the filesystem base path.
         *
         * @example
         * ```cpp
         * // Assuming the asset file is located in "sounds/env/forest/calm_lake_bg.amsound"
         * // Note that the return value in this case is indeed a `SoundHandle`
         * SoundObjectHandle handle = amEngine->GetSoundObjectHandleFromFile("env/forest/calm_lake_bg.amsound");
         * ```
         *
         * @return The `SoundObjectHandle` for the given asset's filename, or an invalid handle if no sound object
         * with that filename was found in any loaded sound bank.
         *
         * @note The return value can be a `SwitchContainerHandle`, a `CollectionHandle`, or a `SoundHandle`.
         */
        [[nodiscard]] virtual SoundObjectHandle GetSoundObjectHandleFromFile(const AmOsString& filename) const = 0;

        /**
         * @brief Gets an `EventHandle` given its name as defined in its asset file (`.amevent`).
         *
         * @param[in] name The unique name as defined in the asset file.
         *
         * @return The `EventHandle` for the given name, or an invalid handle if no event with that name
         * was found in any loaded sound bank.
         */
        [[nodiscard]] virtual EventHandle GetEventHandle(const AmString& name) const = 0;

        /**
         * @brief Gets an `EventHandle` given its ID as defined in its asset file (`.amevent`).
         *
         * @param[in] id The unique ID as defined in the asset file.
         *
         * @return The `EventHandle` for the given ID, or an invalid handle if no event with that ID
         * was found in any loaded sound bank.
         */
        [[nodiscard]] virtual EventHandle GetEventHandle(AmEventID id) const = 0;

        /**
         * @brief Gets an `EventHandle` given its asset's filename.
         *
         * @param[in] filename The asset's filename.
         *
         * @note The asset's filename should be relative path from the `events` directory of your Amplitude
         * project, not an absolute path from the filesystem base path.
         *
         * @example
         * ```cpp
         * // Assuming the asset file is located in "events/gameplay/start_menu.amevent"
         * EventHandle handle = amEngine->GetEventHandleFromFile("gameplay/start_menu.amevent");
         * ```
         *
         * @return The `EventHandle` for the given asset's filename, or an invalid handle if no event
         * with that filename was found in any loaded sound bank.
         */
        [[nodiscard]] virtual EventHandle GetEventHandleFromFile(const AmOsString& filename) const = 0;

        /**
         * @brief Gets an `AttenuationHandle` given its name as defined in its asset file (`.amattenuation`).
         *
         * @param[in] name The unique name as defined in the asset file.
         *
         * @return The `AttenuationHandle` for the given name, or an invalid handle if no attenuation with that name
         * was found in any loaded sound bank.
         */
        [[nodiscard]] virtual AttenuationHandle GetAttenuationHandle(const AmString& name) const = 0;

        /**
         * @brief Gets an `AttenuationHandle` given its ID as defined in its asset file (`.amattenuation`).
         *
         * @param[in] id The unique ID as defined in the asset file.
         *
         * @return The `AttenuationHandle` for the given ID, or an invalid handle if no attenuation with that ID
         * was found in any loaded sound bank.
         */
        [[nodiscard]] virtual AttenuationHandle GetAttenuationHandle(AmAttenuationID id) const = 0;

        /**
         * @brief Gets an `AttenuationHandle` given its asset's filename.
         *
         * @param[in] filename The asset's filename.
         *
         * @note The asset's filename should be relative path from the `attenuators` directory of your Amplitude
         * project, not an absolute path from the filesystem base path.
         *
         * @example
         * ```cpp
         * // Assuming the asset file is located in "attenuators/impact.amattenuation"
         * AttenuationHandle handle = amEngine->GetAttenuationHandleFromFile("impact.amattenuation");
         *```
         *
         * @return The `AttenuationHandle` for the given asset's filename, or an invalid handle if no attenuation
         * with that filename was found in any loaded sound bank.
         */
        [[nodiscard]] virtual AttenuationHandle GetAttenuationHandleFromFile(const AmOsString& filename) const = 0;

        /**
         * @brief Gets a `SwitchHandle` given its name as defined in its asset file (`.amswitch`).
         *
         * @param[in] name The unique name as defined in the asset file.
         *
         * @return The `SwitchHandle` for the given name, or an invalid handle if no switch with that name
         * was found in any loaded sound bank.
         */
        [[nodiscard]] virtual SwitchHandle GetSwitchHandle(const AmString& name) const = 0;

        /**
         * @brief Gets a `SwitchHandle` given its ID as defined in its asset file (`.amswitch`).
         *
         * @param[in] id The unique ID as defined in the asset file.
         *
         * @return The `SwitchHandle` for the given ID, or an invalid handle if no switch with that ID
         * was found in any loaded sound bank.
         */
        [[nodiscard]] virtual SwitchHandle GetSwitchHandle(AmSwitchID id) const = 0;

        /**
         * @brief Gets a `SwitchHandle` given its asset's filename.
         *
         * @param[in] filename The asset's filename.
         *
         * @note The asset's filename should be relative path from the `switches` directory of your Amplitude
         * project, not an absolute path from the filesystem base path.
         *
         * @example
         * ```cpp
         * // Assuming the asset file is located in "switches/env/surfaces.amswitch"
         * SwitchHandle handle = amEngine->GetSwitchHandleFromFile("env/surfaces.amswitch");
         * ```
         *
         * @return The `SwitchHandle` for the given asset's filename, or an invalid handle if no switch
         * with that filename was found in any loaded sound bank.
         */
        [[nodiscard]] virtual SwitchHandle GetSwitchHandleFromFile(const AmOsString& filename) const = 0;

        /**
         * @brief Gets a `RtpcHandle` given its name as defined in its asset file (`.amrtpc`).
         *
         * @param[in] name The unique name as defined in the asset file.
         *
         * @return The `RtpcHandle` for the given name, or an invalid handle if no RTPC with that name
         * was found in any loaded sound bank.
         */
        [[nodiscard]] virtual RtpcHandle GetRtpcHandle(const AmString& name) const = 0;

        /**
         * @brief Gets an `RtpcHandle` given its ID as defined in its asset file (`.amrtpc`).
         *
         * @param[in] id The unique ID as defined in the asset file.
         *
         * @return The `RtpcHandle` for the given ID, or an invalid handle if no RTPC with that ID
         * was found in any loaded sound bank.
         */
        [[nodiscard]] virtual RtpcHandle GetRtpcHandle(AmRtpcID id) const = 0;

        /**
         * @brief Gets an `RtpcHandle` given its asset's filename.
         *
         * @param[in] filename The asset's filename.
         *
         * @note The asset's filename should be relative path from the `rtpc` directory of your Amplitude
         * project, not an absolute path from the filesystem base path.
         *
         * @example
         * ```cpp
         * // Assuming the asset file is located in "rtpc/music_volume.amrtpc"
         * RtpcHandle handle = amEngine->GetRtpcHandleFromFile("music_volume.amrtpc");
         * ```
         *
         * @return The `RtpcHandle` for the given asset's filename, or an invalid handle if no RTPC
         * with that filename was found in any loaded sound bank.
         */
        [[nodiscard]] virtual RtpcHandle GetRtpcHandleFromFile(const AmOsString& filename) const = 0;

        /**
         * @brief Gets an `EffectHandle` given its name as defined in its asset file (`.amfx`).
         *
         * @param[in] name The unique name as defined in the asset file.
         *
         * @return The `EffectHandle` for the given name, or an invalid handle if no effect with that name
         * was found in any loaded sound bank.
         */
        [[nodiscard]] virtual EffectHandle GetEffectHandle(const AmString& name) const = 0;

        /**
         * @brief Gets an `EffectHandle` given its ID as defined in its asset file (`.amfx`).
         *
         * @param[in] id The unique ID as defined in the asset file.
         *
         * @return The `EffectHandle` for the given ID, or an invalid handle if no effect with that ID
         * was found in any loaded sound bank.
         */
        [[nodiscard]] virtual EffectHandle GetEffectHandle(AmEffectID id) const = 0;

        /**
         * @brief Gets an `EffectHandle` given its asset's filename.
         *
         * @param[in] filename The asset's filename.
         *
         * @note The asset's filename should be relative path from the `effects` directory of your Amplitude
         * project, not an absolute path from the filesystem base path.
         *
         * @example
         * ```cpp
         * // Assuming the asset file is located in "effects/echo.amfx"
         * EffectHandle handle = amEngine->GetEffectHandleFromFile("echo.amfx");
         * ```
         *
         * @return The `EffectHandle` for the given asset's filename, or an invalid handle if no effect
         * with that filename was found in any loaded sound bank.
         */
        [[nodiscard]] virtual EffectHandle GetEffectHandleFromFile(const AmOsString& filename) const = 0;

        /**
         * @brief Gets a `PipelineHandle` from the loaded pipeline asset file (`.ampipeline`).
         *
         * @note Only one pipeline can be loaded at a time. The loaded pipeline asset is defined in the
         * [engine configuration file](../../../project/engine-config.md#pipeline).
         *
         * @return The `PipelineHandle` for the loaded pipeline. This method should always return a valid handle.
         */
        [[nodiscard]] virtual PipelineHandle GetPipelineHandle() const = 0;

#pragma endregion

#pragma region Master Gain and Mute State

        /**
         * @brief Adjusts the master gain of the mixer.
         *
         * @param[in] gain The master gain.
         */
        virtual void SetMasterGain(AmReal32 gain) const = 0;

        /**
         * @brief Gets the mixer master gain.
         *
         * @return The mixer master gain.
         */
        [[nodiscard]] virtual AmReal32 GetMasterGain() const = 0;

        /**
         * @brief Mutes the engine, but keep processing audio.
         *
         * @param[in] mute Whether to mute or unmute the engine.
         */
        virtual void SetMute(bool mute) const = 0;

        /**
         * @brief Checks whether the engine is currently muted.
         *
         * @return `true` if the engine is muted, `false` otherwise.
         */
        [[nodiscard]] virtual bool IsMuted() const = 0;

        /**
         * @brief Pauses or resumes all playing sounds and streams.
         *
         * @param[in] pause Whether to pause or resume the engine.
         */
        virtual void Pause(bool pause) const = 0;

        /**
         * @brief Checks whether the engine is currently paused.
         *
         * @return `true` if the engine is currently paused, `false` otherwise.
         */
        [[nodiscard]] virtual bool IsPaused() const = 0;

#pragma endregion

#pragma region Listeners Management

        /**
         * @brief Sets the default sound listener.
         *
         * This method will set the default listener that will render every played sound sources.
         *
         * @note This method takes effect only if the
         * [`listener_fetch_mode`](../../../project/engine-config.md#listener_fetch_mode)
         * engine setting is set to `Default`.
         *
         * @param[in] listener A valid and initialized `Listener` instance.
         */
        virtual void SetDefaultListener(const Listener* listener) = 0;

        /**
         * @brief Sets the default sound listener.
         *
         * This method will set the default listener that will render every played sound sources.
         *
         * @note This method takes effect only if the
         * [`listener_fetch_mode`](../../../project/engine-config.md#listener_fetch_mode)
         * engine setting is set to `Default`.
         *
         * @param[in] id The ID of a valid and registered `Listener`.
         */
        virtual void SetDefaultListener(AmListenerID id) = 0;

        /**
         * @brief Gets the default audio `Listener`.
         *
         * @return An initialized `Listener` object if a default listener was set,
         * otherwise an uninitialized `Listener` object.
         */
        [[nodiscard]] virtual Listener GetDefaultListener() const = 0;

        /**
         * @brief Initializes and returns a new `Listener`.
         *
         * @param[in] id The new listener ID.
         *
         * @return An initialized `Listener`.
         */
        [[nodiscard]] virtual Listener AddListener(AmListenerID id) const = 0;

        /**
         * @brief Returns the `Listener` with the given ID.
         *
         * @param[in] id The listener ID.
         *
         * @return An initialized `Listener` if a one with the given ID has been registered before,
         * otherwise an uninitialized `Listener`.
         */
        [[nodiscard]] virtual Listener GetListener(AmListenerID id) const = 0;

        /**
         * @brief Removes a `Listener` given its ID.
         *
         * @param[in] id The ID of the `Listener` to be removed.
         */
        virtual void RemoveListener(AmListenerID id) const = 0;

        /**
         * @brief Removes a `Listener` given its handle.
         *
         * @param[in] listener The `Listener` to be removed.
         */
        virtual void RemoveListener(const Listener* listener) const = 0;

#pragma endregion

#pragma region Entities Management

        /**
         * @brief Initializes and returns a new `Entity`.
         *
         * @param[in] id The game entity ID.
         *
         * @return An initialized `Entity`.
         */
        [[nodiscard]] virtual Entity AddEntity(AmEntityID id) const = 0;

        /**
         * @brief Returns the `Entity` with the given ID.
         *
         * @param[in] id The game entity ID.
         *
         * @return An initialized `Entity` if that one has been registered before,
         * otherwise an uninitialized `Entity`.
         */
        [[nodiscard]] virtual Entity GetEntity(AmEntityID id) const = 0;

        /**
         * @brief Removes an `Entity`.
         *
         * @param[in] entity The game entity to be removed.
         */
        virtual void RemoveEntity(const Entity* entity) const = 0;

        /**
         * @brief Removes an `Entity` given its ID.
         *
         * @param[in] id The ID of the game entity to be removed.
         */
        virtual void RemoveEntity(AmEntityID id) const = 0;

#pragma endregion

#pragma region Environments Management

        /**
         * @brief Initializes and return a new `Environment`.
         *
         * @param[in] id The game environment ID.
         *
         * @return An initialized `Environment`.
         */
        [[nodiscard]] virtual Environment AddEnvironment(AmEnvironmentID id) const = 0;

        /**
         * @brief Returns the `Environment` with the given ID.
         *
         * @param[in] id The game environment ID.
         *
         * @return An initialized `Environment` if that one has been registered before,
         * otherwise an uninitialized `Environment`.
         */
        [[nodiscard]] virtual Environment GetEnvironment(AmEnvironmentID id) const = 0;

        /**
         * @brief Removes an `Environment`.
         *
         * @param[in] environment The game environment to be removed.
         */
        virtual void RemoveEnvironment(const Environment* environment) const = 0;

        /**
         * @brief Removes an `Environment` given its ID.
         *
         * @param[in] id The ID of the game environment to be removed.
         */
        virtual void RemoveEnvironment(AmEnvironmentID id) const = 0;

#pragma endregion

#pragma region Rooms Management

        /**
         * @brief Initializes and return a new `Room`.
         *
         * @param[in] id The room ID.
         *
         * @return An initialized `Room`.
         */
        [[nodiscard]] virtual Room AddRoom(AmRoomID id) const = 0;

        /**
         * @brief Returns the `Room` with the given ID.
         *
         * @param[in] id The room ID.
         *
         * @return An initialized `Room` if that one has been registered before,
         * otherwise an uninitialized `Room`.
         */
        [[nodiscard]] virtual Room GetRoom(AmRoomID id) const = 0;

        /**
         * @brief Removes a `Room`.
         *
         * @param[in] room The room to be removed.
         */
        virtual void RemoveRoom(const Room* room) const = 0;

        /**
         * @brief Removes a `Room` given its ID.
         *
         * @param[in] id The ID of the room to be removed.
         */
        virtual void RemoveRoom(AmRoomID id) const = 0;

#pragma endregion

#pragma region Buses Management

        /**
         * @brief Returns the `Bus` with the specified name.
         *
         * @param[in] name The name of the bus.
         *
         * @note The name should match one of the buses loaded from the asset file (`.ambus`).
         *
         * @return A valid `Bus` if found, otherwise an invalid `Bus`.
         */
        [[nodiscard]] virtual Bus FindBus(const AmString& name) const = 0;

        /**
         * @brief Returns the `Bus` with the given ID.
         *
         * @param[in] id The ID of the bus.
         *
         * @note The ID should match one of the buses loaded from the asset file (`.ambus`).
         *
         * @return A valid `Bus` if found, otherwise an invalid `Bus`.
         */
        [[nodiscard]] virtual Bus FindBus(AmBusID id) const = 0;

#pragma endregion

#pragma region Playback

        /**
         * @brief Plays a switch container associated with the given handle in the World scope.
         *
         * This method is recommended for switch containers with spatialization disabled, since
         * no positional information need to be provided.
         *
         * @param[in] handle A handle to the switch container to play.
         *
         * @return The channel the switch container is being played on. If the switch container could not be
         * played, or the given handle is invalid, an invalid `Channel` is returned.
         */
        [[nodiscard]] virtual Channel Play(SwitchContainerHandle handle) const = 0;

        /**
         * @brief Plays a switch container associated with the given handle in the World scope.
         *
         * @param[in] handle A handle to the switch container to play.
         * @param[in] location The location at which switch container should be played.
         *
         * @return The channel the switch container is being played on. If the switch container could not be
         * played, or the given handle is invalid, an invalid `Channel` is returned.
         */
        [[nodiscard]] virtual Channel Play(SwitchContainerHandle handle, const AmVec3& location) const = 0;

        /**
         * @brief Plays a switch container associated with the given handle in the World scope.
         *
         * @param[in] handle A handle to the switch container to play.
         * @param[in] location The location at which the switch container should be played.
         * @param[in] userGain The gain of the sound. Must be in the range [0, 1].
         *
         * @note The `userGain` parameter will not be used directly, but instead, it will be used in the final
         * gain computation, which may include other factors like the attenuation and the master gain.
         *
         * @return The channel the switch container is being played on. If the switch container could not be
         * played, or the given handle is invalid, an invalid `Channel` is returned.
         */
        [[nodiscard]] virtual Channel Play(SwitchContainerHandle handle, const AmVec3& location, AmReal32 userGain) const = 0;

        /**
         * @brief Plays a switch container associated with the given handle in an Entity scope.
         *
         * @note Switch containers played using this method should have been set in the `Entity` scope
         * from their asset file. See more [here](../../../project/sound-object.md#scope).
         *
         * @param[in] handle A handle to the switch container to play.
         * @param[in] entity The entity on which the switch container should be played.
         *
         * @return The channel the switch container is being played on. If the switch container could not be
         * played, the given handle is invalid, or the given entity is invalid, an invalid `Channel` is returned.
         */
        [[nodiscard]] virtual Channel Play(SwitchContainerHandle handle, const Entity& entity) const = 0;

        /**
         * @brief Plays a switch container associated with the given handle in an Entity scope.
         *
         * @note Switch containers played using this method should have been set in the `Entity` scope
         * from their asset file. See more [here](../../../project/sound-object.md#scope).
         *
         * @param[in] handle A handle to the switch container to play.
         * @param[in] entity The entity on which the switch container should be played.
         * @param[in] userGain The gain of the sound. Must be in the range [0, 1].
         *
         * @note The `userGain` parameter will not be used directly, but instead, it will be used in the final
         * gain computation, which may include other factors like the attenuation and the master gain.
         *
         * @return The channel the switch container is being played on. If the switch container could not be
         * played, the given handle is invalid, or the given entity is invalid, an invalid `Channel` is returned.
         */
        [[nodiscard]] virtual Channel Play(SwitchContainerHandle handle, const Entity& entity, AmReal32 userGain) const = 0;

        /**
         * @brief Plays a collection associated with the given handle in the World scope.
         *
         * @param[in] handle A handle to the collection to play.
         *
         * @return The channel the collection is being played on. If the collection could not be
         * played, or the handle is invalid, an invalid `Channel` is returned.
         */
        [[nodiscard]] virtual Channel Play(CollectionHandle handle) const = 0;

        /**
         * @brief Plays a collection associated with the given handle in the World scope.
         *
         * @param[in] handle A handle to the collection to play.
         * @param[in] location The location at which the collection should be played.
         *
         * @return The channel the collection is being played on. If the collection could not be
         * played, or the handle is invalid, an invalid `Channel` is returned.
         */
        [[nodiscard]] virtual Channel Play(CollectionHandle handle, const AmVec3& location) const = 0;

        /**
         * @brief Plays a collection associated with the given handle in the World scope.
         *
         * @param[in] handle A handle to the collection to play.
         * @param[in] location The location at which the collection should be played.
         * @param[in] userGain The gain of the sound. Must be in the range [0, 1].
         *
         * @note The `userGain` parameter will not be used directly, but instead, it will be used in the final
         * gain computation, which may include other factors like the attenuation and the master gain.
         *
         * @return The channel the collection is being played on. If the collection could not be
         * played, or the handle is invalid, an invalid `Channel` is returned.
         */
        [[nodiscard]] virtual Channel Play(CollectionHandle handle, const AmVec3& location, AmReal32 userGain) const = 0;

        /**
         * @brief Plays a collection associated with the given handle in the Entity scope.
         *
         * @note Collections played using this method should have been set in the `Entity` scope
         * from their asset file. See more [here](../../../project/sound-object.md#scope).
         *
         * @param[in] handle A handle to the collection to play.
         * @param[in] entity The entity on which the collection should be played.
         *
         * @return The channel the collection is being played on. If the collection could not be
         * played, the given handle is invalid, or the given entity is invalid, an invalid `Channel` is returned.
         */
        [[nodiscard]] virtual Channel Play(CollectionHandle handle, const Entity& entity) const = 0;

        /**
         * @brief Plays a collection associated with the given handle in an Entity scope.
         *
         * @note Collections played using this method should have been set in the `Entity` scope
         * from their asset file. See more [here](../../../project/sound-object.md#scope).
         *
         * @param[in] handle A handle to the collection to play.
         * @param[in] entity The entity on which the collection should be played.
         * @param[in] userGain The gain of the sound. Must be in the range [0, 1].
         *
         * @note The `userGain` parameter will not be used directly, but instead, it will be used in the final
         * gain computation, which may include other factors like the attenuation and the master gain.
         *
         * @return The channel the collection is being played on. If the collection could not be
         * played, the given handle is invalid, or the given entity is invalid, an invalid `Channel` is returned.
         */
        [[nodiscard]] virtual Channel Play(CollectionHandle handle, const Entity& entity, AmReal32 userGain) const = 0;

        /**
         * @brief Plays a sound associated with the given handle in the World scope.
         *
         * @param[in] handle A handle to the sound to play.
         *
         * @return The channel the sound is being played on. If the sound could not be
         * played, the given handle is invalid, an invalid `Channel` is returned.
         */
        [[nodiscard]] virtual Channel Play(SoundHandle handle) const = 0;

        /**
         * @brief Plays a sound associated with the given handle in the World scope.
         *
         * @param[in] handle A handle to the sound to play.
         * @param[in] location The location at which the sound should be played.
         *
         * @return The channel the sound is being played on. If the sound could not be
         * played, the given handle is invalid, an invalid `Channel` is returned.
         */
        [[nodiscard]] virtual Channel Play(SoundHandle handle, const AmVec3& location) const = 0;

        /**
         * @brief Plays a sound associated with the given handle in the World scope.
         *
         * @param[in] handle A handle to the sound to play.
         * @param[in] location The location at which the sound should be played.
         * @param[in] userGain The gain of the sound. Must be in the range [0, 1].
         *
         * @note The `userGain` parameter will not be used directly, but instead, it will be used in the final
         * gain computation, which may include other factors like the attenuation and the master gain.
         *
         * @return The channel the sound is being played on. If the sound could not be
         * played, the given handle is invalid, an invalid `Channel` is returned.
         */
        [[nodiscard]] virtual Channel Play(SoundHandle handle, const AmVec3& location, AmReal32 userGain) const = 0;

        /**
         * @brief Plays a sound associated with the given sound handle in an Entity scope.
         *
         * @note Sounds played using this method should have been set in the `Entity` scope
         * from their asset file. See more [here](../../../project/sound-object.md#scope).
         *
         * @param[in] handle A handle to the sound to play.
         * @param[in] entity The entity on which the sound should be played.
         *
         * @return The channel the sound is being played on. If the sound could not be
         * played, the given handle is invalid, or the given entity is invalid, an invalid `Channel` is returned.
         */
        [[nodiscard]] virtual Channel Play(SoundHandle handle, const Entity& entity) const = 0;

        /**
         * @brief Plays a sound associated with the given sound handle in an Entity.
         *
         * @note Sounds played using this method should have been set in the `Entity` scope
         * from their asset file. See more [here](../../../project/sound-object.md#scope).
         *
         * @param[in] handle A handle to the sound to play.
         * @param[in] entity The entity on which the sound should be played.
         * @param[in] userGain The gain of the sound. Must be in the range [0, 1].
         *
         * @note The `userGain` parameter will not be used directly, but instead, it will be used in the final
         * gain computation, which may include other factors like the attenuation and the master gain.
         *
         * @return The channel the sound is being played on. If the sound could not be
         * played, the given handle is invalid, or the given entity is invalid, an invalid `Channel` is returned.
         */
        [[nodiscard]] virtual Channel Play(SoundHandle handle, const Entity& entity, AmReal32 userGain) const = 0;

        /**
         * @brief Plays a sound object associated with the given name in the World scope.
         *
         * @tip Playing a sound object with its handle is faster than using the
         * name as using the name requires an internal lookup.
         *
         * @param[in] name The name of the sound object to play.
         *
         * @return The channel the sound object is being played on. If the object could not be
         * played, or an object with the given name was not found, an invalid `Channel` is returned.
         */
        [[nodiscard]] virtual Channel Play(const AmString& name) const = 0;

        /**
         * @brief Plays a sound object associated with the given name in the World scope.
         *
         * @tip Playing a sound object with its handle is faster than using the
         * name as using the name requires an internal lookup.
         *
         * @param[in] name The name of the sound object to play.
         * @param[in] location The location at which the sound should be played.
         *
         * @return The channel the sound object is being played on. If the object could not be
         * played, or an object with the given name was not found, an invalid `Channel` is returned.
         */
        [[nodiscard]] virtual Channel Play(const AmString& name, const AmVec3& location) const = 0;

        /**
         * @brief Plays a sound object associated with the given name in the World scope.
         *
         * @tip Playing a sound object with its handle is faster than using the
         * name as using the name requires an internal lookup.
         *
         * @param[in] name The name of the sound object to play.
         * @param[in] location The location at which the sound should be played.
         * @param[in] userGain The gain of the sound. Must be in the range [0, 1].
         *
         * @note The `userGain` parameter will not be used directly, but instead, it will be used in the final
         * gain computation, which may include other factors like the attenuation and the master gain.
         *
         * @return The channel the sound object is being played on. If the object could not be
         * played, or an object with the given name was not found, an invalid `Channel` is returned.
         */
        [[nodiscard]] virtual Channel Play(const AmString& name, const AmVec3& location, AmReal32 userGain) const = 0;

        /**
         * @brief Plays a sound object associated with the given name in an Entity scope.
         *
         * @tip Playing a sound object with its handle is faster than using the
         * name as using the name requires an internal lookup.
         *
         * @note Sound objects played using this method should have been set in the `Entity` scope
         * from their asset file. See more [here](../../../project/sound-object.md#scope).
         *
         * @param[in] name The name of the sound object to play.
         * @param[in] entity The entity on which the sound object should be played.
         *
         * @return The channel the sound object is being played on. If the object could not be
         * played, an object with the given name was not found, or the entity is invalid,
         * an invalid `Channel` is returned.
         */
        [[nodiscard]] virtual Channel Play(const AmString& name, const Entity& entity) const = 0;

        /**
         * @brief Plays a sound object associated with the given name in an Entity scope.
         *
         * @tip Playing a sound object with its handle is faster than using the
         * name as using the name requires an internal lookup.
         *
         * @note Sound objects played using this method should have been set in the `Entity` scope
         * from their asset file. See more [here](../../../project/sound-object.md#scope).
         *
         * @param[in] name The name of the sound object to play.
         * @param[in] entity The entity on which the sound object should be played.
         * @param[in] userGain The gain of the sound. Must be in the range [0, 1].
         *
         * @note The `userGain` parameter will not be used directly, but instead, it will be used in the final
         * gain computation, which may include other factors like the attenuation and the master gain.
         *
         * @return The channel the sound object is being played on. If the object could not be
         * played, an object with the given name was not found, or the entity is invalid,
         * an invalid `Channel` is returned.
         */
        [[nodiscard]] virtual Channel Play(const AmString& name, const Entity& entity, AmReal32 userGain) const = 0;

        /**
         * @brief Plays a sound object associated with the given ID in the  World scope.
         *
         * @tip Playing a sound object with its handle is faster than using the
         * ID as using the ID requires an internal lookup.
         *
         * @param[in] id The ID of the sound object to play.
         *
         * @return The channel the sound object is being played on. If the object could not be
         * played, or an object with the given ID was not found, an invalid `Channel` is returned.
         */
        [[nodiscard]] virtual Channel Play(AmObjectID id) const = 0;

        /**
         * @brief Plays a sound object associated with the given ID in the World scope.
         *
         * @tip Playing a sound object with its handle is faster than using the
         * ID as using the ID requires an internal lookup.
         *
         * @param[in] id The ID of the sound object to play.
         * @param[in] location The location at which the sound object should be played.
         *
         * @return The channel the sound object is being played on. If the object could not be
         * played, or an object with the given ID was not found, an invalid `Channel` is returned.
         */
        [[nodiscard]] virtual Channel Play(AmObjectID id, const AmVec3& location) const = 0;

        /**
         * @brief Plays a sound object associated with the given ID in the World scope.
         *
         * @tip Playing a sound object with its handle is faster than using the
         * ID as using the ID requires an internal lookup.
         *
         * @param[in] id The ID of the sound object to play.
         * @param[in] location The location at which the sound object should be played.
         * @param[in] userGain The gain of the sound. Must be in the range [0, 1].
         *
         * @note The `userGain` parameter will not be used directly, but instead, it will be used in the final
         * gain computation, which may include other factors like the attenuation and the master gain.
         *
         * @return The channel the sound object is being played on. If the object could not be
         * played, or an object with the given ID was not found, an invalid `Channel` is returned.
         */
        [[nodiscard]] virtual Channel Play(AmObjectID id, const AmVec3& location, AmReal32 userGain) const = 0;

        /**
         * @brief Plays a sound object associated with the given ID in an Entity scope.
         *
         * @tip Playing a sound object with its handle is faster than using the
         * ID as using the ID requires an internal lookup.
         *
         * @note Sound objects played using this method should have been set in the `Entity` scope
         * from their asset file. See more [here](../../../project/sound-object.md#scope).
         *
         * @param[in] id The ID of the sound object to play.
         * @param[in] entity The entity on which the sound object should be played.
         *
         * @return The channel the sound object is being played on. If the object could not be
         * played, an object with the given ID was not found, or the entity is invalid,
         * an invalid `Channel` is returned.
         */
        [[nodiscard]] virtual Channel Play(AmObjectID id, const Entity& entity) const = 0;

        /**
         * @brief Plays a sound object associated with the given ID in an Entity scope.
         *
         * @tip Playing a sound object with its handle is faster than using the
         * ID as using the ID requires an internal lookup.
         *
         * @note Sound objects played using this method should have been set in the `Entity` scope
         * from their asset file. See more [here](../../../project/sound-object.md#scope).
         *
         * @param[in] id The ID of the sound object to play.
         * @param[in] entity The entity on which the sound object should be played.
         * @param[in] userGain The gain of the sound. Must be in the range [0, 1].
         *
         * @note The `userGain` parameter will not be used directly, but instead, it will be used in the final
         * gain computation, which may include other factors like the attenuation and the master gain.
         *
         * @return The channel the sound object is being played on. If the object could not be
         * played, an object with the given ID was not found, or the entity is invalid,
         * an invalid `Channel` is returned.
         */
        [[nodiscard]] virtual Channel Play(AmObjectID id, const Entity& entity, AmReal32 userGain) const = 0;

        /**
         * @brief Stops all playing sound objects.
         *
         * This is the equivalent of calling `Stop()` on all generated channels.
         */
        virtual void StopAll() const = 0;

#pragma endregion

#pragma region Events

        /**
         * @brief Triggers the event associated to the given handle.
         *
         * @param[in] handle The handle of the event to trigger.
         * @param[in] entity The entity on which trigger the event.
         *
         * @return An `EventCanceler` object which may be used to cancel the execution of the event.
         */
        [[nodiscard]] virtual EventCanceler Trigger(EventHandle handle, const Entity& entity) const = 0;

        /**
         * @brief Triggers the event associated to the given name.
         *
         * @tip Triggering an event with its `EventHandle` is faster than using the
         * event name as using the name requires an internal lookup.
         *
         * @param[in] name The name of event to trigger.
         * @param[in] entity The entity on which trigger the event.
         *
         * @return An `EventCanceler` object which may be used to cancel the execution of the event.
         */
        [[nodiscard]] virtual EventCanceler Trigger(const AmString& name, const Entity& entity) const = 0;

        /**
         * @brief Triggers the event associated to the given ID.
         *
         * @tip Triggering an event with its `EventHandle` is faster than using the
         * event ID as using the ID requires an internal lookup.
         *
         * @param[in] id The ID of event to trigger.
         * @param[in] entity The entity on which trigger the event.
         *
         * @return An `EventCanceler` object which may be used to cancel the execution of the event.
         */
        [[nodiscard]] virtual EventCanceler Trigger(AmEventID id, const Entity& entity) const = 0;

#pragma endregion

#pragma region Switches

        /**
         * @brief Sets the active state of the defined `Switch`.
         *
         * @param[in] handle The handle of the `Switch`.
         * @param[in] stateId The ID of the active state to set.
         */
        virtual void SetSwitchState(SwitchHandle handle, AmObjectID stateId) const = 0;

        /**
         * @brief Sets the active state of the defined `Switch`.
         *
         * @param[in] handle The handle of the `Switch`.
         * @param[in] stateName The name of the active state to set.
         */
        virtual void SetSwitchState(SwitchHandle handle, const AmString& stateName) const = 0;

        /**
         * @brief Sets the active state of the defined `Switch`.
         *
         * @param[in] handle The handle of the `Switch`.
         * @param[in] state The active state to set.
         */
        virtual void SetSwitchState(SwitchHandle handle, const SwitchState& state) const = 0;

        /**
         * @brief Sets the active state of the defined `Switch`.
         *
         * @param[in] id The ID of the `Switch` to update.
         * @param[in] stateId The ID of the active state to set.
         */
        virtual void SetSwitchState(AmSwitchID id, AmObjectID stateId) const = 0;

        /**
         * @brief Sets the active state of the defined `Switch`.
         *
         * @param[in] id The ID of the `Switch` to update.
         * @param[in] stateName The name of the active state to set.
         */
        virtual void SetSwitchState(AmSwitchID id, const AmString& stateName) const = 0;

        /**
         * @brief Sets the active state of the defined `Switch`.
         *
         * @param[in] id The ID of the `Switch` to update.
         * @param[in] state The active state to set.
         */
        virtual void SetSwitchState(AmSwitchID id, const SwitchState& state) const = 0;

        /**
         * @brief Sets the active state of the defined `Switch`.
         *
         * @param[in] name The name of the `Switch` to update.
         * @param[in] stateId The ID of the active state to set.
         */
        virtual void SetSwitchState(const AmString& name, AmObjectID stateId) const = 0;

        /**
         * @brief Sets the active state of the defined `Switch`.
         *
         * @param[in] name The name of the `Switch` to update.
         * @param[in] stateName The name of the active state to set.
         */
        virtual void SetSwitchState(const AmString& name, const AmString& stateName) const = 0;

        /**
         * @brief Sets the active state of the defined `Switch`.
         *
         * @param[in] name The name of the `Switch` to update.
         * @param[in] state The active state to set.
         */
        virtual void SetSwitchState(const AmString& name, const SwitchState& state) const = 0;

#pragma endregion

#pragma region RTPC

        /**
         * @brief Sets the value of a `RTPC`.
         *
         * @param[in] handle The handle of the `RTPC` to update.
         * @param[in] value The value to set to the `RTPC`.
         */
        virtual void SetRtpcValue(RtpcHandle handle, double value) const = 0;

        /**
         * @brief Sets the value of a `RTPC`.
         *
         * @param[in] id The ID of the `RTPC` to update.
         * @param[in] value The value to set to the `RTPC`.
         */
        virtual void SetRtpcValue(AmRtpcID id, double value) const = 0;

        /**
         * @brief Sets the value of a `RTPC`.
         *
         * @param[in] name THe name of the `RTPC` to update.
         * @param[in] value The value to set to the `RTPC`.
         */
        virtual void SetRtpcValue(const AmString& name, double value) const = 0;

#pragma endregion

#pragma region Driver

        /**
         * @brief Gets the audio driver used by the Engine.
         *
         * @return The audio driver.
         */
        [[nodiscard]] virtual Driver* GetDriver() const = 0;

#pragma endregion

#pragma region Amplimix

        /**
         * @brief Gets the mixer instance.
         *
         * @return The `Amplimix` mixer instance.
         */
        [[nodiscard]] virtual Amplimix* GetMixer() const = 0;

#pragma endregion

#pragma region Engine State and Configuration

        /**
         * @brief Gets the speed of sound, as set in the loaded engine configuration file.
         *
         * @return The speed of sound.
         */
        [[nodiscard]] virtual AmReal32 GetSoundSpeed() const = 0;

        /**
         * @brief Get the Doppler factor, as set in the loaded engine configuration file.
         *
         * @return The Doppler factor.
         */
        [[nodiscard]] virtual AmReal32 GetDopplerFactor() const = 0;

        /**
         * @brief Get the number of samples to process in one stream, as set in the loaded engine configuration file.
         *
         * @return The number of samples per stream.
         */
        [[nodiscard]] virtual AmUInt32 GetSamplesPerStream() const = 0;

        /**
         * @brief Checks whether the game is tracking environment amounts himself.
         *
         * @return Whether the game is tracking environment amounts.
         */
        [[nodiscard]] virtual bool IsGameTrackingEnvironmentAmounts() const = 0;

        /**
         * @brief Gets the maximum number of listeners handled by the engine.
         *
         * @return The maximum number of listeners.
         */
        [[nodiscard]] virtual AmUInt32 GetMaxListenersCount() const = 0;

        /**
         * @brief Gets the maximum number of game entities handled by the engine.
         *
         * This value does not reflect the maximum number of simultaneous sound handled by the engine.
         *
         * @return The maximum number of game entities.
         */
        [[nodiscard]] virtual AmUInt32 GetMaxEntitiesCount() const = 0;

        /**
         * @brief Gets the occlusion coefficient curve, as set in the loaded engine configuration file.
         *
         * @return The occlusion coefficient curve.
         */
        [[nodiscard]] virtual const Curve& GetOcclusionCoefficientCurve() const = 0;

        /**
         * @brief Gets the occlusion gain curve, as set in the loaded engine configuration file.
         *
         * @return The occlusion gain curve.
         */
        [[nodiscard]] virtual const Curve& GetOcclusionGainCurve() const = 0;

        /**
         * @brief Gets the obstruction coefficient curve, as set in the loaded engine configuration file.
         *
         * @return The obstruction coefficient curve.
         */
        [[nodiscard]] virtual const Curve& GetObstructionCoefficientCurve() const = 0;

        /**
         * @brief Gets the obstruction gain curve, as set in the loaded engine configuration file.
         *
         * @return The obstruction gain curve.
         */
        [[nodiscard]] virtual const Curve& GetObstructionGainCurve() const = 0;

        /**
         * @brief Gets the panning mode defined in the loaded engine configuration.
         *
         * @return The panning mode.
         */
        [[nodiscard]] virtual ePanningMode GetPanningMode() const = 0;

        /**
         * @brief Gets the HRIR sphere sampling mode defined in the loaded engine configuration.
         *
         * @return The HRIR sphere sampling mode.
         */
        [[nodiscard]] virtual eHRIRSphereSamplingMode GetHRIRSphereSamplingMode() const = 0;

        /**
         * @brief Gets the HRIR sphere defined in the loaded engine configuration.
         *
         * @return The HRIR sphere. If no HRIR sphere is defined, returns `nullptr`.
         *
         * @note The HRIR sphere is optional and can be null in some cases. If the
         * engine does not have an HRIR sphere defined, this function will return `nullptr`.
         *
         * @see HRIRSphere
         */
        [[nodiscard]] virtual const HRIRSphere* GetHRIRSphere() const = 0;

#pragma endregion

#pragma region Plugins Management

        /**
         * @brief Loads a plugin library from the given path.
         *
         * @param[in] pluginLibraryName The name of the plugin library to load.
         *
         * @return A handle to the loaded plugin library.
         */
        static AmVoidPtr LoadPlugin(const AmOsString& pluginLibraryName);

        /**
         * @brief Adds a path in the plugins search paths list.
         *
         * @param[in] path The path to add in the plugins search paths list.
         */
        static void AddPluginSearchPath(const AmOsString& path);

        /**
         * @brief Removes a path from the plugins search paths list.
         *
         * @param[in] path The path to remove from the plugins search path list.
         */
        static void RemovePluginSearchPath(const AmOsString& path);

        /**
         * @brief Register all default plugins.
         */
        static bool RegisterDefaultPlugins();

        /**
         * @brief Unregister all default plugins.
         */
        static bool UnregisterDefaultPlugins();

#pragma endregion

#pragma region Singleton Implementation

        /**
         * @brief Returns a unique instance of the Amplitude Engine.
         */
        [[nodiscard]] static Engine* GetInstance();

        /**
         * @brief Destroys the unique instance of the Amplitude Engine.
         */
        static void DestroyInstance();

#pragma endregion
    };

} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_CORE_ENGINE_H
