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

#include "AmplitudePlaySoundAnimEvent.h"

// clang-format off
#include <AmplitudeAudio/Core/Cpp20Compat.h>
// clang-format on

#include <Engine/Level/Actors/AnimatedModel.h>

#include <AmplitudeAudio/AnimEvents/AnimEventTarget.h>
#include <Engine/Core/Log.h>
#include <AmplitudeAudio/Types/AmplitudeChannel.h>

AmplitudePlaySoundAnimEvent::AmplitudePlaySoundAnimEvent(const SpawnParams& params)
    : AnimEvent(params)
{
#if USE_EDITOR
    Color = ::Color(0.4f, 0.8f, 1.0f);
#endif
}

void AmplitudePlaySoundAnimEvent::OnEvent(AnimatedModel* actor, Animation* anim, float time, float deltaTime)
{
    if (!SoundObject.IsValid() || !actor)
        return;
    if (BoneName.HasChars() && !AmplitudeAudio::HasAnimEventBone(actor, BoneName) && !_reportedUnknownBone)
    {
        _reportedUnknownBone = true;
        LOG(Warning, "Amplitude play animation event on '{0}' has unknown bone '{1}'. Using the model position.", actor->GetName(), BoneName);
    }
    AmplitudeAudio::PlayFromAnimEvent(actor, SoundObject, EntityName, BoneName)->DeleteObject();
}