// Copyright (c) 2026-present Sparky Studios. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     `http://www.apache.org/licenses/LICENSE-2.0`
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "AmplitudeEntity.h"

#include <Engine/Scripting/ScriptingObject.h>
#include <Engine/Serialization/Serialization.h>

#include <AmplitudeAudio/Core/Helpers.h>
#include <AmplitudeAudio/Runtime/AmplitudeService.h>

using AmplitudeAudio::AmplitudeService;
using SparkyStudios::Audio::Amplitude::Entity;

AmplitudeEntity::AmplitudeEntity(const SpawnParams& params)
    : Actor(params)
{
}

bool AmplitudeEntity::IsValid() const
{
    return _entity.Valid();
}

AmplitudeEntity* AmplitudeEntity::FindNearest(Actor* from)
{
    for (Actor* actor = from; actor; actor = actor->GetParent())
    {
        if (auto* entity = ScriptingObject::Cast<AmplitudeEntity>(actor))
            return entity;
    }
    return nullptr;
}

const Entity& AmplitudeEntity::GetHandle() const
{
    return _entity;
}

void AmplitudeEntity::OnEngineStarted()
{
    auto* engine = AmplitudeService::Get().GetEngine();
    if (!engine)
        return;

    _entity = engine->AddEntity(static_cast<SparkyStudios::Audio::Amplitude::AmObjectID>(
        AmplitudeService::Get().AllocateId()));
    PushState();
}

void AmplitudeEntity::OnEngineStopping()
{
    auto* engine = AmplitudeService::Get().GetEngine();
    if (_entity.Valid() && engine)
    {
        engine->RemoveEntity(&_entity);
        _entity.Clear();
    }
}

void AmplitudeEntity::OnEnable()
{
    Actor::OnEnable();
    if (!_registered)
    {
        AmplitudeService::Get().AddClient(this);
        _registered = true;
    }
}

void AmplitudeEntity::OnDisable()
{
    if (_registered)
    {
        AmplitudeService::Get().RemoveClient(this);
        _registered = false;
    }
    if (_entity.Valid())
    {
        OnEngineStopping();
    }
    Actor::OnDisable();
}

void AmplitudeEntity::OnTransformChanged()
{
    Actor::OnTransformChanged();
    if (_entity.Valid())
        PushState();
}

void AmplitudeEntity::PushState() const
{
    _entity.SetLocation(AmplitudeAudio::FlaxVec3ToAmVec3(GetPosition()));
    _entity.SetOrientation(AmplitudeAudio::FlaxOrientationToAm(GetTransform().GetForward(), GetTransform().GetUp()));
    _entity.SetDirectivity(Directivity, Sharpness);
    _entity.Update();
}

void AmplitudeEntity::Serialize(SerializeStream& stream, const void* otherObj)
{
    Actor::Serialize(stream, otherObj);
    SERIALIZE_GET_OTHER_OBJ(AmplitudeEntity);
    SERIALIZE_MEMBER(Directivity, Directivity);
    SERIALIZE_MEMBER(Sharpness, Sharpness);
}

void AmplitudeEntity::Deserialize(DeserializeStream& stream, ISerializeModifier* modifier)
{
    Actor::Deserialize(stream, modifier);
    DESERIALIZE_MEMBER(Directivity, Directivity);
    DESERIALIZE_MEMBER(Sharpness, Sharpness);
}
