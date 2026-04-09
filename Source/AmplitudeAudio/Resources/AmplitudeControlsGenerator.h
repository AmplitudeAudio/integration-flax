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
//  AmplitudeControlsGenerator — Editor-only audio controls discovery
//
//  Scans an Amplitude Audio project's source JSON directories (events,
//  switches, rtpc, soundbanks, effects) and produces AudioControlEntry
//  values that can be written to an AudioControlCollection JSON asset.
//
//  This class is only available in editor builds (USE_EDITOR).
// ============================================================================

#if USE_EDITOR

#include <Engine/Core/Collections/Array.h>
#include <Engine/Core/Types/String.h>
#include <Engine/Scripting/ScriptingObject.h>

#include "AudioControlCollectionResource.h"

/// <summary>
/// Editor-only utility that generates AudioControlEntry values
///        by scanning an Amplitude Audio project directory.
/// </summary>
/// <remarks>
/// Usage from C# editor scripts:
/// \code
///   AmplitudeControlsGenerator.ReloadControls(
///       "/path/to/amplitude_project",
///       "Content/Sounds/Amplitude/Controls.json");
/// \endcode
/// </remarks>
API_CLASS(Static)
class AMPLITUDEAUDIO_API AmplitudeControlsGenerator
{
    DECLARE_SCRIPTING_TYPE_NO_SPAWN(AmplitudeControlsGenerator);

  public:
    /// <summary>
    /// Scans the Amplitude project at <paramref name="projectPath"/> and writes
    ///        an AudioControlCollection JSON asset to <paramref name="outputAssetPath"/>.
    /// </summary>
    /// <param name="projectPath">
    /// Absolute path to the Amplitude project directory
    ///                        (the folder containing the .amproject file).
    /// </param>
    /// <param name="outputAssetPath">
    /// Project-relative path for the output JSON asset
    ///                        (e.g. "Content/Sounds/Amplitude/Controls.json").
    /// </param>
    /// <returns>True if the asset was written successfully.</returns>
    API_FUNCTION()
    static bool ReloadControls(const String& projectPath, const String& outputAssetPath);

    /// <summary>Returns the resolved project sources path from the configuration asset.</summary>
    /// <remarks>
    /// Reads the Amplitude configuration asset and computes
    /// {kAssetBasePath}/{ProjectSourcesPath}. Returns an empty string if the
    /// config asset cannot be loaded or ProjectSourcesPath is not set.
    /// </remarks>
    API_FUNCTION()
    static String GetProjectSourcesPath();

    /// <summary>
    /// Scans the Amplitude project at <paramref name="projectPath"/> and returns
    ///        all discovered controls.
    /// </summary>
    /// <param name="projectPath">Absolute path to the Amplitude project directory.</param>
    /// <returns>Array of discovered AudioControlEntry values.</returns>
    static Array<AudioControlEntry> GenerateFromProject(const String& projectPath);

  private:
    static void ScanEvents(const String& dir, Array<AudioControlEntry>& out);
    static void ScanSwitches(const String& dir, Array<AudioControlEntry>& out);
    static void ScanRtpcs(const String& dir, Array<AudioControlEntry>& out);
    static void ScanSoundBanks(const String& dir, Array<AudioControlEntry>& out);
    static void ScanEffects(const String& dir, Array<AudioControlEntry>& out);
};

#endif // USE_EDITOR
