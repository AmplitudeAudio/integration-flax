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

#include <atomic>

#include <Engine/Scripting/Plugins/GamePlugin.h>

namespace AmplitudeAudio
{
    class AmplitudeMiddleware;
}

/// <summary>
/// Main entry point for the Amplitude Audio Flax plugin.
/// </summary>
/// <remarks>
/// Flax calls Initialize() after all modules are loaded and Deinitialize()
/// before they are unloaded. The plugin owns the AmplitudeMiddleware instance
/// and is responsible for registering and unregistering it with the AudioSystem.
///
/// Configuration is loaded from the native Flax game settings
/// (Amplitude Configuration, edited via Settings > Game Settings). It must
/// contain per-platform profiles with InitSoundBank and EngineConfigFile entries.
/// </remarks>
API_CLASS()
class AMPLITUDEAUDIO_API AmplitudeAudioPlugin : public GamePlugin
{
    DECLARE_SCRIPTING_TYPE(AmplitudeAudioPlugin);

    void Initialize() override;
    void Deinitialize() override;

private:
    /// <summary>
    /// Guards against double initialization when multiple projects reference this plugin.
    /// </summary>
    static bool _initialized;

    /// <summary>Owned middleware instance. Created in Initialize(), destroyed in Deinitialize().</summary>
    AmplitudeAudio::AmplitudeMiddleware* _middleware = nullptr;
};