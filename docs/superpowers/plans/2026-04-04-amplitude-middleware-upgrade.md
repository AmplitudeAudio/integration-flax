# Amplitude Middleware Upgrade Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Update the Flax Amplitude integration so its middleware lifecycle, bank handling, listener removal, and plugin bootstrap match the current SDK behavior and the O3DE reference safety patterns.

**Architecture:** Keep the existing `AmplitudeMiddleware` / `AmplitudeAudioPlugin` split, but make the middleware startup path transactional and the shutdown path symmetrical. Fix object lifetime hazards in-place instead of restructuring the plugin, and validate with repo-local static checks plus Flax headless project actions.

**Tech Stack:** C++17, Flax Engine plugin modules, Amplitude Audio SDK, Flax Editor headless CLI

---

### Task 1: Add explicit runtime cleanup helpers to `AmplitudeMiddleware`

**Files:**
- Modify: `Source/AmplitudeAudio/AmplitudeMiddleware.h:143-154`
- Modify: `Source/AmplitudeAudio/AmplitudeMiddleware.cpp:36-52`
- Modify: `Source/AmplitudeAudio/AmplitudeMiddleware.cpp:223-252`
- Test: `n/a (repo has no automated test harness; use static validation and headless editor verification later in Task 4)`

- [ ] **Step 1: Add private helper declarations to the middleware header**

```cpp
      private:
        void CleanupRuntimeState(bool closeFileSystem, bool unregisterExtensions);
        void ResetRuntimeState();

        /// Amplitude Engine singleton (obtained via Engine::GetInstance()).
        SparkyStudios::Audio::Amplitude::Engine* m_pEngine;
```

- [ ] **Step 2: Add the helper implementations near the anonymous-namespace utilities**

```cpp
    void AmplitudeMiddleware::CleanupRuntimeState(bool closeFileSystem, bool unregisterExtensions)
    {
        if (m_pEngine != nullptr)
        {
            if (m_pEngine->IsInitialized())
                m_pEngine->Deinitialize();

            if (closeFileSystem)
            {
                m_pEngine->StartCloseFileSystem();
                if (!PollWithTimeout([this]() { return m_pEngine->TryFinalizeCloseFileSystem(); }))
                    LOG(Warning, "AmplitudeMiddleware: Timed out closing file system");
            }
        }

        if (unregisterExtensions)
            AmEngine::UnregisterDefaultExtensions();

        AmEngine::DestroyInstance();
        m_pFileSystem.reset();
        m_pEngine = nullptr;
    }

    void AmplitudeMiddleware::ResetRuntimeState()
    {
        m_pEngine = nullptr;
        m_pFileSystem.reset();
        m_bInitialized = false;
    }
```

- [ ] **Step 3: Wire `Release()` to use the new reset helper**

```cpp
    void AmplitudeMiddleware::Release()
    {
        ResetRuntimeState();
        LOG(Info, "AmplitudeMiddleware: Released");
    }
```

- [ ] **Step 4: Run a whitespace and conflict sanity check**

Run:

```bash
git diff --check
```

Expected: no output

- [ ] **Step 5: Commit**

```bash
git add Source/AmplitudeAudio/AmplitudeMiddleware.h Source/AmplitudeAudio/AmplitudeMiddleware.cpp
git commit -m "refactor: centralize amplitude runtime cleanup"
```

### Task 2: Make startup and shutdown transactional

**Files:**
- Modify: `Source/AmplitudeAudio/AmplitudeMiddleware.cpp:130-246`
- Test: `n/a (validated with static checks and headless editor commands in Task 4)`

- [ ] **Step 1: Rewrite `Startup()` to track which initialization stages completed**

```cpp
    bool AmplitudeMiddleware::Startup()
    {
        if (m_bInitialized)
        {
            LOG(Warning, "AmplitudeMiddleware: Already initialized");
            return true;
        }

        const String platform = AmplitudeAssetProfiles::GetCurrentPlatform();
        const AmplitudeConfiguration* config = m_profiles.Get(platform);
        if (config == nullptr)
        {
            LOG(Error, "AmplitudeMiddleware: No configuration profile for platform: {0}. Call LoadConfiguration first.", platform);
            return false;
        }

        bool extensionsRegistered = false;
        bool fileSystemOpened = false;

        auto failStartup = [this, &extensionsRegistered, &fileSystemOpened](const String& message)
        {
            LOG(Error, "{0}", message);
            CleanupRuntimeState(fileSystemOpened, extensionsRegistered);
            ResetRuntimeState();
            return false;
        };

        static FlaxLogger sFlaxLogger;
        Logger::SetLogger(&sFlaxLogger);

        AmEngine::RegisterDefaultExtensions();
        extensionsRegistered = true;

        m_pFileSystem = std::make_shared<AmplitudeFileSystem>();
        const String& buildPath = m_profiles.GetProjectBuildPath();
        String basePath = String(kAssetBasePath);
        if (!buildPath.IsEmpty())
            basePath += buildPath;

        const StringAnsi basePathAnsi(basePath.Get(), basePath.Length());
        m_pFileSystem->SetBasePath(AM_STRING_TO_OS_STRING(basePathAnsi.Get()));

        m_pEngine = AmEngine::GetInstance();
        if (m_pEngine == nullptr)
            return failStartup(TEXT("AmplitudeMiddleware: Failed to get Engine instance"));

        m_pEngine->SetFileSystem(m_pFileSystem);
        m_pEngine->StartOpenFileSystem();
        if (!PollWithTimeout([this]() { return m_pEngine->TryFinalizeOpenFileSystem(); }))
            return failStartup(TEXT("AmplitudeMiddleware: Timed out opening file system"));

        fileSystemOpened = true;

        const StringAnsi configFileAnsi(config->EngineConfigFile.Get(), config->EngineConfigFile.Length());
        if (!m_pEngine->Initialize(AM_STRING_TO_OS_STRING(configFileAnsi.Get())))
            return failStartup(String::Format(TEXT("AmplitudeMiddleware: Engine initialization failed with config: {0}"), config->EngineConfigFile));

        const StringAnsi initBankAnsi(config->InitSoundBank.Get(), config->InitSoundBank.Length());
        if (!m_pEngine->LoadSoundBank(AM_STRING_TO_OS_STRING(initBankAnsi.Get())))
            return failStartup(String::Format(TEXT("AmplitudeMiddleware: Failed to load init sound bank: {0}"), config->InitSoundBank));

        m_pEngine->StartLoadSoundFiles();
        if (!PollWithTimeout([this]() { return m_pEngine->TryFinalizeLoadSoundFiles(); }))
            return failStartup(TEXT("AmplitudeMiddleware: Timed out loading init sound bank files"));

        m_bInitialized = true;
        LOG(Info, "AmplitudeMiddleware: Startup complete");
        return true;
    }
```

- [ ] **Step 2: Mirror O3DE shutdown behavior by unloading banks before deinit and using the helper**

```cpp
    void AmplitudeMiddleware::Shutdown()
    {
        if (!m_bInitialized && m_pEngine == nullptr)
        {
            ResetRuntimeState();
            return;
        }

        m_bInitialized = false;

        if (m_pEngine != nullptr && m_pEngine->IsInitialized())
            m_pEngine->UnloadSoundBanks();

        CleanupRuntimeState(m_pEngine != nullptr, true);
        ResetRuntimeState();

        LOG(Info, "AmplitudeMiddleware: Shutdown complete");
    }
```

- [ ] **Step 3: Keep destructor behavior idempotent**

```cpp
    AmplitudeMiddleware::~AmplitudeMiddleware()
    {
        if (m_bInitialized || m_pEngine != nullptr || m_pFileSystem != nullptr)
            Shutdown();

        Release();
    }
```

- [ ] **Step 4: Run a formatting-neutral diff check**

Run:

```bash
git diff --check
```

Expected: no output

- [ ] **Step 5: Commit**

```bash
git add Source/AmplitudeAudio/AmplitudeMiddleware.cpp
git commit -m "fix: make amplitude startup transactional"
```

### Task 3: Fix listener, entity, and bank lifetime hazards

**Files:**
- Modify: `Source/AmplitudeAudio/AmplitudeMiddleware.cpp:399-406`
- Modify: `Source/AmplitudeAudio/AmplitudeMiddleware.cpp:488-495`
- Modify: `Source/AmplitudeAudio/AmplitudeMiddleware.cpp:732-771`
- Test: `n/a (validated with headless editor build plus manual runtime checklist in Task 4)`

- [ ] **Step 1: Cancel entity events before removing the entity**

```cpp
    bool AmplitudeMiddleware::RemoveEntity(AudioSystemDataID id, AudioSystemEntityData* data)
    {
        if (!m_bInitialized)
            return false;

        auto* entityData = static_cast<AmplitudeEntityData*>(data);
        const Entity entity = m_pEngine->GetEntity(entityData->EntityID);
        if (entity.Valid())
            m_pEngine->CancelEvents(entity);

        m_pEngine->RemoveEntity(entityData->EntityID);
        return true;
    }
```

- [ ] **Step 2: Clear the default listener pointer before removing the default listener**

```cpp
    bool AmplitudeMiddleware::RemoveListener(AudioSystemDataID id, AudioSystemListenerData* data)
    {
        if (!m_bInitialized)
            return false;

        auto* listenerData = static_cast<AmplitudeListenerData*>(data);
        if (listenerData->IsDefault)
            m_pEngine->SetDefaultListener(static_cast<const Listener*>(nullptr));

        m_pEngine->RemoveListener(listenerData->ListenerID);
        return true;
    }
```

- [ ] **Step 3: Unload banks by filename so duplicate loads remain unloadable**

```cpp
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

        m_pEngine->StartLoadSoundFiles();
        if (!PollWithTimeout([this]() { return m_pEngine->TryFinalizeLoadSoundFiles(); }))
            LOG(Warning, "AmplitudeMiddleware: Timed out loading sound files for bank: {0}", amBank->BankFilename);

        return true;
    }

    bool AmplitudeMiddleware::UnloadBank(AudioSystemBankData* data)
    {
        if (!m_bInitialized)
            return false;

        auto* amBank = static_cast<AmplitudeBankData*>(data);
        if (amBank->BankFilename.IsEmpty())
            return false;

        const StringAnsi filenameAnsi(amBank->BankFilename.Get(), amBank->BankFilename.Length());
        m_pEngine->UnloadSoundBank(AM_STRING_TO_OS_STRING(filenameAnsi.Get()));
        amBank->BankID = 0;
        return true;
    }
```

- [ ] **Step 4: Run a targeted review diff before moving on**

Run:

```bash
git diff -- Source/AmplitudeAudio/AmplitudeMiddleware.cpp
```

Expected: only entity removal, listener removal, and bank load/unload hunks changed

- [ ] **Step 5: Commit**

```bash
git add Source/AmplitudeAudio/AmplitudeMiddleware.cpp
git commit -m "fix: harden amplitude object lifetimes"
```

### Task 4: Fail fast in plugin bootstrap and validate the upgrade

**Files:**
- Modify: `Source/AmplitudeAudio/AmplitudeAudioPlugin.cpp:44-77`
- Test: `AmplitudeAudio.flaxproj`

- [ ] **Step 1: Make plugin initialization stop immediately when configuration loading fails**

```cpp
void AmplitudeAudioPlugin::Initialize()
{
    using namespace SparkyStudios::Audio::Amplitude;

    GamePlugin::Initialize();

    AudioSystem* audioSystem = AudioSystem::Get();
    if (audioSystem == nullptr)
    {
        LOG(Error, "[AmplitudeAudioPlugin] Initialize: AudioSystem is not available. Ensure the AudioSystem plugin is loaded before AmplitudeAudio.");
        return;
    }

    _middleware = New<AmplitudeAudio::AmplitudeMiddleware>();
    audioSystem->RegisterMiddleware(_middleware);

    MemoryManager::Initialize(std::make_unique<AmplitudeAudio::FlaxMemoryAllocator>());

    const String configPath = String(AmplitudeAudio::kAssetBasePath) + TEXT("Config.json");
    if (!audioSystem->LoadConfiguration(configPath))
    {
        LOG(Error, "[AmplitudeAudioPlugin] Failed to load middleware configuration: {0}", configPath);
        audioSystem->UnregisterMiddleware();
        Delete(_middleware);
        _middleware = nullptr;
        MemoryManager::Deinitialize();
        return;
    }

    if (!audioSystem->Startup())
    {
        LOG(Error, "[AmplitudeAudioPlugin] AudioSystem startup failed. Ensure Content/Sounds/Amplitude/Config.json exists with valid platform profiles.");
        audioSystem->UnregisterMiddleware();
        Delete(_middleware);
        _middleware = nullptr;
        MemoryManager::Deinitialize();
        return;
    }

    LOG(Info, "[AmplitudeAudioPlugin] Amplitude Audio middleware registered.");
}
```

- [ ] **Step 2: Run static whitespace validation**

Run:

```bash
git diff --check
```

Expected: no output

- [ ] **Step 3: Regenerate Flax project files to confirm the project still loads**

Run:

```bash
FlaxEditor -project "/Users/na2axl/Projects/AmplitudeAudio/integration-flax/AmplitudeAudio.flaxproj" -headless -std -mute -null -genProjectFiles -exit
```

Expected: standard output shows project generation completed without C++ parse errors

- [ ] **Step 4: Run a headless build smoke test**

Run:

```bash
FlaxEditor -project "/Users/na2axl/Projects/AmplitudeAudio/integration-flax/AmplitudeAudio.flaxproj" -headless -std -mute -null -build "Development" -exit
```

Expected: build completes without errors about `AmplitudeMiddleware.cpp`, `AmplitudeAudioPlugin.cpp`, or missing Amplitude symbols

- [ ] **Step 5: Perform the runtime checklist inside Flax Editor**

```text
1. Open the project in Flax Editor.
2. Confirm plugin initialization loads Content/Sounds/Amplitude/Config.json without startup errors.
3. Trigger one registered event on a positioned entity and then delete that entity.
4. Add and remove the default listener while audio is active.
5. Load and unload the same sound bank twice and confirm there is no stale-bank error.
```

Expected: no crash, no stale default-listener behavior, and no bank unload failure after repeated loads

- [ ] **Step 6: Commit**

```bash
git add Source/AmplitudeAudio/AmplitudeAudioPlugin.cpp
git add Source/AmplitudeAudio/AmplitudeMiddleware.cpp Source/AmplitudeAudio/AmplitudeMiddleware.h
git commit -m "fix: align amplitude startup with sdk lifecycle"
```

### Scope Notes

- [ ] Do not change `AudioControlsComponent.cpp` bank registration in this pass; keep the upgrade focused on lifecycle correctness first.
- [ ] Do not expand RTPC or switch behavior in this pass; document those as a later compatibility pass if needed.
- [ ] Do not add new third-party dependencies, test frameworks, or file-level refactors unrelated to middleware safety.

### Self-Review

- [ ] Coverage check passes: the plan covers startup rollback, shutdown symmetry, bank unload safety, listener removal safety, entity event cancellation, and plugin fail-fast bootstrap.
- [ ] Placeholder scan passes: every code-changing step includes concrete code, and every validation step includes a concrete command or checklist.
- [ ] Type consistency passes: all symbols referenced in later tasks (`CleanupRuntimeState`, `ResetRuntimeState`, `LoadConfiguration`, `UnloadSoundBank`) are defined before use in the plan.
