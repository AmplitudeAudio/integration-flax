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

#include "AmplitudeSplineInstanceGenerator.h"

#include <Engine/Level/Actors/Spline.h>
#include <Engine/Serialization/Serialization.h>

#include <AmplitudeAudio/Runtime/Spatial/PolylineSampling.h>

AmplitudeSplineInstanceGenerator::AmplitudeSplineInstanceGenerator(const SpawnParams& params)
    : Script(params)
{
}

void AmplitudeSplineInstanceGenerator::Generate()
{
    Clear();

    if (!Emitter || !Emitter.Get() || !GetActor())
        return;

    auto* spline = Cast<Spline>(GetActor());
    if (!spline || spline->GetSplinePointsCount() < 2)
        return;

    Array<Vector3> polyline;
    const int32 pointCount = spline->GetSplinePointsCount();
    const int32 segments = spline->GetIsLoop() ? pointCount : pointCount - 1;
    const int32 samples = Math::Max(SamplesPerSegment, 1);

    for (int32 s = 0; s < segments; s++)
    {
        for (int32 k = 0; k < samples; k++)
        {
            const float time = s + static_cast<float>(k) / static_cast<float>(samples);
            polyline.Add(spline->GetSplinePoint(time));
        }
    }
    if (!spline->GetIsLoop())
        polyline.Add(spline->GetSplinePoint(static_cast<float>(pointCount - 1)));

    const Array<Vector3> points = AmplitudeAudio::Spatial::ResampleByArcLength(polyline, Spacing, spline->GetIsLoop());
    for (const Vector3& worldPos : points)
    {
        auto* inst = ScriptingObject::NewObject<AmplitudeSoundInstance>();
        inst->SetPosition(worldPos);
        inst->Weight = Weight;
        inst->SetParent(GetActor(), true, false);
        Emitter->Instances.Add(inst);
        _generated.Add(inst);
    }
}

void AmplitudeSplineInstanceGenerator::Clear()
{
    for (const auto& ref : _generated)
    {
        AmplitudeSoundInstance* inst = ref.Get();
        if (inst)
        {
            if (Emitter && Emitter.Get())
                Emitter->Instances.Remove(inst);
            inst->SetParent(nullptr);
            inst->DeleteObject();
        }
    }
    _generated.Clear();
}

void AmplitudeSplineInstanceGenerator::OnStart()
{
    Script::OnStart();
    if (GenerateOnStart)
        Generate();
}

void AmplitudeSplineInstanceGenerator::OnDisable()
{
    Clear();
    Script::OnDisable();
}

void AmplitudeSplineInstanceGenerator::Serialize(SerializeStream& stream, const void* otherObj)
{
    Script::Serialize(stream, otherObj);
    SERIALIZE_GET_OTHER_OBJ(AmplitudeSplineInstanceGenerator);
    SERIALIZE(Emitter);
    SERIALIZE(Spacing);
    SERIALIZE(SamplesPerSegment);
    SERIALIZE(Weight);
    SERIALIZE(GenerateOnStart);
}

void AmplitudeSplineInstanceGenerator::Deserialize(DeserializeStream& stream, ISerializeModifier* modifier)
{
    Script::Deserialize(stream, modifier);
    DESERIALIZE(Emitter);
    DESERIALIZE(Spacing);
    DESERIALIZE(SamplesPerSegment);
    DESERIALIZE(Weight);
    DESERIALIZE(GenerateOnStart);
}
