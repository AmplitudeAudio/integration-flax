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

#include "SpatialMath.h"

#include <Engine/Core/Math/Math.h>

namespace AmplitudeAudio::Spatial
{
    ZoneShape ZoneShape::MakeBox(const Vector3& center, const Quaternion& rotation, const Vector3& halfExtents)
    {
        ZoneShape zone;
        zone.Kind = ZoneKind::Box;
        zone.Center = center;
        zone.Rotation = rotation;
        zone.Extents = halfExtents;
        return zone;
    }

    ZoneShape ZoneShape::MakeSphere(const Vector3& center, float radius)
    {
        ZoneShape zone;
        zone.Kind = ZoneKind::Sphere;
        zone.Center = center;
        zone.Extents = Vector3(Math::Max(radius, 0.0f), 0.0f, 0.0f);
        return zone;
    }

    ZoneShape ZoneShape::MakeCapsule(const Vector3& center, const Quaternion& rotation, float radius, float halfHeight)
    {
        ZoneShape zone;
        zone.Kind = ZoneKind::Capsule;
        zone.Center = center;
        zone.Rotation = rotation;
        zone.Extents = Vector3(Math::Max(radius, 0.0f), Math::Max(halfHeight, 0.0f), 0.0f);
        return zone;
    }

    Quaternion CapsuleDrawRotation(const Quaternion& actorRotation)
    {
        return actorRotation * Quaternion::Euler(-90.0f, 0.0f, 0.0f);
    }

    bool Contains(const OrientedBox& box, const Vector3& point)
    {
        const Vector3 local = Quaternion::Invert(box.Rotation) * (point - box.Center);
        return Math::Abs(local.X) <= box.HalfExtents.X && Math::Abs(local.Y) <= box.HalfExtents.Y && Math::Abs(local.Z) <= box.HalfExtents.Z;
    }

    float DistanceTo(const OrientedBox& box, const Vector3& point)
    {
        const Vector3 local = Quaternion::Invert(box.Rotation) * (point - box.Center);
        const Vector3 outside(Math::Max(Math::Abs(local.X) - box.HalfExtents.X, 0.0f),
                              Math::Max(Math::Abs(local.Y) - box.HalfExtents.Y, 0.0f),
                              Math::Max(Math::Abs(local.Z) - box.HalfExtents.Z, 0.0f));
        return outside.Length();
    }

    float DistanceTo(const ZoneShape& zone, const Vector3& point)
    {
        switch (zone.Kind)
        {
        case ZoneKind::Box:
            return DistanceTo(OrientedBox{ zone.Center, zone.Rotation, zone.Extents }, point);
        case ZoneKind::Sphere:
            return Math::Max(Vector3::Distance(point, zone.Center) - zone.Extents.X, 0.0f);
        case ZoneKind::Capsule:
            {
                const float radius = zone.Extents.X;
                const float halfSegment = Math::Max(zone.Extents.Y - radius, 0.0f);
                const Vector3 axis = zone.Rotation * Vector3::Up;
                const Vector3 local = point - zone.Center;
                const float along = Math::Clamp(Vector3::Dot(local, axis), -halfSegment, halfSegment);
                return Math::Max(Vector3::Distance(local, axis * along) - radius, 0.0f);
            }
        default:
            return 0.0f; // Cone arrives in Task 8; until then it is everywhere.
        }
    }

    float EnvironmentFactor(const OrientedBox& box, float blendDistance, const Vector3& point)
    {
        return EnvironmentFactor(ZoneShape::MakeBox(box.Center, box.Rotation, box.HalfExtents), blendDistance, point);
    }

    float EnvironmentFactor(const ZoneShape& zone, float blendDistance, const Vector3& point)
    {
        const float distance = DistanceTo(zone, point);
        if (distance <= 0.0f)
            return 1.0f;
        if (blendDistance <= 0.0f)
            return 0.0f;
        return Math::Saturate(1.0f - distance / blendDistance);
    }

    uint64 PickRoom(const RoomCandidate* rooms, int32 count, const Vector3& point)
    {
        uint64 best = 0;
        float bestVolume = MAX_float;
        for (int32 i = 0; i < count; i++)
        {
            if (!Contains(rooms[i].Box, point))
                continue;
            const float volume = rooms[i].Box.Volume();
            if (volume < bestVolume)
            {
                bestVolume = volume;
                best = rooms[i].Id;
            }
        }
        return best;
    }
} // namespace AmplitudeAudio::Spatial
