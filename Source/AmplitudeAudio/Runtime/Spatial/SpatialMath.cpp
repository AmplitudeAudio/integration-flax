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

    float EnvironmentFactor(const OrientedBox& box, float blendDistance, const Vector3& point)
    {
        const float distance = DistanceTo(box, point);
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
