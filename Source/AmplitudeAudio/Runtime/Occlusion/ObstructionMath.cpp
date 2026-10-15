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

#include "ObstructionMath.h"

#include <Engine/Core/Math/Math.h>

namespace AmplitudeAudio::Obstruction
{
    float CombineTransmission(const float* amounts, int32 count)
    {
        float passing = 1.0f;
        for (int32 i = 0; i < count; i++)
            passing *= 1.0f - Math::Saturate(amounts[i]);
        return 1.0f - passing;
    }

    float DistanceScale(float distance, float fullDistance, float maxDistance)
    {
        if (distance <= fullDistance)
            return 1.0f;
        if (maxDistance <= fullDistance)
            return 0.0f;
        return 1.0f - Math::Saturate((distance - fullDistance) / (maxDistance - fullDistance));
    }

    float SmoothTo(float current, float target, float deltaSeconds, float attackSeconds, float releaseSeconds)
    {
        const float tau = target > current ? attackSeconds : releaseSeconds;
        if (tau <= 0.0f)
            return target;
        return current + (target - current) * (1.0f - Math::Exp(-deltaSeconds / tau));
    }

    Classification Classify(float directAmount, bool anyIndirectOpen, bool multipleRays)
    {
        if (directAmount <= kBlockedEpsilon)
            return {0.0f, 0.0f};
        if (!multipleRays || anyIndirectOpen)
            return {directAmount, 0.0f};
        return {0.0f, directAmount};
    }

    Array<Float2> MakeDiskOffsets(int32 count, uint32 seed)
    {
        Array<Float2> points;
        const int32 n = Math::Max(count, 1);
        points.EnsureCapacity(n);

        uint32 state = seed != 0 ? seed : 0x9E3779B9u;
        const auto next = [&state]() {
            state ^= state << 13;
            state ^= state >> 17;
            state ^= state << 5;
            return static_cast<float>(state & 0xFFFFFFu) / static_cast<float>(0x1000000u);
        };

        for (int32 i = 0; i < n; i++)
        {
            const float radius = Math::Sqrt(next());
            const float angle = next() * TWO_PI;
            points.Add(Float2(radius * Math::Cos(angle), radius * Math::Sin(angle)));
        }
        return points;
    }
} // namespace AmplitudeAudio::Obstruction
