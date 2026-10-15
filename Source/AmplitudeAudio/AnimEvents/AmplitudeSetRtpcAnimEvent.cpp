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

#include "AmplitudeSetRtpcAnimEvent.h"

// clang-format off
#include <AmplitudeAudio/Core/Cpp20Compat.h>
// clang-format on

#include <AmplitudeAudio/Amplitude.h>

AmplitudeSetRtpcAnimEvent::AmplitudeSetRtpcAnimEvent(const SpawnParams& params)
    : AnimEvent(params)
{
#if USE_EDITOR
    Color = ::Color(0.6f, 1.0f, 0.5f);
#endif
}

void AmplitudeSetRtpcAnimEvent::OnEvent(AnimatedModel* actor, Animation* anim, float time, float deltaTime)
{
    if (Rtpc.IsValid())
        Amplitude::SetRtpcValue(Rtpc, Value);
}