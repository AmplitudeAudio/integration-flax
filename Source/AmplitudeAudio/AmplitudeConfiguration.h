// Copyright (c) 2026-present Sparky Studios. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

// ============================================================================
//  AmplitudeConfiguration — Per-platform configuration profiles
//
//  Defines the AmplitudeConfiguration struct (per-platform settings),
//  AmplitudePlatformProfile struct, the AmplitudeConfigurationAsset class
//  (editable JSON asset), and the internal AmplitudeAssetProfiles helper.
// ============================================================================

#include <Engine/Core/Collections/Array.h>
#include <Engine/Core/Collections/Dictionary.h>
#include <Engine/Core/ISerializable.h>
#include <Engine/Core/Types/String.h>
#include <Engine/Scripting/ScriptingObject.h>
#include <Engine/Serialization/JsonTools.h>

// ============================================================================
//  AmplitudeConfiguration
// ============================================================================

/// \brief Per-platform Amplitude configuration.
///
/// Groups the two filenames that the plugin needs at startup: the initial
/// sound bank and the Amplitude engine config file.
API_STRUCT()
struct AMPLITUDEAUDIO_API AmplitudeConfiguration : public ISerializable
{
    DECLARE_SCRIPTING_TYPE_MINIMAL(AmplitudeConfiguration);
    API_AUTO_SERIALIZATION();

    /// Filename of the initial sound bank to load at startup.
    /// Example: "init.ambank"
    API_FIELD(Attributes="EditorOrder(0)")
    String InitSoundBank;

    /// Filename of the Amplitude engine config file.
    /// Example: "pc.amconfig"
    API_FIELD(Attributes="EditorOrder(1)")
    String EngineConfigFile;
};

// ============================================================================
//  AmplitudePlatformProfile
// ============================================================================

/// \brief A named platform profile binding a platform name to its configuration.
API_STRUCT()
struct AMPLITUDEAUDIO_API AmplitudePlatformProfile : public ISerializable
{
    DECLARE_SCRIPTING_TYPE_MINIMAL(AmplitudePlatformProfile);
    API_AUTO_SERIALIZATION();

    /// Platform name (e.g. "Desktop", "Mobile").
    API_FIELD(Attributes="EditorOrder(0)")
    String PlatformName;

    /// Configuration for this platform.
    API_FIELD(Attributes="EditorOrder(1)")
    AmplitudeConfiguration Config;
};

// ============================================================================
//  AmplitudeConfigurationAsset — Editable JSON asset
// ============================================================================

/// \brief Flax content asset that stores per-platform Amplitude configuration.
///
/// Create via the editor's "New > Audio > Amplitude Configuration" context menu.
/// Contains an array of platform profiles and an optional Amplitude project path.
API_CLASS(Attributes="ContentContextMenu(\"New/Audio/Amplitude Configuration\")")
class AMPLITUDEAUDIO_API AmplitudeConfigurationAsset : public ScriptingObject, public ISerializable
{
    DECLARE_SCRIPTING_TYPE_WITH_CONSTRUCTOR_IMPL(AmplitudeConfigurationAsset, ScriptingObject);

public:
    /// Per-platform configuration profiles.
    API_FIELD(Attributes="EditorOrder(0)")
    Array<AmplitudePlatformProfile> Profiles;

    /// Path to the Amplitude project build output, relative to kAssetBasePath.
    /// At runtime, the Amplitude file system base path is set to {kAssetBasePath}/{ProjectBuildPath}.
    /// Example: "build" if built assets are at Content/Sounds/Amplitude/build/
    API_FIELD(Attributes="EditorOrder(1)")
    String ProjectBuildPath;

    /// Path to the Amplitude project sources, relative to kAssetBasePath.
    /// Used by the editor controls generator to scan for audio control definitions.
    /// Example: "sources" if source JSON files are at Content/Sounds/Amplitude/sources/
    API_FIELD(Attributes="EditorOrder(2)")
    String ProjectSourcesPath;

    // ========================================================================
    //  ISerializable
    // ========================================================================

    void Serialize(SerializeStream& stream, const void* otherObj) override;
    void Deserialize(DeserializeStream& stream, ISerializeModifier* modifier) override;
};

// ============================================================================
//  AmplitudeAssetProfiles (internal, not exposed to scripting)
// ============================================================================

namespace AmplitudeAudio
{
    /// \brief A named map of per-platform AmplitudeConfiguration values.
    ///
    /// Used internally by the middleware to store loaded configuration.
    class AmplitudeAssetProfiles
    {
    public:
        AmplitudeAssetProfiles() = default;
        ~AmplitudeAssetProfiles() = default;

        void Add(const String& platformName, const AmplitudeConfiguration& config);
        [[nodiscard]] const AmplitudeConfiguration* Get(const String& platformName) const;
        [[nodiscard]] static String GetCurrentPlatform();

        /// Build output path, relative to kAssetBasePath. Used at runtime.
        [[nodiscard]] const String& GetProjectBuildPath() const;

        /// Source project path, relative to kAssetBasePath. Used by editor controls generator.
        [[nodiscard]] const String& GetProjectSourcesPath() const;

        /// Populates this object from an AmplitudeConfigurationAsset's data.
        void LoadFromAsset(const Array<AmplitudePlatformProfile>& profiles,
                           const String& projectBuildPath,
                           const String& projectSourcesPath);

    private:
        Dictionary<String, AmplitudeConfiguration> m_profiles;
        String m_projectBuildPath;
        String m_projectSourcesPath;
    };

} // namespace AmplitudeAudio
