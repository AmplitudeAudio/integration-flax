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

#include <Engine/Core/Collections/Array.h>
#include <Engine/Core/Math/Vector3.h>
#include <Engine/Platform/Defines.h>

namespace AmplitudeAudio::Spatial
{
    /// <summary>Points at equal arc length along a polyline, always including the first point and, when it fits, the last. spacing <= 0 returns the input.</summary>
    AMPLITUDEAUDIO_API Array<Vector3> ResampleByArcLength(const Array<Vector3>& polyline, float spacing, bool closed);
} // namespace AmplitudeAudio::Spatial
