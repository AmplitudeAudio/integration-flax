// Copyright (c) 2026-present Sparky Studios. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <Engine/Scripting/Script.h>

/// <summary>Per-collider override: this surface obstructs by Amount regardless of its material.</summary>
API_CLASS(Namespace = "AmplitudeAudio", Attributes = "FlaxEngine.Category(\"Amplitude\")")
class AMPLITUDEAUDIO_API AmplitudeObstruction : public Script
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCRIPTING_TYPE(AmplitudeObstruction);

public:
    /// <summary>Obstruction amount (0 = transparent, 1 = fully solid).</summary>
    API_FIELD(Attributes = "EditorOrder(0), Limit(0, 1, 0.01f)")
    float Amount = 1.0f;
};
