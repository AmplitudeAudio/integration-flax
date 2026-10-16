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

#include "AmplitudeInstancedEmitter.h"

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

namespace
{
    bool IsInstanceEligible(AmplitudeSoundInstance* instance)
    {
        if (!instance)
            return false;
        return instance->GetScene() ? instance->IsActiveInHierarchy() : instance->GetIsActive();
    }
}

AmplitudeInstancedEmitter::AmplitudeInstancedEmitter(const SpawnParams& params)
    : Script(params)
{
}

void AmplitudeInstancedEmitter::SetMode(AmplitudeInstanceMode mode)
{
    if (InstanceMode == mode)
        return;

    InstanceMode = mode;
    if (GetIsPlaying())
    {
        Stop();
        Play();
    }
}

int32 AmplitudeInstancedEmitter::GetBoundInstanceCount() const
{
    return _bound.Count();
}

void AmplitudeInstancedEmitter::Play()
{
    AmplitudeEntity* entity = AmplitudeEntity::FindNearest(GetActor());

    if (!entity || !entity->IsValid())
    {
        LOG(Error, "[Amplitude] AmplitudeInstancedEmitter '{0}' requires an AmplitudeEntity ancestor.", Name);
        return;
    }

    ReleaseChannel();

    AmplitudeChannel* channel = Amplitude::Play(SoundObject, entity);
    if (!channel || !channel->IsValid())
        return;

    _channel = channel;
    _channel->GetHandle().EnableInstancing(InstanceMode == AmplitudeInstanceMode::Separate ?
        SparkyStudios::Audio::Amplitude::eChannelInstanceMode_Separate :
        SparkyStudios::Audio::Amplitude::eChannelInstanceMode_Blended);

    Reconcile();
}

void AmplitudeInstancedEmitter::Stop()
{
    ReleaseChannel();
}

void AmplitudeInstancedEmitter::Pause()
{
    if (_channel && _channel.Get()->IsValid())
        _channel.Get()->Pause();
}

void AmplitudeInstancedEmitter::Resume()
{
    if (_channel && _channel.Get()->IsValid())
        _channel.Get()->Resume();
}

bool AmplitudeInstancedEmitter::GetIsPlaying() const
{
    return _channel && _channel.Get()->IsValid() && _channel.Get()->IsPlaying();
}

AmplitudeChannel* AmplitudeInstancedEmitter::GetChannel() const
{
    return _channel ? _channel.Get() : nullptr;
}

void AmplitudeInstancedEmitter::Reconcile()
{
    if (!_channel || !_channel.Get()->IsValid())
    {
        UnbindAll();
        return;
    }
    const auto& channel = _channel.Get()->GetHandle();

    // Drop what is no longer wanted: removed from the array, destroyed, or inactive.
    for (int32 i = _bound.Count() - 1; i >= 0; --i)
    {
        AmplitudeSoundInstance* instance = _bound[i].Instance.Get();
        if (instance && Instances.Contains(instance) && IsInstanceEligible(instance) && instance->GetBoundId() == _bound[i].Id)
            continue;
        channel.RemoveInstance(_bound[i].Id);
        if (instance)
            instance->Unbind();
        _bound.RemoveAt(i);
    }

    // Bind what is wanted and not bound yet.
    for (const auto& ref : Instances)
    {
        AmplitudeSoundInstance* instance = ref.Get();
        if (!instance || !IsInstanceEligible(instance) || instance->IsBound())
            continue;
        const auto handle = channel.AddInstance(AmplitudeAudio::FlaxVec3ToAmVec3(instance->GetPosition()));
        if (!handle.Valid())
            continue;
        instance->Bind(handle);
        _bound.Add({ instance, handle.GetId() });
    }
}

void AmplitudeInstancedEmitter::UnbindAll()
{
    const bool channelAlive = _channel && _channel.Get()->IsValid();
    for (const auto& b : _bound)
    {
        if (channelAlive)
            _channel.Get()->GetHandle().RemoveInstance(b.Id);
        AmplitudeSoundInstance* instance = b.Instance.Get();
        if (instance)
            instance->Unbind();
    }
    _bound.Clear();
}

void AmplitudeInstancedEmitter::ReleaseChannel()
{
    if (_channel)
    {
        UnbindAll();
        if (_channel.Get()->IsValid())
            _channel.Get()->Stop(0.0f);
        _channel.Get()->DeleteObject();
        _channel = nullptr;
    }
}

void AmplitudeInstancedEmitter::OnStart()
{
    Script::OnStart();
    if (AutoPlay == AmplitudeAutoPlayMode::OnStart)
        Play();
}

void AmplitudeInstancedEmitter::OnEnable()
{
    Script::OnEnable();
    if (AutoPlay == AmplitudeAutoPlayMode::OnEnable)
        Play();
}

void AmplitudeInstancedEmitter::OnDisable()
{
    if (StopOnDisable)
        ReleaseChannel();
    else
        UnbindAll();

    Script::OnDisable();
}

void AmplitudeInstancedEmitter::OnDestroy()
{
    ReleaseChannel();
    Script::OnDestroy();
}

void AmplitudeInstancedEmitter::OnUpdate()
{
    Script::OnUpdate();

    if (_channel)
    {
        // Paused, fading and switching channels are still alive; only a stopped one is released.
        if (!_channel->IsValid() || _channel->GetState() == AmplitudeChannelState::Stopped)
            ReleaseChannel();
        else
            Reconcile();
    }
}

#if USE_EDITOR
void AmplitudeInstancedEmitter::SetEditorAttenuation(int32 kind, const Array<float>& inner, const Array<float>& outer)
{
    _attenuationKind = kind;
    _attenuationInner = inner;
    _attenuationOuter = outer;
}

void AmplitudeInstancedEmitter::ClearEditorAttenuation()
{
    _attenuationKind = -1;
    _attenuationInner.Clear();
    _attenuationOuter.Clear();
}

void AmplitudeInstancedEmitter::OnDebugDrawSelected()
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

void AmplitudeInstancedEmitter::Serialize(SerializeStream& stream, const void* otherObj)
{
    Script::Serialize(stream, otherObj);
    SERIALIZE_GET_OTHER_OBJ(AmplitudeInstancedEmitter);
    SERIALIZE(Name);
    SERIALIZE(SoundObject);
    SERIALIZE(InstanceMode);
    SERIALIZE(AutoPlay);
    SERIALIZE(StopOnDisable);
    SERIALIZE(Instances);
}

void AmplitudeInstancedEmitter::Deserialize(DeserializeStream& stream, ISerializeModifier* modifier)
{
    Script::Deserialize(stream, modifier);
    DESERIALIZE(Name);
    DESERIALIZE(SoundObject);
    DESERIALIZE(InstanceMode);
    DESERIALIZE(AutoPlay);
    DESERIALIZE(StopOnDisable);
    DESERIALIZE(Instances);
}
