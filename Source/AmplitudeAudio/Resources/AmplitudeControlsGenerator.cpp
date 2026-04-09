// ============================================================================
//  AmplitudeControlsGenerator — Editor-only audio controls discovery
//  (implementation)
// ============================================================================

#if USE_EDITOR

#include "AmplitudeControlsGenerator.h"

#include <Engine/Content/Content.h>
#include <Engine/Content/JsonAsset.h>
#include <Engine/Core/Log.h>
#include <Engine/Platform/File.h>
#include <Engine/Platform/FileSystem.h>
#include <Engine/Serialization/JsonTools.h>
#include <Engine/Serialization/JsonWriter.h>
#include <Engine/Serialization/JsonWriters.h>
#include <Engine/Serialization/Serialization.h>
#include <cstring>

#include "AmplitudeAudio/Core/Common.h"

// ============================================================================
//  Helpers
// ============================================================================

namespace
{
    /// Parses a JSON file at \p path and returns the root document.
    /// Returns false if the file could not be read or parsed.
    bool ParseJsonFile(const String& path, rapidjson_flax::Document& outDoc)
    {
        StringAnsi content;
        if (File::ReadAllText(path, content))
            return false;

        outDoc.Parse(content.Get(), content.Length());
        return !outDoc.HasParseError() && outDoc.IsObject();
    }

    /// Extracts the filename without extension from a full path.
    /// e.g. "/foo/bar/play_throw.json" -> "play_throw"
    String FileNameStem(const String& path)
    {
        // Find last separator.
        int32       lastSep     = path.FindLast(TEXT('/'));
        const int32 lastBackSep = path.FindLast(TEXT('\\'));
        if (lastBackSep > lastSep)
            lastSep = lastBackSep;

        const String filename = (lastSep >= 0) ? path.Substring(lastSep + 1) : path;

        // Remove extension.
        const int32 dot = filename.FindLast(TEXT('.'));
        return (dot >= 0) ? filename.Substring(0, dot) : filename;
    }

    /// Scans a directory for .json files (non-recursive) and returns their paths.
    void ListJsonFiles(const String& dir, Array<String>& outPaths)
    {
        if (!FileSystem::DirectoryExists(dir))
            return;

        FileSystem::DirectoryGetFiles(
            outPaths, dir, TEXT("*.json"), DirectorySearchOption::AllDirectories);
    }
} // namespace

// ============================================================================
//  Per-category scanners
// ============================================================================

void AmplitudeControlsGenerator::ScanEvents(
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

void AmplitudeControlsGenerator::ScanSwitches(
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

void AmplitudeControlsGenerator::ScanRtpcs(
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

void AmplitudeControlsGenerator::ScanSoundBanks(
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

void AmplitudeControlsGenerator::ScanEffects(
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

// ============================================================================
//  GenerateFromProject
// ============================================================================

Array<AudioControlEntry> AmplitudeControlsGenerator::GenerateFromProject(
    const String& projectPath)
{
    Array<AudioControlEntry> entries;

    if (!FileSystem::DirectoryExists(projectPath))
    {
        LOG(Error, "[AmplitudeControlsGenerator] Project directory does not exist: {0}", projectPath);
        return entries;
    }

    // Scan each asset category.
    ScanEvents(projectPath / TEXT("events"), entries);
    ScanSwitches(projectPath / TEXT("switches"), entries);
    ScanRtpcs(projectPath / TEXT("rtpc"), entries);
    ScanSoundBanks(projectPath / TEXT("soundbanks"), entries);
    ScanEffects(projectPath / TEXT("effects"), entries);

    LOG(Info, "[AmplitudeControlsGenerator] Discovered {0} controls from: {1}",
        entries.Count(), projectPath);

    return entries;
}

// ============================================================================
//  ReloadControls — write a Flax JSON asset file
// ============================================================================

bool AmplitudeControlsGenerator::ReloadControls(
    const String& projectPath, const String& outputAssetPath)
{
    // 1. Generate controls from the Amplitude project.
    const Array<AudioControlEntry> entries = GenerateFromProject(projectPath);

    // 2. Build the JSON asset content.
    //    Flax JSON assets have the format:
    //    { "ID": "guid", "TypeName": "AudioControlCollection",
    //      "EngineBuild": 0, "Data": { ... } }
    rapidjson_flax::StringBuffer buffer;
    PrettyJsonWriter             writer(buffer);

    writer.StartObject();

    // ID — generate a deterministic GUID from the output path.
    const Guid assetId = Guid::New();
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
        LOG(Error, "[AmplitudeControlsGenerator] Failed to write asset: {0}", outputAssetPath);
        return false;
    }

    LOG(Info, "[AmplitudeControlsGenerator] Wrote {0} controls to: {1}",
        entries.Count(), outputAssetPath);
    return true;
}

// ============================================================================
//  GetProjectSourcesPath — resolve from config asset
// ============================================================================

String AmplitudeControlsGenerator::GetProjectSourcesPath()
{
    const String configPath = String(AmplitudeAudio::kAssetBasePath) + TEXT("/Config.json");

    const auto asset = Content::Load<JsonAsset>(configPath);
    if (asset == nullptr || asset->WaitForLoaded() || asset->Data == nullptr)
    {
        LOG(Warning, "[AmplitudeControlsGenerator] Could not load config asset: {0}", configPath);
        return String::Empty;
    }

    String projectSourcesPath;
    {
        auto&               stream   = *asset->Data;
        ISerializeModifier* modifier = nullptr;
        DESERIALIZE_MEMBER(ProjectSourcesPath, projectSourcesPath);
    }

    if (projectSourcesPath.IsEmpty())
        return String::Empty;

    return String(AmplitudeAudio::kAssetBasePath) + projectSourcesPath;
}

#endif // USE_EDITOR
