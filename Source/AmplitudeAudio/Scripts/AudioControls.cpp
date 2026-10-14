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

#include "AudioControls.h"

#include <AudioSystem/Core/AudioSystem.h>
#include <Engine/Content/AssetReference.h>
#include <Engine/Content/Content.h>
#include <Engine/Content/JsonAsset.h>
#include <Engine/Core/Log.h>
#include <Engine/Core/Types/StringView.h>
#include <Engine/Serialization/Serialization.h>

#include "../Core/Common.h"
#include "../Resources/AudioControlCollectionResource.h"

// ============================================================================
//  AudioControls — Constructor
// ============================================================================

AudioControls::AudioControls(const SpawnParams& params)
    : Script(params)
{
}

// ============================================================================
//  AudioControls — Script lifecycle
// ============================================================================

void AudioControls::OnEnable()
{
    Script::OnEnable();

    if (AutoLoad)
        Load();
}

void AudioControls::OnDisable()
{
    if (_loaded)
        Unload();

    Script::OnDisable();
}

// ============================================================================
//  AudioControls — Load
// ============================================================================

void AudioControls::Load()
{
    using namespace AmplitudeAudio;

    if (_loaded)
    {
        LOG(Warning, "[AudioControls] Load() called but controls are already loaded. Path: {0}", ControlsAssetPath);
        return;
    }

    if (ControlsAssetPath.IsEmpty())
    {
        LOG(Warning, "[AudioControls] Load() called but ControlsAssetPath is empty. Nothing to load.");
        return;
    }

    // Load the JSON asset from the Flax content system. AssetReference keeps a
    // strong reference for the duration of the call so a concurrent unload
    // cannot free asset->Data under us.
    AssetReference<JsonAsset> asset = Content::Load<JsonAsset>(ControlsAssetPath);
    if (asset == nullptr)
    {
        LOG(Error, "[AudioControls] Failed to load controls asset: {0}", ControlsAssetPath);
        return;
    }

    if (asset->WaitForLoaded())
    {
        LOG(Error, "[AudioControls] Controls asset failed to load: {0}", ControlsAssetPath);
        return;
    }

    if (asset->Data == nullptr)
    {
        LOG(Error, "[AudioControls] Controls asset has no data: {0}", ControlsAssetPath);
        return;
    }

    // Deserialize the controls array directly from the JSON Data node,
    // avoiding the need to construct a ScriptingObject on the stack.
    Array<AudioControlEntry> controls;
    {
        auto&               stream   = *asset->Data;
        ISerializeModifier* modifier = AmplitudeAudio::GetActiveModifier(asset.Get());
        DESERIALIZE_MEMBER(Controls, controls);
    }

    if (controls.IsEmpty())
    {
        LOG(Warning, "[AudioControls] Controls asset is empty: {0}", ControlsAssetPath);
        return;
    }

    AudioSystem* audioSystem = AudioSystem::Get();
    if (audioSystem == nullptr || !audioSystem->IsInitialized())
    {
        LOG(Error,
            "[AudioControls] AudioSystem is not initialized — controls from '{0}' will NOT be registered. "
            "Ensure AmplitudeAudioPlugin::Initialize runs before this script's OnEnable.",
            ControlsAssetPath);
        return;
    }

    int32 registeredCount = 0;

    for (int32 i = 0; i < controls.Count(); ++i)
    {
        const AudioControlEntry& entry = controls[i];
        auto                     id    = static_cast<AudioSystemDataID>(entry.AmplitudeId);

        if (id == INVALID_AUDIO_SYSTEM_ID)
        {
            LOG(Warning, "[AudioControls] Skipping entry '{0}' with invalid ID.", entry.Name);
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
                    LOG(Error, "[AudioControls] Failed to register bank '{0}'.", entry.Name);
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
                    LOG(Error, "[AudioControls] Failed to register trigger '{0}'.", entry.Name);
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
                    LOG(Error, "[AudioControls] Failed to register RTPC '{0}'.", entry.Name);
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
                    LOG(Error, "[AudioControls] Failed to register switch state '{0}'.", entry.Name);
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
                    LOG(Error, "[AudioControls] Failed to register environment '{0}'.", entry.Name);
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
                    LOG(Error, "[AudioControls] Failed to register switch '{0}'.", entry.Name);
                    Delete(data);
                }
                break;
            }
            case AudioControlType::Invalid:
            default:
                LOG(Warning, "[AudioControls] Skipping entry '{0}' with unknown type {1}.", entry.Name, static_cast<int>(entry.Type));
                break;
        }
    }

    if (registeredCount > 0)
    {
        LOG(Info, "[AudioControls] Loaded {0} controls from: {1}", registeredCount, ControlsAssetPath);
        _loaded = true;
    }
    else
    {
        LOG(Warning, "[AudioControls] No controls were registered from: {0}", ControlsAssetPath);
    }
}

// ============================================================================
//  AudioControls — Unload
// ============================================================================

void AudioControls::Unload()
{
    if (!_loaded)
    {
        LOG(Warning, "[AudioControls] Unload() called but controls are not loaded.");
        return;
    }

    AudioSystem* audioSystem = AudioSystem::Get();

    if (audioSystem != nullptr && audioSystem->IsInitialized())
    {
        for (const auto triggerId : _registeredTriggerIds)
            audioSystem->UnregisterTrigger(triggerId);

        for (const auto rtpcId : _registeredRtpcIds)
            audioSystem->UnregisterRtpc(rtpcId);

        for (const auto envId : _registeredEnvironmentIds)
            audioSystem->UnregisterEnvironment(envId);

        // SwitchState entries must be unregistered BEFORE their parent Switch,
        // because the ATL composes switch-state identity from (switchName, stateName)
        // and freeing the parent switch first leaves child states pointing at a
        // released wrapper.
        for (const auto ssId : _registeredSwitchStateIds)
            audioSystem->UnregisterSwitchState(ssId);

        for (const auto switchId : _registeredSwitchIds)
            audioSystem->UnregisterSwitch(switchId);

        for (const auto bankId : _registeredBankIds)
            audioSystem->UnregisterSoundBank(bankId);
    }

    _registeredBankIds.Clear();
    _registeredTriggerIds.Clear();
    _registeredRtpcIds.Clear();
    _registeredSwitchIds.Clear();
    _registeredSwitchStateIds.Clear();
    _registeredEnvironmentIds.Clear();

    LOG(Info, "[AudioControls] Controls unloaded for: {0}", ControlsAssetPath);
    _loaded = false;
}

// ============================================================================
//  AudioControls — IsLoaded
// ============================================================================

bool AudioControls::IsLoaded() const
{
    return _loaded;
}
