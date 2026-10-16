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

#include "AmplitudePlaySoundContinuousAnimEvent.h"

// clang-format off
#include <AmplitudeAudio/Core/Cpp20Compat.h>
// clang-format on

#include <Engine/Level/Actors/AnimatedModel.h>
#include <Engine/Core/Log.h>

#include <AmplitudeAudio/AnimEvents/AnimEventTarget.h>
#include <AmplitudeAudio/Types/AmplitudeChannel.h>

AmplitudePlaySoundContinuousAnimEvent::AmplitudePlaySoundContinuousAnimEvent(const SpawnParams& params)
    : AnimContinuousEvent(params)
{
#if USE_EDITOR
    Color = ::Color(0.4f, 0.8f, 1.0f);
#endif
}

void AmplitudePlaySoundContinuousAnimEvent::OnBegin(AnimatedModel* actor, Animation* anim, float time, float deltaTime)
{
    if (!SoundObject.IsValid() || !actor)
        return;
    if (BoneName.HasChars() && !AmplitudeAudio::HasAnimEventBone(actor, BoneName) && !_reportedUnknownBone)
    {
        _reportedUnknownBone = true;
        LOG(Warning, "Amplitude continuous play animation event on '{0}' has unknown bone '{1}'. Using the model position.", actor->GetName(), BoneName);
    }
    End(actor, 0.0f);
    AmplitudeChannel* channel = AmplitudeAudio::PlayFromAnimEvent(actor, SoundObject, EntityName, BoneName);
    if (!channel->IsValid())
    {
        channel->DeleteObject();
        return;
    }
    _active[actor] = channel;
}

void AmplitudePlaySoundContinuousAnimEvent::OnEvent(AnimatedModel* actor, Animation* anim, float time, float deltaTime)
{
    if (BoneName.IsEmpty() || !actor)
        return;
    auto* channel = _active.TryGet(actor);
    if (channel && *channel && channel->Get()->IsValid())
        channel->Get()->SetLocation(AmplitudeAudio::GetAnimEventLocation(actor, BoneName));
}

void AmplitudePlaySoundContinuousAnimEvent::OnEnd(AnimatedModel* actor, Animation* anim, float time, float deltaTime)
{
    End(actor, FadeOutSeconds);
}

void AmplitudePlaySoundContinuousAnimEvent::End(AnimatedModel* actor, float fadeSeconds)
{
    ScriptingObjectReference<AmplitudeChannel> channel;
    if (!actor || !_active.TryGet(actor, channel))
        return;
    _active.Remove(actor);
    if (channel)
    {
        if (channel->IsValid())
            channel->Stop(fadeSeconds);
        channel->DeleteObject();
    }
}

bool AmplitudePlaySoundContinuousAnimEvent::IsActive(AnimatedModel* model) const
{
    const auto* channel = _active.TryGet(model);
    return channel && *channel && channel->Get()->IsValid();
}

AmplitudePlaySoundContinuousAnimEvent::~AmplitudePlaySoundContinuousAnimEvent()
{
    for (auto& entry : _active)
    {
        if (entry.Value)
        {
            if (entry.Value->IsValid())
                entry.Value->Stop(0.0f);
            entry.Value->DeleteObject();
        }
    }
}