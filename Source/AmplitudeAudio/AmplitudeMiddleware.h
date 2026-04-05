#pragma once

// ============================================================================
//  AmplitudeMiddleware — Core AudioMiddleware implementation
//
//  Implements the full AudioMiddleware interface by delegating to the
//  Amplitude Audio SDK Engine singleton.  All 48 pure-virtual methods are
//  overridden here.
// ============================================================================

#include "Core/Cpp20Compat.h"

#include <AudioSystem/Core/AudioMiddleware.h>
#include <Engine/Core/Collections/Dictionary.h>
#include <SparkyStudios/Audio/Amplitude/Core/Engine.h>

#include "AmplitudeConfiguration.h"

namespace AmplitudeAudio
{
    class AmplitudeFileSystem;

    /// \brief Concrete AudioMiddleware backed by the Amplitude Audio SDK.
    ///
    /// Lifecycle: LoadConfiguration -> Startup -> Update loop -> Shutdown -> Release.
    class AmplitudeMiddleware final : public AudioMiddleware
    {
      public:
        AmplitudeMiddleware();
        ~AmplitudeMiddleware() override;

        // ====================================================================
        //  Lifecycle (6)
        // ====================================================================

        bool LoadConfiguration(StringView configFile) override;
        bool Startup() override;
        void Update(float dt) override;
        void Shutdown() override;
        void Release() override;
        void StopAllSounds() override;

#if USE_EDITOR
        bool DeployFiles(const StringView& outputPath) override;
#endif

        // ====================================================================
        //  Entity (8)
        // ====================================================================

        AudioSystemEntityData* CreateWorldEntity(AudioSystemDataID id) override;
        AudioSystemEntityData* CreateEntityData(AudioSystemDataID id) override;
        void                   DestroyEntityData(AudioSystemEntityData* data) override;
        bool                   AddEntity(AudioSystemDataID id, AudioSystemEntityData* data) override;
        bool                   ResetEntity(AudioSystemDataID id, AudioSystemEntityData* data) override;
        bool                   UpdateEntity(AudioSystemDataID id, AudioSystemEntityData* data) override;
        bool                   RemoveEntity(AudioSystemDataID id, AudioSystemEntityData* data) override;
        bool                   SetEntityTransform(AudioSystemDataID id, AudioSystemEntityData* data, const AudioSystemTransform& transform) override;

        // ====================================================================
        //  Listener (6)
        // ====================================================================

        AudioSystemListenerData* CreateListenerData(AudioSystemDataID id, bool isDefault) override;
        void                     DestroyListenerData(AudioSystemListenerData* data) override;
        bool                     AddListener(AudioSystemDataID id, AudioSystemListenerData* data) override;
        bool                     ResetListener(AudioSystemDataID id, AudioSystemListenerData* data) override;
        bool                     RemoveListener(AudioSystemDataID id, AudioSystemListenerData* data) override;
        bool                     SetListenerTransform(AudioSystemDataID id, AudioSystemListenerData* data, const AudioSystemTransform& transform) override;

        // ====================================================================
        //  Event (3)
        // ====================================================================

        AudioSystemEventData* CreateEventData(AudioSystemDataID id) override;
        void                  ResetEventData(AudioSystemEventData* data) override;
        void                  DestroyEventData(AudioSystemEventData* data) override;

        // ====================================================================
        //  Trigger (5)
        // ====================================================================

        bool LoadTrigger(AudioSystemDataID entityId, AudioSystemTriggerData* triggerData, AudioSystemEventData* eventData) override;
        bool ActivateTrigger(AudioSystemDataID entityId, AudioSystemTriggerData* triggerData, AudioSystemEventData* eventData) override;
        bool UnloadTrigger(AudioSystemDataID entityId, AudioSystemTriggerData* triggerData, AudioSystemEventData* eventData) override;
        bool StopEvent(AudioSystemDataID entityId, AudioSystemEventData* eventData) override;
        bool StopAllEvents(AudioSystemDataID entityId) override;

        // ====================================================================
        //  RTPC (2)
        // ====================================================================

        bool SetRtpc(AudioSystemDataID entityId, AudioSystemRtpcData* data, float value) override;
        bool ResetRtpc(AudioSystemDataID entityId, AudioSystemRtpcData* data) override;

        // ====================================================================
        //  Switch (1)
        // ====================================================================

        bool SetSwitchState(AudioSystemDataID entityId, AudioSystemSwitchStateData* data) override;

        // ====================================================================
        //  Obstruction / Occlusion (1)
        // ====================================================================

        bool SetObstructionAndOcclusion(AudioSystemDataID entityId, AudioSystemEntityData* entityData, float obstruction, float occlusion) override;

        // ====================================================================
        //  Environment (1)
        // ====================================================================

        bool SetEnvironmentAmount(AudioSystemDataID entityId, AudioSystemEnvironmentData* envData, float amount) override;

        // ====================================================================
        //  Bank (3)
        // ====================================================================

        bool LoadBank(AudioSystemBankData* data) override;
        bool UnloadBank(AudioSystemBankData* data) override;
        void DestroyBank(AudioSystemBankData* data) override;

        // ====================================================================
        //  Control Data Destroy (4)
        // ====================================================================

        void DestroyTriggerData(AudioSystemTriggerData* data) override;
        void DestroyRtpcData(AudioSystemRtpcData* data) override;
        void DestroySwitchStateData(AudioSystemSwitchStateData* data) override;
        void DestroyEnvironmentData(AudioSystemEnvironmentData* data) override;

        // ====================================================================
        //  Global (8)
        // ====================================================================

        void       SetLanguage(StringView language) override;
        void       OnMasterGainChange(float gain) override;
        void       OnMuteChange(bool muted) override;
        void       OnLoseFocus() override;
        void       OnGainFocus() override;
        StringView GetMiddlewareName() const override;
        float      GetMasterGain() const override;
        bool       GetMute() const override;

      private:
        struct LoadedBankState
        {
            SparkyStudios::Audio::Amplitude::AmBankID BankID = 0;
            int32 RefCount = 0;
        };

        void CleanupRuntimeState(bool closeFileSystem, bool unregisterExtensions);
        void ResetRuntimeState();
        void ResetBankTracking();
        void ReleaseTrackedBankReference(AudioSystemBankData* data);
        void ReleaseTrackedBankReference(const String& filename);

        /// Amplitude Engine singleton (obtained via Engine::GetInstance()).
        SparkyStudios::Audio::Amplitude::Engine* m_pEngine;

        /// Custom file system bridging Flax I/O to Amplitude.
        std::shared_ptr<AmplitudeFileSystem> m_pFileSystem;

        /// Per-platform configuration profiles loaded from JSON.
        AmplitudeAssetProfiles m_profiles;

        /// Shared runtime load state keyed by sound-bank filename.
        Dictionary<String, LoadedBankState> m_loadedBanksByFilename;

        /// Wrapper-to-filename ownership map used to keep per-wrapper loads idempotent.
        Dictionary<const AudioSystemBankData*, String> m_loadedBankFilenames;

        /// Whether Startup() completed successfully.
        bool m_bInitialized;
    };

} // namespace AmplitudeAudio
