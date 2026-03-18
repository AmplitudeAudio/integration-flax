// ============================================================================
//  AudioControlCollectionResource — Named audio control collection asset
//  (implementation)
// ============================================================================

#include "AudioControlCollectionResource.h"

#include <Engine/Core/Types/StringView.h>

namespace AmplitudeAudio
{
    // ========================================================================
    //  Internal helpers
    // ========================================================================

    namespace
    {
        /// \brief Maps the "Type" JSON string to the corresponding enum value.
        ///
        /// Returns AudioControlType::Invalid for any unrecognised string.
        AudioControlType ParseControlType(const char* typeStr)
        {
            if (typeStr == nullptr)
                return AudioControlType::Invalid;

            if (StringView(typeStr) == StringView("Trigger"))
                return AudioControlType::Trigger;

            if (StringView(typeStr) == StringView("Rtpc"))
                return AudioControlType::Rtpc;

            if (StringView(typeStr) == StringView("SoundBank"))
                return AudioControlType::SoundBank;

            if (StringView(typeStr) == StringView("Switch"))
                return AudioControlType::Switch;

            if (StringView(typeStr) == StringView("SwitchState"))
                return AudioControlType::SwitchState;

            if (StringView(typeStr) == StringView("Environment"))
                return AudioControlType::Environment;

            return AudioControlType::Invalid;
        }
    } // anonymous namespace

    // ========================================================================
    //  AudioControlEntryDeserializer
    // ========================================================================

    AmplitudeTriggerData* AudioControlEntryDeserializer::DeserializeTriggerEntry(
        const AudioControlEntry& entry)
    {
        AmplitudeTriggerData* data = new AmplitudeTriggerData();
        data->EventID = static_cast<AmEventID>(entry.AmplitudeId);
        return data;
    }

    AmplitudeRtpcData* AudioControlEntryDeserializer::DeserializeRtpcEntry(
        const AudioControlEntry& entry)
    {
        AmplitudeRtpcData* data = new AmplitudeRtpcData();
        data->RtpcID = static_cast<AmRtpcID>(entry.AmplitudeId);
        return data;
    }

    AmplitudeSwitchStateData* AudioControlEntryDeserializer::DeserializeSwitchStateEntry(
        const AudioControlEntry& entry)
    {
        AmplitudeSwitchStateData* data = new AmplitudeSwitchStateData();
        // AmplitudeId encodes the switch ID; no separate state ID is provided
        // in an AudioControlEntry — the state is selected at runtime.
        data->SwitchID = static_cast<AmSwitchID>(entry.AmplitudeId);
        data->StateID  = 0;
        return data;
    }

    AmplitudeEnvironmentData* AudioControlEntryDeserializer::DeserializeEnvironmentEntry(
        const AudioControlEntry& entry)
    {
        AmplitudeEnvironmentData* data = new AmplitudeEnvironmentData();
        data->EnvironmentID = static_cast<AmEnvironmentID>(entry.AmplitudeId);
        return data;
    }

    AmplitudeBankData* AudioControlEntryDeserializer::DeserializeBankEntry(
        const AudioControlEntry& entry)
    {
        AmplitudeBankData* data = new AmplitudeBankData();
        data->BankID       = static_cast<AmBankID>(entry.AmplitudeId);
        data->BankFilename = entry.AmplitudeName;
        return data;
    }

    // ========================================================================
    //  AudioControlCollection — data access
    // ========================================================================

    const Array<AudioControlEntry>& AudioControlCollection::GetEntries() const
    {
        return m_entries;
    }

    void AudioControlCollection::Clear()
    {
        m_entries.Clear();
    }

    // ========================================================================
    //  AudioControlCollection — JSON loading
    // ========================================================================

    void AudioControlCollection::LoadFromJson(const JsonTools::Value& node)
    {
        // Replace any previously loaded entries.
        m_entries.Clear();

        // Locate the top-level "Controls" array.
        const auto controlsMember = node.FindMember("Controls");
        if (controlsMember == node.MemberEnd() || !controlsMember->value.IsArray())
            return;

        const JsonTools::Value& controlsArray = controlsMember->value;

        for (rapidjson::SizeType i = 0; i < controlsArray.Size(); ++i)
        {
            const JsonTools::Value& element = controlsArray[i];
            if (!element.IsObject())
                continue;

            AudioControlEntry entry;

            // Required: Name
            const auto nameMember = element.FindMember("Name");
            if (nameMember != element.MemberEnd() && nameMember->value.IsString())
                entry.Name = String(nameMember->value.GetString());

            // Required: Type
            const auto typeMember = element.FindMember("Type");
            if (typeMember != element.MemberEnd() && typeMember->value.IsString())
                entry.Type = ParseControlType(typeMember->value.GetString());

            // Optional: AmplitudeId (numeric)
            const auto idMember = element.FindMember("AmplitudeId");
            if (idMember != element.MemberEnd() && idMember->value.IsUint64())
                entry.AmplitudeId = idMember->value.GetUint64();

            // Optional: AmplitudeName (string)
            const auto ampNameMember = element.FindMember("AmplitudeName");
            if (ampNameMember != element.MemberEnd() && ampNameMember->value.IsString())
                entry.AmplitudeName = String(ampNameMember->value.GetString());

            m_entries.Add(entry);
        }
    }

} // namespace AmplitudeAudio
