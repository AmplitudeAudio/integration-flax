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
