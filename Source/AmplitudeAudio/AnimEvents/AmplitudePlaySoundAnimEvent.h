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
#include <Engine/Core/Types/String.h>

#include <AmplitudeAudio/Types/AmplitudeRefs.h>

/// <summary>Plays a sound object when the animation reaches it.</summary>
API_CLASS(Namespace = "AmplitudeAudio")
class AMPLITUDEAUDIO_API AmplitudePlaySoundAnimEvent : public AnimEvent
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCRIPTING_TYPE(AmplitudePlaySoundAnimEvent);

public:
    /// <summary>The sound object to play.</summary>
    API_FIELD(Attributes = "EditorOrder(0)")
    AmplitudeSoundObjectRef SoundObject;
    /// <summary>Optional matching AmplitudeEntity ancestor name.</summary>
    API_FIELD(Attributes = "EditorOrder(1)")
    ::String EntityName;
    /// <summary>Optional bone to play at in world space.</summary>
    API_FIELD(Attributes = "EditorOrder(2)")
    ::String BoneName;

    void OnEvent(AnimatedModel* actor, Animation* anim, float time, float deltaTime) override;

private:
    bool _reportedUnknownBone = false;
};