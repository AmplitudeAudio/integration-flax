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

#include <memory>

#include <AmplitudeAudio/Core/Cpp20Compat.h>

#include <Engine/Core/Math/Vector3.h>
#include <Engine/Scripting/ScriptingObject.h>

#include <SparkyStudios/Audio/Amplitude/Core/Playback/Channel.h>

/// <summary>Playback state of a channel.</summary>
API_ENUM(Namespace = "AmplitudeAudio")
enum class AmplitudeChannelState
{
    Stopped = 0, Playing = 1, FadingIn = 2, FadingOut = 3, SwitchingState = 4, Paused = 5, Pending = 6,
};

/// <summary>Wraps an Amplitude Channel handle. Created by the plugin, not spawnable from C#.</summary>
API_CLASS(NoSpawn, Namespace = "AmplitudeAudio")
class AMPLITUDEAUDIO_API AmplitudeChannel : public ScriptingObject
{
    DECLARE_SCRIPTING_TYPE_NO_SPAWN(AmplitudeChannel);

public:
    struct CallbackState { AmplitudeChannel* Owner; };

    static AmplitudeChannel* Create(const SparkyStudios::Audio::Amplitude::Channel& channel);
    ~AmplitudeChannel();

    API_PROPERTY() bool IsValid() const;
    API_PROPERTY() bool IsPlaying() const;
    API_PROPERTY() AmplitudeChannelState GetState() const;
    API_PROPERTY() float GetGain() const;
    API_PROPERTY() void SetGain(float value);
    API_PROPERTY() float GetPlaybackPosition() const;
    API_PROPERTY() Vector3 GetLocation() const;
    API_PROPERTY() void SetLocation(const Vector3& value);
    API_FUNCTION() void Stop(float fadeSeconds = 0.0f);
    API_FUNCTION() void Pause(float fadeSeconds = 0.0f);
    API_FUNCTION() void Resume(float fadeSeconds = 0.0f);

    API_EVENT() Delegate<> OnBegin;
    API_EVENT() Delegate<> OnEnd;
    API_EVENT() Delegate<> OnPause;
    API_EVENT() Delegate<> OnResume;
    API_EVENT() Delegate<> OnStop;
    API_EVENT() Delegate<> OnLoop;

private:
    SparkyStudios::Audio::Amplitude::Channel _channel;
    std::shared_ptr<CallbackState> _state;
};
