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

#include "AmplitudeSoundInstance.h"

// clang-format off
#include <AmplitudeAudio/Core/Cpp20Compat.h>
// clang-format on

#include <Engine/Debug/DebugDraw.h>
#include <Engine/Serialization/Serialization.h>

#include <AmplitudeAudio/Core/Helpers.h>
#include <AmplitudeAudio/Runtime/AmplitudeService.h>

using AmplitudeAudio::AmplitudeService;

AmplitudeSoundInstance::AmplitudeSoundInstance(const SpawnParams& params)
    : Actor(params)
{
}

bool AmplitudeSoundInstance::IsBound() const
{
    return _registered && _handle.Valid();
}

uint64 AmplitudeSoundInstance::GetBoundId() const
{
    return IsBound() ? static_cast<uint64>(_handle.GetId()) : 0;
}

void AmplitudeSoundInstance::Bind(const SparkyStudios::Audio::Amplitude::ChannelInstance& handle)
{
    Unbind();

    _handle = handle;
    if (_handle.Valid())
    {
        _registered = true;
        PushState();
        AmplitudeService::Get().AddClient(this);
        AmplitudeService::Get().GetOcclusion().AddSource(this);
        AmplitudeService::Get().GetSpatial().AddTarget(this);
    }
}

void AmplitudeSoundInstance::Unbind()
{
    // Keyed on _registered, not on the handle: the SDK invalidates a handle as soon as the channel drops the instance,
    // before the emitter gets to unbind it.
    if (_registered)
    {
        AmplitudeService::Get().GetSpatial().RemoveTarget(this);
        AmplitudeService::Get().GetOcclusion().RemoveSource(this);
        AmplitudeService::Get().RemoveClient(this);
        _registered = false;
    }
    _handle.Clear();
}

void AmplitudeSoundInstance::OnEngineStarted()
{
    // Sound instances are created and bound dynamically by emitters during playback.
}

void AmplitudeSoundInstance::OnEngineStopping()
{
    // Stays a client until Unbind: the service is iterating its clients and cannot drop one here.
    AmplitudeService::Get().GetSpatial().RemoveTarget(this);
    AmplitudeService::Get().GetOcclusion().RemoveSource(this);
    _handle.Clear();
}

void AmplitudeSoundInstance::OnEndPlay()
{
    Unbind();
    Actor::OnEndPlay();
}

void AmplitudeSoundInstance::OnDeleteObject()
{
    // EndPlay does not run for an actor that never played, but a bound one is still registered.
    Unbind();
    Actor::OnDeleteObject();
}

void AmplitudeSoundInstance::OnTransformChanged()
{
    Actor::OnTransformChanged();
    if (IsBound())
        PushState();
}

void AmplitudeSoundInstance::PushState()
{
    if (!_handle.Valid())
        return;

    _handle.SetLocation(AmplitudeAudio::FlaxVec3ToAmVec3(GetPosition()));
    if (TrackOrientation)
        _handle.SetOrientation(AmplitudeAudio::FlaxOrientationToAm(GetTransform().GetForward(), GetTransform().GetUp()));
    else
        _handle.ResetOrientation();
    _handle.SetWeight(Weight);
}

AmplitudeObstructionMode AmplitudeSoundInstance::GetObstructionMode() const
{
    return ObstructionMode;
}

bool AmplitudeSoundInstance::IsOcclusionActive() const
{
    return IsBound();
}

Vector3 AmplitudeSoundInstance::GetOcclusionLocation() const
{
    return GetPosition();
}

const Actor* AmplitudeSoundInstance::GetOcclusionActor() const
{
    return this;
}

uint32 AmplitudeSoundInstance::GetOcclusionLayers() const
{
    return ObstructionLayers.Mask;
}

void AmplitudeSoundInstance::ApplyOcclusion(float obstruction, float occlusion)
{
    if (!IsBound())
        return;

    _handle.SetObstruction(obstruction);
    _handle.SetOcclusion(occlusion);
}

Vector3 AmplitudeSoundInstance::GetSpatialLocation() const
{
    return GetPosition();
}

void AmplitudeSoundInstance::ApplyRoom(uint64 roomId)
{
    if (!IsBound())
        return;

    _handle.SetRoom(roomId ? AmplitudeService::Get().FindRoom(roomId) : SparkyStudios::Audio::Amplitude::Room());
}

void AmplitudeSoundInstance::ApplyEnvironmentFactor(uint64 environmentId, float factor)
{
    // No-op for sound instance
}

#if USE_EDITOR
void AmplitudeSoundInstance::OnDebugDrawSelected()
{
    DebugDraw::DrawWireSphere(BoundingSphere(GetPosition(), 20.0f), Color::Cyan);
    DebugDraw::DrawLine(GetPosition(), GetPosition() + GetTransform().GetForward() * 50.0f, Color::Blue);

    Actor::OnDebugDrawSelected();
}
#endif

void AmplitudeSoundInstance::Serialize(SerializeStream& stream, const void* otherObj)
{
    Actor::Serialize(stream, otherObj);
    SERIALIZE_GET_OTHER_OBJ(AmplitudeSoundInstance);
    SERIALIZE(Weight);
    SERIALIZE(TrackOrientation);
    SERIALIZE(ObstructionMode);
    SERIALIZE(ObstructionLayers);
}

void AmplitudeSoundInstance::Deserialize(DeserializeStream& stream, ISerializeModifier* modifier)
{
    Actor::Deserialize(stream, modifier);
    DESERIALIZE(Weight);
    DESERIALIZE(TrackOrientation);
    DESERIALIZE(ObstructionMode);
    DESERIALIZE(ObstructionLayers);
}
