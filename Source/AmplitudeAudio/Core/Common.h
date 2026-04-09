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

    /// <summary>Display name of this plugin, used for logging and registration.</summary>
    static constexpr const char* kPluginName = "Amplitude";

    // ========================================================================
    //  Asset paths
    // ========================================================================

    /// <summary>
    /// Root directory (relative to the project's Content folder) under which
    /// all Amplitude asset files are stored.
    /// </summary>
    static constexpr const char* kAssetBasePath = "Content/Sounds/Amplitude/";

    // ========================================================================
    //  File extensions
    // ========================================================================

    /// <summary>Sound bank package files produced by the Amplitude build pipeline.</summary>
    static constexpr const char* kExtSoundBank = ".ambank";

    /// <summary>Engine configuration files consumed at Amplitude initialisation.</summary>
    static constexpr const char* kExtConfig = ".amconfig";

    /// <summary>Binary pack files that bundle multiple Amplitude assets together.</summary>
    static constexpr const char* kExtPack = ".ampk";

    // ========================================================================
    //  Default asset names
    // ========================================================================

    /// <summary>File name (without extension) of the default Amplitude engine config.</summary>
    static constexpr const char* kDefaultConfigName = "pc.amconfig";

    /// <summary>
    /// File name (without extension) of the sound bank that is always loaded
    /// on engine startup.
    /// </summary>
    static constexpr const char* kDefaultInitBankName = "init.ambank";

} // namespace AmplitudeAudio
