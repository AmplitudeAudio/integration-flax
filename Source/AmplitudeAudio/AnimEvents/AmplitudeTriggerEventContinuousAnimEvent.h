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

#pragma once

#include <AmplitudeAudio/Core/Cpp20Compat.h>

#include <Engine/Animations/AnimEvent.h>
#include <Engine/Core/Collections/Dictionary.h>
#include <Engine/Core/Types/String.h>
#include <Engine/Scripting/ScriptingObjectReference.h>

#include <AmplitudeAudio/Types/AmplitudeRefs.h>

class AmplitudeEventHandle;

/// <summary>Triggers an event while an animation event range is active and cancels it on end.</summary>
API_CLASS(Namespace = "AmplitudeAudio")
class AMPLITUDEAUDIO_API AmplitudeTriggerEventContinuousAnimEvent : public AnimContinuousEvent
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCRIPTING_TYPE(AmplitudeTriggerEventContinuousAnimEvent);

public:
    /// <summary>The event to trigger.</summary>
    API_FIELD(Attributes = "EditorOrder(0)")
    AmplitudeEventRef Event;
    /// <summary>Optional matching AmplitudeEntity ancestor name.</summary>
    API_FIELD(Attributes = "EditorOrder(1)")
    ::String EntityName;
    ~AmplitudeTriggerEventContinuousAnimEvent();
    bool IsActive(AnimatedModel* model) const;
    void OnBegin(AnimatedModel* actor, Animation* anim, float time, float deltaTime) override;
    void OnEvent(AnimatedModel* actor, Animation* anim, float time, float deltaTime) override;
    void OnEnd(AnimatedModel* actor, Animation* anim, float time, float deltaTime) override;

private:
    void End(AnimatedModel* actor);
    bool _reported = false;
    Dictionary<AnimatedModel*, ScriptingObjectReference<AmplitudeEventHandle>> _active;
};