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

/// \brief Editor-only utility that generates AudioControlEntry values
///        by scanning an Amplitude Audio project directory.
///
/// Usage from C# editor scripts:
/// \code
///   AmplitudeControlsGenerator.ReloadControls(
///       "/path/to/amplitude_project",
///       "Content/Sounds/Amplitude/Controls.json");
/// \endcode
API_CLASS(Static)
class AMPLITUDEAUDIO_API AmplitudeControlsGenerator
{
    DECLARE_SCRIPTING_TYPE_NO_SPAWN(AmplitudeControlsGenerator);

public:
    /// \brief Scans the Amplitude project at \p projectPath and writes
    ///        an AudioControlCollection JSON asset to \p outputAssetPath.
    ///
    /// \param projectPath     Absolute path to the Amplitude project directory
    ///                        (the folder containing the .amproject file).
    /// \param outputAssetPath Project-relative path for the output JSON asset
    ///                        (e.g. "Content/Sounds/Amplitude/Controls.json").
    /// \return True if the asset was written successfully.
    API_FUNCTION()
    static bool ReloadControls(const String& projectPath, const String& outputAssetPath);

    /// \brief Returns the resolved project sources path from the configuration asset.
    ///
    /// Reads the Amplitude configuration asset and computes
    /// {kAssetBasePath}/{ProjectSourcesPath}. Returns an empty string if the
    /// config asset cannot be loaded or ProjectSourcesPath is not set.
    API_FUNCTION()
    static String GetProjectSourcesPath();

    /// \brief Scans the Amplitude project at \p projectPath and returns
    ///        all discovered controls.
    ///
    /// \param projectPath  Absolute path to the Amplitude project directory.
    /// \return Array of discovered AudioControlEntry values.
    static Array<AudioControlEntry> GenerateFromProject(const String& projectPath);

private:
    static void ScanEvents(const String& dir, Array<AudioControlEntry>& out);
    static void ScanSwitches(const String& dir, Array<AudioControlEntry>& out);
    static void ScanRtpcs(const String& dir, Array<AudioControlEntry>& out);
    static void ScanSoundBanks(const String& dir, Array<AudioControlEntry>& out);
    static void ScanEffects(const String& dir, Array<AudioControlEntry>& out);
};

#endif // USE_EDITOR
