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

#if USE_EDITOR

#include "ControlsGenerator.h"

#include <Engine/Content/AssetReference.h>
#include <Engine/Content/Content.h>
#include <Engine/Content/JsonAsset.h>
#include <Engine/Core/Log.h>
#include <Engine/Platform/File.h>
#include <Engine/Platform/FileSystem.h>
#include <Engine/Platform/StringUtils.h>
#include <Engine/Serialization/JsonTools.h>
#include <Engine/Serialization/JsonWriter.h>
#include <Engine/Serialization/JsonWriters.h>
#include <Engine/Serialization/Serialization.h>
#include <algorithm>
#include <cstring>

#include <AmplitudeAudio/AmplitudeSettings.h>
#include <AmplitudeAudio/Core/Common.h>

namespace
{
    /// <summary>
    /// Parses a JSON file at <paramref name="path"/> and returns the root document.
    /// Returns false if the file could not be read or parsed.
    /// </summary>
    bool ParseJsonFile(const String& path, rapidjson_flax::Document& outDoc)
    {
        StringAnsi content;
        if (File::ReadAllText(path, content))
            return false;

        outDoc.Parse(content.Get(), content.Length());
        return !outDoc.HasParseError() && outDoc.IsObject();
    }

    /// <summary>
    /// Extracts the filename without extension from a full path.
    /// e.g. "/foo/bar/play_throw.json" -> "play_throw"
    /// </summary>
    String FileNameStem(const String& path)
    {
        // Find last separator.
        int32       lastSep     = path.FindLast(TEXT('/'));
        const int32 lastBackSep = path.FindLast(TEXT('\\'));
        lastSep                 = std::max(lastBackSep, lastSep);

        const String filename = (lastSep >= 0) ? path.Substring(lastSep + 1) : path;

        // Remove extension.
        const int32 dot = filename.FindLast(TEXT('.'));
        return (dot >= 0) ? filename.Substring(0, dot) : filename;
    }

    /// <summary>Scans a directory for .json files (non-recursive) and returns their paths.</summary>
    void ListJsonFiles(const String& dir, Array<String>& outPaths)
    {
        if (!FileSystem::DirectoryExists(dir))
            return;

        FileSystem::DirectoryGetFiles(
            outPaths, dir, TEXT("*.json"), DirectorySearchOption::AllDirectories);
    }

    /// <summary>
    /// Parses the "ID" field from an existing Flax JSON asset's text content.
    /// Returns true and populates <paramref name="outId"/> on success; returns
    /// false (without modifying outId) when the file is not a JSON object or
    /// has no usable "ID" field.
    /// </summary>
    bool TryParseExistingAssetId(const StringAnsi& jsonContent, Guid& outId)
    {
        rapidjson_flax::Document doc;
        doc.Parse(jsonContent.Get(), jsonContent.Length());
        if (doc.HasParseError() || !doc.IsObject())
            return false;

        if (!doc.HasMember("ID") || !doc["ID"].IsString())
            return false;

        const String idStr(doc["ID"].GetString());
        Guid         parsed;
        // Guid::Parse returns TRUE on failure (Flax convention).
        if (Guid::Parse(StringView(*idStr, idStr.Length()), parsed))
            return false;

        outId = parsed;
        return true;
    }

    /// <summary>
    /// Derives a deterministic 128-bit GUID by hashing the bytes of
    /// <paramref name="path"/> with four FNV-1a accumulators seeded
    /// distinctly. The same path always yields the same GUID, which keeps
    /// scene/prefab/script references stable across regeneration.
    /// </summary>
    Guid DeriveStableGuidFromPath(const String& path)
    {
        const StringAnsi pathAnsi(path);
        const char*      data = pathAnsi.Get();
        const int32      len  = pathAnsi.Length();

        // FNV-1a 32-bit constants.
        constexpr uint32 kFnvOffset = 0x811C9DC5u;
        constexpr uint32 kFnvPrime  = 0x01000193u;

        // Seed four independent accumulators so the four output uint32s are
        // distinct functions of the same input.
        uint32 a = kFnvOffset;
        uint32 b = kFnvOffset ^ 0xA5A5A5A5u;
        uint32 c = kFnvOffset ^ 0x5A5A5A5Au;
        uint32 d = kFnvOffset ^ 0xDEADBEEFu;

        for (int32 i = 0; i < len; ++i)
        {
            const uint8 byte = static_cast<uint8>(data[i]);
            a                = (a ^ byte) * kFnvPrime;
            b                = (b ^ byte) * kFnvPrime;
            c                = (c ^ byte) * kFnvPrime;
            d                = (d ^ byte) * kFnvPrime;
        }

        return Guid(a, b, c, d);
    }
} // namespace

void ControlsGenerator::ScanEvents(
    const String& dir, Array<AudioControlEntry>& out)
{
    Array<String> files;
    ListJsonFiles(dir, files);

    for (const String& path : files)
    {
        rapidjson_flax::Document doc;
        if (!ParseJsonFile(path, doc))
            continue;

        AudioControlEntry entry;
        entry.Type = AudioControlType::Trigger;
        entry.Name = FileNameStem(path);

        if (doc.HasMember("name") && doc["name"].IsString())
            entry.AmplitudeName = String(doc["name"].GetString());

        if (doc.HasMember("id") && doc["id"].IsUint64())
            entry.AmplitudeId = doc["id"].GetUint64();

        if (!entry.Name.IsEmpty())
            out.Add(entry);
    }
}

void ControlsGenerator::ScanSwitches(
    const String& dir, Array<AudioControlEntry>& out)
{
    Array<String> files;
    ListJsonFiles(dir, files);

    for (const String& path : files)
    {
        rapidjson_flax::Document doc;
        if (!ParseJsonFile(path, doc))
            continue;

        // Add the switch group itself.
        AudioControlEntry switchEntry;
        switchEntry.Type = AudioControlType::Switch;
        switchEntry.Name = FileNameStem(path);

        if (doc.HasMember("name") && doc["name"].IsString())
            switchEntry.AmplitudeName = String(doc["name"].GetString());

        if (doc.HasMember("id") && doc["id"].IsUint64())
            switchEntry.AmplitudeId = doc["id"].GetUint64();

        // Keep the generated switch entry label aligned with the authored
        // switch token used to namespace switch states. Fall back to the file
        // stem only when the authored token is unavailable.
        if (!switchEntry.AmplitudeName.IsEmpty())
            switchEntry.Name = switchEntry.AmplitudeName;

        if (!switchEntry.Name.IsEmpty())
            out.Add(switchEntry);

        // Add each state within the switch.
        if (doc.HasMember("states") && doc["states"].IsArray())
        {
            const auto& states = doc["states"];
            for (rapidjson::SizeType i = 0; i < states.Size(); ++i)
            {
                const auto& stateNode = states[i];
                if (!stateNode.IsObject())
                    continue;

                AudioControlEntry stateEntry;
                stateEntry.Type = AudioControlType::SwitchState;

                // State Name = authored parent switch token + "/" + state name from JSON.
                if (stateNode.HasMember("name") && stateNode["name"].IsString())
                {
                    const String  stateName(stateNode["name"].GetString());
                    const String& switchToken      = switchEntry.Name;
                    stateEntry.Name                = switchToken + TEXT("/") + stateName;
                    stateEntry.AmplitudeName       = stateName;
                    stateEntry.ParentAmplitudeName = switchToken;
                }

                if (stateNode.HasMember("id") && stateNode["id"].IsUint64())
                    stateEntry.AmplitudeId = stateNode["id"].GetUint64();

                stateEntry.ParentAmplitudeId = switchEntry.AmplitudeId;

                if (!stateEntry.Name.IsEmpty())
                    out.Add(stateEntry);
            }
        }
    }
}

void ControlsGenerator::ScanRtpcs(
    const String& dir, Array<AudioControlEntry>& out)
{
    Array<String> files;
    ListJsonFiles(dir, files);

    for (const String& path : files)
    {
        rapidjson_flax::Document doc;
        if (!ParseJsonFile(path, doc))
            continue;

        AudioControlEntry entry;
        entry.Type = AudioControlType::Rtpc;
        entry.Name = FileNameStem(path);

        if (doc.HasMember("name") && doc["name"].IsString())
            entry.AmplitudeName = String(doc["name"].GetString());

        if (doc.HasMember("id") && doc["id"].IsUint64())
            entry.AmplitudeId = doc["id"].GetUint64();

        if (!entry.Name.IsEmpty())
            out.Add(entry);
    }
}

void ControlsGenerator::ScanSoundBanks(
    const String& dir, Array<AudioControlEntry>& out)
{
    Array<String> files;
    ListJsonFiles(dir, files);

    for (const String& path : files)
    {
        rapidjson_flax::Document doc;
        if (!ParseJsonFile(path, doc))
            continue;

        AudioControlEntry entry;
        entry.Type = AudioControlType::SoundBank;
        entry.Name = FileNameStem(path);

        if (doc.HasMember("name") && doc["name"].IsString())
        {
            entry.AmplitudeName = String(doc["name"].GetString());
        }

        if (doc.HasMember("id") && doc["id"].IsUint64())
            entry.AmplitudeId = doc["id"].GetUint64();

        if (!entry.Name.IsEmpty())
            out.Add(entry);
    }
}

void ControlsGenerator::ScanEffects(
    const String& dir, Array<AudioControlEntry>& out)
{
    Array<String> files;
    ListJsonFiles(dir, files);

    for (const String& path : files)
    {
        rapidjson_flax::Document doc;
        if (!ParseJsonFile(path, doc))
            continue;

        AudioControlEntry entry;
        entry.Type = AudioControlType::Environment;
        entry.Name = FileNameStem(path);

        if (doc.HasMember("name") && doc["name"].IsString())
            entry.AmplitudeName = String(doc["name"].GetString());

        if (doc.HasMember("id") && doc["id"].IsUint64())
            entry.AmplitudeId = doc["id"].GetUint64();

        if (!entry.Name.IsEmpty())
            out.Add(entry);
    }
}

Array<AudioControlEntry> ControlsGenerator::GenerateFromProject(
    const String& projectPath)
{
    Array<AudioControlEntry> entries;

    if (!FileSystem::DirectoryExists(projectPath))
    {
        LOG(Error, "[ControlsGenerator] Project directory does not exist: {0}", projectPath);
        return entries;
    }

    // Scan each asset category.
    ScanEvents(projectPath / TEXT("events"), entries);
    ScanSwitches(projectPath / TEXT("switches"), entries);
    ScanRtpcs(projectPath / TEXT("rtpc"), entries);
    ScanSoundBanks(projectPath / TEXT("soundbanks"), entries);
    ScanEffects(projectPath / TEXT("effects"), entries);

    LOG(Info, "[ControlsGenerator] Discovered {0} controls from: {1}",
        entries.Count(), projectPath);

    return entries;
}

bool ControlsGenerator::ReloadControls(
    const String& projectPath, const String& outputAssetPath)
{
    // 1. Generate controls from the Amplitude project.
    const Array<AudioControlEntry> entries = GenerateFromProject(projectPath);

    // 1b. Resolve the asset's stable GUID. Re-using an existing asset's ID
    //     preserves scene/prefab/script references across regeneration; only
    //     when no asset exists do we derive a new deterministic ID from the
    //     output path. If the existing file is unreadable but present, refuse
    //     to overwrite so the failure surfaces loudly.
    Guid assetId;
    if (FileSystem::FileExists(outputAssetPath))
    {
        StringAnsi existingContent;
        // File::ReadAllText returns TRUE on failure (Flax convention).
        if (File::ReadAllText(outputAssetPath, existingContent))
        {
            LOG(Error, "[ControlsGenerator] Could not read existing asset to preserve its ID: {0}", outputAssetPath);
            return false;
        }

        if (!TryParseExistingAssetId(existingContent, assetId))
        {
            LOG(Error,
                "[ControlsGenerator] Existing asset at '{0}' is unreadable as a Flax JSON asset; "
                "refusing to overwrite to avoid silently breaking inbound references.",
                outputAssetPath);
            return false;
        }
    }
    else
    {
        assetId = DeriveStableGuidFromPath(outputAssetPath);

        // Ensure the parent directory exists before we attempt to write the
        // new asset. FileSystem::CreateDirectory returns TRUE on failure.
        const String parentDir = String(StringUtils::GetDirectoryName(outputAssetPath));
        if (!parentDir.IsEmpty() && !FileSystem::DirectoryExists(parentDir))
        {
            if (FileSystem::CreateDirectory(parentDir))
            {
                LOG(Error, "[ControlsGenerator] Failed to create parent directory: {0}", parentDir);
                return false;
            }
        }
    }

    // 2. Build the JSON asset content.
    //    Flax JSON assets have the format:
    //    { "ID": "guid", "TypeName": "AudioControlCollection",
    //      "EngineBuild": 0, "Data": { ... } }
    rapidjson_flax::StringBuffer buffer;
    PrettyJsonWriter             writer(buffer);

    writer.StartObject();

    writer.JKEY("ID");
    writer.Guid(assetId);

    writer.JKEY("TypeName");
    const char* typeName = "AmplitudeAudio.AudioControlCollection";
    writer.String(typeName, static_cast<int32>(std::strlen(typeName)));

    writer.JKEY("EngineBuild");
    writer.Int(0);

    // Data — serialize the controls array using the same key name
    // that AudioControlCollection::Serialize uses.
    writer.JKEY("Data");
    writer.StartObject();

    writer.JKEY("Controls");
    writer.StartArray();

    for (int32 i = 0; i < entries.Count(); ++i)
    {
        const AudioControlEntry& e = entries[i];
        writer.StartObject();

        writer.JKEY("Name");
        const StringAnsi nameAnsi(e.Name);
        writer.String(nameAnsi.Get(), nameAnsi.Length());

        writer.JKEY("Type");
        writer.Int(static_cast<int32>(e.Type));

        writer.JKEY("AmplitudeId");
        writer.Uint64(e.AmplitudeId);

        writer.JKEY("AmplitudeName");
        const StringAnsi ampNameAnsi(e.AmplitudeName);
        writer.String(ampNameAnsi.Get(), ampNameAnsi.Length());

        writer.JKEY("ParentAmplitudeId");
        writer.Uint64(e.ParentAmplitudeId);

        writer.JKEY("ParentAmplitudeName");
        const StringAnsi parentAmpNameAnsi(e.ParentAmplitudeName);
        writer.String(parentAmpNameAnsi.Get(), parentAmpNameAnsi.Length());

        writer.EndObject();
    }

    writer.EndArray();
    writer.EndObject(); // Data
    writer.EndObject(); // root

    // 3. Write the JSON string to the output file.
    const StringAnsi jsonContent(buffer.GetString(), static_cast<int32>(buffer.GetSize()));
    if (File::WriteAllText(outputAssetPath, String(jsonContent), Encoding::UTF8))
    {
        LOG(Error, "[ControlsGenerator] Failed to write asset: {0}", outputAssetPath);
        return false;
    }

    LOG(Info, "[ControlsGenerator] Wrote {0} controls to: {1}",
        entries.Count(), outputAssetPath);
    return true;
}

String ControlsGenerator::GetProjectSourcesPath()
{
    const AmplitudeSettings* settings = AmplitudeSettings::Get();
    if (settings == nullptr)
    {
        LOG(Warning, "[ControlsGenerator] Amplitude Configuration settings not found.");
        return String::Empty;
    }

    const String& projectSourcesPath = settings->ProjectSourcesPath;
    if (projectSourcesPath.IsEmpty())
        return String(AmplitudeAudio::kAssetBasePath) + String("sources");

    return String(AmplitudeAudio::kAssetBasePath) + projectSourcesPath;
}

#endif // USE_EDITOR