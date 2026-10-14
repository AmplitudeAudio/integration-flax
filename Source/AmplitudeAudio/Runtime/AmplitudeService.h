// Copyright (c) 2026-present Sparky Studios. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     `http://www.apache.org/licenses/LICENSE-2.0`
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

// clang-format off
#include <AmplitudeAudio/Core/Cpp20Compat.h>
// clang-format on

#include <atomic>
#include <memory>
#include <vector>

#include <Engine/Core/Collections/Array.h>
#include <Engine/Core/Types/String.h>
#include <Engine/Platform/CriticalSection.h>

#include <SparkyStudios/Audio/Amplitude/Core/Engine.h>
#include <SparkyStudios/Audio/Amplitude/IO/FileSystem.h>

#include <AmplitudeAudio/Core/BankRegistry.h>
#include <AmplitudeAudio/Runtime/IAmplitudeServiceClient.h>

struct AmplitudePlatformProfile;

namespace AmplitudeAudio
{
    /// <summary>Owns the Amplitude engine for one session: the whole run in a cooked game, one Play session in the editor.</summary>
    /// <remarks>
    /// Main thread only. The SDK locks only inside Initialize, Deinitialize and AdvanceFrame, so every other call must
    /// come from the thread that drives AdvanceFrame.
    /// </remarks>
    class AMPLITUDEAUDIO_API AmplitudeService
    {
    public:
        /// <summary>The single service instance.</summary>
        static AmplitudeService& Get();

        /// <summary>Starts with the running platform's profile from AmplitudeSettings.</summary>
        bool Startup();

        /// <summary>Starts with an explicit profile. Assets are read from <c>amplitudeRoot/buildPath</c>, or from <c>amplitudeRoot/buildPath.ampk</c> when that package exists.</summary>
        bool Startup(const AmplitudePlatformProfile& profile, const ::String& amplitudeRoot, const ::String& buildPath);

        /// <summary>Stops the engine and unloads every sound bank. Safe to call when not running.</summary>
        void Shutdown();

        /// <summary>Advances the engine by one frame. Driven by Engine::Update.</summary>
        void Tick();

        /// <summary>Whether the engine is started.</summary>
        [[nodiscard]] bool IsRunning() const;

        /// <summary>Whether assets are read from an .ampk package.</summary>
        [[nodiscard]] bool IsUsingPackage() const;

        /// <summary>Loads a bank or adds a reference; with wait, blocks until its files are loaded.</summary>
        bool LoadSoundBank(const ::String& file, bool wait);

        /// <summary>Releases a reference taken by LoadSoundBank.</summary>
        void UnloadSoundBank(const ::String& file);

        /// <summary>Whether the bank and its sound files are loaded.</summary>
        [[nodiscard]] bool IsSoundBankLoaded(const ::String& file) const;

        /// <summary>Registers a client that will be notified when the engine starts and stops.</summary>
        void AddClient(IAmplitudeServiceClient* client);

        /// <summary>Unregisters a client.</summary>
        void RemoveClient(IAmplitudeServiceClient* client);

        /// <summary>Returns the SDK engine pointer, or nullptr when not running.</summary>
        [[nodiscard]] SparkyStudios::Audio::Amplitude::Engine* GetEngine() const;

        /// <summary>Allocates a unique, non-reusable ID for scene objects (listener, entity, etc.).</summary>
        [[nodiscard]] SparkyStudios::Audio::Amplitude::AmObjectID AllocateId();

        /// <summary>Posts work to run on the main thread at the end of the next Tick.</summary>
        void Post(::Function<void()> work);

    private:
        Array<IAmplitudeServiceClient*> m_clients;
        CriticalSection m_postLock;
        Array<Function<void()>> m_posted;
        std::atomic<uint64> m_nextId{1};
        AmplitudeService();

        static void OnEngineUpdate();
        static void OnEnginePause();
        static void OnEngineUnpause();

        bool OpenFileSystem(const ::String& amplitudeRoot, const ::String& buildPath);
        void LoadPlugins(const AmplitudePlatformProfile& profile, const ::String& amplitudeRoot);
        void ApplyPause();
        bool Fail(const ::String& message);

        SparkyStudios::Audio::Amplitude::Engine* m_engine = nullptr;
        std::shared_ptr<SparkyStudios::Audio::Amplitude::FileSystem> m_fileSystem;
        std::vector<SparkyStudios::Audio::Amplitude::AmOsString> m_pluginSearchPaths;
        BankRegistry m_banks;
        bool m_running = false;
        bool m_ownsMemoryManager = false;
        bool m_usingPackage = false;
        bool m_extensionsRegistered = false;
        bool m_fileSystemOpened = false;
        bool m_appPaused = false;
        bool m_paused = false;
    };
} // namespace AmplitudeAudio
