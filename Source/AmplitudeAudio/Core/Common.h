#pragma once

// ============================================================================
//  AmplitudeAudio — Common constants
//
//  Shared compile-time constants used across the AmplitudeAudio plugin.
//  This file intentionally contains only defines and constexpr values.
//  No classes, functions, or mutable state belong here.
// ============================================================================

namespace AmplitudeAudio
{
    // ========================================================================
    //  Plugin identity
    // ========================================================================

    /// Display name of this plugin, used for logging and registration.
    static constexpr const char* kPluginName = "Amplitude";

    // ========================================================================
    //  Asset paths
    // ========================================================================

    /// Root directory (relative to the project's Content folder) under which
    /// all Amplitude asset files are stored.
    static constexpr const char* kAssetBasePath = "Content/Sounds/Amplitude/";

    // ========================================================================
    //  File extensions
    // ========================================================================

    /// Sound bank package files produced by the Amplitude build pipeline.
    static constexpr const char* kExtSoundBank = ".ambank";

    /// Engine configuration files consumed at Amplitude initialisation.
    static constexpr const char* kExtConfig = ".amconfig";

    /// Binary pack files that bundle multiple Amplitude assets together.
    static constexpr const char* kExtPack = ".ampack";

    // ========================================================================
    //  Default asset names
    // ========================================================================

    /// File name (without extension) of the default Amplitude engine config.
    static constexpr const char* kDefaultConfigName = "pc.amconfig";

    /// File name (without extension) of the sound bank that is always loaded
    /// on engine startup.
    static constexpr const char* kDefaultInitBankName = "init.ambank";

} // namespace AmplitudeAudio
