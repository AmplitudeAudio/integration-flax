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

// clang-formatt off
#include "Cpp20Compat.h"
// clang-format on

#include <AudioSystem/Core/AudioSystemData.h>
#include <Engine/Core/Types/String.h>
#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/Core/Event.h>

namespace AmplitudeAudio
{
    using namespace SparkyStudios::Audio::Amplitude;
    // ========================================================================
    //  AmplitudeEntityData
    // ========================================================================

    /// <summary>Holds the Amplitude entity ID for a registered game entity.</summary>
    /// <remarks>
    /// <c>hasPosition</c> indicates whether the entity has a 3-D spatial position
    /// that the middleware should track each frame.
    /// </remarks>
    class AmplitudeEntityData final : public AudioSystemEntityData
    {
      public:
        AmplitudeEntityData();
        ~AmplitudeEntityData() override = default;

        /// <summary>Amplitude entity ID (0 = not registered).</summary>
        AmEntityID EntityID;

        /// <summary>Whether the entity contributes a 3-D position to the mixer.</summary>
        bool HasPosition;
    };

    // ========================================================================
    //  AmplitudeListenerData
    // ========================================================================

    /// <summary>
    /// Holds the Amplitude listener ID for a registered audio listener.
    /// </summary>
    class AmplitudeListenerData final : public AudioSystemListenerData
    {
      public:
        AmplitudeListenerData();
        ~AmplitudeListenerData() override = default;

        /// <summary>Amplitude listener ID (0 = not registered).</summary>
        AmListenerID ListenerID;

        /// <summary>Whether the listener is the default listener.</summary>
        bool IsDefault;
    };

    // ========================================================================
    //  AmplitudeTriggerData
    // ========================================================================

    /// <summary>Holds the Amplitude event ID that backs an audio trigger template.</summary>
    /// <remarks>
    /// A "trigger" in the AudioSystem corresponds to an Amplitude <c>Event</c> asset;
    /// this class stores the ID used to look up and fire that event at runtime.
    /// </remarks>
    class AmplitudeTriggerData final : public AudioSystemTriggerData
    {
      public:
        AmplitudeTriggerData();
        ~AmplitudeTriggerData() override = default;

        /// <summary>Amplitude event asset ID used when triggering this cue (0 = unset).</summary>
        AmEventID EventID;
    };

    // ========================================================================
    //  PlaybackState
    // ========================================================================

    /// <summary>Lifecycle state of a live Amplitude event instance.</summary>
    enum class PlaybackState : uint8_t
    {
        /// <summary>No event has been triggered yet (or it has fully completed).</summary>
        Idle = 0,

        /// <summary>The event instance is actively running.</summary>
        Playing = 1,

        /// <summary>A stop/cancel has been requested; the instance is winding down.</summary>
        Stopping = 2,
    };

    // ========================================================================
    //  AmplitudeEventData
    // ========================================================================

    /// <summary>Holds the live state of a triggered Amplitude event instance.</summary>
    /// <remarks>
    /// Created when an <c>AmplitudeTriggerData</c> is fired. Stores the event
    /// asset ID, the <c>EventCanceler</c> token needed to abort the instance, and
    /// the current playback state.
    /// </remarks>
    class AmplitudeEventData final : public AudioSystemEventData
    {
      public:
        AmplitudeEventData();
        ~AmplitudeEventData() override = default;

        /// <summary>Amplitude event asset ID that was triggered (0 = unset).</summary>
        AmEventID EventID;

        /// <summary>Token used to cancel/abort this running event instance.</summary>
        EventCanceler Canceler;

        /// <summary>Current lifecycle state of this event instance.</summary>
        PlaybackState State;
    };

    // ========================================================================
    //  AmplitudeRtpcData
    // ========================================================================

    /// <summary>Holds the Amplitude RTPC (Real-Time Parameter Control) ID.</summary>
    class AmplitudeRtpcData final : public AudioSystemRtpcData
    {
      public:
        AmplitudeRtpcData();
        ~AmplitudeRtpcData() override = default;

        /// <summary>Amplitude RTPC asset ID (0 = unset).</summary>
        AmRtpcID RtpcID;
    };

    // ========================================================================
    //  AmplitudeSwitchData
    // ========================================================================

    /// <summary>Holds the Amplitude switch container ID.</summary>
    class AmplitudeSwitchData final : public AudioSystemSwitchData
    {
      public:
        AmplitudeSwitchData();
        ~AmplitudeSwitchData() override = default;

        /// <summary>Amplitude switch asset ID (0 = unset).</summary>
        AmSwitchID SwitchID;
    };

    // ========================================================================
    //  AmplitudeSwitchStateData
    // ========================================================================

    /// <summary>Holds the Amplitude switch ID and the selected state object ID.</summary>
    /// <remarks>
    /// <c>SwitchID</c> identifies the switch container asset; <c>StateID</c> identifies
    /// the currently selected state within that container.
    /// </remarks>
    class AmplitudeSwitchStateData final : public AudioSystemSwitchStateData
    {
      public:
        AmplitudeSwitchStateData();
        ~AmplitudeSwitchStateData() override = default;

        /// <summary>Amplitude switch asset ID (0 = unset).</summary>
        AmSwitchID SwitchID;

        /// <summary>ID of the currently selected state within the switch (0 = unset).</summary>
        AmObjectID StateID;
    };

    // ========================================================================
    //  AmplitudeEnvironmentData
    // ========================================================================

    /// <summary>Holds the Amplitude environment (aux-bus / reverb zone) ID.</summary>
    class AmplitudeEnvironmentData final : public AudioSystemEnvironmentData
    {
      public:
        AmplitudeEnvironmentData();
        ~AmplitudeEnvironmentData() override = default;

        /// <summary>Amplitude environment asset ID (0 = unset).</summary>
        AmEnvironmentID EnvironmentID;
    };

    // ========================================================================
    //  AmplitudeBankData
    // ========================================================================

    /// <summary>Holds the Amplitude sound bank ID and its filename on disk.</summary>
    /// <remarks>
    /// <c>BankFilename</c> is the bare filename (including extension) of the
    /// \c .ambank file relative to the Amplitude asset directory, used when
    /// requesting a load or unload of the bank at runtime.
    /// </remarks>
    class AmplitudeBankData final : public AudioSystemBankData
    {
      public:
        AmplitudeBankData();
        ~AmplitudeBankData() override = default;

        /// <summary>Current runtime Amplitude bank handle assigned by middleware bank tracking (0 = unloaded).</summary>
        AmBankID BankID;

        /// <summary>Filename of the bank file (e.g. "init.ambank").</summary>
        String BankFilename;
    };

} // namespace AmplitudeAudio
