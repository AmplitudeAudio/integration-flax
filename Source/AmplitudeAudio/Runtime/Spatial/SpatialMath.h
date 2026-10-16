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
    enum class ZoneKind : uint8
    {
        Box = 0,
        Sphere = 1,
        Capsule = 2,
        Cone = 3,
    };

    struct OrientedBox
    {
        Vector3 Center = Vector3::Zero;
        Quaternion Rotation = Quaternion::Identity;
        Vector3 HalfExtents = Vector3::Zero;

        float Volume() const { return 8.0f * HalfExtents.X * HalfExtents.Y * HalfExtents.Z; }
    };

    /// <summary>An environment zone in world space. The meaning of Extents depends on Kind.</summary>
    struct AMPLITUDEAUDIO_API ZoneShape
    {
        ZoneKind Kind = ZoneKind::Box;
        Vector3 Center = Vector3::Zero;            // Box, Sphere, Capsule: the center. Cone: the apex.
        Quaternion Rotation = Quaternion::Identity;
        Vector3 Extents = Vector3::Zero;           // Box: half extents. Sphere: X = radius.
                                                   // Capsule: X = radius, Y = half height (caps included).
                                                   // Cone: X = base radius, Y = height.

        static ZoneShape MakeBox(const Vector3& center, const Quaternion& rotation, const Vector3& halfExtents);
        static ZoneShape MakeSphere(const Vector3& center, float radius);
        static ZoneShape MakeCapsule(const Vector3& center, const Quaternion& rotation, float radius, float halfHeight);
        static ZoneShape MakeCone(const Vector3& apex, const Quaternion& rotation, float baseRadius, float height);
    };

    /// <summary>Returns the rotation to draw a wire capsule whose axis aligns with the actor's local Y axis.</summary>
    AMPLITUDEAUDIO_API Quaternion CapsuleDrawRotation(const Quaternion& actorRotation);

    /// <summary>Whether the point is inside or on the surface of the box.</summary>
    AMPLITUDEAUDIO_API bool Contains(const OrientedBox& box, const Vector3& point);

    /// <summary>Distance from the point to the box surface; 0 inside.</summary>
    AMPLITUDEAUDIO_API float DistanceTo(const OrientedBox& box, const Vector3& point);

    /// <summary>Distance from the point to the zone; 0 inside.</summary>
    AMPLITUDEAUDIO_API float DistanceTo(const ZoneShape& zone, const Vector3& point);

    /// <summary>1 inside, falling linearly to 0 at blendDistance outside the surface. A blendDistance of 0 is a hard edge.</summary>
    AMPLITUDEAUDIO_API float EnvironmentFactor(const OrientedBox& box, float blendDistance, const Vector3& point);

    /// <summary>1 inside, falling linearly to 0 at blendDistance outside the surface. A blendDistance of 0 is a hard edge.</summary>
    AMPLITUDEAUDIO_API float EnvironmentFactor(const ZoneShape& zone, float blendDistance, const Vector3& point);

    struct RoomCandidate
    {
        uint64 Id;
        OrientedBox Box;
    };

    /// <summary>Id of the smallest room containing the point, 0 when none does. Equal volumes keep the earlier candidate.</summary>
    AMPLITUDEAUDIO_API uint64 PickRoom(const RoomCandidate* rooms, int32 count, const Vector3& point);
} // namespace AmplitudeAudio::Spatial
