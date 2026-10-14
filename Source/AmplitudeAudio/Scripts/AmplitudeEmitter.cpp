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
#include <Engine/Serialization/Serialization.h>

#include <AmplitudeAudio/Actors/AmplitudeEntity.h>
#include <AmplitudeAudio/Amplitude.h>
#include <AmplitudeAudio/Types/AmplitudeChannel.h>

AmplitudeEmitter::AmplitudeEmitter(const SpawnParams& params)
    : Script(params)
{
}

void AmplitudeEmitter::Play()
{
    if (!SoundObject.IsValid())
        return;

    // Stop any current channel first
    if (_channel && _channel.Get()->IsValid())
        _channel.Get()->Stop(0.0f);

    AmplitudeEntity* entity = AmplitudeEntity::FindNearest(GetActor());
    bool hasEntity = entity && entity->IsValid();

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
    if (PlayOnStart)
        Play();
}

void AmplitudeEmitter::OnDisable()
{
    if (StopOnDisable)
        Stop();
}

void AmplitudeEmitter::Serialize(SerializeStream& stream, const void* otherObj)
{
    Script::Serialize(stream, otherObj);
    SERIALIZE_GET_OTHER_OBJ(AmplitudeEmitter);
    SERIALIZE_MEMBER(Name, Name);
    SERIALIZE_MEMBER(SoundObject, SoundObject);
    SERIALIZE_MEMBER(PlayOnStart, PlayOnStart);
    SERIALIZE_MEMBER(StopOnDisable, StopOnDisable);
    SERIALIZE_MEMBER(Spatialization, Spatialization);
}

void AmplitudeEmitter::Deserialize(DeserializeStream& stream, ISerializeModifier* modifier)
{
    Script::Deserialize(stream, modifier);
    DESERIALIZE_MEMBER(Name, Name);
    DESERIALIZE_MEMBER(SoundObject, SoundObject);
    DESERIALIZE_MEMBER(PlayOnStart, PlayOnStart);
    DESERIALIZE_MEMBER(StopOnDisable, StopOnDisable);
    DESERIALIZE_MEMBER(Spatialization, Spatialization);
}
