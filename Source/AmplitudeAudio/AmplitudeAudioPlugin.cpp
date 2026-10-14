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

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>

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
    GamePlugin::Initialize();

    if (_initialized)
        return;

    // Must run before anything allocates through the SDK.
    SparkyStudios::Audio::Amplitude::MemoryManager::Initialize(std::make_unique<AmplitudeAudio::FlaxMemoryAllocator>());
    _initialized = true;
}

void AmplitudeAudioPlugin::Deinitialize()
{
    if (_initialized)
    {
        SparkyStudios::Audio::Amplitude::MemoryManager::Deinitialize();
        _initialized = false;
    }

    GamePlugin::Deinitialize();
}
