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

#include "AmplitudePointInstanceGenerator.h"

#include <Engine/Serialization/Serialization.h>

AmplitudePointInstanceGenerator::AmplitudePointInstanceGenerator(const SpawnParams& params)
    : Script(params)
{
}

void AmplitudePointInstanceGenerator::Generate()
{
    Clear();

    if (!Emitter || !Emitter.Get() || !GetActor())
        return;

    for (const Vector3& localPos : Points)
    {
        const Vector3 worldPos = GetActor()->GetTransform().LocalToWorld(localPos);
        auto* inst = ScriptingObject::NewObject<AmplitudeSoundInstance>();
        inst->SetPosition(worldPos);
        inst->Weight = Weight;
        inst->SetParent(GetActor(), true, false);
        Emitter->Instances.Add(inst);
        _generated.Add(inst);
    }
}

void AmplitudePointInstanceGenerator::Clear()
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

void AmplitudePointInstanceGenerator::OnStart()
{
    Script::OnStart();
    if (GenerateOnStart)
        Generate();
}

void AmplitudePointInstanceGenerator::OnDisable()
{
    Clear();
    Script::OnDisable();
}

void AmplitudePointInstanceGenerator::Serialize(SerializeStream& stream, const void* otherObj)
{
    Script::Serialize(stream, otherObj);
    SERIALIZE_GET_OTHER_OBJ(AmplitudePointInstanceGenerator);
    SERIALIZE(Emitter);
    SERIALIZE(Points);
    SERIALIZE(Weight);
    SERIALIZE(GenerateOnStart);
}

void AmplitudePointInstanceGenerator::Deserialize(DeserializeStream& stream, ISerializeModifier* modifier)
{
    Script::Deserialize(stream, modifier);
    DESERIALIZE(Emitter);
    DESERIALIZE(Points);
    DESERIALIZE(Weight);
    DESERIALIZE(GenerateOnStart);
}
