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

// ============================================================================
//  AudioControls — Flax component for loading AudioControlCollections
//
//  Attaches an AudioControlCollection JSON asset to an actor. When
//  AutoLoad is true the collection is registered with the AudioSystem on
//  OnEnable() and deregistered on OnDisable().
// ============================================================================

#include <AudioSystem/Core/AudioSystemData.h>
#include <Engine/Core/Collections/Array.h>
#include <Engine/Core/Types/String.h>
#include <Engine/Scripting/Script.h>

/// <summary>
/// Script component that loads and manages an AudioControlCollection
///        asset at runtime.
/// </summary>
/// <remarks>
/// Drop this script on any actor to bind a JSON audio-controls file to that
/// actor's lifetime. Set ControlsAssetPath to a project-relative path
/// (e.g. "Content/Sounds/Amplitude/MyControls.json") and leave AutoLoad
/// enabled to have controls loaded automatically when play starts.
///
/// Load-order assumption: Load() requires the AudioSystem to be initialized,
/// which happens in AmplitudeAudioPlugin::Initialize() via CommitMiddleware().
/// Because Flax loads plugins before script OnEnable runs, that ordering is
/// guaranteed in normal play. If Load() runs before the plugin Initialize
/// (e.g. an out-of-tree controls script with no AmplitudeAudioPlugin
/// dependency), Load() logs an Error and returns without registering anything.
/// </remarks>
API_CLASS(Attributes = "FlaxEngine.Category(\"Audio System/Amplitude\")")
class AMPLITUDEAUDIO_API AudioControls : public Script
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCRIPTING_TYPE(AudioControls);

  public:
    // ========================================================================
    //  Serialised properties
    // ========================================================================

    /// <summary>Project-relative path to the AudioControlCollection JSON asset.</summary>
    API_FIELD(Attributes = "EditorOrder(0)")
    String ControlsAssetPath;

    /// <summary>When true, Load() is called automatically during OnBeginPlay().</summary>
    API_FIELD(Attributes = "EditorOrder(1)")
    bool AutoLoad = true;

    // ========================================================================
    //  Public API
    // ========================================================================

    /// <summary>
    /// Loads the AudioControlCollection and registers all controls with
    ///        the AudioSystem.
    /// </summary>
    /// <remarks>
    /// Reads the JSON asset at ControlsAssetPath, parses it into an
    /// AudioControlCollection, and submits each control to the AudioSystem for
    /// registration. Has no effect if the collection is already loaded or if
    /// ControlsAssetPath is empty.
    /// </remarks>
    API_FUNCTION()
    void Load();

    /// <summary>Unregisters all controls that were previously loaded by Load().</summary>
    /// <remarks>Has no effect if the collection is not currently loaded.</remarks>
    API_FUNCTION()
    void Unload();

    /// <returns>True if the controls have been loaded and registered.</returns>
    API_FUNCTION()
    bool IsLoaded() const;

    // ========================================================================
    //  Script lifecycle overrides
    // ========================================================================

    void OnEnable() override;
    void OnDisable() override;

  private:
    bool _loaded = false;

    /// <summary>
    /// IDs of registered controls, grouped by type. Unload() unregisters in
    /// dependency order: Trigger -> Rtpc -> Environment -> SwitchState -> Switch -> Bank.
    /// </summary>
    Array<AudioSystemDataID> _registeredBankIds;
    Array<AudioSystemDataID> _registeredTriggerIds;
    Array<AudioSystemDataID> _registeredRtpcIds;
    Array<AudioSystemDataID> _registeredSwitchIds;
    Array<AudioSystemDataID> _registeredSwitchStateIds;
    Array<AudioSystemDataID> _registeredEnvironmentIds;
};
