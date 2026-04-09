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
//  AmplitudeAudio — Data Wrapper Classes (implementation)
//
//  Only constructors are defined here; all destructor bodies are defaulted in
//  the header so that the compiler can inline them in the translation units
//  that actually destroy these objects.
// ============================================================================

#include "AmplitudeAudioData.h"

namespace AmplitudeAudio
{
    // ------------------------------------------------------------------------
    //  AmplitudeEntityData
    // ------------------------------------------------------------------------

    AmplitudeEntityData::AmplitudeEntityData()
        : EntityID(0), HasPosition(false)
    {
    }

    // ------------------------------------------------------------------------
    //  AmplitudeListenerData
    // ------------------------------------------------------------------------

    AmplitudeListenerData::AmplitudeListenerData()
        : ListenerID(0)
    {
    }

    // ------------------------------------------------------------------------
    //  AmplitudeTriggerData
    // ------------------------------------------------------------------------

    AmplitudeTriggerData::AmplitudeTriggerData()
        : EventID(0)
    {
    }

    // ------------------------------------------------------------------------
    //  AmplitudeEventData
    // ------------------------------------------------------------------------

    AmplitudeEventData::AmplitudeEventData()
        : EventID(0), Canceler(), State(PlaybackState::Idle)
    {
    }

    // ------------------------------------------------------------------------
    //  AmplitudeRtpcData
    // ------------------------------------------------------------------------

    AmplitudeRtpcData::AmplitudeRtpcData()
        : RtpcID(0)
    {
    }

    // ------------------------------------------------------------------------
    //  AmplitudeSwitchData
    // ------------------------------------------------------------------------

    AmplitudeSwitchData::AmplitudeSwitchData()
        : SwitchID(0)
    {
    }

    // ------------------------------------------------------------------------
    //  AmplitudeSwitchStateData
    // ------------------------------------------------------------------------

    AmplitudeSwitchStateData::AmplitudeSwitchStateData()
        : SwitchID(0), StateID(0)
    {
    }

    // ------------------------------------------------------------------------
    //  AmplitudeEnvironmentData
    // ------------------------------------------------------------------------

    AmplitudeEnvironmentData::AmplitudeEnvironmentData()
        : EnvironmentID(0)
    {
    }

    // ------------------------------------------------------------------------
    //  AmplitudeBankData
    // ------------------------------------------------------------------------

    AmplitudeBankData::AmplitudeBankData()
        : BankID(0), BankFilename(TEXT(""))
    {
    }

} // namespace AmplitudeAudio
