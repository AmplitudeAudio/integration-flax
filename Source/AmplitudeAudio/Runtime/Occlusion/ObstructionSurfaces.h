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
#include <Engine/Core/Collections/Dictionary.h>
#include <Engine/Physics/PhysicalMaterial.h>
#include <Engine/Platform/Defines.h>

class AmplitudeObstruction;

namespace AmplitudeAudio
{
    /// <summary>Table resolving per-surface obstruction amounts from materials, tags, or scripts.</summary>
    class AMPLITUDEAUDIO_API ObstructionSurfaces
    {
    public:
        void Build(const AmplitudeObstructionSettings& settings);

        /// <summary>True when a material reference in the settings was not loaded yet; Build again later.</summary>
        bool HasUnresolved() const { return _unresolved; }

        /// <summary>Resolves obstruction amount: 1. script override, 2. exact material, 3. material tag walked up its hierarchy, 4. default.</summary>
        float Resolve(const AmplitudeObstruction* script, PhysicalMaterial* material) const;

    private:
        Dictionary<PhysicalMaterial*, float> _materials;
        Dictionary<uint32, float> _tags;
        float _default = 1.0f;
        bool _unresolved = false;
    };
} // namespace AmplitudeAudio
