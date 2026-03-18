#pragma once

// ============================================================================
//  AudioControlsComponent — Flax component for loading AudioControlCollections
//
//  Attaches an AudioControlCollection JSON asset to an actor. When
//  AutoLoad is true the collection is registered with the AudioSystem on
//  OnBeginPlay() and deregistered on OnEndPlay().
// ============================================================================

#include <Engine/Core/Types/String.h>

#include "AmplitudeComponent.h"

/// \brief Script component that loads and manages an AudioControlCollection
///        asset at runtime.
///
/// Drop this script on any actor to bind a JSON audio-controls file to that
/// actor's lifetime. Set ControlsAssetPath to a project-relative path
/// (e.g. "Content/Sounds/Amplitude/MyControls.json") and leave AutoLoad
/// enabled to have controls loaded automatically when play starts.
API_CLASS(Attributes="Category(\"Sound/Amplitude\")")
class AMPLITUDEAUDIO_API AudioControlsComponent : public AmplitudeComponent
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCRIPTING_TYPE(AudioControlsComponent);

public:
    // ========================================================================
    //  Serialised properties
    // ========================================================================

    /// Project-relative path to the AudioControlCollection JSON asset.
    API_FIELD(Attributes="EditorOrder(0)")
    String ControlsAssetPath;

    /// When true, Load() is called automatically during OnBeginPlay().
    API_FIELD(Attributes="EditorOrder(1)")
    bool AutoLoad = true;

    // ========================================================================
    //  Public API
    // ========================================================================

    /// \brief Loads the AudioControlCollection and registers all controls with
    ///        the AudioSystem.
    ///
    /// Reads the JSON asset at ControlsAssetPath, parses it into an
    /// AudioControlCollection, and submits each control to the AudioSystem for
    /// registration. Has no effect if the collection is already loaded or if
    /// ControlsAssetPath is empty.
    API_FUNCTION() void Load();

    /// \brief Unregisters all controls that were previously loaded by Load().
    ///
    /// Has no effect if the collection is not currently loaded.
    API_FUNCTION() void Unload();

    /// \return True if the controls have been loaded and registered.
    API_FUNCTION() bool IsLoaded() const;

    // ========================================================================
    //  Script lifecycle overrides
    // ========================================================================

    void OnBeginPlay() override;
    void OnEndPlay() override;

private:
    bool _loaded = false;
};
