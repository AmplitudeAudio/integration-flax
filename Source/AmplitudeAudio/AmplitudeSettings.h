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

#include <Engine/Core/Collections/Array.h>
#include <Engine/Core/Config/Settings.h>
#include <Engine/Core/ISerializable.h>
#include <Engine/Core/Types/String.h>
#include <Engine/Platform/Defines.h>
#include <Engine/Scripting/ScriptingType.h>

#include <AmplitudeAudio/Types/AmplitudeSoundBankRef.h>

/// <summary>How the Amplitude project build ships with cooked games.</summary>
API_ENUM(Namespace = "AmplitudeAudio")
enum class AmplitudePackagingMode
{
    /// <summary>Copy the build folder as is.</summary>
    LooseFiles = 0,

    /// <summary>Package the build folder into a single .ampk file with the ampk tool.</summary>
    Ampk = 1,
};

/// <summary>Compression used for .ampk packages. Values match the ampk <c>-c</c> option.</summary>
API_ENUM(Namespace = "AmplitudeAudio")
enum class AmplitudeCompression
{
    /// <summary>No compression.</summary>
    None = 0,

    /// <summary>ZLib compression.</summary>
    ZLib = 1,
};

/// <summary>An Amplitude plugin library to load, by name.</summary>
/// <remarks>A struct, not a plain String: Flax's C# bindings can't marshal Array&lt;String&gt; inside an API_STRUCT.</remarks>
API_STRUCT(Namespace = "AmplitudeAudio")
struct AMPLITUDEAUDIO_API AmplitudePlugin : public ISerializable
{
    DECLARE_SCRIPTING_TYPE_MINIMAL(AmplitudePlugin);
    API_AUTO_SERIALIZATION();

    /// <summary>Plugin library name, as passed to the engine's plugin loader.</summary>
    API_FIELD()
    String Name;
};

/// <summary>A folder searched for Amplitude plugin libraries, relative to <c>Content/Sounds/Amplitude</c>.</summary>
/// <remarks>A struct, not a plain String: Flax's C# bindings can't marshal Array&lt;String&gt; inside an API_STRUCT.</remarks>
API_STRUCT(Namespace = "AmplitudeAudio")
struct AMPLITUDEAUDIO_API AmplitudePluginSearchPath : public ISerializable
{
    DECLARE_SCRIPTING_TYPE_MINIMAL(AmplitudePluginSearchPath);
    API_AUTO_SERIALIZATION();

    /// <summary>Folder relative to Content/Sounds/Amplitude.</summary>
    API_FIELD()
    String Path;
};

/// <summary>Amplitude startup configuration for one platform.</summary>
API_STRUCT(Namespace = "AmplitudeAudio")
struct AMPLITUDEAUDIO_API AmplitudePlatformProfile : public ISerializable
{
    DECLARE_SCRIPTING_TYPE_MINIMAL(AmplitudePlatformProfile);
    API_AUTO_SERIALIZATION();

    /// <summary>Platform this profile applies to. Ignored on the default profile.</summary>
    API_FIELD(Attributes = "EditorOrder(0)")
    PlatformType Platform = PlatformType::Windows;

    /// <summary>Engine configuration file, relative to the project build folder (e.g. <c>pc.config.amconfig</c>).</summary>
    API_FIELD(Attributes = "EditorOrder(1)")
    String EngineConfigFile;

    /// <summary>Banks loaded at startup, before any scene.</summary>
    API_FIELD(Attributes = "EditorOrder(2)")
    Array<AmplitudeSoundBankRef> InitSoundBanks;

    /// <summary>Amplitude plugin libraries to load before the engine initializes.</summary>
    API_FIELD(Attributes = "EditorOrder(3)")
    Array<AmplitudePlugin> Plugins;

    /// <summary>Folders searched for plugin libraries, relative to <c>Content/Sounds/Amplitude</c>. Always deployed as loose files.</summary>
    API_FIELD(Attributes = "EditorOrder(4)")
    Array<AmplitudePluginSearchPath> PluginSearchPaths;
};

/// <summary>Amplitude Audio settings, stored in the game settings under "Amplitude Settings".</summary>
API_CLASS(sealed, Namespace = "FlaxEditor.Content.Settings", Attributes = "ContentContextMenu(\"New/Amplitude/Settings\")")
class AMPLITUDEAUDIO_API AmplitudeSettings : public SettingsBase
{
    DECLARE_SCRIPTING_TYPE_MINIMAL(AmplitudeSettings);
    API_AUTO_SERIALIZATION();

public:
    /// <summary>Amplitude project sources, relative to <c>Content/Sounds/Amplitude</c>. Editor only.</summary>
    API_FIELD(Attributes = "EditorOrder(0), EditorDisplay(\"Project\")")
    String ProjectSourcesPath = TEXT("sources");

    /// <summary>Amplitude project build, relative to <c>Content/Sounds/Amplitude</c>.</summary>
    API_FIELD(Attributes = "EditorOrder(1), EditorDisplay(\"Project\")")
    String ProjectBuildPath = TEXT("build");

    /// <summary>Profile used when no entry in Profiles matches the running platform.</summary>
    API_FIELD(Attributes = "EditorOrder(10), EditorDisplay(\"Profiles\")")
    AmplitudePlatformProfile DefaultProfile;

    /// <summary>Per-platform profiles; the first entry matching the running platform wins.</summary>
    API_FIELD(Attributes = "EditorOrder(11), EditorDisplay(\"Profiles\")")
    Array<AmplitudePlatformProfile> Profiles;

    /// <summary>How the project build ships with cooked games.</summary>
    API_FIELD(Attributes = "EditorOrder(20), EditorDisplay(\"Cooking\")")
    AmplitudePackagingMode PackagingMode = AmplitudePackagingMode::LooseFiles;

    /// <summary>Path to the ampk executable. When empty, <c>$AM_SDK_PATH/bin</c> then <c>PATH</c> are searched.</summary>
    API_FIELD(Attributes = "EditorOrder(21), EditorDisplay(\"Cooking\")")
    String AmpkToolPath;

    /// <summary>Compression used for .ampk packages.</summary>
    API_FIELD(Attributes = "EditorOrder(22), EditorDisplay(\"Cooking\")")
    AmplitudeCompression Compression = AmplitudeCompression::None;

    /// <summary>Compression block size, in KB.</summary>
    API_FIELD(Attributes = "EditorOrder(23), EditorDisplay(\"Cooking\"), Limit(1)")
    int32 BlockSize = 64;

    /// <summary>Returns the profile for <paramref name="platform"/>, or DefaultProfile when none matches.</summary>
    AmplitudePlatformProfile GetProfile(PlatformType platform) const;

    DECLARE_SETTINGS_GETTER(AmplitudeSettings);
};
