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

#include "AmplitudeEmitter.h"

// clang-format off
#include <AmplitudeAudio/Core/Cpp20Compat.h>
// clang-format on

#include <Engine/Core/Log.h>
#include <Engine/Core/Math/OrientedBoundingBox.h>
#include <Engine/Debug/DebugDraw.h>
#include <Engine/Serialization/Serialization.h>

#include <AmplitudeAudio/Actors/AmplitudeEntity.h>
#include <AmplitudeAudio/Amplitude.h>
#include <AmplitudeAudio/Core/Helpers.h>
#include <AmplitudeAudio/Runtime/Spatial/SpatialMath.h>
#include <AmplitudeAudio/Types/AmplitudeChannel.h>

AmplitudeEmitter::AmplitudeEmitter(const SpawnParams& params)
    : Script(params)
{
}

void AmplitudeEmitter::Play()
{
    ReleaseChannel();
    if (!SoundObject.IsValid())
        return;

    AmplitudeEntity* entity    = AmplitudeEntity::FindNearest(GetActor());
    bool             hasEntity = entity && entity->IsValid();

    // Resolve spatialization target
    switch (Spatialization)
    {
        case AmplitudeSpatialization::Auto:
            if (hasEntity)
            {
                _channel = Amplitude::Play(SoundObject, entity);
            }
            else
            {
                _channel = Amplitude::PlayAt(SoundObject, GetActor()->GetPosition());
            }
            break;
        case AmplitudeSpatialization::Entity:
            if (hasEntity)
                _channel = Amplitude::Play(SoundObject, entity);
            else
                LOG(Error, "AmplitudeEmitter '{0}' on '{1}' has no AmplitudeEntity for Entity spatialization.", Name, GetActor()->GetName());
            break;
        case AmplitudeSpatialization::Location:
            _channel = Amplitude::PlayAt(SoundObject, GetActor()->GetPosition());
            break;
        case AmplitudeSpatialization::None:
            _channel = Amplitude::Play(SoundObject);
            break;
    }
}

void AmplitudeEmitter::ReleaseChannel()
{
    if (!_channel)
        return;
    if (_channel.Get()->IsValid())
        _channel.Get()->Stop(0.0f);
    _channel.Get()->DeleteObject();
    _channel = nullptr;
}

void AmplitudeEmitter::Stop()
{
    if (_channel && _channel.Get()->IsValid())
        _channel.Get()->Stop(0.0f);
}

void AmplitudeEmitter::Pause()
{
    if (_channel && _channel.Get()->IsValid())
        _channel.Get()->Pause(0.0f);
}

void AmplitudeEmitter::Resume()
{
    if (_channel && _channel.Get()->IsValid())
        _channel.Get()->Resume(0.0f);
}

bool AmplitudeEmitter::GetIsPlaying() const
{
    return _channel && _channel.Get()->IsValid() && _channel.Get()->IsPlaying();
}

AmplitudeChannel* AmplitudeEmitter::GetChannel() const
{
    return _channel ? _channel.Get() : nullptr;
}

void AmplitudeEmitter::OnStart()
{
    if (GetPlayOnStart())
        Play();
}

void AmplitudeEmitter::OnEnable()
{
    if (GetPlayOnEnable())
        Play();
}

void AmplitudeEmitter::OnDisable()
{
    if (StopOnDisable)
        ReleaseChannel();
}

#if USE_EDITOR
void AmplitudeEmitter::SetEditorAttenuation(int32 kind, const Array<float>& inner, const Array<float>& outer)
{
    _attenuationKind = kind;
    _attenuationInner = AmplitudeAudio::AmLengthsToFlaxUnits(inner);
    _attenuationOuter = AmplitudeAudio::AmLengthsToFlaxUnits(outer);
}

void AmplitudeEmitter::ClearEditorAttenuation()
{
    _attenuationKind = -1;
    _attenuationInner.Clear();
    _attenuationOuter.Clear();
}

void AmplitudeEmitter::OnDebugDrawSelected()
{
    if (_attenuationKind >= 0 && GetActor())
    {
        const Vector3 pos = GetActor()->GetPosition();
        const Quaternion rot = GetActor()->GetOrientation();
        const Color innerColor = Color::Yellow;
        Color outerColor = Color::Yellow;
        outerColor.A *= 0.5f;

        switch (_attenuationKind)
        {
        case 0: // Sphere
            if (_attenuationInner.Count() >= 1)
                DebugDraw::DrawWireSphere(BoundingSphere(pos, _attenuationInner[0]), innerColor);
            if (_attenuationOuter.Count() >= 1)
                DebugDraw::DrawWireSphere(BoundingSphere(pos, _attenuationOuter[0]), outerColor);
            break;
        case 1: // Box
            if (_attenuationInner.Count() >= 3)
            {
                OrientedBoundingBox innerObb(Vector3::Zero, Vector3(_attenuationInner[0], _attenuationInner[1], _attenuationInner[2]));
                innerObb.Transform(GetActor()->GetTransform());
                DebugDraw::DrawWireBox(innerObb, innerColor);
            }
            if (_attenuationOuter.Count() >= 3)
            {
                OrientedBoundingBox outerObb(Vector3::Zero, Vector3(_attenuationOuter[0], _attenuationOuter[1], _attenuationOuter[2]));
                outerObb.Transform(GetActor()->GetTransform());
                DebugDraw::DrawWireBox(outerObb, outerColor);
            }
            break;
        case 2: // Capsule
            if (_attenuationInner.Count() >= 2)
            {
                const float r = _attenuationInner[0];
                const float length = Math::Max(2.0f * (_attenuationInner[1] - r), 0.0f);
                DebugDraw::DrawWireCapsule(pos, AmplitudeAudio::Spatial::CapsuleDrawRotation(rot), r, length, innerColor);
            }
            if (_attenuationOuter.Count() >= 2)
            {
                const float r = _attenuationOuter[0];
                const float length = Math::Max(2.0f * (_attenuationOuter[1] - r), 0.0f);
                DebugDraw::DrawWireCapsule(pos, AmplitudeAudio::Spatial::CapsuleDrawRotation(rot), r, length, outerColor);
            }
            break;
        case 3: // Cone
            if (_attenuationInner.Count() >= 2)
            {
                const float r = _attenuationInner[0];
                const float h = _attenuationInner[1];
                const Vector3 forward = rot * Vector3::Forward;
                const Vector3 right = rot * Vector3::Right;
                const Vector3 up = rot * Vector3::Up;
                const Vector3 baseCenter = pos + forward * h;
                Vector3 prev = baseCenter + right * r;
                for (int32 i = 1; i <= 24; i++)
                {
                    const float angle = i * (TWO_PI / 24.0f);
                    const Vector3 rim = baseCenter + (right * Math::Cos(angle) + up * Math::Sin(angle)) * r;
                    DebugDraw::DrawLine(prev, rim, innerColor);
                    if (i % 3 == 0)
                        DebugDraw::DrawLine(pos, rim, innerColor);
                    prev = rim;
                }
            }
            if (_attenuationOuter.Count() >= 2)
            {
                const float r = _attenuationOuter[0];
                const float h = _attenuationOuter[1];
                const Vector3 forward = rot * Vector3::Forward;
                const Vector3 right = rot * Vector3::Right;
                const Vector3 up = rot * Vector3::Up;
                const Vector3 baseCenter = pos + forward * h;
                Vector3 prev = baseCenter + right * r;
                for (int32 i = 1; i <= 24; i++)
                {
                    const float angle = i * (TWO_PI / 24.0f);
                    const Vector3 rim = baseCenter + (right * Math::Cos(angle) + up * Math::Sin(angle)) * r;
                    DebugDraw::DrawLine(prev, rim, outerColor);
                    if (i % 3 == 0)
                        DebugDraw::DrawLine(pos, rim, outerColor);
                    prev = rim;
                }
            }
            break;
        }
    }

    Script::OnDebugDrawSelected();
}
#endif

void AmplitudeEmitter::Serialize(SerializeStream& stream, const void* otherObj)
{
    Script::Serialize(stream, otherObj);
    SERIALIZE_GET_OTHER_OBJ(AmplitudeEmitter);
    SERIALIZE_MEMBER(Name, Name);
    SERIALIZE_MEMBER(SoundObject, SoundObject);
    SERIALIZE_MEMBER(AutoPlay, AutoPlay);
    SERIALIZE_MEMBER(StopOnDisable, StopOnDisable);
    SERIALIZE_MEMBER(Spatialization, Spatialization);
}

void AmplitudeEmitter::Deserialize(DeserializeStream& stream, ISerializeModifier* modifier)
{
    Script::Deserialize(stream, modifier);
    DESERIALIZE_MEMBER(Name, Name);
    DESERIALIZE_MEMBER(SoundObject, SoundObject);
    DESERIALIZE_MEMBER(AutoPlay, AutoPlay);
    DESERIALIZE_MEMBER(StopOnDisable, StopOnDisable);
    DESERIALIZE_MEMBER(Spatialization, Spatialization);
}