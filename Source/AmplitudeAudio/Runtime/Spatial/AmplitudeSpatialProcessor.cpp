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

#include "AmplitudeSpatialProcessor.h"

#include <Engine/Core/Math/Math.h>

namespace AmplitudeAudio
{
    void AmplitudeSpatialProcessor::AddTarget(ISpatialTarget* target)
    {
        if (!target)
            return;
        for (const TargetState& state : m_targets)
        {
            if (state.Target == target)
                return;
        }
        TargetState state;
        state.Target = target;
        m_targets.Add(state);
    }

    void AmplitudeSpatialProcessor::RemoveTarget(ISpatialTarget* target)
    {
        if (!target)
            return;
        for (int32 i = 0; i < m_targets.Count(); ++i)
        {
            if (m_targets[i].Target == target)
            {
                m_targets.RemoveAt(i);
                break;
            }
        }
    }

    int32 AmplitudeSpatialProcessor::GetTargetCount() const
    {
        return m_targets.Count();
    }

    void AmplitudeSpatialProcessor::SetWorld(const Array<Spatial::RoomCandidate>& rooms, const Array<EnvironmentCandidate>& environments)
    {
        m_rooms = rooms;
        m_environments = environments;
        m_worldVersion++;
    }

    void AmplitudeSpatialProcessor::Update(bool applyRooms, bool applyEnvironments)
    {
        m_lastUpdatedCount = 0;

        for (TargetState& state : m_targets)
        {
            if (!state.Target)
                continue;

            const Vector3 currentLocation = state.Target->GetSpatialLocation();
            const bool moved = !state.HasLocation || Vector3::DistanceSquared(currentLocation, state.LastLocation) > 1.0f;
            if (moved)
            {
                state.RoomDirty = true;
                state.EnvDirty = true;
            }

            bool updatedThisTarget = false;

            if (applyRooms && (state.RoomDirty || state.RoomVersion != m_worldVersion))
            {
                const uint64 id = Spatial::PickRoom(m_rooms.Get(), m_rooms.Count(), currentLocation);
                if (!state.RoomComputed || id != state.AppliedRoom)
                {
                    state.Target->ApplyRoom(id);
                    state.AppliedRoom = id;
                    state.RoomComputed = true;
                }
                state.RoomDirty = false;
                state.RoomVersion = m_worldVersion;
                updatedThisTarget = true;
            }

            if (applyEnvironments && (state.EnvDirty || state.EnvVersion != m_worldVersion))
            {
                for (const EnvironmentCandidate& env : m_environments)
                {
                    const float factor = Spatial::EnvironmentFactor(env.Box, env.BlendDistance, currentLocation);
                    float applied = 0.0f;
                    if (state.AppliedFactors.TryGet(env.Id, applied))
                    {
                        if (Math::Abs(factor - applied) > 0.001f)
                        {
                            state.Target->ApplyEnvironmentFactor(env.Id, factor);
                            state.AppliedFactors[env.Id] = factor;
                        }
                    }
                    else
                    {
                        if (Math::Abs(factor - 0.0f) > 0.001f)
                        {
                            state.Target->ApplyEnvironmentFactor(env.Id, factor);
                            state.AppliedFactors[env.Id] = factor;
                        }
                    }
                }

                // Check for removed environments
                Array<uint64> toRemove;
                for (auto it = state.AppliedFactors.Begin(); it != state.AppliedFactors.End(); ++it)
                {
                    const uint64 id = it->Key;
                    bool exists = false;
                    for (const EnvironmentCandidate& env : m_environments)
                    {
                        if (env.Id == id)
                        {
                            exists = true;
                            break;
                        }
                    }
                    if (!exists)
                    {
                        state.Target->ApplyEnvironmentFactor(id, 0.0f);
                        toRemove.Add(id);
                    }
                }
                for (uint64 id : toRemove)
                    state.AppliedFactors.Remove(id);

                state.EnvDirty = false;
                state.EnvVersion = m_worldVersion;
                updatedThisTarget = true;
            }

            if (!state.RoomDirty && !state.EnvDirty)
            {
                state.LastLocation = currentLocation;
                state.HasLocation = true;
            }

            if (updatedThisTarget)
                m_lastUpdatedCount++;
        }
    }

    int32 AmplitudeSpatialProcessor::GetLastUpdatedCount() const
    {
        return m_lastUpdatedCount;
    }
} // namespace AmplitudeAudio
