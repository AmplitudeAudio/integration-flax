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

#include <Engine/Core/Types/String.h>

#include <AmplitudeAudio/Types/AmplitudeRefs.h>

class AmplitudeChannel;

/// <summary>Interface for Amplitude emitters that can play sound objects (C++ only, not exposed to C#).</summary>
class AMPLITUDEAUDIO_API IAmplitudeEmitter
{
public:
    virtual ~IAmplitudeEmitter() = default;

    virtual ::String GetName() const = 0;
    virtual AmplitudeSoundObjectRef GetSoundObject() const = 0;
    virtual bool GetPlayOnStart() const = 0;
    virtual bool GetStopOnDisable() const = 0;
    virtual void Play() = 0;
    virtual void Stop() = 0;
    virtual void Pause() = 0;
    virtual void Resume() = 0;
    virtual bool GetIsPlaying() const = 0;
    virtual AmplitudeChannel* GetChannel() const = 0;
};
