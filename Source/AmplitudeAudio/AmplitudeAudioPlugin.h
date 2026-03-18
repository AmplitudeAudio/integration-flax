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
API_CLASS() class AMPLITUDEAUDIO_API AmplitudeAudioPlugin : public GamePlugin
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCRIPTING_TYPE(AmplitudeAudioPlugin);

public:
    // ========================================================================
    //  GamePlugin overrides
    // ========================================================================

    /// \brief Creates the AmplitudeMiddleware instance and registers it with
    ///        the AudioSystem.
    void Initialize() override;

    /// \brief Unregisters and destroys the AmplitudeMiddleware instance.
    void Deinitialize() override;

    // ========================================================================
    //  Plugin metadata
    // ========================================================================

    /// \return Human-readable plugin name shown in the Flax editor.
    String GetName() const override;

    /// \return Short description of what this plugin provides.
    String GetDescription() const override;

private:
    /// Owned middleware instance. Created in Initialize(), destroyed in Deinitialize().
    AmplitudeAudio::AmplitudeMiddleware* _middleware = nullptr;
};
