// Copyright (c) 2026-present Sparky Studios. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <Engine/Core/Collections/Array.h>
#include <Engine/Core/Math/Vector2.h>
#include <Engine/Platform/Defines.h>

namespace AmplitudeAudio::Obstruction
{
    /// <summary>Transmission along one ray: 1 - Π(1 - oᵢ). Every extra surface can only add obstruction.</summary>
    AMPLITUDEAUDIO_API float CombineTransmission(const float* amounts, int32 count);

    /// <summary>1 up to fullDistance, falling linearly to 0 at maxDistance.</summary>
    AMPLITUDEAUDIO_API float DistanceScale(float distance, float fullDistance, float maxDistance);

    /// <summary>Moves current toward target with separate attack (rising) and release (falling) time constants. A time constant of 0 snaps.</summary>
    AMPLITUDEAUDIO_API float SmoothTo(float current, float target, float deltaSeconds, float attackSeconds, float releaseSeconds);

    /// <summary>Obstruction and occlusion amounts resolved by Classify.</summary>
    struct Classification
    {
        float Obstruction;
        float Occlusion;
    };

    /// <summary>Direct blocked and an indirect path open: obstruction. Everything blocked: occlusion. Single ray mode never occludes.</summary>
    AMPLITUDEAUDIO_API Classification Classify(float directAmount, bool anyIndirectOpen, bool multipleRays);

    /// <summary>Deterministic points in the unit disk (x, y), cycled by the processor.</summary>
    AMPLITUDEAUDIO_API Array<Float2> MakeDiskOffsets(int32 count, uint32 seed);

    constexpr float kBlockedEpsilon = 0.001f;
    constexpr float kIndirectOpenThreshold = 0.1f;
} // namespace AmplitudeAudio::Obstruction
