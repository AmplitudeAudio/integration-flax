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

/// <summary>Per-platform Amplitude configuration.</summary>
/// <remarks>
/// Groups the two filenames that the plugin needs at startup: the initial
/// sound bank and the Amplitude engine config file.
/// </remarks>
API_STRUCT()
struct AMPLITUDEAUDIO_API AmplitudeConfiguration : public ISerializable
{
    DECLARE_SCRIPTING_TYPE_MINIMAL(AmplitudeConfiguration);
    API_AUTO_SERIALIZATION();

    /// <summary>
    /// Filename of the initial sound bank to load at startup.
    /// Example: "init.ambank"
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(0)")
    String InitSoundBank;

    /// <summary>
    /// Filename of the Amplitude engine config file.
    /// Example: "pc.amconfig"
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(1)")
    String EngineConfigFile;
};

// ============================================================================
//  AmplitudePlatformProfile
// ============================================================================

/// <summary>A named platform profile binding a platform name to its configuration.</summary>
API_STRUCT()
struct AMPLITUDEAUDIO_API AmplitudePlatformProfile : public ISerializable
{
    DECLARE_SCRIPTING_TYPE_MINIMAL(AmplitudePlatformProfile);
    API_AUTO_SERIALIZATION();

    /// <summary>Platform name (e.g. "Desktop", "Mobile").</summary>
    API_FIELD(Attributes = "EditorOrder(0)")
    String PlatformName;

    /// <summary>Configuration for this platform.</summary>
    API_FIELD(Attributes = "EditorOrder(1)")
    AmplitudeConfiguration Config;
};

// ============================================================================
//  AmplitudeConfigurationAsset — Editable JSON asset
// ============================================================================

/// <summary>Flax content asset that stores per-platform Amplitude configuration.</summary>
/// <remarks>
/// Create via the editor's "New > Audio > Amplitude Configuration" context menu.
/// Contains an array of platform profiles and an optional Amplitude project path.
/// </remarks>
API_CLASS(Attributes = "ContentContextMenu(\"New/Audio System/Amplitude/Configuration\")")
class AMPLITUDEAUDIO_API AmplitudeConfigurationAsset : public ScriptingObject, public ISerializable
{
    DECLARE_SCRIPTING_TYPE_WITH_CONSTRUCTOR_IMPL(AmplitudeConfigurationAsset, ScriptingObject);

  public:
    /// <summary>Per-platform configuration profiles.</summary>
    API_FIELD(Attributes = "EditorOrder(0)")
    Array<AmplitudePlatformProfile> Profiles;

    /// <summary>
    /// Path to the Amplitude project build output, relative to kAssetBasePath.
    /// At runtime, the Amplitude file system base path is set to {kAssetBasePath}/{ProjectBuildPath}.
    /// Example: "build" if built assets are at Content/Sounds/Amplitude/build/
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(1)")
    String ProjectBuildPath;

    /// <summary>
    /// Path to the Amplitude project sources, relative to kAssetBasePath.
    /// Used by the editor controls generator to scan for audio control definitions.
    /// Example: "sources" if source JSON files are at Content/Sounds/Amplitude/sources/
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(2)")
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
    /// <summary>A named map of per-platform AmplitudeConfiguration values.</summary>
    /// <remarks>Used internally by the middleware to store loaded configuration.</remarks>
    class AmplitudeAssetProfiles
    {
      public:
        AmplitudeAssetProfiles()  = default;
        ~AmplitudeAssetProfiles() = default;

        void                                        Add(const String& platformName, const AmplitudeConfiguration& config);
        [[nodiscard]] const AmplitudeConfiguration* Get(const String& platformName) const;
        [[nodiscard]] static String                 GetCurrentPlatform();

        /// <summary>Build output path, relative to kAssetBasePath. Used at runtime.</summary>
        [[nodiscard]] const String& GetProjectBuildPath() const;

        /// <summary>Source project path, relative to kAssetBasePath. Used by editor controls generator.</summary>
        [[nodiscard]] const String& GetProjectSourcesPath() const;

        /// <summary>Populates this object from an AmplitudeConfigurationAsset's data.</summary>
        void LoadFromAsset(const Array<AmplitudePlatformProfile>& profiles,
                           const String&                          projectBuildPath,
                           const String&                          projectSourcesPath);

      private:
        Dictionary<String, AmplitudeConfiguration> m_profiles;
        String                                     m_projectBuildPath;
        String                                     m_projectSourcesPath;
    };

} // namespace AmplitudeAudio
