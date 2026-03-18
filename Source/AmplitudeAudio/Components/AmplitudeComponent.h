#pragma once

// ============================================================================
//  AmplitudeComponent — Abstract base for all Amplitude Audio components
//
//  Provides editor categorization under "Sound/Amplitude" for all
//  Amplitude-specific script components. Contains no logic of its own.
// ============================================================================

#include <Engine/Scripting/Script.h>

/// \brief Abstract base class for all Amplitude Audio script components.
///
/// Inheriting from this class places the component under the
/// "Sound/Amplitude" category in the Flax editor's Add Script menu.
API_CLASS(Attributes="Category(\"Sound/Amplitude\")")
class AMPLITUDEAUDIO_API AmplitudeComponent : public Script
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCRIPTING_TYPE(AmplitudeComponent);
};
