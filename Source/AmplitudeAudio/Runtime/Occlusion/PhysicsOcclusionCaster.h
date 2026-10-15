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

#include <AmplitudeAudio/AmplitudeSettings.h>
#include <AmplitudeAudio/Runtime/Occlusion/ObstructionSurfaces.h>
#include <Engine/Level/Actor.h>
#include <Engine/Platform/Defines.h>

namespace AmplitudeAudio
{
    class AMPLITUDEAUDIO_API PhysicsOcclusionCaster
    {
    public:
        explicit PhysicsOcclusionCaster(const AmplitudeObstructionSettings& settings, const ObstructionSurfaces& surfaces)
            : _settings(settings)
            , _surfaces(surfaces)
        {
        }

        /// <summary>Combined obstruction between two points, ignoring triggers and the source's own hierarchy.</summary>
        float Cast(const Actor* sourceActor, uint32 sourceLayers, const Vector3& from, const Vector3& to) const;

    private:
        const AmplitudeObstructionSettings& _settings;
        const ObstructionSurfaces& _surfaces;
    };
} // namespace AmplitudeAudio
