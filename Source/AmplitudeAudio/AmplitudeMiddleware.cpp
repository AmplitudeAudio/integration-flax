// ============================================================================
//  AmplitudeMiddleware — Core AudioMiddleware implementation
//  (implementation)
//
//  Implements all 48 methods of the AudioMiddleware interface by delegating
//  to the Amplitude Audio SDK Engine singleton.
// ============================================================================

#include "AmplitudeMiddleware.h"

#include "Core/AmplitudeAudioData.h"
#include "Core/AmplitudeHelpers.h"
#include "Core/Common.h"
#include "IO/AmplitudeFile.h"
#include "IO/AmplitudeFileSystem.h"

#include <Engine/Engine/Globals.h>
#include <Engine/Content/Content.h>
#include <Engine/Content/JsonAsset.h>
#include <Engine/Core/Log.h>
#include <Engine/Platform/FileSystem.h>
#include <Engine/Platform/StringUtils.h>
#include <Engine/Serialization/Serialization.h>
#include <Engine/Platform/File.h>

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>

#include <chrono>
#include <thread>

namespace AmplitudeAudio
{
    using namespace SparkyStudios::Audio::Amplitude;
    using AmEngine = SparkyStudios::Audio::Amplitude::Engine;

    namespace
    {
        /// Poll a finalize callback with a timeout. Returns true if finalized,
        /// false if the timeout was reached.
        template<typename Fn>
        bool PollWithTimeout(Fn&& tryFinalize, int timeoutMs = 10000, int intervalMs = 10)
        {
            const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs);
            while (!tryFinalize())
            {
                if (std::chrono::steady_clock::now() >= deadline)
                    return false;
                std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
            }
            return true;
        }
    } // anonymous namespace

    // ========================================================================
    //  Construction / Destruction
    // ========================================================================

    AmplitudeMiddleware::AmplitudeMiddleware()
        : m_pEngine(nullptr)
        , m_pFileSystem(nullptr)
        , m_profiles()
        , m_bInitialized(false)
    {
    }

    AmplitudeMiddleware::~AmplitudeMiddleware()
    {
        if (m_bInitialized)
        {
            Shutdown();
        }

        Release();
    }

    // ========================================================================
    //  Lifecycle (6)
    // ========================================================================

    bool AmplitudeMiddleware::LoadConfiguration(StringView configFile)
    {
        // Load the JSON asset from the Flax content system.
        const auto asset = Content::Load<JsonAsset>(configFile);
        if (asset == nullptr)
        {
            LOG(Error, "AmplitudeMiddleware: Failed to load configuration asset: {0}", String(configFile));
            return false;
        }

        if (asset->WaitForLoaded())
        {
            LOG(Error, "AmplitudeMiddleware: Configuration asset failed to load: {0}", String(configFile));
            return false;
        }

        if (asset->Data == nullptr)
        {
            LOG(Error, "AmplitudeMiddleware: Configuration asset has no data: {0}", String(configFile));
            return false;
        }

        // Deserialize configuration directly from the JSON Data node.
        Array<AmplitudePlatformProfile> profiles;
        String projectBuildPath;
        String projectSourcesPath;
        {
            auto& stream = *asset->Data;
            ISerializeModifier* modifier = nullptr;
            DESERIALIZE_MEMBER(Profiles, profiles);
            DESERIALIZE_MEMBER(ProjectBuildPath, projectBuildPath);
            DESERIALIZE_MEMBER(ProjectSourcesPath, projectSourcesPath);
        }

        // Populate the internal profiles map.
        m_profiles.LoadFromAsset(profiles, projectBuildPath, projectSourcesPath);

        // Validate that a profile exists for the current platform.
        const String platform = AmplitudeAssetProfiles::GetCurrentPlatform();
        const AmplitudeConfiguration* config = m_profiles.Get(platform);
        if (config == nullptr)
        {
            LOG(Error, "AmplitudeMiddleware: No configuration profile found for platform: {0}", platform);
            return false;
        }

        LOG(Info, "AmplitudeMiddleware: Configuration loaded for platform: {0}", platform);
        return true;
    }

    bool AmplitudeMiddleware::Startup()
    {
        if (m_bInitialized)
        {
            LOG(Warning, "AmplitudeMiddleware: Already initialized");
            return true;
        }

        // 1. Retrieve the current platform configuration.
        const String platform = AmplitudeAssetProfiles::GetCurrentPlatform();
        const AmplitudeConfiguration* config = m_profiles.Get(platform);
        if (config == nullptr)
        {
            LOG(Error, "AmplitudeMiddleware: No configuration profile for platform: {0}. Call LoadConfiguration first.", platform);
            return false;
        }

        // 2. Initialize the Amplitude memory manager with Flax-backed allocator.
        MemoryManager::Initialize(std::make_unique<FlaxMemoryAllocator>());

        // 3. Set up the Flax logger as the default Amplitude logger.
        static FlaxLogger sFlaxLogger;
        Logger::SetLogger(&sFlaxLogger);

        // 4. Register default Amplitude SDK extensions (codecs, drivers, etc.).
        AmEngine::RegisterDefaultExtensions();

        // 5. Create and configure the custom file system.
        //    Base path = {kAssetBasePath}/{ProjectBuildPath}
        m_pFileSystem = std::make_shared<AmplitudeFileSystem>();
        const String& buildPath = m_profiles.GetProjectBuildPath();
        String basePath = String(kAssetBasePath);
        if (!buildPath.IsEmpty())
        {
            basePath += buildPath;
        }
        const StringAnsi basePathAnsi(basePath.Get(), basePath.Length());
        m_pFileSystem->SetBasePath(AM_STRING_TO_OS_STRING(basePathAnsi.Get()));

        // 6. Get the engine singleton and attach the file system.
        m_pEngine = AmEngine::GetInstance();
        if (m_pEngine == nullptr)
        {
            LOG(Error, "AmplitudeMiddleware: Failed to get Engine instance");
            return false;
        }

        m_pEngine->SetFileSystem(m_pFileSystem);

        // 7. Open the file system.
        m_pEngine->StartOpenFileSystem();
        if (!PollWithTimeout([this]() { return m_pEngine->TryFinalizeOpenFileSystem(); }))
        {
            LOG(Error, "AmplitudeMiddleware: Timed out opening file system");
            return false;
        }

        // 8. Initialize the engine with the platform config file.
        const StringAnsi configFileAnsi(config->EngineConfigFile.Get(), config->EngineConfigFile.Length());
        const AmOsString configFileOs = AM_STRING_TO_OS_STRING(configFileAnsi.Get());
        if (!m_pEngine->Initialize(configFileOs))
        {
            LOG(Error, "AmplitudeMiddleware: Engine initialization failed with config: {0}", config->EngineConfigFile);
            return false;
        }

        // 9. Load the initial sound bank and finalize sound file loading.
        const StringAnsi initBankAnsi(config->InitSoundBank.Get(), config->InitSoundBank.Length());
        const AmOsString initBankOs = AM_STRING_TO_OS_STRING(initBankAnsi.Get());
        if (!m_pEngine->LoadSoundBank(initBankOs))
        {
            LOG(Error, "AmplitudeMiddleware: Failed to load init sound bank: {0}", config->InitSoundBank);
            return false;
        }

        m_pEngine->StartLoadSoundFiles();
        if (!PollWithTimeout([this]() { return m_pEngine->TryFinalizeLoadSoundFiles(); }))
        {
            LOG(Error, "AmplitudeMiddleware: Timed out loading init sound bank files");
            return false;
        }

        m_bInitialized = true;
        LOG(Info, "AmplitudeMiddleware: Startup complete");
        return true;
    }

    void AmplitudeMiddleware::Update(float dt)
    {
        if (!m_bInitialized)
            return;

        // Convert seconds to milliseconds (AmTime is in ms).
        m_pEngine->AdvanceFrame(static_cast<AmTime>(dt * 1000.0));
    }

    void AmplitudeMiddleware::Shutdown()
    {
        if (!m_bInitialized)
            return;

        m_bInitialized = false;

        // Deinitialize the engine (stops all audio processing).
        if (m_pEngine != nullptr)
        {
            m_pEngine->Deinitialize();

            // Close the file system.
            m_pEngine->StartCloseFileSystem();
            if (!PollWithTimeout([this]() { return m_pEngine->TryFinalizeCloseFileSystem(); }))
            {
                LOG(Warning, "AmplitudeMiddleware: Timed out closing file system during shutdown");
            }
        }

        // Unregister default extensions and destroy the engine singleton.
        AmEngine::UnregisterDefaultExtensions();
        AmEngine::DestroyInstance();
        m_pEngine = nullptr;

        // Tear down the Amplitude memory manager.
        MemoryManager::Deinitialize();

        LOG(Info, "AmplitudeMiddleware: Shutdown complete");
    }

    void AmplitudeMiddleware::Release()
    {
        m_pFileSystem.reset();

        LOG(Info, "AmplitudeMiddleware: Released");
    }

    void AmplitudeMiddleware::StopAllSounds()
    {
        if (!m_bInitialized)
            return;

        m_pEngine->StopAll();
    }

    // ========================================================================
    //  DeployFiles (editor only)
    // ========================================================================

#if USE_EDITOR
    bool AmplitudeMiddleware::DeployFiles(const StringView& outputPath)
    {
        const String& buildPath = m_profiles.GetProjectBuildPath();
        if (buildPath.IsEmpty())
        {
            LOG(Warning, "[AmplitudeMiddleware] DeployFiles: no ProjectBuildPath configured — nothing to deploy.");
            return true;
        }

        // Source: <ProjectFolder>/Content/Sounds/Amplitude/<BuildPath>/
        const String sourceDir = Globals::ProjectFolder / String(kAssetBasePath) / buildPath;
        if (!FileSystem::DirectoryExists(sourceDir))
        {
            LOG(Warning, "[AmplitudeMiddleware] DeployFiles: build directory not found: {0}", sourceDir);
            return false;
        }

        // Destination: <outputPath>/Content/Sounds/Amplitude/<BuildPath>/
        const String destDir = String(outputPath) / String(kAssetBasePath) / buildPath;

        if (!FileSystem::DirectoryExists(destDir))
        {
            if (FileSystem::CreateDirectory(destDir))
            {
                LOG(Error, "[AmplitudeMiddleware] DeployFiles: failed to create directory: {0}", destDir);
                return false;
            }
        }

        Array<String, HeapAllocation> files;
        FileSystem::DirectoryGetFiles(files, sourceDir, TEXT("*"), DirectorySearchOption::AllDirectories);

        int32 copiedCount = 0;
        for (const String& srcFile : files)
        {
            const String relativePart = srcFile.Substring(sourceDir.Length());
            const String destFile = destDir + relativePart;

            const String destFileDir = String(StringUtils::GetDirectoryName(destFile));
            if (!FileSystem::DirectoryExists(destFileDir))
            {
                if (FileSystem::CreateDirectory(destFileDir))
                {
                    LOG(Warning, "[AmplitudeMiddleware] DeployFiles: could not create directory: {0}", destFileDir);
                    continue;
                }
            }

            if (FileSystem::CopyFile(destFile, srcFile))
            {
                LOG(Warning, "[AmplitudeMiddleware] DeployFiles: failed to copy: {0}", srcFile);
                continue;
            }

            ++copiedCount;
        }

        LOG(Info, "[AmplitudeMiddleware] DeployFiles: copied {0} file(s) from {1} to {2}", copiedCount, sourceDir, destDir);
        return true;
    }
#endif

    // ========================================================================
    //  Entity (8)
    // ========================================================================

    AudioSystemEntityData* AmplitudeMiddleware::CreateWorldEntity(AudioSystemDataID id)
    {
        auto* entityData = new AmplitudeEntityData();
        entityData->EntityID = static_cast<AmEntityID>(id);
        entityData->HasPosition = false;

        if (m_bInitialized)
        {
            m_pEngine->AddEntity(entityData->EntityID);
        }

        return entityData;
    }

    AudioSystemEntityData* AmplitudeMiddleware::CreateEntityData(AudioSystemDataID id)
    {
        auto* entityData = new AmplitudeEntityData();
        entityData->EntityID = static_cast<AmEntityID>(id);
        entityData->HasPosition = true;
        return entityData;
    }

    void AmplitudeMiddleware::DestroyEntityData(AudioSystemEntityData* data)
    {
        delete static_cast<AmplitudeEntityData*>(data);
    }

    bool AmplitudeMiddleware::AddEntity(AudioSystemDataID id, AudioSystemEntityData* data)
    {
        if (!m_bInitialized)
            return false;

        auto* entityData = static_cast<AmplitudeEntityData*>(data);
        const Entity entity = m_pEngine->AddEntity(entityData->EntityID);
        if (!entity.Valid())
        {
            LOG(Error, "AmplitudeMiddleware: Failed to add entity with ID: {0}", id);
            return false;
        }

        return true;
    }

    bool AmplitudeMiddleware::ResetEntity(AudioSystemDataID id, AudioSystemEntityData* data)
    {
        if (!m_bInitialized)
            return false;

        // Reset obstruction/occlusion to defaults.
        auto* entityData = static_cast<AmplitudeEntityData*>(data);
        const Entity entity = m_pEngine->GetEntity(entityData->EntityID);
        if (!entity.Valid())
            return false;

        entity.SetObstruction(0.0f);
        entity.SetOcclusion(0.0f);
        return true;
    }

    bool AmplitudeMiddleware::UpdateEntity(AudioSystemDataID id, AudioSystemEntityData* data)
    {
        // Entity state is tracked externally by the AudioSystem; nothing to do here.
        return m_bInitialized;
    }

    bool AmplitudeMiddleware::RemoveEntity(AudioSystemDataID id, AudioSystemEntityData* data)
    {
        if (!m_bInitialized)
            return false;

        auto* entityData = static_cast<AmplitudeEntityData*>(data);
        m_pEngine->RemoveEntity(entityData->EntityID);
        return true;
    }

    bool AmplitudeMiddleware::SetEntityTransform(
        AudioSystemDataID id, AudioSystemEntityData* data, const AudioSystemTransform& transform)
    {
        if (!m_bInitialized)
            return false;

        auto* entityData = static_cast<AmplitudeEntityData*>(data);
        if (!entityData->HasPosition)
            return true; // World entity does not track position.

        const Entity entity = m_pEngine->GetEntity(entityData->EntityID);
        if (!entity.Valid())
            return false;

        AmVector3 location;
        Orientation orientation(0.0f, 0.0f, 0.0f);
        FlaxTransformToAmplitude(transform, location, orientation);

        entity.SetLocation(location);
        entity.SetOrientation(orientation);
        return true;
    }

    // ========================================================================
    //  Listener (6)
    // ========================================================================

    AudioSystemListenerData* AmplitudeMiddleware::CreateListenerData(AudioSystemDataID id)
    {
        auto* listenerData = new AmplitudeListenerData();
        listenerData->ListenerID = static_cast<AmListenerID>(id);
        return listenerData;
    }

    void AmplitudeMiddleware::DestroyListenerData(AudioSystemListenerData* data)
    {
        delete static_cast<AmplitudeListenerData*>(data);
    }

    bool AmplitudeMiddleware::AddListener(AudioSystemDataID id, AudioSystemListenerData* data)
    {
        if (!m_bInitialized)
            return false;

        auto* listenerData = static_cast<AmplitudeListenerData*>(data);
        const Listener listener = m_pEngine->AddListener(listenerData->ListenerID);
        if (!listener.Valid())
        {
            LOG(Error, "AmplitudeMiddleware: Failed to add listener with ID: {0}", id);
            return false;
        }

        return true;
    }

    bool AmplitudeMiddleware::ResetListener(AudioSystemDataID id, AudioSystemListenerData* data)
    {
        if (!m_bInitialized)
            return false;

        auto* listenerData = static_cast<AmplitudeListenerData*>(data);
        const Listener listener = m_pEngine->GetListener(listenerData->ListenerID);
        if (!listener.Valid())
            return false;

        // Reset to origin with default orientation.
        const AmVector3 origin = { 0.0f, 0.0f, 0.0f };
        listener.SetLocation(origin);
        listener.SetOrientation(Orientation(0.0f, 0.0f, 0.0f));
        return true;
    }

    bool AmplitudeMiddleware::RemoveListener(AudioSystemDataID id, AudioSystemListenerData* data)
    {
        if (!m_bInitialized)
            return false;

        auto* listenerData = static_cast<AmplitudeListenerData*>(data);
        m_pEngine->RemoveListener(listenerData->ListenerID);
        return true;
    }

    bool AmplitudeMiddleware::SetListenerTransform(
        AudioSystemDataID id, AudioSystemListenerData* data, const AudioSystemTransform& transform)
    {
        if (!m_bInitialized)
            return false;

        auto* listenerData = static_cast<AmplitudeListenerData*>(data);
        const Listener listener = m_pEngine->GetListener(listenerData->ListenerID);
        if (!listener.Valid())
            return false;

        AmVector3 location;
        Orientation orientation(0.0f, 0.0f, 0.0f);
        FlaxTransformToAmplitude(transform, location, orientation);

        listener.SetLocation(location);
        listener.SetOrientation(orientation);
        return true;
    }

    // ========================================================================
    //  Event (3)
    // ========================================================================

    AudioSystemEventData* AmplitudeMiddleware::CreateEventData(AudioSystemDataID id)
    {
        auto* eventData = new AmplitudeEventData();
        eventData->EventID = static_cast<AmEventID>(id);
        eventData->State = PlaybackState::Idle;
        return eventData;
    }

    void AmplitudeMiddleware::ResetEventData(AudioSystemEventData* data)
    {
        auto* eventData = static_cast<AmplitudeEventData*>(data);
        eventData->Canceler = EventCanceler();
        eventData->State = PlaybackState::Idle;
    }

    void AmplitudeMiddleware::DestroyEventData(AudioSystemEventData* data)
    {
        delete static_cast<AmplitudeEventData*>(data);
    }

    // ========================================================================
    //  Trigger (5)
    // ========================================================================

    bool AmplitudeMiddleware::LoadTrigger(
        AudioSystemDataID entityId, AudioSystemTriggerData* triggerData, AudioSystemEventData* eventData)
    {
        if (!m_bInitialized)
            return false;

        auto* amTrigger = static_cast<AmplitudeTriggerData*>(triggerData);
        auto* amEvent = static_cast<AmplitudeEventData*>(eventData);

        // Store the event ID from the trigger so we can look up the handle later.
        amEvent->EventID = amTrigger->EventID;

        // Verify the event handle exists in the engine.
        const EventHandle handle = m_pEngine->GetEventHandle(amTrigger->EventID);
        if (handle == nullptr)
        {
            LOG(Error, "AmplitudeMiddleware: No event handle for trigger event ID: {0}", amTrigger->EventID);
            return false;
        }

        return true;
    }

    bool AmplitudeMiddleware::ActivateTrigger(
        AudioSystemDataID entityId, AudioSystemTriggerData* triggerData, AudioSystemEventData* eventData)
    {
        if (!m_bInitialized)
            return false;

        auto* amTrigger = static_cast<AmplitudeTriggerData*>(triggerData);
        auto* amEvent = static_cast<AmplitudeEventData*>(eventData);

        // Look up the entity in the engine.
        const Entity entity = m_pEngine->GetEntity(static_cast<AmEntityID>(entityId));
        if (!entity.Valid())
        {
            LOG(Error, "AmplitudeMiddleware: Cannot activate trigger - entity {0} not found", entityId);
            return false;
        }

        // Trigger the event on the entity.
        const EventCanceler canceler = m_pEngine->Trigger(amTrigger->EventID, entity);
        amEvent->Canceler = canceler;
        amEvent->State = PlaybackState::Playing;
        return true;
    }

    bool AmplitudeMiddleware::UnloadTrigger(
        AudioSystemDataID entityId, AudioSystemTriggerData* triggerData, AudioSystemEventData* eventData)
    {
        if (!m_bInitialized)
            return false;

        auto* amEvent = static_cast<AmplitudeEventData*>(eventData);

        // Clear the event reference; the event handle itself is owned by the engine.
        amEvent->EventID = 0;
        amEvent->Canceler = EventCanceler();
        amEvent->State = PlaybackState::Idle;
        return true;
    }

    bool AmplitudeMiddleware::StopEvent(AudioSystemDataID entityId, AudioSystemEventData* eventData)
    {
        if (!m_bInitialized)
            return false;

        auto* amEvent = static_cast<AmplitudeEventData*>(eventData);
        if (amEvent->Canceler.Valid())
        {
            amEvent->Canceler.Cancel();
            amEvent->State = PlaybackState::Stopping;
        }

        return true;
    }

    bool AmplitudeMiddleware::StopAllEvents(AudioSystemDataID entityId)
    {
        if (!m_bInitialized)
            return false;

        const Entity entity = m_pEngine->GetEntity(static_cast<AmEntityID>(entityId));
        if (!entity.Valid())
            return false;

        // Cancel all events associated with this entity.
        m_pEngine->CancelEvents(entity);
        return true;
    }

    // ========================================================================
    //  RTPC (2)
    // ========================================================================

    bool AmplitudeMiddleware::SetRtpc(AudioSystemDataID entityId, AudioSystemRtpcData* data, float value)
    {
        if (!m_bInitialized)
            return false;

        auto* amRtpc = static_cast<AmplitudeRtpcData*>(data);
        const RtpcHandle handle = m_pEngine->GetRtpcHandle(amRtpc->RtpcID);
        if (handle == nullptr)
        {
            LOG(Error, "AmplitudeMiddleware: No RTPC handle for ID: {0}", amRtpc->RtpcID);
            return false;
        }

        handle->SetValue(static_cast<AmReal64>(value));
        return true;
    }

    bool AmplitudeMiddleware::ResetRtpc(AudioSystemDataID entityId, AudioSystemRtpcData* data)
    {
        if (!m_bInitialized)
            return false;

        auto* amRtpc = static_cast<AmplitudeRtpcData*>(data);
        const RtpcHandle handle = m_pEngine->GetRtpcHandle(amRtpc->RtpcID);
        if (handle == nullptr)
        {
            LOG(Error, "AmplitudeMiddleware: No RTPC handle for ID: {0}", amRtpc->RtpcID);
            return false;
        }

        handle->Reset();
        return true;
    }

    // ========================================================================
    //  Switch (1)
    // ========================================================================

    bool AmplitudeMiddleware::SetSwitchState(AudioSystemDataID entityId, AudioSystemSwitchStateData* data)
    {
        if (!m_bInitialized)
            return false;

        auto* amSwitch = static_cast<AmplitudeSwitchStateData*>(data);
        m_pEngine->SetSwitchState(amSwitch->SwitchID, amSwitch->StateID);
        return true;
    }

    // ========================================================================
    //  Obstruction / Occlusion (1)
    // ========================================================================

    bool AmplitudeMiddleware::SetObstructionAndOcclusion(
        AudioSystemDataID entityId, AudioSystemEntityData* entityData, float obstruction, float occlusion)
    {
        if (!m_bInitialized)
            return false;

        auto* amEntity = static_cast<AmplitudeEntityData*>(entityData);
        const Entity entity = m_pEngine->GetEntity(amEntity->EntityID);
        if (!entity.Valid())
            return false;

        entity.SetObstruction(static_cast<AmReal32>(obstruction));
        entity.SetOcclusion(static_cast<AmReal32>(occlusion));
        return true;
    }

    // ========================================================================
    //  Environment (1)
    // ========================================================================

    bool AmplitudeMiddleware::SetEnvironmentAmount(
        AudioSystemDataID entityId, AudioSystemEnvironmentData* envData, float amount)
    {
        if (!m_bInitialized)
            return false;

        auto* amEnv = static_cast<AmplitudeEnvironmentData*>(envData);
        const Entity entity = m_pEngine->GetEntity(static_cast<AmEntityID>(entityId));
        if (!entity.Valid())
            return false;

        entity.SetEnvironmentFactor(amEnv->EnvironmentID, static_cast<AmReal32>(amount));
        return true;
    }

    // ========================================================================
    //  Bank (3)
    // ========================================================================

    bool AmplitudeMiddleware::LoadBank(AudioSystemBankData* data)
    {
        if (!m_bInitialized)
            return false;

        auto* amBank = static_cast<AmplitudeBankData*>(data);
        const StringAnsi filenameAnsi(amBank->BankFilename.Get(), amBank->BankFilename.Length());
        const AmOsString filenameOs = AM_STRING_TO_OS_STRING(filenameAnsi.Get());

        AmBankID outID = 0;
        if (!m_pEngine->LoadSoundBank(filenameOs, outID))
        {
            LOG(Error, "AmplitudeMiddleware: Failed to load bank: {0}", amBank->BankFilename);
            return false;
        }

        amBank->BankID = outID;

        // Finalize loading of sound files referenced by the bank.
        m_pEngine->StartLoadSoundFiles();
        if (!PollWithTimeout([this]() { return m_pEngine->TryFinalizeLoadSoundFiles(); }))
        {
            LOG(Warning, "AmplitudeMiddleware: Timed out loading sound files for bank: {0}", amBank->BankFilename);
        }

        return true;
    }

    bool AmplitudeMiddleware::UnloadBank(AudioSystemBankData* data)
    {
        if (!m_bInitialized)
            return false;

        auto* amBank = static_cast<AmplitudeBankData*>(data);
        if (amBank->BankID == 0)
            return false;

        m_pEngine->UnloadSoundBank(amBank->BankID);
        amBank->BankID = 0;
        return true;
    }

    void AmplitudeMiddleware::DestroyBank(AudioSystemBankData* data)
    {
        delete static_cast<AmplitudeBankData*>(data);
    }

    // ========================================================================
    //  Control Data Destroy (4)
    // ========================================================================

    void AmplitudeMiddleware::DestroyTriggerData(AudioSystemTriggerData* data)
    {
        delete static_cast<AmplitudeTriggerData*>(data);
    }

    void AmplitudeMiddleware::DestroyRtpcData(AudioSystemRtpcData* data)
    {
        delete static_cast<AmplitudeRtpcData*>(data);
    }

    void AmplitudeMiddleware::DestroySwitchStateData(AudioSystemSwitchStateData* data)
    {
        delete static_cast<AmplitudeSwitchStateData*>(data);
    }

    void AmplitudeMiddleware::DestroyEnvironmentData(AudioSystemEnvironmentData* data)
    {
        delete static_cast<AmplitudeEnvironmentData*>(data);
    }

    // ========================================================================
    //  Global (8)
    // ========================================================================

    void AmplitudeMiddleware::SetLanguage(StringView language)
    {
        // Amplitude does not currently expose a runtime language-switch API.
        // Log the request for diagnostics.
        LOG(Info, "AmplitudeMiddleware: Language set to: {0}", String(language));
    }

    void AmplitudeMiddleware::OnMasterGainChange(float gain)
    {
        if (!m_bInitialized)
            return;

        m_pEngine->SetMasterGain(static_cast<AmReal32>(gain));
    }

    void AmplitudeMiddleware::OnMuteChange(bool muted)
    {
        if (!m_bInitialized)
            return;

        m_pEngine->SetMute(muted);
    }

    void AmplitudeMiddleware::OnLoseFocus()
    {
        if (!m_bInitialized)
            return;

        m_pEngine->Pause(true);
    }

    void AmplitudeMiddleware::OnGainFocus()
    {
        if (!m_bInitialized)
            return;

        m_pEngine->Pause(false);
    }

    StringView AmplitudeMiddleware::GetMiddlewareName() const
    {
        return StringView(TEXT("Amplitude"));
    }

    float AmplitudeMiddleware::GetMasterGain() const
    {
        if (!m_bInitialized)
            return 1.0f;

        return static_cast<float>(m_pEngine->GetMasterGain());
    }

    bool AmplitudeMiddleware::GetMute() const
    {
        if (!m_bInitialized)
            return false;

        return m_pEngine->IsMuted();
    }

} // namespace AmplitudeAudio
