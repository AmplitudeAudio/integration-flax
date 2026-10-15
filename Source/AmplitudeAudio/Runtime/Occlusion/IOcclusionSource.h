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
#include <Engine/Core/Math/Vector3.h>
#include <Engine/Platform/Defines.h>

namespace AmplitudeAudio
{
    /// <summary>Something the processor obstructs: an entity now, a sound instance in phase 5.</summary>
    class AMPLITUDEAUDIO_API IOcclusionSource
    {
    public:
        virtual ~IOcclusionSource() = default;
        virtual AmplitudeObstructionMode GetObstructionMode() const = 0;
        /// <summary>False when nothing it plays is audible, so no rays are spent on it.</summary>
        virtual bool IsOcclusionActive() const = 0;
        virtual Vector3 GetOcclusionLocation() const = 0;
        /// <summary>Written only when a value changed by more than a small epsilon.</summary>
        virtual void ApplyOcclusion(float obstruction, float occlusion) = 0;
    };
} // namespace AmplitudeAudio
