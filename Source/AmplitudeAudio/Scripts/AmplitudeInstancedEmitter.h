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

#include <AmplitudeAudio/Core/Cpp20Compat.h>

#include <Engine/Scripting/Script.h>
#include <Engine/Scripting/ScriptingObjectReference.h>

#include <SparkyStudios/Audio/Amplitude/Core/Playback/Channel.h>

#include <AmplitudeAudio/Actors/AmplitudeSoundInstance.h>
#include <AmplitudeAudio/Scripts/AmplitudeEmitter.h>
#include <AmplitudeAudio/Scripts/IAmplitudeEmitter.h>
#include <AmplitudeAudio/Types/AmplitudeRefs.h>

class AmplitudeChannel;

/// <summary>How instanced channels position their instances.</summary>
API_ENUM(Namespace = "AmplitudeAudio")
enum class AmplitudeInstanceMode
{
    Blended = 0,
    Separate = 1,
};

/// <summary>Plays a sound object across multiple AmplitudeSoundInstance locations.</summary>
API_CLASS(Namespace = "AmplitudeAudio", Attributes = "FlaxEngine.Category(\"Amplitude\")")
class AMPLITUDEAUDIO_API AmplitudeInstancedEmitter : public Script, public IAmplitudeEmitter
{
    DECLARE_SCRIPTING_TYPE(AmplitudeInstancedEmitter);

public:
    /// <summary>The name of this emitter.</summary>
    API_FIELD(Attributes = "EditorOrder(0)")
    ::String Name;

    /// <summary>The sound object to play.</summary>
    API_FIELD(Attributes = "EditorOrder(1)")
    AmplitudeSoundObjectRef SoundObject;

    /// <summary>Instance playback mode (Blended or Separate).</summary>
    API_FIELD(Attributes = "EditorOrder(2)")
    AmplitudeInstanceMode InstanceMode = AmplitudeInstanceMode::Blended;

    /// <summary>When to autoplay the sound.</summary>
    API_FIELD(Attributes = "EditorOrder(3)")
    AmplitudeAutoPlayMode AutoPlay = AmplitudeAutoPlayMode::None;

    /// <summary>Whether to stop on disable.</summary>
    API_FIELD(Attributes = "EditorOrder(4)")
    bool StopOnDisable = true;

    /// <summary>List of sound instance actors bound to this emitter.</summary>
    API_FIELD(Attributes = "EditorOrder(5)")
    Array<ScriptingObjectReference<AmplitudeSoundInstance>> Instances;

    /// <summary>Sets the instance playback mode, restarting playback if currently playing.</summary>
    API_PROPERTY()
    void SetMode(AmplitudeInstanceMode mode);

    /// <summary>Gets the instance playback mode.</summary>
    API_PROPERTY()
    AmplitudeInstanceMode GetMode() const { return InstanceMode; }

    /// <summary>Number of currently bound sound instances.</summary>
    API_PROPERTY()
    int32 GetBoundInstanceCount() const;

    /// <summary>Plays the sound object across all eligible instances.</summary>
    API_FUNCTION()
    void Play() override;

    /// <summary>Stops the sound and unbinds instances.</summary>
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
    bool GetPlayOnStart() const override { return AutoPlay == AmplitudeAutoPlayMode::OnStart; }
    bool GetPlayOnEnable() const override { return AutoPlay == AmplitudeAutoPlayMode::OnEnable; }
    bool GetStopOnDisable() const override { return StopOnDisable; }

#if USE_EDITOR
    API_FUNCTION()
    void SetEditorAttenuation(int32 kind, const Array<float>& inner, const Array<float>& outer);

    API_FUNCTION()
    void ClearEditorAttenuation();

    bool HasEditorAttenuation() const { return _attenuationKind >= 0; }

    void OnDebugDrawSelected() override;
#endif

    // Script
    void OnStart() override;
    void OnEnable() override;
    void OnDisable() override;
    void OnDestroy() override;
    void OnUpdate() override;
    void Serialize(SerializeStream& stream, const void* otherObj) override;
    void Deserialize(DeserializeStream& stream, ISerializeModifier* modifier) override;

private:
    struct Bound
    {
        ScriptingObjectReference<AmplitudeSoundInstance> Instance;
        SparkyStudios::Audio::Amplitude::AmChannelInstanceID Id = 0;
    };

    void Reconcile();
    void UnbindAll();
    void ReleaseChannel();

    ScriptingObjectReference<AmplitudeChannel> _channel;
    Array<Bound> _bound;
#if USE_EDITOR
    int32 _attenuationKind = -1;
    Array<float> _attenuationInner;
    Array<float> _attenuationOuter;
#endif
};
