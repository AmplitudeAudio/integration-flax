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

#include <Engine/Core/Math/Vector3.h>

#include <AudioSystem/Core/AudioSystemData.h>

#include <SparkyStudios/Audio/Amplitude/Math/LinearAlgebra.h>
#include <SparkyStudios/Audio/Amplitude/Math/Orientation.h>

namespace AmplitudeAudio
{
    using namespace SparkyStudios::Audio::Amplitude;

    /// <summary>
    /// Converts a Flax Vector3 to an Amplitude AmVector3.
    /// </summary>
    AmVector3 FlaxVec3ToAmVec3(const Vector3& vec);

    /// <summary>
    /// Converts a <see cref="AudioSystemTransform"/> to an Amplitude's location and orientation.
    /// </summary>
    /// <param name="transform">The audio system's spatial transform.</param>
    /// <param name="outLocation">The world-space position expressed as AmVector3.</param>
    /// <param name="outOrientation">The orientation built from the transform's forward and up unit vectors.</param>
    void FlaxTransformToAmplitude(const AudioSystemTransform& transform, AmVector3& outLocation, Orientation& outOrientation);

    /// <summary>
    /// Converts an AmOsString to a Flax String.
    /// </summary>
    /// <remarks>
    /// Flax's Char type can be char16_t (Unix) or wchar_t (Windows), so we
    /// cannot safely pass a wchar_t* to the Flax String constructor on all
    /// platforms.  We always route through StringAnsi: on wchar platforms the
    /// wide AmOsString (std::wstring) is narrowed first via the Amplitude SDK
    /// helper macro, then wrapped in StringAnsi for the Flax String constructor.
    /// </remarks>
    ::String AmOsStringToFlaxString(const AmOsString& path);
} // namespace AmplitudeAudio