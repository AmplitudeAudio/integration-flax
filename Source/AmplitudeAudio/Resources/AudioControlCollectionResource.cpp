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

// ============================================================================
//  AudioControlCollectionResource — implementation
// ============================================================================

#include "AudioControlCollectionResource.h"

#include <Engine/Core/Log.h>
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
        data->EventID              = static_cast<AmEventID>(entry.AmplitudeId);
        return data;
    }

    AmplitudeRtpcData* AudioControlEntryDeserializer::DeserializeRtpcEntry(
        const AudioControlEntry& entry)
    {
        AmplitudeRtpcData* data = New<AmplitudeRtpcData>();
        data->RtpcID            = static_cast<AmRtpcID>(entry.AmplitudeId);
        return data;
    }

    AmplitudeSwitchData* AudioControlEntryDeserializer::DeserializeSwitchEntry(
        const AudioControlEntry& entry)
    {
        AmplitudeSwitchData* data = New<AmplitudeSwitchData>();
        data->SwitchID            = static_cast<AmSwitchID>(entry.AmplitudeId);
        return data;
    }

    AmplitudeSwitchStateData* AudioControlEntryDeserializer::DeserializeSwitchStateEntry(
        const AudioControlEntry& entry)
    {
        if (entry.ParentAmplitudeId == 0)
        {
            LOG(Warning,
                "[AudioControlEntryDeserializer] SwitchState entry '{0}' is missing parent switch metadata. Regenerate controls.",
                entry.Name);
            return nullptr;
        }

        AmplitudeSwitchStateData* data = New<AmplitudeSwitchStateData>();
        data->SwitchID                 = static_cast<AmSwitchID>(entry.ParentAmplitudeId);
        data->StateID                  = static_cast<AmObjectID>(entry.AmplitudeId);
        return data;
    }

    AmplitudeEnvironmentData* AudioControlEntryDeserializer::DeserializeEnvironmentEntry(
        const AudioControlEntry& entry)
    {
        AmplitudeEnvironmentData* data = New<AmplitudeEnvironmentData>();
        data->EnvironmentID            = static_cast<AmEnvironmentID>(entry.AmplitudeId);
        return data;
    }

    AmplitudeBankData* AudioControlEntryDeserializer::DeserializeBankEntry(
        const AudioControlEntry& entry)
    {
        AmplitudeBankData* data = New<AmplitudeBankData>();
        data->BankID            = static_cast<AmBankID>(entry.AmplitudeId);
        data->BankFilename      = entry.AmplitudeName + TEXT(".ambank");
        return data;
    }
} // namespace AmplitudeAudio
