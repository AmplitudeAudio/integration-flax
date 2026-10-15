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

#include <Engine/Level/SceneObject.h>
#include <Engine/Scripting/Script.h>
#include <Engine/Scripting/ScriptingObject.h>
#include <Engine/Serialization/Serialization.h>

#include <AmplitudeAudio/Core/Helpers.h>
#include <AmplitudeAudio/Runtime/AmplitudeService.h>
#include <AmplitudeAudio/Scripts/IAmplitudeEmitter.h>

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
    // Cleared even without an engine: after a shutdown the handle points at freed state.
    if (!_entity.Valid())
        return;

    if (auto* engine = AmplitudeService::Get().GetEngine())
        engine->RemoveEntity(&_entity);
    _entity.Clear();
}

void AmplitudeEntity::Register()
{
    if (_registered)
        return;
    _registered = true;
    AmplitudeService::Get().AddClient(this);
}

void AmplitudeEntity::Unregister()
{
    if (_registered)
    {
        AmplitudeService::Get().RemoveClient(this);
        _registered = false;
    }
    if (_entity.Valid())
        OnEngineStopping();
}

void AmplitudeEntity::OnBeginPlay()
{
    Actor::OnBeginPlay();
    Register();
}

void AmplitudeEntity::OnEndPlay()
{
    Unregister();
    Actor::OnEndPlay();
}

void AmplitudeEntity::OnEnable()
{
    Actor::OnEnable();
    Register();
}

void AmplitudeEntity::OnDisable()
{
    Unregister();
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

IAmplitudeEmitter* AmplitudeEntity::GetEmitter(const ::StringView& name)
{
    // Search scripts on this actor
    auto scripts = GetScripts<Script>();
    for (int i = 0; i < scripts.Count(); i++)
    {
        if (auto* e = dynamic_cast<IAmplitudeEmitter*>(static_cast<SceneObject*>(scripts[i])))
        {
            if (e->GetName() == name)
                return e;
        }
    }
    // Search descendants
    for (Actor* child : GetChildren<Actor>())
    {
        auto childScripts = child->GetScripts<Script>();
        for (int i = 0; i < childScripts.Count(); i++)
        {
            if (auto* e = dynamic_cast<IAmplitudeEmitter*>(static_cast<SceneObject*>(childScripts[i])))
            {
                if (e->GetName() == name)
                    return e;
            }
        }
    }
    return nullptr;
}

Array<IAmplitudeEmitter*> AmplitudeEntity::GetEmitters()
{
    Array<IAmplitudeEmitter*> result;
    auto scripts = GetScripts<Script>();
    for (int i = 0; i < scripts.Count(); i++)
    {
        if (auto* e = dynamic_cast<IAmplitudeEmitter*>(static_cast<SceneObject*>(scripts[i])))
            result.Add(e);
    }
    for (Actor* child : GetChildren<Actor>())
    {
        auto childScripts = child->GetScripts<Script>();
        for (int i = 0; i < childScripts.Count(); i++)
        {
            if (auto* e = dynamic_cast<IAmplitudeEmitter*>(static_cast<SceneObject*>(childScripts[i])))
                result.Add(e);
        }
    }
    return result;
}
