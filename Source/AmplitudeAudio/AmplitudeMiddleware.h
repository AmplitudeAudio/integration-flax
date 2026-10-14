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

// clang-format off
#include "Core/Cpp20Compat.h"
// clang-format on

#include <AudioSystem/Core/AudioMiddleware.h>
#include <Engine/Core/Collections/Dictionary.h>
#include <SparkyStudios/Audio/Amplitude/Core/Engine.h>

#include "AmplitudeSettings.h"

namespace AmplitudeAudio
{
    class FlaxFileSystem;

    /// <summary>Concrete AudioMiddleware backed by the Amplitude Audio SDK.</summary>
    /// <remarks>Lifecycle: LoadConfiguration -> Startup -> Update loop -> Shutdown -> Release.</remarks>
    class AmplitudeMiddleware final : public AudioMiddleware
    {
    public:
        AmplitudeMiddleware();
        ~AmplitudeMiddleware() override;

        bool LoadConfiguration(StringView configFile) override;
        bool Startup() override;
        void Update(float dt) override;
        void Shutdown() override;
        void Release() override;
        void StopAllSounds() override;

#if USE_EDITOR
        bool DeployFiles(const StringView& outputPath) override;
#endif

        AudioSystemEntityData* CreateWorldEntity(AudioSystemDataID id) override;
        AudioSystemEntityData* CreateEntityData(AudioSystemDataID id) override;
        void                   DestroyEntityData(AudioSystemEntityData* data) override;
        bool                   AddEntity(AudioSystemDataID id, AudioSystemEntityData* data) override;
        bool                   ResetEntity(AudioSystemDataID id, AudioSystemEntityData* data) override;
        bool                   UpdateEntity(AudioSystemDataID id, AudioSystemEntityData* data) override;
        bool                   RemoveEntity(AudioSystemDataID id, AudioSystemEntityData* data) override;
        bool                   SetEntityTransform(AudioSystemDataID id, AudioSystemEntityData* data, const AudioSystemTransform& transform) override;

        AudioSystemListenerData* CreateListenerData(AudioSystemDataID id, bool isDefault) override;
        void                     DestroyListenerData(AudioSystemListenerData* data) override;
        bool                     AddListener(AudioSystemDataID id, AudioSystemListenerData* data) override;
        bool                     ResetListener(AudioSystemDataID id, AudioSystemListenerData* data) override;
        bool                     RemoveListener(AudioSystemDataID id, AudioSystemListenerData* data) override;
        bool                     SetListenerTransform(AudioSystemDataID id, AudioSystemListenerData* data, const AudioSystemTransform& transform) override;

        AudioSystemEventData* CreateEventData(AudioSystemDataID id) override;
        void                  ResetEventData(AudioSystemEventData* data) override;
        void                  DestroyEventData(AudioSystemEventData* data) override;

        bool LoadTrigger(AudioSystemDataID entityId, AudioSystemTriggerData* triggerData) override;
        bool ActivateTrigger(AudioSystemDataID entityId, AudioSystemTriggerData* triggerData, AudioSystemEventData* eventData) override;
        bool ActivateTriggerMulti(AudioSystemDataID entityId, AudioSystemTriggerData* triggerData,
            const Array<TriggerLocationEntry>& locations, AudioSystemMultiPositionBehavior behavior) override;
        bool UnloadTrigger(AudioSystemDataID entityId, AudioSystemTriggerData* triggerData) override;
        bool StopEvent(AudioSystemDataID entityId, AudioSystemEventData* eventData) override;
        bool StopAllEvents(AudioSystemDataID entityId) override;

        bool SetRtpc(AudioSystemDataID entityId, AudioSystemRtpcData* data, float value) override;
        bool ResetRtpc(AudioSystemDataID entityId, AudioSystemRtpcData* data) override;

        bool SetSwitchState(AudioSystemDataID entityId, AudioSystemSwitchStateData* data) override;

        bool SetObstructionAndOcclusion(AudioSystemDataID entityId, AudioSystemEntityData* entityData, float obstruction, float occlusion) override;

        bool SetEnvironmentAmount(AudioSystemDataID entityId, AudioSystemEnvironmentData* envData, float amount) override;

        bool LoadBank(AudioSystemBankData* data) override;
        bool UnloadBank(AudioSystemBankData* data) override;
        void DestroyBank(AudioSystemBankData* data) override;

        void DestroyTriggerData(AudioSystemTriggerData* data) override;
        void DestroyRtpcData(AudioSystemRtpcData* data) override;
        void DestroySwitchData(AudioSystemSwitchData* data) override;
        void DestroySwitchStateData(AudioSystemSwitchStateData* data) override;
        void DestroyEnvironmentData(AudioSystemEnvironmentData* data) override;

        void       SetLanguage(StringView language) override;
        void       OnMasterGainChange(float gain) override;
        void       OnMuteChange(bool muted) override;
        void       OnPaused(bool paused) override;
        void       OnLoseFocus() override;
        void       OnGainFocus() override;
        StringView GetMiddlewareName() const override;
        float      GetMasterGain() const override;
        bool       GetMute() const override;

    private:
        struct LoadedBankState
        {
            SparkyStudios::Audio::Amplitude::AmBankID BankID   = 0;
            int32                                     RefCount = 0;
        };

        void CleanupRuntimeState(bool closeFileSystem, bool unregisterExtensions);
        void ResetRuntimeState();
        void ResetBankTracking();
        void ReleaseTrackedBankReference(AudioSystemBankData* data);
        void ReleaseTrackedBankReference(const String& filename);

        /// <summary>Amplitude Engine singleton (obtained via Engine::GetInstance()).</summary>
        SparkyStudios::Audio::Amplitude::Engine* m_pEngine;

        /// <summary>Custom file system bridging Flax I/O to Amplitude.</summary>
        std::shared_ptr<FlaxFileSystem> m_pFileSystem;

        /// <summary>Per-platform configuration profiles loaded from game settings.</summary>
        AmplitudeAssetProfiles m_profiles;

        /// <summary>Shared runtime load state keyed by sound-bank filename.</summary>
        Dictionary<String, LoadedBankState> m_loadedBanksByFilename;

        /// <summary>Wrapper-to-filename ownership map used to keep per-wrapper loads idempotent.</summary>
        Dictionary<const AudioSystemBankData*, String> m_loadedBankFilenames;

        /// <summary>Whether Startup() completed successfully.</summary>
        bool m_bInitialized;
    };
} // namespace AmplitudeAudio