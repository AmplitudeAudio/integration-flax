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

#include <AmplitudeAudio/Types/AmplitudeRefs.h>

/// <summary>Sets a switch state when the animation reaches this event.</summary>
API_CLASS(Namespace = "AmplitudeAudio")
class AMPLITUDEAUDIO_API AmplitudeSetSwitchAnimEvent : public AnimEvent
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCRIPTING_TYPE(AmplitudeSetSwitchAnimEvent);

public:
    /// <summary>The switch group to set.</summary>
    API_FIELD(Attributes = "EditorOrder(0)")
    AmplitudeSwitchRef Switch;
    /// <summary>The state to select.</summary>
    API_FIELD(Attributes = "EditorOrder(1)")
    AmplitudeSwitchStateRef State;
    void OnEvent(AnimatedModel* actor, Animation* anim, float time, float deltaTime) override;
};