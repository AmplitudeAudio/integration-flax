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

#include "AmplitudeEnvironment.h"

// clang-format off
#include <AmplitudeAudio/Core/Cpp20Compat.h>
// clang-format on

#include <Engine/Debug/DebugDraw.h>
#include <Engine/Core/Math/OrientedBoundingBox.h>
#include <Engine/Serialization/Serialization.h>

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>
#include <SparkyStudios/Audio/Amplitude/Math/Shape.h>

#include <AmplitudeAudio/Core/Helpers.h>
#include <AmplitudeAudio/Runtime/AmplitudeService.h>

using AmplitudeAudio::AmplitudeService;
using AmplitudeAudio::EnvironmentCandidate;
using SparkyStudios::Audio::Amplitude::AmSharedPtr;

AmplitudeEnvironment::AmplitudeEnvironment(const SpawnParams& params)
    : Actor(params)
{
}

bool AmplitudeEnvironment::IsValid() const
{
    return _environment.Valid();
}

void AmplitudeEnvironment::OnEngineStarted()
{
    auto* engine = AmplitudeService::Get().GetEngine();
    if (!engine)
        return;

    _id = AmplitudeService::Get().AllocateId();
    _environment = engine->AddEnvironment(static_cast<SparkyStudios::Audio::Amplitude::AmEnvironmentID>(_id));
    PushState();
    AmplitudeService::Get().RegisterEnvironment(MakeCandidate());
}

void AmplitudeEnvironment::OnEngineStopping()
{
    if (_id != 0)
    {
        AmplitudeService::Get().UnregisterEnvironment(_id);
        if (auto* engine = AmplitudeService::Get().GetEngine())
            engine->RemoveEnvironment(&_environment);
        _environment.Clear();
        _id = 0;
    }
}

void AmplitudeEnvironment::Register()
{
    if (_registered)
        return;
    _registered = true;
    AmplitudeService::Get().AddClient(this);
}

void AmplitudeEnvironment::Unregister()
{
    if (_registered)
    {
        AmplitudeService::Get().RemoveClient(this);
        _registered = false;
    }
    if (_environment.Valid())
        OnEngineStopping();
}

void AmplitudeEnvironment::OnBeginPlay()
{
    Actor::OnBeginPlay();
    Register();
}

void AmplitudeEnvironment::OnEndPlay()
{
    Unregister();
    Actor::OnEndPlay();
}

void AmplitudeEnvironment::OnEnable()
{
    Actor::OnEnable();
    Register();
}

void AmplitudeEnvironment::OnDisable()
{
    Unregister();
    Actor::OnDisable();
}

void AmplitudeEnvironment::OnTransformChanged()
{
    Actor::OnTransformChanged();
    if (_environment.Valid())
    {
        PushState();
        AmplitudeService::Get().UpdateEnvironment(MakeCandidate());
    }
}

void AmplitudeEnvironment::PushState() const
{
    if (!_environment.Valid())
        return;

    std::shared_ptr<SparkyStudios::Audio::Amplitude::Zone> zone;
    switch (Shape)
    {
    case AmplitudeEnvironmentShape::Sphere:
        {
            const SparkyStudios::Audio::Amplitude::AmVector3 innerExtents = AmplitudeAudio::ToAmDimensions(Vector3(Radius, 0, 0));
            const SparkyStudios::Audio::Amplitude::AmVector3 outerExtents = AmplitudeAudio::ToAmDimensions(Vector3(Radius + BlendDistance, 0, 0));

            auto inner = amshared(SparkyStudios::Audio::Amplitude::SphereShape, innerExtents[0]);
            auto outer = amshared(SparkyStudios::Audio::Amplitude::SphereShape, outerExtents[0]);
            zone = amshared(SparkyStudios::Audio::Amplitude::SphereZone, inner, outer);
            break;
        }
    case AmplitudeEnvironmentShape::Box:
    default:
        {
            const SparkyStudios::Audio::Amplitude::AmVector3 innerExtents = AmplitudeAudio::ToAmDimensions(Size * 0.5f);
            const SparkyStudios::Audio::Amplitude::AmVector3 outerExtents = AmplitudeAudio::ToAmDimensions(Size * 0.5f + Vector3(BlendDistance));

            auto inner = amshared(SparkyStudios::Audio::Amplitude::BoxShape, innerExtents[0], innerExtents[1], innerExtents[2]);
            auto outer = amshared(SparkyStudios::Audio::Amplitude::BoxShape, outerExtents[0], outerExtents[1], outerExtents[2]);
            zone = amshared(SparkyStudios::Audio::Amplitude::BoxZone, inner, outer);
            break;
        }
    }

    if (zone)
        _environment.SetZone(zone);

    _environment.SetLocation(AmplitudeAudio::FlaxVec3ToAmVec3(GetPosition()));
    _environment.SetOrientation(AmplitudeAudio::FlaxOrientationToAm(GetTransform().GetForward(), GetTransform().GetUp()));

    if (Effect.IsValid())
        _environment.SetEffect(static_cast<SparkyStudios::Audio::Amplitude::AmEffectID>(Effect.ID));
}

AmplitudeAudio::Spatial::ZoneShape AmplitudeEnvironment::MakeZone() const
{
    if (Shape == AmplitudeEnvironmentShape::Sphere)
        return AmplitudeAudio::Spatial::ZoneShape::MakeSphere(GetPosition(), Radius);

    return AmplitudeAudio::Spatial::ZoneShape::MakeBox(GetPosition(), GetOrientation(), Size * 0.5f);
}

EnvironmentCandidate AmplitudeEnvironment::MakeCandidate() const
{
    return { _id, MakeZone(), BlendDistance };
}

#if USE_EDITOR
void AmplitudeEnvironment::OnDebugDrawSelected()
{
    if (Shape == AmplitudeEnvironmentShape::Sphere)
    {
        DebugDraw::DrawWireSphere(BoundingSphere(GetPosition(), Radius), Color::Orange);

        if (BlendDistance > 0.0f)
        {
            Color blendColor = Color::Orange;
            blendColor.A *= 0.5f;
            DebugDraw::DrawWireSphere(BoundingSphere(GetPosition(), Radius + BlendDistance), blendColor);
        }
    }
    else
    {
        OrientedBoundingBox obb(Vector3::Zero, Size * 0.5f);
        obb.Transform(GetTransform());
        DebugDraw::DrawWireBox(obb, Color::Orange);

        if (BlendDistance > 0.0f)
        {
            OrientedBoundingBox blendObb(Vector3::Zero, Size * 0.5f + Vector3(BlendDistance));
            blendObb.Transform(GetTransform());
            Color blendColor = Color::Orange;
            blendColor.A *= 0.5f;
            DebugDraw::DrawWireBox(blendObb, blendColor);
        }
    }

    Actor::OnDebugDrawSelected();
}
#endif

void AmplitudeEnvironment::Serialize(SerializeStream& stream, const void* otherObj)
{
    Actor::Serialize(stream, otherObj);
    SERIALIZE_GET_OTHER_OBJ(AmplitudeEnvironment);
    SERIALIZE(Shape);
    SERIALIZE(Size);
    SERIALIZE(Radius);
    SERIALIZE(BlendDistance);
    SERIALIZE(Effect);
}

void AmplitudeEnvironment::Deserialize(DeserializeStream& stream, ISerializeModifier* modifier)
{
    Actor::Deserialize(stream, modifier);
    DESERIALIZE(Shape);
    DESERIALIZE(Size);
    DESERIALIZE(Radius);
    DESERIALIZE(BlendDistance);
    DESERIALIZE(Effect);
}
