#include <Engine/Content/Content.h>
#include <Engine/Content/JsonAsset.h>
#include <Engine/Core/Log.h>
#include <Engine/Core/Types/StringView.h>
#include <Engine/Serialization/Serialization.h>

#include <AudioSystem/Core/AudioSystem.h>

#include "AudioControlsComponent.h"
#include "AmplitudeAudio/Resources/AudioControlCollectionResource.h"

// ============================================================================
//  AudioControlsComponent — Constructor
// ============================================================================

AudioControlsComponent::AudioControlsComponent(const SpawnParams& params)
    : Script(params)
{
}

// ============================================================================
//  AudioControlsComponent — Script lifecycle
// ============================================================================

void AudioControlsComponent::OnEnable()
{
    Script::OnEnable();

    if (AutoLoad)
        Load();
}

void AudioControlsComponent::OnDisable()
{
    if (_loaded)
        Unload();

    Script::OnDisable();
}

// ============================================================================
//  AudioControlsComponent — Load
// ============================================================================

void AudioControlsComponent::Load()
{
    using namespace AmplitudeAudio;

    if (_loaded)
    {
        LOG(Warning, "[AudioControlsComponent] Load() called but controls are already loaded. Path: {0}", ControlsAssetPath);
        return;
    }

    if (ControlsAssetPath.IsEmpty())
    {
        LOG(Warning, "[AudioControlsComponent] Load() called but ControlsAssetPath is empty. Nothing to load.");
        return;
    }

    // Load the JSON asset from the Flax content system.
    const auto asset = Content::Load<JsonAsset>(ControlsAssetPath);
    if (asset == nullptr)
    {
        LOG(Error, "[AudioControlsComponent] Failed to load controls asset: {0}", ControlsAssetPath);
        return;
    }

    if (asset->WaitForLoaded())
    {
        LOG(Error, "[AudioControlsComponent] Controls asset failed to load: {0}", ControlsAssetPath);
        return;
    }

    if (asset->Data == nullptr)
    {
        LOG(Error, "[AudioControlsComponent] Controls asset has no data: {0}", ControlsAssetPath);
        return;
    }

    // Deserialize the controls array directly from the JSON Data node,
    // avoiding the need to construct a ScriptingObject on the stack.
    Array<AudioControlEntry> controls;
    {
        auto& stream = *asset->Data;
        ISerializeModifier* modifier = nullptr;
        DESERIALIZE_MEMBER(Controls, controls);
    }

    if (controls.IsEmpty())
    {
        LOG(Warning, "[AudioControlsComponent] Controls asset is empty: {0}", ControlsAssetPath);
        return;
    }

    AudioSystem* audioSystem = AudioSystem::Get();
    if (audioSystem == nullptr || !audioSystem->IsInitialized())
    {
        LOG(Error, "[AudioControlsComponent] AudioSystem is not available or not initialized.");
        return;
    }

    int32 registeredCount = 0;

    for (int32 i = 0; i < controls.Count(); ++i)
    {
        const AudioControlEntry& entry = controls[i];
        auto id = static_cast<AudioSystemDataID>(entry.AmplitudeId);

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
            LOG(Warning, "[AudioControlsComponent] Skipping entry '{0}' with invalid ID.", entry.Name);
            continue;
        }

        switch (entry.Type)
        {
        // case AudioControlType::SoundBank:
        // {
        //     AudioSystemBankData* data = AudioControlEntryDeserializer::DeserializeBankEntry(entry);
        //     if (audioSystem->RegisterSoundBank(id, data))
        //     {
        //         _registeredBankIds.Add(id);

        //         // Banks must be loaded after registration.
        //         AudioRequest req;
        //         req.Type = AudioRequestType::LoadBank;
        //         req.ObjectId = id;
        //         audioSystem->SendRequest(MoveTemp(req));
        //         ++registeredCount;
        //     }
        //     else
        //     {
        //         LOG(Error, "[AudioControlsComponent] Failed to register bank '{0}'.", entry.Name);
        //         Delete(data);
        //     }
        //     break;
        // }
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
                LOG(Error, "[AudioControlsComponent] Failed to register trigger '{0}'.", entry.Name);
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
                LOG(Error, "[AudioControlsComponent] Failed to register RTPC '{0}'.", entry.Name);
                Delete(data);
            }
            break;
        }
        case AudioControlType::SwitchState:
        {
            AudioSystemSwitchStateData* data = AudioControlEntryDeserializer::DeserializeSwitchStateEntry(entry);
            if (data != nullptr && audioSystem->RegisterSwitchState(id, entry.Name, data))
            {
                _registeredSwitchStateIds.Add(id);
                ++registeredCount;
            }
            else if (data != nullptr)
            {
                LOG(Error, "[AudioControlsComponent] Failed to register switch state '{0}'.", entry.Name);
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
                LOG(Error, "[AudioControlsComponent] Failed to register environment '{0}'.", entry.Name);
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
                LOG(Error, "[AudioControlsComponent] Failed to register switch '{0}'.", entry.Name);
                Delete(data);
            }
            break;
        }
        case AudioControlType::Invalid:
        default:
            LOG(Warning, "[AudioControlsComponent] Skipping entry '{0}' with unknown type {1}.", entry.Name, static_cast<int>(entry.Type));
            break;
        }
    }

    if (registeredCount > 0)
    {
        LOG(Info, "[AudioControlsComponent] Loaded {0} controls from: {1}", registeredCount, ControlsAssetPath);
        _loaded = true;
    }
    else
    {
        LOG(Warning, "[AudioControlsComponent] No controls were registered from: {0}", ControlsAssetPath);
    }
}

// ============================================================================
//  AudioControlsComponent — Unload
// ============================================================================

void AudioControlsComponent::Unload()
{
    if (!_loaded)
    {
        LOG(Warning, "[AudioControlsComponent] Unload() called but controls are not loaded.");
        return;
    }

    AudioSystem* audioSystem = AudioSystem::Get();

    if (audioSystem != nullptr && audioSystem->IsInitialized())
    {
        // // Unload banks first (request unload before unregistering).
        // for (const auto bankId : _registeredBankIds)
        // {
        //     AudioRequest req;
        //     req.Type = AudioRequestType::UnloadBank;
        //     req.ObjectId = bankId;
        //     audioSystem->SendRequestSync(MoveTemp(req));
        //     audioSystem->UnregisterSoundBank(bankId);
        // }

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
    }

    // _registeredBankIds.Clear();
    _registeredTriggerIds.Clear();
    _registeredRtpcIds.Clear();
    _registeredSwitchIds.Clear();
    _registeredSwitchStateIds.Clear();
    _registeredEnvironmentIds.Clear();

    LOG(Info, "[AudioControlsComponent] Controls unloaded for: {0}", ControlsAssetPath);
    _loaded = false;
}

// ============================================================================
//  AudioControlsComponent — IsLoaded
// ============================================================================

bool AudioControlsComponent::IsLoaded() const
{
    return _loaded;
}
