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

#include "AudioControlsScript.h"

#include <AudioSystem/Core/AudioSystem.h>
#include <Engine/Content/Content.h>
#include <Engine/Content/JsonAsset.h>
#include <Engine/Core/Log.h>
#include <Engine/Core/Types/StringView.h>
#include <Engine/Serialization/Serialization.h>

#include "AmplitudeAudio/Resources/AudioControlCollectionResource.h"

// ============================================================================
//  AudioControlsScript — Constructor
// ============================================================================

AudioControlsScript::AudioControlsScript(const SpawnParams& params)
    : Script(params)
{
}

// ============================================================================
//  AudioControlsScript — Script lifecycle
// ============================================================================

void AudioControlsScript::OnEnable()
{
    Script::OnEnable();

    if (AutoLoad)
        Load();
}

void AudioControlsScript::OnDisable()
{
    if (_loaded)
        Unload();

    Script::OnDisable();
}

// ============================================================================
//  AudioControlsScript — Load
// ============================================================================

void AudioControlsScript::Load()
{
    using namespace AmplitudeAudio;

    if (_loaded)
    {
        LOG(Warning, "[AudioControlsScript] Load() called but controls are already loaded. Path: {0}", ControlsAssetPath);
        return;
    }

    if (ControlsAssetPath.IsEmpty())
    {
        LOG(Warning, "[AudioControlsScript] Load() called but ControlsAssetPath is empty. Nothing to load.");
        return;
    }

    // Load the JSON asset from the Flax content system.
    const auto asset = Content::Load<JsonAsset>(ControlsAssetPath);
    if (asset == nullptr)
    {
        LOG(Error, "[AudioControlsScript] Failed to load controls asset: {0}", ControlsAssetPath);
        return;
    }

    if (asset->WaitForLoaded())
    {
        LOG(Error, "[AudioControlsScript] Controls asset failed to load: {0}", ControlsAssetPath);
        return;
    }

    if (asset->Data == nullptr)
    {
        LOG(Error, "[AudioControlsScript] Controls asset has no data: {0}", ControlsAssetPath);
        return;
    }

    // Deserialize the controls array directly from the JSON Data node,
    // avoiding the need to construct a ScriptingObject on the stack.
    Array<AudioControlEntry> controls;
    {
        auto&               stream   = *asset->Data;
        ISerializeModifier* modifier = nullptr;
        DESERIALIZE_MEMBER(Controls, controls);
    }

    if (controls.IsEmpty())
    {
        LOG(Warning, "[AudioControlsScript] Controls asset is empty: {0}", ControlsAssetPath);
        return;
    }

    AudioSystem* audioSystem = AudioSystem::Get();
    if (audioSystem == nullptr || !audioSystem->IsInitialized())
    {
        LOG(Error, "[AudioControlsScript] AudioSystem is not available or not initialized.");
        return;
    }

    int32 registeredCount = 0;

    for (int32 i = 0; i < controls.Count(); ++i)
    {
        const AudioControlEntry& entry = controls[i];
        auto                     id    = static_cast<AudioSystemDataID>(entry.AmplitudeId);

        // For sound banks, the numeric ID may be 0 if the bank is identified
        // only by filename. In that case, derive a stable ID from the name.
        // if (id == INVALID_AUDIO_SYSTEM_ID && entry.Type == AudioControlType::SoundBank
        //     && !entry.AmplitudeName.IsEmpty())
        // {
        //     // Use the entry name hash as a stable, non-zero registration ID.
        //     id = static_cast<AudioSystemDataID>(GetHash(entry.Name));
        //     if (id == INVALID_AUDIO_SYSTEM_ID)
        //         id = 1; // Ensure non-zero in the unlikely event of a zero hash.
        // }

        if (id == INVALID_AUDIO_SYSTEM_ID)
        {
            LOG(Warning, "[AudioControlsScript] Skipping entry '{0}' with invalid ID.", entry.Name);
            continue;
        }

        switch (entry.Type)
        {
            case AudioControlType::SoundBank:
            {
                AudioSystemBankData* data = AudioControlEntryDeserializer::DeserializeBankEntry(entry);
                if (audioSystem->RegisterSoundBank(id, entry.AmplitudeName, data))
                {
                    _registeredBankIds.Add(id);
                    ++registeredCount;
                }
                else
                {
                    LOG(Error, "[AudioControlsScript] Failed to register bank '{0}'.", entry.Name);
                    Delete(data);
                }
                break;
            }
            case AudioControlType::Trigger:
            {
                AudioSystemTriggerData* data = AudioControlEntryDeserializer::DeserializeTriggerEntry(entry);
                if (audioSystem->RegisterTrigger(id, entry.AmplitudeName, data))
                {
                    _registeredTriggerIds.Add(id);
                    ++registeredCount;
                }
                else
                {
                    LOG(Error, "[AudioControlsScript] Failed to register trigger '{0}'.", entry.Name);
                    Delete(data);
                }
                break;
            }
            case AudioControlType::Rtpc:
            {
                AudioSystemRtpcData* data = AudioControlEntryDeserializer::DeserializeRtpcEntry(entry);
                if (audioSystem->RegisterRtpc(id, entry.AmplitudeName, data))
                {
                    _registeredRtpcIds.Add(id);
                    ++registeredCount;
                }
                else
                {
                    LOG(Error, "[AudioControlsScript] Failed to register RTPC '{0}'.", entry.Name);
                    Delete(data);
                }
                break;
            }
            case AudioControlType::SwitchState:
            {
                AudioSystemSwitchStateData* data = AudioControlEntryDeserializer::DeserializeSwitchStateEntry(entry);
                if (data != nullptr && audioSystem->RegisterSwitchState(id, entry.ParentAmplitudeName, entry.AmplitudeName, data))
                {
                    _registeredSwitchStateIds.Add(id);
                    ++registeredCount;
                }
                else if (data != nullptr)
                {
                    LOG(Error, "[AudioControlsScript] Failed to register switch state '{0}'.", entry.Name);
                    Delete(data);
                }
                break;
            }
            case AudioControlType::Environment:
            {
                AudioSystemEnvironmentData* data = AudioControlEntryDeserializer::DeserializeEnvironmentEntry(entry);
                if (audioSystem->RegisterEnvironment(id, entry.AmplitudeName, data))
                {
                    _registeredEnvironmentIds.Add(id);
                    ++registeredCount;
                }
                else
                {
                    LOG(Error, "[AudioControlsScript] Failed to register environment '{0}'.", entry.Name);
                    Delete(data);
                }
                break;
            }
            case AudioControlType::Switch:
            {
                AudioSystemSwitchData* data = AudioControlEntryDeserializer::DeserializeSwitchEntry(entry);
                if (audioSystem->RegisterSwitch(id, entry.AmplitudeName, data))
                {
                    _registeredSwitchIds.Add(id);
                    ++registeredCount;
                }
                else
                {
                    LOG(Error, "[AudioControlsScript] Failed to register switch '{0}'.", entry.Name);
                    Delete(data);
                }
                break;
            }
            case AudioControlType::Invalid:
            default:
                LOG(Warning, "[AudioControlsScript] Skipping entry '{0}' with unknown type {1}.", entry.Name, static_cast<int>(entry.Type));
                break;
        }
    }

    if (registeredCount > 0)
    {
        LOG(Info, "[AudioControlsScript] Loaded {0} controls from: {1}", registeredCount, ControlsAssetPath);
        _loaded = true;
    }
    else
    {
        LOG(Warning, "[AudioControlsScript] No controls were registered from: {0}", ControlsAssetPath);
    }
}

// ============================================================================
//  AudioControlsComponent — Unload
// ============================================================================

void AudioControlsScript::Unload()
{
    if (!_loaded)
    {
        LOG(Warning, "[AudioControlsScript] Unload() called but controls are not loaded.");
        return;
    }

    AudioSystem* audioSystem = AudioSystem::Get();

    if (audioSystem != nullptr && audioSystem->IsInitialized())
    {
        for (const auto triggerId : _registeredTriggerIds)
            audioSystem->UnregisterTrigger(triggerId);

        for (const auto rtpcId : _registeredRtpcIds)
            audioSystem->UnregisterRtpc(rtpcId);

        for (const auto switchId : _registeredSwitchIds)
            audioSystem->UnregisterSwitch(switchId);

        for (const auto ssId : _registeredSwitchStateIds)
            audioSystem->UnregisterSwitchState(ssId);

        for (const auto envId : _registeredEnvironmentIds)
            audioSystem->UnregisterEnvironment(envId);

        for (const auto bankId : _registeredBankIds)
            audioSystem->UnregisterSoundBank(bankId);
    }

    // _registeredBankIds.Clear();
    _registeredTriggerIds.Clear();
    _registeredRtpcIds.Clear();
    _registeredSwitchIds.Clear();
    _registeredSwitchStateIds.Clear();
    _registeredEnvironmentIds.Clear();

    LOG(Info, "[AudioControlsScript] Controls unloaded for: {0}", ControlsAssetPath);
    _loaded = false;
}

// ============================================================================
//  AudioControlsScript — IsLoaded
// ============================================================================

bool AudioControlsScript::IsLoaded() const
{
    return _loaded;
}
