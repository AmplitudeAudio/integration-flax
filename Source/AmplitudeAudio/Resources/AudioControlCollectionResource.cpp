// ============================================================================
//  AudioControlCollectionResource — implementation
// ============================================================================

#include "AudioControlCollectionResource.h"

#include <Engine/Serialization/Serialization.h>

// ============================================================================
//  AudioControlCollection — property accessors
// ============================================================================

const Array<AudioControlEntry>& AudioControlCollection::GetControls() const
{
    return _controls;
}

void AudioControlCollection::SetControls(const Array<AudioControlEntry>& value)
{
    _controls = value;
}

// ============================================================================
//  AudioControlCollection — ISerializable
// ============================================================================

void AudioControlCollection::Serialize(SerializeStream& stream, const void* otherObj)
{
    const auto* other = static_cast<const AudioControlCollection*>(otherObj);
    SERIALIZE_MEMBER(Controls, _controls);
}

void AudioControlCollection::Deserialize(DeserializeStream& stream, ISerializeModifier* modifier)
{
    DESERIALIZE_MEMBER(Controls, _controls);
}

// ============================================================================
//  AudioControlEntryDeserializer
// ============================================================================

namespace AmplitudeAudio
{
    AmplitudeTriggerData* AudioControlEntryDeserializer::DeserializeTriggerEntry(
        const AudioControlEntry& entry)
    {
        AmplitudeTriggerData* data = New<AmplitudeTriggerData>();
        data->EventID = static_cast<AmEventID>(entry.AmplitudeId);
        return data;
    }

    AmplitudeRtpcData* AudioControlEntryDeserializer::DeserializeRtpcEntry(
        const AudioControlEntry& entry)
    {
        AmplitudeRtpcData* data = New<AmplitudeRtpcData>();
        data->RtpcID = static_cast<AmRtpcID>(entry.AmplitudeId);
        return data;
    }

    AmplitudeSwitchStateData* AudioControlEntryDeserializer::DeserializeSwitchStateEntry(
        const AudioControlEntry& entry)
    {
        AmplitudeSwitchStateData* data = New<AmplitudeSwitchStateData>();
        data->SwitchID = static_cast<AmSwitchID>(entry.AmplitudeId);
        data->StateID = 0;
        return data;
    }

    AmplitudeEnvironmentData* AudioControlEntryDeserializer::DeserializeEnvironmentEntry(
        const AudioControlEntry& entry)
    {
        AmplitudeEnvironmentData* data = New<AmplitudeEnvironmentData>();
        data->EnvironmentID = static_cast<AmEnvironmentID>(entry.AmplitudeId);
        return data;
    }

    AmplitudeBankData* AudioControlEntryDeserializer::DeserializeBankEntry(
        const AudioControlEntry& entry)
    {
        AmplitudeBankData* data = New<AmplitudeBankData>();
        data->BankID = static_cast<AmBankID>(entry.AmplitudeId);
        data->BankFilename = entry.AmplitudeName;
        return data;
    }
} // namespace AmplitudeAudio
