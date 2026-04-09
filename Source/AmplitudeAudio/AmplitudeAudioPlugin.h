// Copyright (c) 2026-present Sparky Studios. All rights reserved.
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

// ============================================================================
//  AmplitudeAudioPlugin — Flax GamePlugin entry point
//
//  This is the class Flax discovers and loads when the AmplitudeAudio plugin
//  is enabled for a project. Initialize() wires the AmplitudeMiddleware into
//  the AudioSystem; Deinitialize() tears it back down.
// ============================================================================

#include <Engine/Scripting/Plugins/GamePlugin.h>

namespace AmplitudeAudio
{
    class AmplitudeMiddleware;
}

/// \brief Main entry point for the Amplitude Audio Flax plugin.
///
/// Flax calls Initialize() after all modules are loaded and Deinitialize()
/// before they are unloaded. The plugin owns the AmplitudeMiddleware instance
/// and is responsible for registering and unregistering it with the AudioSystem.
///
/// Configuration is loaded from a well-known JSON asset at
/// Content/Sounds/Amplitude/Config.json. This asset must contain per-platform
/// profiles with InitSoundBank and EngineConfigFile entries.
API_CLASS()
class AMPLITUDEAUDIO_API AmplitudeAudioPlugin : public GamePlugin
{
    DECLARE_SCRIPTING_TYPE(AmplitudeAudioPlugin);

  public:
    // ========================================================================
    //  GamePlugin overrides
    // ========================================================================

    /// \brief Creates the AmplitudeMiddleware instance, loads configuration,
    ///        and starts the AudioSystem.
    void Initialize() override;

    /// \brief Unregisters and destroys the AmplitudeMiddleware instance.
    void Deinitialize() override;

  private:
    /// Owned middleware instance. Created in Initialize(), destroyed in Deinitialize().
    AmplitudeAudio::AmplitudeMiddleware* _middleware = nullptr;
};
