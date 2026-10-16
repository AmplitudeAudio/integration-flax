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

#include <Engine/Core/Math/Quaternion.h>
#include <Engine/Core/Math/Vector3.h>
#include <Engine/Core/Types/BaseTypes.h>
#include <Engine/Platform/Defines.h>

namespace AmplitudeAudio::Spatial
{
    struct OrientedBox
    {
        Vector3 Center = Vector3::Zero;
        Quaternion Rotation = Quaternion::Identity;
        Vector3 HalfExtents = Vector3::Zero;

        float Volume() const { return 8.0f * HalfExtents.X * HalfExtents.Y * HalfExtents.Z; }
    };

    /// <summary>Whether the point is inside or on the surface of the box.</summary>
    AMPLITUDEAUDIO_API bool Contains(const OrientedBox& box, const Vector3& point);

    /// <summary>Distance from the point to the box surface; 0 inside.</summary>
    AMPLITUDEAUDIO_API float DistanceTo(const OrientedBox& box, const Vector3& point);

    /// <summary>1 inside, falling linearly to 0 at blendDistance outside the surface. A blendDistance of 0 is a hard edge.</summary>
    AMPLITUDEAUDIO_API float EnvironmentFactor(const OrientedBox& box, float blendDistance, const Vector3& point);

    struct RoomCandidate
    {
        uint64 Id;
        OrientedBox Box;
    };

    /// <summary>Id of the smallest room containing the point, 0 when none does. Equal volumes keep the earlier candidate.</summary>
    AMPLITUDEAUDIO_API uint64 PickRoom(const RoomCandidate* rooms, int32 count, const Vector3& point);
} // namespace AmplitudeAudio::Spatial
