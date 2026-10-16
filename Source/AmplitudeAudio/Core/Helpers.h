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
#include "Cpp20Compat.h"
// clang-format on

#include <string>

#include <Engine/Core/Collections/Array.h>
#include <Engine/Core/Math/Vector3.h>
#include <Engine/Core/Types/String.h>

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/Math/LinearAlgebra.h>
#include <SparkyStudios/Audio/Amplitude/Math/Orientation.h>

namespace AmplitudeAudio
{
    using namespace SparkyStudios::Audio::Amplitude;

    /// <summary>Converts a Flax Vector3 to an Amplitude AmVector3.</summary>
    AMPLITUDEAUDIO_API AmVector3 FlaxVec3ToAmVec3(const Vector3& vec);

    /// <summary>Converts a list of Amplitude lengths (meters) to Flax units.</summary>
    AMPLITUDEAUDIO_API Array<float> AmLengthsToFlaxUnits(const Array<float>& lengths);

    /// <summary>Converts Flax dimensions to Amplitude shape dimensions with axis mapping.</summary>
    AMPLITUDEAUDIO_API AmVector3 ToAmDimensions(const Vector3& dimensions);

    /// <summary>Converts an Amplitude AmVector3 to a Flax Vector3 (inverse of FlaxVec3ToAmVec3).</summary>
    AMPLITUDEAUDIO_API Vector3 AmVec3ToFlaxVec3(const AmVector3& vec);

    /// <summary>Converts seconds to the SDK's time unit (milliseconds).</summary>
    inline AmTime SecondsToAmTime(float seconds)
    {
        return static_cast<AmTime>(seconds) * 1000.0;
    }

    /// <summary>Builds an Amplitude orientation from Flax forward and up vectors.</summary>
    AMPLITUDEAUDIO_API Orientation FlaxOrientationToAm(const Vector3& forward, const Vector3& up);

    /// <summary>Converts a Flax string to a narrow std::string.</summary>
    std::string FlaxStringToStdString(const ::String& value);

    /// <summary>Converts a Flax string to an Amplitude OS string.</summary>
    AmOsString FlaxStringToAmOsString(const ::String& value);

    /// <summary>Converts an Amplitude OS string to a Flax string.</summary>
    /// <remarks>
    /// Flax's Char can be char16_t (Unix) or wchar_t (Windows), so the conversion always goes through StringAnsi.
    /// </remarks>
    ::String AmOsStringToFlaxString(const AmOsString& path);
} // namespace AmplitudeAudio
