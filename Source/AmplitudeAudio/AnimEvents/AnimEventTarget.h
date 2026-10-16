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

#include <Engine/Core/Math/Vector3.h>
#include <Engine/Core/Types/String.h>

#include <AmplitudeAudio/Types/AmplitudeRefs.h>

class AnimatedModel;
class AmplitudeChannel;
class AmplitudeEntity;

namespace AmplitudeAudio
{
    /// <summary>Finds the matching AmplitudeEntity ancestor of an animated model.</summary>
    AmplitudeEntity* FindAnimEventEntity(AnimatedModel* model, const ::StringView& entityName);
    AmplitudeEntity* ResolveTriggerEntity(AnimatedModel* model, const ::StringView& entityName, bool& reported, const Char* what);
    bool HasAnimEventBone(AnimatedModel* model, const ::StringView& boneName);
    /// <summary>World position of the named bone, or the model when it is empty or unknown.</summary>
    Vector3 GetAnimEventLocation(AnimatedModel* model, const ::StringView& boneName);
    /// <summary>Plays at the entity or location; the caller owns the returned wrapper.</summary>
    AmplitudeChannel* PlayFromAnimEvent(AnimatedModel* model, const AmplitudeSoundObjectRef& sound, const ::StringView& entityName, const ::StringView& boneName);
}