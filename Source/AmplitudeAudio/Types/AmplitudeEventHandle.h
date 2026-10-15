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
#include <atomic>

#include <AmplitudeAudio/Core/Cpp20Compat.h>

#include <Engine/Scripting/ScriptingObject.h>

#include <AmplitudeAudio/Runtime/IAmplitudeServiceClient.h>

#include <SparkyStudios/Audio/Amplitude/Core/Event.h>

/// <summary>Wraps an Amplitude EventCanceler handle. Created by the plugin, not spawnable from C#.</summary>
API_CLASS(NoSpawn, Namespace = "AmplitudeAudio")
class AMPLITUDEAUDIO_API AmplitudeEventHandle : public ScriptingObject, public AmplitudeAudio::IAmplitudeServiceClient
{
    DECLARE_SCRIPTING_TYPE_NO_SPAWN(AmplitudeEventHandle);

public:
    explicit AmplitudeEventHandle(const SpawnParams& params)
        : ScriptingObject(params)
    {
    }

    static AmplitudeEventHandle* Create(const SparkyStudios::Audio::Amplitude::EventCanceler& canceler);
    API_PROPERTY() static int GetLiveCount();
    ~AmplitudeEventHandle();

    // IAmplitudeServiceClient
    void OnEngineStarted() override;
    void OnEngineStopping() override;

    API_PROPERTY() bool IsValid() const;
    API_FUNCTION() void Cancel();

    /// <summary>Raised when the event finishes; the argument is true when it was cancelled.</summary>
    API_EVENT() Delegate<bool> OnFinished;

private:
    static void ForwardFinish(const SparkyStudios::Audio::Amplitude::EventCanceler& canceler,
        std::function<void(bool aborted)> callback);

    struct CallbackState { AmplitudeEventHandle* Owner; };

    SparkyStudios::Audio::Amplitude::EventCanceler _canceler;
    std::shared_ptr<CallbackState> _state;
};
