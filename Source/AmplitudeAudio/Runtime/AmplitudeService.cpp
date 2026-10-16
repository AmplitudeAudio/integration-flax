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

#include "AmplitudeService.h"

#include <chrono>
#include <thread>

#include <Engine/Core/Collections/HashSet.h>
#include <Engine/Core/Log.h>
#include <Engine/Engine/Engine.h>
#include <Engine/Engine/Globals.h>
#include <Engine/Engine/Time.h>
#include <Engine/Platform/FileSystem.h>
#include <Engine/Platform/Platform.h>
#include <Engine/Profiler/ProfilerCPU.h>

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>
#include <SparkyStudios/Audio/Amplitude/IO/Log.h>
#include <SparkyStudios/Audio/Amplitude/IO/PackageFileSystem.h>

#include <AmplitudeAudio/AmplitudeSettings.h>
#include <AmplitudeAudio/Actors/AmplitudeEntity.h>
#include <AmplitudeAudio/Core/Common.h>
#include <AmplitudeAudio/Core/FlaxMemoryAllocator.h>
#include <AmplitudeAudio/Core/Helpers.h>
#include <AmplitudeAudio/IO/FlaxFileSystem.h>
#include <AmplitudeAudio/IO/FlaxLogger.h>
#include <AmplitudeAudio/Runtime/IAmplitudeServiceClient.h>
#include <AmplitudeAudio/Runtime/Occlusion/PhysicsOcclusionCaster.h>

#if COMPILE_WITH_DEBUG_DRAW
#include <Engine/Debug/DebugDraw.h>
#endif

namespace AmplitudeAudio
{
    using namespace SparkyStudios::Audio::Amplitude;
    using AmEngine = SparkyStudios::Audio::Amplitude::Engine;

    namespace
    {
        constexpr int32 kBlockingTimeoutMs = 10000;

        FlaxLogger gLogger;

        // Plugin libraries can't be unloaded, and loading one again re-registers its extensions, so each one is
        // loaded once per process even though the editor starts the engine on every Play session.
        HashSet<::String> gLoadedPlugins;

        template <typename Fn>
        bool PollWithTimeout(Fn&& tryFinalize)
        {
            const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(kBlockingTimeoutMs);
            while (!tryFinalize())
            {
                if (std::chrono::steady_clock::now() >= deadline)
                    return false;

                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }

            return true;
        }

        BankRegistry::Callbacks MakeBankCallbacks(AmEngine*& engine)
        {
            BankRegistry::Callbacks callbacks;
            callbacks.Load = [&engine](const ::String& file, BankRegistry::BankId& outId)
            {
                // AmBankID and Flax's uint64 can be distinct 64-bit types, so the ID goes through a local.
                AmBankID id = 0;
                const bool loaded = engine->LoadSoundBank(FlaxStringToAmOsString(file), id);
                outId = id;
                return loaded;
            };
            callbacks.Unload = [&engine](BankRegistry::BankId id)
            {
                engine->UnloadSoundBank(static_cast<AmBankID>(id));
            };
            callbacks.StartLoadFiles = [&engine]
            {
                engine->StartLoadSoundFiles();
            };
            callbacks.TryFinalizeLoadFiles = [&engine]
            {
                return engine->TryFinalizeLoadSoundFiles();
            };
            return callbacks;
        }
    } // namespace

    AmplitudeService& AmplitudeService::Get()
    {
        static AmplitudeService instance;
        return instance;
    }

    AmplitudeService::AmplitudeService()
        : m_banks(MakeBankCallbacks(m_engine))
    {
    }

    bool AmplitudeService::Startup()
    {
        const AmplitudeSettings* settings = AmplitudeSettings::Get();
        return Startup(settings->GetProfile(::Platform::GetPlatformType()), Globals::ProjectFolder / kAssetBasePath, settings->ProjectBuildPath);
    }

    bool AmplitudeService::Startup(const AmplitudePlatformProfile& profile, const ::String& amplitudeRoot, const ::String& buildPath)
    {
        if (m_running)
            return true;

        PROFILE_CPU();

        if (profile.EngineConfigFile.IsEmpty())
            return Fail(TEXT("The Amplitude profile for this platform has no engine config file. Set it in Amplitude Settings."));

        // Owned here rather than by the plugin: in the editor the plugin only initializes during Play, but tests start
        // the service in edit mode. Must run before anything allocates through the SDK.
        if (!MemoryManager::IsInitialized())
        {
            MemoryManager::Initialize(std::make_unique<FlaxMemoryAllocator>());
            m_ownsMemoryManager = true;
        }

        Logger::SetLogger(&gLogger);

        AmEngine::RegisterDefaultExtensions();
        m_extensionsRegistered = true;

        LoadPlugins(profile, amplitudeRoot);

        m_engine = AmEngine::GetInstance();
        if (m_engine == nullptr)
            return Fail(TEXT("Could not create the Amplitude engine instance."));

        if (!OpenFileSystem(amplitudeRoot, buildPath))
            return Fail(TEXT("Timed out opening the Amplitude file system."));

        if (!m_engine->Initialize(FlaxStringToAmOsString(profile.EngineConfigFile)))
            return Fail(::String::Format(TEXT("Amplitude failed to initialize with config '{0}'."), profile.EngineConfigFile));

        for (const AmplitudeSoundBankRef& bank : profile.InitSoundBanks)
        {
            if (!m_banks.Acquire(bank.File))
                return Fail(::String::Format(TEXT("Failed to load init sound bank '{0}'."), bank.File));
        }

        if (!m_banks.WaitForPending(kBlockingTimeoutMs))
            return Fail(TEXT("Timed out loading the init sound banks' files."));

        // Engine::Update fires at the start of a frame: one tick commits everything the previous frame wrote,
        // including script LateUpdate, which has no Flax hook after it.
        ::Engine::Update.Bind<&AmplitudeService::OnEngineUpdate>();
        ::Engine::Pause.Bind<&AmplitudeService::OnEnginePause>();
        ::Engine::Unpause.Bind<&AmplitudeService::OnEngineUnpause>();

        m_surfacesDirty = true;
        m_running = true;
        LOG(Info, "[Amplitude] Engine started with config '{0}'.", profile.EngineConfigFile);

        m_startupWarnings = BuildTrackingWarnings(m_engine->IsGameTrackingRooms(), m_engine->IsGameTrackingEnvironmentAmounts());
        for (const ::String& warning : m_startupWarnings)
            LOG(Warning, "[Amplitude] {0}", warning);

        // Notify clients that the engine has started
        for (IAmplitudeServiceClient* client : m_clients)
            client->OnEngineStarted();

        return true;
    }

    void AmplitudeService::Shutdown()
    {
        const bool wasRunning = m_running;

        if (m_running)
        {
            // Notify clients before shutting down; handles are still valid
            for (IAmplitudeServiceClient* client : m_clients)
                client->OnEngineStopping();

            m_occlusion.Clear();

            ::Engine::Update.Unbind<&AmplitudeService::OnEngineUpdate>();
            ::Engine::Pause.Unbind<&AmplitudeService::OnEnginePause>();
            ::Engine::Unpause.Unbind<&AmplitudeService::OnEngineUnpause>();
            m_running = false;
        }
        else
        {
            m_occlusion.Clear();
        }

        if (m_engine != nullptr)
        {
            if (m_engine->IsInitialized())
            {
                m_banks.ReleaseAll();
                m_engine->Deinitialize();
            }

            if (m_fileSystemOpened)
            {
                m_engine->StartCloseFileSystem();
                if (!PollWithTimeout([this] { return m_engine->TryFinalizeCloseFileSystem(); }))
                    LOG(Warning, "[Amplitude] Timed out closing the file system.");
            }
        }

        for (const AmOsString& path : m_pluginSearchPaths)
            AmEngine::RemovePluginSearchPath(path);
        m_pluginSearchPaths.clear();

        if (m_extensionsRegistered)
        {
            AmEngine::UnregisterDefaultExtensions();
            m_extensionsRegistered = false;
        }

        Logger::SetLogger(nullptr);

        m_fileSystem.reset();
        m_fileSystemOpened = false;
        m_usingPackage     = false;

        if (m_engine != nullptr)
        {
            AmEngine::DestroyInstance();
            m_engine = nullptr;
        }

        if (m_ownsMemoryManager)
        {
            MemoryManager::Deinitialize();
            m_ownsMemoryManager = false;
        }

        m_appPaused = false;
        m_paused    = false;
        m_startupWarnings.Clear();
        m_rooms.Clear();
        m_environments.Clear();
        m_spatialDirty = true;

        if (wasRunning)
            LOG(Info, "[Amplitude] Engine stopped.");
    }

    Array<::String> AmplitudeService::BuildTrackingWarnings(bool trackingRooms, bool trackingEnvironments)
    {
        Array<::String> warnings;
        if (!trackingRooms)
            warnings.Add(TEXT("The engine config has track_rooms = false: AmplitudeRoom actors will not be applied. Set \"track_rooms\": true in the config's \"game\" section."));
        if (!trackingEnvironments)
            warnings.Add(TEXT("The engine config has track_environments = false: AmplitudeEnvironment factors will not be applied. Set \"track_environments\": true in the config's \"game\" section."));
        return warnings;
    }

    void AmplitudeService::Tick()
    {
        if (!m_running)
            return;

        PROFILE_CPU_NAMED("Amplitude.AdvanceFrame");

        ApplyPause();
        m_banks.Update();

        const AmplitudeSettings* settings = AmplitudeSettings::Get();
        const AmplitudeObstructionSettings defaults;
        const AmplitudeObstructionSettings& obstruction = settings ? settings->Obstruction : defaults;

        if (m_surfacesDirty || m_surfaces.HasUnresolved())
        {
            m_surfaces.Build(obstruction);
            m_surfacesDirty = false;
        }

        if (m_spatialDirty)
            RebuildSpatialWorld();

        m_spatial.Update(m_engine->IsGameTrackingRooms(), m_engine->IsGameTrackingEnvironmentAmounts());

        const auto listener = m_engine->GetDefaultListener();
        if (listener.Valid() && m_occlusion.GetSourceCount() > 0)
        {
            const PhysicsOcclusionCaster caster(obstruction, m_surfaces);
#if COMPILE_WITH_DEBUG_DRAW
            if (obstruction.DebugDraw)
            {
                m_occlusion.SetRayObserver([](const Vector3& from, const Vector3& to, float amount, bool isDirect) {
                    Color color = Color::Lerp(Color::Green, Color::Red, amount);
                    if (!isDirect)
                        color.A *= 0.5f;
                    DebugDraw::DrawLine(from, to, color, 0.0f);
                });
            }
#endif
            m_occlusion.Update(AmVec3ToFlaxVec3(listener.GetLocation()), ::Time::Update.UnscaledDeltaTime.GetTotalSeconds(), obstruction,
                [&caster](IOcclusionSource& source, const Vector3& from, const Vector3& to) {
                    auto* entity = static_cast<AmplitudeEntity*>(&source);
                    return caster.Cast(entity, entity->ObstructionLayers.Mask, from, to);
                });
#if COMPILE_WITH_DEBUG_DRAW
            if (obstruction.DebugDraw)
            {
                m_occlusion.ClearRayObserver();
            }
#endif
        }

        m_engine->AdvanceFrame(::Time::Update.UnscaledDeltaTime.GetTotalMilliseconds());

        // Dispatch posted work on the main thread
        Array<Function<void()>> posted;
        {
            ScopeLock lock(m_postLock);
            m_posted.Swap(posted);
        }
        for (Function<void()>& work : posted)
            work();
    }

    bool AmplitudeService::IsRunning() const
    {
        return m_running;
    }

    bool AmplitudeService::IsUsingPackage() const
    {
        return m_usingPackage;
    }

    bool AmplitudeService::LoadSoundBank(const ::String& file, bool wait)
    {
        if (!m_running)
        {
            LOG(Warning, "[Amplitude] Cannot load sound bank '{0}': the engine is not running.", file);
            return false;
        }

        if (!m_banks.Acquire(file))
        {
            LOG(Error, "[Amplitude] Failed to load sound bank '{0}'.", file);
            return false;
        }

        if (m_banks.RefCount(file) == 1)
            LOG(Info, "[Amplitude] Loaded sound bank '{0}'.", file);

        if (wait && !m_banks.WaitForPending(kBlockingTimeoutMs))
            LOG(Warning, "[Amplitude] Timed out loading the files of sound bank '{0}'.", file);

        return true;
    }

    void AmplitudeService::UnloadSoundBank(const ::String& file)
    {
        if (!m_running || m_banks.RefCount(file) == 0)
            return;

        m_banks.Release(file);

        if (m_banks.RefCount(file) == 0)
            LOG(Info, "[Amplitude] Unloaded sound bank '{0}'.", file);
    }

    bool AmplitudeService::IsSoundBankLoaded(const ::String& file) const
    {
        return m_running && m_banks.IsLoaded(file);
    }

    void AmplitudeService::AddClient(IAmplitudeServiceClient* client)
    {
        m_clients.AddUnique(client);
        if (m_running)
            client->OnEngineStarted();
    }

    void AmplitudeService::RemoveClient(IAmplitudeServiceClient* client)
    {
        m_clients.Remove(client);
    }

    SparkyStudios::Audio::Amplitude::Engine* AmplitudeService::GetEngine() const
    {
        return m_running ? m_engine : nullptr;
    }

    SparkyStudios::Audio::Amplitude::AmObjectID AmplitudeService::AllocateId()
    {
        return m_nextId.fetch_add(1);
    }

    void AmplitudeService::Post(::Function<void()> work)
    {
        ScopeLock lock(m_postLock);
        m_posted.Add(MoveTemp(work));
    }

    void AmplitudeService::RebuildSpatialWorld()
    {
        Array<Spatial::RoomCandidate> rooms;
        rooms.EnsureCapacity(m_rooms.Count());
        for (auto it = m_rooms.Begin(); it != m_rooms.End(); ++it)
            rooms.Add(it->Value.Candidate);

        Array<EnvironmentCandidate> envs;
        envs.EnsureCapacity(m_environments.Count());
        for (auto it = m_environments.Begin(); it != m_environments.End(); ++it)
            envs.Add(it->Value);

        m_spatial.SetWorld(rooms, envs);
        m_spatialDirty = false;
    }

    void AmplitudeService::RegisterRoom(uint64 id, const Room& handle, const Spatial::RoomCandidate& candidate)
    {
        m_rooms[id] = { handle, candidate };
        m_spatialDirty = true;
    }

    void AmplitudeService::UpdateRoom(uint64 id, const Spatial::RoomCandidate& candidate)
    {
        RoomEntry* entry = m_rooms.TryGet(id);
        if (entry)
        {
            entry->Candidate = candidate;
            m_spatialDirty = true;
        }
    }

    void AmplitudeService::UnregisterRoom(uint64 id)
    {
        m_rooms.Remove(id);
        m_spatialDirty = true;
    }

    void AmplitudeService::RegisterEnvironment(const EnvironmentCandidate& candidate)
    {
        m_environments[candidate.Id] = candidate;
        m_spatialDirty = true;
    }

    void AmplitudeService::UpdateEnvironment(const EnvironmentCandidate& candidate)
    {
        m_environments[candidate.Id] = candidate;
        m_spatialDirty = true;
    }

    void AmplitudeService::UnregisterEnvironment(uint64 id)
    {
        m_environments.Remove(id);
        m_spatialDirty = true;
    }

    Room AmplitudeService::FindRoom(uint64 id) const
    {
        const RoomEntry* entry = m_rooms.TryGet(id);
        return entry ? entry->Handle : Room();
    }

    void AmplitudeService::OnEngineUpdate()
    {
        Get().Tick();
    }

    void AmplitudeService::OnEnginePause()
    {
        Get().m_appPaused = true;
    }

    void AmplitudeService::OnEngineUnpause()
    {
        Get().m_appPaused = false;
    }

    bool AmplitudeService::OpenFileSystem(const ::String& amplitudeRoot, const ::String& buildPath)
    {
        auto flaxFileSystem = std::make_shared<FlaxFileSystem>();

        const ::String packageFile = buildPath + kExtPack;
        m_usingPackage = ::FileSystem::FileExists(amplitudeRoot / packageFile);

        if (!m_usingPackage)
        {
            flaxFileSystem->SetBasePath(FlaxStringToAmOsString(amplitudeRoot / buildPath));
            m_fileSystem = flaxFileSystem;
        }
        else
        {
            flaxFileSystem->SetBasePath(FlaxStringToAmOsString(amplitudeRoot));

            // The platform file system must be set before the base path: SetBasePath resolves through it.
            auto packageFileSystem = std::make_shared<PackageFileSystem>();
            packageFileSystem->SetPlatformFileSystem(flaxFileSystem);
            packageFileSystem->SetBasePath(FlaxStringToAmOsString(packageFile));
            m_fileSystem = packageFileSystem;

            LOG(Info, "[Amplitude] Reading assets from package '{0}'.", packageFile);
        }

        m_engine->SetFileSystem(m_fileSystem);
        m_engine->StartOpenFileSystem();

        if (!PollWithTimeout([this] { return m_engine->TryFinalizeOpenFileSystem(); }))
            return false;

        m_fileSystemOpened = true;
        return true;
    }

    void AmplitudeService::LoadPlugins(const AmplitudePlatformProfile& profile, const ::String& amplitudeRoot)
    {
        for (const AmplitudePluginSearchPath& searchPath : profile.PluginSearchPaths)
        {
            const AmOsString path = FlaxStringToAmOsString(amplitudeRoot / searchPath.Path);
            AmEngine::AddPluginSearchPath(path);
            m_pluginSearchPaths.push_back(path);
        }

        for (const AmplitudePlugin& plugin : profile.Plugins)
        {
            if (gLoadedPlugins.Contains(plugin.Name))
                continue;

            if (AmEngine::LoadPlugin(FlaxStringToAmOsString(plugin.Name)) == nullptr)
            {
                LOG(Warning, "[Amplitude] Could not load plugin '{0}'.", plugin.Name);
                continue;
            }

            gLoadedPlugins.Add(plugin.Name);
        }
    }

    void AmplitudeService::ApplyPause()
    {
        const bool paused = m_appPaused || ::Time::GetGamePaused();
        if (paused == m_paused)
            return;

        m_paused = paused;
        m_engine->Pause(paused);
    }

    bool AmplitudeService::Fail(const ::String& message)
    {
        LOG(Error, "[Amplitude] {0}", message);
        Shutdown();
        return false;
    }
} // namespace AmplitudeAudio
