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

class ISerializeModifier;
class JsonAsset;

namespace AmplitudeAudio
{
    /// <summary>
    /// Display name of this plugin, used for logging and registration.
    /// </summary>
    static constexpr const char* kPluginName = "Amplitude";

    /// <summary>
    /// Root directory (relative to the project's Content folder) under which
    /// all Amplitude asset files are stored.
    /// </summary>
    static constexpr const char* kAssetBasePath = "Content/Sounds/Amplitude/";

    /// <summary>
    /// Sound bank package files produced by the Amplitude build pipeline.
    /// </summary>
    static constexpr const char* kExtSoundBank = ".ambank";

    /// <summary>
    /// Engine configuration files consumed at Amplitude initialization.
    /// </summary>
    static constexpr const char* kExtConfig = ".amconfig";

    /// <summary>
    /// Binary pack files for release builds.
    /// </summary>
    static constexpr const char* kExtPack = ".ampk";

    /// <summary>
    /// File name (without extension) of the default Amplitude engine config.
    /// </summary>
    static constexpr const char* kDefaultConfigName = "pc.amconfig";

    /// <summary>
    /// File name (without extension) of the sound bank that is always loaded
    /// on engine startup.
    /// </summary>
    static constexpr const char* kDefaultInitBankName = "init.ambank";

    /// <summary>
    /// Returns the active <c>ISerializeModifier</c> for a JsonAsset deserialize
    /// pass. The Flax C++ JsonAsset pipeline does not currently expose a
    /// modifier; this helper exists as the single migration point for when a
    /// future asset-versioning hook is added.
    /// </summary>
    inline ISerializeModifier* GetActiveModifier(const JsonAsset* /*asset*/)
    {
        return nullptr;
    }
} // namespace AmplitudeAudio
