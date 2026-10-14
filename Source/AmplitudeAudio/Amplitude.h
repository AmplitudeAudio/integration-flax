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

#include <Engine/Scripting/ScriptingType.h>

#include <AmplitudeAudio/Types/AmplitudeSoundBankRef.h>
#include <AmplitudeAudio/Types/AmplitudeRefs.h>

/// <summary>Global entry point to the Amplitude engine.</summary>
API_CLASS(Static, Namespace = "AmplitudeAudio")
class AMPLITUDEAUDIO_API Amplitude
{
    DECLARE_SCRIPTING_TYPE_NO_SPAWN(Amplitude);

public:
    /// <summary>Starts the engine with the running platform's profile. The plugin calls this in cooked games, the editor when Play starts.</summary>
    API_FUNCTION()
    static bool Startup();

    /// <summary>Stops the engine and unloads every sound bank.</summary>
    API_FUNCTION()
    static void Shutdown();

    /// <summary>Whether the engine is running.</summary>
    API_PROPERTY()
    static bool IsRunning();

    /// <summary>Loads a sound bank, or adds a reference to it if it is already loaded.</summary>
    /// <param name="bank">The bank to load.</param>
    /// <param name="wait">Block until the bank's sound files are loaded.</param>
    API_FUNCTION()
    static bool LoadSoundBank(const AmplitudeSoundBankRef& bank, bool wait = true);

    /// <summary>Releases a reference taken by LoadSoundBank. The bank unloads when none remain.</summary>
    API_FUNCTION()
    static void UnloadSoundBank(const AmplitudeSoundBankRef& bank);

    /// <summary>Whether the bank and its sound files are loaded.</summary>
    API_FUNCTION()
    static bool IsSoundBankLoaded(const AmplitudeSoundBankRef& bank);
};
