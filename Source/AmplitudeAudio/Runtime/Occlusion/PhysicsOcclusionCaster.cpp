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

#include "PhysicsOcclusionCaster.h"

#include <AmplitudeAudio/Runtime/Occlusion/ObstructionMath.h>
#include <AmplitudeAudio/Scripts/AmplitudeObstruction.h>
#include <Engine/Core/Math/Math.h>
#include <Engine/Physics/Colliders/Collider.h>
#include <Engine/Physics/Physics.h>

namespace AmplitudeAudio
{
    namespace
    {
        bool IsUnder(const Actor* root, const Actor* actor)
        {
            if (!root || !actor)
                return false;

            for (const Actor* a = actor; a; a = a->GetParent())
            {
                if (a == root)
                    return true;
            }
            return false;
        }
    } // namespace

    float PhysicsOcclusionCaster::Cast(const Actor* sourceActor, uint32 sourceLayers, const Vector3& from, const Vector3& to) const
    {
        using namespace AmplitudeAudio::Obstruction;

        Vector3 delta = to - from;
        float length = delta.Length();
        if (length < 1.0f)
            return 0.0f;
        length = Math::Min(length, _settings.MaxDistance);

        Array<RayCastHit, HeapAllocation> hits;
        if (!Physics::RayCastAll(from, delta / delta.Length(), hits, length, _settings.Layers.Mask & sourceLayers, false))
            return 0.0f;

        Array<float, InlinedAllocation<16>> amounts;
        for (const RayCastHit& hit : hits)
        {
            if (!hit.Collider || IsUnder(sourceActor, hit.Collider))
                continue;

            const AmplitudeObstruction* script = hit.Collider->GetScript<AmplitudeObstruction>();
            const float surface = _surfaces.Resolve(script, hit.Material);
            amounts.Add(surface * DistanceScale(hit.Distance, _settings.FullObstructionDistance, _settings.MaxDistance));
        }
        return CombineTransmission(amounts.Get(), amounts.Count());
    }
} // namespace AmplitudeAudio
