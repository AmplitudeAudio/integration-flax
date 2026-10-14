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

#include "AmplitudeAudioPlugin.h"

// clang-format off
#include <AmplitudeAudio/Core/Cpp20Compat.h>
// clang-format on

#include <Engine/Core/Log.h>

#include <AudioSystem/Core/AudioSystem.h>

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>

#include <AmplitudeAudio/AmplitudeMiddleware.h>
#include <AmplitudeAudio/Core/Common.h>
#include <AmplitudeAudio/Core/FlaxMemoryAllocator.h>

bool AmplitudeAudioPlugin::_initialized = false;

AmplitudeAudioPlugin::AmplitudeAudioPlugin(const SpawnParams& params)
    : GamePlugin(params)
{
    _description.Name          = TEXT("Amplitude Audio SDK");
    _description.Category      = TEXT("Audio");
    _description.Author        = TEXT("Sparky Studios");
    _description.Description   = TEXT("Amplitude Audio SDK integration for Flax Engine.");
    _description.RepositoryUrl = TEXT("https://github.com/AmplitudeAudio/integration-flax");
}

void AmplitudeAudioPlugin::Initialize()
{
    using namespace SparkyStudios::Audio::Amplitude;

    GamePlugin::Initialize();

    if (_initialized)
        return;

    AudioSystem* audioSystem = AudioSystem::Get();
    if (audioSystem == nullptr)
    {
        LOG(Error, "[AmplitudeAudioPlugin] Initialize: AudioSystem is not available. "
            "Ensure the AudioSystem plugin is loaded before AmplitudeAudio.");
        return;
    }

    // Initialize the Amplitude memory manager with Flax-backed allocator BEFORE
    // any code path that may touch Amplitude allocators (including
    // New<AmplitudeMiddleware>() which constructs SDK-allocated members).
    MemoryManager::Initialize(std::make_unique<AmplitudeAudio::FlaxMemoryAllocator>());

    _middleware = New<AmplitudeAudio::AmplitudeMiddleware>();
    audioSystem->RegisterMiddleware(_middleware);

    const auto rollbackInitialization = [this, audioSystem]()
    {
        audioSystem->UnregisterMiddleware();

        if (_middleware != nullptr)
        {
            Delete(_middleware);
            _middleware = nullptr;
        }

        MemoryManager::Deinitialize();
    };

    // Load middleware configuration from the native Flax game settings
    // (AmplitudeSettings). The file path argument is unused.
    if (!_middleware->LoadConfiguration(StringView::Empty))
    {
        LOG(Error, "[AmplitudeAudioPlugin] Failed to load middleware configuration. "
            "Add the Amplitude Configuration via Settings > Game Settings.");
        rollbackInitialization();
        return;
    }

    // Signal the AudioSystem to finalize startup now that the middleware is
    // registered and configured. AudioSystemPlugin defers Startup() to this
    // explicit hook so dependents (like this plugin) can RegisterMiddleware
    // during their own Initialize().
    if (!audioSystem->CommitMiddleware())
    {
        LOG(Error, "[AmplitudeAudioPlugin] AudioSystem startup failed. "
            "Ensure the Amplitude Configuration game settings contain a profile for the current platform.");
        rollbackInitialization();
        return;
    }

    _initialized = true;
    LOG(Info, "[AmplitudeAudioPlugin] Amplitude Audio middleware registered.");
}

void AmplitudeAudioPlugin::Deinitialize()
{
    using namespace SparkyStudios::Audio::Amplitude;

    if (!_initialized)
    {
        GamePlugin::Deinitialize();
        return;
    }

    AudioSystem* audioSystem = AudioSystem::Get();
    if (audioSystem != nullptr)
    {
        audioSystem->Shutdown();
        audioSystem->UnregisterMiddleware();
    }

    if (_middleware != nullptr)
    {
        Delete(_middleware);
        _middleware = nullptr;
    }

    // Tear down the Amplitude memory manager.
    MemoryManager::Deinitialize();

    _initialized = false;
    LOG(Info, "[AmplitudeAudioPlugin] Amplitude Audio middleware unregistered.");

    GamePlugin::Deinitialize();
}