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

#include <AmplitudeAudio/AmplitudeSettings.h>
#include <AmplitudeAudio/Types/AmplitudeSoundBankRef.h>
#include <AmplitudeAudio/Types/AmplitudeRefs.h>
#include <AmplitudeAudio/Types/AmplitudeChannel.h>
#include <AmplitudeAudio/Types/AmplitudeEventHandle.h>

class AmplitudeEntity;

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

    /// <summary>Obstruction settings with the plugin's default values. A C# <c>new AmplitudeObstructionSettings()</c> is all zeros.</summary>
    API_FUNCTION()
    static AmplitudeObstructionSettings GetDefaultObstructionSettings();

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

    /// <summary>Plays a sound object at the default listener position.</summary>
    API_FUNCTION()
    static AmplitudeChannel* Play(const AmplitudeSoundObjectRef& sound);

    /// <summary>Plays a sound object on an entity.</summary>
    API_FUNCTION()
    static AmplitudeChannel* Play(const AmplitudeSoundObjectRef& sound, AmplitudeEntity* entity);

    /// <summary>Plays a sound object at a world position.</summary>
    API_FUNCTION()
    static AmplitudeChannel* PlayAt(const AmplitudeSoundObjectRef& sound, const Vector3& position);

    /// <summary>Triggers an event on an entity.</summary>
    API_FUNCTION()
    static AmplitudeEventHandle* TriggerEvent(const AmplitudeEventRef& ev, AmplitudeEntity* entity);

    /// <summary>Cancels all events on an entity.</summary>
    API_FUNCTION()
    static void CancelEvents(AmplitudeEntity* entity);

    /// <summary>Cancels all events globally.</summary>
    API_FUNCTION()
    static void CancelAllEvents();

    /// <summary>Sets an RTPC value.</summary>
    API_FUNCTION()
    static void SetRtpcValue(const AmplitudeRtpcRef& rtpc, double value);

    /// <summary>Gets an RTPC value.</summary>
    API_FUNCTION()
    static double GetRtpcValue(const AmplitudeRtpcRef& rtpc);

    /// <summary>Resets an RTPC to its default value.</summary>
    API_FUNCTION()
    static void ResetRtpcValue(const AmplitudeRtpcRef& rtpc);

    /// <summary>Sets a switch state.</summary>
    API_FUNCTION()
    static void SetSwitchState(const AmplitudeSwitchRef& switchRef, const AmplitudeSwitchStateRef& state);

    /// <summary>Gets the current switch state.</summary>
    API_FUNCTION()
    static AmplitudeSwitchStateRef GetSwitchState(const AmplitudeSwitchRef& switchRef);

    /// <summary>Sets the master gain (0..1).</summary>
    API_FUNCTION()
    static void SetMasterGain(float value);

    /// <summary>Gets the master gain.</summary>
    API_PROPERTY()
    static float GetMasterGain();

    /// <summary>Sets whether the engine is muted.</summary>
    API_FUNCTION()
    static void SetIsMuted(bool value);

    /// <summary>Gets whether the engine is muted.</summary>
    API_PROPERTY()
    static bool GetIsMuted();

    /// <summary>Sets whether the engine is paused.</summary>
    API_FUNCTION()
    static void SetIsPaused(bool value);

    /// <summary>Gets whether the engine is paused.</summary>
    API_PROPERTY()
    static bool GetIsPaused();

    /// <summary>Stops all playback.</summary>
    API_FUNCTION()
    static void StopAll();
};
