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

#include <Engine/Core/Collections/Array.h>
#include <Engine/Core/Collections/Dictionary.h>
#include <Engine/Core/Math/Vector3.h>
#include <Engine/Core/Types/BaseTypes.h>
#include <Engine/Platform/Defines.h>

#include <AmplitudeAudio/Runtime/Spatial/ISpatialTarget.h>
#include <AmplitudeAudio/Runtime/Spatial/SpatialMath.h>

namespace AmplitudeAudio
{
    struct EnvironmentCandidate
    {
        uint64 Id = 0;
        Spatial::OrientedBox Box;
        float BlendDistance = 0.0f;
    };

    class AMPLITUDEAUDIO_API AmplitudeSpatialProcessor
    {
    public:
        void AddTarget(ISpatialTarget* target);
        void RemoveTarget(ISpatialTarget* target);
        int32 GetTargetCount() const;

        /// <summary>Replaces the world; bumps the version so every target is recomputed once.</summary>
        void SetWorld(const Array<Spatial::RoomCandidate>& rooms, const Array<EnvironmentCandidate>& environments);

        /// <summary>Recomputes moved targets (or all after a world change). A flag that is off skips that part.</summary>
        void Update(bool applyRooms, bool applyEnvironments);

        /// <summary>Targets recomputed by the last Update.</summary>
        int32 GetLastUpdatedCount() const;

    private:
        struct TargetState
        {
            ISpatialTarget* Target = nullptr;
            Vector3 LastLocation = Vector3::Zero;
            bool RoomDirty = true;
            bool EnvDirty = true;
            bool HasLocation = false;
            bool RoomComputed = false;
            uint32 RoomVersion = 0;
            uint32 EnvVersion = 0;
            uint64 AppliedRoom = 0;
            Dictionary<uint64, float> AppliedFactors;
        };

        Array<TargetState> m_targets;
        Array<Spatial::RoomCandidate> m_rooms;
        Array<EnvironmentCandidate> m_environments;
        uint32 m_worldVersion = 1;
        int32 m_lastUpdatedCount = 0;
    };
} // namespace AmplitudeAudio
