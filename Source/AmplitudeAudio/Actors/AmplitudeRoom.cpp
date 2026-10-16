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

#include "AmplitudeRoom.h"

// clang-format off
#include <AmplitudeAudio/Core/Cpp20Compat.h>
// clang-format on

#include <Engine/Debug/DebugDraw.h>
#include <Engine/Core/Math/OrientedBoundingBox.h>
#include <Engine/Serialization/Serialization.h>

#include <AmplitudeAudio/Core/Helpers.h>
#include <AmplitudeAudio/Runtime/AmplitudeService.h>

using AmplitudeAudio::AmplitudeService;
using AmplitudeAudio::Spatial::RoomCandidate;

AmplitudeRoom::AmplitudeRoom(const SpawnParams& params)
    : Actor(params)
{
}

bool AmplitudeRoom::IsValid() const
{
    return _room.Valid();
}

void AmplitudeRoom::OnEngineStarted()
{
    auto* engine = AmplitudeService::Get().GetEngine();
    if (!engine)
        return;

    _id = AmplitudeService::Get().AllocateId();
    _room = engine->AddRoom(static_cast<SparkyStudios::Audio::Amplitude::AmRoomID>(_id));
    PushState();
    AmplitudeService::Get().RegisterRoom(_id, _room, MakeCandidate());
}

void AmplitudeRoom::OnEngineStopping()
{
    if (_id != 0)
    {
        AmplitudeService::Get().UnregisterRoom(_id);
        if (auto* engine = AmplitudeService::Get().GetEngine())
            engine->RemoveRoom(&_room);
        _room.Clear();
        _id = 0;
    }
}

void AmplitudeRoom::Register()
{
    if (_registered)
        return;
    _registered = true;
    AmplitudeService::Get().AddClient(this);
}

void AmplitudeRoom::Unregister()
{
    if (_registered)
    {
        AmplitudeService::Get().RemoveClient(this);
        _registered = false;
    }
    if (_room.Valid())
        OnEngineStopping();
}

void AmplitudeRoom::OnBeginPlay()
{
    Actor::OnBeginPlay();
    Register();
}

void AmplitudeRoom::OnEndPlay()
{
    Unregister();
    Actor::OnEndPlay();
}

void AmplitudeRoom::OnEnable()
{
    Actor::OnEnable();
    Register();
}

void AmplitudeRoom::OnDisable()
{
    Unregister();
    Actor::OnDisable();
}

void AmplitudeRoom::OnTransformChanged()
{
    Actor::OnTransformChanged();
    if (_room.Valid())
    {
        PushState();
        AmplitudeService::Get().UpdateRoom(_id, MakeCandidate());
    }
}

void AmplitudeRoom::PushState() const
{
    if (!_room.Valid())
        return;

    _room.SetLocation(AmplitudeAudio::FlaxVec3ToAmVec3(GetPosition()));
    _room.SetOrientation(AmplitudeAudio::FlaxOrientationToAm(GetTransform().GetForward(), GetTransform().GetUp()));
    _room.SetDimensions(AmplitudeAudio::ToAmDimensions(Size));
    _room.SetGain(Gain);
    using SparkyStudios::Audio::Amplitude::RoomWallMaterial;
    using SparkyStudios::Audio::Amplitude::eRoomWallMaterialType;

    _room.SetWallMaterials(
        RoomWallMaterial(static_cast<eRoomWallMaterialType>(LeftWall)),
        RoomWallMaterial(static_cast<eRoomWallMaterialType>(RightWall)),
        RoomWallMaterial(static_cast<eRoomWallMaterialType>(Floor)),
        RoomWallMaterial(static_cast<eRoomWallMaterialType>(Ceiling)),
        RoomWallMaterial(static_cast<eRoomWallMaterialType>(FrontWall)),
        RoomWallMaterial(static_cast<eRoomWallMaterialType>(BackWall)));
    _room.Update();
}

RoomCandidate AmplitudeRoom::MakeCandidate() const
{
    AmplitudeAudio::Spatial::OrientedBox box;
    box.Center = GetPosition();
    box.Rotation = GetOrientation();
    box.HalfExtents = Size * 0.5f;
    return { _id, box };
}

#if USE_EDITOR
void AmplitudeRoom::OnDebugDrawSelected()
{
    OrientedBoundingBox obb(Vector3::Zero, Size * 0.5f);
    obb.Transform(GetTransform());
    DebugDraw::DrawWireBox(obb, Color::Cyan);

    Actor::OnDebugDrawSelected();
}
#endif

void AmplitudeRoom::Serialize(SerializeStream& stream, const void* otherObj)
{
    Actor::Serialize(stream, otherObj);
    SERIALIZE_GET_OTHER_OBJ(AmplitudeRoom);
    SERIALIZE(Size);
    SERIALIZE(Gain);
    SERIALIZE(LeftWall);
    SERIALIZE(RightWall);
    SERIALIZE(Floor);
    SERIALIZE(Ceiling);
    SERIALIZE(FrontWall);
    SERIALIZE(BackWall);
}

void AmplitudeRoom::Deserialize(DeserializeStream& stream, ISerializeModifier* modifier)
{
    Actor::Deserialize(stream, modifier);
    DESERIALIZE(Size);
    DESERIALIZE(Gain);
    DESERIALIZE(LeftWall);
    DESERIALIZE(RightWall);
    DESERIALIZE(Floor);
    DESERIALIZE(Ceiling);
    DESERIALIZE(FrontWall);
    DESERIALIZE(BackWall);
}
