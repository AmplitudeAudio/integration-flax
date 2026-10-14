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

#include "AmplitudeListener.h"

#include <Engine/Serialization/Serialization.h>

#include <AmplitudeAudio/Core/Helpers.h>
#include <AmplitudeAudio/Runtime/AmplitudeService.h>

using AmplitudeAudio::AmplitudeService;
using SparkyStudios::Audio::Amplitude::Listener;

AmplitudeListener::AmplitudeListener(const SpawnParams& params)
    : Actor(params)
{
}

bool AmplitudeListener::IsValid() const
{
    return _listener.Valid();
}

uint64 AmplitudeListener::GetListenerId() const
{
    return _id;
}

void AmplitudeListener::OnEngineStarted()
{
    auto* engine = AmplitudeService::Get().GetEngine();
    if (!engine)
        return;

    _id = AmplitudeService::Get().AllocateId();
    _listener = engine->AddListener(static_cast<SparkyStudios::Audio::Amplitude::AmObjectID>(_id));
    PushState();
    if (IsDefault)
        engine->SetDefaultListener(&_listener);
}

void AmplitudeListener::OnEngineStopping()
{
    if (_listener.Valid())
    {
        _listener.Clear();
    }
    _id = 0;
}

void AmplitudeListener::OnEnable()
{
    Actor::OnEnable();
    if (!_registered)
    {
        AmplitudeService::Get().AddClient(this);
        _registered = true;
    }
}

void AmplitudeListener::OnDisable()
{
    if (_registered)
    {
        AmplitudeService::Get().RemoveClient(this);
        _registered = false;
    }
    if (_listener.Valid())
    {
        OnEngineStopping();
    }
    Actor::OnDisable();
}

void AmplitudeListener::OnTransformChanged()
{
    Actor::OnTransformChanged();
    if (_listener.Valid())
        PushState();
}

void AmplitudeListener::PushState() const
{
    _listener.SetLocation(AmplitudeAudio::FlaxVec3ToAmVec3(GetPosition()));
    _listener.SetOrientation(AmplitudeAudio::FlaxOrientationToAm(GetTransform().GetForward(), GetTransform().GetUp()));
    _listener.SetDirectivity(Directivity, Sharpness);
    _listener.Update();
}

void AmplitudeListener::Serialize(SerializeStream& stream, const void* otherObj)
{
    Actor::Serialize(stream, otherObj);
    SERIALIZE_GET_OTHER_OBJ(AmplitudeListener);
    SERIALIZE_MEMBER(IsDefault, IsDefault);
    SERIALIZE_MEMBER(Directivity, Directivity);
    SERIALIZE_MEMBER(Sharpness, Sharpness);
}

void AmplitudeListener::Deserialize(DeserializeStream& stream, ISerializeModifier* modifier)
{
    Actor::Deserialize(stream, modifier);
    DESERIALIZE_MEMBER(IsDefault, IsDefault);
    DESERIALIZE_MEMBER(Directivity, Directivity);
    DESERIALIZE_MEMBER(Sharpness, Sharpness);
}
