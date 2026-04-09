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

// clang-formatt off
#include "Core/Cpp20Compat.h"
// clang-format on

#include <AudioSystem/Core/AudioSystem.h>
#include <Engine/Core/Log.h>
#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>

#include "AmplitudeMiddleware.h"
#include "Core/AmplitudeHelpers.h"
#include "Core/Common.h"

// ============================================================================
//  AmplitudeAudioPlugin — Constructor
// ============================================================================

AmplitudeAudioPlugin::AmplitudeAudioPlugin(const SpawnParams& params)
    : GamePlugin(params)
{
    _description.Name          = TEXT("Amplitude Audio SDK");
    _description.Category      = TEXT("Audio");
    _description.Author        = TEXT("Sparky Studios");
    _description.Description   = TEXT("Amplitude Audio SDK integration for Flax Engine.");
    _description.RepositoryUrl = TEXT("https://github.com/AmplitudeAudio/integration-flax");
}

// ============================================================================
//  AmplitudeAudioPlugin — Initialize
// ============================================================================

void AmplitudeAudioPlugin::Initialize()
{
    using namespace SparkyStudios::Audio::Amplitude;

    GamePlugin::Initialize();

    AudioSystem* audioSystem = AudioSystem::Get();
    if (audioSystem == nullptr)
    {
        LOG(Error, "[AmplitudeAudioPlugin] Initialize: AudioSystem is not available. "
                   "Ensure the AudioSystem plugin is loaded before AmplitudeAudio.");
        return;
    }

    _middleware = New<AmplitudeAudio::AmplitudeMiddleware>();
    audioSystem->RegisterMiddleware(_middleware);

    // Initialize the Amplitude memory manager with Flax-backed allocator.
    MemoryManager::Initialize(std::make_unique<AmplitudeAudio::FlaxMemoryAllocator>());

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

    // Load middleware configuration
    const String configPath = String(AmplitudeAudio::kAssetBasePath) + TEXT("Config.json");
    if (!_middleware->LoadConfiguration(configPath))
    {
        LOG(Error, "[AmplitudeAudioPlugin] Failed to load middleware configuration: {0}", configPath);
        rollbackInitialization();
        return;
    }

    // Start the AudioSystem now that the middleware is registered and configured
    if (!audioSystem->Startup())
    {
        LOG(Error, "[AmplitudeAudioPlugin] AudioSystem startup failed. "
                   "Ensure Content/Sounds/Amplitude/Config.json exists with valid platform profiles.");
        rollbackInitialization();
        return;
    }

    LOG(Info, "[AmplitudeAudioPlugin] Amplitude Audio middleware registered.");
}

// ============================================================================
//  AmplitudeAudioPlugin — Deinitialize
// ============================================================================

void AmplitudeAudioPlugin::Deinitialize()
{
    using namespace SparkyStudios::Audio::Amplitude;

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

    LOG(Info, "[AmplitudeAudioPlugin] Amplitude Audio middleware unregistered.");

    GamePlugin::Deinitialize();
}
