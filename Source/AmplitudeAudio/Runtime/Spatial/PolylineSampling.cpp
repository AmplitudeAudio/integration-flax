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

#include "PolylineSampling.h"

namespace AmplitudeAudio::Spatial
{
    Array<Vector3> ResampleByArcLength(const Array<Vector3>& polyline, float spacing, bool closed)
    {
        Array<Vector3> out;
        if (polyline.Count() < 2 || spacing <= 0.0f)
        {
            out = polyline;
            return out;
        }

        out.Add(polyline[0]);
        float carried = 0.0f; // distance walked since the last emitted point
        const int32 segments = closed ? polyline.Count() : polyline.Count() - 1;
        for (int32 i = 0; i < segments; i++)
        {
            Vector3 from = polyline[i];
            const Vector3 to = polyline[(i + 1) % polyline.Count()];
            float remaining = Vector3::Distance(from, to);
            while (carried + remaining >= spacing - 0.0001f)
            {
                const float step = spacing - carried;
                from = Vector3::Lerp(from, to, step / remaining);
                remaining -= step;
                carried = 0.0f;
                out.Add(from);
            }
            carried += remaining;
        }

        // A closed path ends where it started: do not repeat the first point.
        if (closed && out.Count() > 1 && Vector3::Distance(out.Last(), out[0]) < spacing * 0.5f)
            out.RemoveLast();
        return out;
    }
} // namespace AmplitudeAudio::Spatial
