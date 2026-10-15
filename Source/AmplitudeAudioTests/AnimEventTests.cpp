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

#include "TestFixture.h"

#include <Engine/Level/Actors/AnimatedModel.h>
#include <Engine/Scripting/ScriptingObject.h>

#include <ThirdParty/catch2/catch.hpp>

#include <AmplitudeAudio/AnimEvents/AmplitudePlaySoundAnimEvent.h>
#include <AmplitudeAudio/AnimEvents/AmplitudePlaySoundContinuousAnimEvent.h>
#include <AmplitudeAudio/AnimEvents/AmplitudeSetRtpcAnimEvent.h>
#include <AmplitudeAudio/AnimEvents/AmplitudeSetSwitchAnimEvent.h>
#include <AmplitudeAudio/AnimEvents/AmplitudeTriggerEventAnimEvent.h>
#include <AmplitudeAudio/AnimEvents/AmplitudeTriggerEventContinuousAnimEvent.h>
#include <AmplitudeAudio/Amplitude.h>
#include <AmplitudeAudio/Types/AmplitudeChannel.h>

TEST_CASE("continuous play event safely ends without an active channel", "[anim-event][play]")
{
    auto* first = ScriptingObject::NewObject<AnimatedModel>();
    auto* second = ScriptingObject::NewObject<AnimatedModel>();
    auto* event = ScriptingObject::NewObject<AmplitudePlaySoundContinuousAnimEvent>();
    event->OnEnd(first, nullptr, 0, 0);
    event->OnBegin(first, nullptr, 0, 0);
    event->OnEnd(first, nullptr, 0, 0);
    CHECK_FALSE(event->IsActive(first));
    CHECK_FALSE(event->IsActive(second));
    event->DeleteObjectNow();
    first->DeleteObjectNow();
    second->DeleteObjectNow();
}

TEST_CASE("empty RTPC and switch events are harmless", "[anim-event][rtpc][switch]")
{
    auto* model = ScriptingObject::NewObject<AnimatedModel>();
    auto* rtpc = ScriptingObject::NewObject<AmplitudeSetRtpcAnimEvent>();
    auto* sw = ScriptingObject::NewObject<AmplitudeSetSwitchAnimEvent>();
    rtpc->OnEvent(model, nullptr, 0, 0);
    sw->OnEvent(model, nullptr, 0, 0);
    rtpc->DeleteObjectNow();
    sw->DeleteObjectNow();
    model->DeleteObjectNow();
    SUCCEED();
}