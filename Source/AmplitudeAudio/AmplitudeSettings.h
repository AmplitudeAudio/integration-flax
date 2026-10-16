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
#include <Engine/Core/Types/LayersMask.h>
#include <Engine/Core/Types/String.h>
#include <Engine/Content/AssetReference.h>
#include <Engine/Content/JsonAsset.h>
#include <Engine/Level/Tags.h>
#include <Engine/Physics/PhysicalMaterial.h>
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

/// <summary>How an audio source is checked for physics obstruction.</summary>
API_ENUM(Namespace = "AmplitudeAudio")
enum class AmplitudeObstructionMode
{
    /// <summary>No obstruction or occlusion ray casting.</summary>
    None = 0,

    /// <summary>Cast a single ray between the source and listener.</summary>
    SingleRay = 1,

    /// <summary>Cast a direct ray plus indirect rays on a disk to distinguish obstruction from occlusion.</summary>
    MultipleRays = 2,
};

/// <summary>Obstruction amount assigned to an exact physical material.</summary>
API_STRUCT(Namespace = "AmplitudeAudio")
struct AMPLITUDEAUDIO_API AmplitudeObstructionMaterialAmount : public ISerializable
{
    DECLARE_SCRIPTING_TYPE_MINIMAL(AmplitudeObstructionMaterialAmount);
    API_AUTO_SERIALIZATION();

    /// <summary>The physical material.</summary>
    API_FIELD(Attributes = "EditorOrder(0), AssetReference(typeof(PhysicalMaterial))")
    AssetReference<JsonAsset> Material;

    /// <summary>Obstruction amount (0 = transparent, 1 = fully solid).</summary>
    API_FIELD(Attributes = "EditorOrder(1), Limit(0, 1, 0.01f)")
    float Amount = 1.0f;
};

/// <summary>Obstruction amount assigned to a surface tag or tag hierarchy.</summary>
API_STRUCT(Namespace = "AmplitudeAudio")
struct AMPLITUDEAUDIO_API AmplitudeObstructionTagAmount : public ISerializable
{
    DECLARE_SCRIPTING_TYPE_MINIMAL(AmplitudeObstructionTagAmount);
    API_AUTO_SERIALIZATION();

    /// <summary>The surface tag matched against PhysicalMaterial.Tag.</summary>
    API_FIELD(Attributes = "EditorOrder(0)")
    Tag SurfaceTag;

    /// <summary>Obstruction amount (0 = transparent, 1 = fully solid).</summary>
    API_FIELD(Attributes = "EditorOrder(1), Limit(0, 1, 0.01f)")
    float Amount = 1.0f;
};

/// <summary>Settings for physics ray cast obstruction and occlusion.</summary>
API_STRUCT(Namespace = "AmplitudeAudio")
struct AMPLITUDEAUDIO_API AmplitudeObstructionSettings : public ISerializable
{
    DECLARE_SCRIPTING_TYPE_MINIMAL(AmplitudeObstructionSettings);

    // Written by hand: the editor saves this asset from C#, which serializes LayersMask as {"Mask": n}
    // while Flax's native LayersMask only reads a bare number.
    void Serialize(SerializeStream& stream, const void* otherObj) override;
    void Deserialize(DeserializeStream& stream, ISerializeModifier* modifier) override;

    /// <summary>Default obstruction amount when a surface has no material or tag rule.</summary>
    API_FIELD(Attributes = "EditorOrder(0), Limit(0, 1, 0.01f)")
    float DefaultAmount = 1.0f;

    /// <summary>Per-material obstruction amounts.</summary>
    API_FIELD(Attributes = "EditorOrder(1)")
    Array<AmplitudeObstructionMaterialAmount> MaterialAmounts;

    /// <summary>Per-tag obstruction amounts.</summary>
    API_FIELD(Attributes = "EditorOrder(2)")
    Array<AmplitudeObstructionTagAmount> TagAmounts;

    /// <summary>Maximum distance from listener at which obstruction rays are cast (units).</summary>
    API_FIELD(Attributes = "EditorOrder(3), Limit(0)")
    float MaxDistance = 5000.0f;

    /// <summary>Distance up to which obstruction is fully applied before linear falloff to MaxDistance.</summary>
    API_FIELD(Attributes = "EditorOrder(4), Limit(0)")
    float FullObstructionDistance = 1000.0f;

    /// <summary>Maximum rays cast per frame across all sources.</summary>
    API_FIELD(Attributes = "EditorOrder(5), Limit(0)")
    int32 RayBudgetPerFrame = 16;

    /// <summary>Number of indirect rays in a full disk cycle for MultipleRays mode.</summary>
    API_FIELD(Attributes = "EditorOrder(6), Limit(0)")
    int32 IndirectRayCount = 8;

    /// <summary>Radius of the indirect ray disk around the listener (units).</summary>
    API_FIELD(Attributes = "EditorOrder(7), Limit(0)")
    float IndirectRadius = 150.0f;

    /// <summary>Seed used to generate deterministic indirect disk ray offsets.</summary>
    API_FIELD(Attributes = "EditorOrder(8)")
    uint32 RandomSeed = 1337;

    /// <summary>Smoothing time constant when obstruction increases (seconds).</summary>
    API_FIELD(Attributes = "EditorOrder(9), Limit(0)")
    float AttackSmoothing = 0.05f;

    /// <summary>Smoothing time constant when obstruction decreases (seconds).</summary>
    API_FIELD(Attributes = "EditorOrder(10), Limit(0)")
    float ReleaseSmoothing = 0.25f;

    /// <summary>Physics layer mask used for obstruction ray casts.</summary>
    API_FIELD(Attributes = "EditorOrder(11)")
    LayersMask Layers;

    /// <summary>Draw debug visualization lines for obstruction rays.</summary>
    API_FIELD(Attributes = "EditorOrder(12)")
    bool DebugDraw = false;
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

    /// <summary>Physics obstruction and occlusion settings.</summary>
    API_FIELD(Attributes = "EditorOrder(30), EditorDisplay(\"Obstruction\")")
    AmplitudeObstructionSettings Obstruction;

    /// <summary>Returns the profile for <paramref name="platform"/>, or DefaultProfile when none matches.</summary>
    AmplitudePlatformProfile GetProfile(PlatformType platform) const;

    DECLARE_SETTINGS_GETTER(AmplitudeSettings);
};
