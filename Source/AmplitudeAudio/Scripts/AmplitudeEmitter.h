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

#include <Engine/Core/Math/Vector3.h>
#include <Engine/Scripting/Script.h>
#include <Engine/Scripting/ScriptingObjectReference.h>

#include <AmplitudeAudio/Types/AmplitudeRefs.h>
#include <AmplitudeAudio/Scripts/IAmplitudeEmitter.h>

class AmplitudeChannel;

/// <summary>How the emitter positions its sound.</summary>
API_ENUM(Namespace = "AmplitudeAudio")
enum class AmplitudeSpatialization
{
    Auto = 0, Entity = 1, Location = 2, None = 3,
};

/// <summary>Plays a sound object from an actor. Attach to an actor with an AmplitudeEntity ancestor.</summary>
API_CLASS(Namespace = "AmplitudeAudio", Attributes = "FlaxEngine.Category(\"Amplitude\")")
class AMPLITUDEAUDIO_API AmplitudeEmitter : public Script, public IAmplitudeEmitter
{
    DECLARE_SCRIPTING_TYPE(AmplitudeEmitter);

public:
    /// <summary>The name of this emitter.</summary>
    API_FIELD(Attributes = "EditorOrder(0)")
    ::String Name;

    /// <summary>The sound object to play.</summary>
    API_FIELD(Attributes = "EditorOrder(1)")
    AmplitudeSoundObjectRef SoundObject;

    /// <summary>Whether to play on start.</summary>
    API_FIELD(Attributes = "EditorOrder(2)")
    bool PlayOnStart = false;

    /// <summary>Whether to stop on disable.</summary>
    API_FIELD(Attributes = "EditorOrder(3)")
    bool StopOnDisable = true;

    /// <summary>How the emitter positions its sound.</summary>
    API_FIELD(Attributes = "EditorOrder(4)")
    AmplitudeSpatialization Spatialization = AmplitudeSpatialization::Auto;

    /// <summary>Plays the sound object.</summary>
    API_FUNCTION()
    void Play() override;

    /// <summary>Stops the sound.</summary>
    API_FUNCTION()
    void Stop() override;

    /// <summary>Pauses the sound.</summary>
    API_FUNCTION()
    void Pause() override;

    /// <summary>Resumes the sound.</summary>
    API_FUNCTION()
    void Resume() override;

    /// <summary>Whether the sound is currently playing.</summary>
    API_PROPERTY()
    bool GetIsPlaying() const override;

    /// <summary>The current channel, or null.</summary>
    API_PROPERTY()
    AmplitudeChannel* GetChannel() const override;

    // IAmplitudeEmitter
    ::String GetName() const override { return Name; }
    AmplitudeSoundObjectRef GetSoundObject() const override { return SoundObject; }
    bool GetPlayOnStart() const override { return PlayOnStart; }
    bool GetStopOnDisable() const override { return StopOnDisable; }

    // Script
    void OnStart() override;
    void OnDisable() override;
    void Serialize(SerializeStream& stream, const void* otherObj) override;
    void Deserialize(DeserializeStream& stream, ISerializeModifier* modifier) override;

private:
    ScriptingObjectReference<AmplitudeChannel> _channel;
};
