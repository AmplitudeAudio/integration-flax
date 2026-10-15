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

#include "AmplitudeOcclusionProcessor.h"

#include <AmplitudeAudio/Runtime/Occlusion/ObstructionMath.h>
#include <Engine/Core/Collections/Sorting.h>
#include <Engine/Core/Math/Math.h>

namespace AmplitudeAudio
{
    void AmplitudeOcclusionProcessor::AddSource(IOcclusionSource* source)
    {
        if (!source)
            return;

        for (const auto& state : _states)
        {
            if (state.Source == source)
                return;
        }

        SourceState state;
        state.Source = source;
        if (_offsets.HasItems())
        {
            state.IndirectOpen.Resize(_offsets.Count());
            for (int32 i = 0; i < state.IndirectOpen.Count(); i++)
                state.IndirectOpen[i] = -1;
        }
        _states.Add(state);
    }

    void AmplitudeOcclusionProcessor::RemoveSource(IOcclusionSource* source)
    {
        if (!source)
            return;

        for (int32 i = 0; i < _states.Count(); i++)
        {
            if (_states[i].Source == source)
            {
                _states.RemoveAt(i);
                break;
            }
        }
    }

    int32 AmplitudeOcclusionProcessor::GetSourceCount() const
    {
        return _states.Count();
    }

    void AmplitudeOcclusionProcessor::Update(const Vector3& listener, float dt, const AmplitudeObstructionSettings& s, const CastFunction& cast)
    {
        using namespace Obstruction;
        _lastRays = 0;
        if (_offsets.Count() != Math::Max(s.IndirectRayCount, 1) || _seed != s.RandomSeed)
        {
            _offsets = MakeDiskOffsets(s.IndirectRayCount, s.RandomSeed);
            _seed = s.RandomSeed;
            for (auto& state : _states)
            {
                state.IndirectOpen.Resize(_offsets.Count());
                for (int32 i = 0; i < state.IndirectOpen.Count(); i++)
                    state.IndirectOpen[i] = -1;
            }
        }

        // 1. Decide who is a candidate.
        Array<SourceState*, InlinedAllocation<32>> candidates;
        for (auto& state : _states)
        {
            IOcclusionSource& src = *state.Source;
            const float distance = Vector3::Distance(src.GetOcclusionLocation(), listener);
            const bool eligible = src.GetObstructionMode() != AmplitudeObstructionMode::None && src.IsOcclusionActive() && distance <= s.MaxDistance;
            if (!eligible)
            {
                state.TargetObstruction = state.TargetOcclusion = 0.0f;
                state.FramesSinceCast = 0;
                continue;
            }
            state.Distance = distance;
            state.Priority = (state.FramesSinceCast + 1) * (1.0f - distance / Math::Max(s.MaxDistance, 1.0f));
            candidates.Add(&state);
        }

        // 2. Highest priority first, then spend the budget.
        Sorting::QuickSort(candidates.Get(), candidates.Count(), &ComparePriority);
        int32 budget = s.RayBudgetPerFrame;
        for (SourceState* state : candidates)
        {
            const bool multiple = state->Source->GetObstructionMode() == AmplitudeObstructionMode::MultipleRays;
            const int32 cost = multiple ? 2 : 1;
            if (budget < cost)
            {
                state->FramesSinceCast++;
                continue;
            }
            budget -= cost;
            _lastRays += cost;
            state->FramesSinceCast = 0;

            const Vector3 from = state->Source->GetOcclusionLocation();
            state->DirectAmount = cast(*state->Source, from, listener);
            if (_rayObserver.IsBinded())
                _rayObserver(from, listener, state->DirectAmount, true);

            if (multiple)
            {
                // Disk perpendicular to the direct path, centred on the listener.
                Vector3 axis = listener - from;
                if (axis.LengthSquared() < 1.0f)
                    axis = Vector3::Forward;
                axis.Normalize();
                Vector3 right = Vector3::Cross(axis, Vector3::Up);
                if (right.LengthSquared() < 0.0001f)
                    right = Vector3::Cross(axis, Vector3::Right);
                right.Normalize();
                const Vector3 up = Vector3::Cross(right, axis);

                const Float2 o = _offsets[state->Cursor % _offsets.Count()];
                const Vector3 target = listener + (right * o.X + up * o.Y) * s.IndirectRadius;
                const float indirect = cast(*state->Source, from, target);
                if (_rayObserver.IsBinded())
                    _rayObserver(from, target, indirect, false);

                state->IndirectOpen[state->Cursor % _offsets.Count()] = indirect < kIndirectOpenThreshold ? 1 : 0;
                state->Cursor++;
            }

            bool anyOpen = false;
            for (int8 v : state->IndirectOpen)
                anyOpen |= v == 1;
            const Classification c = Classify(state->DirectAmount, anyOpen, multiple);
            state->TargetObstruction = c.Obstruction;
            state->TargetOcclusion = c.Occlusion;
        }

        // 3. Smooth every source every frame and write the changes.
        for (auto& state : _states)
        {
            state.Obstruction = SmoothTo(state.Obstruction, state.TargetObstruction, dt, s.AttackSmoothing, s.ReleaseSmoothing);
            state.Occlusion = SmoothTo(state.Occlusion, state.TargetOcclusion, dt, s.AttackSmoothing, s.ReleaseSmoothing);

            if (Math::Abs(state.Obstruction - state.WrittenObstruction) > 0.001f || Math::Abs(state.Occlusion - state.WrittenOcclusion) > 0.001f)
            {
                state.Source->ApplyOcclusion(state.Obstruction, state.Occlusion);
                state.WrittenObstruction = state.Obstruction;
                state.WrittenOcclusion = state.Occlusion;
            }
        }
    }
} // namespace AmplitudeAudio
