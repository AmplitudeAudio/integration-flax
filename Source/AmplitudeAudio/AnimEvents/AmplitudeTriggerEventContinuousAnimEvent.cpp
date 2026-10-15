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

#include "AmplitudeTriggerEventContinuousAnimEvent.h"

// clang-format off
#include <AmplitudeAudio/Core/Cpp20Compat.h>
// clang-format on

#include <AmplitudeAudio/AnimEvents/AnimEventTarget.h>
#include <AmplitudeAudio/Amplitude.h>
#include <AmplitudeAudio/Types/AmplitudeEventHandle.h>

AmplitudeTriggerEventContinuousAnimEvent::AmplitudeTriggerEventContinuousAnimEvent(const SpawnParams& params)
    : AnimContinuousEvent(params)
{
#if USE_EDITOR
    Color = ::Color(1.0f, 0.7f, 0.3f);
#endif
}

void AmplitudeTriggerEventContinuousAnimEvent::OnBegin(AnimatedModel* actor, Animation* anim, float time, float deltaTime)
{
    if (!actor || !Event.IsValid())
        return;
    End(actor);
    AmplitudeEntity* entity = AmplitudeAudio::ResolveTriggerEntity(actor, EntityName, _reported, TEXT("continuous trigger"));
    if (entity)
        _active[actor] = Amplitude::TriggerEvent(Event, entity);
}

void AmplitudeTriggerEventContinuousAnimEvent::OnEvent(AnimatedModel* actor, Animation* anim, float time, float deltaTime)
{
}

void AmplitudeTriggerEventContinuousAnimEvent::OnEnd(AnimatedModel* actor, Animation* anim, float time, float deltaTime)
{
    End(actor);
}

void AmplitudeTriggerEventContinuousAnimEvent::End(AnimatedModel* actor)
{
    if (!actor)
        return;
    ScriptingObjectReference<AmplitudeEventHandle> handle;
    if (!_active.TryGet(actor, handle))
        return;
    _active.Remove(actor);
    if (handle)
    {
        if (handle->IsValid())
            handle->Cancel();
        handle->DeleteObject();
    }
}

bool AmplitudeTriggerEventContinuousAnimEvent::IsActive(AnimatedModel* model) const
{
    const auto* handle = _active.TryGet(model);
    return handle && *handle && handle->Get()->IsValid();
}

AmplitudeTriggerEventContinuousAnimEvent::~AmplitudeTriggerEventContinuousAnimEvent()
{
    for (auto& entry : _active)
    {
        if (entry.Value)
        {
            if (entry.Value->IsValid())
                entry.Value->Cancel();
            entry.Value->DeleteObject();
        }
    }
}