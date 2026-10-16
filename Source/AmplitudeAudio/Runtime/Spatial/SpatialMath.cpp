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
#include <Engine/Core/Math/Vector2.h>

namespace AmplitudeAudio::Spatial
{
    namespace
    {
        // Distance from p to the segment ab, in 2D.
        float SegmentDistance2D(const Vector2& p, const Vector2& a, const Vector2& b)
        {
            const Vector2 ab = b - a;
            const float lengthSquared = Vector2::Dot(ab, ab);
            const float t = lengthSquared > 0.0f ? Math::Saturate(Vector2::Dot(p - a, ab) / lengthSquared) : 0.0f;
            return Vector2::Distance(p, a + ab * t);
        }

        float Cross2D(const Vector2& a, const Vector2& b)
        {
            return a.X * b.Y - a.Y * b.X;
        }
    }

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

    ZoneShape ZoneShape::MakeCone(const Vector3& apex, const Quaternion& rotation, float baseRadius, float height)
    {
        ZoneShape zone;
        zone.Kind = ZoneKind::Cone;
        zone.Center = apex;
        zone.Rotation = rotation;
        zone.Extents = Vector3(Math::Max(baseRadius, 0.0f), Math::Max(height, 0.0f), 0.0f);
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
        case ZoneKind::Cone:
            {
                const float radius = zone.Extents.X;
                const float height = zone.Extents.Y;
                const Vector3 axis = zone.Rotation * Vector3::Forward;
                const Vector3 local = point - zone.Center;
                const float t = Vector3::Dot(local, axis);
                const float rho = (local - axis * t).Length();

                if (height <= 0.0f)
                {
                    // Flat disc at apex or point
                    const Vector2 p(t, rho), a(0.0f, 0.0f), b(0.0f, radius);
                    return SegmentDistance2D(p, a, b);
                }
                if (radius <= 0.0f)
                {
                    // Thin line from apex to base
                    const Vector2 p(t, rho), a(0.0f, 0.0f), b(height, 0.0f);
                    return SegmentDistance2D(p, a, b);
                }

                // Half cross-section: apex, base center, rim. Inside when it is on the same side of all three edges.
                const Vector2 p(t, rho), a(0.0f, 0.0f), b(height, 0.0f), c(height, radius);
                const float s1 = Cross2D(b - a, p - a);
                const float s2 = Cross2D(c - b, p - b);
                const float s3 = Cross2D(a - c, p - c);
                const bool hasNegative = s1 < 0.0f || s2 < 0.0f || s3 < 0.0f;
                const bool hasPositive = s1 > 0.0f || s2 > 0.0f || s3 > 0.0f;
                if (!(hasNegative && hasPositive))
                    return 0.0f;

                return Math::Min(SegmentDistance2D(p, a, b), Math::Min(SegmentDistance2D(p, b, c), SegmentDistance2D(p, c, a)));
            }
        }
        return 0.0f;
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
