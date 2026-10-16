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

#include "ObstructionSurfaces.h"

#include <AmplitudeAudio/Scripts/AmplitudeObstruction.h>
#include <Engine/Core/Math/Math.h>
#include <Engine/Level/Tags.h>

namespace AmplitudeAudio
{
    void ObstructionSurfaces::Build(const AmplitudeObstructionSettings& settings)
    {
        _materials.Clear();
        _tags.Clear();
        _unresolved = false;
        _default = Math::Saturate(settings.DefaultAmount);

        for (const auto& entry : settings.MaterialAmounts)
        {
            PhysicalMaterial* material = entry.Material ? entry.Material->GetInstance<PhysicalMaterial>() : nullptr;
            if (material)
                _materials[material] = Math::Saturate(entry.Amount);
            else if (entry.Material.GetID().IsValid())
                _unresolved = true;
        }
        for (const auto& entry : settings.TagAmounts)
        {
            if (entry.SurfaceTag)
                _tags[entry.SurfaceTag.Index] = Math::Saturate(entry.Amount);
        }
    }

    float ObstructionSurfaces::Resolve(const AmplitudeObstruction* script, PhysicalMaterial* material) const
    {
        if (script)
            return Math::Saturate(script->Amount);
        if (!material)
            return _default;

        float amount;
        if (_materials.TryGet(material, amount))
            return amount;

        // Walk "Surface.Wood.Thin" -> "Surface.Wood" -> "Surface".
        String name = material->Tag.ToString();
        while (name.HasChars())
        {
            const Tag tag = Tags::Find(name);
            if (tag && _tags.TryGet(tag.Index, amount))
                return amount;
            const int32 dot = name.FindLast('.');
            if (dot < 0)
                break;
            name = name.Left(dot);
        }
        return _default;
    }
} // namespace AmplitudeAudio
