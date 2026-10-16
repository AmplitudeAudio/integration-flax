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

#include "AmplitudeEvent.h"

// clang-format off
#include <AmplitudeAudio/Core/Cpp20Compat.h>
// clang-format on

#include <Engine/Core/Log.h>
#include <Engine/Serialization/Serialization.h>

#include <AmplitudeAudio/Actors/AmplitudeEntity.h>
#include <AmplitudeAudio/Amplitude.h>
#include <AmplitudeAudio/Types/AmplitudeEventHandle.h>

using SparkyStudios::Audio::Amplitude::EventCanceler;

AmplitudeEvent::AmplitudeEvent(const SpawnParams& params)
    : Script(params)
{
}

AmplitudeEventHandle* AmplitudeEvent::Trigger()
{
    for (int32 i = _handles.Count() - 1; i >= 0; --i)
    {
        if (!_handles[i] || !_handles[i].Get()->IsValid())
        {
            if (_handles[i])
                _handles[i].Get()->DeleteObject();
            _handles.RemoveAt(i);
        }
    }
    AmplitudeEntity* entity = EntityOverride ? EntityOverride.Get() : AmplitudeEntity::FindNearest(GetActor());
    if (!entity)
    {
        LOG(Error, "AmplitudeEvent on '{0}' has no AmplitudeEntity (itself, a parent, or EntityOverride).", GetActor()->GetName());
        return AmplitudeEventHandle::Create(EventCanceler());
    }
    AmplitudeEventHandle* handle = Amplitude::TriggerEvent(Event, entity);
    _handles.Add(handle);
    return handle;
}

void AmplitudeEvent::CancelAll()
{
    for (auto& handle : _handles)
    {
        if (handle && handle.Get()->IsValid())
            handle.Get()->Cancel();
        if (handle)
            handle.Get()->DeleteObject();
    }
    _handles.Clear();
}

void AmplitudeEvent::OnStart()
{
    if (AutoTrigger == AmplitudeAutoTriggerMode::OnStart)
        Trigger();
}

void AmplitudeEvent::OnEnable()
{
    if (AutoTrigger == AmplitudeAutoTriggerMode::OnEnable)
        Trigger();
}

void AmplitudeEvent::OnDisable()
{
    if (CancelOnDisable)
        CancelAll();
}

void AmplitudeEvent::Serialize(SerializeStream& stream, const void* otherObj)
{
    Script::Serialize(stream, otherObj);
    SERIALIZE_GET_OTHER_OBJ(AmplitudeEvent);
    SERIALIZE_MEMBER(Event, Event);
    SERIALIZE_MEMBER(EntityOverride, EntityOverride);
    SERIALIZE_MEMBER(AutoTrigger, AutoTrigger);
    SERIALIZE_MEMBER(CancelOnDisable, CancelOnDisable);
}

void AmplitudeEvent::Deserialize(DeserializeStream& stream, ISerializeModifier* modifier)
{
    Script::Deserialize(stream, modifier);
    DESERIALIZE_MEMBER(Event, Event);
    DESERIALIZE_MEMBER(EntityOverride, EntityOverride);
    DESERIALIZE_MEMBER(AutoTrigger, AutoTrigger);
    DESERIALIZE_MEMBER(CancelOnDisable, CancelOnDisable);
}
