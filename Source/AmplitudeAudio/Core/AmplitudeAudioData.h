#pragma once

// ============================================================================
//  AmplitudeAudio — Data Wrapper Classes
//
//  Concrete implementations of the AudioSystem opaque base data classes.
//  Each class stores the Amplitude SDK handle or ID that identifies the
//  corresponding audio object on the middleware side.
//
//  These are plain data holders: no logic lives here beyond default
//  construction and destruction.
// ============================================================================

#include <Engine/Core/Types/String.h>

#include <AudioSystem/Core/AudioSystemData.h>

#include "Cpp20Compat.h"

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/Core/Event.h>

namespace AmplitudeAudio
{
    using namespace SparkyStudios::Audio::Amplitude;
    // ========================================================================
    //  AmplitudeEntityData
    // ========================================================================

    /// \brief Holds the Amplitude entity ID for a registered game entity.
    ///
    /// \c hasPosition indicates whether the entity has a 3-D spatial position
    /// that the middleware should track each frame.
    class AmplitudeEntityData final : public AudioSystemEntityData
    {
    public:
        AmplitudeEntityData();
        ~AmplitudeEntityData() override = default;

        /// Amplitude entity ID (0 = not registered).
        AmEntityID EntityID;

        /// Whether the entity contributes a 3-D position to the mixer.
        bool HasPosition;
    };

    // ========================================================================
    //  AmplitudeListenerData
    // ========================================================================

    /// \brief Holds the Amplitude listener ID for a registered audio listener.
    class AmplitudeListenerData final : public AudioSystemListenerData
    {
    public:
        AmplitudeListenerData();
        ~AmplitudeListenerData() override = default;

        /// Amplitude listener ID (0 = not registered).
        AmListenerID ListenerID;

        /// Whether the listener is the default listener.
        bool IsDefault;
    };

    // ========================================================================
    //  AmplitudeTriggerData
    // ========================================================================

    /// \brief Holds the Amplitude event ID that backs an audio trigger template.
    ///
    /// A "trigger" in the AudioSystem corresponds to an Amplitude \c Event asset;
    /// this class stores the ID used to look up and fire that event at runtime.
    class AmplitudeTriggerData final : public AudioSystemTriggerData
    {
    public:
        AmplitudeTriggerData();
        ~AmplitudeTriggerData() override = default;

        /// Amplitude event asset ID used when triggering this cue (0 = unset).
        AmEventID EventID;
    };

    // ========================================================================
    //  PlaybackState
    // ========================================================================

    /// \brief Lifecycle state of a live Amplitude event instance.
    enum class PlaybackState : uint8_t
    {
        /// No event has been triggered yet (or it has fully completed).
        Idle = 0,

        /// The event instance is actively running.
        Playing = 1,

        /// A stop/cancel has been requested; the instance is winding down.
        Stopping = 2,
    };

    // ========================================================================
    //  AmplitudeEventData
    // ========================================================================

    /// \brief Holds the live state of a triggered Amplitude event instance.
    ///
    /// Created when an \c AmplitudeTriggerData is fired. Stores the event
    /// asset ID, the \c EventCanceler token needed to abort the instance, and
    /// the current playback state.
    class AmplitudeEventData final : public AudioSystemEventData
    {
    public:
        AmplitudeEventData();
        ~AmplitudeEventData() override = default;

        /// Amplitude event asset ID that was triggered (0 = unset).
        AmEventID EventID;

        /// Token used to cancel/abort this running event instance.
        EventCanceler Canceler;

        /// Current lifecycle state of this event instance.
        PlaybackState State;
    };

    // ========================================================================
    //  AmplitudeRtpcData
    // ========================================================================

    /// \brief Holds the Amplitude RTPC (Real-Time Parameter Control) ID.
    class AmplitudeRtpcData final : public AudioSystemRtpcData
    {
    public:
        AmplitudeRtpcData();
        ~AmplitudeRtpcData() override = default;

        /// Amplitude RTPC asset ID (0 = unset).
        AmRtpcID RtpcID;
    };

    // ========================================================================
    //  AmplitudeSwitchData
    // ========================================================================

    /// \brief Holds the Amplitude switch container ID.
    class AmplitudeSwitchData final : public AudioSystemSwitchData
    {
    public:
        AmplitudeSwitchData();
        ~AmplitudeSwitchData() override = default;

        /// Amplitude switch asset ID (0 = unset).
        AmSwitchID SwitchID;
    };

    // ========================================================================
    //  AmplitudeSwitchStateData
    // ========================================================================

    /// \brief Holds the Amplitude switch ID and the selected state object ID.
    ///
    /// \c SwitchID identifies the switch container asset; \c StateID identifies
    /// the currently selected state within that container.
    class AmplitudeSwitchStateData final : public AudioSystemSwitchStateData
    {
    public:
        AmplitudeSwitchStateData();
        ~AmplitudeSwitchStateData() override = default;

        /// Amplitude switch asset ID (0 = unset).
        AmSwitchID SwitchID;

        /// ID of the currently selected state within the switch (0 = unset).
        AmObjectID StateID;
    };

    // ========================================================================
    //  AmplitudeEnvironmentData
    // ========================================================================

    /// \brief Holds the Amplitude environment (aux-bus / reverb zone) ID.
    class AmplitudeEnvironmentData final : public AudioSystemEnvironmentData
    {
    public:
        AmplitudeEnvironmentData();
        ~AmplitudeEnvironmentData() override = default;

        /// Amplitude environment asset ID (0 = unset).
        AmEnvironmentID EnvironmentID;
    };

    // ========================================================================
    //  AmplitudeBankData
    // ========================================================================

    /// \brief Holds the Amplitude sound bank ID and its filename on disk.
    ///
    /// \c BankFilename is the bare filename (including extension) of the
    /// \c .ambank file relative to the Amplitude asset directory, used when
    /// requesting a load or unload of the bank at runtime.
    class AmplitudeBankData final : public AudioSystemBankData
    {
    public:
        AmplitudeBankData();
        ~AmplitudeBankData() override = default;

        /// Current runtime Amplitude bank handle assigned by middleware bank tracking (0 = unloaded).
        AmBankID BankID;

        /// Filename of the bank file (e.g. "init.ambank").
        String BankFilename;
    };

} // namespace AmplitudeAudio
