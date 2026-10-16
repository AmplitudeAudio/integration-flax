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

namespace AmplitudeAudio
{
    /// <summary>Implemented by scene objects that hold SDK handles, which exist only while the engine runs.</summary>
    class AMPLITUDEAUDIO_API IAmplitudeServiceClient
    {
    public:
        virtual ~IAmplitudeServiceClient() = default;

        /// <summary>The engine just started (or the client was added while it runs). Create SDK handles here.</summary>
        virtual void OnEngineStarted() = 0;

        /// <summary>The engine is about to stop; handles are still valid. Release them here.</summary>
        virtual void OnEngineStopping() = 0;
    };
} // namespace AmplitudeAudio
